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

#include "linphone/api/c-push-notification-config.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "push-notification/push-notification-config.h"

using namespace LinphonePrivate;

LinphonePushNotificationConfig *linphone_push_notification_config_new(void) {
	return PushNotificationConfig::createCObject();
}

LinphonePushNotificationConfig *
linphone_push_notification_config_clone(const LinphonePushNotificationConfig *push_cfg) {
	return PushNotificationConfig::toCpp(push_cfg)->clone()->toC();
}

bool_t linphone_push_notification_config_is_equal(const LinphonePushNotificationConfig *push_cfg,
                                                  const LinphonePushNotificationConfig *other_config) {
	return PushNotificationConfig::toCpp(push_cfg)->isEqual(*PushNotificationConfig::toCpp(other_config));
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

const char *linphone_push_notification_config_get_provider(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getProvider());
}

void linphone_push_notification_config_set_provider(LinphonePushNotificationConfig *push_cfg, const char *provider) {
	PushNotificationConfig::toCpp(push_cfg)->setProvider(L_C_TO_STRING(provider));
}

void linphone_push_notification_config_set_remote_push_interval(LinphonePushNotificationConfig *push_cfg, const char *remote_push_interval) {
	PushNotificationConfig::toCpp(push_cfg)->setRemotePushInterval(L_C_TO_STRING(remote_push_interval));
}

const char *linphone_push_notification_config_get_team_id(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getTeamId());
}

void linphone_push_notification_config_set_team_id(LinphonePushNotificationConfig *push_cfg, const char *team_id) {
	PushNotificationConfig::toCpp(push_cfg)->setTeamId(L_C_TO_STRING(team_id));
}

const char *linphone_push_notification_config_get_msg_str(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getMsgStr());
}

void linphone_push_notification_config_set_msg_str(LinphonePushNotificationConfig *push_cfg, const char *msg_str) {
	PushNotificationConfig::toCpp(push_cfg)->setMsgStr(L_C_TO_STRING(msg_str));
}

const char *linphone_push_notification_config_get_call_str(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getCallStr());
}

void linphone_push_notification_config_set_call_str(LinphonePushNotificationConfig *push_cfg, const char *call_str) {
	PushNotificationConfig::toCpp(push_cfg)->setCallStr(L_C_TO_STRING(call_str));
}

const char *linphone_push_notification_config_get_group_chat_str(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getGroupChatStr());
}

void linphone_push_notification_config_set_group_chat_str(LinphonePushNotificationConfig *push_cfg,
                                                          const char *group_chat_str) {
	PushNotificationConfig::toCpp(push_cfg)->setGroupChatStr(L_C_TO_STRING(group_chat_str));
}

const char *linphone_push_notification_config_get_call_snd(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getCallSnd());
}

void linphone_push_notification_config_set_call_snd(LinphonePushNotificationConfig *push_cfg, const char *call_snd) {
	PushNotificationConfig::toCpp(push_cfg)->setCallSnd(L_C_TO_STRING(call_snd));
}

const char *linphone_push_notification_config_get_msg_snd(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getMsgSnd());
}

void linphone_push_notification_config_set_msg_snd(LinphonePushNotificationConfig *push_cfg, const char *msg_snd) {
	PushNotificationConfig::toCpp(push_cfg)->setMsgSnd(L_C_TO_STRING(msg_snd));
}

const char *linphone_push_notification_config_get_bundle_identifier(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getBundleIdentifer());
}

void linphone_push_notification_config_set_bundle_identifier(LinphonePushNotificationConfig *push_cfg,
                                                             const char *bundle_identifier) {
	PushNotificationConfig::toCpp(push_cfg)->setBundleIdentifer(L_C_TO_STRING(bundle_identifier));
}

const char *linphone_push_notification_config_get_voip_token(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getVoipToken());
}

void linphone_push_notification_config_set_voip_token(LinphonePushNotificationConfig *push_cfg,
                                                      const char *voip_token) {
	PushNotificationConfig::toCpp(push_cfg)->setVoipToken(L_C_TO_STRING(voip_token));
}

const char *linphone_push_notification_config_get_remote_token(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getRemoteToken());
}

void linphone_push_notification_config_set_remote_token(LinphonePushNotificationConfig *push_cfg,
                                                        const char *remote_token) {
	PushNotificationConfig::toCpp(push_cfg)->setRemoteToken(L_C_TO_STRING(remote_token));
}

const char *linphone_push_notification_config_get_param(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getParam());
}

void linphone_push_notification_config_set_param(LinphonePushNotificationConfig *push_cfg, const char *param) {
	PushNotificationConfig::toCpp(push_cfg)->setParam(L_C_TO_STRING(param));
}

const char *linphone_push_notification_config_get_prid(const LinphonePushNotificationConfig *push_cfg) {
	return L_STRING_TO_C(PushNotificationConfig::toCpp(push_cfg)->getPrid());
}

void linphone_push_notification_config_set_prid(LinphonePushNotificationConfig *push_cfg, const char *prid) {
	PushNotificationConfig::toCpp(push_cfg)->setPrid(L_C_TO_STRING(prid));
}
