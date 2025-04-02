/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "c-wrapper/c-wrapper.h"
#include "call/audio-device/audio-device.h"
#include "chat/encryption/encryption-engine.h"
#include "chat/encryption/legacy-encryption-engine.h"
#include "core_private.h"
#include "ldap/ldap.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/api/c-ldap.h"
#include "linphone/api/c-types.h"
#include "linphone/utils/utils.h"
#include "linphone/wrapper_utils.h"
#include "private_structs.h"
#include "push-notification-message/push-notification-message.h"
#include "search/remote-contact-directory.h"
#include "vcard/carddav-params.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void _linphone_core_constructor(LinphoneCore *lc);
static void _linphone_core_destructor(LinphoneCore *lc);

L_INTERNAL_DECLARE_C_OBJECT_FUNCTIONS(Core, _linphone_core_constructor, _linphone_core_destructor);

static void _linphone_core_constructor(LinphoneCore *lc) {
	lc->state = LinphoneGlobalOff;
	new (&lc->cache) LinphoneCore::Cache();
}

static void _linphone_core_destructor(LinphoneCore *lc) {
	lc->cache.~Cache();
	_linphone_core_uninit(lc);
}
void linphone_core_set_im_encryption_engine(LinphoneCore *lc, LinphoneImEncryptionEngine *imee) {
	if (lc->im_encryption_engine) {
		linphone_im_encryption_engine_unref(lc->im_encryption_engine);
		lc->im_encryption_engine = NULL;
	}
	if (imee) {
		imee->lc = lc;
		lc->im_encryption_engine = linphone_im_encryption_engine_ref(imee);
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setEncryptionEngine(
		    new LegacyEncryptionEngine(L_GET_CPP_PTR_FROM_C_OBJECT(lc)));
	}
}

void linphone_core_enable_lime_x3dh(LinphoneCore *lc, bool_t enable) {
	linphone_config_set_bool(linphone_core_get_config(lc), "lime", "enabled", enable);
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableLimeX3dh(enable ? true : false);
}

bool_t linphone_core_lime_x3dh_enabled(const LinphoneCore *lc) {
	bool isEnabled = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhEnabled();
	return isEnabled ? TRUE : FALSE;
}

bool_t linphone_core_lime_x3dh_available(const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhAvailable();
}

void linphone_core_set_lime_x3dh_server_url(LinphoneCore *lc, const char *url) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setX3dhServerUrl(L_C_TO_STRING(url));
}

const char *linphone_core_get_lime_x3dh_server_url(LinphoneCore *lc) {
	lc->cache.lime_server_url = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getX3dhServerUrl();
	return L_STRING_TO_C(lc->cache.lime_server_url);
}

// Deprecated
const char *linphone_core_get_linphone_specs(const LinphoneCore *lc) {
	return linphone_config_get_string(linphone_core_get_config(lc), "sip", "linphone_specs", NULL);
}

// Deprecated
void linphone_core_set_linphone_specs(LinphoneCore *lc, const char *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecs(L_C_TO_STRING(specs));
}

void linphone_core_set_linphone_specs_list(LinphoneCore *lc, const bctbx_list_t *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecs(L_GET_CPP_LIST_FROM_C_LIST(specs, const char *, string));
}

void linphone_core_add_linphone_spec(LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addSpec(L_C_TO_STRING(spec));
}

void linphone_core_remove_linphone_spec(LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->removeSpec(L_C_TO_STRING(spec));
}

bctbx_list_t *linphone_core_get_linphone_specs_list(LinphoneCore *lc) {
	return L_GET_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getSpecsList());
}

void linphone_core_enable_friend_list_subscription(LinphoneCore *lc, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableFriendListSubscription(enable == TRUE ? true : false);
}

bool_t linphone_core_is_friend_list_subscription_enabled(LinphoneCore *lc) {
	return linphone_core_friend_list_subscription_enabled(lc);
}

bool_t linphone_core_friend_list_subscription_enabled(LinphoneCore *core) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(core)->isFriendListSubscriptionEnabled() ? TRUE : FALSE;
}

void linphone_core_ensure_registered(LinphoneCore *lc) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pushNotificationReceived("", "", false);
}

void linphone_core_process_push_notification(LinphoneCore *lc, const char *call_id) {
	CoreLogContextualizer logContextualizer(lc);
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pushNotificationReceived(call_id, "", false);
}

void linphone_core_push_notification_received(LinphoneCore *lc, const char *payload, const char *call_id) {
	CoreLogContextualizer logContextualizer(lc);
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pushNotificationReceived(call_id, payload, false);
}

void linphone_core_push_notification_received_2(LinphoneCore *lc,
                                                const char *payload,
                                                const char *call_id,
                                                bool_t is_core_starting) {
	CoreLogContextualizer logContextualizer(lc);
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pushNotificationReceived(call_id, payload, is_core_starting);
}

LinphonePushNotificationMessage *linphone_core_get_new_message_from_callid(LinphoneCore *lc, const char *call_id) {
	std::shared_ptr<PushNotificationMessage> cppMsg =
	    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getPushNotificationMessage(L_C_TO_STRING(call_id));
	if (!cppMsg) return NULL;

	LinphonePushNotificationMessage *msg = (LinphonePushNotificationMessage *)cppMsg->toC();
	if (msg) {
		// We need to take a ref on the object because this function is called from outside linphone-sdk.
		belle_sip_object_ref(msg);
	}
	return msg;
}

/* Uses the chat_room_addr instead of the call_id like linphone_core_get_new_message_from_callid to get the chatroom.
Using the call_id to get the chat room require to add a new param to chat room objects where the conference address is
already here */
LinphoneChatRoom *linphone_core_get_new_chat_room_from_conf_addr(LinphoneCore *lc, const char *chat_room_addr) {
	std::shared_ptr<ChatRoom> cppChatRoom =
	    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getPushNotificationChatRoom(L_C_TO_STRING(chat_room_addr));
	return toC(cppChatRoom);
}

bctbx_list_t *linphone_core_get_audio_devices(const LinphoneCore *lc) {
	return LinphonePrivate::AudioDevice::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getAudioDevices());
}

bctbx_list_t *linphone_core_get_extended_audio_devices(const LinphoneCore *lc) {
	return LinphonePrivate::AudioDevice::getCListFromCppList(
	    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getExtendedAudioDevices());
}

void linphone_core_set_input_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setInputAudioDevice(
	    (audio_device ? LinphonePrivate::AudioDevice::getSharedFromThis(audio_device) : NULL));
}

void linphone_core_set_output_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setOutputAudioDevice(
	    (audio_device ? LinphonePrivate::AudioDevice::getSharedFromThis(audio_device) : NULL));
}

const LinphoneAudioDevice *linphone_core_get_input_audio_device(const LinphoneCore *lc) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

const LinphoneAudioDevice *linphone_core_get_output_audio_device(const LinphoneCore *lc) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

void linphone_core_set_default_input_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultInputAudioDevice(
		    LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

void linphone_core_set_default_output_audio_device(LinphoneCore *lc, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultOutputAudioDevice(
		    LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

const LinphoneAudioDevice *linphone_core_get_default_input_audio_device(const LinphoneCore *lc) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

const LinphoneAudioDevice *linphone_core_get_default_output_audio_device(const LinphoneCore *lc) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

VideoStream *linphone_core_get_preview_stream(LinphoneCore *lc) {
	return lc->previewstream;
}

const char *linphone_core_get_conference_version(const LinphoneCore *lc) {
	return lc->conference_version;
}

const char *linphone_core_get_groupchat_version(const LinphoneCore *lc) {
	return lc->groupchat_version;
}

const char *linphone_core_get_ephemeral_version(const LinphoneCore *lc) {
	return lc->ephemeral_version;
}

bool_t linphone_core_is_in_background(const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->isInBackground();
}

void linphone_core_enable_conference_ics_in_message_body(LinphoneCore *core, bool_t enable) {
	linphone_config_set_bool(linphone_core_get_config(core), "misc", "send_conference_ics_in_message_body", enable);
}

bool_t linphone_core_conference_ics_in_message_body_enabled(const LinphoneCore *core) {
	return linphone_config_get_bool(linphone_core_get_config(core), "misc", "send_conference_ics_in_message_body",
	                                TRUE);
}

LinphoneRemoteContactDirectory *linphone_core_create_card_dav_remote_contact_directory(BCTBX_UNUSED(LinphoneCore *core),
                                                                                       LinphoneCardDavParams *params) {
	auto cardDavParams = CardDavParams::toCpp(params)->getSharedFromThis();
	return RemoteContactDirectory::createCObject(cardDavParams);
}

LinphoneRemoteContactDirectory *linphone_core_create_ldap_remote_contact_directory(BCTBX_UNUSED(LinphoneCore *core),
                                                                                   LinphoneLdapParams *params) {
	auto ldapParams = LdapParams::toCpp(params)->getSharedFromThis();
	return RemoteContactDirectory::createCObject(ldapParams);
}

void linphone_core_add_remote_contact_directory(LinphoneCore *core,
                                                LinphoneRemoteContactDirectory *remoteContactDirectory) {
	CoreLogContextualizer logContextualizer(core);
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->addRemoteContactDirectory(
	    RemoteContactDirectory::toCpp(remoteContactDirectory)->getSharedFromThis());
}

void linphone_core_remove_remote_contact_directory(LinphoneCore *core,
                                                   LinphoneRemoteContactDirectory *remoteContactDirectory) {
	CoreLogContextualizer logContextualizer(core);
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->removeRemoteContactDirectory(
	    RemoteContactDirectory::toCpp(remoteContactDirectory)->getSharedFromThis());
}

bctbx_list_t *linphone_core_get_remote_contact_directories(LinphoneCore *lc) {
	CoreLogContextualizer logContextualizer(lc);
	return RemoteContactDirectory::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getRemoteContactDirectories());
}

LinphoneCardDavParams *linphone_core_create_card_dav_params(LinphoneCore *core) {
	return CardDavParams::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core));
}

LinphoneLdapParams *linphone_core_create_ldap_params(LinphoneCore *core) {
	return LdapParams::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core));
}

LinphoneLdap *linphone_core_create_ldap(LinphoneCore *core) {
	return Ldap::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core));
}

LinphoneLdap *linphone_core_create_ldap_with_params(LinphoneCore *core, LinphoneLdapParams *params) {
	LinphoneLdap *ldap = linphone_ldap_new_with_params(core, params);
	linphone_core_add_ldap(core, ldap);
	return ldap;
}

void linphone_core_clear_ldaps(LinphoneCore *core) {
	CoreLogContextualizer logContextualizer(core);

	std::list<std::shared_ptr<RemoteContactDirectory>> toErase;
	for (auto rdc : L_GET_CPP_PTR_FROM_C_OBJECT(core)->getRemoteContactDirectories()) {
		if (rdc->getType() == LinphoneRemoteContactDirectoryTypeLdap) {
			toErase.push_back(rdc);
		}
	}
	for (auto rdc : toErase) {
		L_GET_CPP_PTR_FROM_C_OBJECT(core)->removeRemoteContactDirectory(rdc);
	}
}

void linphone_core_add_ldap(LinphoneCore *core, LinphoneLdap *ldap) {
	CoreLogContextualizer logContextualizer(core);

	LinphoneLdapParams *params = linphone_ldap_get_params(ldap);
	auto ldapParams = LdapParams::toCpp(params)->getSharedFromThis();
	auto rdc = RemoteContactDirectory::create(ldapParams);
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->addRemoteContactDirectory(rdc);
}

void linphone_core_remove_ldap(LinphoneCore *core, LinphoneLdap *ldap) {
	CoreLogContextualizer logContextualizer(core);

	LinphoneLdapParams *params = linphone_ldap_get_params(ldap);
	auto ldapParams = LdapParams::toCpp(params)->getSharedFromThis();
	shared_ptr<RemoteContactDirectory> toRemove = nullptr;
	for (auto rdc : L_GET_CPP_PTR_FROM_C_OBJECT(core)->getRemoteContactDirectories()) {
		if (rdc->getType() == LinphoneRemoteContactDirectoryTypeLdap && rdc->getLdapParams() == ldapParams) {
			toRemove = rdc;
			break;
		}
	}
	if (toRemove) {
		L_GET_CPP_PTR_FROM_C_OBJECT(core)->removeRemoteContactDirectory(toRemove);
	}
}

bctbx_list_t *linphone_core_get_ldap_list(LinphoneCore *lc) {
	CoreLogContextualizer logContextualizer(lc);

	bctbx_list_t *results = NULL;
	for (auto rdc : L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getRemoteContactDirectories()) {
		if (rdc->getType() == LinphoneRemoteContactDirectoryTypeLdap) {
			auto ldap = Ldap::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(lc), rdc->getLdapParams());
			results = bctbx_list_append(results, ldap);
		}
	}
	return results;
}

bool_t linphone_core_get_chat_messages_aggregation_enabled(LinphoneCore *core) {
	return linphone_config_get_bool(linphone_core_get_config(core), "sip", "chat_messages_aggregation", FALSE);
}

void linphone_core_set_chat_messages_aggregation_enabled(LinphoneCore *core, bool_t enabled) {
	linphone_config_set_bool(linphone_core_get_config(core), "sip", "chat_messages_aggregation", enabled);
}

void linphone_core_set_video_codec_priority_policy(LinphoneCore *core, LinphoneCodecPriorityPolicy policy) {
	CoreLogContextualizer logContextualizer(core);
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->setVideoCodecPriorityPolicy(policy);
}

LinphoneCodecPriorityPolicy linphone_core_get_video_codec_priority_policy(const LinphoneCore *core) {
	CoreLogContextualizer logContextualizer(core);
	return L_GET_CPP_PTR_FROM_C_OBJECT(core)->getVideoCodecPriorityPolicy();
}

LinphoneVcard *linphone_core_create_vcard_from_text(const LinphoneCore *core, const char *input) {
	if (input == NULL) return NULL;
#ifdef VCARD_ENABLED
	return linphone_vcard_context_get_vcard_from_buffer(core->vcard_context, input);
#else
	return NULL;
#endif
}