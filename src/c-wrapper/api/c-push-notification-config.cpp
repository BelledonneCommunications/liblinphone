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

#include "linphone/api/c-push-notification-config.h"
#include "c-wrapper/c-wrapper.h"
#include "push-notification/push-notification-config.h"

using namespace LinphonePrivate;

LinphonePushNotificationConfig *linphone_push_notification_config_new(void) {
	return PushNotificationConfig::createCObject();
}

LinphonePushNotificationConfig *linphone_push_notification_config_ref(LinphonePushNotificationConfig *push_cfg) {
	if (push_cfg) {
		PushNotificationConfig::toCpp(push_cfg)->ref();
		return push_cfg;
	}
	return NULL;
}

void linphone_push_notification_config_unref(LinphonePushNotificationConfig *push_cfg) {
	if (push_cfg) {
		PushNotificationConfig::toCpp(push_cfg)->unref();
	}
}

const char *linphone_push_notification_config_get_provider (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getProvider().c_str();
}

void linphone_push_notification_config_set_provider (LinphonePushNotificationConfig *push_cfg, const char *provider) {
	PushNotificationConfig::toCpp(push_cfg)->setProvider(provider);
}

const char *linphone_push_notification_config_get_team_id (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getTeamId().c_str();
}

void linphone_push_notification_config_set_team_id (LinphonePushNotificationConfig *push_cfg, const char *team_id) {
	PushNotificationConfig::toCpp(push_cfg)->setTeamId(team_id);
}

const char *linphone_push_notification_config_get_msg_str (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getMsgStr().c_str();
}

void linphone_push_notification_config_set_msg_str (LinphonePushNotificationConfig *push_cfg, const char *msg_str) {
	PushNotificationConfig::toCpp(push_cfg)->setMsgStr(msg_str);
}

const char *linphone_push_notification_config_get_call_str (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getCallStr().c_str();
}

void linphone_push_notification_config_set_call_str (LinphonePushNotificationConfig *push_cfg, const char *call_str) {
	PushNotificationConfig::toCpp(push_cfg)->setCallStr(call_str);
}

const char *linphone_push_notification_config_get_group_chat_str (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getGroupChatStr().c_str();
}

void linphone_push_notification_config_set_group_chat_str (LinphonePushNotificationConfig *push_cfg, const char *group_chat_str) {
	PushNotificationConfig::toCpp(push_cfg)->setGroupChatStr(group_chat_str);
}

const char *linphone_push_notification_config_get_call_snd (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getCallSnd().c_str();
}

void linphone_push_notification_config_set_call_snd (LinphonePushNotificationConfig *push_cfg, const char *call_snd) {
	PushNotificationConfig::toCpp(push_cfg)->setCallSnd(call_snd);
}

const char *linphone_push_notification_config_get_msg_snd (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getMsgSnd().c_str();
}

void linphone_push_notification_config_set_msg_snd (LinphonePushNotificationConfig *push_cfg, const char *msg_snd) {
	PushNotificationConfig::toCpp(push_cfg)->setMsgSnd(msg_snd);
}

const char *linphone_push_notification_config_get_bundle_identifier (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getBundleIdentifer().c_str();
}

void linphone_push_notification_config_set_bundle_identifier (LinphonePushNotificationConfig *push_cfg, const char *bundle_identifier) {
	PushNotificationConfig::toCpp(push_cfg)->setBundleIdentifer(bundle_identifier);
}

const char *linphone_push_notification_config_get_voip_token (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getVoipToken().c_str();
}

void linphone_push_notification_config_set_voip_token (LinphonePushNotificationConfig *push_cfg, const char *voip_token) {
	PushNotificationConfig::toCpp(push_cfg)->setVoipToken(voip_token);
}

const char *linphone_push_notification_config_get_remote_token (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getRemoteToken().c_str();
}

void linphone_push_notification_config_set_remote_token (LinphonePushNotificationConfig *push_cfg, const char *remote_token) {
	PushNotificationConfig::toCpp(push_cfg)->setRemoteToken(remote_token);
}

const char *linphone_push_notification_config_get_param (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getParam().c_str();
}

void linphone_push_notification_config_set_param (LinphonePushNotificationConfig *push_cfg, const char *param) {
	PushNotificationConfig::toCpp(push_cfg)->setParam(param);
}

const char *linphone_push_notification_config_get_prid (const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->getPrid().c_str();
}

void linphone_push_notification_config_set_prid (LinphonePushNotificationConfig *push_cfg, const char *prid) {
	PushNotificationConfig::toCpp(push_cfg)->setPrid(prid);
}
