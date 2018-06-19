package android.rc632;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * {@link PiccBlock} is contain PICC block information. The block data is a public attribute.
 * As application programmer do not have much work on this class. Only Use when read block {@see Rc632Manager#readBlock(int, int, char)}.  
 * */

public class PiccBlock implements Parcelable {
	int blockNo;
	int blockWidth;
	int dataBuffLength;
	SectorKey key;
	
	public byte[] data;
	
	
	public PiccBlock(int blockNo, int blockWidth, int dataBuffLength, SectorKey key){
		this.blockNo = blockNo;
		this.blockWidth = blockWidth;
		this.key = key;
		this.dataBuffLength = dataBuffLength;
		this.data = new byte[dataBuffLength];
	}
	
	
	public PiccBlock(Parcel source) {
		this.blockNo = source.readInt();
		this.blockWidth = source.readInt();
		this.key = source.readParcelable(SectorKey.class.getClassLoader());
		this.dataBuffLength = source.readInt();
		this.data = new byte[dataBuffLength];
		source.readByteArray(this.data);
	}


	public void setData(byte[] data){
		for(int i=0; i<blockWidth; i++)
			this.data[i] = data[i];
	}


	/**
	 * @return the blockNo
	 */
	public int getBlockNo() {
		return blockNo;
	}


	/**
	 * @return the blockWidth
	 */
	public int getBlockWidth() {
		return blockWidth;
	}


	/**
	 * @return the key
	 */
	public SectorKey getKey() {
		return key;
	}


	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}


	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt(this.blockNo);
		dest.writeInt(this.blockWidth);
		dest.writeParcelable(this.key, flags);
		dest.writeInt(this.dataBuffLength);
		//if(this.data == null)this.data = new byte[dataBuffLength];
		dest.writeByteArray(this.data);
				
	}
	
	public static final Parcelable.Creator<PiccBlock> CREATOR = new Parcelable.Creator<PiccBlock>() {

		public PiccBlock createFromParcel(Parcel source) {
			
			return new PiccBlock(source);
		}

		public PiccBlock[] newArray(int size) {
			
			return new PiccBlock[size];
		}
		
	};

}
