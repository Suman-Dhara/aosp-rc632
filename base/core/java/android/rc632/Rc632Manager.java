package android.rc632;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.rc632.IRc632Service;
import android.util.Slog;

/**
 * {@link Rc632Manager} is a service manager class of RC632 Service.
 * */

public class Rc632Manager {
	
	private static final int MSG_PICC_DETECTED = 100;
	private static final String TAG = "Rc632Manager";
	private IRc632Service mService;
	private Rc632ManagerReceiver mClientReceiver;
	private Context mContext;
	
	private Handler mHandler = new Handler(){

		/* (non-Javadoc)
		 * @see android.os.Handler#handleMessage(android.os.Message)
		 */
		@Override
		public void handleMessage(Message msg) {
			//super.handleMessage(msg);
			Slog.v(TAG,"At handleMessage \n");
			if(mClientReceiver != null){
				switch (msg.what) {
				case MSG_PICC_DETECTED:
					 mClientReceiver.onPiccDetect(String.valueOf(msg.obj));
					break;

				default:
					break;
				}
			}
		}
		
	};
	
	public Rc632Manager(Context contex, IRc632Service service){
        mContext = contex;
        mService = service;
        if (mService == null) {
            Slog.v(TAG, "Rc632ManagerService was null");
        }
	}

	
	private IRc632ServiceReceiver mServiceReceiver = new IRc632ServiceReceiver.Stub() {

		@Override
		public void onPiccDetect(String atq) throws RemoteException {
			Slog.v(TAG,"At mServiceReceiver onPiccDetect : "+atq+"\n");
			mClientReceiver.onPiccDetect(atq);
			//mHandler.obtainMessage(MSG_PICC_DETECTED, atq);
			
		}
		

	};
	
	
	public void setVirtualPicc(VirtualPicc virtualPicc){
		try{
			this.mService.setVirtualPicc(virtualPicc);
			Slog.v(TAG,"Set virtualPicc at manager \n");
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		
	}
	/**
	 * use for block write. 
	 * 
	 * @param data it is byte array of data.
	 * @param sectorIndex it is an integer value for sector number[0..n]
	 * @param blockIndexWithinSector it is a block number in sector. [0..maxBlockNoInSector]
	 * @param keyType it is one character [A/B]
	 * 
	 * @return {@link PiccBlock}
	 * */
	public PiccBlock writeBlock(byte[] data, int sectorIndex, int blockIndexWithinSector, char keyType){
		PiccBlock mBlock = null;
		try{
			mBlock = this.mService.writeBlock(data,sectorIndex,blockIndexWithinSector,keyType);
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		return mBlock;
		
	}
	
	/**
	 * use for block read.
	 * 
	 * @param sectorIndex it is integer sector index value[0..n] 
	 * @param blockIndexWithinSector it is a block number in sector. [0..maxBlockNoInSector]
	 * @param keyType it is one character [A/B]
	 * 
	 * @return {@link PiccBlock}
	 * */
	public PiccBlock readBlock(int sectorIndex, int blockIndexWithinSector, char keyType){
		PiccBlock mBlock = null;
		try{
			mBlock = this.mService.readBlock(sectorIndex,blockIndexWithinSector,keyType);
		}catch(RemoteException e){
			Slog.v(TAG, (e.getMessage()!= null)?e.getMessage():"readBlock RemoteException");
		}
		return mBlock;

	}
	
	public PiccSector writeSector(byte[] data, int sectorIndex, char keyType, boolean sectorTrailer){
		PiccSector mSector = null;
		try{
			mSector = this.mService.writeSector(data,sectorIndex,keyType,sectorTrailer);
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		return mSector;

	}

	public PiccSector readSector(int sectorIndex, char keyType, boolean sectorTrailer){
		PiccSector mSector = null;
		try{
			mSector = this.mService.readSector(sectorIndex,keyType,sectorTrailer);
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		return mSector;
	}
	
	public void addListener(Rc632ManagerReceiver receiver){
		this.mClientReceiver = receiver;
		try{
			this.mService.startListening(mServiceReceiver);
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}

	}
	
	public void removeListener(){
		this.mClientReceiver = null;
		try{
			this.mService.stopListening();
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
	}
	
	/**
	 * This function will stop picc event listener. 
	 * */
	public int piccScanOff(){
		int ret =1;
		try{
			ret = this.mService.scanOff();
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		return ret;
	}
	
	/**
	 * This function will start picc event listener.
	 * */
	public int piccScanOn(){
		int ret = 1;
		try{
			ret = this.mService.scanOn();
		}catch(RemoteException e){
			Slog.v(TAG, e.getMessage());
		}
		return ret;
	}

}
