package android.rc632;

import android.os.Parcel;
import android.os.Parcelable;

public class SectorKey implements Parcelable {
	
	public SectorKey(String keyA, String keyB) {
		this.keyA = keyA;
		this.keyB = keyB;
	}
	public SectorKey(Parcel source) {
		this.keyA = source.readString();
		this.keyB = source.readString();
	}
	private String keyA;
	private String keyB;
	/**
	 * @return the keyA
	 */
	public String getKeyA() {
		return keyA;
	}
	/**
	 * @return the keyB
	 */
	public String getKeyB() {
		return keyB;
	}
	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(this.keyA);
		dest.writeString(this.keyB);		
	}
	
	public static final Parcelable.Creator<SectorKey> CREATOR = new Parcelable.Creator<SectorKey>(){

		public SectorKey createFromParcel(Parcel source) {
			return new SectorKey(source);
		}

		public SectorKey[] newArray(int size) {
			return new SectorKey[size];
		}
		
	};
	
}
