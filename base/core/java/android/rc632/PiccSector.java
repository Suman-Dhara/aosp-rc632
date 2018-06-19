package android.rc632;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * {@link PiccSector } is contain one sector information. IT is derive from {@link PiccBlock} class.
 * As application programmer do not have much work on this class. Only Use when read sector {@see Rc632Manager#readSector(int, char, boolean)}.
 * */

public class PiccSector extends PiccBlock{
	
	protected int secotrNo;
	protected int blockCount;
	
	public PiccSector(int sectrNo, int startBlockIndex ,int blockCount, int blockWidth, SectorKey key){
		super(startBlockIndex, blockWidth, (blockCount*blockWidth),key);
		this.secotrNo = sectrNo;
		this.blockCount = blockCount;
	}

	public PiccSector(Parcel source) {
		super(source);
		this.secotrNo = source.readInt();
		this.blockCount = source.readInt();
	}
	
	/**
	 * @return the secotrNo
	 */
	public int getSecotrNo() {
		return secotrNo;
	}

	/**
	 * @return the blockCount
	 */
	public int getBlockCount() {
		return blockCount;
	}

	/* (non-Javadoc)
	 * @see android.service.rc632.PiccBlock#describeContents()
	 */
	@Override
	public int describeContents() {
		// TODO Auto-generated method stub
		return super.describeContents();
	}

	/* (non-Javadoc)
	 * @see android.service.rc632.PiccBlock#writeToParcel(android.os.Parcel, int)
	 */
	@Override
	public void writeToParcel(Parcel dest, int flags) {
		
		super.writeToParcel(dest, flags);
		dest.writeInt(this.secotrNo);
		dest.writeInt(this.blockCount);
	}
	
	public static final Parcelable.Creator<PiccSector> CREATOR = new Parcelable.Creator<PiccSector>(){

		public PiccSector createFromParcel(Parcel source) {
			
			return new PiccSector(source);
		}

		public PiccSector[] newArray(int size) {
			// TODO Auto-generated method stub
			return new PiccSector[size];
		}
		
	};
	
}
