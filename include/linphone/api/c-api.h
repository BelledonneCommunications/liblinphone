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

#ifndef _L_C_API_H_
#define _L_C_API_H_

#include "linphone/utils/general.h"

#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-device.h"
#include "linphone/api/c-account-manager-services-request-cbs.h"
#include "linphone/api/c-account-manager-services-request.h"
#include "linphone/api/c-account-manager-services.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-alert-cbs.h"
#include "linphone/api/c-alert.h"
#include "linphone/api/c-audio-device.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-bearer-token.h"
#include "linphone/api/c-call-cbs.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-call-stats.h"
#include "linphone/api/c-call.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-carddav-params.h"
#include "linphone/api/c-chat-message-cbs.h"
#include "linphone/api/c-chat-message-reaction.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-params.h"
#include "linphone/api/c-chat-room-cbs.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-conference-scheduler.h"
#include "linphone/api/c-conference.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-dial-plan.h"
#include "linphone/api/c-dictionary.h"
#include "linphone/api/c-digest-authentication-policy.h"
#include "linphone/api/c-ekt-info.h"
#include "linphone/api/c-event-cbs.h"
#include "linphone/api/c-event-log.h"
#include "linphone/api/c-event.h"
#include "linphone/api/c-friend-device.h"
#include "linphone/api/c-friend-phone-number.h"
#include "linphone/api/c-friend.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/api/c-ldap.h"
#include "linphone/api/c-magic-search.h"
#include "linphone/api/c-message-waiting-indication.h"
#include "linphone/api/c-nat-policy.h"
#include "linphone/api/c-participant-device-cbs.h"
#include "linphone/api/c-participant-device-identity.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-imdn-state.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/api/c-push-notification-config.h"
#include "linphone/api/c-push-notification-message.h"
#include "linphone/api/c-recorder-params.h"
#include "linphone/api/c-recorder.h"
#include "linphone/api/c-remote-contact-directory.h"
#include "linphone/api/c-search-result.h"
#include "linphone/api/c-signal-information.h"
#include "linphone/api/c-types.h"
#include "linphone/api/c-video-source-descriptor.h"

#endif // ifndef _L_C_API_H_
