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

#include <iterator>

#include <bctoolbox/defs.h>

#include "linphone/utils/algorithm.h"

#include "address/address.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/chat-room/basic-chat-room.h"
#include "chat/chat-room/chat-room.h"
#include "conference/conference-context.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/server-conference.h"
#include "core-p.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-chat-message-cbs.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room.h"
#include "logger/logger.h"

#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-chat-room.h"
#include "conference/handlers/client-conference-list-event-handler.h"
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
 * If withGruu is true, returns the local address with its gruu parameter. FlexisipChat kind of chatroom (also referred
 * as ClientChatRoom) require a local address with gruu, unlike basic chatrooms.
 */
std::shared_ptr<const Address> CorePrivate::getDefaultLocalAddress(const std::shared_ptr<const Address> peerAddress,
                                                                   bool withGruu) const {
	L_Q();
	std::shared_ptr<Account> account = nullptr;

	if (peerAddress) {
		account = q->lookupKnownAccount(peerAddress, true);
	}

	if (!account) {
		account = q->getDefaultAccount();
	}

	std::shared_ptr<const Address> localAddress = nullptr;
	if (account) {
		localAddress = (withGruu && account->getContactAddress()) ? account->getContactAddress()
		                                                          : account->getAccountParams()->getIdentityAddress();
	} else {
		LinphoneCore *cCore = getCCore();
		localAddress = Address::create(linphone_core_get_primary_contact(cCore));
	}
	return localAddress;
}

std::shared_ptr<const Address>
CorePrivate::getIdentityAddressWithGruu(const std::shared_ptr<const Address> &identityAddress) const {
	L_Q();
	std::shared_ptr<const Address> identityAddressWithGruu;

	if (identityAddress && identityAddress->isValid()) {
		const auto &account = q->lookupKnownAccount(identityAddress, true);
		if (account) {
			identityAddressWithGruu = account->getContactAddress();
		}
	}

	return identityAddressWithGruu;
}

// -----------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
// Base server group chat room creator
shared_ptr<AbstractChatRoom>
CorePrivate::createServerChatRoom(BCTBX_UNUSED(const std::shared_ptr<const Address> &conferenceFactoryUri),
                                  SalCallOp *op,
                                  const std::shared_ptr<ConferenceParams> &params) {
#ifdef HAVE_ADVANCED_IM
	L_Q();
	if (!params || !params->isValid()) {
		lWarning() << "Invalid chat room parameters given for server group chat room creation";
		return nullptr;
	}

	LinphoneCore *cCore = getCCore();
	auto newConferenceParameters = params->clone()->toSharedPtr();
	newConferenceParameters->enableChat(true);
	const auto localAddr = Address::create(op->getTo());
	if (!newConferenceParameters->getAccount() && localAddr && localAddr->isValid()) {
		LinphoneAccount *account = linphone_core_lookup_known_account(cCore, localAddr->toC());
		if (account) {
			newConferenceParameters->setAccount(Account::toCpp(account)->getSharedFromThis());
		}
	}

	auto conference = dynamic_pointer_cast<ServerConference>(
	    (new ServerConference(q->getSharedFromThis(), nullptr, newConferenceParameters))->toSharedPtr());
	conference->init(op, conference.get());
	return conference->getChatRoom();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return nullptr;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
// Base client group chat room creator
shared_ptr<AbstractChatRoom>
CorePrivate::createClientChatRoom(const std::shared_ptr<const Address> &conferenceFactoryUri,
                                  const ConferenceId &conferenceId,
                                  SalCallOp *op,
                                  const std::shared_ptr<ConferenceParams> &params) {
#ifdef HAVE_ADVANCED_IM
	L_Q();
	if (!params || !params->isValid()) {
		lWarning() << "Invalid chat room parameters given for client group chat room creation";
		return nullptr;
	}
	LinphoneCore *cCore = getCCore();
	auto newConferenceParameters = params->clone()->toSharedPtr();
	newConferenceParameters->enableChat(true);
	newConferenceParameters->getChatParams()->enableEphemeral(
	    (params->getChatParams()->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) &&
	    (params->getChatParams()->getEphemeralLifetime() > 0));
	const auto &localAddr = conferenceId.getLocalAddress();
	if (!newConferenceParameters->getAccount() && localAddr && localAddr->isValid()) {
		LinphoneAccount *account = linphone_core_lookup_known_account(cCore, localAddr->toC());
		if (account) {
			newConferenceParameters->setAccount(Account::toCpp(account)->getSharedFromThis());
		}
	}

	auto conference = dynamic_pointer_cast<ClientConference>(
	    (new ClientConference(q->getSharedFromThis(), nullptr, newConferenceParameters))->toSharedPtr());
	conference->initWithFocus(conferenceFactoryUri, nullptr, op, conference.get());
	if (conferenceId.isValid()) {
		conference->setConferenceId(conferenceId);
	}
	lInfo() << *conference << " with id " << conferenceId
	        << " and chat only capabilities has been successfully created";
	return conference->getChatRoom();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return nullptr;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
shared_ptr<AbstractChatRoom> CorePrivate::createClientChatRoom(const string &subject,
                                                               const ConferenceId &conferenceId,
                                                               SalCallOp *op,
                                                               bool encrypted,
                                                               AbstractChatRoom::EphemeralMode ephemerableMode,
                                                               long ephemeralLifeTime) {
#ifdef HAVE_ADVANCED_IM
	L_Q();
	shared_ptr<ConferenceParams> params = ConferenceParams::create(q->getSharedFromThis());
	params->setUtf8Subject(subject);
	params->setSecurityLevel(encrypted ? ConferenceParams::SecurityLevel::EndToEnd
	                                   : ConferenceParams::SecurityLevel::None);
	params->setGroup(true);
	params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);
	params->getChatParams()->setEphemeralMode(ephemerableMode);
	params->getChatParams()->setEphemeralLifetime(ephemeralLifeTime);
	return createClientChatRoom(conferenceId.getPeerAddress(), conferenceId, op, params);
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return nullptr;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

// From deprecated public API
shared_ptr<AbstractChatRoom> CorePrivate::createClientChatRoom(const string &subject, bool fallback, bool encrypted) {
	L_Q();

	auto defaultLocalAddress = getDefaultLocalAddress(nullptr, true);
	const auto &conferenceFactoryUri = Core::getConferenceFactoryAddress(q->getSharedFromThis(), defaultLocalAddress);
	shared_ptr<ConferenceParams> params = ConferenceParams::create(q->getSharedFromThis());
	params->setChatDefaults();
	params->setUtf8Subject(subject);
	params->setSecurityLevel(encrypted ? ConferenceParams::SecurityLevel::EndToEnd
	                                   : ConferenceParams::SecurityLevel::None);
	params->setGroup(!fallback);
	params->getChatParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged);
	params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);

	return createClientChatRoom(conferenceFactoryUri,
	                            ConferenceId(nullptr, defaultLocalAddress, q->createConferenceIdParams()), nullptr,
	                            params);
}

shared_ptr<AbstractChatRoom> CorePrivate::createBasicChatRoom(const ConferenceId &conferenceId,
                                                              const shared_ptr<ConferenceParams> &params) {
	L_Q();

	LinphoneCore *cCore = getCCore();
	auto newConferenceParameters = params->clone()->toSharedPtr();
	newConferenceParameters->enableChat(true);
	const auto &localAddr = conferenceId.getLocalAddress();
	if (!newConferenceParameters->getAccount() && localAddr && localAddr->isValid()) {
		LinphoneAccount *account = linphone_core_lookup_known_account(cCore, localAddr->toC());
		if (account) {
			newConferenceParameters->setAccount(Account::toCpp(account)->getSharedFromThis());
		}
	}

	shared_ptr<AbstractChatRoom> chatRoom;
	shared_ptr<AbstractChatRoom> basicChatRoom =
	    (new BasicChatRoom(q->getSharedFromThis(), conferenceId, newConferenceParameters))->toSharedPtr();
	chatRoom = basicChatRoom;
	chatRoom->setState(ConferenceInterface::State::Instantiated);
	chatRoom->setState(ConferenceInterface::State::Created);

	return chatRoom;
}

std::shared_ptr<AbstractChatRoom>
CorePrivate::searchChatRoom(const std::shared_ptr<ConferenceParams> &params,
                            const std::shared_ptr<const Address> &localAddress,
                            const std::shared_ptr<const Address> &remoteAddress,
                            const std::list<std::shared_ptr<Address>> &participants) const {
	L_Q();
	ConferenceContext referenceConferenceContext(params, localAddress, remoteAddress, participants);
	const auto &chatRooms = q->getRawChatRoomList();
	const auto it = std::find_if(chatRooms.begin(), chatRooms.end(), [&](const auto &chatRoom) {
		ConferenceContext conferenceContext(chatRoom->getCurrentParams(), chatRoom->getLocalAddress(),
		                                    chatRoom->getPeerAddress(), chatRoom->getParticipantAddresses());
		return (referenceConferenceContext == conferenceContext);
	});

	std::shared_ptr<AbstractChatRoom> chatRoom;
	if (it != chatRooms.cend()) {
		chatRoom = *it;
	}
	return chatRoom;
}

std::shared_ptr<AbstractChatRoom> CorePrivate::searchChatRoom(const std::string identifier) const {
	auto [localAddress, peerAddress] = ConferenceId::parseIdentifier(identifier);
	if (!localAddress || !localAddress->isValid() || !peerAddress || !peerAddress->isValid()) {
		return nullptr;
	}
	return searchChatRoom(nullptr, localAddress, peerAddress, {});
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ConferenceParams> &params,
                                                         const std::list<std::shared_ptr<Address>> &participants) {
	L_Q();
	if (!params) {
		lWarning() << "Trying to create chat room with null parameters";
		return nullptr;
	}
	if (!params->chatEnabled()) {
		lWarning() << "Trying to create chat room when the chat capability is disabled in the conference parameters";
		return nullptr;
	}
	if (!params->isValid()) {
		lWarning() << "Trying to create chat room with invalid parameters " << params->toString();
		return nullptr;
	}
	if (!linphone_factory_is_chatroom_backend_available(
	        linphone_factory_get(), static_cast<LinphoneChatRoomBackend>(params->getChatParams()->getBackend()))) {
		lWarning() << "Tying to create chat room with unavailable backend";
		return nullptr;
	}

	auto account = params->getAccount();
	if (!account) {
		account = q->getDefaultAccount();
	}
	const std::shared_ptr<const Address> localAddr =
	    account ? account->getAccountParams()->getIdentityAddress() : getDefaultLocalAddress(nullptr, false);

	shared_ptr<AbstractChatRoom> chatRoom;
	if (params->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat) {
#ifdef HAVE_ADVANCED_IM
		const auto &conferenceFactoryUri = account->getAccountParams()->getConferenceFactoryAddress();
		if (!conferenceFactoryUri || !conferenceFactoryUri->isValid()) {
			lWarning() << "Not creating group chat room: no conference factory uri for local address [" << *localAddr
			           << "]";
			return nullptr;
		}

		if (!params->isGroup() && participants.size() > 0) {
			chatRoom = searchChatRoom(params, localAddr, nullptr, participants);
			if (chatRoom) {
				lWarning() << "Found already existing 1-1 chat room that matches the given parameters, using this one "
				           << *chatRoom;
				return chatRoom;
			}
		}

		ConferenceId conferenceId(nullptr, localAddr, q->createConferenceIdParams());
		chatRoom = createClientChatRoom(conferenceFactoryUri, conferenceId, nullptr, params);
		if (!chatRoom) {
			lWarning() << "Cannot create createClientChatRoom with subject [" << params->getSubject() << "]";
			return nullptr;
		}

		if (chatRoom->getConference()->inviteAddresses(participants, nullptr) != 0) {
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

		std::shared_ptr<const Address> remoteAddr = participants.front();
		chatRoom = searchChatRoom(params, localAddr, remoteAddr, {});
		if (chatRoom == nullptr) {
			std::shared_ptr<const Address> remoteAddr = participants.front();
			chatRoom = createBasicChatRoom(ConferenceId(remoteAddr, localAddr, q->createConferenceIdParams()), params);
			insertChatRoom(chatRoom);
			insertChatRoomWithDb(chatRoom);
		} else {
			lInfo() << "Found an existing BasicChatRoom with " << *remoteAddr
			        << ", using it instead of creating a new one";
		}
	}
	return chatRoom;
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const std::string &subject,
                                                         const std::list<std::shared_ptr<Address>> &participants) {
	L_Q();
	shared_ptr<ConferenceParams> params = ConferenceParams::create(q->getSharedFromThis());
	params->setChatDefaults();
	if (participants.size() > 1) {
		// Try to infer chat room type based on requested participants number
		params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);
	} else {
		params->getChatParams()->setBackend(ChatParams::Backend::Basic);
	}
	params->setAccount(q->getDefaultAccount());
	params->setUtf8Subject(subject);
	return createChatRoom(params, participants);
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ConferenceParams> &params,
                                                         const std::shared_ptr<Address> &participant) {
	const std::list<std::shared_ptr<Address>> participants{participant};
	return createChatRoom(params, participants);
}

// Assume basic chat room creation
shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const std::shared_ptr<Address> &participant) {
	return createChatRoom("", {participant});
}

void CorePrivate::insertChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_ASSERT(chatRoom);
	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	auto it = mChatRoomsById.find(conferenceId);
	L_ASSERT(it == mChatRoomsById.end() || it->second == chatRoom);
	if (it == mChatRoomsById.end()) {
		// Remove chat room from workaround cache.
		if (linphone_core_get_global_state(getCCore()) != LinphoneGlobalStartup) {
			lInfo() << "Insert chat room " << chatRoom << " (id " << conferenceId << ") to core map";
		}
		mChatRoomsById[conferenceId] = chatRoom;
	}
}

void CorePrivate::insertChatRoomWithDb(const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
	const auto chatRoomState = chatRoom->getState();
	if (mainDb->isInitialized() && ((chatRoomState == ConferenceInterface::State::CreationPending) ||
	                                (chatRoomState == ConferenceInterface::State::Created))) {
		mainDb->insertChatRoom(chatRoom, notifyId);
	}
}

void CorePrivate::loadChatRooms() {
	mChatRoomsById.clear();
#ifdef HAVE_ADVANCED_IM
	if (clientListEventHandler) clientListEventHandler->clearHandlers();
#endif
	if (!mainDb->isInitialized()) return;
	lInfo() << "Beginning loadChatRooms";
	std::set<Address, Address::WeakLess> friendAddresses;
	std::list<pair<shared_ptr<Address>, string>> deviceAddressesAndNames;
	for (auto &chatRoom : mainDb->getChatRooms()) {
		const auto &chatRoomParams = chatRoom->getCurrentParams();
		// We are looking for a one to one chatroom which isn't basic
		if (chatRoomParams->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::Basic) {
			insertChatRoom(chatRoom);
		} else {
			const auto &conference = chatRoom->getConference();
			const ConferenceId &conferenceId = conference->getConferenceId();
			mConferenceById.insert(std::make_pair(conferenceId, conference));
		}

		// TODO FIXME: Remove later when devices for friends will be notified through presence

		for (const auto &p : chatRoom->getParticipants()) {
			const auto &pAddress = p->getAddress();
			auto [it, success] = friendAddresses.insert(*pAddress);
			if (success) {
				for (const auto &d : p->getDevices()) {
					auto gruu = d->getAddress();
					auto &name = d->getName();
					deviceAddressesAndNames.push_back(make_pair(gruu, name));
					// lDebug() << "[Friend] Inserting existing device of participant [" << *pAddress << "] with name ["
					//          << name << "] and address [" << *gruu << "]";
				}
			}
		}
	}
	mainDb->insertDevices(deviceAddressesAndNames);
	lInfo() << "End loadChatRooms";
	sendDeliveryNotifications();
}

void CorePrivate::handleEphemeralMessages(time_t currentTime) {
	if (!ephemeralMessages.empty()) {
		shared_ptr<ChatMessage> msg = ephemeralMessages.front();
		time_t expireTime = msg->getEphemeralExpireTime();
		if (currentTime > expireTime) {
			shared_ptr<LinphonePrivate::EventLog> event =
			    LinphonePrivate::MainDb::getEvent(mainDb, msg->getStorageId());
			shared_ptr<AbstractChatRoom> chatRoom = msg->getChatRoom();
			if (chatRoom && event) {
				LinphonePrivate::EventLog::deleteFromDatabase(event);
				lInfo() << "[Ephemeral] Message " << msg << " (call ID " << msg->getPrivate()->getCallId()
				        << ") deleted from database";

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
				LinphoneChatRoom *cr = chatRoom->toC();
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

void CorePrivate::initEphemeralMessages() {
	L_Q();
	if (mainDb && mainDb->isInitialized()) {
		ephemeralMessages.clear();
		ephemeralMessages = mainDb->getEphemeralMessages();
		if (!ephemeralMessages.empty()) {
			lInfo() << "[Ephemeral] list initiated on core " << linphone_core_get_identity(q->getCCore());
			shared_ptr<ChatMessage> msg = ephemeralMessages.front();
			startEphemeralMessageTimer(msg->getEphemeralExpireTime());
		}
	}
}

void CorePrivate::updateEphemeralMessages(const shared_ptr<ChatMessage> &message) {
	if (ephemeralMessages.empty()) {
		// Can not determine this message will expire most quickly, so init this list.
		initEphemeralMessages();
	} else {
		shared_ptr<ChatMessage> lastmsg = ephemeralMessages.back();
		if (lastmsg->getEphemeralLifetime() < message->getEphemeralLifetime()) {
			// The last message of this list will expire more quickly than this message, can not determine this message
			// will expire most quickly in the remaining messages.
			return;
		}
		for (std::list<shared_ptr<ChatMessage>>::iterator it = ephemeralMessages.begin(); it != ephemeralMessages.end();
		     ++it) {
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

void CorePrivate::sendDeliveryNotifications() {
	L_Q();
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(q->getCCore());
	if (linphone_im_notif_policy_get_send_imdn_delivered(policy)) {
		auto chatMessages = mainDb->findChatMessagesToBeNotifiedAsDelivered();
		for (const auto &chatMessage : chatMessages) {
			chatMessage->getChatRoom()->sendDeliveryNotifications(chatMessage);
		}
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
shared_ptr<AbstractChatRoom>
CorePrivate::findExhumableOneToOneChatRoom(const std::shared_ptr<Address> &localAddress,
                                           const std::shared_ptr<Address> &participantAddress,
                                           bool encrypted) const {
#ifdef HAVE_ADVANCED_IM
	L_Q();

	lInfo() << "Looking for exhumable 1-1 chat room with local address [" << *localAddress << "] and participant ["
	        << *participantAddress << "]";
	for (const auto &chatRoom : q->getRawChatRoomList(false, true)) {
		const std::shared_ptr<Address> &curLocalAddress = chatRoom->getLocalAddress();
		const auto &chatRoomParams = chatRoom->getCurrentParams();
		if ((chatRoomParams->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::FlexisipChat) &&
		    !chatRoomParams->isGroup() && (encrypted == chatRoomParams->getChatParams()->isEncrypted())) {
			if (chatRoom->getParticipants().size() > 0 && localAddress->weakEqual(*curLocalAddress) &&
			    participantAddress->weakEqual(*chatRoom->getParticipants().front()->getAddress())) {
				return chatRoom;
			}
		}
	}

	lInfo() << "Unable to find exhumable 1-1 chat room with local address [" << localAddress->toString()
	        << "] and participant [" << participantAddress->toString() << "]";
#endif
	return nullptr;
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
shared_ptr<AbstractChatRoom>
CorePrivate::findExumedChatRoomFromPreviousConferenceId(const ConferenceId conferenceId) const {
#ifdef HAVE_ADVANCED_IM
	L_Q();
	for (const auto &chatRoom : q->getRawChatRoomList(false, true)) {
		const shared_ptr<ClientChatRoom> &clientGroupChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
		// Check it isn't a ServerChatRoom
		if (clientGroupChatRoom) {
			const list<ConferenceId> &previousIds = clientGroupChatRoom->getPreviousConferenceIds();
			auto prevIdIt = find(previousIds.begin(), previousIds.end(), conferenceId);
			if (prevIdIt != previousIds.cend()) {
				return chatRoom;
			}
		}
	}
#endif
	return nullptr;
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void CorePrivate::updateChatRoomConferenceId(const shared_ptr<AbstractChatRoom> &chatRoom,
                                             ConferenceId oldConferenceId) {
#ifdef HAVE_ADVANCED_IM
	const ConferenceId &newConferenceId = chatRoom->getConferenceId();
	lInfo() << "Chat room [" << oldConferenceId << "] has been exhumed into [" << newConferenceId << "]";

	mConferenceById.erase(oldConferenceId);
	mConferenceById[newConferenceId] = chatRoom->getConference();

	mainDb->updateChatRoomConferenceId(oldConferenceId, newConferenceId);
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

// -----------------------------------------------------------------------------

static bool compare_chat_room(const shared_ptr<AbstractChatRoom> &first, const shared_ptr<AbstractChatRoom> &second) {
	return first->getLastUpdateTime() > second->getLastUpdateTime();
}

std::shared_ptr<const Address> Core::getConferenceFactoryAddress(const shared_ptr<Core> &core,
                                                                 const std::shared_ptr<const Address> &localAddress) {
	auto account = core->lookupKnownAccount(localAddress, true);
	if (!account) {
		// lWarning() << "No account found for local address: [" << *localAddress << "]";
		return nullptr;
	} else return getConferenceFactoryAddress(core, account);
}

std::shared_ptr<const Address> Core::getConferenceFactoryAddress(BCTBX_UNUSED(const shared_ptr<Core> &core),
                                                                 const std::shared_ptr<Account> &account) {
	const auto &params = account->getAccountParams();
	if (params) {
		const auto &uri = params->getConferenceFactoryAddress();
		if (uri) {
			return uri;
		}
	}
	return nullptr;
}

// -----------------------------------------------------------------------------
std::list<std::shared_ptr<AbstractChatRoom>> Core::getRawChatRoomList(bool includeBasic, bool includeConference) const {
	L_D();
	std::list<std::shared_ptr<AbstractChatRoom>> chatRooms;
	if (includeConference) {
		for (const auto &[id, conference] : d->mConferenceById) {
			const auto &chatRoom = conference->getChatRoom();
			if (chatRoom) {
				chatRooms.push_back(chatRoom);
			}
		}
	}

	if (includeBasic) {
		for (const auto &chatRoomPair : d->mChatRoomsById) {
			const auto &chatRoom = chatRoomPair.second;
			if (chatRoom) {
				chatRooms.push_back(chatRoom);
			}
		}
	}
	return chatRooms;
}

void Core::updateChatRoomList() const {
	LinphoneCore *lc = getCCore();
	LinphoneConfig *config = linphone_core_get_config(lc);

	bool hideChatRoomsWithMedia = !!linphone_config_get_int(config, "chat", "hide_chat_rooms_with_media", 1);
	bool hideEmptyChatRooms = !!linphone_config_get_int(config, "misc", "hide_empty_chat_rooms", 1);

	bool hideChatRoomsFromRemovedProxyConfig =
	    !!linphone_config_get_int(config, "misc", "hide_chat_rooms_from_removed_proxies", 1);
	list<shared_ptr<const Address>> localAddresses;
	if (hideChatRoomsFromRemovedProxyConfig) {
		for (const auto &account : getAccounts()) {
			auto localAddress = account->getAccountParams()->getIdentityAddress();
			localAddresses.push_front(localAddress);
		}
	}

	list<shared_ptr<AbstractChatRoom>> rooms;

	for (const auto &chatRoom : getRawChatRoomList()) {
		const auto &chatRoomParams = chatRoom->getCurrentParams();

		if (hideChatRoomsWithMedia) {
			if (chatRoomParams->audioEnabled() || chatRoomParams->videoEnabled()) {
				continue;
			}
		}
		if (hideEmptyChatRooms) {
			if (chatRoom->isEmpty() && !chatRoomParams->isGroup()) {
				continue;
			}
		}

		if (hideChatRoomsFromRemovedProxyConfig) {
			auto chatRoomLocalAddress = chatRoom->getLocalAddress();
			const auto found = std::find_if(std::begin(localAddresses), std::end(localAddresses),
			                                [&](const auto &addr) { return addr->weakEqual(chatRoomLocalAddress); });
			if (found == std::end(localAddresses)) {
				continue;
			}
		}

		rooms.push_front(chatRoom);
	}

	rooms.sort(compare_chat_room);
	mChatRooms.mList = rooms;
}

list<shared_ptr<AbstractChatRoom>> &Core::getChatRooms() const {
	updateChatRoomList();
	return mChatRooms.mList;
}

const bctbx_list_t *Core::getChatRoomsCList() const {
	updateChatRoomList();
	return mChatRooms.getCList();
}

shared_ptr<AbstractChatRoom> Core::findChatRoom(const ConferenceId &conferenceId, bool logIfNotFound) const {
	L_D();
	auto chatRoom = d->searchChatRoom(nullptr, conferenceId.getLocalAddress(), conferenceId.getPeerAddress(), {});
	if (chatRoom) {
		lDebug() << "Found chat room in RAM for conference ID " << conferenceId << ".";
		return chatRoom;
	}

	auto alreadyExhumedOneToOne = d->findExumedChatRoomFromPreviousConferenceId(conferenceId);
	if (alreadyExhumedOneToOne) {
		lWarning() << "Found conference id as already exhumed chat room with new conference ID "
		           << alreadyExhumedOneToOne->getConferenceId() << ".";
		return alreadyExhumedOneToOne;
	}
	if (logIfNotFound) lInfo() << "Unable to find chat room in RAM: " << conferenceId << ".";
	return nullptr;
}

list<shared_ptr<AbstractChatRoom>> Core::findChatRooms(const std::shared_ptr<Address> &peerAddress) const {
	list<shared_ptr<AbstractChatRoom>> output;
	for (const auto &chatRoom : getRawChatRoomList()) {
		if (*chatRoom->getPeerAddress() == *peerAddress) {
			output.push_front(chatRoom);
		}
	}

	return output;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom(const ConferenceId &conferenceId) {
	L_D();
	ChatRoom::CapabilitiesMask capabilities({ChatRoom::Capabilities::Basic, ChatRoom::Capabilities::OneToOne});
	auto params = ConferenceParams::fromCapabilities(capabilities, getSharedFromThis());
	shared_ptr<AbstractChatRoom> chatRoom =
	    d->searchChatRoom(params, conferenceId.getLocalAddress(), conferenceId.getPeerAddress(), {});
	if (chatRoom) {
		return chatRoom;
	}
	chatRoom = d->createBasicChatRoom(conferenceId, params);
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);
	lInfo() << "Basic chat room [" << chatRoom << "] with id " << conferenceId << " has been successfully created";
	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom(const std::shared_ptr<const Address> &localAddress,
                                                            const std::shared_ptr<const Address> &peerAddress) {
	L_D();
	auto chatRoomLocalAddress =
	    (localAddress && localAddress->isValid()) ? localAddress : d->getDefaultLocalAddress(peerAddress, false);
	auto conferenceIdParams = createConferenceIdParams();
	ConferenceId conferenceId(peerAddress, chatRoomLocalAddress, conferenceIdParams);
	return getOrCreateBasicChatRoom(conferenceId);
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoomFromUri(const std::string &localAddressUri,
                                                                   const std::string &peerAddressUri) {
	std::shared_ptr<Address> peerAddress(interpretUrl(peerAddressUri, true));
	if (!peerAddress || !peerAddress->isValid()) {
		lError() << "Cannot make a valid address with: `" << peerAddressUri << "`.";
		return nullptr;
	}
	std::shared_ptr<Address> localAddress(interpretUrl(localAddressUri, true));
	if (!localAddress || !localAddress->isValid()) {
		lError() << "Cannot make a valid address with: `" << localAddressUri << "`.";
		return nullptr;
	}
	return getOrCreateBasicChatRoom(localAddress, peerAddress);
}

void Core::deleteChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom) {
	auto core = chatRoom->getCore();

	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	lInfo() << "Trying to delete chat room [" << chatRoom << "] with conference ID " << conferenceId << ".";

	auto chatRoomInCoreMap = core->findChatRoom(conferenceId, false);
	if (chatRoomInCoreMap) {
		CorePrivate *d = core->getPrivate();
		d->mConferenceById.erase(conferenceId);
		d->mChatRoomsById.erase(conferenceId);
		if (d->mainDb->isInitialized()) d->mainDb->deleteChatRoom(conferenceId);
	} else {
		lError() << "Unable to delete chat room [" << chatRoom << "] with conference ID " << conferenceId
		         << " because it cannot be found.";
	}
}

const std::string Core::groupChatVersionAsString() {
	std::ostringstream os;
	os << CorePrivate::groupChatProtocolVersion;
	return os.str();
}

const std::string Core::ephemeralVersionAsString() {
	std::ostringstream os;
	os << CorePrivate::ephemeralProtocolVersion;
	return os.str();
}

void CorePrivate::stopChatMessagesAggregationTimer() {
	L_Q();

	if (chatMessagesAggregationTimer) {
		LinphoneCore *cCore = q->getCCore();

		if (cCore && cCore->sal) {
			cCore->sal->cancelTimer(chatMessagesAggregationTimer);
		}

		belle_sip_object_unref(chatMessagesAggregationTimer);
		chatMessagesAggregationTimer = nullptr;
	}

	for (const auto &chatRoom : q->getRawChatRoomList()) {
		chatRoom->notifyAggregatedChatMessages();
	}

	chatMessagesAggregationBackgroundTask.stop();
}

bool Core::isCurrentlyAggregatingChatMessages() const {
	L_D();

	return d->chatMessagesAggregationTimer != nullptr;
}

bool Core::canAggregateChatMessages() const {
	LinphoneCore *cCore = getCCore();
	bool chatMessagesAggregationEnabled = !!linphone_core_get_chat_messages_aggregation_enabled(cCore);
	int chatMessagesAggregationDelay =
	    linphone_config_get_int(linphone_core_get_config(cCore), "sip", "chat_messages_aggregation_delay", 0);
	lDebug() << "Chat messages aggregation enabled? " << chatMessagesAggregationEnabled << " with delay "
	         << chatMessagesAggregationDelay;
	return (chatMessagesAggregationEnabled && chatMessagesAggregationDelay > 0);
}

LinphoneReason
Core::handleChatMessagesAggregation(shared_ptr<AbstractChatRoom> chatRoom, SalOp *op, const SalMessage *sal_msg) {
	L_D();
	if (canAggregateChatMessages()) {
		LinphoneCore *cCore = getCCore();
		int chatMessagesAggregationDelay =
		    linphone_config_get_int(linphone_core_get_config(cCore), "sip", "chat_messages_aggregation_delay", 0);
		if (!d->chatMessagesAggregationTimer) {
			d->chatMessagesAggregationTimer = cCore->sal->createTimer(
			    [d]() -> bool {
				    d->stopChatMessagesAggregationTimer();
				    return false; // BELLE_SIP_STOP
			    },
			    (unsigned int)chatMessagesAggregationDelay, "chat messages aggregation timeout");
		} else {
			belle_sip_source_set_timeout_int64(d->chatMessagesAggregationTimer,
			                                   (unsigned int)chatMessagesAggregationDelay);
		}
		d->chatMessagesAggregationBackgroundTask.start(getSharedFromThis());
	}

	return chatRoom->onSipMessageReceived(op, sal_msg);
}

LinphoneReason Core::onSipMessageReceived(SalOp *op, const SalMessage *sal_msg) {
	L_D();

	LinphoneCore *cCore = getCCore();
	LinphoneReason reason = LinphoneReasonNotAcceptable;
	Address peerAddress;
	Address localAddress;

	if (linphone_core_conference_server_enabled(cCore)) {
		localAddress = peerAddress = Address(op->getToAddress());
	} else {
		peerAddress = Address(op->getFromAddress());
		localAddress = Address(op->getToAddress());
	}

	ConferenceId conferenceId(std::move(peerAddress), std::move(localAddress), createConferenceIdParams());
	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId, false);
	if (chatRoom) {
		bool isBasic = (chatRoom->getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
		if (isBasic) {
			auto localAccount =
			    guessLocalAccountFromMalformedMessage(conferenceId.getLocalAddress(), conferenceId.getPeerAddress());
			if (localAccount) {
				// We have a match for the from domain and the to username.
				// We may face an IPBPX that sets the To domain to our IP address, which is
				// a terribly stupid idea.
				lWarning() << "Applying workaround to have this existing chat room assigned to a known account.";
				auto oldConfId = chatRoom->getConferenceId();
				conferenceId.setLocalAddress(localAccount->getAccountParams()->getIdentityAddress(), true);

				d->mainDb->updateChatRoomConferenceId(oldConfId, conferenceId);

				auto basicChatRoom = dynamic_pointer_cast<BasicChatRoom>(chatRoom);
				basicChatRoom->setConferenceId(conferenceId);

				d->mChatRoomsById.erase(oldConfId);
				d->mChatRoomsById[conferenceId] = chatRoom;

				updateChatRoomList();
			}
		}

		reason = handleChatMessagesAggregation(chatRoom, op, sal_msg);
	} else if (!linphone_core_conference_server_enabled(cCore)) {
		const char *session_mode = sal_custom_header_find(op->getRecvCustomHeaders(), "Session-mode");
		/* Client mode but check that it is really for basic chatroom before creating it.*/
		if (session_mode && strcasecmp(session_mode, "true") == 0) {
			lError() << "Message is received in the context of a client chatroom for which we have no context.";
			reason = LinphoneReasonNotAcceptable;
		} else {
			auto localAccount =
			    guessLocalAccountFromMalformedMessage(conferenceId.getLocalAddress(), conferenceId.getPeerAddress());
			if (localAccount) {
				// We have a match for the from domain and the to username.
				// We may face an IPBPX that sets the To domain to our IP address, which is
				// a terribly stupid idea.
				lWarning() << "Applying workaround to have this chat room assigned to a known account.";
				conferenceId.setLocalAddress(localAccount->getAccountParams()->getIdentityAddress(), true);
			}

			chatRoom = getOrCreateBasicChatRoom(conferenceId);
			if (chatRoom) {
				reason = handleChatMessagesAggregation(chatRoom, op, sal_msg);
			}
		}
	} else {
		/* Server mode but chatroom not found. */
		lError() << "The core has been configured as a conference server, therefore the chatroom must be created "
		            "beforehand.";
		reason = LinphoneReasonNotFound;
	}

	auto callId = op->getCallId();
	if (!callId.empty() && d->lastPushReceivedCallId == callId) {
		lInfo() << "Chat message Call-ID matches last push received Call-ID, stopping push background task";
		d->lastPushReceivedCallId = "";
		d->pushReceivedBackgroundTask.stop();
		static_cast<PlatformHelpers *>(cCore->platform_helper)->stopPushService();
	}

	return reason;
}

void Core::setImdnToEverybodyThreshold(const int threshold) {
	mImdnToEverybodyThreshold = threshold;
}

int Core::getImdnToEverybodyThreshold() const {
	return mImdnToEverybodyThreshold;
}

void Core::enableEmptyChatroomsDeletion(const bool enable) {
	deleteEmptyChatrooms = enable;
}

bool Core::emptyChatroomsDeletionEnabled() const {
	return deleteEmptyChatrooms;
}

unsigned int Core::getRemainingUploadFileCount() const {
	return mRemainingUploadFileCount;
}

void Core::incrementRemainingUploadFileCount() {
	LinphoneCore *cCore = getCCore();
	if ((mRemainingDownloadFileCount == 0) && (mRemainingUploadFileCount == 0)) {
		static_cast<PlatformHelpers *>(cCore->platform_helper)->startFileTransferService();
	}
	mRemainingUploadFileCount++;
	linphone_core_notify_remaining_number_of_file_transfer_changed(cCore, mRemainingDownloadFileCount,
	                                                               mRemainingUploadFileCount);
}

void Core::decrementRemainingUploadFileCount() {
	if (mRemainingUploadFileCount == 0) {
		lFatal() << "Unexpectedly reaching negative upload file count";
	}
	mRemainingUploadFileCount--;
	LinphoneCore *cCore = getCCore();
	linphone_core_notify_remaining_number_of_file_transfer_changed(cCore, mRemainingDownloadFileCount,
	                                                               mRemainingUploadFileCount);
	if ((mRemainingDownloadFileCount == 0) && (mRemainingUploadFileCount == 0)) {
		static_cast<PlatformHelpers *>(cCore->platform_helper)->stopFileTransferService();
	}
}

unsigned int Core::getRemainingDownloadFileCount() const {
	return mRemainingDownloadFileCount;
}

void Core::incrementRemainingDownloadFileCount() {
	LinphoneCore *cCore = getCCore();
	if ((mRemainingDownloadFileCount == 0) && (mRemainingUploadFileCount == 0)) {
		static_cast<PlatformHelpers *>(cCore->platform_helper)->startFileTransferService();
	}
	mRemainingDownloadFileCount++;
	linphone_core_notify_remaining_number_of_file_transfer_changed(cCore, mRemainingDownloadFileCount,
	                                                               mRemainingUploadFileCount);
}

void Core::decrementRemainingDownloadFileCount() {
	if (mRemainingDownloadFileCount == 0) {
		lFatal() << "Unexpectedly reaching negative dowload file count";
	}
	mRemainingDownloadFileCount--;
	LinphoneCore *cCore = getCCore();
	linphone_core_notify_remaining_number_of_file_transfer_changed(cCore, mRemainingDownloadFileCount,
	                                                               mRemainingUploadFileCount);
	if ((mRemainingDownloadFileCount == 0) && (mRemainingUploadFileCount == 0)) {
		static_cast<PlatformHelpers *>(cCore->platform_helper)->stopFileTransferService();
	}
}

LINPHONE_END_NAMESPACE
