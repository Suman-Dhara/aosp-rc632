/*
 * Copyright (C) 2018 The Android Open Source Project Written by SUMAN DHARA <dhara_suman@yahoo.in>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
package android.rc632;
 
import android.os.Bundle;
import android.rc632.IRc632ServiceReceiver;
import android.rc632.VirtualPicc;
import android.rc632.PiccBlock;
import android.rc632.PiccSector;

/**
 * Binder interface that clients running in application context
 * can use to interface with remote service
*/

interface IRc632Service{

	void setVirtualPicc(in VirtualPicc virtualPicc);
	
	PiccBlock writeBlock(in byte[] data, int sectorIndex, int blockIndexWithinSector, char keyType);
	PiccBlock readBlock(int sectorIndex, int blockIndexWithinSector, char keyType);
	PiccSector writeSector(in byte[] data, int sectorIndex, char keyType, boolean sectorTrailer);
	PiccSector readSector(int sectorIndex, char keyType, boolean sectorTrailer);
	
	void startListening(/*IBinder token, */ IRc632ServiceReceiver receiver);
	void stopListening(/*IBinder token*/);
	
	int scanOn();
	int scanOff();
	
}
