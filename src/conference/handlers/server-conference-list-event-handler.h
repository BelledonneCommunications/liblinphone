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

#ifndef _L_LOCAL_CONFERENCE_LIST_EVENT_HANDLER_H_
#define _L_LOCAL_CONFERENCE_LIST_EVENT_HANDLER_H_

#include <unordered_map>

#include "conference/conference-id.h"
#include "core/core-accessor.h"
#include "event/event-subscribe.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ServerConferenceEventHandler;

class ServerConferenceListEventHandler : public CoreAccessor {
public:
	ServerConferenceListEventHandler(const std::shared_ptr<Core> &core);

	void subscribeReceived(const std::shared_ptr<EventSubscribe> &lev, const LinphoneContent *body);
	void addHandler(std::shared_ptr<ServerConferenceEventHandler> handler);
	void removeHandler(std::shared_ptr<ServerConferenceEventHandler> handler);
	std::shared_ptr<ServerConferenceEventHandler> findHandler(const ConferenceId &conferenceId) const;

	static void notifyResponseCb(LinphoneEvent *lev);

private:
	std::unordered_map<ConferenceId,
	                   std::weak_ptr<ServerConferenceEventHandler>,
	                   ConferenceId::WeakHash,
	                   ConferenceId::WeakEqual>
	    handlers;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_LIST_EVENT_HANDLER_H_
