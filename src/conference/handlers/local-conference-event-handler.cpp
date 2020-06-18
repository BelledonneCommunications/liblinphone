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
#include "conference/participant.h"
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

LocalConferenceEventHandler::LocalConferenceEventHandler (Conference *conference) {
	conf = conference;
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

		for (const auto &device : participant->getDevices()) {
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

	return createNotify(confInfo, true);
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
		conf->setLastNotify(eventNotifyId == -1 ? (conf->getLastNotify()+1) : static_cast<unsigned int>(eventNotifyId));
		switch (eventLog->getType()) {
			case EventLog::Type::ConferenceParticipantAdded: {
				shared_ptr<ConferenceParticipantEvent> addedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdded(
					addedEvent->getParticipantAddress()
				);
			} break;

			case EventLog::Type::ConferenceParticipantRemoved: {
				shared_ptr<ConferenceParticipantEvent> removedEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantRemoved(
					removedEvent->getParticipantAddress()
				);
			} break;

			case EventLog::Type::ConferenceParticipantSetAdmin: {
				shared_ptr<ConferenceParticipantEvent> setAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdminStatusChanged(
					setAdminEvent->getParticipantAddress(),
					true
				);
			} break;

			case EventLog::Type::ConferenceParticipantUnsetAdmin: {
				shared_ptr<ConferenceParticipantEvent> unsetAdminEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				body = createNotifyParticipantAdminStatusChanged(
					unsetAdminEvent->getParticipantAddress(),
					false
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceAdded: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceAddedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				body = createNotifyParticipantDeviceAdded(
					deviceAddedEvent->getParticipantAddress(),
					deviceAddedEvent->getDeviceAddress()
				);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceRemoved: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceRemovedEvent = static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				body = createNotifyParticipantDeviceRemoved(
					deviceRemovedEvent->getParticipantAddress(),
					deviceRemovedEvent->getDeviceAddress()
				);
			} break;

			case EventLog::Type::ConferenceSubjectChanged: {
				shared_ptr<ConferenceSubjectEvent> subjectEvent = static_pointer_cast<ConferenceSubjectEvent>(eventLog);
				body = createNotifySubjectChanged(
					subjectEvent->getSubject()
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

string LocalConferenceEventHandler::createNotifyParticipantAdded (const Address &addr) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);
	UserType user = UserType();
	UserRolesType roles;
	UserType::EndpointSequence endpoints;

	shared_ptr<Participant> p = conf->findParticipant(addr);
	if (p) {
		for (const auto &device : p->getDevices()) {
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

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantAdminStatusChanged (const Address &addr, bool isAdmin) {
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

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantRemoved (const Address &addr) {
	string entity = conf->getConferenceAddress().asString();
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	user.setEntity(addr.asStringUriOnly());
	user.setState(StateType::deleted);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceAdded (const Address &addr, const Address &gruu) {
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
		shared_ptr<ParticipantDevice> device = p->findDevice(gruu);
		if (device) {
			const string &displayName = device->getName();
			if (!displayName.empty())
				endpoint.setDisplayText(displayName);
		}
	}
	endpoint.setState(StateType::full);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceRemoved (const Address &addr, const Address &gruu) {
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

	if (handler->conf) {
		bool allCallReleased = true;
		for (const auto &p : handler->conf->getParticipants()) {
printf("%s - participant %p state %s\n", __func__, p.get(), Utils::toString(p->getSession()->getState()).c_str());
			for (const auto &d : p->getDevices()) {
if (d->getSession()) {
printf("%s - device %p state %s\n", __func__, d.get(), Utils::toString(d->getSession()->getState()).c_str());
} else { printf("%s - no session for device %p\n", __func__, d.get()); }
				if ((d->getConferenceSubscribeEvent() == ev) && (d->getState() == ParticipantDevice::State::Joining)) {
					handler->conf->onFirstNotifyReceived(d->getAddress());
					return;
				}
			}
			// Search as long as one device whose session is not in released state is found
			if (allCallReleased == true) {
				// Find the 1st device whose call session is not in Released state
				// If find_if returns the end of the list, it means that no device has been found
				allCallReleased = (std::find_if(p->getDevices().cbegin(), p->getDevices().cend(), [](const std::shared_ptr<ParticipantDevice> d) { 
					if (d->getSession()) {
						return (d->getSession()->getState() != CallSession::State::Released);
					} else {
						return true;
					}
				 }) == p->getDevices().cend());
			}
printf("%s - all call released %0d\n", __func__, allCallReleased);
		}
printf("%s - END all call released %0d\n", __func__, allCallReleased);
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

void LocalConferenceEventHandler::notifyParticipant (const string &notify, const shared_ptr<Participant> &participant) {
	for (const auto &device : participant->getDevices())
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

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::subscribeReceived (LinphoneEvent *lev, bool oneToOne) {
	const LinphoneAddress *lAddr = linphone_event_get_from(lev);
	char *addrStr = linphone_address_as_string(lAddr);
	unsigned int lastNotify = conf->getLastNotify();
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

void LocalConferenceEventHandler::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const IdentityAddress addr = event->getParticipantAddress();
	shared_ptr<Participant> participant = conf->findParticipant(addr);
	notifyAllExcept(createNotifyParticipantAdded(addr), participant);
}

void LocalConferenceEventHandler::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const IdentityAddress addr = event->getParticipantAddress();
	shared_ptr<Participant> participant = conf->findParticipant(addr);
	notifyAllExcept(createNotifyParticipantRemoved(addr), participant);
}

void LocalConferenceEventHandler::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const IdentityAddress addr = event->getParticipantAddress();
	const bool isAdmin = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	notifyAll(createNotifyParticipantAdminStatusChanged(addr, isAdmin));
}

void LocalConferenceEventHandler::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {
	notifyAll(createNotifySubjectChanged());
}

void LocalConferenceEventHandler::onAvailableMediaChanged (const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {
}

void LocalConferenceEventHandler::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	const IdentityAddress addr = event->getParticipantAddress();
	const IdentityAddress gruu = event->getDeviceAddress();
	notifyAll(createNotifyParticipantDeviceAdded(addr, gruu));
}

void LocalConferenceEventHandler::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	const IdentityAddress addr = event->getParticipantAddress();
	const IdentityAddress gruu = event->getDeviceAddress();
	notifyAll(createNotifyParticipantDeviceRemoved(addr, gruu));
}

LINPHONE_END_NAMESPACE
