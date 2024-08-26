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
shared_ptr<AbstractChatRoom> CorePrivate::createServerChatRoom(const std::shared_ptr<Address> &conferenceFactoryUri,
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
	    (new ServerConference(q->getSharedFromThis(), conferenceFactoryUri, nullptr, newConferenceParameters))
	        ->toSharedPtr());
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
shared_ptr<AbstractChatRoom> CorePrivate::createClientChatRoom(const std::shared_ptr<Address> &conferenceFactoryUri,
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
	    (new ClientConference(q->getSharedFromThis(), conferenceId.getLocalAddress(), nullptr, newConferenceParameters))
	        ->toSharedPtr());
	conference->initWithFocus(conferenceFactoryUri, nullptr, op, conference.get());
	if (conferenceId.isValid()) {
		conference->setConferenceId(conferenceId);
	}
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
	params->setSubject(subject);
	params->setSecurityLevel(encrypted ? ConferenceParams::SecurityLevel::EndToEnd
	                                   : ConferenceParams::SecurityLevel::None);
	params->setGroup(!fallback);
	params->getChatParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged);
	params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);

	return createClientChatRoom(conferenceFactoryUri, ConferenceId(nullptr, defaultLocalAddress), nullptr, params);
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

shared_ptr<AbstractChatRoom>
CorePrivate::searchChatRoom(const shared_ptr<ConferenceParams> &params,
                            const std::shared_ptr<const Address> &localAddress,
                            const std::shared_ptr<const Address> &remoteAddress,
                            const std::list<std::shared_ptr<Address>> &participants) const {
	L_Q();
	for (const auto &chatRoom : q->getRawChatRoomList()) {
		if (params) {
			const auto &chatRoomParams = chatRoom->getCurrentParams();
			if (params->getChatParams()->getBackend() != chatRoomParams->getChatParams()->getBackend()) continue;

			if (params->isGroup() != chatRoomParams->isGroup()) continue;

			if (params->isGroup() &&
			    (chatRoomParams->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::Basic))
				continue;

			if (params->getChatParams()->isEncrypted() != chatRoomParams->getChatParams()->isEncrypted()) continue;

			// Subject doesn't make any sense for basic chat room
			if ((params->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::FlexisipChat) &&
			    (!params->getSubject().empty() && params->getSubject() != chatRoom->getSubject()))
				continue;
		}

		std::shared_ptr<Address> curLocalAddress = chatRoom->getLocalAddress();
		const auto curLocalAddressWithoutGruu = curLocalAddress->getUriWithoutGruu();
		const auto localAddressWithoutGruu =
		    (localAddress && localAddress->isValid()) ? localAddress->getUriWithoutGruu() : Address();
		if (localAddressWithoutGruu.isValid() && (localAddressWithoutGruu != curLocalAddressWithoutGruu)) continue;

		std::shared_ptr<Address> curRemoteAddress = chatRoom->getPeerAddress();
		const auto curRemoteAddressWithoutGruu = curRemoteAddress->getUriWithoutGruu();
		const auto remoteAddressWithoutGruu =
		    (remoteAddress && remoteAddress->isValid()) ? remoteAddress->getUriWithoutGruu() : Address();
		if (remoteAddressWithoutGruu.isValid() && (remoteAddressWithoutGruu != curRemoteAddressWithoutGruu)) continue;

		bool allFound = true;
		for (const auto &participant : participants) {
			bool found = false;
			for (const auto &p : chatRoom->getParticipants()) {
				if (participant->weakEqual(*(p->getAddress()))) {
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

shared_ptr<AbstractChatRoom>
CorePrivate::createChatRoom(const shared_ptr<ConferenceParams> &params,
                            const std::shared_ptr<const Address> &localAddr,
                            const std::list<std::shared_ptr<const Address>> &participants) {
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
	        linphone_factory_get(), static_cast<LinphoneChatRoomBackend>(params->getChatParams()->getBackend()))) {
		lWarning() << "Tying to create chat room with unavailable backend";
		return nullptr;
	}
	shared_ptr<AbstractChatRoom> chatRoom;
	if (params->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat) {
#ifdef HAVE_ADVANCED_IM
		const auto &conferenceFactoryUri = Core::getConferenceFactoryAddress(q->getSharedFromThis(), localAddr);
		if (!conferenceFactoryUri || !conferenceFactoryUri->isValid()) {
			lWarning() << "Not creating group chat room: no conference factory uri for local address [" << localAddr
			           << "]";
			return nullptr;
		}

		ConferenceId conferenceId = ConferenceId(nullptr, localAddr);
		if (!params->isGroup() && participants.size() > 0) {
			// Prevent multiple 1-1 conference based chat room with same local/remote addresses
			chatRoom = q->findOneToOneChatRoom(localAddr, participants.front(), false, true,
			                                   params->getChatParams()->isEncrypted());
			if (chatRoom != nullptr) {
				lWarning() << "Found already existing 1-1 chat room that matches, using this one " << chatRoom;
				return chatRoom;
			}
		}

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
		std::list<std::shared_ptr<Address>> emptyList;
		chatRoom = searchChatRoom(params, localAddr, remoteAddr, emptyList);
		if (chatRoom == nullptr) {
			chatRoom = createBasicChatRoom(ConferenceId(remoteAddr, localAddr), params);
			insertChatRoom(chatRoom);
			insertChatRoomWithDb(chatRoom);
		} else {
			lInfo() << "Found an existing BasicChatRoom with this participant, using it instead of creating a new one";
		}
	}
	return chatRoom;
}

shared_ptr<AbstractChatRoom>
CorePrivate::createChatRoom(const shared_ptr<ConferenceParams> &params,
                            const std::list<std::shared_ptr<const Address>> &participants) {
	auto defaultLocalAddress =
	    getDefaultLocalAddress(nullptr, params->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat);
	return createChatRoom(params, defaultLocalAddress, participants);
}

shared_ptr<AbstractChatRoom>
CorePrivate::createChatRoom(const std::string &subject, const std::list<std::shared_ptr<const Address>> &participants) {
	L_Q();

	shared_ptr<ConferenceParams> params = ConferenceParams::create(q->getSharedFromThis());
	params->setChatDefaults();
	if (participants.size() > 1) {
		// Try to infer chat room type based on requested participants number
		params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);
	} else {
		params->getChatParams()->setBackend(ChatParams::Backend::Basic);
	}
	auto defaultLocalAddress =
	    getDefaultLocalAddress(nullptr, params->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat);
	params->setSubject(subject);
	return createChatRoom(params, defaultLocalAddress, participants);
}

shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const shared_ptr<ConferenceParams> &params,
                                                         const std::shared_ptr<Address> &localAddr,
                                                         const std::shared_ptr<const Address> &participant) {
	const std::list<std::shared_ptr<const Address>> &participants{participant};
	return createChatRoom(params, localAddr, participants);
}

// Assume basic chat room creation
shared_ptr<AbstractChatRoom> CorePrivate::createChatRoom(const std::shared_ptr<const Address> &participant) {
	return createChatRoom("", {participant});
}

void CorePrivate::insertChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_ASSERT(chatRoom);
	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	auto it = chatRoomsById.find(conferenceId);
	L_ASSERT(it == chatRoomsById.end() || it->second == chatRoom);
	if (it == chatRoomsById.end()) {
		// Remove chat room from workaround cache.
		if (linphone_core_get_global_state(getCCore()) != LinphoneGlobalStartup) {
			lInfo() << "Insert chat room " << chatRoom << " (id " << conferenceId << ") to core map";
		}
		chatRoomsById[conferenceId] = chatRoom;
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
	chatRoomsById.clear();
#ifdef HAVE_ADVANCED_IM
	if (clientListEventHandler) clientListEventHandler->clearHandlers();
#endif
	if (!mainDb->isInitialized()) return;
	for (auto &chatRoom : mainDb->getChatRooms()) {
		const auto &chatRoomParams = chatRoom->getCurrentParams();
		// We are looking for a one to one chatroom which isn't basic
		if (chatRoomParams->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::Basic) {
			insertChatRoom(chatRoom);
		} else {
			const auto &conference = chatRoom->getConference();
#ifdef HAVE_DB_STORAGE
			const auto &conferenceAddress = conference->getConferenceAddress();
			if (conferenceAddress) {
				auto conferenceInfo = mainDb->getConferenceInfoFromURI(conferenceAddress);
				ConferenceInfo::participant_list_t participantInfos;
				if (conferenceInfo) {
					participantInfos = conferenceInfo->getParticipants();
				} else {
					const auto &participants = conference->getParticipants();
					for (const auto &participant : participants) {
						auto participantInfo = ParticipantInfo::create(participant->getAddress());
						participantInfo->setRole(Participant::Role::Speaker);
						participantInfos.push_back(participantInfo);
					}
				}
				conference->setInvitedParticipants(participantInfos);
			}
#endif // HAVE_DB_STORAGE
			const ConferenceId &conferenceId = conference->getConferenceId();
			conferenceById.insert(std::make_pair(conferenceId, conference));
		}

		// TODO FIXME: Remove later when devices for friends will be notified through presence
		for (const auto &p : chatRoom->getParticipants()) {
			auto devices = mainDb->getDevices(p->getAddress());
			if (devices.empty()) {
				for (const auto &d : p->getDevices()) {
					auto gruu = d->getAddress();
					auto name = d->getName();
					lDebug() << "[Friend] Inserting existing device with name [" << name << "] and address ["
					         << gruu->asStringUriOnly() << "]";
					mainDb->insertDevice(gruu, name);
				}
			}
		}
	}
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
	for (const auto &chatRoom : q->getRawChatRoomList()) {
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
	for (const auto &chatRoom : q->getRawChatRoomList()) {
		const auto &chatRoomParams = chatRoom->getCurrentParams();
		// We are looking for a one to one chatroom which isn't basic
		if ((chatRoomParams->getChatParams()->getBackend() == LinphonePrivate::ChatParams::Backend::Basic) &&
		    chatRoomParams->isGroup()) {
			continue;
		}

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

	conferenceById.erase(oldConferenceId);
	conferenceById[newConferenceId] = chatRoom->getConference();

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

const std::shared_ptr<Address> Core::getConferenceFactoryAddress(const shared_ptr<Core> &core,
                                                                 const std::shared_ptr<const Address> &localAddress) {
	auto account = core->lookupKnownAccount(localAddress, true);
	if (!account) {
		lWarning() << "No account found for local address: [" << *localAddress << "]";
		return nullptr;
	} else return getConferenceFactoryAddress(core, account);
}

const std::shared_ptr<Address> Core::getConferenceFactoryAddress(BCTBX_UNUSED(const shared_ptr<Core> &core),
                                                                 const std::shared_ptr<Account> account) {
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
std::list<std::shared_ptr<AbstractChatRoom>> Core::getRawChatRoomList() const {
	L_D();
	std::list<std::shared_ptr<AbstractChatRoom>> chatRooms;
	for (const auto &[id, conference] : d->conferenceById) {
		const auto &chatRoom = conference->getChatRoom();
		if (chatRoom) {
			chatRooms.push_back(chatRoom);
		}
	}

	for (const auto &chatRoomPair : d->chatRoomsById) {
		const auto &chatRoom = chatRoomPair.second;
		if (chatRoom) {
			chatRooms.push_back(chatRoom);
		}
	}
	return chatRooms;
}

void Core::updateChatRoomList() const {
	LinphoneCore *lc = getCCore();
	LinphoneConfig *config = linphone_core_get_config(lc);
	bool hideEmptyChatRooms = !!linphone_config_get_int(config, "misc", "hide_empty_chat_rooms", 1);
	bool hideChatRoomsFromRemovedProxyConfig =
	    !!linphone_config_get_int(config, "misc", "hide_chat_rooms_from_removed_proxies", 1);

	list<shared_ptr<AbstractChatRoom>> rooms;

	for (const auto &chatRoom : getRawChatRoomList()) {
		if (hideEmptyChatRooms) {
			const auto &chatRoomParams = chatRoom->getCurrentParams();
			if (chatRoom->isEmpty() && !chatRoomParams->isGroup()) {
				continue;
			}
		}

		if (hideChatRoomsFromRemovedProxyConfig) {
			bool found = false;
			for (const auto &account : getAccounts()) {
				auto localAddress = account->getAccountParams()->getIdentityAddress();
				if (localAddress->weakEqual(*chatRoom->getLocalAddress())) {
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
	std::list<std::shared_ptr<Address>> emptyList;
	auto chatRoom =
	    d->searchChatRoom(nullptr, conferenceId.getLocalAddress(), conferenceId.getPeerAddress(), emptyList);
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

shared_ptr<AbstractChatRoom> Core::findOneToOneChatRoom(const std::shared_ptr<const Address> &localAddress,
                                                        const std::shared_ptr<const Address> &participantAddress,
                                                        bool basicOnly,
                                                        bool conferenceOnly,
                                                        bool encrypted) const {
	for (const auto &chatRoom : getRawChatRoomList()) {
		const std::shared_ptr<Address> &curLocalAddress = chatRoom->getLocalAddress();
		const auto &chatRoomParams = chatRoom->getCurrentParams();

		// We are looking for a one to one chatroom
		// Do not return a group chat room that everyone except one person has left
		if (chatRoomParams->isGroup()) continue;

		if (encrypted != chatRoomParams->getChatParams()->isEncrypted()) continue;

		const auto chatBackend = chatRoomParams->getChatParams()->getBackend();
		// One to one client group chat room
		// The only participant's address must match the participantAddress argument
		if (!basicOnly && (chatBackend == LinphonePrivate::ChatParams::Backend::FlexisipChat) &&
		    !chatRoom->getParticipants().empty() && localAddress->weakEqual(*curLocalAddress) &&
		    participantAddress->weakEqual(*chatRoom->getParticipants().front()->getAddress()))
			return chatRoom;

		// One to one basic chat room (addresses without gruu)
		// The peer address must match the participantAddress argument
		if (!conferenceOnly && (chatBackend == LinphonePrivate::ChatParams::Backend::Basic) &&
		    localAddress->weakEqual(*curLocalAddress) && participantAddress->weakEqual(*chatRoom->getPeerAddress()))
			return chatRoom;
	}
	return nullptr;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom(const ConferenceId &conferenceId) {
	L_D();

	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	if (chatRoom) {
		return chatRoom;
	}
	ChatRoom::CapabilitiesMask capabilities({ChatRoom::Capabilities::Basic, ChatRoom::Capabilities::OneToOne});
	auto params = ConferenceParams::fromCapabilities(capabilities, getSharedFromThis());
	chatRoom = d->createBasicChatRoom(conferenceId, params);
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
}

shared_ptr<AbstractChatRoom> Core::getOrCreateBasicChatRoom(const std::shared_ptr<const Address> &localAddress,
                                                            const std::shared_ptr<const Address> &peerAddress) {
	L_D();

	shared_ptr<AbstractChatRoom> chatRoom = findOneToOneChatRoom(localAddress, peerAddress, true, false, false);
	if (chatRoom) return chatRoom;

	ChatRoom::CapabilitiesMask capabilities({ChatRoom::Capabilities::Basic, ChatRoom::Capabilities::OneToOne});
	chatRoom = d->createBasicChatRoom(ConferenceId(peerAddress, (localAddress && localAddress->isValid()
	                                                                 ? localAddress
	                                                                 : d->getDefaultLocalAddress(peerAddress, false))),
	                                  ConferenceParams::fromCapabilities(capabilities, getSharedFromThis()));
	d->insertChatRoom(chatRoom);
	d->insertChatRoomWithDb(chatRoom);

	return chatRoom;
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
	lInfo() << "Trying to delete chat room with conference ID " << conferenceId << ".";

	auto chatRoomInCoreMap = core->findChatRoom(conferenceId);
	if (chatRoomInCoreMap) {
		CorePrivate *d = core->getPrivate();
		d->conferenceById.erase(conferenceId);
		d->chatRoomsById.erase(conferenceId);
		if (d->mainDb->isInitialized()) d->mainDb->deleteChatRoom(conferenceId);
	} else {
		lError() << "Unable to delete chat room with conference ID " << conferenceId << " because it cannot be found.";
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

	for (const auto &chatRoom : q->getChatRooms()) {
		chatRoom->notifyAggregatedChatMessages();
	}

	chatMessagesAggregationBackgroundTask.stop();
}

bool Core::isCurrentlyAggregatingChatMessages() {
	L_D();

	return d->chatMessagesAggregationTimer != nullptr;
}

LinphoneReason
Core::handleChatMessagesAggregation(shared_ptr<AbstractChatRoom> chatRoom, SalOp *op, const SalMessage *sal_msg) {
	L_D();
	LinphoneCore *cCore = getCCore();

	bool chatMessagesAggregationEnabled = !!linphone_core_get_chat_messages_aggregation_enabled(cCore);
	int chatMessagesAggregationDelay =
	    linphone_config_get_int(linphone_core_get_config(cCore), "sip", "chat_messages_aggregation_delay", 0);
	lDebug() << "Chat messages aggregation enabled? " << chatMessagesAggregationEnabled << " with delay "
	         << chatMessagesAggregationDelay;
	if (chatMessagesAggregationEnabled && chatMessagesAggregationDelay > 0) {
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
	std::shared_ptr<Address> peerAddress;
	std::shared_ptr<Address> localAddress;

	if (linphone_core_conference_server_enabled(cCore)) {
		localAddress = peerAddress = Address::create(op->getTo());
	} else {
		peerAddress = Address::create(op->getFrom());
		localAddress = Address::create(op->getTo());
	}

	ConferenceId conferenceId{peerAddress, localAddress};
	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	if (chatRoom) {
		reason = handleChatMessagesAggregation(chatRoom, op, sal_msg);
	} else if (!linphone_core_conference_server_enabled(cCore)) {
		const char *session_mode = sal_custom_header_find(op->getRecvCustomHeaders(), "Session-mode");
		/* Client mode but check that it is really for basic chatroom before creating it.*/
		if (session_mode && strcasecmp(session_mode, "true") == 0) {
			lError() << "Message is received in the context of a client chatroom for which we have no context.";
			reason = LinphoneReasonNotAcceptable;
		} else {
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
