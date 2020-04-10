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

#ifndef _L_IMDN_H_
#define _L_IMDN_H_

#include "linphone/utils/general.h"

#include "core/core-listener.h"
#include "utils/background-task.h"

#include "private.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ChatMessage;
class ChatRoom;
class ImdnMessage;

class Imdn : public CoreListener {
public:
	enum class Type {
		Delivery,
		Display
	};

	struct MessageReason {
		MessageReason (const std::shared_ptr<ChatMessage> &message, LinphoneReason reason)
			: message(message), reason(reason) {}

		bool operator== (const MessageReason &other) const {
			return (message == other.message) && (reason == other.reason);
		}

		const std::shared_ptr<ChatMessage> message;
		LinphoneReason reason;
	};

	Imdn (ChatRoom *chatRoom);
	~Imdn ();

	void notifyDelivery (const std::shared_ptr<ChatMessage> &message);
	void notifyDeliveryError (const std::shared_ptr<ChatMessage> &message, LinphoneReason reason);
	void notifyDisplay (const std::shared_ptr<ChatMessage> &message);

	void onImdnMessageDelivered (const std::shared_ptr<ImdnMessage> &message);
	bool hasUndeliveredImdnMessage();

	// CoreListener
	void onGlobalStateChanged (LinphoneGlobalState state) override;
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onRegistrationStateChanged(LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message) override;
	bool aggregationEnabled () const;

	static std::string createXml (const std::string &id, time_t time, Imdn::Type imdnType, LinphoneReason reason);
	static void parse (const std::shared_ptr<ChatMessage> &chatMessage);
	static bool isError (const std::shared_ptr<ChatMessage> &chatMessage);

private:
	LinphoneProxyConfig *getRelatedProxyConfig();
	static int timerExpired (void *data, unsigned int revents);

	void send ();
	void startTimer ();
	void stopTimer ();

private:
	ChatRoom *chatRoom = nullptr;
	std::list<std::shared_ptr<ChatMessage>> deliveredMessages;
	std::list<std::shared_ptr<ChatMessage>> displayedMessages;
	std::list<MessageReason> nonDeliveredMessages;
	std::list<std::shared_ptr<ImdnMessage>> sentImdnMessages;
	belle_sip_source_t *timer = nullptr;
	BackgroundTask bgTask { "IMDN sending" };
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_IMDN_H_
