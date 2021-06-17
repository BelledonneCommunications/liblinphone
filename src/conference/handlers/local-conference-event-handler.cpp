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

#include <ctime>

#include "linphone/api/c-content.h"
#include "linphone/utils/utils.h"

#include "c-wrapper/c-wrapper.h"
#include "conference/conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "chat/chat-room/server-group-chat-room-p.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"
#include "local-conference-event-handler.h"
#include "logger/logger.h"

// TODO: remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;

// =============================================================================

LocalConferenceEventHandler::LocalConferenceEventHandler (Conference *conference, ConferenceListener *listener): conf(conference), confListener(listener) {
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::notifyFullState (const string &notify, const shared_ptr<ParticipantDevice> &device) {
	notifyParticipantDevice(notify, device);
}

void LocalConferenceEventHandler::notifyAllExcept (const string &notify, const shared_ptr<Participant> &exceptParticipant) {
	for (const auto &participant : conf->getParticipants()) {
		if (participant != exceptParticipant)
			notifyParticipant(notify, participant);
	}
}

void LocalConferenceEventHandler::notifyAll (const string &notify) {
	for (const auto &participant : conf->getParticipants())
		notifyParticipant(notify, participant);
}

string LocalConferenceEventHandler::createNotifyFullState (bool oneToOne) {
	string entity = conf->getConferenceAddress().asString();
	string subject = conf->getSubject();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	const auto & confParams = conf->getCurrentParams();
	const auto & audioEnabled = confParams.audioEnabled();
	const LinphoneMediaDirection audioDirection = audioEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto & videoEnabled = confParams.videoEnabled();
	const LinphoneMediaDirection videoDirection = videoEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto & textEnabled = confParams.chatEnabled();
	const LinphoneMediaDirection textDirection = textEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	addAvailableMediaCapabilities(audioDirection, videoDirection, textDirection, confDescr);
	if (oneToOne) {
		KeywordsType keywords(sizeof(char), "one-to-one");
		confDescr.setKeywords(keywords);
	}
	confInfo.setUsers(users);
	confInfo.setConferenceDescription((const ConferenceDescriptionType) confDescr);

	std::list<std::shared_ptr<Participant>> participants(conf->getParticipants());

	// Add local participant only if it is enabled
	if (confParams.localParticipantEnabled()) {
		std::shared_ptr<Participant> me = conf->getMe();
		if (me) {
			participants.push_front(me);
		}
	}

	for (const auto &participant : participants) {
		UserType user = UserType();
		UserRolesType roles;
		UserType::EndpointSequence endpoints;
		user.setRoles(roles);
		user.setEndpoint(endpoints);
		user.setEntity(participant->getAddress().asString());
		user.getRoles()->getEntry().push_back(participant->isAdmin() ? "admin" : "participant");
		user.setState(StateType::full);

		for (const auto &device : participant->getDevices()) {
			const string &gruu = device->getAddress().asString();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(device, endpoint);

			endpoint.setState(StateType::full);
			user.getEndpoint().push_back(endpoint);
		}

		confInfo.getUsers()->getUser().push_back(user);
	}

	return createNotify(confInfo, true);
}

void LocalConferenceEventHandler::addAvailableMediaCapabilities(const LinphoneMediaDirection audioDirection, const LinphoneMediaDirection videoDirection, const LinphoneMediaDirection textDirection, ConferenceDescriptionType & confDescr) {
	ConferenceMediaType mediaType;
	ConferenceMediumType audio("audio", "1");
	audio.setDisplayText("audio");
	audio.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(audioDirection));
	mediaType.getEntry().push_back(audio);

	ConferenceMediumType video("video", "2");
	video.setDisplayText("video");
	video.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(videoDirection));
	mediaType.getEntry().push_back(video);

	ConferenceMediumType text("text", "3");
	text.setDisplayText("text");
	text.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(textDirection));
	mediaType.getEntry().push_back(text);
	confDescr.setAvailableMedia(mediaType);

}


void LocalConferenceEventHandler::addMediaCapabilities(const std::shared_ptr<ParticipantDevice> & device, EndpointType & endpoint) {
	const auto &audioDirection = device->getAudioDirection();
	MediaType audio = MediaType("1");
	audio.setDisplayText("audio");
	audio.setType("audio");
	audio.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(audioDirection));
	endpoint.getMedia().push_back(audio);

	const auto &videoDirection = device->getVideoDirection();
	MediaType video = MediaType("2");
	video.setDisplayText("video");
	video.setType("video");
	video.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(videoDirection));
	endpoint.getMedia().push_back(video);

	const auto &textDirection = device->getTextDirection();
	MediaType text = MediaType("3");
	text.setDisplayText("text");
	text.setType("text");
	text.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(textDirection));
	endpoint.getMedia().push_back(text);

}

string LocalConferenceEventHandler::createNotifyMultipart (int notifyId) {
	list<shared_ptr<EventLog>> events = conf->getCore()->getPrivate()->mainDb->getConferenceNotifiedEvents(
		ConferenceId(conf->getConferenceAddress(), conf->getConferenceAddress()),
		static_cast<unsigned int>(notifyId)
	);

	list<Content> contents;
	for (const auto &eventLog : events) {
		string body;
		shared_ptr<ConferenceNotifiedEvent> notifiedEvent = static_pointer_cast<ConferenceNotifiedEvent>(eventLog);
		int eventNotifyId = static_cast<int>(notifiedEvent->getNotifyId());
		conf->setLastNotify(eventNotifyId == -1 ? (conf->getLastNotify()+1) : static_cast<unsigned int>(eventNotifyId));

		switch (eventLog->getType()) {
			case EventLog::Type::ConferenceParticipantAdded: {
				shared_ptr<ConferenceParticipantEvent> addedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const Address & participantAddress = addedEvent->getParticipantAddress().asAddress();
				body = createNotifyParticipantAdded(
					participantAddress
				);
			} break;

			case EventLog::Type::ConferenceParticipantRemoved: {
				shared_ptr<ConferenceParticipantEvent> removedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const Address & participantAddress = removedEvent->getParticipantAddress().asAddress();
				body = createNotifyParticipantRemoved(
					participantAddress
				);
			} break;

			case EventLog::Type::ConferenceParticipantSetAdmin: {
				shared_ptr<ConferenceParticipantEvent> setAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const Address & participantAddress = setAdminEvent->getParticipantAddress().asAddress();
				body = createNotifyParticipantAdminStatusChanged(
					participantAddress,
					true
				);
			} break;

			case EventLog::Type::ConferenceParticipantUnsetAdmin: {
				shared_ptr<ConferenceParticipantEvent> unsetAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const Address & participantAddress = unsetAdminEvent->getParticipantAddress().asAddress();
				body = createNotifyParticipantAdminStatusChanged(
					participantAddress,
					false
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceAdded: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceAddedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const Address & participantAddress = deviceAddedEvent->getParticipantAddress().asAddress();
				const Address & deviceAddress = deviceAddedEvent->getDeviceAddress().asAddress();
				body = createNotifyParticipantDeviceAdded(
					participantAddress,
					deviceAddress
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceRemoved: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceRemovedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const Address & participantAddress = deviceRemovedEvent->getParticipantAddress().asAddress();
				const Address & deviceAddress = deviceRemovedEvent->getDeviceAddress().asAddress();
				body = createNotifyParticipantDeviceRemoved(
					participantAddress,
					deviceAddress
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceMediaChanged: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceMediaChangedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const Address & participantAddress = deviceMediaChangedEvent->getParticipantAddress().asAddress();
				const Address & deviceAddress = deviceMediaChangedEvent->getDeviceAddress().asAddress();
				body = createNotifyParticipantDeviceMediaChanged(
					participantAddress,
					deviceAddress
				);
			} break;

			case EventLog::Type::ConferenceSubjectChanged: {
				shared_ptr<ConferenceSubjectEvent> subjectEvent = static_pointer_cast<ConferenceSubjectEvent>(eventLog);
				body = createNotifySubjectChanged(
					subjectEvent->getSubject()
				);
			} break;

			case EventLog::Type::ConferenceAvailableMediaChanged: {
				shared_ptr<ConferenceAvailableMediaEvent> availableMediaEvent = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog);
				body = createNotifyAvailableMediaChanged(
					availableMediaEvent->getAvailableMediaType()
				);
			} break;

			default:
				// We should never pass here!
				L_ASSERT(false);
				continue;
		}
		contents.emplace_back(Content());
		contents.back().setContentType(ContentType::ConferenceInfo);
		contents.back().setBodyFromUtf8(body);
	}

	if (contents.empty())
		return Utils::getEmptyConstRefObject<string>();

	list<Content *> contentPtrs;
	for (auto &content : contents)
		contentPtrs.push_back(&content);
	string multipart = ContentManager::contentListToMultipart(contentPtrs).getBodyAsUtf8String();
	return multipart;
}

string LocalConferenceEventHandler::createNotifyParticipantAdded (const Address & pAddress) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);
	UserType user = UserType();
	UserRolesType roles;
	UserType::EndpointSequence endpoints;

	shared_ptr<Participant> participant = conf->findParticipant(pAddress);
	if (participant) {
		for (const auto &device : participant->getDevices()) {
			const string &gruu = device->getAddress().asString();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(device, endpoint);

			endpoint.setState(StateType::full);
			user.getEndpoint().push_back(endpoint);
		}
	}

	user.setRoles(roles);
	user.setEntity(pAddress.asStringUriOnly());
	user.getRoles()->getEntry().push_back("participant");
	user.setState(StateType::full);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantAdminStatusChanged (const Address & pAddress, bool isAdmin) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserRolesType roles;
	user.setRoles(roles);
	user.setEntity(pAddress.asStringUriOnly());
	user.getRoles()->getEntry().push_back(isAdmin ? "admin" : "participant");
	user.setState(StateType::partial);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantRemoved (const Address & pAddress) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	user.setEntity(pAddress.asStringUriOnly());
	user.setState(StateType::deleted);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

MediaStatusType LocalConferenceEventHandler::mediaDirectionToMediaStatus (LinphoneMediaDirection direction) {
	switch (direction) {
		case LinphoneMediaDirectionInactive:
			return MediaStatusType::inactive;
		case LinphoneMediaDirectionSendOnly:
			return MediaStatusType::sendonly;
		case LinphoneMediaDirectionRecvOnly:
			return MediaStatusType::recvonly;
		case LinphoneMediaDirectionSendRecv:
			return MediaStatusType::sendrecv;
		case LinphoneMediaDirectionInvalid:
			lError() << "LinphoneMediaDirectionInvalid shall not be used";
			return MediaStatusType::inactive;
	}
	return MediaStatusType::sendrecv;
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceAdded (const Address & pAddress, const Address & dAddress) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress.asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress.asStringUriOnly());
	shared_ptr<Participant> participant = conf->findParticipant(pAddress);
	if (participant) {
		shared_ptr<ParticipantDevice> participantDevice = participant->findDevice(dAddress);
		if (participantDevice) {
			const string &displayName = participantDevice->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(participantDevice, endpoint);
		}
	}
	endpoint.setState(StateType::full);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceRemoved (const Address & pAddress, const Address & dAddress) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress.asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress.asStringUriOnly());

	endpoint.setState(StateType::deleted);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceMediaChanged (const Address & pAddress, const Address & dAddress) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress.asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress.asStringUriOnly());
	shared_ptr<Participant> participant = conf->findParticipant(pAddress);
	if (participant) {
		shared_ptr<ParticipantDevice> participantDevice = participant->findDevice(dAddress);
		if (participantDevice) {
			const string &displayName = participantDevice->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(participantDevice, endpoint);

		}
	}
	endpoint.setState(StateType::full);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}


string LocalConferenceEventHandler::createNotifySubjectChanged () {
	return createNotifySubjectChanged(conf->getSubject());
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::notifyResponseCb (const LinphoneEvent *ev) {
	LinphoneEventCbs *cbs = linphone_event_get_callbacks(ev);
	LocalConferenceEventHandler *handler = reinterpret_cast<LocalConferenceEventHandler *>(
		linphone_event_cbs_get_user_data(cbs)
	);
	linphone_event_cbs_set_user_data(cbs, nullptr);
	linphone_event_cbs_set_notify_response(cbs, nullptr);

	if (linphone_event_get_reason(ev) != LinphoneReasonNone)
		return;

	if (handler && handler->conf) {
		LinphonePrivate::ConferenceInterface::State confState = handler->conf->getState();
		if ((confState != ConferenceInterface::State::Deleted) && (confState != ConferenceInterface::State::Terminated)) {
			if (handler->confListener) {
				for (const auto &p : handler->conf->getParticipants()) {
					for (const auto &d : p->getDevices()) {
						if ((d->getConferenceSubscribeEvent() == ev) && (d->getState() == ParticipantDevice::State::Joining)) {
							//fixme confListener should be removed in the futur. On only relevant for server group chatroom
							handler->confListener->onFirstNotifyReceived(d->getAddress());
							return;
						}
					}
				}
			}
		}

	} else {
		lInfo() << "Unable to process event " << ev << " because conference was likely already terminated.";
	}
}

// -----------------------------------------------------------------------------

string LocalConferenceEventHandler::createNotify (ConferenceType confInfo, bool isFullState) {
	confInfo.setVersion(conf->getLastNotify());
	confInfo.setState(isFullState ? StateType::full : StateType::partial);

	if (!confInfo.getConferenceDescription()) {
		ConferenceDescriptionType description = ConferenceDescriptionType();
		confInfo.setConferenceDescription(description);
	}

	time_t result = time(nullptr);
	confInfo.getConferenceDescription()->setFreeText(Utils::toString(static_cast<long>(result)));

	stringstream notify;
	Xsd::XmlSchema::NamespaceInfomap map;
	map[""].name = "urn:ietf:params:xml:ns:conference-info";
	serializeConferenceInfo(notify, confInfo, map);
	return notify.str();
}

string LocalConferenceEventHandler::createNotifySubjectChanged (const string &subject) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyAvailableMediaChanged (const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	LinphoneMediaDirection audioDirection = LinphoneMediaDirectionInactive;
	try {
		const auto & audioEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Audio);
		audioDirection = audioEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range&) {

	}
	LinphoneMediaDirection videoDirection = LinphoneMediaDirectionInactive;
	try {
		const auto & videoEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Video);
		videoDirection = videoEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range&) {

	}
	LinphoneMediaDirection textDirection = LinphoneMediaDirectionInactive;
	try {
		const auto & textEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Text);
		textDirection = textEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range&) {

	}
	addAvailableMediaCapabilities(audioDirection, videoDirection, textDirection, confDescr);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}


void LocalConferenceEventHandler::notifyParticipant (const string &notify, const shared_ptr<Participant> &participant) {
	for (const auto &device : participant->getDevices()){
		/* Only notify to device that are present in the conference. */
		switch(device->getState()){
			case ParticipantDevice::State::Present:
			case ParticipantDevice::State::Joining:
			case ParticipantDevice::State::ScheduledForJoining:
				notifyParticipantDevice(notify, device);
				break;
			case ParticipantDevice::State::Leaving:
			case ParticipantDevice::State::Left:
			case ParticipantDevice::State::ScheduledForLeaving:
				break;
		}
	}
}

void LocalConferenceEventHandler::notifyParticipantDevice (const string &notify, const shared_ptr<ParticipantDevice> &device, bool multipart) {
	if (!device->isSubscribedToConferenceEventPackage() || notify.empty())
		return;

	LinphoneEvent *ev = device->getConferenceSubscribeEvent();
	LinphoneEventCbs *cbs = linphone_event_get_callbacks(ev);
	linphone_event_cbs_set_user_data(cbs, this);
	linphone_event_cbs_set_notify_response(cbs, notifyResponseCb);

	Content content;
	content.setBodyFromUtf8(notify);
	ContentType contentType;
	if (multipart) {
		contentType = ContentType(ContentType::Multipart);
		contentType.addParameter("boundary", MultipartBoundary);
	} else
		contentType = ContentType(ContentType::ConferenceInfo);

	content.setContentType(contentType);
	if (linphone_core_content_encoding_supported(conf->getCore()->getCCore(), "deflate"))
		content.setContentEncoding("deflate");
	LinphoneContent *cContent = L_GET_C_BACK_PTR(&content);
	linphone_event_notify(ev, cContent);
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::subscribeReceived (LinphoneEvent *lev, bool oneToOne) {
	const LinphoneAddress *lAddr = linphone_event_get_from(lev);
	char *addrStr = linphone_address_as_string(lAddr);
	Address participantAddress(addrStr);
	bctbx_free(addrStr);
	unsigned int lastNotify = conf->getLastNotify();

	shared_ptr<Participant> participant = getConferenceParticipant (participantAddress);
	if (!participant) {
		ConferenceAddress conferenceAddress = conf->getConferenceAddress();
		lError() << "Received SUBSCRIBE corresponds to no participant of the conference [" << conferenceAddress << "], no NOTIFY sent";
		linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
		return;
	}

	const LinphoneAddress *lContactAddr = linphone_event_get_remote_contact(lev);
	char *contactAddrStr = linphone_address_as_string(lContactAddr);
	IdentityAddress contactAddr(contactAddrStr);
	bctbx_free(contactAddrStr);
	shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
	if (!device || (device->getState() != ParticipantDevice::State::Present && device->getState() != ParticipantDevice::State::Joining)) {
		lError() << "Received SUBSCRIBE for conference [" << conf->getConferenceAddress()
			<< "], device sending subscribe [" << contactAddr << "] is not known, no NOTIFY sent";
		linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
		return;
	}

	linphone_event_accept_subscription(lev);
	if (linphone_event_get_subscription_state(lev) == LinphoneSubscriptionActive) {
		unsigned int evLastNotify = static_cast<unsigned int>(Utils::stoi(linphone_event_get_custom_header(lev, "Last-Notify-Version")));
		device->setConferenceSubscribeEvent(lev);
		if (evLastNotify == 0 || (device->getState() == ParticipantDevice::State::Joining)) {
			lInfo() << "Sending initial notify of conference [" << conf->getConferenceAddress() << "] to: " << device->getAddress();
			notifyFullState(createNotifyFullState(oneToOne), device);
		} else if (evLastNotify < lastNotify) {
			lInfo() << "Sending all missed notify [" << evLastNotify << "-" << lastNotify <<
				"] for conference [" << conf->getConferenceAddress() << "] to: " << participant->getAddress();
			notifyParticipantDevice(createNotifyMultipart(static_cast<int>(evLastNotify)), device, true);
		} else if (evLastNotify > lastNotify) {
			lError() << "Last notify received by client [" << evLastNotify << "] for conference [" <<
				conf->getConferenceAddress() <<
				"] should not be higher than last notify sent by server [" << lastNotify << "]";
		}
	}
}

void LocalConferenceEventHandler::subscriptionStateChanged (LinphoneEvent *lev, LinphoneSubscriptionState state) {
	if (state == LinphoneSubscriptionTerminated && conf) {
		const LinphoneAddress *lAddr = linphone_event_get_from(lev);
		char *addrStr = linphone_address_as_string(lAddr);
		Address participantAddress(addrStr);
		shared_ptr<Participant> participant = getConferenceParticipant (participantAddress);
		bctbx_free(addrStr);
		if (!participant)
			return;
		const LinphoneAddress *lContactAddr = linphone_event_get_remote_contact(lev);
		char *contactAddrStr = linphone_address_as_string(lContactAddr);
		IdentityAddress contactAddr(contactAddrStr);
		bctbx_free(contactAddrStr);
		shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
		if (!device)
			return;
		lInfo() << "End of subscription for device [" << device->getAddress()
			<< "] of conference [" << conf->getConferenceAddress() << "]";
		device->setConferenceSubscribeEvent(nullptr);
	}
}

string LocalConferenceEventHandler::getNotifyForId (int notifyId, bool oneToOne) {
	unsigned int lastNotify = conf->getLastNotify();
	if (notifyId == 0)
		return createNotifyFullState(oneToOne);
	else if (notifyId < static_cast<int>(lastNotify))
		return createNotifyMultipart(notifyId);

	return Utils::getEmptyConstRefObject<string>();
}

void LocalConferenceEventHandler::onFullStateReceived () {
}

void LocalConferenceEventHandler::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAllExcept(createNotifyParticipantAdded(participant->getAddress().asAddress()), participant);
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress() << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAllExcept(createNotifyParticipantRemoved(participant->getAddress().asAddress()), participant);
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress() << " being removed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	const bool isAdmin = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(createNotifyParticipantAdminStatusChanged(participant->getAddress().asAddress(), isAdmin));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress() << " admin status changed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(createNotifySubjectChanged());
	} else {
		lWarning() << __func__ << ": Not sending notification of conference subject change because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onAvailableMediaChanged (const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(createNotifyAvailableMediaChanged(event->getAvailableMediaType()));
	} else {
		lWarning() << __func__ << ": Not sending notification of conference subject change because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		Participant *participant = device->getParticipant();
		notifyAll(createNotifyParticipantDeviceAdded(participant->getAddress().asAddress(), device->getAddress().asAddress()));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress() << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		Participant *participant = device->getParticipant();
		notifyAll(createNotifyParticipantDeviceRemoved(participant->getAddress().asAddress(), device->getAddress().asAddress()));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress() << " being removed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceMediaChanged (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		Participant *participant = device->getParticipant();
		notifyAll(createNotifyParticipantDeviceMediaChanged(participant->getAddress().asAddress(), device->getAddress().asAddress()));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress() << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onStateChanged (LinphonePrivate::ConferenceInterface::State state) {
}

shared_ptr<Participant> LocalConferenceEventHandler::getConferenceParticipant (const Address & address) const {
	shared_ptr<Core> core = conf->getCore();
	ConferenceAddress conferenceAddress = conf->getConferenceAddress();
	ConferenceId conferenceId(conferenceAddress, conferenceAddress);

	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom (conferenceId);
	std::shared_ptr<LinphonePrivate::ServerGroupChatRoom> sgcr = nullptr;
	if (chatRoom && (chatRoom->getConference().get() == conf)) {
		sgcr = dynamic_pointer_cast<LinphonePrivate::ServerGroupChatRoom>(chatRoom);
	}

	shared_ptr<Participant> participant = nullptr;
	if (sgcr) {
		// If conference belongs to a server group chat room, then search in the cached participants
		participant = sgcr->findCachedParticipant(address);
	} else {
		participant = conf->findParticipant(address);
	}

	return participant;
}
LINPHONE_END_NAMESPACE
