/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * SUMAN DHARA <dhara_suman@yahoo.in> FORTUNA IMPLEX PVT. LTD.
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

#define LOG_TAG "Rc632ServiceJNI"

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include <utils/misc.h>
#include <utils/Log.h>
#include <linux/types.h>
#include <hardware/hardware.h>
#include <hardware/rc632hw.h>

namespace android
{

static struct rc632hw_device_t *rc632hw_HAL;

/**
 * JNI Layer init function;
 * Load HAL and its method table for later use;
 * return a pointer to the same structure
 * (not mandatory btw, we can use a global variable)
 */
static jlong init_native(JNIEnv *env, jobject clazz) {
  int err;
  struct hw_module_t *module;
  struct hw_device_t *device;
  ALOGE("Entering %s\n", __func__);
  ALOGE("%d\n", __LINE__);
  /* Load the HAL module using libhardware hw_get_module */
  err = hw_get_module(RC632_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
  ALOGE("%d\n", __LINE__);
  if (err) {
	  ALOGE("%d\n", __LINE__);
  	  ALOGE("hw_get_module Error");

  }
  ALOGE("%d\n", __LINE__);

  /* Open the HAL object and get the rc632hw_device_t structure
   * filled with methods pointers
   */

  err = module->methods->open(module, "rc632JNI", &device);
  if (err) {
	  ALOGE("%d\n", __LINE__);
  	  ALOGE("module->methods->open() Error");
  }
  ALOGE("%d\n", __LINE__);

  rc632hw_HAL =  (struct rc632hw_device_t *)device;
  ALOGE("%d\n", __LINE__);
  if(err) return -err;
  return (jlong)rc632hw_HAL;
}

/**
 * rc632hw wrapper function;
 * Call the native rc632hw function from Framework layer
 */

static int rc632hw_block_read_native(	JNIEnv *env,       /* interface pointer */
										jobject obj,       /* "this" pointer	*/
										jbyteArray buffer, jint start_block_no, jint block_count, jbyteArray read_key, jbyte key_type) {
	int err;
	jbyte* real_byte_array;
	jbyte* key_real_byte_array;
	int i;
	jsize len,key_len;

	picckey_t picc_key;
	ALOGD("Entering %s\n", __func__);
	ALOGD("%d\n", __LINE__);

	len = env->GetArrayLength(buffer);
	key_len = env->GetArrayLength(read_key);
	picc_key.picc_key_size = (__u16)len;
	picc_key.picc_key_type = (__u16)key_type;
	key_real_byte_array    = env->GetByteArrayElements(read_key,0);
	for(i=0;i<key_len;i++){
		picc_key.picc_key[i] = key_real_byte_array[i];
		ALOGD("read_native Key %X\n", picc_key.picc_key[i]);
	}

	real_byte_array = env->GetByteArrayElements(buffer,0);
	if(rc632hw_HAL != NULL)
	err = rc632hw_HAL->rc632hw_block_read((char*)real_byte_array,(__u16)start_block_no,(__u16)block_count,picc_key);
	if (err) {
		ALOGE("Error rc632hw_HAL %d\n", __LINE__);
		//TODO
	}

	env->ReleaseByteArrayElements(buffer, real_byte_array, 0);
	env->ReleaseByteArrayElements(read_key, key_real_byte_array, 0);

	return err;

}


static int rc632hw_block_write_native(
							JNIEnv *env,       /* interface pointer */
							jobject obj,       /* "this" pointer	*/
							jbyteArray buffer, jint start_block_no, jint block_count, jbyteArray key, jbyte key_type){
	int err;
	jbyte* real_byte_array;
	jbyte* key_real_byte_array;
	int i;
	jsize len;

	picckey_t picc_key;
	ALOGE("Entering %s\n", __func__);
	ALOGE("%d\n", __LINE__);

	len = env->GetArrayLength(buffer);
	picc_key.picc_key_size = (__u16)len;
	picc_key.picc_key_type = (__u16)key_type;
	key_real_byte_array    = env->GetByteArrayElements(key, 0);
	for(i=0;i<len;i++){
		picc_key.picc_key[i] = key_real_byte_array[i];
	}

	real_byte_array = env->GetByteArrayElements(buffer, 0);
	err = rc632hw_HAL->rc632hw_block_write((char*)real_byte_array,(__u16)start_block_no,(__u16)block_count,picc_key);
	if (err) {
		ALOGE("%d\n", __LINE__);
	    //TODO

	}

	env->ReleaseByteArrayElements(buffer, real_byte_array, 0);
	env->ReleaseByteArrayElements(key, key_real_byte_array, 0);

	return err;
}

static int rc632hw_scan_on_native(
							JNIEnv *env,       /* interface pointer */
							jobject obj,       /* "this" pointer	*/
							jint poll_interval, jint poll_interval_min, jint poll_interval_max){
	int err;
	ALOGE("Entering %s\n", __func__);
	ALOGE("%d\n", __LINE__);
	ALOGD("poll_interval %d\n", poll_interval);
	ALOGD("poll_interval_min %d\n", poll_interval_min);
	ALOGD("poll_interval_max %d\n", poll_interval_max);
	if(rc632hw_HAL != NULL)
	err = rc632hw_HAL->rc632hw_scan_on((__u32)poll_interval,(__u32)poll_interval_min,(__u32)poll_interval_max);
	if (err) {
		ALOGE("%d\n", __LINE__);
	    //TODO

	}


	return err;
}

static int rc632hw_scan_off_native( JNIEnv *env,       /* interface pointer */
				    jobject obj       /* "this" pointer	*/
												){
	int err;
	ALOGE("Entering %s\n", __func__);
	ALOGE("%d\n", __LINE__);

	err = rc632hw_HAL->rc632hw_scan_off();
	if (err) {
		ALOGE("%d\n", __LINE__);
	    //TODO

	}

	return err;
}

static int rc632hw_readEvent_native(	JNIEnv *env,       /* interface pointer */
					jobject obj       /* "this" pointer	*/){

	ALOGD("%d- rc632hw_readEvent_native \n", __LINE__);
	return rc632hw_HAL->rc632hw_readEvent();

}



static JNINativeMethod method_table[] = {
    { "initRc632_native", "()J", (void*)init_native },
    { "blockRead_native", "([BII[BB)I", (void*)rc632hw_block_read_native },
    { "blockWrite_native", "([BII[BB)I", (void*)rc632hw_block_write_native },
    { "scanOn_native", "(III)I", (void*)rc632hw_scan_on_native },
    { "scanOff_native", "()I", (void*)rc632hw_scan_off_native },
    { "readEvent_native", "()I", (void*)rc632hw_readEvent_native }


};



int register_android_server_rc632_Rc632Service(JNIEnv *env)
{
/*	jclass clazz = env->FindClass("com/android/server/rc632/Rc632Service");
	    if (clazz == NULL) {
		ALOGE("Can't find com/android/server/rc632/Rc632Service");
		return -1;
	    }
*/
	    return jniRegisterNativeMethods(env, "com/android/server/rc632/Rc632Service",
            method_table, NELEM(method_table));
}

};
