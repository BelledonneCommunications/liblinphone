/*
 * core-chat-room.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <iterator>

#include "linphone/utils/algorithm.h"

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/chat-room/basic-chat-room.h"
#include "chat/chat-room/chat-room-p.h"
#include "chat/chat-room/real-time-text-chat-room.h"
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
static IdentityAddress getDefaultLocalAddress(const shared_ptr<Core> &core, const IdentityAddress *peerAddress, bool withGruu) {
	LinphoneCore *cCore = core->getCCore();
	LinphoneProxyConfig *proxy = nullptr;

	if (peerAddress) {
		LinphoneAddress *cPeerAddress = linphone_address_new(peerAddress->asString().c_str());
		proxy = linphone_core_lookup_known_proxy(cCore, cPeerAddress);
		linphone_address_unref(cPeerAddress);
	} else {
		proxy =	linphone_core_get_default_proxy_config(cCore);
	}

	IdentityAddress localAddress;
	if (proxy) {
		char *identity = linphone_address_as_string(
			withGruu ? linphone_proxy_config_get_contact(proxy) : linphone_proxy_config_get_identity_address(proxy));
		localAddress = IdentityAddress(identity);
		bctbx_free(identity);
	} else
		localAddress = IdentityAddress(linphone_core_get_primary_contact(cCore));

	return localAddress;
}

//
static string getConferenceFactoryUri(const shared_ptr<Core> &core, const IdentityAddress &localAddress) {
	Address	addr(localAddress);
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
		lError() << "createClientGroupChatRoom(): local address must have a gruu.";
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
	chatRoom->getPrivate()->setState(ChatRoom::State::Instantiated);
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

	shared_ptr<ChatRoomParams> params = ChatRoomParams::create(encrypted, true, ChatRoomParams::ChatRoomBackend::FlexisipChat);
	shared_ptr<ClientGroupChatRoom> clientGroupChatRoom(new ClientGroupChatRoom(
										    q->getSharedFromThis(),
										    conferenceId.getPeerAddress(),
										    conferenceId,
										    subject,
										    content,
										    ChatRoomParams::toCapabilities(params),
										    params
										    ));

	clientGroupChatRoom->getPrivate()->setState(ChatRoom::State::Instantiated);
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

	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(q->getSharedFromThis(), nullptr, true);
	IdentityAddress conferenceFactoryUri(getConferenceFactoryUri(q->getSharedFromThis(), defaultLocalAddress));
	shared_ptr<ChatRoomParams> params = ChatRoomParams::create(encrypted, !fallback, ChatRoomParams::ChatRoomBackend::FlexisipChat);

	return createClientGroupChatRoom(subject, conferenceFactoryUri, ConferenceId(IdentityAddress(), defaultLocalAddress), Content(), ChatRoomParams::toCapabilities(params), params, fallback);
}

shared_ptr<AbstractChatRoom> CorePrivate::createBasicChatRoom (
	const ConferenceId &conferenceId,
	ChatRoom::CapabilitiesMask capabilities,
	const shared_ptr<ChatRoomParams> &params
) {
	L_Q();

	shared_ptr<AbstractChatRoom> chatRoom;
	if (capabilities & ChatRoom::Capabilities::RealTimeText)
		chatRoom.reset(new RealTimeTextChatRoom(q->getSharedFromThis(), conferenceId, params));
	else {
		BasicChatRoom *basicChatRoom = new BasicChatRoom(q->getSharedFromThis(), conferenceId, params);
		string conferenceFactoryUri = getConferenceFactoryUri(q->getSharedFromThis(), conferenceId.getLocalAddress());
		if (basicToFlexisipChatroomMigrationEnabled()) {
			capabilities.set(ChatRoom::Capabilities::Migratable);
		}else{
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
	}
	AbstractChatRoomPrivate *dChatRoom = chatRoom->getPrivate();
	dChatRoom->setState(ChatRoom::State::Instantiated);
	dChatRoom->setState(ChatRoom::State::Created);

	return chatRoom;
}


shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddr, const std::string &subject, const std::list<IdentityAddress> &participants) {
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
		string conferenceFactoryUri = getConferenceFactoryUri(q->getSharedFromThis(), localAddr);
		if (conferenceFactoryUri.empty()) {
			lWarning() << "Not creating group chat room: no conference factory uri for local address [" << localAddr << "]";
			return nullptr;
		}

		chatRoom = createClientGroupChatRoom(subject,
						     IdentityAddress(conferenceFactoryUri),
						     ConferenceId(IdentityAddress(), localAddr),
						     Content(),
						     ChatRoomParams::toCapabilities(params),
						     params,
						     false);
		chatRoom->addParticipants(participants, nullptr, false);
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
		
		chatRoom = createBasicChatRoom(ConferenceId(IdentityAddress(participants.front()), localAddr),
					       capabilities,
					       params);
		insertChatRoom(chatRoom);
		insertChatRoomWithDb(chatRoom);
	}
	return chatRoom;
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ChatRoomParams> &params, const std::string &subject, const std::list<IdentityAddress> &participants) {
	L_Q();
	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(q->getSharedFromThis(), nullptr,
				params->getChatRoomBackend() == ChatRoomParams::ChatRoomBackend::FlexisipChat);
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
	IdentityAddress defaultLocalAddress = getDefaultLocalAddress(q->getSharedFromThis(), nullptr,
					params->getChatRoomBackend() == ChatRoomParams::ChatRoomBackend::FlexisipChat);
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
		chatRoomsById[conferenceId] = chatRoom;
	}
}

void CorePrivate::insertChatRoomWithDb (const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
	L_ASSERT(chatRoom->getState() == ChatRoom::State::Created);
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

void CorePrivate::initMessageKillers () {
	if (!mainDb->isInitialized()) return;
	mainDb->updateEphemeralMessageKillers(messageKillers);
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

// -----------------------------------------------------------------------------

static bool compare_chat_room (const shared_ptr<AbstractChatRoom>& first, const shared_ptr<AbstractChatRoom>& second) {
	return first->getLastUpdateTime() > second->getLastUpdateTime();
}

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
				if (L_GET_CPP_PTR_FROM_C_OBJECT(identityAddr)->weakEqual(chatRoom->getLocalAddress())) {
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
	if (it != d->chatRoomsById.cend())
		return it->second;

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
			localAddress == curLocalAddress &&
			participantAddress.getAddressWithoutGruu() == chatRoom->getParticipants().front()->getAddress()
		)
			return chatRoom;

		// One to one basic chat room (addresses without gruu)
		// The peer address must match the participantAddress argument
		if (
			(capabilities & ChatRoom::Capabilities::Basic) &&
			localAddress.getAddressWithoutGruu() == curLocalAddress.getAddressWithoutGruu() &&
			participantAddress.getAddressWithoutGruu() == chatRoom->getPeerAddress().getAddressWithoutGruu()
		)
			return chatRoom;
	}
	return nullptr;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom (const ConferenceId &conferenceId, bool isRtt) {
	L_D();

	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	if (chatRoom) {
		if (isRtt && !(chatRoom->getCapabilities() & ChatRoom::Capabilities::RealTimeText)) {
			lError() << "Found chatroom but without RealTimeText capability. This is a bug, fixme";
			return nullptr;
		}
		return chatRoom;
	}
	ChatRoom::CapabilitiesMask capabilities(ChatRoom::Capabilities::OneToOne);
	if (isRtt) {
		capabilities |=	ChatRoom::Capabilities::RealTimeText;
	}
	if (d->basicToFlexisipChatroomMigrationEnabled()) {
		capabilities |= ChatRoom::Capabilities::Migratable;
	}
	chatRoom = d->createBasicChatRoom(conferenceId, capabilities, ChatRoomParams::fromCapabilities(capabilities));
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom (const IdentityAddress &peerAddress, bool isRtt) {
	L_D();

	list<shared_ptr<AbstractChatRoom>> chatRooms = findChatRooms(peerAddress);
	if (!chatRooms.empty()) {
		shared_ptr<AbstractChatRoom> ret = chatRooms.front();
		if (isRtt && !(ret->getCapabilities() & ChatRoom::Capabilities::RealTimeText)) {
			lError() << "Found chatroom but without RealTimeText capability. This is a bug, fixme";
			ret = nullptr;
		}
		return ret;
	}

	ChatRoom::CapabilitiesMask capabilities(ChatRoom::Capabilities::OneToOne);
	if (isRtt) {
		capabilities |=	ChatRoom::Capabilities::RealTimeText;
	}
	if (d->basicToFlexisipChatroomMigrationEnabled()) {
		capabilities |= ChatRoom::Capabilities::Migratable;
	}
	shared_ptr<AbstractChatRoom> chatRoom = d->createBasicChatRoom(
	       ConferenceId(peerAddress, getDefaultLocalAddress(getSharedFromThis(), &peerAddress, false)),
	       capabilities,
	       ChatRoomParams::fromCapabilities(capabilities)
	);
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoomFromUri (const string &peerAddress, bool isRtt) {
	Address address(interpretUrl(peerAddress));
	if (!address.isValid()) {
		lError() << "Cannot make a valid address with: `" << peerAddress << "`.";
		return nullptr;
	}
	return getOrCreateBasicChatRoom(address, isRtt);
}

void Core::deleteChatRoom (const shared_ptr<const AbstractChatRoom> &chatRoom) {
	CorePrivate *d = chatRoom->getCore()->getPrivate();

	d->noCreatedClientGroupChatRooms.erase(chatRoom.get());
	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	auto chatRoomsByIdIt = d->chatRoomsById.find(conferenceId);
	if (chatRoomsByIdIt != d->chatRoomsById.end()) {
		d->chatRoomsById.erase(chatRoomsByIdIt);
		if (d->mainDb->isInitialized()) d->mainDb->deleteChatRoom(conferenceId);
	}
}

LINPHONE_END_NAMESPACE
