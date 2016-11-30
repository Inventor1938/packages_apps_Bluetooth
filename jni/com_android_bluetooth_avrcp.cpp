/*
 * Copyright (C) 2013-2015, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "BluetoothAvrcpServiceJni"

//#define LOG_NDEBUG 0

#include "com_android_bluetooth.h"
#include "hardware/bt_rc.h"
#include "utils/Log.h"
#include "android_runtime/AndroidRuntime.h"

#include <string.h>

namespace android {
static jmethodID method_getRcFeatures;
static jmethodID method_getPlayStatus;
static jmethodID method_onListPlayerAttributeRequest;
static jmethodID method_getElementAttr;
static jmethodID method_registerNotification;
static jmethodID method_volumeChangeCallback;
static jmethodID method_handlePassthroughCmd;
static jmethodID method_handlePassthroughRsp;
static jmethodID method_getFolderItems;
static jmethodID method_setAddressedPlayer;
static jmethodID method_setBrowsedPlayer;
static jmethodID method_changePath;
static jmethodID method_playItem;
static jmethodID method_getItemAttr;
static jmethodID method_onListPlayerAttributeValues;
static jmethodID method_onGetPlayerAttributeValues;
static jmethodID method_setPlayerAppSetting;
static jmethodID method_getplayerattribute_text;
static jmethodID method_getplayervalue_text;
static jmethodID method_onConnectionStateChanged;
static jmethodID method_getTotalNumberOfItems;

static const btrc_interface_t *sBluetoothMultiAvrcpInterface = NULL;
static jobject mCallbacksObj = NULL;
static JNIEnv *sCallbackEnv = NULL;

static bool checkCallbackThread() {
    // Always fetch the latest callbackEnv from AdapterService.
    // Caching this could cause this sCallbackEnv to go out-of-sync
    // with the AdapterService's ENV if an ASSOCIATE/DISASSOCIATE event
    // is received
    sCallbackEnv = getCallbackEnv();

    JNIEnv* env = AndroidRuntime::getJNIEnv();
    if (sCallbackEnv != env || sCallbackEnv == NULL) return false;
    return true;
}

static void btavrcp_remote_features_callback(bt_bdaddr_t* bd_addr,
        btrc_remote_features_t features) {

    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for remote features");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }

    if (mCallbacksObj) {
        sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getRcFeatures, addr,
                                                         (jint)features, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_get_play_status_callback(bt_bdaddr_t* bd_addr) {
    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for get play status");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);

    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getPlayStatus, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_get_element_attr_callback(uint8_t num_attr, btrc_media_attr_t *p_attrs,
        bt_bdaddr_t *bd_addr) {
    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for get_element_attr command");
        return;
    }

    jintArray attrs = (jintArray)sCallbackEnv->NewIntArray(num_attr);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        sCallbackEnv->DeleteLocalRef(addr);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);

    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj ,method_onListPlayerAttributeValues,
                                    (jbyte)player_att, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_register_notification_callback(btrc_event_id_t event_id, uint32_t param,
    bt_bdaddr_t *bd_addr) {

    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for player attribute");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);

    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj,method_onListPlayerAttributeRequest, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_volume_change_callback(uint8_t volume, uint8_t ctype,
    bt_bdaddr_t *bd_addr) {

    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for player app setting");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    attrs = (jintArray)sCallbackEnv->NewIntArray(num_attr);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetIntArrayRegion(attrs, 0, num_attr, (jint *)p_attrs);

    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj,method_onGetPlayerAttributeValues,
                                     (jbyte)num_attr,attrs, addr);
    }
    else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(attrs);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_set_playerapp_setting_value_callback(btrc_player_settings_t *attr,
                                                         bt_bdaddr_t* bd_addr)
{
    jbyteArray attrs_ids;
    jbyteArray attrs_value;
    ALOGV("%s", __FUNCTION__);
    jbyteArray addr;

    if (!checkCallbackThread()) {
        ALOGE("Callback: '%s' is not called on the correct thread", __FUNCTION__);
        return;
    }
    addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for set playerapp");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    attrs_ids   = (jbyteArray)sCallbackEnv->NewByteArray(attr->num_attr);
    if (!attrs_ids) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(attrs_ids, 0, attr->num_attr, (jbyte *)attr->attr_ids);
    attrs_value = (jbyteArray)sCallbackEnv->NewByteArray(attr->num_attr);
    if (!attrs_value) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(attrs_value, 0, attr->num_attr, (jbyte *)attr->attr_values);
    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_setPlayerAppSetting,
                            (jbyte)attr->num_attr ,attrs_ids ,attrs_value, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    sCallbackEnv->DeleteLocalRef(attrs_ids);
    sCallbackEnv->DeleteLocalRef(attrs_value);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_set_addressed_player_callback(uint16_t player_id,
    bt_bdaddr_t *bd_addr) {

    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for getPlayer app");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    attrs   = (jbyteArray)sCallbackEnv->NewByteArray(num);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(attrs, 0, num, (jbyte *)att);
    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getplayerattribute_text,
                                     (jbyte) num ,attrs, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    sCallbackEnv->DeleteLocalRef(attrs);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_set_browsed_player_callback(uint16_t player_id, bt_bdaddr_t *bd_addr) {
    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for getPlayer app");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    Attr_Value   = (jbyteArray)sCallbackEnv->NewByteArray(num_val);
    if (!Attr_Value) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(Attr_Value, 0, num_val, (jbyte *)value);
    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getplayervalue_text,(jbyte) attr_id,
                                     (jbyte) num_val , Attr_Value, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    sCallbackEnv->DeleteLocalRef(Attr_Value);
}

static void btavrcp_get_folder_items_callback(uint8_t scope, uint32_t start_item,
            uint32_t end_item,uint8_t num_attr, uint32_t *p_attr_ids, bt_bdaddr_t *bd_addr) {
    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for element attr");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }

    uint32_t *puiAttr = (uint32_t *)p_attr_ids;
    jintArray attr_ids = NULL;
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);

    /* check number of attributes requested by remote device */
    if ((num_attr != BTRC_NUM_ATTR_ALL) && (num_attr != BTRC_NUM_ATTR_NONE)) {
        /* allocate memory for attr_ids only if some attributes passed from below layer */
        attr_ids = (jintArray)sCallbackEnv->NewIntArray(num_attr);
        if (!attr_ids) {
            ALOGE("Fail to allocate new jintArray for attrs");
            sCallbackEnv->DeleteLocalRef(addr);
            return;
        }
        sCallbackEnv->SetIntArrayRegion(attr_ids, 0, num_attr, (jint *)puiAttr);
    }

    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getFolderItemsCallback, addr,
            (jbyte) scope, (jint) start_item, (jint) end_item, (jbyte) num_attr, attr_ids);

    if (attr_ids != NULL) sCallbackEnv->DeleteLocalRef(attr_ids);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_change_path_callback(uint8_t direction, uint8_t* folder_uid,
    bt_bdaddr_t *bd_addr) {
    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray attrs = sCallbackEnv->NewByteArray(BTRC_UID_SIZE);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for volume change");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->SetByteArrayRegion(
            attrs, 0, sizeof(uint8_t)*BTRC_UID_SIZE, (jbyte *)folder_uid);
    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_changePathCallback, addr,
         (jbyte) direction, attrs);

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_get_item_attr_callback( uint8_t scope, uint8_t* uid, uint16_t uid_counter,
    uint8_t num_attr, btrc_media_attr_t *p_attrs, bt_bdaddr_t *bd_addr) {

    ALOGI("%s", __func__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    if (mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray attr_uid = sCallbackEnv->NewByteArray(BTRC_UID_SIZE);
    if (!attr_uid) {
        ALOGE("Fail to new jintArray for attr_uid");
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for get folder items");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    if (num_attr == 0xff) {
        num_attr = 0; // 0xff signifies no attribute required in response
    } else if (num_attr == 0) {
        num_attr = 8; // 0x00 signifies all attributes required in response
    }

    jintArray attrs = (jintArray)sCallbackEnv->NewIntArray(num_attr);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetIntArrayRegion(attrs, 0, num_attr, (jint *)param->attrs);

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->SetIntArrayRegion(attrs, 0, num_attr, (jint *)p_attrs);
    sCallbackEnv->SetByteArrayRegion(attr_uid, 0, sizeof(uint8_t)*BTRC_UID_SIZE, (jbyte *)uid);

    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_getItemAttrCallback, addr,
        (jbyte) scope, attr_uid, (jint) uid_counter, (jbyte)num_attr, attrs);

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(attrs);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_play_item_callback(uint8_t scope, uint16_t uid_counter, uint8_t* uid,
    bt_bdaddr_t *bd_addr) {
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray attrs = sCallbackEnv->NewByteArray(BTRC_UID_SIZE);
    if (!attrs) {
        ALOGE("%s:Fail to new jByteArray attrs for play_item command", __func__);
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for set addressed player");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->SetByteArrayRegion(attrs, 0, sizeof(uint8_t)*BTRC_UID_SIZE, (jbyte *)uid);
    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_playItemCallback, addr,
                (jbyte) scope, (jint) uid_counter, attrs);

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_get_total_num_items_callback(uint8_t scope, bt_bdaddr_t *bd_addr) {
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for change path");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    if (mCallbacksObj) {
        sCallbackEnv->CallVoidMethod(mCallbacksObj, method_changePath, (jbyte)direction,
                                     (jlong)uid, addr);
    } else {
        ALOGE("%s: mCallbacksObj is null", __FUNCTION__);
    }
    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_play_item_callback(uint8_t scope, uint64_t uid, bt_bdaddr_t* bd_addr) {
    jbyteArray addr;
    ALOGV("%s", __FUNCTION__);
    ALOGV("scope: %d, uid: %lu", scope, uid);

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->CallVoidMethod(
            mCallbacksObj, method_getTotalNumOfItemsCallback, addr, (jbyte) scope);

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_search_callback(uint16_t charset_id, uint16_t str_len, uint8_t* p_str,
    bt_bdaddr_t *bd_addr) {
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;
    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray attrs = sCallbackEnv->NewByteArray(str_len);
    if (!attrs) {
        ALOGE("Fail to new jintArray for attrs");
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for get item attr");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->SetByteArrayRegion(
            attrs, 0, str_len*sizeof(uint8_t), (jbyte *)p_str);
    sCallbackEnv->CallVoidMethod(
            mCallbacksObj, method_searchCallback, addr, (jint) charset_id, attrs);

    checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
    sCallbackEnv->DeleteLocalRef(attrs);
    sCallbackEnv->DeleteLocalRef(addr);
}

static void btavrcp_add_to_play_list_callback(uint8_t scope,
                       uint8_t* uid, uint16_t uid_counter, bt_bdaddr_t *bd_addr) {

    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;
    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(bt_bdaddr_t));
    if (!addr) {
        ALOGE("Fail to new jbyteArray bd addr for connection state");
        checkAndClearExceptionFromCallback(sCallbackEnv, __FUNCTION__);
        return;
    }

    jbyteArray attrs = sCallbackEnv->NewByteArray(BTRC_UID_SIZE);
    if (!attrs) {
        ALOGE("Fail to new jByteArray for attrs");
        sCallbackEnv->DeleteLocalRef(addr);
        return;
    }

    sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(bt_bdaddr_t), (jbyte*) bd_addr);
    sCallbackEnv->SetByteArrayRegion(attrs, 0, sizeof(uint8_t)*BTRC_UID_SIZE, (jbyte *)uid);
    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_addToPlayListCallback, addr,
                 (jbyte) scope, attrs, (jint) uid_counter);

    sCallbackEnv->DeleteLocalRef(attrs);
    sCallbackEnv->DeleteLocalRef(addr);
}

static btrc_callbacks_t sBluetoothAvrcpCallbacks = {
    sizeof(sBluetoothAvrcpCallbacks),
    btavrcp_remote_features_callback,
    btavrcp_get_play_status_callback,
    btavrcp_get_player_attribute_id_callback,
    btavrcp_get_player_seeting_value_callback,
    btavrcp_getcurrent_player_app_setting_values,
    btavrcp_getPlayer_app_attribute_text,
    btavrcp_getPlayer_app_value_text,
    btavrcp_set_playerapp_setting_value_callback,
    btavrcp_get_element_attr_callback,
    btavrcp_register_notification_callback,
    btavrcp_volume_change_callback,
    btavrcp_passthrough_command_callback,
    btavrcp_get_folder_items_callback,
    btavrcp_set_addressed_player_callback,
    btavrcp_set_browsed_player_callback,
    btavrcp_change_path_callback,
    btavrcp_play_item_callback,
    btavrcp_get_item_attr_callback,
    btavrcp_connection_state_callback,
    btavrcp_get_total_items_callback
};

static void classInitNative(JNIEnv* env, jclass clazz) {
    method_getRcFeatures =
        env->GetMethodID(clazz, "getRcFeatures", "([BI)V");
    method_getPlayStatus =
        env->GetMethodID(clazz, "getPlayStatus", "([B)V");
    method_onListPlayerAttributeRequest =
        env->GetMethodID(clazz , "onListPlayerAttributeRequest" , "([B)V");
    method_onListPlayerAttributeValues =
        env->GetMethodID(clazz , "onListPlayerAttributeValues" , "(B[B)V");
    method_getElementAttr =
        env->GetMethodID(clazz, "getElementAttr", "(B[I[B)V");
    method_setPlayerAppSetting =
        env->GetMethodID(clazz, "setPlayerAppSetting","(B[B[B[B)V");
    method_getplayerattribute_text =
        env->GetMethodID(clazz, "getplayerattribute_text" , "(B[B[B)V");
    method_getplayervalue_text =
        env->GetMethodID(clazz, "getplayervalue_text" , "(BB[B[B)V");
    method_registerNotification =
        env->GetMethodID(clazz, "registerNotification", "(II[B)V");
    method_onGetPlayerAttributeValues =
        env->GetMethodID(clazz, "onGetPlayerAttributeValues", "(B[I[B)V");
    method_volumeChangeCallback =
        env->GetMethodID(clazz, "volumeChangeCallback", "(II[B)V");
    method_handlePassthroughCmd =
        env->GetMethodID(clazz, "handlePassthroughCmd", "(II[B)V");
    //setAddressedPlayer: attributes to pass: Player ID
    method_setAddressedPlayer =
        env->GetMethodID(clazz, "setAddressedPlayer", "(I[B)V");
    //getFolderItems: attributes to pass: Scope, Start, End, Attr Cnt
    method_getFolderItems =
        env->GetMethodID(clazz, "getFolderItems", "(BJJII[I[B)V");
    method_setBrowsedPlayer =
        env->GetMethodID(clazz, "setBrowsedPlayer", "(I[B)V");
    method_changePath =
        env->GetMethodID(clazz, "changePath", "(BJ[B)V");
    method_playItem =
        env->GetMethodID(clazz, "playItem", "(BJ[B)V");
    method_getItemAttr =
        env->GetMethodID(clazz, "getItemAttr", "(BJB[II[B)V");
    method_onConnectionStateChanged =
        env->GetMethodID(clazz, "onConnectionStateChanged", "(Z[B)V");
    method_getTotalNumberOfItems =
        env->GetMethodID(clazz, "getTotalNumberOfItems", "(B[B)V");
    ALOGV("%s: succeeds", __FUNCTION__);
}

static void initNative(JNIEnv *env, jobject object) {
    const bt_interface_t* btInf = getBluetoothInterface();
    if (btInf == NULL) {
        ALOGE("Bluetooth module is not loaded");
        return;
    }

    if (sBluetoothMultiAvrcpInterface !=NULL) {
         ALOGW("Cleaning up Avrcp Interface before initializing...");
         sBluetoothMultiAvrcpInterface->cleanup();
         sBluetoothMultiAvrcpInterface = NULL;
    }

    if (mCallbacksObj != NULL) {
         ALOGW("Cleaning up Avrcp callback object");
         env->DeleteGlobalRef(mCallbacksObj);
         mCallbacksObj = NULL;
    }

    sBluetoothAvrcpInterface = (btrc_interface_t *) btInf->get_profile_interface(BT_PROFILE_AV_RC_ID);
    if (sBluetoothAvrcpInterface == NULL) {
        ALOGE("Failed to get Bluetooth Avrcp Interface");
        return;
    }

    bt_status_t status = sBluetoothAvrcpInterface->init(&sBluetoothAvrcpCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed to initialize Bluetooth Avrcp, status: %d", status);
        sBluetoothMultiAvrcpInterface = NULL;
        return;
    }

    mCallbacksObj = env->NewGlobalRef(object);
}

static void cleanupNative(JNIEnv *env, jobject object) {
    const bt_interface_t* btInf = getBluetoothInterface();
    if (btInf == NULL) {
        ALOGE("Bluetooth module is not loaded");
        return;
    }

    if (sBluetoothMultiAvrcpInterface !=NULL) {
        sBluetoothMultiAvrcpInterface->cleanup();
        sBluetoothMultiAvrcpInterface = NULL;
    }

    if (mCallbacksObj != NULL) {
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }
}

static jboolean getPlayStatusRspNative(JNIEnv *env, jobject object,
        jbyteArray address, jint playStatus, jint songLen, jint songPos) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    bt_status_t status = sBluetoothAvrcpInterface->get_play_status_rsp((bt_bdaddr_t*)addr,
        (btrc_play_status_t)playStatus, songLen, songPos);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_play_status_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean getElementAttrRspNative(JNIEnv *env, jobject object,
        jbyteArray address, jbyte numAttr, jintArray attrIds,
        jobjectArray textArray) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    if( numAttr > BTRC_MAX_APP_ATTR_SIZE) {
        ALOGE("get_element_attr_rsp: number of attributes exceed maximum");
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE ;
    }
    for (i = 0; i < numAttr; ++i) {
        pAttrs[i] = (btrc_player_attr_t)attr[i];
    }
    if (i < numAttr) {
        delete[] pAttrs;
        env->ReleaseByteArrayElements(attrIds, attr, 0);
        return JNI_FALSE;
    }
    //Call Stack Method
    if ((status = sBluetoothMultiAvrcpInterface->list_player_app_attr_rsp(numAttr, pAttrs,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed list_player_app_attr_rsp, status: %d", status);
    }
    delete[] pAttrs;
    env->ReleaseByteArrayElements(attrIds, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

    btrc_element_attr_val_t *pAttrs = new btrc_element_attr_val_t[numAttr];
    if (!pAttrs) {
        ALOGE("getPlayerAppValueRspNative: not have enough memeory");
        return JNI_FALSE;
    }
    attr = env->GetByteArrayElements(value, NULL);

    addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        delete[] pAttrs;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    jint *attr = env->GetIntArrayElements(attrIds, NULL);
    if (!attr) {
        delete[] pAttrs;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    for (i = 0; i < numvalue; ++i) {
        pAttrs[i] = (uint8_t)attr[i];
    }
    if (i < numvalue) {
        delete[] pAttrs;
        env->ReleaseByteArrayElements(value, attr, 0);
        return JNI_FALSE;
    }
    if ((status = sBluetoothMultiAvrcpInterface->list_player_app_value_rsp(numvalue, pAttrs,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed list_player_app_value_rsp, status: %d", status);
    }
    delete[] pAttrs;
    env->ReleaseByteArrayElements(value, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

    int attr_cnt;
    for (attr_cnt = 0; attr_cnt < numAttr; ++attr_cnt) {
        pAttrs[attr_cnt].attr_id = attr[attr_cnt];
        jstring text = (jstring) env->GetObjectArrayElement(textArray, attr_cnt);

        if (!copy_jstring(pAttrs[attr_cnt].text, BTRC_MAX_ATTR_STR_LEN, text, env)) {
            env->DeleteLocalRef(text);
            break;
        }

    addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    if (attr_cnt < numAttr) {
        delete[] pAttrs;
        env->ReleaseIntArrayElements(attrIds, attr, 0);
        ALOGE("%s: Failed to copy attributes", __func__);
        return JNI_FALSE;
    }
    attr = env->GetByteArrayElements(value, NULL);
    if (!attr) {
        delete pAttrs;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    pAttrs->num_attr = numattr/2 ;
    for(i =0 ; i < numattr; i+=2)
    {
        pAttrs->attr_ids[i/2]    =  attr[i];
        pAttrs->attr_values[i/2] =  attr[i+1];
    }
    if ((status = sBluetoothMultiAvrcpInterface->get_player_app_value_rsp(pAttrs,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_player_app_value_rsp, status: %d", status);
    }
    delete pAttrs;
    env->ReleaseByteArrayElements(value, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->get_element_attr_rsp(btAddr, numAttr, pAttrs);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_element_attr_rsp, status: %d", status);
    }
    if ((status = sBluetoothMultiAvrcpInterface->set_player_app_value_rsp(player_rsp,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed set_player_app_value_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}


//JNI Method Called to Respond to PDU 0x15
static jboolean sendSettingsTextRspNative(JNIEnv *env, jobject object, jint num_attr,
                                jbyteArray attr,jint length , jobjectArray textArray,
                                jbyteArray address) {
    btrc_player_setting_text_t *pAttrs = NULL;
    bt_status_t status;
    jbyte *addr;
    int i;
    jstring text;
    const char* textStr;
    jbyte *arr ;

    if (!sBluetoothMultiAvrcpInterface) return JNI_FALSE;

    addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    if (num_attr > BTRC_MAX_ELEM_ATTR_SIZE) {
        ALOGE("get_element_attr_rsp: number of attributes exceed maximum");
        return JNI_FALSE;
    }
    pAttrs = new btrc_player_setting_text_t[num_attr];
    if (!pAttrs) {
        ALOGE("sendSettingsTextRspNative: not have enough memeory");
        return JNI_FALSE;
    }
    arr = env->GetByteArrayElements(attr, NULL);
    if (!arr) {
        delete[] pAttrs;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    for (i = 0; i < num_attr ; ++i) {
        text = (jstring) env->GetObjectArrayElement(textArray, i);
        textStr = env->GetStringUTFChars(text, NULL);
        if (!textStr) {
            ALOGE("get_element_attr_rsp: GetStringUTFChars return NULL");
            env->DeleteLocalRef(text);
            break;
        }
        pAttrs[i].id = arr[i];
        if (strlen(textStr) >= BTRC_MAX_ATTR_STR_LEN) {
            ALOGE("sendSettingsTextRspNative: string length exceed maximum");
        }
        strlcpy((char *)pAttrs[i].text, textStr, BTRC_MAX_ATTR_STR_LEN);
        //Check out if release need to be done in for loop
        env->ReleaseStringUTFChars(text, textStr);
        env->DeleteLocalRef(text);
    }
    //Call Stack Methos to Respond PDU 0x16
    if ((status = sBluetoothMultiAvrcpInterface->get_player_app_attr_text_rsp(num_attr, pAttrs,
                                            (bt_bdaddr_t *)addr)) !=  BT_STATUS_SUCCESS) {
        ALOGE("Failed get_player_app_attr_text_rsp, status: %d", status);
    }
    delete[] pAttrs;
    env->ReleaseByteArrayElements(attr, arr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean getItemAttrRspNative(JNIEnv *env, jobject object, jbyteArray address,
                        jint rspStatus, jbyte numAttr, jintArray attrIds, jobjectArray textArray) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }
    arr = env->GetByteArrayElements(attr, NULL);
    if (!arr) {
        delete[] pAttrs;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    for (i = 0; i < num_attr ; ++i) {
        text = (jstring) env->GetObjectArrayElement(textArray, i);
        textStr = env->GetStringUTFChars(text, NULL);
        if (!textStr) {
            ALOGE("sendValueTextRspNative: GetStringUTFChars return NULL");
            env->DeleteLocalRef(text);
            break;
        }
        pAttrs[i].id = arr[i];
        if (strlen(textStr) >= BTRC_MAX_ATTR_STR_LEN) {
           ALOGE("sendValueTextRspNative: string length exceed maximum");
        }
        strlcpy((char *)pAttrs[i].text, textStr, BTRC_MAX_ATTR_STR_LEN);
        env->ReleaseStringUTFChars(text, textStr);
        env->DeleteLocalRef(text);
    }
    //Call Stack Method to Respond to PDU 0x16
    if ((status = sBluetoothMultiAvrcpInterface->get_player_app_value_text_rsp(num_attr, pAttrs,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_player_app_value_text_rsp, status: %d", status);
    }
    delete[] pAttrs;
    env->ReleaseByteArrayElements(attr, arr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

  static jboolean getElementAttrRspNative(JNIEnv *env, jobject object, jbyte numAttr,
                                          jintArray attrIds, jobjectArray textArray,
                                          jbyteArray address) {
    jint *attr;
    bt_status_t status;
    jbyte *addr;
    jstring text;
    int i;
    btrc_element_attr_val_t *pAttrs = NULL;
    const char* textStr;

    if (!sBluetoothMultiAvrcpInterface) return JNI_FALSE;

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    if (numAttr > BTRC_MAX_ELEM_ATTR_SIZE) {
        ALOGE("get_element_attr_rsp: number of attributes exceed maximum");
        return JNI_FALSE;
    }

    btrc_element_attr_val_t *pAttrs = new btrc_element_attr_val_t[numAttr];
    if (!pAttrs) {
        ALOGE("get_element_attr_rsp: not have enough memeory");
        return JNI_FALSE;
    }

    jint *attr = NULL;
    if (attrIds != NULL) {
        attr = env->GetIntArrayElements(attrIds, NULL);
        if (!attr) {
            delete[] pAttrs;
            jniThrowIOException(env, EINVAL);
            env->ReleaseByteArrayElements(address, addr, 0);
            return JNI_FALSE;
        }
    }

    for (int attr_cnt = 0; attr_cnt < numAttr; ++attr_cnt) {
        pAttrs[attr_cnt].attr_id = attr[attr_cnt];
        jstring text = (jstring) env->GetObjectArrayElement(textArray, attr_cnt);

        if (!copy_jstring(pAttrs[attr_cnt].text, BTRC_MAX_ATTR_STR_LEN, text, env)) {
            rspStatus = BTRC_STS_INTERNAL_ERR;
            env->DeleteLocalRef(text);
            break;
        }

        pAttrs[i].attr_id = attr[i];
        if (strlen(textStr) >= BTRC_MAX_ATTR_STR_LEN) {
            ALOGE("get_element_attr_rsp: string length exceed maximum");
        }
        strlcpy((char *)pAttrs[i].text, textStr, BTRC_MAX_ATTR_STR_LEN);
        env->ReleaseStringUTFChars(text, textStr);
        env->DeleteLocalRef(text);
    }

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->get_item_attr_rsp(btAddr,
            (btrc_status_t)rspStatus, numAttr, pAttrs);
    if (status != BT_STATUS_SUCCESS)
        ALOGE("Failed get_item_attr_rsp, status: %d", status);

    if ((status = sBluetoothMultiAvrcpInterface->get_element_attr_rsp(numAttr, pAttrs,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_element_attr_rsp, status: %d", status);
    }

    delete[] pAttrs;
    env->ReleaseIntArrayElements(attrIds, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationPlayerAppRspNative(JNIEnv *env, jobject object ,jint type,
                                                jbyte numattr ,jbyteArray value ,
                                                jbyteArray address) {
    bt_status_t status;
    jbyte *addr;
    int i;
    jbyte *attr;
    btrc_register_notification_t *param= NULL;

    if (!sBluetoothMultiAvrcpInterface) return JNI_FALSE;

    addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    if( numattr > BTRC_MAX_APP_ATTR_SIZE || numattr == 0) {
        ALOGE("registerNotificationPlayerAppRspNative: number of attributes exceed maximum");
        return JNI_FALSE;
    }
    param = new btrc_register_notification_t;

    if (!param) {
        ALOGE("registerNotificationPlayerAppRspNative: not have enough memeory");
        return JNI_FALSE;
    }
    attr = env->GetByteArrayElements(value, NULL);
    if (!attr) {
        delete param;
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    param->player_setting.num_attr  = numattr/2;
    for(i =0 ; i < numattr; i+=2)
    {
        param->player_setting.attr_ids[i/2] = attr[i];
        param->player_setting.attr_values[i/2] =  attr[i+1];
    }
    //Call Stack Method
    if ((status =
                sBluetoothMultiAvrcpInterface->register_notification_rsp(
                BTRC_EVT_APP_SETTINGS_CHANGED,
                (btrc_notification_type_t)type,param,
                (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp, status: %d", status);
    }
    delete param;
    env->ReleaseByteArrayElements(value, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspPlayStatusNative(JNIEnv *env, jobject object,
                                                        jint type, jint playStatus) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    param.play_status = (btrc_play_status_t)playStatus;

    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(BTRC_EVT_PLAY_STATUS_CHANGED,
                  (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp play status, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspTrackChangeNative(JNIEnv *env, jobject object,
                                                         jint type, jbyteArray track) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *trk = env->GetByteArrayElements(track, NULL);
    if (!trk) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    for (int uid_idx = 0; uid_idx < BTRC_UID_SIZE; ++uid_idx) {
      param.track[uid_idx] = trk[uid_idx];
    }

    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(BTRC_EVT_TRACK_CHANGE,
                  (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp track change, status: %d", status);
    }

    env->ReleaseByteArrayElements(track, trk, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspPlayPosNative(JNIEnv *env, jobject object,
                                                        jint type, jint playPos) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    param.song_pos = (uint32_t)playPos;

    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(BTRC_EVT_PLAY_POS_CHANGED,
                  (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp play position, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspNowPlayingChangedNative(
            JNIEnv *env, jobject object,jint type) {

    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(
            BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED, (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp, nowPlaying Content status: %d",
                    status);
    }
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspUIDsChangedNative(
            JNIEnv *env, jobject object,jint type, jint uidCounter) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    param.uids_changed.uid_counter = (uint16_t)uidCounter;

    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(
            BTRC_EVT_UIDS_CHANGED, (btrc_notification_type_t)type,&param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp, uids changed status: %d",
                    status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean registerNotificationRspAddrPlayerChangedNative(JNIEnv *env,
        jobject object, jint type, jint playerId, jint uidCounter) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    param.addr_player_changed.player_id = (uint16_t)playerId;
    param.addr_player_changed.uid_counter = (uint16_t)uidCounter;

    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(BTRC_EVT_ADDR_PLAYER_CHANGE,
                  (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp address player changed status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;

}

static jboolean registerNotificationRspAvalPlayerChangedNative(JNIEnv *env, jobject object,
                                                        jint type) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    btrc_register_notification_t param;
    bt_status_t status = sBluetoothAvrcpInterface->register_notification_rsp(BTRC_EVT_AVAL_PLAYER_CHANGE,
                  (btrc_notification_type_t)type, &param);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed register_notification_rsp available player changed status, status: %d",
            status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean setVolumeNative(JNIEnv *env, jobject object, jint volume) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    bt_status_t status = sBluetoothAvrcpInterface->set_volume((uint8_t)volume);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed set_volume, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

/* native response for scope as Media player */
static jboolean mediaPlayerListRspNative(JNIEnv *env, jobject object, jbyteArray address,
                                      jint rspStatus, jint uidCounter,jbyte itemType,jint numItems,
                                      jbyteArray playerTypes,jintArray playerSubtypes,
                                      jbyteArray playStatusValues,
                                      jshortArray featureBitmask,jobjectArray textArray) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    param.status = statusCode;
    param.uid_counter = 0;

    jbyte* p_playerTypes = NULL,*p_PlayStatusValues = NULL;
    jshort *p_FeatBitMaskValues = NULL;
    jint *p_playerSubTypes = NULL;
    btrc_folder_items_t *p_items = NULL;
    if (rspStatus == BTRC_STS_NO_ERROR) {
        /* allocate memory */
        p_playerTypes = env->GetByteArrayElements(playerTypes, NULL);
        p_playerSubTypes = env->GetIntArrayElements(playerSubtypes, NULL);
        p_PlayStatusValues = env->GetByteArrayElements(playStatusValues, NULL);
        p_FeatBitMaskValues = env->GetShortArrayElements(featureBitmask, NULL);
        p_items = new btrc_folder_items_t[numItems];
        /* deallocate memory and return if allocation failed */
        if (!p_playerTypes || !p_playerSubTypes || !p_PlayStatusValues || !p_FeatBitMaskValues
                                                                                || !p_items) {
            if (p_playerTypes) env->ReleaseByteArrayElements(playerTypes, p_playerTypes, 0);
            if (p_playerSubTypes) env->ReleaseIntArrayElements(playerSubtypes,
                    p_playerSubTypes, 0);
            if (p_PlayStatusValues) env->ReleaseByteArrayElements(playStatusValues,
                    p_PlayStatusValues , 0);
            if (p_FeatBitMaskValues)
                env->ReleaseShortArrayElements(featureBitmask, p_FeatBitMaskValues, 0);
            if (p_items) delete[] p_items;

        uidElements = env->GetLongArrayElements(uid, NULL);
        if (!uidElements) {
            jniThrowIOException(env, EINVAL);
            return JNI_FALSE;
        }

        typeElements = env->GetIntArrayElements(type, NULL);
        if (!typeElements) {
            jniThrowIOException(env, EINVAL);
            return JNI_FALSE;
        }

        playableElements = env->GetByteArrayElements(playable, NULL);
        if (!playableElements) {
            jniThrowIOException(env, EINVAL);
            return JNI_FALSE;
        }

        /* copy list of media players along with other parameters */
        int itemIdx;
        for (itemIdx = 0; itemIdx < numItems; ++itemIdx) {
            p_items[itemIdx].player.player_id = (uint16_t)(itemIdx+1);
            p_items[itemIdx].player.major_type = p_playerTypes[itemIdx];
            p_items[itemIdx].player.sub_type = p_playerSubTypes[itemIdx];
            p_items[itemIdx].player.play_status = p_PlayStatusValues[itemIdx];
            p_items[itemIdx].player.charset_id = BTRC_CHARSET_ID_UTF8;

            jstring text = (jstring) env->GetObjectArrayElement(textArray, itemIdx);
            /* copy player name */
            if (!copy_jstring(p_items[itemIdx].player.name, BTRC_MAX_ATTR_STR_LEN, text, env))
                break;
            }
            ALOGI("getFolderItemsRspNative: Disp Elem Length: %d", utfStringLength);
            total_len = total_len + utfStringLength + BTRC_FOLDER_ITEM_HEADER;
            if (total_len > size) {
                ALOGI("total_len = : %d, count = %d", total_len, count);
                if (count != 0) {
                    env->DeleteLocalRef(text);
                    break;
                } else {
                    utfStringLength =
                            size - (BTRC_FOLDER_ITEM_HEADER + BTRC_ITEM_TYPE_N_LEN_OCT + 1);
                    ALOGI("modified utfStringLength = : %d", utfStringLength);
                }
            }

            textStr = env->GetStringUTFChars(text, NULL);
            if (!textStr) {
                ALOGE("getFolderItemsRspNative: GetStringUTFChars return NULL");
                env->DeleteLocalRef(text);
                break;
            }
            param.p_item_list[count].u.folder.name.charset_id = BTRC_CHARSET_UTF8;
            param.p_item_list[count].u.folder.name.str_len = utfStringLength;
            param.p_item_list[count].u.folder.name.p_str = new uint8_t[utfStringLength + 1];
            strlcpy((char *)param.p_item_list[count].u.folder.name.p_str, textStr,
                                                                    utfStringLength + 1);
            env->ReleaseStringUTFChars(text, textStr);
            env->DeleteLocalRef(text);
        } else if (itemTypeElements[count] == BTRC_TYPE_MEDIA_ELEMENT) {
            num_attr = 0;
            param.p_item_list[count].u.media.uid = uidElements[count];
            ALOGI("getFolderItemsRspNative: uid: %l", param.p_item_list[count].u.media.uid);
            param.p_item_list[count].u.media.type = (uint8_t)typeElements[count];
            ALOGI("getFolderItemsRspNative: type: %d", param.p_item_list[count].u.media.type);
            text = (jstring) env->GetObjectArrayElement(displayName, count);
            if (text == NULL) {
                ALOGE("getFolderItemsRspNative: App string is NULL, bail out");
                break;
            }
            utfStringLength = env->GetStringUTFLength(text);
            if (!utfStringLength) {
                ALOGE("getFolderItemsRspNative: GetStringUTFLength return NULL");
                env->DeleteLocalRef(text);
                break;
            }
            ALOGI("getFolderItemsRspNative: Disp Elem Length: %d", utfStringLength);

            /* Feature bit mask is 128-bit value each */
            for (int InnCnt = 0; InnCnt < 16; InnCnt ++) {
                p_items[itemIdx].player.features[InnCnt]
                    = (uint8_t)p_FeatBitMaskValues[(itemIdx*16) + InnCnt];
            }
            param.p_item_list[count].u.media.name.charset_id = BTRC_CHARSET_UTF8;
            param.p_item_list[count].u.media.name.str_len = utfStringLength;
            param.p_item_list[count].u.media.name.p_str = new uint8_t[utfStringLength + 1];
            strlcpy((char *)param.p_item_list[count].u.media.name.p_str, textStr,
                                                                    utfStringLength + 1);
            env->ReleaseStringUTFChars(text, textStr);
            env->DeleteLocalRef(text);
            ALOGI("getFolderItemsRspNative: numAttr: %d", numAttElements[count]);
            param.p_item_list[count].u.media.p_attr_list =
                            new btrc_attr_entry_t[numAttElements[count]];

            for (int i = 0; i < numAttElements[count]; i++) {
                text = (jstring) env->GetObjectArrayElement(attValues, (8 * count) + i);
                if (text == NULL) {
                    ALOGE("getFolderItemsRspNative: Attribute string is NULL, continue to next");
                    continue;
                }
                utfStringLength = env->GetStringUTFLength(text);
                if (!utfStringLength) {
                    ALOGE("getFolderItemsRspNative: GetStringUTFLength return NULL");
                    env->DeleteLocalRef(text);
                    continue;
                }
                total_len = total_len + utfStringLength + BTRC_ITEM_ATTRIBUTE_HEADER;
                if (total_len > size) {
                    ALOGI("total_len = %d, count = %d", total_len, count);
                    if (count != 0)
                        num_attr = 0;
                    env->DeleteLocalRef(text);
                    break;
                }

                textStr = env->GetStringUTFChars(text, NULL);
                if (!textStr) {
                    ALOGE("getFolderItemsRspNative: GetStringUTFChars return NULL");
                    env->DeleteLocalRef(text);
                    continue;
                }
                param.p_item_list[count].u.media.p_attr_list[num_attr].attr_id =
                                                    attIdsElements[(8 * count) + i];
                ALOGI("getFolderItemsRspNative: Attr id: %d",
                    param.p_item_list[count].u.media.p_attr_list[num_attr].attr_id);
                param.p_item_list[count].u.media.p_attr_list[num_attr].name.charset_id =
                                                                        BTRC_CHARSET_UTF8;
                param.p_item_list[count].u.media.p_attr_list[num_attr].name.str_len =
                                                                        utfStringLength;
                ALOGI("getFolderItemsRspNative: Attr Length: %d",
                    param.p_item_list[count].u.media.p_attr_list[num_attr].name.str_len);
                param.p_item_list[count].u.media.p_attr_list[num_attr].name.p_str =
                                                            new uint8_t[utfStringLength + 1];
                strlcpy((char *)param.p_item_list[count].u.media.p_attr_list[num_attr].
                                                name.p_str, textStr, utfStringLength + 1);
                num_attr++;
                env->ReleaseStringUTFChars(text, textStr);
                env->DeleteLocalRef(text);
            }
            param.p_item_list[count].u.media.attr_count = num_attr;
            ALOGI("getFolderItemsRspNative: effective numAttr: %d",
                            param.p_item_list[count].u.media.attr_count);
        }
        if (total_len > size) {
            ALOGI("total_len final =: %d", total_len);
            break;
        }
    }


    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->get_folder_items_list_rsp(btAddr,
            (btrc_status_t)rspStatus, uidCounter, numItems, p_items);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_folder_items_list_rsp, status: %d", status);
    }

    if (numItems > 0) {
        env->ReleaseIntArrayElements(itemType, itemTypeElements, 0);
        env->ReleaseLongArrayElements(uid, uidElements, 0);
        env->ReleaseIntArrayElements(type, typeElements, 0);
        env->ReleaseByteArrayElements(playable, playableElements, 0);
        env->ReleaseByteArrayElements(numAtt, numAttElements, 0);
        env->ReleaseIntArrayElements(attIds, attIdsElements, 0);
    }

    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean getFolderItemsRspNative(JNIEnv *env, jobject object, jbyteArray address,
        jint rspStatus, jshort uidCounter,jbyte scope, jint numItems, jbyteArray folderType,
        jbyteArray playable,jbyteArray itemType, jbyteArray itemUidArray,
        jobjectArray displayNameArray, jintArray numAttrs, jintArray attributesIds,
        jobjectArray attributesArray) {


    addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    folderElements = env->GetByteArrayElements(folderItems, NULL);
    if (!folderElements) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    jbyte *p_playable = NULL, *p_item_uid = NULL;
    jbyte* p_item_types = NULL;            /* Folder or Media Item */
    jint* p_attributesIds = NULL;
    jbyte* p_folder_types = NULL;          /* Folder properties like Album/Genre/Artists etc */
    jint* p_num_attrs = NULL;
    btrc_folder_items_t *p_items = NULL;
    /* none of the parameters should be null when no error */
    if (rspStatus == BTRC_STS_NO_ERROR) {
        /* allocate memory to each rsp item */
        if (folderType != NULL)
            p_folder_types = env->GetByteArrayElements(folderType, NULL);
        if (playable != NULL)
            p_playable = env->GetByteArrayElements(playable, NULL);
        if (itemType != NULL)
            p_item_types = env->GetByteArrayElements(itemType, NULL);
        if (NULL != numAttrs)
            p_num_attrs = env->GetIntArrayElements(numAttrs, NULL);
        if (NULL != attributesIds)
            p_attributesIds = env->GetIntArrayElements(attributesIds, NULL);
        if (itemUidArray != NULL)
            p_item_uid = (jbyte*) env->GetByteArrayElements(itemUidArray, NULL);

        p_items = new btrc_folder_items_t[numItems];

        /* if memory alloc failed, release memory */
        if (p_items && p_folder_types && p_playable && p_item_types && p_item_uid &&
             /* attributes can be null if remote requests 0 attributes */
            ((numAttrs != NULL && p_num_attrs)||(!numAttrs && !p_num_attrs)) &&
            ((attributesIds != NULL && p_attributesIds)|| (!attributesIds && !p_attributesIds))) {
            memset(p_items, 0, sizeof(btrc_folder_items_t)*numItems);
            if (scope == BTRC_SCOPE_FILE_SYSTEM || scope == BTRC_SCOPE_SEARCH ||
                scope == BTRC_SCOPE_NOW_PLAYING) {
                int attribCopiedIndex = 0;
                for (int item_idx = 0; item_idx < numItems; item_idx++) {
                    if (BTRC_ITEM_FOLDER == p_item_types[item_idx]){
                        btrc_folder_items_t *pitem = &p_items[item_idx];

                        memcpy(pitem->folder.uid, p_item_uid + item_idx*BTRC_UID_SIZE,
                            BTRC_UID_SIZE);
                        pitem->item_type = (uint8_t)BTRC_ITEM_FOLDER;
                        pitem->folder.charset_id = BTRC_CHARSET_ID_UTF8;
                        pitem->folder.type = p_folder_types[item_idx];
                        pitem->folder.playable = p_playable[item_idx];

                        jstring text = (jstring) env->GetObjectArrayElement(displayNameArray, item_idx);
                        if (!copy_jstring(pitem->folder.name, BTRC_MAX_ATTR_STR_LEN,
                            text, env)) {
                            rspStatus = BTRC_STS_INTERNAL_ERR;
                            env->DeleteLocalRef(text);
                            ALOGE("%s: failed to copy display name of folder item", __func__);
                            break;
                        }
                        env->DeleteLocalRef(text);
                    }
                    else if (BTRC_ITEM_MEDIA == p_item_types[item_idx])
                    {
                        btrc_folder_items_t *pitem = &p_items[item_idx];
                        memcpy(pitem->media.uid, p_item_uid + item_idx*BTRC_UID_SIZE,
                            BTRC_UID_SIZE);

                        pitem->item_type = (uint8_t)BTRC_ITEM_MEDIA;
                        pitem->media.charset_id = BTRC_CHARSET_ID_UTF8;
                        pitem->media.type = BTRC_MEDIA_TYPE_AUDIO;
                        pitem->media.num_attrs = (p_num_attrs != NULL) ?
                            p_num_attrs[item_idx] : 0;

                        jstring text = (jstring) env->GetObjectArrayElement(displayNameArray, item_idx);
                        if (!copy_jstring(pitem->media.name, BTRC_MAX_ATTR_STR_LEN, text, env)){
                            rspStatus = BTRC_STS_INTERNAL_ERR;
                            env->DeleteLocalRef(text);
                            ALOGE("%s: failed to copy display name of media item", __func__);
                            break;
                        }
                        env->DeleteLocalRef(text);

                        /* copy item attributes */
                        if (!copy_item_attributes(env, object, pitem, p_attributesIds,
                                attributesArray, item_idx, attribCopiedIndex)) {
                            ALOGE("%s: error in copying attributes of item = %s",
                                __func__, pitem->media.name);
                            rspStatus = BTRC_STS_INTERNAL_ERR;
                            break;
                        }
                        attribCopiedIndex +=  pitem->media.num_attrs;
                    }
                }
            }
        } else {
            rspStatus = BTRC_STS_INTERNAL_ERR;
            ALOGE("%s: unable to allocate memory", __func__);
        }
    }

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->get_folder_items_list_rsp(btAddr,
            (btrc_status_t)rspStatus, uidCounter,numItems,p_items);
    if (status != BT_STATUS_SUCCESS)
        ALOGE("Failed get_folder_items_list_rsp, status: %d", status);


    /* Release allocated memory for all attributes in each media item */
    if (p_items)
        cleanup_items(p_items, numItems);

    env->ReleaseByteArrayElements(folderItems, folderElements, 0);
    env->ReleaseIntArrayElements(folderItemLengths, folderElementLengths, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean setAddressedPlayerRspNative(JNIEnv *env, jobject object, jbyteArray address,
        jint rspStatus) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->set_addressed_player_rsp(btAddr,
            (btrc_status_t)rspStatus);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("Failed set_addressed_player_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean setBrowsedPlayerRspNative(JNIEnv *env, jobject object, jbyteArray address,
        jint rspStatus, jbyte depth, jint numItems, jobjectArray textArray) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    if (numAttr > BTRC_MAX_ELEM_ATTR_SIZE) {
        ALOGE("get_item_attr_rsp: number of attributes exceed maximum");
        return JNI_FALSE;
    }

    btrc_br_folder_name_t *p_folders = NULL;
    if (rspStatus == BTRC_STS_NO_ERROR) {
        if (depth > 0) {
            p_folders = new btrc_br_folder_name_t[depth];
        }

        for (int folder_idx = 0; folder_idx < depth; folder_idx++) {
            /* copy folder names */
            jstring text = (jstring) env->GetObjectArrayElement(textArray, folder_idx);

    for (i = 0; i < numAttr; ++i) {
        text = (jstring) env->GetObjectArrayElement(textArray, i);

        utfStringLength = env->GetStringUTFLength(text);
        total_len = total_len + utfStringLength + BTRC_ITEM_ATTRIBUTE_HEADER;
        if (total_len > size) {
            ALOGI("total_len: %d", total_len);
            env->DeleteLocalRef(text);
            break;
        }

        textStr = env->GetStringUTFChars(text, NULL);
        if (!textStr) {
            ALOGE("get_item_attr_rsp: GetStringUTFChars return NULL");
            env->DeleteLocalRef(text);
            break;
        }

        pAttrs[i].attr_id = attr[i];
        if (utfStringLength >= BTRC_MAX_ATTR_STR_LEN) {
            ALOGE("get_item_attr_rsp: string length exceed maximum");
        }
        strlcpy((char *)pAttrs[i].text, textStr, BTRC_MAX_ATTR_STR_LEN);
        env->ReleaseStringUTFChars(text, textStr);
        env->DeleteLocalRef(text);
    }

    uint8_t folder_depth = depth; /* folder_depth is 0 if current folder is root */
    uint16_t charset_id = BTRC_CHARSET_ID_UTF8;
    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->set_browsed_player_rsp(btAddr,
            (btrc_status_t) rspStatus, numItems, charset_id, folder_depth, p_folders);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("%s: Failed set_browsed_player_rsp, status: %d", __func__, status);
    }

    if (depth > 0) {
        delete[] p_folders;
    }

    delete[] pAttrs;
    env->ReleaseIntArrayElements(attrIds, attr, 0);
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean changePathRspNative(JNIEnv *env, jobject object, jbyteArray address,
            jint rspStatus, jint numItems) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    param.status = statusCode;
    param.uid_counter = uidCounter;
    param.num_items = itemCount;
    param.charset_id = charId;
    param.folder_depth = folderDepth;

    ALOGI("statusCode: %d", statusCode);
    ALOGI("uidCounter: %d", uidCounter);
    ALOGI("itemCount: %d", itemCount);
    ALOGI("charId: %d", charId);
    ALOGI("folderDepth: %d", folderDepth);

    param.p_folders = new btrc_name_t[folderDepth];

    for (count = 0; count < folderDepth; ++count) {
        text = (jstring) env->GetObjectArrayElement(folderNames, count);

        utfStringLength = env->GetStringUTFLength(text);
        if (!utfStringLength) {
            ALOGE("setBrowsedPlayerRspNative: GetStringUTFLength return NULL");
            env->DeleteLocalRef(text);
            break;
        }

        textStr = env->GetStringUTFChars(text, NULL);
        if (!textStr) {
            ALOGE("setBrowsedPlayerRspNative: GetStringUTFChars return NULL");
            env->DeleteLocalRef(text);
            break;
        }

        param.p_folders[count].str_len = utfStringLength;
        param.p_folders[count].p_str = new uint8_t[utfStringLength + 1];
        strlcpy((char *)param.p_folders[count].p_str, textStr, utfStringLength + 1);
        env->ReleaseStringUTFChars(text, textStr);
        env->DeleteLocalRef(text);

    uint32_t nItems = (uint32_t)numItems;
    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->change_path_rsp(btAddr, (btrc_status_t)rspStatus,
            (uint32_t) nItems);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("Failed change_path_rsp, status: %d", status);
    }

    if ((status = sBluetoothMultiAvrcpInterface->set_browsed_player_rsp(&param,
                                            (bt_bdaddr_t *)addr)) != BT_STATUS_SUCCESS) {
        ALOGE("Failed setBrowsedPlayerRspNative, status: %u", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean searchRspNative(JNIEnv *env, jobject object, jbyteArray address, jint rspStatus,
            jint uidCounter, jint numItems) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    ALOGV("%s: sBluetoothMultiAvrcpInterface: %p", __FUNCTION__, sBluetoothMultiAvrcpInterface);
    ALOGV("status: %d, itemCount: %l", errStatus, itemCount);

    uint32_t nItems = (uint32_t)numItems;
    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->search_rsp(btAddr, (btrc_status_t)rspStatus,
            (uint32_t) uidCounter, (uint32_t) nItems);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("Failed search_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean playItemRspNative(JNIEnv *env, jobject object, jbyteArray address,
        jint rspStatus) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    ALOGV("%s: sBluetoothMultiAvrcpInterface: %p", __FUNCTION__, sBluetoothMultiAvrcpInterface);
    ALOGV("status: %d", errStatus);

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->play_item_rsp(btAddr, (btrc_status_t)rspStatus);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("Failed play_item_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean getTotalNumOfItemsRspNative(JNIEnv *env, jobject object, jbyteArray address,
            jint rspStatus, jint uidCounter, jint numItems) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }
    ALOGI("%s: sBluetoothMultiAvrcpInterface: %p", __FUNCTION__, sBluetoothMultiAvrcpInterface);

    uint32_t nItems = (uint32_t)numItems;
    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->get_total_num_of_items_rsp(btAddr,
            (btrc_status_t)rspStatus, (uint32_t) uidCounter, (uint32_t) nItems);
    if (status != BT_STATUS_SUCCESS) {
        ALOGE("Failed get_total_num_of_items_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);

    ALOGI("isDeviceActiveInHandOffNative: status: %d", status);

    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}

static jboolean addToNowPlayingRspNative(JNIEnv *env, jobject object, jbyteArray address,
        jint rspStatus) {
    if (!sBluetoothAvrcpInterface) {
        ALOGE("%s: sBluetoothAvrcpInterface is null", __func__);
        return JNI_FALSE;
    }

    jbyte *addr = env->GetByteArrayElements(address, NULL);
    if (!addr) {
        jniThrowIOException(env, EINVAL);
        return JNI_FALSE;
    }

    bt_bdaddr_t *btAddr = (bt_bdaddr_t *) addr;
    bt_status_t status = sBluetoothAvrcpInterface->add_to_now_playing_rsp(btAddr,
        (btrc_status_t)rspStatus);
    if (status != BT_STATUS_SUCCESS) {
          ALOGE("Failed add_to_now_playing_rsp, status: %d", status);
    }
    env->ReleaseByteArrayElements(address, addr, 0);
    return (status == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
}


static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void *) classInitNative},
    {"initNative", "(I)V", (void *) initNative},
    {"cleanupNative", "()V", (void *) cleanupNative},
    {"getPlayStatusRspNative", "(III[B)Z", (void *) getPlayStatusRspNative},
    {"getElementAttrRspNative", "(B[I[Ljava/lang/String;[B)Z", (void *) getElementAttrRspNative},
    {"getListPlayerappAttrRspNative", "(B[B[B)Z", (void *) getListPlayerappAttrRspNative},
    {"getPlayerAppValueRspNative", "(B[B[B)Z", (void *) getPlayerAppValueRspNative},
    {"registerNotificationRspPlayStatusNative", "(II[B)Z",
     (void *) registerNotificationRspPlayStatusNative},
    {"SendCurrentPlayerValueRspNative", "(B[B[B)Z",
     (void *) SendCurrentPlayerValueRspNative},
    {"registerNotificationPlayerAppRspNative", "(IB[B[B)Z",
     (void *) registerNotificationPlayerAppRspNative},
    {"registerNotificationRspTrackChangeNative", "(I[B[B)Z",
     (void *) registerNotificationRspTrackChangeNative},
    {"SendSetPlayerAppRspNative", "(I[B)Z",
     (void *) SendSetPlayerAppRspNative},
    {"sendSettingsTextRspNative" , "(I[BI[Ljava/lang/String;[B)Z",
     (void *) sendSettingsTextRspNative},
    {"sendValueTextRspNative" , "(I[BI[Ljava/lang/String;[B)Z",
     (void *) sendValueTextRspNative},
    {"registerNotificationRspPlayPosNative", "(II[B)Z",
     (void *) registerNotificationRspPlayPosNative},
    {"setVolumeNative", "(I[B)Z",
     (void *) setVolumeNative},
    {"setAdressedPlayerRspNative", "(B[B)Z",
     (void *) setAdressedPlayerRspNative},
    {"getMediaPlayerListRspNative", "(BII[B[I[B)Z",
     (void *) getMediaPlayerListRspNative},
    {"registerNotificationRspAddressedPlayerChangedNative", "(II[B)Z",
     (void *) registerNotificationRspAddressedPlayerChangedNative},
    {"registerNotificationRspAvailablePlayersChangedNative", "(I[B)Z",
     (void *) registerNotificationRspAvailablePlayersChangedNative},
    {"registerNotificationRspNowPlayingContentChangedNative", "(I[B)Z",
     (void *) registerNotificationRspNowPlayingContentChangedNative},
    {"setBrowsedPlayerRspNative", "(BIIII[Ljava/lang/String;[B)Z",
                                (void *) setBrowsedPlayerRspNative},
    {"changePathRspNative", "(IJ[B)Z", (void *) changePathRspNative},
    {"playItemRspNative", "(I[B)Z", (void *) playItemRspNative},
    {"getItemAttrRspNative", "(B[I[Ljava/lang/String;I[B)Z", (void *) getItemAttrRspNative},
    {"getFolderItemsRspNative", "(BJ[I[J[I[B[Ljava/lang/String;[B[Ljava/lang/String;[II[B)Z",
                                                            (void *) getFolderItemsRspNative},
    {"isDeviceActiveInHandOffNative", "([B)Z", (void *) isDeviceActiveInHandOffNative},
    {"getTotalNumberOfItemsRspNative", "(IJI[B)Z",
                                     (void *) getTotalNumberOfItemsRspNative},
};

int register_com_android_bluetooth_avrcp(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "com/android/bluetooth/avrcp/Avrcp",
                                    sMethods, NELEM(sMethods));
}

/* Helper function to copy attributes of item.
 * Assumes that all items in response have same number of attributes
 *
 * returns true on succes, false otherwise.
*/
static bool copy_item_attributes(JNIEnv *env, jobject object, btrc_folder_items_t *pitem,
    jint* p_attributesIds, jobjectArray attributesArray, int item_idx, int attribCopiedIndex) {
    bool success = true;

    /* copy attributes of the item */
    if (0 < pitem->media.num_attrs) {
        int num_attrs = pitem->media.num_attrs;
        ALOGI("%s num_attr = %d", __func__, num_attrs);
        pitem->media.p_attrs = new btrc_element_attr_val_t[num_attrs];
        if (!pitem->media.p_attrs) {
            return false;
        }

        for (int tempAtrCount = 0; tempAtrCount < pitem->media.num_attrs; ++tempAtrCount) {
            pitem->media.p_attrs[tempAtrCount].attr_id =
                    p_attributesIds[attribCopiedIndex + tempAtrCount];

            jstring text = (jstring) env->GetObjectArrayElement(attributesArray,
                        attribCopiedIndex + tempAtrCount);

            if (!copy_jstring(pitem->media.p_attrs[tempAtrCount].text, BTRC_MAX_ATTR_STR_LEN,
                    text, env)) {
                success = false;
                env->DeleteLocalRef(text);
                ALOGE("%s: failed to copy attributes", __func__);
                break;
            }
            env->DeleteLocalRef(text);
        }
    }
    return success;
}


/* Helper function to copy String data from java to native
 *
 * returns true on succes, false otherwise
 */
static bool copy_jstring(uint8_t* str, int maxBytes, jstring jstr,JNIEnv* env) {
    if (str == NULL || jstr == NULL || env == NULL)
        return false;

    memset(str, 0, maxBytes);
    const char *p_str = env->GetStringUTFChars(jstr, NULL);
    size_t len = strnlen(p_str, maxBytes-1);
    memcpy(str, p_str, len);

    env->ReleaseStringUTFChars(jstr, p_str);
    return true;
}


/* Helper function to cleanup items */
static void cleanup_items(btrc_folder_items_t* p_items, int numItems) {
    for (int item_idx = 0; item_idx < numItems; item_idx++) {
        /* release memory for attributes in case item is media item */
        if ((BTRC_ITEM_MEDIA == p_items[item_idx].item_type)
            && p_items[item_idx].media.p_attrs != NULL)
            delete[] p_items[item_idx].media.p_attrs;
    }
}

}
