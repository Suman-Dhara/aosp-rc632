package android.rc632;

import java.util.ArrayList;
import java.util.HashMap;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class VirtualPicc implements Parcelable{
	protected static final String TAG = "VirtualPicc";
	protected static final boolean DEBUG = true;
	private int cardSize;
	private HashMap <Integer,SectorKey> sectorKeyMap;
	private ArrayList<SectorType> sectorTypes;
	private SectorKey defaultKey;
	
	
	public VirtualPicc(int cardSize, String defaultKeyA, String defaultKeyB){
		this.cardSize = cardSize;
		this.sectorKeyMap = new HashMap <Integer,SectorKey>();
		this.sectorTypes = new ArrayList<SectorType>();
		this.defaultKey = new SectorKey(defaultKeyA, defaultKeyB);
	}
	
	private VirtualPicc(Parcel source){
		this.cardSize = source.readInt();
		this.sectorKeyMap = (HashMap<Integer,SectorKey>)source.readHashMap(SectorKey.class.getClassLoader());
		//source.readList(this.sectorTypes, ArrayList.class.getClassLoader());
		Bundle b = source.readBundle(SectorType.class.getClassLoader());
		this.sectorTypes = b.getParcelableArrayList("SectorTypes");
		this.defaultKey = source.readParcelable(SectorKey.class.getClassLoader());
	}
	
	/**
	 * {@link #setKeyMap(int, String, String)} is set sector Key A and key B at virtual PICC 
	 * for read write operation at reat PICC.
	 * 
	 * @return {@link SectorKey} Class that is set at virtual PICC
	 * 
	 * @param sectorNo is a int value [0..n]
	 * @param keyA is a String of KeyA
	 * @param keyB is a String of KeyB
	 * */
	public SectorKey setKeyMap(int sectorNo, String keyA, String keyB){
		
		return this.sectorKeyMap.put(new Integer(sectorNo),new SectorKey(keyA, keyB));
	}
	
	/**
	 * {@link #removeAllKeyMap()} this function clear all Sector => Key mapping  table.
	 * */
	public void removeAllKeyMap(){
		this.sectorKeyMap.clear();
	}
	
	public SectorKey getKey(int sectorNo){
		return this.sectorKeyMap.get(new Integer(sectorNo));
	}
	
	/**
	 * {@link #isEmptyKeyMap()} is a boolean function for Sector and Key mapping table.
	 * */
	public boolean isEmptyKeyMap(){
		return this.sectorKeyMap.isEmpty();
	}

	/*.......................................................................*/
	
	/**
	 * 
	 * This function should not use by the applicatin devoloper.
	 * */
	public PiccBlock getBlankBlock(int sectorNo, int sectorOnBlock){
		int blockNo;
		int bloclWidth;
		int blockCount;
		PiccBlock block = null;
		SectorType sectorType = getSectorTypeForSectorNo(sectorNo);
		if(sectorType != null){
			blockCount = sectorType.getBlockCountOnSector();
			bloclWidth = sectorType.getBlockWidthOnSector();
			int i = sectorNo - sectorType.getSectorStartIndex();
			
			blockNo = ((i*blockCount)+sectorOnBlock)+sectorType.getBlockStartIndex();
			block = new PiccBlock(blockNo, bloclWidth,bloclWidth,this.getSectorKey(sectorNo));
			
		}
		return block;
		
	}
	
	/**
	 * This function should not use by the applicatin devoloper.
	 * */
	public PiccSector getBlankSector(int sectorNo, boolean sectorTrailer){
		PiccSector sector = null;
		int startBlockIndex;
		int blockCount;
		int blockWidth;
		SectorType sectorType = getSectorTypeForSectorNo(sectorNo);
		if(sectorType != null){
			blockCount = sectorType.getBlockCountOnSector();	
			blockWidth = sectorType.getBlockWidthOnSector();
			int i = sectorNo - sectorType.getSectorStartIndex();
			
			startBlockIndex = ((i*blockCount)+0)+sectorType.getBlockStartIndex();
			
			blockCount = (sectorTrailer)? blockCount : blockCount -1;
			sector = new PiccSector(sectorNo, startBlockIndex, blockCount, blockWidth,this.getSectorKey(sectorNo));
		}
		return sector;
	}

	/**
	 * This function should not use by the applicatin devoloper.
	 * */
	public SectorType getSectorTypeForSectorNo(int sectorNo) {
		SectorType sectorType;
		for(int index=0; index < this.sectorTypes.size(); index++){
			sectorType = this.sectorTypes.get(index);
			if((sectorType.getSectorStartIndex() <= sectorNo) && (sectorNo <= sectorType.getSectorEndIndex())){
				return sectorType;
			}
		}
		return null;
	}
	/*...........................................................................................*/
	
	/**
	 * This function return the {@link SectorKey} of particular sector of virtual PICC.
	 * 
	 * @return {@link SectorKey}
	 * 
	 * @param sectorNo is a integer number. [0..n]
	 * 
	 * */
	public SectorKey getSectorKey(int sectorNo){
		SectorKey key = this.getKey(sectorNo);
		if(key == null){
			key = this.defaultKey;
		}
		return key;
	}

	/**
	 * @return the cardSize
	 */
	public int getCardSize() {
		return cardSize;
	}


	/**
	 * @return the defaultKey
	 */
	public SectorKey getDefaultKey() {
		return defaultKey;
	}


	/**
	 *  SectorType the sectorType to set
	 */
	public void setSectorTypes(int blockSizeOnSector, int blockCountOnSector, int sectorStartIndex,int sectorEndIndex, int blockStartIndex, int blockrEndIndex) {
		SectorType sectorType = new SectorType(blockSizeOnSector, blockCountOnSector, sectorStartIndex, sectorEndIndex, blockStartIndex, blockrEndIndex);
		this.sectorTypes.add(sectorType);
	}


	/**
	 * @return the sectorTypes ArrayList
	 */
	public ArrayList<SectorType> getSectorTypes() {
		return sectorTypes;
	}
	
	/**
	 * @return the SectorType
	 */
	public SectorType getSectorTypes(int index) {
		return this.sectorTypes.get(index);
	}


	/**
	 * @param sectorTypes the sectorTypes to set
	 */
	public void setSectorTypes(ArrayList<SectorType> sectorTypes) {
		this.sectorTypes = sectorTypes;
	}
	
	/**
	 * Tihis{@link #setSectorTypes(SectorType, int)}function set how many defarent sector is present in Virtual PICC,
	 * for read write operation in real PICC.
	 * 
	 *  @param sectorType is a definition of defarent SectorType in real PICC.
	 *  @param index the index numder of corrosponding sectorType, start from 0..n
	 */
	public void setSectorTypes(SectorType sectorType, int index) {
		this.sectorTypes.add(index, sectorType);
	}


	public int describeContents() {
		return 0;
	}


	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt(this.cardSize);
		dest.writeMap(this.sectorKeyMap);
		Bundle b = new Bundle();
		b.putParcelableArrayList("SectorTypes", this.sectorTypes);
		dest.writeBundle(b);
		//dest.writeList(this.sectorTypes);
		dest.writeParcelable(this.defaultKey, flags);	
		if(DEBUG)Log.v(TAG, "VirtualPicc writeToParcel\n");
	}
	
	public static final Parcelable.Creator<VirtualPicc> CREATOR = new Parcelable.Creator<VirtualPicc>(){

		public VirtualPicc createFromParcel(Parcel source) {
			if(DEBUG)Log.v(TAG, "VirtualPicc createFromParcel\n");
			return new VirtualPicc(source);
		}

		public VirtualPicc[] newArray(int size) {
			
			return new VirtualPicc[size];
		}
		
	};
	

}
