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
#include "conference/local-conference.h"
#include "conference/participant-device.h"
#include "conference/participant-p.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"
#include "local-conference-event-handler.h"
#include "logger/logger.h"
#include "object/object-p.h"

// TODO: remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;

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

string LocalConferenceEventHandler::createNotifyFullState (int notifyId, bool oneToOne) {
	string entity = conf->getConferenceAddress().asString();
	string subject = conf->getSubject();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	if (oneToOne) {
		KeywordsType keywords(sizeof(char), "one-to-one");
		confDescr.setKeywords(keywords);
	}
	confInfo.setUsers(users);
	confInfo.setConferenceDescription((const ConferenceDescriptionType) confDescr);

	for (const auto &participant : conf->getParticipants()) {
		UserType user = UserType();
		UserRolesType roles;
		UserType::EndpointSequence endpoints;
		user.setRoles(roles);
		user.setEndpoint(endpoints);
		user.setEntity(participant->getAddress().asString());
		user.getRoles()->getEntry().push_back(participant->isAdmin() ? "admin" : "participant");
		user.setState(StateType::full);

		for (const auto &device : participant->getPrivate()->getDevices()) {
			const string &gruu = device->getAddress().asString();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			endpoint.setState(StateType::full);
			user.getEndpoint().push_back(endpoint);
		}

		confInfo.getUsers()->getUser().push_back(user);
	}

	return createNotify(confInfo, notifyId, true);
}

string LocalConferenceEventHandler::createNotifyMultipart (int notifyId) {
	list<shared_ptr<EventLog>> events = conf->getCore()->getPrivate()->mainDb->getConferenceNotifiedEvents(
		ConferenceId(conf->getConferenceAddress(), conf->getConferenceAddress()),
		static_cast<unsigned int>(notifyId)
	);

	list<Content> contents;
	for (const auto &eventLog : events) {
		Content *content = new Content();
		content->setContentType(ContentType::ConferenceInfo);
		string body;
		shared_ptr<ConferenceNotifiedEvent> notifiedEvent = static_pointer_cast<ConferenceNotifiedEvent>(eventLog);
		int eventNotifyId = static_cast<int>(notifiedEvent->getNotifyId());
		switch (eventLog->getType()) {
			case EventLog::Type::ConferenceParticipantAdded: {
				shared_ptr<ConferenceParticipantEvent> addedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdded(
					addedEvent->getParticipantAddress(),
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceParticipantRemoved: {
				shared_ptr<ConferenceParticipantEvent> removedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantRemoved(
					removedEvent->getParticipantAddress(),
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceParticipantSetAdmin: {
				shared_ptr<ConferenceParticipantEvent> setAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdminStatusChanged(
					setAdminEvent->getParticipantAddress(),
					true,
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceParticipantUnsetAdmin: {
				shared_ptr<ConferenceParticipantEvent> unsetAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdminStatusChanged(
					unsetAdminEvent->getParticipantAddress(),
					false,
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceAdded: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceAddedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				body = createNotifyParticipantDeviceAdded(
					deviceAddedEvent->getParticipantAddress(),
					deviceAddedEvent->getDeviceAddress(),
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceRemoved: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceRemovedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				body = createNotifyParticipantDeviceRemoved(
					deviceRemovedEvent->getParticipantAddress(),
					deviceRemovedEvent->getDeviceAddress(),
					eventNotifyId
				);
			} break;

			case EventLog::Type::ConferenceSubjectChanged: {
				shared_ptr<ConferenceSubjectEvent> subjectEvent = static_pointer_cast<ConferenceSubjectEvent>(eventLog);
				body = createNotifySubjectChanged(
					subjectEvent->getSubject(),
					eventNotifyId
				);
			} break;

			default:
				// We should never pass here!
				L_ASSERT(false);
				continue;
		}
		contents.emplace_back(Content());
		contents.back().setContentType(ContentType::ConferenceInfo);
		contents.back().setBody(body);
	}

	if (contents.empty())
		return Utils::getEmptyConstRefObject<string>();

	list<Content *> contentPtrs;
	for (auto &content : contents)
		contentPtrs.push_back(&content);
	string multipart = ContentManager::contentListToMultipart(contentPtrs).getBodyAsUtf8String();
	return multipart;
}

string LocalConferenceEventHandler::createNotifyParticipantAdded (const Address &addr, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);
	UserType user = UserType();
	UserRolesType roles;
	UserType::EndpointSequence endpoints;

	shared_ptr<Participant> p = conf->findParticipant(addr);
	if (p) {
		for (const auto &device : p->getPrivate()->getDevices()) {
			const string &gruu = device->getAddress().asString();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);

			endpoint.setState(StateType::full);
			user.getEndpoint().push_back(endpoint);
		}
	}

	user.setRoles(roles);
	user.setEntity(addr.asStringUriOnly());
	user.getRoles()->getEntry().push_back("participant");
	user.setState(StateType::full);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo, notifyId);
}

string LocalConferenceEventHandler::createNotifyParticipantAdminStatusChanged (const Address &addr, bool isAdmin, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserRolesType roles;
	user.setRoles(roles);
	user.setEntity(addr.asStringUriOnly());
	user.getRoles()->getEntry().push_back(isAdmin ? "admin" : "participant");
	user.setState(StateType::partial);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo, notifyId);
}

string LocalConferenceEventHandler::createNotifyParticipantRemoved (const Address &addr, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	user.setEntity(addr.asStringUriOnly());
	user.setState(StateType::deleted);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo, notifyId);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceAdded (const Address &addr, const Address &gruu, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(addr.asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(gruu.asStringUriOnly());
	shared_ptr<Participant> p = conf->findParticipant(addr);
	if (p) {
		shared_ptr<ParticipantDevice> device = p->getPrivate()->findDevice(gruu);
		if (device) {
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);
		}
	}
	endpoint.setState(StateType::full);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo, notifyId);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceRemoved (const Address &addr, const Address &gruu, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(addr.asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(gruu.asStringUriOnly());
	endpoint.setState(StateType::deleted);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo, notifyId);
}

string LocalConferenceEventHandler::createNotifySubjectChanged (int notifyId) {
	return createNotifySubjectChanged(conf->getSubject(), notifyId);
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

	for (const auto &p : handler->conf->getParticipants()) {
		for (const auto &d : p->getPrivate()->getDevices()) {
			if ((d->getConferenceSubscribeEvent() == ev) && (d->getState() == ParticipantDevice::State::Joining)) {
				handler->conf->onFirstNotifyReceived(d->getAddress());
				return;
			}
		}
	}
}

// -----------------------------------------------------------------------------

string LocalConferenceEventHandler::createNotify (ConferenceType confInfo, int notifyId, bool isFullState) {
	confInfo.setVersion(notifyId == -1 ? ++lastNotify : static_cast<unsigned int>(notifyId));
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

string LocalConferenceEventHandler::createNotifySubjectChanged (const string &subject, int notifyId) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo, notifyId);
}

void LocalConferenceEventHandler::notifyParticipant (const string &notify, const shared_ptr<Participant> &participant) {
	for (const auto &device : participant->getPrivate()->getDevices())
		notifyParticipantDevice(notify, device);
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

// =============================================================================

LocalConferenceEventHandler::LocalConferenceEventHandler (LocalConference *localConference, unsigned int notify) :
	Object(*new ObjectPrivate) {
	conf = localConference;
	lastNotify = notify;
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::subscribeReceived (LinphoneEvent *lev, bool oneToOne) {
	const LinphoneAddress *lAddr = linphone_event_get_from(lev);
	char *addrStr = linphone_address_as_string(lAddr);
	shared_ptr<Participant> participant = conf->findParticipant(Address(addrStr));
	bctbx_free(addrStr);
	if (!participant) {
		lError() << "Received SUBSCRIBE corresponds to no participant of the conference [" << conf->getConferenceAddress() << "], no NOTIFY sent";
		linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
		return;
	}

	const LinphoneAddress *lContactAddr = linphone_event_get_remote_contact(lev);
	char *contactAddrStr = linphone_address_as_string(lContactAddr);
	IdentityAddress contactAddr(contactAddrStr);
	bctbx_free(contactAddrStr);
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(contactAddr);
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
			notifyFullState(createNotifyFullState(static_cast<int>(lastNotify), oneToOne), device);
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
	if (state == LinphoneSubscriptionTerminated) {
		const LinphoneAddress *lAddr = linphone_event_get_from(lev);
		char *addrStr = linphone_address_as_string(lAddr);
		shared_ptr<Participant> participant = conf->findParticipant(Address(addrStr));
		bctbx_free(addrStr);
		if (!participant)
			return;
		const LinphoneAddress *lContactAddr = linphone_event_get_remote_contact(lev);
		char *contactAddrStr = linphone_address_as_string(lContactAddr);
		IdentityAddress contactAddr(contactAddrStr);
		bctbx_free(contactAddrStr);
		shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(contactAddr);
		if (!device)
			return;
		lInfo() << "End of subscription for device [" << device->getAddress()
			<< "] of conference [" << conf->getConferenceAddress() << "]";
		device->setConferenceSubscribeEvent(nullptr);
	}
}

shared_ptr<ConferenceParticipantEvent> LocalConferenceEventHandler::notifyParticipantAdded (const Address &addr) {
	shared_ptr<Participant> participant = conf->findParticipant(addr);
	notifyAllExcept(createNotifyParticipantAdded(addr), participant);
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConferenceEventHandler::notifyParticipantRemoved (const Address &addr) {
	shared_ptr<Participant> participant = conf->findParticipant(addr);
	notifyAllExcept(createNotifyParticipantRemoved(addr), participant);
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConferenceEventHandler::notifyParticipantSetAdmin (const Address &addr, bool isAdmin) {
	notifyAll(createNotifyParticipantAdminStatusChanged(addr, isAdmin));
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceSubjectEvent> LocalConferenceEventHandler::notifySubjectChanged () {
	notifyAll(createNotifySubjectChanged());
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
		time(nullptr),
		conferenceId,
		lastNotify,
		conf->getSubject()
	);
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConferenceEventHandler::notifyParticipantDeviceAdded (const Address &addr, const Address &gruu) {
	notifyAll(createNotifyParticipantDeviceAdded(addr, gruu));
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConferenceEventHandler::notifyParticipantDeviceRemoved (const Address &addr, const Address &gruu) {
	notifyAll(createNotifyParticipantDeviceRemoved(addr, gruu));
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	return event;
}

void LocalConferenceEventHandler::setLastNotify (unsigned int newLastNotify) {
	lastNotify = newLastNotify;
}

void LocalConferenceEventHandler::setConferenceId (const ConferenceId &newConferenceId) {
	conferenceId = newConferenceId;
}

const ConferenceId &LocalConferenceEventHandler::getConferenceId () const {
	return conferenceId;
}

string LocalConferenceEventHandler::getNotifyForId (int notifyId, bool oneToOne) {
	if (notifyId == 0)
		return createNotifyFullState(static_cast<int>(lastNotify), oneToOne);
	else if (notifyId < static_cast<int>(lastNotify))
		return createNotifyMultipart(notifyId);

	return Utils::getEmptyConstRefObject<string>();
}

LINPHONE_END_NAMESPACE
