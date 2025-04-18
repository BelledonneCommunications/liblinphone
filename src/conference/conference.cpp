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

#include <bctoolbox/defs.h>

#include "call/call.h"
#include "conference/conference-id-params.h"
#include "conference/conference.h"
#include "conference/notify-conference-listener.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-device.h"
#include "conference/participant-info.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/ms2-streams.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "core/core.h"
#include "factory/factory.h"
#include "logger/logger.h"
#include "participant.h"
#include "private_functions.h"

#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/server-chat-room.h"
#include "xml/resource-lists.h"
#endif

#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string Conference::SecurityModeParameter = "conference-security-mode";
const std::string Conference::ConfIdParameter = "conf-id";
const std::string Conference::AdminParameter = "admin";
const std::string Conference::IsFocusParameter = "isfocus";
const std::string Conference::TextParameter = "text";

Conference::Conference(const shared_ptr<Core> &core,
                       std::shared_ptr<CallSessionListener> callSessionListener,
                       const std::shared_ptr<const ConferenceParams> params)
    : CoreAccessor(core) {
	mCallSessionListener = callSessionListener;
	update(*params);
	if (mConfParams->videoEnabled()) {
		// If video is enabled, then always enable audio capabilities
		mConfParams->enableAudio(true);
	}

	addListener(std::make_shared<NotifyConferenceListener>(this));

	if (mConfParams->getStartTime() < 0) {
		mConfParams->setStartTime(ms_time(NULL));
	}

	if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
		auto startTime = mConfParams->getStartTime();
		auto endTime = mConfParams->getEndTime();
		auto duration = (endTime >= 0) ? (endTime - startTime) : -1;

		if ((endTime >= 0) && (duration < 0)) {
			lError() << "Unable to create conference due to an invalid time settings";
			lError() << "Start time (" << startTime << "): " << mConfParams->getStartTimeString();
			lError() << "End time (" << endTime << "): " << mConfParams->getEndTimeString();
			lError() << "Duration: " << duration << " seconds";
			setState(ConferenceInterface::State::CreationFailed);
		}
	}
}

Conference::~Conference() {
	setChatRoom(nullptr);
	mConfListeners.clear();
}

// -----------------------------------------------------------------------------
void Conference::inititializeMe() {
	const auto &core = getCore();
	auto account = mConfParams->getAccount();
	if (!account) {
		account = core->getDefaultAccount();
		if (account) {
			lInfo() << "Using default account " << *account << " for " << *this
			        << " because it was not set in the conference parameters";
			mConfParams->setAccount(account);
		} else {
			lInfo() << "Not setting account for " << *this
			        << " because it was not set in the conference parameters and the core has no default account";
		}
	}

	if (account) {
		auto meAddress = account->getAccountParams()->getIdentityAddress()->getUri();
		mConferenceId = ConferenceId(Address(), std::move(meAddress), core->createConferenceIdParams());
		mMe = Participant::create(getSharedFromThis(), mConferenceId.getLocalAddress());
	}
}

void Conference::removeListener(std::shared_ptr<ConferenceListenerInterface> listener) {
	mConfListeners.remove(listener);
}

void Conference::addListener(std::shared_ptr<ConferenceListenerInterface> listener) {
	if (listener) {
		// Avoid adding twice the same listener.
		// It may happen when a conference with chat capabilities is rejoined without restarting the core
		const auto it = std::find_if(mConfListeners.cbegin(), mConfListeners.cend(),
		                             [&listener](const auto &l) { return l == listener; });
		if (it == mConfListeners.cend()) {
			mConfListeners.push_back(listener);
		}
	}
}

void Conference::invalidateAccount() {
	mConfParams->setAccount(nullptr);
}

std::shared_ptr<Account> Conference::getAccount() {
	auto account = mConfParams->getAccount();
	if (!account) {
		account = getCore()->findAccountByIdentityAddress(mConferenceId.getLocalAddress());
		mConfParams->setAccount(account);
	}
	if (!account) {
		lWarning() << "Unable to find account with identity [" << *mConferenceId.getLocalAddress()
		           << "] to associate account to " << *this;
	}
	return account;
}

time_t Conference::getStartTime() const {
	return mConfParams->getStartTime();
}

int Conference::getDuration() const {
	return (int)(ms_time(nullptr) - getStartTime());
}

shared_ptr<Participant> Conference::getActiveParticipant() const {
	return mActiveParticipant;
}

void Conference::clearParticipants() {
	mMe->clearDevices();
	mParticipants.clear();
}

// -----------------------------------------------------------------------------
std::shared_ptr<ParticipantDevice> Conference::createParticipantDevice(std::shared_ptr<Participant> participant,
                                                                       std::shared_ptr<Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	const auto &session = call->getActiveSession();
	// If device is not found, then add it
	if (participant->findDevice(session, false) == nullptr) {
		shared_ptr<ParticipantDevice> device = participant->addDevice(session);
		// If there is already a call for this participant, then he/she is joining the conference
		const auto &dAddress = device->getAddress();
		auto invitedInfo = findInvitedParticipant(dAddress);
		bool joining =
		    (invitedInfo || (mConfParams->getParticipantListType() == ConferenceParams::ParticipantListType::Open));
		auto deviceState = ParticipantDevice::State::Joining;
		if (joining) {
			deviceState = ParticipantDevice::State::Joining;
			lInfo() << "Participant with address " << *remoteAddress << " has added device with session " << session
			        << " (address " << *dAddress << ") to " << *this;
		} else {
			deviceState = ParticipantDevice::State::RequestingToJoin;
			lInfo() << "Participant with address " << *remoteAddress << " has added device with session " << session
			        << " (address " << *dAddress << ") which is requesting to join " << *this;
		}
		device->setState(deviceState, false);
		// By default send a NOTIFY to inform the other clients that has been added
		device->setSendAddedNotify(true);
		return device;
	} else {
		lDebug() << "Participant with address " << *remoteAddress << " has already a device with session " << session;
	}
	return nullptr;
}

bool Conference::addParticipantDevice(std::shared_ptr<Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	auto p = findParticipant(remoteAddress);
	if (p) {
		auto device = createParticipantDevice(p, call);
		bool added = (device != nullptr);
		if (added) {
			notifyNewDevice(device);
		}
		return added;
	}

	return false;
}

void Conference::addParticipantDevice(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                      BCTBX_UNUSED(const shared_ptr<ParticipantDeviceIdentity> &deviceInfo)) {
	lWarning() << __func__ << " not implemented";
}

void Conference::notifyNewDevice(const std::shared_ptr<ParticipantDevice> &device) {
	if (device) {
		const auto &p = device->getParticipant();
		if (p) {
			time_t creationTime = ms_time(nullptr);
			if (device->getState() == ParticipantDevice::State::Joining) {
				notifyParticipantDeviceAdded(creationTime, false, p, device);
			} else {
				notifyParticipantDeviceJoiningRequest(creationTime, false, p, device);
			}
		}
	}
}

Address Conference::createParticipantAddressForResourceList(const std::shared_ptr<Participant> &p) {
	Address address = *p->getAddress();
	address.setUriParam(ParticipantInfo::roleParameter, Participant::roleToText(p->getRole()));
	return address;
}

Address Conference::createParticipantAddressForResourceList(const ConferenceInfo::participant_list_t::value_type &p) {
	Address address = p->getAddress()->getUri();
	for (const auto &[name, value] : p->getAllParameters()) {
		address.setUriParam(name, value);
	}
	return address;
}

void Conference::fillParticipantAttributes(std::shared_ptr<Participant> &p) const {
	const auto &pAddress = p->getAddress();
	const auto participantInfo =
	    std::find_if(mInvitedParticipants.cbegin(), mInvitedParticipants.cend(),
	                 [&pAddress](const auto &info) { return pAddress->weakEqual(*info->getAddress()); });

	if (participantInfo == mInvitedParticipants.cend()) {
		if (mInvitedParticipants.empty()) {
			// It is a conference created on the fly, therefore all participants are speakers
			p->setRole(Participant::Role::Speaker);
			lInfo() << *this
			        << " has been created on the fly, either by inviting addresses or by merging existing calls "
			           "therefore participant "
			        << *pAddress << " is given the role of " << p->getRole();
		} else {
			const bool isThereAListener =
			    std::find_if(mInvitedParticipants.cbegin(), mInvitedParticipants.cend(), [](const auto &info) {
				    return (info->getRole() == Participant::Role::Listener);
			    }) != mInvitedParticipants.cend();

			std::string reason;
			Participant::Role role = Participant::Role::Unknown;
			if (isThereAListener) {
				role = Participant::Role::Listener;
				reason.assign("at least one participant with role Listener has been found");
			} else {
				role = Participant::Role::Speaker;
				reason.assign("no participants with role Listener has been found");
			}
			p->setRole(role);

			lInfo() << "Unable to find participant " << *pAddress
			        << " in the list of invited participants. Assuming its role to be " << p->getRole() << " in "
			        << *this << " because " << reason;
		}
	} else {
		const auto &role = (*participantInfo)->getRole();
		if (role == Participant::Role::Unknown) {
			p->setRole(Participant::Role::Speaker);
			lInfo() << "No role was given to participant " << *pAddress << " when " << *this
			        << " was created. Assuming its role to be " << p->getRole();
		} else {
			p->setRole(role);
		}
	}
}

bool Conference::addParticipant(BCTBX_UNUSED(const std::shared_ptr<ParticipantInfo> &info)) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

std::shared_ptr<Participant> Conference::createParticipant(std::shared_ptr<const Address> participantAddress) {
	auto participant = Participant::create(getSharedFromThis(), participantAddress);
	auto session = participant->createSession(*this, nullptr, true);
	session->addListener(getSharedFromThis());
	const auto confAddr = getConferenceAddress();
	bool isFocus = participantAddress && confAddr && (*participantAddress == *confAddr);
	participant->setFocus(isFocus);
	participant->setPreserveSession(false);
	fillParticipantAttributes(participant);
	return participant;
}

std::shared_ptr<Participant> Conference::createParticipant(std::shared_ptr<Call> call) {
	auto session = call->getActiveSession();
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	auto participant = Participant::create(getSharedFromThis(), remoteAddress);
	fillParticipantAttributes(participant);
	participant->setFocus(false);
	std::shared_ptr<Address> toAddr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		if (op) {
			toAddr = Address::create(op->getTo());
		}
	}

	if (toAddr && toAddr->isValid()) {
		participant->setPreserveSession(!toAddr->hasUriParam(Conference::ConfIdParameter));
	} else {
		participant->setPreserveSession(true);
	}

	auto device = createParticipantDevice(participant, call);
	device->setSendAddedNotify(false);

	return participant;
}

bool Conference::addParticipant(std::shared_ptr<Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<Participant> participant = findParticipant(remoteAddress);
	bool success = false;
	// Add a new participant only if it is not in the conference
	if (participant == nullptr) {
		auto participant = createParticipant(call);
		mParticipants.push_back(participant);
		time_t creationTime = ms_time(nullptr);
		notifyParticipantAdded(creationTime, false, participant);
		success = true;
	} else {
		lWarning() << "Participant with address " << *remoteAddress << " is already part of " << *this;
		success = false;
	}
	return success;
}

bool Conference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	shared_ptr<Participant> participant = findParticipant(participantAddress);
	if (participant) {
		lWarning() << "Not adding participant '" << *participantAddress << "' because it is already a participant of "
		           << *this;
		return false;
	}
	participant = createParticipant(participantAddress);
	mParticipants.push_back(participant);
	if (!mActiveParticipant) mActiveParticipant = participant;

	lInfo() << "Participant with address " << *participantAddress << " has been added to " << *this;
	time_t creationTime = ms_time(nullptr);
	notifyParticipantAdded(creationTime, false, participant);
	return true;
}

std::shared_ptr<CallSession> Conference::getMainSession() const {
	return mMe ? mMe->getSession() : nullptr;
}

bool Conference::addParticipants(const std::list<std::shared_ptr<Address>> &addresses) {
	list<std::shared_ptr<Address>> sortedAddresses(addresses);
	sortedAddresses.sort([](const auto &addr1, const auto &addr2) { return *addr1 < *addr2; });
	sortedAddresses.unique([](const auto &addr1, const auto &addr2) { return addr1->weakEqual(*addr2); });

	bool soFarSoGood = true;
	for (const auto &address : sortedAddresses)
		soFarSoGood &= addParticipant(address);
	return soFarSoGood;
}

bool Conference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
	list<std::shared_ptr<Call>> sortedCalls(calls);
	sortedCalls.sort();
	sortedCalls.unique();

	bool soFarSoGood = true;
	for (const auto &call : sortedCalls)
		soFarSoGood &= addParticipant(call);

	return soFarSoGood;
}

bool Conference::setParticipants(const std::list<std::shared_ptr<Participant>> &&newParticipants) {
	mParticipants.clear();
	mParticipants = std::move(newParticipants);
	return 0;
}

void Conference::setInvitedParticipants(const std::list<std::shared_ptr<Participant>> &invitedParticipants) {
	mInvitedParticipants = invitedParticipants;
}

void Conference::removeParticipantDevice(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                         BCTBX_UNUSED(const std::shared_ptr<Address> &deviceAddress)) {
	lError() << __func__ << " Should be overridden by the derived class";
}

int Conference::removeParticipantDevice(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		// If device is not found, then add it
		if (device != nullptr) {
			device->setState(ParticipantDevice::State::ScheduledForLeaving);
			shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
			if (ev) {
				// try to terminate subscription if any, but do not wait for answer.
				ev->clearCallbacksList();
				ev->terminate();
			}

			CallSession::State sessionState = session->getState();
			auto sessionHasEnded =
			    (sessionState == CallSession::State::Released) || (sessionState == CallSession::State::End);
			auto initiatingRemoval = static_pointer_cast<MediaSession>(session)->isTerminator() || !sessionHasEnded;
			const auto ei = session->getErrorInfo();
			device->setDisconnectionData(initiatingRemoval, linphone_error_info_get_protocol_code(ei),
			                             linphone_error_info_get_reason(ei));
			device->setState(ParticipantDevice::State::Left);
			time_t creationTime = ms_time(nullptr);
			notifyParticipantDeviceRemoved(creationTime, false, p, device);

			lInfo() << "Removing device with session " << session << " from participant " << *p->getAddress()
			        << " in conference " << *getConferenceAddress();
			p->removeDevice(session);

			auto op = session->getPrivate()->getOp();
			shared_ptr<Call> call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
			if (call) {
				call->setConference(nullptr);
			}
			return 0;
		}
	}

	return -1;
}

int Conference::removeParticipant(std::shared_ptr<Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (!p) {
		lDebug() << "Unable to participant with address " << remoteAddress;
		return -1;
	}
	return removeParticipant(p);
}

int Conference::removeParticipant(const std::shared_ptr<CallSession> &session,
                                  BCTBX_UNUSED(const bool preserveSession)) {
	const std::shared_ptr<Address> &pAddress = session->getRemoteAddress();
	const auto &conferenceAddress = getConferenceAddress();
	std::shared_ptr<Participant> p = findParticipant(pAddress);
	removeParticipantDevice(session);
	if (!p) {
		lInfo() << "Participant removal failed: Participant with address " << *pAddress
		        << " has not been found in conference " << *conferenceAddress;
		return -1;
	}
	if (p->getDevices().empty()) {
		lInfo() << "Remove participant with address " << *pAddress << " from conference " << *conferenceAddress;
		mParticipants.remove(p);
		time_t creationTime = ms_time(nullptr);
		notifyParticipantRemoved(creationTime, false, p);
		return 0;
	}
	return -1;
}

int Conference::removeParticipant(const std::shared_ptr<Address> &addr) {
	std::shared_ptr<Participant> p = findParticipant(addr);
	return removeParticipant(p);
}

bool Conference::removeParticipant(const std::shared_ptr<Participant> &participant) {
	if (!participant) {
		lWarning() << "Asking to remove a participant whose pointer is NULL";
		return false;
	}
	auto conferenceParticipant = findParticipant(participant->getAddress());
	if (!conferenceParticipant) {
		lWarning() << "Unable to remove participant " << *participant->getAddress() << " from conference "
		           << *getConferenceAddress() << " because it is not a member of it";
		return false;
	}

	lInfo() << "Removing participant with address " << *conferenceParticipant->getAddress() << " from conference "
	        << *getConferenceAddress();

	if (!mConfParams->chatEnabled()) {
		auto &devices = conferenceParticipant->getDevices();
		// Delete all devices of a participant
		for (const auto &device : devices) {
			shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
			if (ev) {
				// try to terminate subscription if any, but do not wait for answer.
				ev->clearCallbacksList();
				ev->terminate();
			}

			time_t creationTime = ms_time(nullptr);
			notifyParticipantDeviceRemoved(creationTime, false, conferenceParticipant, device);
		}

		conferenceParticipant->clearDevices();
		auto pSession = conferenceParticipant->getSession();
		// Detach call from conference
		if (pSession) {
			auto op = pSession->getPrivate()->getOp();
			shared_ptr<Call> pSessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
			if (pSessionCall) {
				pSessionCall->setConference(nullptr);
			}
		}
	}
	mParticipants.remove(conferenceParticipant);
	return true;
}

void Conference::addInvitedParticipant(const std::shared_ptr<Call> &call) {
	addInvitedParticipant(createParticipant(call));
}

void Conference::addInvitedParticipant(const std::shared_ptr<Address> &address) {
	auto participant = Participant::create(getSharedFromThis(), address);
	Participant::Role role = Participant::Role::Speaker;
	lInfo() << "The role of invited participant " << *address << " has not been given, therefore assuming it to be "
	        << role;
	participant->setRole(role);
	addInvitedParticipant(participant);
}

void Conference::addInvitedParticipant(const std::shared_ptr<Participant> &participant) {
	auto pAddress = participant->getAddress();
	if (!findInvitedParticipant(pAddress)) {
		mInvitedParticipants.push_back(participant);
	} else {
		lError() << *this << ": Inviting twice participant with address " << *pAddress;
	}
}

void Conference::fillInvitedParticipantList(const ConferenceInfo::participant_list_t infos) {
	for (const auto &info : infos) {
		const auto &address = info->getAddress();
		auto participant = Participant::create(getSharedFromThis(), address);
		participant->setRole(info->getRole());
		participant->setSequenceNumber(info->getSequenceNumber());
		addInvitedParticipant(participant);
	}
}

void Conference::fillInvitedParticipantList(SalCallOp *op, const std::shared_ptr<Address> &organizer, bool cancelling) {
	mInvitedParticipants.clear();
	const auto &resourceList = op->getContentInRemote(ContentType::ResourceLists);
	if (resourceList && !resourceList.value().get().isEmpty()) {
		auto invitees = Utils::parseResourceLists(resourceList);
		for (const auto &invitee : invitees) {
			auto participant = Participant::create(getSharedFromThis(), invitee->getAddress());
			participant->setRole(invitee->getRole());
			participant->setSequenceNumber(invitee->getSequenceNumber());
			addInvitedParticipant(participant);
		}
		if (!cancelling && organizer) {
			auto organizerNotFound = std::find_if(mInvitedParticipants.begin(), mInvitedParticipants.end(),
			                                      [&organizer](const auto &invitee) {
				                                      return organizer->weakEqual(*invitee->getAddress());
			                                      }) == mInvitedParticipants.end();
			if (organizerNotFound) {
				lInfo() << "The organizer " << *organizer
				        << " has not been found in the list of invited participants - adding it";
				addInvitedParticipant(organizer);
			}
		}
	}
}

shared_ptr<ParticipantDevice>
Conference::findInvitedParticipantDevice(const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : mInvitedParticipants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session) return device;
		}
	}

	return nullptr;
}

void Conference::removeInvitedParticipant(const std::shared_ptr<Address> &address) {
	auto c = std::find_if(mInvitedParticipants.begin(), mInvitedParticipants.end(),
	                      [&address](const auto &p) { return (address->weakEqual(*p->getAddress())); });
	if (c == mInvitedParticipants.end()) {
		lDebug() << "Unable to find participant " << *address << " in the list of cached participants of " << *this;
	} else {
		mInvitedParticipants.erase(c);
	}
}

std::list<std::shared_ptr<Participant>> Conference::getInvitedParticipants() const {
	return mInvitedParticipants;
}

std::list<std::shared_ptr<const Address>> Conference::getInvitedAddresses() const {
	list<std::shared_ptr<const Address>> addresses;
	for (auto &participant : mInvitedParticipants) {
		addresses.push_back(participant->getAddress());
	}
	return addresses;
}

std::list<std::shared_ptr<Participant>> Conference::getFullParticipantList() const {
	auto participantList = mInvitedParticipants;
	// Add participants that are not part of the invitees'list
	for (const auto &p : getParticipants()) {
		const auto &pAddress = p->getAddress();
		auto pIt = std::find_if(participantList.begin(), participantList.end(), [&pAddress](const auto &participant) {
			return (pAddress->weakEqual(*participant->getAddress()));
		});
		if (pIt == participantList.end()) {
			participantList.push_back(p);
		}
	}
	return participantList;
}

int Conference::terminate() {
	mParticipants.clear();
	return 0;
}

LinphoneStatus Conference::updateMainSession(bool modifyParams) {
	if (getCore()->getCCore()->sal->mediaDisabled()) return -1;

	LinphoneStatus ret = -1;
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();
		if (params->rtpBundleEnabled()) {
			MediaSessionParams *currentParams = params->clone();
			currentParams->getPrivate()->setInternalCallUpdate(false);

			// Update parameters based on conference capabilities
			if (!mConfParams->audioEnabled()) {
				currentParams->enableAudio(mConfParams->audioEnabled());
			}
			if (!mConfParams->videoEnabled()) {
				currentParams->enableVideo(mConfParams->videoEnabled());
			}

			bool wasScreenSharingEnabled = currentParams->screenSharingEnabled();
			if (modifyParams) {
				const auto screenSharingParticipant = getScreenSharingParticipant();
				bool screenSharingEnabled = false;
				if (getCachedScreenSharingDevice()) {
					// Disable screen sharing if no participant is screen sharing or the participant who is sharing its
					// screen is not the me participant
					screenSharingEnabled = screenSharingParticipant && isMe(screenSharingParticipant->getAddress());
				} else {
					// If no one was screen sharing, we can try again to enabled screen sharing
					screenSharingEnabled = wasScreenSharingEnabled;
				}
				currentParams->enableScreenSharing(screenSharingEnabled);
			}
			ret = session->update(currentParams);
			// Restore the screen sharing flag as it was before and change local parameters
			currentParams->enableScreenSharing(wasScreenSharingEnabled);
			session->getPrivate()->setParams(currentParams);
		} else {
			lWarning() << "Unable to update session " << session << " of conference " << *getConferenceAddress()
			           << " because RTP bundle is disabled";
			ret = 0;
		}
	}
	return ret;
}

ConferenceLayout Conference::getLayout() const {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	return session ? session->getParams()->getConferenceVideoLayout()
	               : (ConferenceLayout)linphone_core_get_default_conference_layout(getCore()->getCCore());
}

void Conference::setLayout(const ConferenceLayout layout) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session && (getLayout() != layout)) {
		lInfo() << "Changing layout of conference " << getConferenceAddress() << " from " << getLayout() << " to "
		        << layout;
		const_cast<CallSessionParams *>(session->getParams())->setConferenceVideoLayout(layout);
		updateMainSession();
	}
}

std::shared_ptr<Address> Conference::getConferenceAddress() const {
	return mConfParams->getConferenceAddress();
}

void Conference::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	const auto state = getState();
	if ((state == ConferenceInterface::State::Instantiated) || (state == ConferenceInterface::State::CreationPending)) {
		if (!conferenceAddress || !conferenceAddress->isValid()) {
			lError() << "Cannot set the conference address to " << *conferenceAddress;
			shared_ptr<CallSession> session = getMe()->getSession();
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, "SIP", LinphoneReasonUnknown, 500, "Server internal error", NULL);
			session->decline(ei);
			linphone_error_info_unref(ei);
			setState(ConferenceInterface::State::CreationFailed);
			return;
		}

		mConfParams->setConferenceAddress(conferenceAddress);
		setState(ConferenceInterface::State::CreationPending);
		lInfo() << "Conference " << this << " has been given the address " << *mConfParams->getConferenceAddress();
	} else {
		lDebug() << "Cannot set the conference address of the Conference in state " << state << " to "
		         << *conferenceAddress;
		return;
	}
}

shared_ptr<Participant> Conference::getMe() const {
	return mMe;
}

int Conference::getParticipantCount() const {
	return static_cast<int>(getParticipants().size());
}

const list<shared_ptr<Participant>> &Conference::getParticipants() const {
	return mParticipants;
}

std::list<std::shared_ptr<Address>> Conference::getParticipantAddresses() const {
	list<std::shared_ptr<Address>> addresses;
	for (auto &participant : mParticipants) {
		addresses.push_back(participant->getAddress());
	}
	return addresses;
}

list<shared_ptr<ParticipantDevice>> Conference::getParticipantDevices(bool includeMe) const {
	list<shared_ptr<ParticipantDevice>> devices;
	for (const auto &p : mParticipants) {
		const auto &d = p->getDevices();
		if (!d.empty()) {
			devices.insert(devices.end(), d.begin(), d.end());
		}
	}
	if (isIn() && includeMe) {
		const auto &d = getMe()->getDevices();
		if (!d.empty()) {
			devices.insert(devices.begin(), d.begin(), d.end());
		}
	}

	return devices;
}

void Conference::setCachedScreenSharingDevice() {
	mCachedScreenSharingDevice = getScreenSharingDevice();
}

void Conference::resetCachedScreenSharingDevice() {
	mCachedScreenSharingDevice = nullptr;
}

std::shared_ptr<ParticipantDevice> Conference::getCachedScreenSharingDevice() const {
	return mCachedScreenSharingDevice;
}

std::shared_ptr<Participant> Conference::getScreenSharingParticipant() const {
	const auto device = getScreenSharingDevice();
	return (device) ? device->getParticipant() : nullptr;
}

std::shared_ptr<ParticipantDevice> Conference::getScreenSharingDevice() const {
	const auto devices = getParticipantDevices();
	const auto screenSharingDeviceIt =
	    std::find_if(devices.cbegin(), devices.cend(), [](const auto &d) { return d->screenSharingEnabled(); });
	return (screenSharingDeviceIt == devices.cend()) ? nullptr : (*screenSharingDeviceIt);
}

const string &Conference::getSubject() const {
	return mConfParams->getSubject();
}

const string &Conference::getUtf8Subject() const {
	return mConfParams->getUtf8Subject();
}

const string &Conference::getUsername() const {
	return mUsername;
}

void Conference::join(BCTBX_UNUSED(const std::shared_ptr<Address> &participantAddress)) {
}

void Conference::leave() {
}

void Conference::setLocalParticipantStreamCapability(BCTBX_UNUSED(const LinphoneMediaDirection &direction),
                                                     BCTBX_UNUSED(const LinphoneStreamType type)) {
}

bool Conference::update(const ConferenceParamsInterface &newParameters) {
	const ConferenceParams &newConfParams = static_cast<const ConferenceParams &>(newParameters);
	std::shared_ptr<Account> account;
	bool isUpdate = (mConfParams != nullptr);
	if (isUpdate) {
		if ((*mConfParams->getConferenceFactoryAddress() != *newConfParams.getConferenceFactoryAddress()) ||
		    (*mConfParams->getConferenceAddress() != *newConfParams.getConferenceAddress())) {
			lError() << "Trying to change frozen conference parameters:";
			lError() << " -  factory address: actual " << *mConfParams->getConferenceFactoryAddress() << " new value "
			         << *newConfParams.getConferenceFactoryAddress();
			lError() << " -  conference address: actual " << *mConfParams->getConferenceAddress() << " new value "
			         << *newConfParams.getConferenceAddress();
			return false;
		}
		account = mConfParams->getAccount();
	}
	mConfParams = ConferenceParams::create(newConfParams);
	// The conference parameter account should not change if the application is updating them
	if (isUpdate) {
		mConfParams->setAccount(account);
	}
	return true;
};

bool Conference::removeParticipants(const list<shared_ptr<Participant>> &participants) {
	bool soFarSoGood = true;
	for (const auto &p : participants)
		soFarSoGood &= removeParticipant(p);
	return soFarSoGood;
}

void Conference::setParticipantAdminStatus(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                           BCTBX_UNUSED(bool isAdmin)) {
	lError() << "Conference class does not handle setParticipantAdminStatus() generically";
}

void Conference::setUtf8Subject(const string &subject) {
	mConfParams->setUtf8Subject(subject);
}

void Conference::setSubject(const string &subject) {
	setUtf8Subject(Utils::localeToUtf8(subject));
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceStateChanged(time_t creationTime,
                                                const bool isFullState,
                                                const std::shared_ptr<Participant> &participant,
                                                const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceStatusChanged, creationTime, mConferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);
	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceStateChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceScreenSharingChanged(time_t creationTime,
                                                        const bool isFullState,
                                                        const std::shared_ptr<Participant> &participant,
                                                        const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceStatusChanged, creationTime, mConferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceScreenSharingChanged(event, participantDevice);
	}
	return event;
}

void Conference::notifySpeakingDevice(uint32_t ssrc, bool isSpeaking) {
	for (const auto &participant : mParticipants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
				_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
				for (const auto &l : mConfListeners) {
					l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
				}
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
			_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
			for (const auto &l : mConfListeners) {
				l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
			}
			return;
		}
	}
	lDebug() << "IsSpeaking: unable to notify speaking device because there is no device found.";
}

void Conference::notifyMutedDevice(uint32_t ssrc, bool muted) {
	for (const auto &participant : mParticipants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
				_linphone_participant_device_notify_is_muted(device->toC(), muted);
				for (const auto &l : mConfListeners) {
					l->onParticipantDeviceIsMuted(device, muted);
				}
				mPendingParticipantsMutes.erase(ssrc);
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
			_linphone_participant_device_notify_is_muted(device->toC(), muted);
			for (const auto &l : mConfListeners) {
				l->onParticipantDeviceIsMuted(device, muted);
			}
			mPendingParticipantsMutes.erase(ssrc);
			return;
		}
	}
	mPendingParticipantsMutes[ssrc] = muted;
	lDebug() << "IsMuted: unable to notify muted device because there is no device found - queuing it waiting to match "
	            "a device to ssrc "
	         << ssrc;
}

void Conference::notifyLocalMutedDevices(bool muted) {
	for (const auto &device : getMe()->getDevices()) {
		_linphone_participant_device_notify_is_muted(device->toC(), muted);
		for (const auto &l : mConfListeners) {
			l->onParticipantDeviceIsMuted(device, muted);
		}
	}
}

const std::map<uint32_t, bool> &Conference::getPendingParticipantsMutes() const {
	return mPendingParticipantsMutes;
}

void Conference::setUsername(const string &username) {
	mUsername = username;
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::findParticipant(const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : mParticipants) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session)) {
			return participant;
		}
	}

	lDebug() << "Unable to find participant in " << *this << " with session " << session;
	return nullptr;
}

shared_ptr<Participant> Conference::findParticipant(const std::shared_ptr<const Address> &addr) const {
	for (const auto &participant : mParticipants) {
		if (participant->getAddress()->weakEqual(*addr)) {
			return participant;
		}
	}

	lDebug() << "Unable to find participant in " << *this << " with address " << *addr;
	return nullptr;
}

std::shared_ptr<Participant>
Conference::findInvitedParticipant(const std::shared_ptr<const Address> &participantAddress) const {
	const auto &it = std::find_if(
	    mInvitedParticipants.begin(), mInvitedParticipants.end(),
	    [&participantAddress](const auto &invitee) { return participantAddress->weakEqual(*invitee->getAddress()); });

	if (it != mInvitedParticipants.end()) {
		return (*it);
	}

	lDebug() << "Unable to find invited participant in " << *this << " with address " << *participantAddress;
	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDeviceByLabel(const LinphoneStreamType type,
                                                                       const std::string &label) const {
	for (const auto &participant : mParticipants) {
		auto device = participant->findDevice(type, label, false);
		if (device) return device;
	}

	lDebug() << "Unable to find invited participant in " << *this << " with "
	         << std::string(linphone_stream_type_to_string(type)) << " label " << label;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDeviceBySsrc(uint32_t ssrc, LinphoneStreamType type) const {
	for (const auto &participant : mParticipants) {
		auto device = participant->findDeviceBySsrc(ssrc, type);
		if (device) {
			return device;
		}
	}

	lDebug() << "Unable to find participant device in " << *this << " with ssrc " << ssrc;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice(const std::shared_ptr<const Address> &pAddr,
                                                                const std::shared_ptr<const Address> &dAddr) const {
	for (const auto &participant : mParticipants) {
		if (pAddr->weakEqual(*participant->getAddress())) {
			auto device = participant->findDevice(dAddr, false);
			if (device) {
				return device;
			}
		}
	}

	lDebug() << "Unable to find participant device in " << *this << " with device address " << *dAddr
	         << " belonging to participant " << *pAddr;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice(const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : mParticipants) {
		auto device = participant->findDevice(session, false);
		if (device) {
			return device;
		}
	}

	lDebug() << "Unable to find participant device in " << *this << " with call session " << session;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::getActiveSpeakerParticipantDevice() const {
	return mActiveSpeakerDevice;
}

std::map<ConferenceMediaCapabilities, bool> Conference::getMediaCapabilities() const {
	std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
	mediaCapabilities[ConferenceMediaCapabilities::Audio] = mConfParams->audioEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Video] = mConfParams->videoEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Text] = mConfParams->chatEnabled();
	return mediaCapabilities;
}

// -----------------------------------------------------------------------------

bool Conference::isMe(const std::shared_ptr<const Address> &addr) const {
	if (!mMe) {
		// Cannot know if it is the me participant if it is not defined.
		// This may happen when a server chat room is retrieved from the database. The me participant is not defined as
		// it has already been created. The server is therefore a passive component whose task is to dispatch MESSAGE
		// request or handle participants.
		lDebug() << *this << ": Unable to known if the me participant is not defined";
		return false;
	}
	if (!addr || !addr->isValid()) {
		lError() << *this << ": Unable to known if an invalid address is the me participant";
		return false;
	}
	Address cleanedAddr = addr->getUriWithoutGruu();
	Address cleanedMeAddr = mMe->getAddress()->getUriWithoutGruu();
	return cleanedMeAddr == cleanedAddr;
}

// -----------------------------------------------------------------------------

void Conference::incrementLastNotify() {
	setLastNotify(mLastNotify + 1);
}

void Conference::setLastNotify(unsigned int lastNotify) {
	mLastNotify = lastNotify;
}

void Conference::resetLastNotify() {
	setLastNotify(0);
}

void Conference::setConferenceId(const ConferenceId &conferenceId) {
	mConferenceId = conferenceId;
	getCore()->insertConference(getSharedFromThis());
}

const ConferenceId &Conference::getConferenceId() const {
	return mConferenceId;
}

std::optional<std::reference_wrapper<const std::string>> Conference::getIdentifier() const {
	if (mState == ConferenceInterface::State::Instantiated) {
		return std::nullopt;
	}
	return mConferenceId.getIdentifier();
}

void Conference::notifyFullState() {
	for (const auto &l : mConfListeners) {
		l->onFullStateReceived();
	}
}
shared_ptr<ConferenceNotifiedEvent> Conference::notifyAllowedParticipantListChanged(time_t creationTime,
                                                                                    const bool isFullState) {
	shared_ptr<ConferenceNotifiedEvent> event = make_shared<ConferenceNotifiedEvent>(
	    EventLog::Type::ConferenceAllowedParticipantListChanged, creationTime, mConferenceId);
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onAllowedParticipantListChanged(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantAdded(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    EventLog::Type::ConferenceParticipantAdded, creationTime, mConferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantAdded(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantRemoved(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    EventLog::Type::ConferenceParticipantRemoved, creationTime, mConferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantRemoved(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent>
Conference::notifyParticipantSetRole(time_t creationTime,
                                     const bool isFullState,
                                     const std::shared_ptr<Participant> &participant,
                                     Participant::Role role) {
	EventLog::Type eventType = EventLog::Type::None;
	switch (role) {
		case Participant::Role::Speaker:
			eventType = EventLog::Type::ConferenceParticipantRoleSpeaker;
			break;
		case Participant::Role::Listener:
			eventType = EventLog::Type::ConferenceParticipantRoleListener;
			break;
		case Participant::Role::Unknown:
			eventType = EventLog::Type::ConferenceParticipantRoleUnknown;
			break;
	}
	shared_ptr<ConferenceParticipantEvent> event =
	    make_shared<ConferenceParticipantEvent>(eventType, creationTime, mConferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantSetRole(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantSetAdmin(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
	    creationTime, mConferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantSetAdmin(event, participant);
	}
	return event;
}

shared_ptr<ConferenceSubjectEvent>
Conference::notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject) {
	shared_ptr<ConferenceSubjectEvent> event =
	    make_shared<ConferenceSubjectEvent>(creationTime, mConferenceId, subject);
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onSubjectChanged(event);
	}
	return event;
}

shared_ptr<ConferenceAvailableMediaEvent> Conference::notifyAvailableMediaChanged(
    time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	shared_ptr<ConferenceAvailableMediaEvent> event =
	    make_shared<ConferenceAvailableMediaEvent>(creationTime, mConferenceId, mediaCapabilities);
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onAvailableMediaChanged(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceAdded(time_t creationTime,
                                         const bool isFullState,
                                         const std::shared_ptr<Participant> &participant,
                                         const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceAdded, creationTime, mConferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceAdded(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceRemoved(time_t creationTime,
                                           const bool isFullState,
                                           const std::shared_ptr<Participant> &participant,
                                           const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceRemoved, creationTime, mConferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceRemoved(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceMediaCapabilityChanged(time_t creationTime,
                                                          const bool isFullState,
                                                          const std::shared_ptr<Participant> &participant,
                                                          const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged, creationTime, mConferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceMediaCapabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceMediaAvailabilityChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged, creationTime, mConferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceMediaAvailabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceJoiningRequest(time_t creationTime,
                                                  const bool isFullState,
                                                  const std::shared_ptr<Participant> &participant,
                                                  const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceJoiningRequest, creationTime, mConferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onParticipantDeviceJoiningRequest(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralModeChanged(time_t creationTime, const bool isFullState, const EventLog::Type type) {
	L_ASSERT((type == EventLog::Type::ConferenceEphemeralMessageManagedByAdmin) ||
	         (type == EventLog::Type::ConferenceEphemeralMessageManagedByParticipants));
	shared_ptr<ConferenceEphemeralMessageEvent> event =
	    make_shared<ConferenceEphemeralMessageEvent>(type, creationTime, mConferenceId, 0);
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onEphemeralModeChanged(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralMessageEnabled(time_t creationTime, const bool isFullState, const bool enable) {

	shared_ptr<ConferenceEphemeralMessageEvent> event =
	    make_shared<ConferenceEphemeralMessageEvent>((enable) ? EventLog::Type::ConferenceEphemeralMessageEnabled
	                                                          : EventLog::Type::ConferenceEphemeralMessageDisabled,
	                                                 creationTime, getConferenceId(), 0);

	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onEphemeralMessageEnabled(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralLifetimeChanged(time_t creationTime, const bool isFullState, const long lifetime) {
	shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
	    EventLog::Type::ConferenceEphemeralMessageLifetimeChanged, creationTime, mConferenceId, lifetime);
	event->setFullState(isFullState);
	event->setNotifyId(mLastNotify);

	for (const auto &l : mConfListeners) {
		l->onEphemeralLifetimeChanged(event);
	}
	return event;
}

bool Conference::isTerminationState(ConferenceInterface::State state) {
	switch (state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::CreationFailed:
		case ConferenceInterface::State::Created:
			return false;
			break;
		case ConferenceInterface::State::TerminationPending:
		case ConferenceInterface::State::Terminated:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Deleted:
			return true;
			break;
	}
	return false;
}

void Conference::setState(ConferenceInterface::State state) {
	try {
		ConferenceInterface::State previousState = getState();
		shared_ptr<Conference> ref = getSharedFromThis();
		LinphoneGlobalState coreState = linphone_core_get_global_state(getCore()->getCCore());
		// Change state if:
		// - current state is not Deleted
		// - current state is Deleted and trying to move to Instantiated state
		if ((previousState != ConferenceInterface::State::Deleted) ||
		    ((previousState == ConferenceInterface::State::Deleted) &&
		     (state == ConferenceInterface::State::Instantiated))) {
			if (mState != state) {
				if (coreState == LinphoneGlobalStartup) {
					lDebug() << "Switching " << *this << " from state " << mState << " to " << state;
				} else {
					lInfo() << "Switching " << *this << " from state " << mState << " to " << state;
				}
				mState = state;
				// Do not notify conference state changes when the core is starting up to avoid the application from
				// being overwhelmed by the notifications The core must be notified of state changes when shutting down
				// to free the last reference to the conference it holds
				if ((coreState == LinphoneGlobalOn) || (coreState == LinphoneGlobalShutdown)) {
					notifyStateChanged(mState);
				}
			}
		}

		if (mState == ConferenceInterface::State::Terminated) {
			onConferenceTerminated(getConferenceAddress());
		} else if (mState == ConferenceInterface::State::Deleted) {
			// If core is in Global Shutdown state, then do not remove it from the map as it will be freed by
			// Core::uninit()
			if (coreState != LinphoneGlobalShutdown) {
				getCore()->deleteConference(ref);
			}
#ifdef HAVE_ADVANCED_IM
			setChatRoom(nullptr);
#endif // HAVE_ADVANCED_IM
		}
	} catch (const bad_weak_ptr &) {
	}
}

void Conference::notifyStateChanged(ConferenceInterface::State state) {
	for (const auto &l : mConfListeners) {
		l->onStateChanged(state);
	}
}

void Conference::notifyActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &participantDevice) {
	mActiveSpeakerDevice = participantDevice;
	for (const auto &l : mConfListeners) {
		l->onActiveSpeakerParticipantDevice(participantDevice);
	}
}

void Conference::setOrganizer(const std::shared_ptr<Address> &organizer) const {
	mOrganizer = organizer->clone()->toSharedPtr();
}

const std::shared_ptr<ConferenceInfo> Conference::createOrGetConferenceInfo() const {
	// Do not create conference infos if the conference doesn't have audio or video capabilities
	// Pure chatrooms do not need a conference information
	mConferenceInfo = nullptr;
	if (!mConfParams->audioEnabled() && !mConfParams->videoEnabled()) return mConferenceInfo;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		if (mConferenceInfoId == -1) {
			mConferenceInfo = mainDb->getConferenceInfoFromURI(getConferenceAddress());
		} else {
			mConferenceInfo = mainDb->getConferenceInfo(mConferenceInfoId);
		}
	}
#endif // HAVE_DB_STORAGE
	if (!mConferenceInfo) {
		mConferenceInfo = createConferenceInfo();
	}
	return mConferenceInfo;
}

std::shared_ptr<ConferenceInfo> Conference::createConferenceInfo() const {
	return nullptr;
}

const std::shared_ptr<ParticipantDevice> Conference::getFocusOwnerDevice() const {
	std::shared_ptr<ParticipantDevice> focusOwnerDevice = nullptr;
	const auto devices = getParticipantDevices();
	const auto deviceIt = std::find_if(devices.cbegin(), devices.cend(), [](const auto &device) {
		return (device->getJoiningMethod() == ParticipantDevice::JoiningMethod::FocusOwner);
	});
	if (deviceIt != devices.cend()) {
		focusOwnerDevice = (*deviceIt);
	}
	return focusOwnerDevice;
}

const std::shared_ptr<ConferenceInfo> Conference::getUpdatedConferenceInfo() const {
	auto conferenceInfo = createOrGetConferenceInfo();
	if (conferenceInfo) {
		const auto focusOwnerDevice = getFocusOwnerDevice();
		if (focusOwnerDevice) {
			const auto &organizer = focusOwnerDevice->getParticipant()->getAddress();
			if (organizer) {
				auto organizerInfo = ParticipantInfo::create(Address::create(organizer->getUri()));
				for (const auto &[name, value] : organizer->getParams()) {
					organizerInfo->addParameter(name, value);
				}
				conferenceInfo->setOrganizer(organizerInfo);
			}
		}

		// Update me only if he/she is already in the list of participants info
		updateParticipantInfoInConferenceInfo(conferenceInfo, getMe());
		for (const auto &participant : getParticipants()) {
			updateParticipantInfoInConferenceInfo(conferenceInfo, participant);
		}

		conferenceInfo->setSecurityLevel(mConfParams->getSecurityLevel());
		conferenceInfo->setUtf8Subject(mConfParams->getUtf8Subject());

		// Update start time and duration as this information can be sent through the SUBSCRIBE/NOTIFY dialog. In fact,
		// if a client dials a conference without prior knowledge (for example it is given an URI to call), the start
		// and end time are initially estimated as there is no conference information associated to that URI.
		time_t startTime = mConfParams->getStartTime();
		time_t endTime = mConfParams->getEndTime();
		conferenceInfo->setDateTime(startTime);
		if ((startTime >= 0) && (endTime >= 0) && (endTime > startTime)) {
			unsigned int duration = (static_cast<unsigned int>(endTime - startTime)) / 60;
			conferenceInfo->setDuration(duration);
		}

		conferenceInfo->setSecurityLevel(mConfParams->getSecurityLevel());
		conferenceInfo->setUtf8Subject(mConfParams->getUtf8Subject());
	}

	return conferenceInfo;
}

std::shared_ptr<ConferenceInfo> Conference::createConferenceInfoWithCustomParticipantList(
    const std::shared_ptr<Address> &organizer,
    const std::list<std::shared_ptr<Participant>> &invitedParticipants) const {
	ConferenceInfo::participant_list_t infos;
	for (const auto &participant : invitedParticipants) {
		const auto &address = participant->getAddress();
		auto info = ParticipantInfo::create(address);
		info->setRole(participant->getRole());
		info->setSequenceNumber(participant->getSequenceNumber());
		for (const auto &[name, value] : address->getParams()) {
			info->addParameter(name, value);
		}
		infos.push_back(info);
	}
	return createConferenceInfoWithCustomParticipantList(organizer, infos);
}
std::shared_ptr<ConferenceInfo> Conference::createConferenceInfoWithCustomParticipantList(
    const std::shared_ptr<Address> &organizer, const ConferenceInfo::participant_list_t &invitedParticipants) const {
	std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
	if (organizer) {
		auto organizerInfo = ParticipantInfo::create(Address::create(organizer->getUri()));
		for (const auto &[name, value] : organizer->getParams()) {
			organizerInfo->addParameter(name, value);
		}
		info->setOrganizer(organizerInfo);
	}
	for (const auto &participant : invitedParticipants) {
		info->addParticipant(participant);
	}

	const auto &conferenceAddress = getConferenceAddress();
	if (conferenceAddress && conferenceAddress->isValid()) {
		info->setUri(conferenceAddress);
	}

	time_t startTime = mConfParams->getStartTime();
	time_t endTime = mConfParams->getEndTime();
	info->setDateTime(startTime);
	if ((startTime >= 0) && (endTime >= 0) && (endTime > startTime)) {
		unsigned int duration = (static_cast<unsigned int>(endTime - startTime)) / 60;
		info->setDuration(duration);
	}

	info->setUtf8Subject(mConfParams->getUtf8Subject());
	info->setSecurityLevel(mConfParams->getSecurityLevel());

	info->setCapability(LinphoneStreamTypeAudio, mConfParams->audioEnabled());
	info->setCapability(LinphoneStreamTypeVideo, mConfParams->videoEnabled());
	info->setCapability(LinphoneStreamTypeText, mConfParams->chatEnabled());

	info->setEarlierJoiningTime(mConfParams->getEarlierJoiningTime());
	info->setExpiryTime(mConfParams->getExpiryTime());

	return info;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateSecurityLevelInConferenceInfo(const ConferenceParams::SecurityLevel &level) const {
#ifdef HAVE_DB_STORAGE
	const auto state = getState();
	if ((state == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			info->setSecurityLevel(level);

			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because its security level has been changed to " << level;
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateSubjectInConferenceInfo(const std::string &subject) const {
#ifdef HAVE_DB_STORAGE
	const auto state = getState();
	if ((state == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			info->setUtf8Subject(subject);

			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because its subject has been changed to " << subject;
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateParticipantRoleInConferenceInfo(const std::shared_ptr<Participant> &participant) const {
#ifdef HAVE_DB_STORAGE
	const auto state = getState();
	if ((state == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			const auto &address = participant->getAddress();
			const auto &newRole = participant->getRole();
			const auto &participantInfo = info->findParticipant(address);
			if (participantInfo) {
				auto newParticipantInfo = participantInfo->clone()->toSharedPtr();
				newParticipantInfo->setRole(newRole);

				info->updateParticipant(newParticipantInfo);

				// Store into DB after the start incoming notification in order to have a valid conference address being
				// the contact address of the call
				auto &mainDb = getCore()->getPrivate()->mainDb;
				if (mainDb) {
					lInfo() << "Updating conference information of conference " << *getConferenceAddress()
					        << " because the role of participant " << *address << " changed to " << newRole;
					mainDb->insertConferenceInfo(info);
				}
			} else {
				lError() << "Unable to update role of participant " << *address << " to " << newRole
				         << " because it cannot be found in the conference info linked to conference "
				         << *getConferenceAddress();
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

bool Conference::updateParticipantInfoInConferenceInfo(std::shared_ptr<ConferenceInfo> &info,
                                                       const std::shared_ptr<Participant> &participant) const {
	bool update = false;
	const auto &participantAddress = participant->getAddress();
	const auto &currentParticipants = info->getParticipants();
	const auto participantInfoIt =
	    std::find_if(currentParticipants.begin(), currentParticipants.end(), [&participantAddress](const auto &p) {
		    return (participantAddress->weakEqual(*p->getAddress()));
	    });

	const auto &participantRole = participant->getRole();
	if (participantInfoIt == currentParticipants.end()) {
		auto participantInfo = ParticipantInfo::create(participantAddress);
		participantInfo->setRole(participantRole);
		participantInfo->setSequenceNumber(participant->getSequenceNumber());
		info->addParticipant(participantInfo);
		update = true;
	} else {
		auto participantInfo = (*participantInfoIt)->clone()->toSharedPtr();
		if (participantInfo->getRole() != participantRole) {
			participantInfo->setRole(participantRole);
			info->updateParticipant(participantInfo);
			update = true;
		}
	}
	return update;
}

void Conference::updateParticipantInConferenceInfo(const std::shared_ptr<Participant> &participant) const {
	const auto &participantAddress = participant->getAddress();
	if (!participant) {
		lError() << "Conference " << *getConferenceAddress()
		         << " received a request to update the conference info to add participant with address "
		         << *participantAddress << " but it looks like he/she is not part of this conference";
		return;
	}

#ifdef HAVE_DB_STORAGE
	const auto state = getState();
	if ((state == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();
		if (info) {
			bool update = updateParticipantInfoInConferenceInfo(info, participant);

			// Store into DB after the start incoming notification in order to have a valid conference address being
			// the contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb && update) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because participant " << *participantAddress
				        << " has been added or has modified its informations";
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}

bool Conference::updateMinatureRequestedFlag() const {
	auto oldMinaturesRequested = thumbnailsRequested;
	int thumbnailAvailableCount = 0;
	for (const auto &p : mParticipants) {
		for (const auto &d : p->getDevices()) {
			auto dir = d->getThumbnailStreamCapability();
			if ((dir == LinphoneMediaDirectionSendOnly) || (dir == LinphoneMediaDirectionSendRecv)) {
				thumbnailAvailableCount++;
			}
		}
	}
	int max_thumbnails = linphone_core_get_conference_max_thumbnails(getCore()->getCCore());
	thumbnailsRequested = (thumbnailAvailableCount <= max_thumbnails) && sessionParamsAllowThumbnails();
	bool changed = (oldMinaturesRequested != thumbnailsRequested);
	for (const auto &p : mParticipants) {
		for (const auto &d : p->getDevices()) {
			// Even if the request of thumbnails has not changed, it may be possible that the stream availabilities have
			// changed if one participant devices stqrts or stops sharing its screen
			auto availabilityChanges = d->updateStreamAvailabilities();
			changed |= (availabilityChanges.find(LinphoneStreamTypeVideo) != availabilityChanges.cend());
		}
	}

	return changed;
}

bool Conference::areThumbnailsRequested(bool update) const {
	if (update) {
		updateMinatureRequestedFlag();
	}
	return thumbnailsRequested;
}

std::pair<bool, LinphoneMediaDirection>
Conference::getMainStreamVideoDirection(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                        BCTBX_UNUSED(bool localIsOfferer),
                                        BCTBX_UNUSED(bool useLocalParams)) const {
	lWarning() << __func__ << " not implemented";
	return std::make_pair(false, LinphoneMediaDirectionInactive);
}

LinphoneMediaDirection Conference::verifyVideoDirection(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                                        const LinphoneMediaDirection suggestedVideoDirection) const {
	// By default do not do anything
	return suggestedVideoDirection;
}

void Conference::setInputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice) {
		const auto &currentInputDevice = getInputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current
		// audio device is set, then return true
		bool change =
		    currentInputDevice ? ((audioDevice != currentInputDevice) || (*audioDevice != *currentInputDevice)) : true;

		if (!change) {
			lInfo() << "Ignoring request to change input audio device of conference " << *getConferenceAddress()
			        << " to [" << audioDevice << "] (" << audioDevice
			        << ") because it is the same as the one currently used";
			return;
		}
		if (audioDevice &&
		    ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) != 0)) {
			AudioControlInterface *aci = getAudioControlInterface();
			if (aci) {
				lInfo() << "Set input audio device [" << audioDevice->toString() << "] (" << audioDevice
				        << ") to audio control interface " << aci << " for conference " << *getConferenceAddress();
				aci->setInputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set input audio device [" << audioDevice->toString() << "] (" << audioDevice
				         << ") of conference " << *getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set input audio device to [" << audioDevice->toString() << "] (" << audioDevice
			         << ") for conference " << *getConferenceAddress() << " due to missing record capability";
		}
	} else {
		lError() << "Unable to set undefined input audio device (" << audioDevice << ") for conference "
		         << *getConferenceAddress();
	}
}

void Conference::setOutputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice) {
		const auto &currentOutputDevice = getOutputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current
		// audio device is set, then return true
		bool change = currentOutputDevice
		                  ? ((audioDevice != currentOutputDevice) || (*audioDevice != *currentOutputDevice))
		                  : true;

		if (!change) {
			lInfo() << "Ignoring request to change output audio device of conference " << *getConferenceAddress()
			        << " to [" << audioDevice->toString() << "] (" << audioDevice
			        << ") because it is the same as the one currently used";
			return;
		}
		if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) != 0) {
			AudioControlInterface *aci = getAudioControlInterface();
			if (aci) {
				lInfo() << "Set output audio device [" << audioDevice->toString() << "] (" << audioDevice
				        << ") to audio control interface " << aci << " for conference " << *getConferenceAddress();
				aci->setOutputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set output audio device [" << audioDevice->toString() << "] (" << audioDevice
				         << ") of conference " << *getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set output audio device to [" << audioDevice->toString() << "] (" << audioDevice
			         << ") for conference " << *getConferenceAddress() << " due to missing play capability";
		}
	} else {
		lError() << "Unable to set undefined output audio device (" << audioDevice << ") for conference "
		         << *getConferenceAddress();
	}
}

int Conference::stopRecording() {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		aci->stopRecording();
	} else {
		lError() << "Conference::stopRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

bool Conference::isRecording() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->isRecording();
	}
	return false;
}

shared_ptr<AudioDevice> Conference::getInputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getInputDevice();
	}

	lError() << "Unable to retrieve input audio device from undefined audio control interface of conference "
	         << *getConferenceAddress();
	return nullptr;
}

shared_ptr<AudioDevice> Conference::getOutputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getOutputDevice();
	}

	lError() << "Unable to retrieve output audio device from undefined audio control interface of conference "
	         << *getConferenceAddress();
	return nullptr;
}

bool Conference::getMicrophoneMuted() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return !aci->micEnabled();
	}
	lError() << "Unable to get status of microphone because the audio control interface of conference "
	         << *getConferenceAddress() << " cannot be found";
	return false;
}

void Conference::setMicrophoneMuted(bool muted) {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		aci->enableMic(!muted);
		for (const auto &participant : mParticipants) {
			for (const auto &device : participant->getDevices()) {
				// If the core is holding a conference (conference server or client holding the conference because it
				// has scheduled a conference without having a conference server set), every participant device has a
				// media session associated to. In such a scenario all calls are muted one by one.
				auto deviceSession = device->getSession();
				if (deviceSession) {
					auto op = deviceSession->getPrivate()->getOp();
					shared_ptr<Call> call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
					if (call) {
						call->setMicrophoneMuted(muted);
					}
				}
			}
		}
		bool coreMicrophoneEnabled = !!linphone_core_mic_enabled(getCore()->getCCore());
		notifyLocalMutedDevices(muted || !coreMicrophoneEnabled);
	} else {
		lError() << "Unable to " << std::string(muted ? "disable" : "enable")
		         << " microphone because the audio control interface of " << *this << " cannot be found";
	}
}

float Conference::getRecordVolume() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getRecordVolume();
	}
	lError() << "Unable to get record volume because the audio control interface of conference "
	         << *getConferenceAddress() << " cannot be found";
	return 0.0;
}

bool Conference::isConferenceExpired() const {
	const auto &expiryTime = mConfParams->getExpiryTime();
	const auto now = ms_time(NULL);
	const auto conferenceExpired = ((expiryTime >= 0) && (expiryTime < now));
	return conferenceExpired;
}

bool Conference::isConferenceEnded() const {
	const auto &endTime = mConfParams->getEndTime();
	const auto now = ms_time(NULL);
	const auto conferenceEnded = (endTime >= 0) && (endTime < now);
	return conferenceEnded;
}

bool Conference::isConferenceAvailable() const {
	const auto &earlierJoiningTime = mConfParams->getEarlierJoiningTime();
	const auto now = ms_time(NULL);
	// negative joining time means immediate start
	const auto conferenceAvailable = (earlierJoiningTime < 0) || (earlierJoiningTime <= now);
	return conferenceAvailable;
}

bool Conference::isConferenceStarted() const {
	const auto &startTime = mConfParams->getStartTime();
	const auto now = ms_time(NULL);
	// negative start time means immediate start
	const auto conferenceStarted = (startTime < 0) || (startTime <= now);
	return conferenceStarted;
}

bool Conference::isSubscriptionUnderWay() const {
	return false;
}

std::shared_ptr<Player> Conference::getPlayer() const {
	lWarning() << "Getting a player is not available for this conference.";
	return nullptr;
}

bool Conference::isChatOnly() const {
	return (!supportsMedia() && mConfParams->chatEnabled());
}

bool Conference::supportsMedia() const {
	return ((mConfParams->audioEnabled() || mConfParams->videoEnabled()));
}

void Conference::setChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom) {
	if (mChatRoom) {
		removeListener(mChatRoom);
	}
	mChatRoom = chatRoom;
	if (mChatRoom) {
		addListener(mChatRoom);
	}
}

const std::shared_ptr<AbstractChatRoom> Conference::getChatRoom() const {
	return mChatRoom;
}

std::unique_ptr<LogContextualizer> Conference::getLogContextualizer() {
	return unique_ptr<LogContextualizer>(new ConferenceLogContextualizer(*this));
}

ConferenceLogContextualizer::~ConferenceLogContextualizer() {
	if (mPushed) bctbx_pop_log_tag(sTagIdentifier);
}

void ConferenceLogContextualizer::pushTag(const Conference &conference) {
	auto address = conference.getConferenceAddress();
	if (address) {
		const char *value = address->getUriParamValueCstr(Conference::ConfIdParameter);
		if (!value) value = address->getUsernameCstr();
		if (!value) value = address->getDomainCstr();
		if (value) {
			bctbx_push_log_tag(sTagIdentifier, value);
			mPushed = true;
		}
	}
}

LINPHONE_END_NAMESPACE
