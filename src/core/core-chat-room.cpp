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

#include <iterator>

#include "linphone/utils/algorithm.h"

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/chat-room/basic-chat-room.h"
#include "chat/chat-room/chat-room-p.h"
#include "conference/participant.h"
#include "core-p.h"
#include "logger/logger.h"

#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/basic-to-client-group-chat-room.h"
#include "chat/chat-room/client-group-chat-room-p.h"
#include "chat/chat-room/client-group-to-basic-chat-room.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#endif

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"

#include "chat/chat-message/chat-message-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------
// Helpers.
// -----------------------------------------------------------------------------

/*
 * Returns the best local address to talk with peer address.
 * If peerAddress is not defined, returns the local address of the default proxy config.
 * If withGruu is true, returns the local address with its gruu parameter. FlexisipChat kind of chatroom (also refered as ClientGroupChatRoom)
 * require a local address with gruu, unlike basic chatrooms.
 */
IdentityAddress CorePrivate::getDefaultLocalAddress(const IdentityAddress *peerAddress, bool withGruu) const {
	LinphoneCore *cCore = getCCore();
	LinphoneProxyConfig *proxy = nullptr;

	if (peerAddress) {
		LinphoneAddress *cPeerAddress = linphone_address_new(peerAddress->asString().c_str());
		if (cPeerAddress) {
			proxy = linphone_core_lookup_known_proxy(cCore, cPeerAddress);
			linphone_address_unref(cPeerAddress);
		}
	}

	if (!proxy)
		proxy =	linphone_core_get_default_proxy_config(cCore);

	IdentityAddress localAddress;
	if (proxy) {
		char *identity = linphone_address_as_string(
			(withGruu && linphone_proxy_config_get_contact(proxy))? linphone_proxy_config_get_contact(proxy) : linphone_proxy_config_get_identity_address(proxy));
		localAddress = IdentityAddress(identity);
		bctbx_free(identity);
	} else
		localAddress = IdentityAddress(linphone_core_get_primary_contact(cCore));

	return localAddress;
}

IdentityAddress CorePrivate::getIdentityAddressWithGruu(const IdentityAddress &identityAddress) const {
	LinphoneCore *cCore = getCCore();
	LinphoneProxyConfig *proxyConfig = nullptr;
	IdentityAddress identityAddressWithGruu;

	if (identityAddress.isValid()) {
		LinphoneAddress *cIdentityAddress = linphone_address_new(identityAddress.asString().c_str());
		proxyConfig = linphone_core_lookup_known_proxy(cCore, cIdentityAddress);
		linphone_address_unref(cIdentityAddress);

		if (proxyConfig) {
			const LinphoneAddress *contactAddress = linphone_proxy_config_get_contact(proxyConfig);
			if (contactAddress) {
				char *contact = linphone_address_as_string(contactAddress);
				identityAddressWithGruu = IdentityAddress(contact);
				bctbx_free(contact);
			}
		}
	}

	return identityAddressWithGruu;
}

// -----------------------------------------------------------------------------

//Base client group chat room creator
shared_ptr<AbstractChatRoom> CorePrivate::createClientGroupChatRoom (
	const std::string &subject,
	const IdentityAddress &conferenceFactoryUri,
	const ConferenceId &conferenceId,
	const Content &content,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const std::shared_ptr<ChatRoomParams> &params,
	bool fallback
) {
#ifdef HAVE_ADVANCED_IM
	L_Q();

	if (!params || !params->isValid()) {
		lWarning() << "Invalid chat room parameters given for client group chat room creation";
		return nullptr;
	}
	if (!conferenceId.getLocalAddress().hasGruu()){
		lError() << "createClientGroupChatRoom(): local address [" << conferenceId.getLocalAddress() << "] must have a gruu.";
		return nullptr;
	}
	shared_ptr<ClientGroupChatRoom> clientGroupChatRoom(new ClientGroupChatRoom(q->getSharedFromThis(),
										    conferenceFactoryUri,
										    conferenceId,
										    subject,
										    content,
										    capabilities,
										    params
										    ));

	shared_ptr<AbstractChatRoom> chatRoom;
	if (fallback && (clientGroupChatRoom->getCapabilities() & ClientGroupChatRoom::Capabilities::OneToOne)) {
		//Create a ClientGroupToBasicChatRoom to handle fallback from ClientGroupChatRoom to BasicGroupChatRoom if
		//only one participant is invited and that it does not support group chat.
		chatRoom = make_shared<ClientGroupToBasicChatRoom>(clientGroupChatRoom);
		ClientGroupChatRoomPrivate *dClientGroupChatRoom = clientGroupChatRoom->getPrivate();
		dClientGroupChatRoom->setCallSessionListener(chatRoom->getPrivate());
		dClientGroupChatRoom->setChatRoomListener(chatRoom->getPrivate());
	} else {
		chatRoom = clientGroupChatRoom;
	}
	chatRoom->setState(ConferenceInterface::State::Instantiated);
	noCreatedClientGroupChatRooms[chatRoom.get()] = chatRoom;
	return chatRoom;
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return nullptr;
#endif
}

shared_ptr<AbstractChatRoom> CorePrivate::createClientGroupChatRoom (
	const string &subject,
	const ConferenceId &conferenceId,
	const Content &content,
	bool encrypted
) {
#ifdef HAVE_ADVANCED_IM
	L_Q();

	shared_ptr<ChatRoomParams> params = ChatRoomParams::create(subject, encrypted, true, ChatRoomParams::ChatRoomBackend::FlexisipChat);
	shared_ptr<ClientGroupChatRoom> clientGroupChatRoom(new ClientGroupChatRoom(
										    q->getSharedFromThis(),
										    conferenceId.getPeerAddress(),
										    conferenceId,
										    subject,
										    content,
										    ChatRoomParams::toCapabilities(params),
										    params
										    ));

	clientGroupChatRoom->setState(ConferenceInterface::State::Instantiated);
	noCreatedClientGroupChatRooms[clientGroupChatRoom.get()] = clientGroupChatRoom;
	return clientGroupChatRoom;
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return nullptr;
#endif
}

//From deprecated public API
shared_ptr<AbstractChatRoom> CorePrivate::createClientGroupChatRoom(const string &subject, bool fallback, bool encrypted) {
	L_Q();

	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(nullptr, true);
	IdentityAddress conferenceFactoryUri(Core::getConferenceFactoryUri(q->getSharedFromThis(), defaultLocalAddress));
	shared_ptr<ChatRoomParams> params = ChatRoomParams::create(subject, encrypted, !fallback, ChatRoomParams::ChatRoomBackend::FlexisipChat);

	return createClientGroupChatRoom(subject, conferenceFactoryUri, ConferenceId(IdentityAddress(), defaultLocalAddress), Content(), ChatRoomParams::toCapabilities(params), params, fallback);
}

shared_ptr<AbstractChatRoom> CorePrivate::createBasicChatRoom (
	const ConferenceId &conferenceId,
	ChatRoom::CapabilitiesMask capabilities,
	const shared_ptr<ChatRoomParams> &params
) {
	L_Q();

	shared_ptr<AbstractChatRoom> chatRoom;
	BasicChatRoom *basicChatRoom = new BasicChatRoom(q->getSharedFromThis(), conferenceId, params);
	string conferenceFactoryUri = Core::getConferenceFactoryUri(q->getSharedFromThis(), conferenceId.getLocalAddress());
	if (basicToFlexisipChatroomMigrationEnabled()) {
		capabilities.set(ChatRoom::Capabilities::Migratable);
	} else {
		capabilities.unset(ChatRoom::Capabilities::Migratable);
	}

#ifdef HAVE_ADVANCED_IM
	if ((capabilities & ChatRoom::Capabilities::Migratable) && !conferenceFactoryUri.empty()) {
		chatRoom.reset(new BasicToClientGroupChatRoom(shared_ptr<BasicChatRoom>(basicChatRoom)));
	}
	else {
#endif
		chatRoom.reset(basicChatRoom);
#ifdef HAVE_ADVANCED_IM
	}
#endif
	chatRoom->setState(ConferenceInterface::State::Instantiated);
	chatRoom->setState(ConferenceInterface::State::Created);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> CorePrivate::searchChatRoom (const shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddress, const IdentityAddress &remoteAddress, const std::list<IdentityAddress> &participants) const {
	for (auto it = chatRoomsById.begin(); it != chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		const IdentityAddress &curLocalAddress = chatRoom->getLocalAddress();
		const IdentityAddress &curRemoteAddress = chatRoom->getPeerAddress();
		ChatRoom::CapabilitiesMask capabilities = chatRoom->getCapabilities();

		if (params) {
			if (!params->isGroup() && !(capabilities & ChatRoom::Capabilities::OneToOne))
				continue;

			if (params->isGroup() && !(capabilities & ChatRoom::Capabilities::Conference))
				continue;

			if (params->isEncrypted() != bool(capabilities & ChatRoom::Capabilities::Encrypted))
				continue;

			if (!params->getSubject().empty() && params->getSubject() != chatRoom->getSubject())
				continue;
		}

		if (localAddress.getAddressWithoutGruu() != curLocalAddress.getAddressWithoutGruu())
			continue;

		if (remoteAddress.isValid() && remoteAddress.getAddressWithoutGruu() != curRemoteAddress.getAddressWithoutGruu())
			continue;

		bool allFound = true;
		for (const auto &participant : participants) {
			bool found = false;
			for (const auto &p : chatRoom->getParticipants()) {
				if (p->getAddress().getAddressWithoutGruu() == participant.getAddressWithoutGruu()) {
					found = true;
					break;
				}
			}
			if (!found) {
				allFound = false;
				break;
			}
		}
		if (!allFound) continue;

		return chatRoom;
	}
	return nullptr;
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddr, const std::string &subject, const std::list<IdentityAddress> &participants) {
	params->setSubject(subject);
	return createChatRoom(params, localAddr, participants);
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddr, const std::list<IdentityAddress> &participants) {
#ifdef HAVE_ADVANCED_IM
	L_Q();
#endif

	if (!params) {
		lWarning() << "Trying to create chat room with null parameters";
		return nullptr;
	}
	if (!params->isValid()) {
		lWarning() << "Trying to create chat room with invalid parameters " << params->toString();
		return nullptr;
	}
	if (!linphone_factory_is_chatroom_backend_available(
			linphone_factory_get(),
			static_cast<LinphoneChatRoomBackend>(params->getChatRoomBackend()))
	) {
		lWarning() << "Tying to create chat room with unavailable backend";
		return nullptr;
	}

	shared_ptr<AbstractChatRoom> chatRoom;
	if (params->getChatRoomBackend() == ChatRoomParams::ChatRoomBackend::FlexisipChat) {
#ifdef HAVE_ADVANCED_IM
		string conferenceFactoryUri = Core::getConferenceFactoryUri(q->getSharedFromThis(), localAddr);
		if (conferenceFactoryUri.empty()) {
			lWarning() << "Not creating group chat room: no conference factory uri for local address [" << localAddr << "]";
			return nullptr;
		}


		ConferenceId conferenceId = ConferenceId(IdentityAddress(), localAddr);
		if (!localAddr.hasGruu()) {
			lWarning() << "Local identity address [" << localAddr << "] doesn't have a gruu, let's try to find it";
			IdentityAddress localAddrWithGruu = getIdentityAddressWithGruu(localAddr);
			if (localAddrWithGruu.isValid()) {
				lInfo() << "Found matching contact address [" << localAddrWithGruu << "] to use instead";
				conferenceId = ConferenceId(IdentityAddress(), localAddrWithGruu);
			} else {
				lError() << "Failed to find matching contact address with gruu for identity address [" << localAddr << "], client group chat room creation will fail!";
			}
		}
		if (!params->isGroup() && participants.size()>0) {
			// Prevent multiple 1-1 conference based chat room with same local/remote addresses
			chatRoom = q->findOneToOneChatRoom(localAddr, participants.front(), false, true, params->isEncrypted());
			if (chatRoom != nullptr) {
				lWarning() << "Found already existing 1-1 chat room that matches, using this one";
				return chatRoom;
			}
		}

		chatRoom = createClientGroupChatRoom(params->getSubject(),
						     IdentityAddress(conferenceFactoryUri),
						     conferenceId,
						     Content(),
						     ChatRoomParams::toCapabilities(params),
						     params,
						     false);

		if (!chatRoom) {
			lWarning() << "Cannot create createClientGroupChatRoom with subject [" << params->getSubject() <<"]";
			return nullptr;
		}
		if (!chatRoom->addParticipants(participants)) {
			lWarning() << "Couldn't add participants to newly created chat room, aborting";
			return nullptr;
		}
#else
		lWarning() << "Advanced IM such as group chat is disabled!";
		return nullptr;
#endif
	} else {
		if (participants.size() != 1) {
			lWarning() << "Trying to create BasicChatRoom with zero or more than one participant";
			return nullptr;
		}
		ChatRoom::CapabilitiesMask capabilities = ChatRoomParams::toCapabilities(params);

		IdentityAddress remoteAddr = IdentityAddress(participants.front());
		list<IdentityAddress> emptyList;
		chatRoom = searchChatRoom(params, localAddr, remoteAddr, emptyList);

		if (chatRoom == nullptr) {
			chatRoom = createBasicChatRoom(ConferenceId(remoteAddr, localAddr), capabilities, params);
			insertChatRoom(chatRoom);
			insertChatRoomWithDb(chatRoom);
		} else {
			lInfo() << "Found an existing BasicChatRoom with this participant, using it instead of creating a new one";
		}
	}
	return chatRoom;
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const std::string &subject, const std::list<IdentityAddress> &participants) {
	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(nullptr, params->getChatRoomBackend() == ChatRoomParams::ChatRoomBackend::FlexisipChat);
	return createChatRoom(params, defaultLocalAddress, subject, participants);
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const std::string &subject, const std::list<IdentityAddress> &participants) {
	L_Q();

	shared_ptr<ChatRoomParams> params = ChatRoomParams::getDefaults(q->getSharedFromThis());
	if (participants.size() > 1) {
		//Try to infer chat room type based on requested participants number
		params->setChatRoomBackend(ChatRoomParams::ChatRoomBackend::FlexisipChat);
	} else {
		params->setChatRoomBackend(ChatRoomParams::ChatRoomBackend::Basic);
	}
	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(nullptr, params->getChatRoomBackend() == ChatRoomParams::ChatRoomBackend::FlexisipChat);
	return createChatRoom(params, defaultLocalAddress, subject, participants);
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddr, const IdentityAddress &participant) {
	return createChatRoom(params, localAddr, "", {participant});
}

//Assume basic chat room creation
shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const IdentityAddress &participant) {
	return createChatRoom("", {participant});
}

void CorePrivate::insertChatRoom (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_ASSERT(chatRoom);

	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	auto it = chatRoomsById.find(conferenceId);
	// Chat room not exist or yes but with the same pointer!
	L_ASSERT(it == chatRoomsById.end() || it->second == chatRoom);
	if (it == chatRoomsById.end()) {
		// Remove chat room from workaround cache.
		noCreatedClientGroupChatRooms.erase(chatRoom.get());
		lInfo() << "Insert chat room " << conferenceId << " to core map";
		chatRoomsById[conferenceId] = chatRoom;
	}
}

void CorePrivate::insertChatRoomWithDb (const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
	L_ASSERT(chatRoom->getState() == ConferenceInterface::State::Created);
	if (mainDb->isInitialized()) mainDb->insertChatRoom(chatRoom, notifyId);
}

void CorePrivate::loadChatRooms () {
	chatRoomsById.clear();
#ifdef HAVE_ADVANCED_IM
	if (remoteListEventHandler)
		remoteListEventHandler->clearHandlers();
#endif

	if (!mainDb->isInitialized()) return;
	for (auto &chatRoom : mainDb->getChatRooms()) {
		insertChatRoom(chatRoom);
	}
	sendDeliveryNotifications();
}

void CorePrivate::handleEphemeralMessages (time_t currentTime) {
	if (!ephemeralMessages.empty()) {
		shared_ptr<ChatMessage> msg = ephemeralMessages.front();
		time_t expireTime = msg->getEphemeralExpireTime();
		if (currentTime > expireTime) {
			shared_ptr<LinphonePrivate::EventLog> event = LinphonePrivate::MainDb::getEvent(mainDb, msg->getStorageId());
			shared_ptr<AbstractChatRoom> chatRoom = msg->getChatRoom();
			if (chatRoom && event) {
				LinphonePrivate::EventLog::deleteFromDatabase(event);
				lInfo() << "[Ephemeral] Message deleted from database";
				
				// Notify ephemeral message deleted to message if exists.
				LinphoneChatMessage *message = L_GET_C_BACK_PTR(msg.get());
				if (message) {
					LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(message);
					if (cbs && linphone_chat_message_cbs_get_ephemeral_message_deleted(cbs)) {
						linphone_chat_message_cbs_get_ephemeral_message_deleted(cbs)(message);
					}
					_linphone_chat_message_notify_ephemeral_message_deleted(message);
				}

				// Notify ephemeral message deleted to chat room & core.
				LinphoneChatRoom *cr = L_GET_C_BACK_PTR(chatRoom);
				_linphone_chat_room_notify_ephemeral_message_deleted(cr, L_GET_C_BACK_PTR(event));
				linphone_core_notify_chat_room_ephemeral_message_deleted(linphone_chat_room_get_core(cr), cr);

			}

			// Delete message from this list even when chatroom is gone.
			ephemeralMessages.pop_front();
			handleEphemeralMessages(currentTime);
		} else {
			startEphemeralMessageTimer(expireTime);
		}
	} else {
		initEphemeralMessages();
	}
}

void CorePrivate::initEphemeralMessages () {
	if (mainDb && mainDb->isInitialized()) {
		ephemeralMessages.clear();
		ephemeralMessages = mainDb->getEphemeralMessages();
		if (!ephemeralMessages.empty()) {
			lInfo() << "[Ephemeral] list initiated";
			shared_ptr<ChatMessage> msg = ephemeralMessages.front();
			startEphemeralMessageTimer(msg->getEphemeralExpireTime());
		}
	}
}

void CorePrivate::updateEphemeralMessages (const shared_ptr<ChatMessage> &message) {
	if (ephemeralMessages.empty()) {
		// Can not determine this message will expire most quickly, so init this list.
		initEphemeralMessages();
	} else {
		shared_ptr<ChatMessage> lastmsg = ephemeralMessages.back();
		if (lastmsg->getEphemeralLifetime() < message->getEphemeralLifetime()) {
			// The last message of this list will expire more quickly than this message, can not determine this message will expire most quickly in the remaining messages.
			return;
		}
		for (std::list<shared_ptr<ChatMessage>>::iterator it=ephemeralMessages.begin(); it!=ephemeralMessages.end(); ++it) {
			shared_ptr<ChatMessage> msg = *it;
			if (msg->getEphemeralExpireTime() > message->getEphemeralExpireTime()) {
				// This message will expire more quickly than the last message of this list, add it.
				if (it == ephemeralMessages.begin()) {
					ephemeralMessages.push_front(message);
					startEphemeralMessageTimer(message->getEphemeralExpireTime());
				} else {
					it = --it;
					ephemeralMessages.insert(it, message);
				}
				return;
			}
		}
	}
}

void CorePrivate::sendDeliveryNotifications () {
	L_Q();
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(q->getCCore());
	if (linphone_im_notif_policy_get_send_imdn_delivered(policy)) {
		auto chatMessages = mainDb->findChatMessagesToBeNotifiedAsDelivered();
		for (const auto &chatMessage : chatMessages) {
			chatMessage->getChatRoom()->getPrivate()->sendDeliveryNotifications(chatMessage);
		}
	}
}

void CorePrivate::replaceChatRoom (const shared_ptr<AbstractChatRoom> &replacedChatRoom, const shared_ptr<AbstractChatRoom> &newChatRoom) {
	const ConferenceId &replacedConferenceId = replacedChatRoom->getConferenceId();
	const ConferenceId &newConferenceId = newChatRoom->getConferenceId();

	if (replacedChatRoom->getCapabilities() & ChatRoom::Capabilities::Proxy) {
		chatRoomsById.erase(replacedConferenceId);
		chatRoomsById[newConferenceId] = replacedChatRoom;
	} else {
		chatRoomsById.erase(replacedConferenceId);
		chatRoomsById[newConferenceId] = newChatRoom;
	}
}

shared_ptr<AbstractChatRoom> CorePrivate::findExhumableOneToOneChatRoom (
		const IdentityAddress &localAddress,
		const IdentityAddress &participantAddress,
		bool encrypted) const {
#ifdef HAVE_ADVANCED_IM
	lInfo() << "Looking for exhumable 1-1 chat room with local address [" << localAddress.asString() << "] and participant [" << participantAddress.asString() << "]";
	
	for (auto it = chatRoomsById.begin(); it != chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		const IdentityAddress &curLocalAddress = chatRoom->getLocalAddress();
		ChatRoom::CapabilitiesMask capabilities = chatRoom->getCapabilities();
		// Don't check if terminated, it can be exhumed before the BYE has been received
		if (/*chatRoom->getState() == ChatRoom::State::Terminated
				&& */capabilities & ChatRoom::Capabilities::Conference
				&& capabilities & ChatRoom::Capabilities::OneToOne
				&& encrypted == bool(capabilities & ChatRoom::Capabilities::Encrypted)) {
			if (localAddress.getAddressWithoutGruu() == curLocalAddress.getAddressWithoutGruu()
					&& participantAddress.getAddressWithoutGruu() == chatRoom->getParticipants().front()->getAddress().getAddressWithoutGruu()) {
				return chatRoom;
			}
		}
	}

	lInfo() << "Unable to find exhumable 1-1 chat room with local address [" << localAddress.asString() << "] and participant [" << participantAddress.asString() << "]";
#endif
	return nullptr;
}

shared_ptr<AbstractChatRoom> CorePrivate::findExumedChatRoomFromPreviousConferenceId(const ConferenceId conferenceId) const {
#ifdef HAVE_ADVANCED_IM
	for (auto it = chatRoomsById.begin(); it != chatRoomsById.end(); it++) {
		const shared_ptr<AbstractChatRoom> &chatRoom = it->second;
		ChatRoom::CapabilitiesMask capabilities = chatRoom->getCapabilities();

		// We are looking for a one to one chatroom which isn't basic
		if ((capabilities & ChatRoom::Capabilities::Basic) || !(capabilities & ChatRoom::Capabilities::OneToOne))
			continue;
		
		const shared_ptr<ClientGroupChatRoom> &clientGroupChatRoom = dynamic_pointer_cast<ClientGroupChatRoom>(chatRoom);
		// Check it isn't a ServerGroupChatRoom
		if (clientGroupChatRoom) {
			const list<ConferenceId>& previousIds = clientGroupChatRoom->getPrivate()->getPreviousConferenceIds();
			auto prevIdIt = find(previousIds.begin(), previousIds.end(), conferenceId);
			if (prevIdIt != previousIds.cend()) {
				return chatRoom;
			}
		}
	}
#endif
	return nullptr;
}

void CorePrivate::updateChatRoomConferenceId (const shared_ptr<AbstractChatRoom> &chatRoom, ConferenceId oldConferenceId) {
#ifdef HAVE_ADVANCED_IM
	const ConferenceId &newConferenceId = chatRoom->getConferenceId();
	lInfo() << "Chat room [" << oldConferenceId << "] has been exhumed into [" << newConferenceId << "]";

	chatRoomsById.erase(oldConferenceId);
	chatRoomsById[newConferenceId] = chatRoom;

	mainDb->updateChatRoomConferenceId(oldConferenceId, newConferenceId);
#endif
}

// -----------------------------------------------------------------------------

static bool compare_chat_room (const shared_ptr<AbstractChatRoom>& first, const shared_ptr<AbstractChatRoom>& second) {
	return first->getLastUpdateTime() > second->getLastUpdateTime();
}

//
string Core::getConferenceFactoryUri(const shared_ptr<Core> &core, const IdentityAddress &localAddress) {
	Address addr(localAddress.asAddress());
	LinphoneProxyConfig *proxy = linphone_core_lookup_proxy_by_identity(core->getCCore(), L_GET_C_BACK_PTR(&addr));

	if (!proxy) {
		lWarning() << "No proxy configuration found for local address: [" << localAddress.asString() << "]";
		return string();
	}
	const char *uri = linphone_proxy_config_get_conference_factory_uri(proxy);
	if (uri) {
		return uri;
	}
	return string();
}

// -----------------------------------------------------------------------------

list<shared_ptr<AbstractChatRoom>> Core::getChatRooms () const {
	L_D();

	LinphoneCore *lc = getCCore();
	LinphoneConfig *config = linphone_core_get_config(lc);
	bool hideEmptyChatRooms = !!linphone_config_get_int(config, "misc", "hide_empty_chat_rooms", 1);
	bool hideChatRoomsFromRemovedProxyConfig = !!linphone_config_get_int(config, "misc", "hide_chat_rooms_from_removed_proxies", 1);

	list<shared_ptr<AbstractChatRoom>> rooms;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		if (hideEmptyChatRooms) {
			if (chatRoom->isEmpty() && (chatRoom->getCapabilities() & LinphoneChatRoomCapabilitiesOneToOne)) {
				continue;
			}
		}

		if (hideChatRoomsFromRemovedProxyConfig) {
			const bctbx_list_t *it;
			bool found = false;
			for (it = linphone_core_get_proxy_config_list(lc); it != NULL; it = it->next) {
				LinphoneProxyConfig *cfg = (LinphoneProxyConfig *)it->data;
				const LinphoneAddress *identityAddr = linphone_proxy_config_get_identity_address(cfg);
				if (L_GET_CPP_PTR_FROM_C_OBJECT(identityAddr)->weakEqual(chatRoom->getLocalAddress().asAddress())) {
					found = true;
					break;
				}
			}
			if (!found) {
				continue;
			}
		}

		rooms.push_front(chatRoom);
	}

	rooms.sort(compare_chat_room);
	return rooms;
}

shared_ptr<AbstractChatRoom> Core::findChatRoom (const ConferenceId &conferenceId, bool logIfNotFound) const {
	L_D();

	auto it = d->chatRoomsById.find(conferenceId);
	if (it != d->chatRoomsById.cend()) {
		lDebug() << "Found chat room in RAM for conference ID " << conferenceId << ".";
		return it->second;
	}

	auto alreadyExhumedOneToOne = d->findExumedChatRoomFromPreviousConferenceId(conferenceId);
	if (alreadyExhumedOneToOne) {
		lWarning() << "Found conference id as already exhumed chat room with new conference ID " << alreadyExhumedOneToOne->getConferenceId() << ".";
		return alreadyExhumedOneToOne;
	}

	if (logIfNotFound)
		lInfo() << "Unable to find chat room in RAM: " << conferenceId << ".";
	
	return nullptr;
}

list<shared_ptr<AbstractChatRoom>> Core::findChatRooms (const IdentityAddress &peerAddress) const {
	L_D();

	list<shared_ptr<AbstractChatRoom>> output;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		if (chatRoom->getPeerAddress() == peerAddress) {
			output.push_front(chatRoom);
		}
	}

	return output;
}

shared_ptr<AbstractChatRoom> Core::findOneToOneChatRoom (
	const IdentityAddress &localAddress,
	const IdentityAddress &participantAddress,
	bool basicOnly,
	bool conferenceOnly,
	bool encrypted
) const {
	L_D();
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		const IdentityAddress &curLocalAddress = chatRoom->getLocalAddress();
		ChatRoom::CapabilitiesMask capabilities = chatRoom->getCapabilities();

		// We are looking for a one to one chatroom
		// Do not return a group chat room that everyone except one person has left
		if (!(capabilities & ChatRoom::Capabilities::OneToOne))
			continue;

		if (encrypted != bool(capabilities & ChatRoom::Capabilities::Encrypted))
			continue;

		// One to one client group chat room
		// The only participant's address must match the participantAddress argument
		if (
			!basicOnly &&
		        (capabilities & ChatRoom::Capabilities::Conference) &&
			!chatRoom->getParticipants().empty() &&
			localAddress.getAddressWithoutGruu() == curLocalAddress.getAddressWithoutGruu() &&
			participantAddress.getAddressWithoutGruu() == chatRoom->getParticipants().front()->getAddress()
		)
			return chatRoom;

		// One to one basic chat room (addresses without gruu)
		// The peer address must match the participantAddress argument
		if (
			!conferenceOnly && 
				(capabilities & ChatRoom::Capabilities::Basic) &&
			localAddress.getAddressWithoutGruu() == curLocalAddress.getAddressWithoutGruu() &&
			participantAddress.getAddressWithoutGruu() == chatRoom->getPeerAddress().getAddressWithoutGruu()
		)
			return chatRoom;
	}
	return nullptr;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom (const ConferenceId &conferenceId) {
	L_D();

	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	if (chatRoom) {
		return chatRoom;
	}
	ChatRoom::CapabilitiesMask capabilities(ChatRoom::Capabilities::OneToOne);
	if (d->basicToFlexisipChatroomMigrationEnabled()) {
		capabilities |= ChatRoom::Capabilities::Migratable;
	}
	chatRoom = d->createBasicChatRoom(conferenceId, capabilities, ChatRoomParams::fromCapabilities(capabilities));
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom (const IdentityAddress &peerAddress) {
	L_D();

	list<shared_ptr<AbstractChatRoom>> chatRooms = findChatRooms(peerAddress);
	if (!chatRooms.empty()) {
		shared_ptr<AbstractChatRoom> ret = chatRooms.front();
		return ret;
	}

	ChatRoom::CapabilitiesMask capabilities(ChatRoom::Capabilities::OneToOne);
	if (d->basicToFlexisipChatroomMigrationEnabled()) {
		capabilities |= ChatRoom::Capabilities::Migratable;
	}
	shared_ptr<AbstractChatRoom> chatRoom = d->createBasicChatRoom(
	       ConferenceId(peerAddress, d->getDefaultLocalAddress(&peerAddress, false)),
	       capabilities,
	       ChatRoomParams::fromCapabilities(capabilities)
	);
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoomFromUri (const string &peerAddress) {
	Address address(interpretUrl(peerAddress));
	if (!address.isValid()) {
		lError() << "Cannot make a valid address with: `" << peerAddress << "`.";
		return nullptr;
	}
	return getOrCreateBasicChatRoom(address);
}

void Core::deleteChatRoom (const shared_ptr<const AbstractChatRoom> &chatRoom) {
	CorePrivate *d = chatRoom->getCore()->getPrivate();

	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	lInfo() << "Trying to delete chat room with conference ID " << conferenceId << ".";

	d->noCreatedClientGroupChatRooms.erase(chatRoom.get());
	auto chatRoomsByIdIt = d->chatRoomsById.find(conferenceId);
	if (chatRoomsByIdIt != d->chatRoomsById.end()) {
		d->chatRoomsById.erase(chatRoomsByIdIt);
		if (d->mainDb->isInitialized()) d->mainDb->deleteChatRoom(conferenceId);
	} else {
		lError() << "Unable to delete chat room with conference ID " << conferenceId << " because it cannot be found.";
	}
}

LINPHONE_END_NAMESPACE
