package com.android.server.rc632;

import android.util.Slog;
import android.content.Context;
import com.android.server.SystemService;


import android.os.Handler;
import android.os.RemoteException;
import android.rc632.*;

public class Rc632Service extends SystemService {

    private Context mContext;
    private IRc632ServiceReceiver receiver;
    private static final String TAG = "Rc632Service";
    private static final boolean DEBUG = true;
	private Handler piccScanHandler;
	private Runnable piccScanRunnable;
	private Thread thread;
	
			

	public Rc632Service(Context context) {
		super(context);
		this.mContext	=	context;
		this.piccScanHandler = new Handler();
		
		this.thread = new Thread(){
			int temp;
			@Override
			public void run() {
				while(true){
					super.run();
					temp = 0;//readEvent_native();
					Slog.d(TAG, "At Thread"+ temp +" = readEvent_native()");
					if(temp > 0){
						rc632CallBack(String.valueOf(temp));
					}
					try {
						sleep(1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					//piccScanHandler.postDelayed(this,1000);
				}
			}
			
		};
		
		this.piccScanRunnable = new Runnable() {
			int temp;
			public void run() {
				//Slog.d(TAG, "At runnable readEvent_native()");
				temp = readEvent_native();
				Slog.d(TAG, "At runnable"+ temp +" = readEvent_native()");
				//if(temp > 0){
					rc632CallBack(String.valueOf(temp));
				//}
				piccScanHandler.postDelayed(this,1000);
			}
		};
		
		//thread = new Thread(this.piccScanRunnable);
		if(DEBUG){
			Slog.d(TAG, "Build Rc632 service");
		}
		publishBinderService(Context.RC632_SERVICE,  new Rc632ServiceWrapper());
	}

	/* this function call when new Picc detection */
	private void rc632CallBack(String atqa){
		if(this.receiver != null){
			try{
				this.receiver.onPiccDetect(atqa);
			}catch(RemoteException e){
				Slog.d(TAG, "Can't send message to client"+ e.getMessage());
			}
			
		}
			
	}
	
	void addListener(IRc632ServiceReceiver receiver){
		Slog.v(TAG, "addListener(IRc632ServiceReceiver)");
		if (this.receiver == null){
			this.receiver = receiver;
			//this.piccScanHandler.post(this.piccScanRunnable);
			this.thread.start();
		}else{
			if (DEBUG) Slog.v(TAG, "listener already registered for ");
		}		
		 
	}
	
	void  removeListener(){
		if(this.receiver != null){
			this.receiver = null;
			//this.piccScanHandler.removeCallbacks(this.piccScanRunnable);
		}
		
	}
	
	public void onStart() {
		Slog.v(TAG, "Rc632Service onStart() OK");
		this.mNativePointer = initRc632_native();
		
	}

	private final class Rc632ServiceWrapper extends IRc632Service.Stub{
		
		private VirtualPicc mVirtualPicc;
		private PiccBlock mBlock;
		private PiccSector mSector;
		
		@Override
		public void setVirtualPicc(VirtualPicc virtualPicc){
			this.mVirtualPicc = virtualPicc;
			if(DEBUG){
				Slog.v(TAG, "setVirtualPicc OK");
				Slog.v(TAG, "VirtualPicc Card Size "+virtualPicc.getCardSize());
				Slog.v(TAG, "VirtualPicc DefaultKeyA "+virtualPicc.getDefaultKey().getKeyA());
				Slog.v(TAG, "VirtualPicc DefaultKeyB "+virtualPicc.getDefaultKey().getKeyB());
				
				Slog.v(TAG, "VirtualPicc SectorTypes "+virtualPicc.getSectorTypes().toString());
				Slog.v(TAG, "VirtualPicc SectorKeyMap "+virtualPicc.getSectorKey(1).getKeyA());
				
				
			}
		}
		
		@Override
		public PiccBlock writeBlock(byte[] data, int sectorIndex, int blockIndexWithinSector, char keyType) {
			byte[] key;
			byte nativeKeyType;
			int retVal;
			if(this.mVirtualPicc == null) return null;
			this.mBlock = this.mVirtualPicc.getBlankBlock(sectorIndex, blockIndexWithinSector);
			mBlock.setData(data);
			
			if(keyType == 'A' || keyType == 'a') {
				key = mBlock.getKey().getKeyA().getBytes();
				nativeKeyType = 0;//A
			}else{
				key =  mBlock.getKey().getKeyB().getBytes();
				nativeKeyType = 1;//B
			}
			//mBlock.setData(data);
			
			retVal = blockWrite_native(mBlock.data,mBlock.getBlockNo(),1,key,nativeKeyType);
			if(retVal == 0) return this.mBlock;
			return null;
		}
		@Override
		public PiccBlock readBlock(int sectorIndex, int blockIndexWithinSector, char keyType) {
			int retVal =-1;
			byte nativeKeyType;
			byte[] key;
			
			if(this.mVirtualPicc == null) return null;
			this.mBlock = this.mVirtualPicc.getBlankBlock(sectorIndex, blockIndexWithinSector);
			
			if(keyType == 'A') {
				key = mBlock.getKey().getKeyA().getBytes();
				nativeKeyType = 0;
			}else{
				key =  mBlock.getKey().getKeyB().getBytes();
				nativeKeyType = 1;
			}
			//mNativePointer = initRc632_native();
			//if(mNativePointer < 0)
			retVal = blockRead_native(mBlock.data,mBlock.getBlockNo(),1,key,nativeKeyType);
			if(retVal == 0) return this.mBlock;
			return null;
		}
		@Override
		public PiccSector writeSector(byte[] data, int sectorIndex, char keyType, boolean sectorTrailer) {
			int retVal;
			byte nativeKeyType;
			byte[] key;
			
			if(this.mVirtualPicc == null) return null;
			this.mSector = this.mVirtualPicc.getBlankSector(sectorIndex, sectorTrailer);
			
			if(keyType == 'A') {
				key = mSector.getKey().getKeyA().getBytes();
				nativeKeyType = 0;
			}else{
				key =  mSector.getKey().getKeyB().getBytes();
				nativeKeyType = 1;
			}
			mSector.setData(data);
			
			retVal = blockWrite_native(mSector.data,mSector.getBlockNo(),mSector.getBlockCount(),key,nativeKeyType);
			if(retVal == 0) return  this.mSector;
			return null;
		}
		@Override
		public PiccSector readSector(int sectorIndex, char keyType, boolean sectorTrailer) {
			int retVal;
			byte nativeKeyType;
			byte[] key;
			
			if(this.mVirtualPicc == null) return null;
			this.mSector = this.mVirtualPicc.getBlankSector(sectorIndex, sectorTrailer);
			
			if(keyType == 'A') {
				key = mSector.getKey().getKeyA().getBytes();
				nativeKeyType = 0;
			}else{
				key =  mSector.getKey().getKeyB().getBytes();
				nativeKeyType = 1;
			}
			retVal = blockRead_native(mBlock.data,mBlock.getBlockNo(),mSector.getBlockCount(),key,nativeKeyType);
			if(retVal == 0) return  this.mSector;
			return null;
		}
		
		@Override
		public int scanOn(){
			thread.notify();
			return scanOn_native(/*int poll_interval*/1, /*int poll_interval_min*/1, /*int poll_interval_max*/1);
		}
		
		@Override
		public int scanOff(){
			try {
				thread.wait();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return scanOff_native();
		}
		
		@Override
		public void startListening(IRc632ServiceReceiver receiver) {
			Slog.v(TAG, "startListening Call");
			addListener(receiver);
		}
		@Override // Binder call
		public void stopListening() {
			removeListener();
		}
	};
	
	private long mNativePointer;
	private static native long initRc632_native();
	private static native int blockRead_native(byte[] buffer,int start_block_no,int block_count,byte[] read_key, byte key_type);
	private static native int blockWrite_native(byte[] buffer,int start_block_no,int block_count,byte[] key, byte key_type);
	private static native int scanOn_native(int poll_interval, int poll_interval_min, int poll_interval_max);
	private static native int scanOff_native();
	private static native int readEvent_native();
}


