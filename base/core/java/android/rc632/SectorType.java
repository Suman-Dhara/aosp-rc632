package android.rc632;

import android.os.Parcel;
import android.os.Parcelable;

public class SectorType implements Parcelable{
	private int blockSizeOnSector;
	private int blockCountOnSector;
	private int sectorStartIndex;
	private int sectorEndIndex;
	private int blockStartIndex;
	private int blockrEndIndex;
	
	public SectorType(int blockSizeOnSector, int blockCountOnSector, int sectorStartIndex,int sectorEndIndex, int blockStartIndex, int blockrEndIndex){
		this.blockSizeOnSector = blockSizeOnSector;
		this.blockCountOnSector = blockCountOnSector;
		this.sectorStartIndex = sectorStartIndex;
		this.sectorEndIndex = sectorEndIndex;
		this.blockStartIndex = blockStartIndex;
		this.blockrEndIndex = blockrEndIndex;
	}
	public SectorType(Parcel source){
		this.blockSizeOnSector = source.readInt();
		this.blockCountOnSector = source.readInt();
		this.sectorStartIndex = source.readInt();
		this.sectorEndIndex = source.readInt();
		this.blockStartIndex = source.readInt();
		this.blockrEndIndex = source.readInt();
	}
	/**
	 * @return the blockSizeOnSector
	 */
	public int getBlockWidthOnSector() {
		return blockSizeOnSector;
	}

	/**
	 * @return the blockCountOnSector
	 */
	public int getBlockCountOnSector() {
		return blockCountOnSector;
	}

	/**
	 * @return the sectorStartIndex
	 */
	public int getSectorStartIndex() {
		return sectorStartIndex;
	}

	/**
	 * @return the sectorEndIndex
	 */
	public int getSectorEndIndex() {
		return sectorEndIndex;
	}

	/**
	 * @return the blockrEndIndex
	 */
	public int getBlockEndIndex() {
		return blockrEndIndex;
	}

	/**
	 * @return the blockrStartIndex
	 */
	public int getBlockStartIndex() {
		return blockStartIndex;
	}

	@Override
	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt(this.blockSizeOnSector);
		dest.writeInt(this.blockCountOnSector);
		dest.writeInt(this.sectorStartIndex);
		dest.writeInt(this.sectorEndIndex);
		dest.writeInt(this.blockStartIndex);
		dest.writeInt(this.blockrEndIndex);
				
	}

	public static final Parcelable.Creator<SectorType> CREATOR = new Parcelable.Creator<SectorType>(){

		@Override
		public SectorType createFromParcel(Parcel source) {
			return new SectorType(source);
		}

		@Override
		public SectorType[] newArray(int size) {
			return new SectorType[size];
		}
		
	};

}
