/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "linphone/wrapper_utils.h"
#include "linphone/utils/utils.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core.h"

#include "private_structs.h"

#include "chat/encryption/encryption-engine.h"
#include "chat/encryption/legacy-encryption-engine.h"
#include "call/audio-device/audio-device.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void _linphone_core_constructor (LinphoneCore *lc);
static void _linphone_core_destructor (LinphoneCore *lc);

L_DECLARE_C_OBJECT_IMPL_WITH_XTORS(
	Core,
	_linphone_core_constructor, _linphone_core_destructor,
	LINPHONE_CORE_STRUCT_FIELDS;

	struct Cache {
		~Cache () {
			
		}

		string lime_server_url;
	} mutable cache;
)

static void _linphone_core_constructor (LinphoneCore *lc) {
	lc->state = LinphoneGlobalOff;
	new(&lc->cache) LinphoneCore::Cache();
}

static void _linphone_core_destructor (LinphoneCore *lc) {
	if (lc->callsCache)
		bctbx_list_free_with_data(lc->callsCache, (bctbx_list_free_func)linphone_call_unref);
	lc->cache.~Cache();
	_linphone_core_uninit(lc);
}

void linphone_core_set_im_encryption_engine (LinphoneCore *lc, LinphoneImEncryptionEngine *imee) {
	if (lc->im_encryption_engine) {
		linphone_im_encryption_engine_unref(lc->im_encryption_engine);
		lc->im_encryption_engine = NULL;
	}
	if (imee) {
		imee->lc = lc;
		lc->im_encryption_engine = linphone_im_encryption_engine_ref(imee);
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setEncryptionEngine(new LegacyEncryptionEngine(L_GET_CPP_PTR_FROM_C_OBJECT(lc)));
	}
	
}

void linphone_core_enable_lime_x3dh (LinphoneCore *lc, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableLimeX3dh(enable ? true : false);
}

bool_t linphone_core_lime_x3dh_enabled (const LinphoneCore *lc) {
	bool isEnabled = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhEnabled();
	return isEnabled ? TRUE : FALSE;
}

bool_t linphone_core_lime_x3dh_available (const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhAvailable();
}

void linphone_core_set_lime_x3dh_server_url(LinphoneCore *lc, const char *url) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setX3dhServerUrl(L_C_TO_STRING(url));
}

const char *linphone_core_get_lime_x3dh_server_url(LinphoneCore *lc) {
	lc->cache.lime_server_url = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getX3dhServerUrl();
	return L_STRING_TO_C(lc->cache.lime_server_url);
}

//Deprecated
const char *linphone_core_get_linphone_specs (const LinphoneCore *lc) {
	return lp_config_get_string(linphone_core_get_config(lc), "sip", "linphone_specs", NULL);
}

//Deprecated
void linphone_core_set_linphone_specs (LinphoneCore *lc, const char *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecs(Utils::cStringToCppString(specs));
}

void linphone_core_set_linphone_specs_list (LinphoneCore *lc, const bctbx_list_t *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecsList(L_GET_CPP_LIST_FROM_C_LIST(specs, const char *, string));
}

void linphone_core_add_linphone_spec (LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addSpec(Utils::cStringToCppString(spec));
}

void linphone_core_remove_linphone_spec (LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->removeSpec(Utils::cStringToCppString(spec));
}

const bctbx_list_t *linphone_core_get_linphone_specs_list (LinphoneCore *lc) {
	return L_GET_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getSpecsList());
}

void linphone_core_enable_friend_list_subscription(LinphoneCore *lc, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableFriendListSubscription(enable == TRUE ? true : false);
}

bool_t linphone_core_is_friend_list_subscription_enabled(LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->isFriendListSubscriptionEnabled() ? TRUE : FALSE;
}

void linphone_core_ensure_registered(LinphoneCore *lc) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pushNotificationReceived();
}

bctbx_list_t *linphone_core_get_audio_devices(const LinphoneCore *lc) {
	return LinphonePrivate::AudioDevice::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getAudioDevices());
}

bctbx_list_t *linphone_core_get_extended_audio_devices(const LinphoneCore *lc) {
	return LinphonePrivate::AudioDevice::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getExtendedAudioDevices());
}

void linphone_core_set_input_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setInputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

void linphone_core_set_output_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setOutputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

const LinphoneAudioDevice* linphone_core_get_input_audio_device(const LinphoneCore *lc) {
	LinphonePrivate::AudioDevice *audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

const LinphoneAudioDevice* linphone_core_get_output_audio_device(const LinphoneCore *lc) {
	LinphonePrivate::AudioDevice *audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

void linphone_core_set_default_input_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultInputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

void linphone_core_set_default_output_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultOutputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

const LinphoneAudioDevice* linphone_core_get_default_input_audio_device(const LinphoneCore *lc) {
	LinphonePrivate::AudioDevice *audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

const LinphoneAudioDevice* linphone_core_get_default_output_audio_device(const LinphoneCore *lc) {
	LinphonePrivate::AudioDevice *audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}
