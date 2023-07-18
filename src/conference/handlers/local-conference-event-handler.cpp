/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include <ctime>

#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/server-group-chat-room-p.h"
#include "conference/conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "conference_private.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"
#include "linphone/api/c-content.h"
#include "linphone/utils/utils.h"
#include "local-conference-event-handler.h"
#include "logger/logger.h"

#include <xsd/cxx/xml/dom/serialization-source.hxx>

// TODO: remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::ConferenceInfoLinphoneExtension;

// =============================================================================

LocalConferenceEventHandler::LocalConferenceEventHandler(Conference *conference, ConferenceListener *listener)
    : conf(conference), confListener(listener) {
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::notifyFullState(const Content &notify, const shared_ptr<ParticipantDevice> &device) {
	notifyParticipantDevice(notify, device);
}

void LocalConferenceEventHandler::notifyAllExceptDevice(const Content &notify,
                                                        const shared_ptr<ParticipantDevice> &exceptDevice) {
	for (const auto &participant : conf->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			if (device != exceptDevice) {
				/* Only notify to device that are present in the conference. */
				notifyParticipantDevice(notify, device);
			}
		}
	}
}

void LocalConferenceEventHandler::notifyAllExcept(const Content &notify,
                                                  const shared_ptr<Participant> &exceptParticipant) {
	for (const auto &participant : conf->getParticipants()) {
		if (participant != exceptParticipant) {
			notifyParticipant(notify, participant);
		}
	}
}

void LocalConferenceEventHandler::notifyAll(const Content &notify) {
	for (const auto &participant : conf->getParticipants()) {
		notifyParticipant(notify, participant);
	}
}

Content LocalConferenceEventHandler::createNotifyFullState(const shared_ptr<EventSubscribe> &ev) {
	vector<string> acceptedContents = vector<string>();
	if (ev) {
		const auto message = (belle_sip_message_t *)ev->getOp()->getRecvCustomHeaders();
		for (belle_sip_header_t *acceptHeader = belle_sip_message_get_header(message, "Accept"); acceptHeader != NULL;
		     acceptHeader = belle_sip_header_get_next(acceptHeader)) {
			acceptedContents.push_back(L_C_TO_STRING(belle_sip_header_get_unparsed_value(acceptHeader)));
		}
	}

	std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
	ConferenceId conferenceId(conferenceAddress, conferenceAddress);
	// Enquire whether this conference belongs to a server group chat room
	shared_ptr<Core> core = conf->getCore();
	std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
	const bool oneToOne = chatRoom ? !!(chatRoom->getCapabilities() & AbstractChatRoom::Capabilities::OneToOne) : false;
	const bool ephemerable =
	    chatRoom ? !!(chatRoom->getCapabilities() & AbstractChatRoom::Capabilities::Ephemeral) : false;
	string entity = conferenceAddress->asStringUriOnly();
	string subject = conf->getUtf8Subject();
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	if (!subject.empty()) {
		confDescr.setSubject(subject);
	}
	const auto &confParams = conf->getCurrentParams();
	const auto &audioEnabled = confParams.audioEnabled();
	const LinphoneMediaDirection audioDirection =
	    audioEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto &videoEnabled = confParams.videoEnabled();
	const LinphoneMediaDirection videoDirection =
	    videoEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto &textEnabled = confParams.chatEnabled();
	const LinphoneMediaDirection textDirection =
	    textEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	addAvailableMediaCapabilities(audioDirection, videoDirection, textDirection, confDescr);
	std::string keywordList;
	if (oneToOne) {
		keywordList += "one-to-one ";
	}
	if (ephemerable) {
		keywordList += "ephemeral ";
	}

	if (!keywordList.empty()) {
		KeywordsType keywords(sizeof(char), keywordList.c_str());
		confDescr.setKeywords(keywords);
	}

	if (ephemerable && chatRoom) {
		const ModeType mode =
		    (chatRoom->getCurrentParams()->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged)
		        ? "admin-managed"
		        : "device-managed";

		const auto ephemeral = Ephemeral(mode, std::to_string(chatRoom->getCurrentParams()->getEphemeralLifetime()));

		auto &confDescrDOMDoc = confDescr.getDomDocument();

		::xercesc::DOMElement *e(confDescrDOMDoc.createElementNS(
		    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
		    ::xsd::cxx::xml::string("linphone-cie:ephemeral").c_str()));
		*e << ephemeral;

		confDescr.getAny().push_back(e);
	}

	ConferenceParamsInterface::SecurityLevel securityLevel = confParams.getSecurityLevel();
	if (chatRoom) {
		securityLevel =
		    (chatRoom->getCurrentParams()->isEncrypted() ? ConferenceParamsInterface::SecurityLevel::EndToEnd
		                                                 : ConferenceParamsInterface::SecurityLevel::None);
	}
	const auto cryptoSecurityLevel = CryptoSecurityLevel(ConferenceParams::getSecurityLevelAttribute(securityLevel));
	auto &confDescrDOMDoc = confDescr.getDomDocument();
	::xercesc::DOMElement *e(confDescrDOMDoc.createElementNS(
	    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
	    ::xsd::cxx::xml::string("linphone-cie:crypto-security-level").c_str()));
	*e << cryptoSecurityLevel;
	confDescr.getAny().push_back(e);

	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	UsersType users;
	confInfo.setUsers(users);

	std::list<std::shared_ptr<Participant>> participants(conf->getParticipants());

	// Add local participant only if it is enabled
	if (confParams.localParticipantEnabled() && conf->isIn()) {
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
		user.setEntity(participant->getAddress()->asStringUriOnly());
		user.getRoles()->getEntry().push_back(participant->isAdmin() ? "admin" : "participant");
		user.setState(StateType::full);

		for (const auto &device : participant->getDevices()) {
			const string &gruu = device->getAddress()->asStringUriOnly();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty()) endpoint.setDisplayText(displayName);

			auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
			for (const auto &protocol : protocols) {
				std::ostringstream versionStr;
				versionStr << protocol.second;
				const auto ephemeralService = ServiceDescription(protocol.first, versionStr.str());
				auto &endpointDOMDoc = endpoint.getDomDocument();
				::xercesc::DOMElement *e(endpointDOMDoc.createElementNS(
				    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
				    ::xsd::cxx::xml::string("linphone-cie:service-description").c_str()));
				*e << ephemeralService;
				//				endpoint.setAnyAttribute(e);
			}

			// Media capabilities
			addMediaCapabilities(device, endpoint);

			// Enpoint session info
			addEndpointSessionInfo(device, endpoint);

			// Call ID
			addEndpointCallInfo(device, endpoint);

			endpoint.setState(StateType::full);

			user.getEndpoint().push_back(endpoint);
		}

		confInfo.getUsers()->getUser().push_back(user);
	}
	return makeContent(createNotify(confInfo, true));
}

void LocalConferenceEventHandler::addAvailableMediaCapabilities(const LinphoneMediaDirection audioDirection,
                                                                const LinphoneMediaDirection videoDirection,
                                                                const LinphoneMediaDirection textDirection,
                                                                ConferenceDescriptionType &confDescr) {
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

void LocalConferenceEventHandler::addEndpointSessionInfo(const std::shared_ptr<ParticipantDevice> &device,
                                                         EndpointType &endpoint) {
	const auto &state = device->getState();
	auto status = EndpointStatusType::pending;
	const auto &joiningMethod = device->getJoiningMethod();
	std::string reasonText = std::string();
	switch (joiningMethod) {
		case ParticipantDevice::JoiningMethod::DialedIn:
			reasonText = "Ad-hoc invitation";
			break;
		case ParticipantDevice::JoiningMethod::DialedOut:
			reasonText = "Added by focus";
			break;
		case ParticipantDevice::JoiningMethod::FocusOwner:
			reasonText = "is focus";
			break;
	}

	switch (state) {
		case ParticipantDevice::State::ScheduledForJoining:
			status = EndpointStatusType::pending;
			break;
		case ParticipantDevice::State::Joining:
			switch (joiningMethod) {
				case ParticipantDevice::JoiningMethod::DialedIn:
					status = EndpointStatusType::dialing_in;
					break;
				case ParticipantDevice::JoiningMethod::DialedOut:
					status = EndpointStatusType::dialing_out;
					break;
				case ParticipantDevice::JoiningMethod::FocusOwner:
					lError() << "Focus owner device " << device->getAddress()
					         << " should never be in the joining state";
					break;
			}
			break;
		case ParticipantDevice::State::Alerting:
			status = EndpointStatusType::alerting;
			break;
		case ParticipantDevice::State::Present:
			status = EndpointStatusType::connected;
			break;
		case ParticipantDevice::State::OnHold:
			status = EndpointStatusType::on_hold;
			break;
		case ParticipantDevice::State::ScheduledForLeaving:
		case ParticipantDevice::State::Leaving:
			status = EndpointStatusType::disconnecting;
			break;
		case ParticipantDevice::State::Left:
			status = EndpointStatusType::disconnected;
			break;
		case ParticipantDevice::State::MutedByFocus:
			status = EndpointStatusType::muted_via_focus;
			break;
	}
	endpoint.setStatus(status);

	auto joiningMethodNotify = JoiningType::dialed_in;
	switch (joiningMethod) {
		case ParticipantDevice::JoiningMethod::DialedIn:
			joiningMethodNotify = JoiningType::dialed_in;
			break;
		case ParticipantDevice::JoiningMethod::DialedOut:
			joiningMethodNotify = JoiningType::dialed_out;
			break;
		case ParticipantDevice::JoiningMethod::FocusOwner:
			joiningMethodNotify = JoiningType::focus_owner;
			break;
	}

	JoiningType joiningMethodType = JoiningType(joiningMethodNotify);
	endpoint.setJoiningMethod(joiningMethodType);

	ExecutionType joiningInfoType = ExecutionType();
	auto joiningTime = device->getTimeOfJoining();
	auto utcTimeStruct = Utils::getTimeTAsTm(joiningTime);

	LinphonePrivate::Xsd::XmlSchema::DateTime utcTime(
	    (utcTimeStruct.tm_year + 1900), static_cast<short unsigned int>(utcTimeStruct.tm_mon + 1),
	    static_cast<short unsigned int>(utcTimeStruct.tm_mday), static_cast<short unsigned int>(utcTimeStruct.tm_hour),
	    static_cast<short unsigned int>(utcTimeStruct.tm_min), utcTimeStruct.tm_sec);
	joiningInfoType.setWhen(utcTime);

	std::string reason = std::string("Reason: SIP;text=") + reasonText;
	joiningInfoType.setReason(reason);
	endpoint.setJoiningInfo(joiningInfoType);
}

void LocalConferenceEventHandler::addEndpointCallInfo(const std::shared_ptr<ParticipantDevice> &device,
                                                      EndpointType &endpoint) {
	if (!device->getCallId().empty() || !device->getFromTag().empty() || !device->getToTag().empty()) {
		SipDialogIdType sipDialogIdType =
		    SipDialogIdType(device->getCallId(), device->getFromTag(), device->getToTag());
		CallType callInfoType = CallType();
		callInfoType.setSip(sipDialogIdType);
		endpoint.setCallInfo(callInfoType);
	}
}

void LocalConferenceEventHandler::addMediaCapabilities(const std::shared_ptr<ParticipantDevice> &device,
                                                       EndpointType &endpoint) {
	const auto &audioDirection = device->getStreamCapability(LinphoneStreamTypeAudio);
	MediaType audio = MediaType("1");
	audio.setDisplayText("audio");
	audio.setType("audio");
	if (audioDirection != LinphoneMediaDirectionInactive) {
		if (device->getSsrc(LinphoneStreamTypeAudio) > 0) {
			audio.setSrcId(std::to_string(device->getSsrc(LinphoneStreamTypeAudio)));
		}
	}
	if (!device->getLabel(LinphoneStreamTypeAudio).empty()) {
		audio.setLabel(device->getLabel(LinphoneStreamTypeAudio));
	}
	audio.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(audioDirection));
	endpoint.getMedia().push_back(audio);

	const auto &videoDirection = device->getStreamCapability(LinphoneStreamTypeVideo);
	MediaType video = MediaType("2");
	video.setDisplayText("video");
	video.setType("video");
	if (videoDirection != LinphoneMediaDirectionInactive) {
		if (!device->getLabel(LinphoneStreamTypeVideo).empty()) {
			video.setLabel(device->getLabel(LinphoneStreamTypeVideo));
		}
		if (device->getSsrc(LinphoneStreamTypeVideo) > 0) {
			video.setSrcId(std::to_string(device->getSsrc(LinphoneStreamTypeVideo)));
		}
	}
	video.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(videoDirection));
	endpoint.getMedia().push_back(video);

	const auto &textDirection = device->getStreamCapability(LinphoneStreamTypeText);
	MediaType text = MediaType("3");
	text.setDisplayText("text");
	text.setType("text");
	text.setStatus(LocalConferenceEventHandler::mediaDirectionToMediaStatus(textDirection));
	endpoint.getMedia().push_back(text);
}

Content LocalConferenceEventHandler::createNotifyMultipart(int notifyId) {

	list<shared_ptr<EventLog>> events = conf->getCore()->getPrivate()->mainDb->getConferenceNotifiedEvents(
	    ConferenceId(conf->getConferenceAddress(), conf->getConferenceAddress()), static_cast<unsigned int>(notifyId));

	list<Content> contents;
	for (const auto &eventLog : events) {
		string body;
		shared_ptr<ConferenceNotifiedEvent> notifiedEvent = static_pointer_cast<ConferenceNotifiedEvent>(eventLog);
		int eventNotifyId = static_cast<int>(notifiedEvent->getNotifyId());
		conf->setLastNotify(eventNotifyId == -1 ? (conf->getLastNotify() + 1)
		                                        : static_cast<unsigned int>(eventNotifyId));

		switch (eventLog->getType()) {
			case EventLog::Type::ConferenceParticipantAdded: {
				shared_ptr<ConferenceParticipantEvent> addedEvent =
				    static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = addedEvent->getParticipantAddress();
				body = createNotifyParticipantAdded(participantAddress);
			} break;

			case EventLog::Type::ConferenceParticipantRemoved: {
				shared_ptr<ConferenceParticipantEvent> removedEvent =
				    static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = removedEvent->getParticipantAddress();
				body = createNotifyParticipantRemoved(participantAddress);
			} break;

			case EventLog::Type::ConferenceParticipantSetAdmin: {
				shared_ptr<ConferenceParticipantEvent> setAdminEvent =
				    static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = setAdminEvent->getParticipantAddress();
				body = createNotifyParticipantAdminStatusChanged(participantAddress, true);
			} break;

			case EventLog::Type::ConferenceParticipantUnsetAdmin: {
				shared_ptr<ConferenceParticipantEvent> unsetAdminEvent =
				    static_pointer_cast<ConferenceParticipantEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = unsetAdminEvent->getParticipantAddress();
				body = createNotifyParticipantAdminStatusChanged(participantAddress, false);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceAdded: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceAddedEvent =
				    static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = deviceAddedEvent->getParticipantAddress();
				const std::shared_ptr<Address> &deviceAddress = deviceAddedEvent->getDeviceAddress();
				body = createNotifyParticipantDeviceAdded(participantAddress, deviceAddress);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceRemoved: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceRemovedEvent =
				    static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = deviceRemovedEvent->getParticipantAddress();
				const std::shared_ptr<Address> &deviceAddress = deviceRemovedEvent->getDeviceAddress();
				body = createNotifyParticipantDeviceRemoved(participantAddress, deviceAddress);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceStatusChanged: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceStatusChangedEvent =
				    static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress = deviceStatusChangedEvent->getParticipantAddress();
				const std::shared_ptr<Address> &deviceAddress = deviceStatusChangedEvent->getDeviceAddress();
				body = createNotifyParticipantDeviceDataChanged(participantAddress, deviceAddress);
			} break;

			case EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged:
			case EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged: {
				shared_ptr<ConferenceParticipantDeviceEvent> deviceMediaCapabilityChangedEvent =
				    static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);
				const std::shared_ptr<Address> &participantAddress =
				    deviceMediaCapabilityChangedEvent->getParticipantAddress();
				const std::shared_ptr<Address> &deviceAddress = deviceMediaCapabilityChangedEvent->getDeviceAddress();
				body = createNotifyParticipantDeviceDataChanged(participantAddress, deviceAddress);
			} break;

			case EventLog::Type::ConferenceSubjectChanged: {
				shared_ptr<ConferenceSubjectEvent> subjectEvent = static_pointer_cast<ConferenceSubjectEvent>(eventLog);
				body = createNotifySubjectChanged(subjectEvent->getSubject());
			} break;

			case EventLog::Type::ConferenceAvailableMediaChanged: {
				shared_ptr<ConferenceAvailableMediaEvent> availableMediaEvent =
				    static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog);
				body = createNotifyAvailableMediaChanged(availableMediaEvent->getAvailableMediaType());
			} break;
			default:
				// We should never pass here!
				L_ASSERT(false);
				continue;
		}
		contents.emplace_back(makeContent(body));
	}

	if (contents.empty()) return Content();

	list<Content *> contentPtrs;
	for (auto &content : contents)
		contentPtrs.push_back(&content);
	Content multipart = ContentManager::contentListToMultipart(contentPtrs);
	if (linphone_core_content_encoding_supported(conf->getCore()->getCCore(), "deflate"))
		multipart.setContentEncoding("deflate");
	return multipart;
}

string LocalConferenceEventHandler::createNotifyParticipantAdded(const std::shared_ptr<Address> &pAddress) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);
	UserType user = UserType();
	UserRolesType roles;
	UserType::EndpointSequence endpoints;

	shared_ptr<Participant> participant = conf->isMe(pAddress) ? conf->getMe() : conf->findParticipant(pAddress);
	if (participant) {
		for (const auto &device : participant->getDevices()) {
			const string &gruu = device->getAddress()->asStringUriOnly();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty()) endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(device, endpoint);

			// Enpoint session info
			addEndpointSessionInfo(device, endpoint);

			// Call ID
			addEndpointCallInfo(device, endpoint);

			endpoint.setState(StateType::full);
			user.getEndpoint().push_back(endpoint);
		}
	}

	user.setRoles(roles);
	user.setEntity(pAddress->asStringUriOnly());
	user.getRoles()->getEntry().push_back((participant && participant->isAdmin()) ? "admin" : "participant");
	user.setState(StateType::full);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantAdminStatusChanged(const std::shared_ptr<Address> &pAddress,
                                                                              bool isAdmin) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserRolesType roles;
	user.setRoles(roles);
	user.setEntity(pAddress->asStringUriOnly());
	user.getRoles()->getEntry().push_back(isAdmin ? "admin" : "participant");
	user.setState(StateType::partial);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantRemoved(const std::shared_ptr<Address> &pAddress) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	user.setEntity(pAddress->asStringUriOnly());
	user.setState(StateType::deleted);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

MediaStatusType LocalConferenceEventHandler::mediaDirectionToMediaStatus(LinphoneMediaDirection direction) {
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

string LocalConferenceEventHandler::createNotifyParticipantDeviceAdded(const std::shared_ptr<Address> &pAddress,
                                                                       const std::shared_ptr<Address> &dAddress) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress->asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress->asStringUriOnly());
	shared_ptr<Participant> participant = conf->isMe(pAddress) ? conf->getMe() : conf->findParticipant(pAddress);
	if (participant) {
		shared_ptr<ParticipantDevice> participantDevice = participant->findDevice(dAddress);
		if (participantDevice) {
			const string &displayName = participantDevice->getName();
			if (!displayName.empty()) endpoint.setDisplayText(displayName);

			auto protocols = Utils::parseCapabilityDescriptor(participantDevice->getCapabilityDescriptor());
			for (const auto &protocol : protocols) {
				std::ostringstream versionStr;
				versionStr << protocol.second;
				const auto ephemeralService = ServiceDescription(protocol.first, versionStr.str());
				auto &endpointDOMDoc = endpoint.getDomDocument();
				::xercesc::DOMElement *e(endpointDOMDoc.createElementNS(
				    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
				    ::xsd::cxx::xml::string("linphone-cie:service-description").c_str()));
				*e << ephemeralService;
				//				endpoint.setAnyAttribute(e);
			}

			// Media capabilities
			addMediaCapabilities(participantDevice, endpoint);

			// Enpoint session info
			addEndpointSessionInfo(participantDevice, endpoint);

			// Call ID
			addEndpointCallInfo(participantDevice, endpoint);
		}
	}
	endpoint.setState(StateType::full);
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceRemoved(const std::shared_ptr<Address> &pAddress,
                                                                         const std::shared_ptr<Address> &dAddress) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress->asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress->asStringUriOnly());

	endpoint.setState(StateType::deleted);

	shared_ptr<Participant> participant = conf->isMe(pAddress) ? conf->getMe() : conf->findParticipant(pAddress);
	if (participant) {
		shared_ptr<ParticipantDevice> participantDevice = participant->findDevice(dAddress);
		if (participantDevice) {
			const auto &timeOfDisconnection = participantDevice->getTimeOfDisconnection();
			if (timeOfDisconnection > -1) {
				ExecutionType disconnectionInfoType = ExecutionType();
				auto utcTimeStruct = Utils::getTimeTAsTm(timeOfDisconnection);

				LinphonePrivate::Xsd::XmlSchema::DateTime utcTime(
				    (utcTimeStruct.tm_year + 1900), static_cast<short unsigned int>(utcTimeStruct.tm_mon + 1),
				    static_cast<short unsigned int>(utcTimeStruct.tm_mday),
				    static_cast<short unsigned int>(utcTimeStruct.tm_hour),
				    static_cast<short unsigned int>(utcTimeStruct.tm_min), utcTimeStruct.tm_sec);
				disconnectionInfoType.setWhen(utcTime);
				const auto &reason = participantDevice->getDisconnectionReason();
				if (!reason.empty()) {
					disconnectionInfoType.setReason(reason);
				}
				endpoint.setDisconnectionInfo(disconnectionInfoType);

				const auto disconnectionMethod = participantDevice->getDisconnectionMethod();
				auto method = DisconnectionType::departed;
				switch (disconnectionMethod) {
					case ParticipantDevice::DisconnectionMethod::Booted:
						method = DisconnectionType::booted;
						break;
					case ParticipantDevice::DisconnectionMethod::Departed:
						method = DisconnectionType::departed;
						break;
					case ParticipantDevice::DisconnectionMethod::Failed:
						method = DisconnectionType::failed;
						break;
					case ParticipantDevice::DisconnectionMethod::Busy:
						method = DisconnectionType::busy;
						break;
				}
				DisconnectionType disconnectionMethodType = DisconnectionType(method);
				endpoint.setDisconnectionMethod(disconnectionMethodType);
			}

			addEndpointCallInfo(participantDevice, endpoint);
		}
	}

	user.getEndpoint().push_back(endpoint);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyParticipantDeviceDataChanged(const std::shared_ptr<Address> &pAddress,
                                                                             const std::shared_ptr<Address> &dAddress) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	UserType::EndpointSequence endpoints;
	user.setEntity(pAddress->asStringUriOnly());
	user.setState(StateType::partial);

	EndpointType endpoint = EndpointType();
	endpoint.setEntity(dAddress->asStringUriOnly());
	endpoint.setState(StateType::partial);
	shared_ptr<Participant> participant = conf->isMe(pAddress) ? conf->getMe() : conf->findParticipant(pAddress);
	if (participant) {
		shared_ptr<ParticipantDevice> participantDevice = participant->findDevice(dAddress);
		if (participantDevice) {
			const string &displayName = participantDevice->getName();
			if (!displayName.empty()) endpoint.setDisplayText(displayName);

			// Media capabilities
			addMediaCapabilities(participantDevice, endpoint);

			// Enpoint session info
			addEndpointSessionInfo(participantDevice, endpoint);

			// Call ID
			addEndpointCallInfo(participantDevice, endpoint);

			const auto &state = participantDevice->getState();
			endpoint.setState((state == ParticipantDevice::State::Left) ? StateType::deleted : StateType::partial);
		}
	}
	user.getEndpoint().push_back(endpoint);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifySubjectChanged() {
	return createNotifySubjectChanged(conf->getUtf8Subject());
}

// -----------------------------------------------------------------------------

void LocalConferenceEventHandler::notifyResponseCb(const LinphoneEvent *lev) {
	auto ev = dynamic_pointer_cast<EventSubscribe>(Event::toCpp(const_cast<LinphoneEvent *>(lev))->getSharedFromThis());
	auto cbs = EventCbs::create();
	LocalConferenceEventHandler *handler = static_cast<LocalConferenceEventHandler *>(cbs->getUserData());
	cbs->setUserData(nullptr);
	cbs->notifyResponseCb = nullptr;

	if (ev->getReason() != LinphoneReasonNone) return;

	if (handler && handler->conf) {
		LinphonePrivate::ConferenceInterface::State confState = handler->conf->getState();
		if ((confState != ConferenceInterface::State::Deleted) &&
		    (confState != ConferenceInterface::State::Terminated)) {
			if (handler->confListener) {
				for (const auto &p : handler->conf->getParticipants()) {
					for (const auto &d : p->getDevices()) {
						if ((d->getConferenceSubscribeEvent() == ev) &&
						    (d->getState() == ParticipantDevice::State::Joining)) {
							// fixme confListener should be removed in the futur. On only relevant for server group
							// chatroom
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

string LocalConferenceEventHandler::createNotify(ConferenceType confInfo, bool isFullState) {
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
	map["linphone-cie"].name = "linphone:xml:ns:conference-info-linphone-extension";
	serializeConferenceInfo(notify, confInfo, map);
	return notify.str();
}

string LocalConferenceEventHandler::createNotifySubjectChanged(const string &subject) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyEphemeralMode(const EventLog::Type &type) {
	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string entity = conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string("<unknown>");
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	std::string keywordList;
	keywordList += "ephemeral";
	if (!keywordList.empty()) {
		KeywordsType keywords(sizeof(char), keywordList.c_str());
		confDescr.setKeywords(keywords);
	}

	ConferenceId conferenceId(conferenceAddress, conferenceAddress);
	// Enquire whether this conference belongs to a server group chat room
	shared_ptr<Core> core = conf->getCore();
	std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);

	const ModeType mode =
	    (type == EventLog::Type::ConferenceEphemeralMessageManagedByAdmin) ? "admin-managed" : "device-managed";
	long lifetime = linphone_core_get_default_ephemeral_lifetime(core->getCCore());
	if (chatRoom) {
		lifetime = chatRoom->getCurrentParams()->getEphemeralLifetime();
	}

	const auto ephemeral = Ephemeral(mode, std::to_string(lifetime));
	auto &confDescrDOMDoc = confDescr.getDomDocument();

	::xercesc::DOMElement *e(confDescrDOMDoc.createElementNS(
	    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
	    ::xsd::cxx::xml::string("linphone-cie:ephemeral").c_str()));
	*e << ephemeral;

	confDescr.getAny().push_back(e);

	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyEphemeralLifetime(const long &lifetime) {
	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string entity = conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string("<unknown>");
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	if (lifetime != 0) {
		std::string keywordList;
		keywordList += "ephemeral";
		if (!keywordList.empty()) {
			KeywordsType keywords(sizeof(char), keywordList.c_str());
			confDescr.setKeywords(keywords);
		}
	}

	ConferenceId conferenceId(conferenceAddress, conferenceAddress);
	// Enquire whether this conference belongs to a server group chat room
	shared_ptr<Core> core = conf->getCore();
	std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);

	auto chatRoomMode = static_cast<AbstractChatRoom::EphemeralMode>(
	    linphone_core_chat_room_get_default_ephemeral_mode(core->getCCore()));
	if (chatRoom) {
		chatRoomMode = chatRoom->getCurrentParams()->getEphemeralMode();
	}

	const ModeType mode =
	    (chatRoomMode == AbstractChatRoom::EphemeralMode::AdminManaged) ? "admin-managed" : "device-managed";
	const auto ephemeral = Ephemeral(mode, std::to_string(lifetime));

	auto &confDescrDOMDoc = confDescr.getDomDocument();

	::xercesc::DOMElement *e(confDescrDOMDoc.createElementNS(
	    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
	    ::xsd::cxx::xml::string("linphone-cie:ephemeral").c_str()));
	*e << ephemeral;

	confDescr.getAny().push_back(e);

	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

string LocalConferenceEventHandler::createNotifyAvailableMediaChanged(
    const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	string entity = (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly()
	                                              : std::string("<unknown-conference-address>"));
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	LinphoneMediaDirection audioDirection = LinphoneMediaDirectionInactive;
	try {
		const auto &audioEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Audio);
		audioDirection = audioEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range &) {
	}
	LinphoneMediaDirection videoDirection = LinphoneMediaDirectionInactive;
	try {
		const auto &videoEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Video);
		videoDirection = videoEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range &) {
	}
	LinphoneMediaDirection textDirection = LinphoneMediaDirectionInactive;
	try {
		const auto &textEnabled = mediaCapabilities.at(ConferenceMediaCapabilities::Text);
		textDirection = textEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	} catch (std::out_of_range &) {
	}
	addAvailableMediaCapabilities(audioDirection, videoDirection, textDirection, confDescr);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

void LocalConferenceEventHandler::notifyParticipant(const Content &notify, const shared_ptr<Participant> &participant) {
	for (const auto &device : participant->getDevices()) {
		/* Only notify to device that are present in the conference. */
		switch (device->getState()) {
			case ParticipantDevice::State::Joining:
			case ParticipantDevice::State::ScheduledForJoining:
			case ParticipantDevice::State::Alerting:
			case ParticipantDevice::State::Present:
			case ParticipantDevice::State::OnHold:
			case ParticipantDevice::State::MutedByFocus:
				notifyParticipantDevice(notify, device);
				break;
			case ParticipantDevice::State::Leaving:
			case ParticipantDevice::State::Left:
			case ParticipantDevice::State::ScheduledForLeaving:
				break;
		}
	}
}

void LocalConferenceEventHandler::notifyParticipantDevice(const Content &notify,
                                                          const shared_ptr<ParticipantDevice> &device) {
	if (!device->isSubscribedToConferenceEventPackage()) return;

	shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
	shared_ptr<EventCbs> cbs = EventCbs::create();
	cbs->setUserData(this);
	cbs->notifyResponseCb = notifyResponseCb;
	ev->addCallbacks(cbs);

	LinphoneContent *cContent = notify.isEmpty() ? nullptr : L_GET_C_BACK_PTR(&notify);
	ev->notify(cContent);
	linphone_core_notify_notify_sent(conf->getCore()->getCCore(), ev->toC(), cContent);
}

// -----------------------------------------------------------------------------

LinphoneStatus LocalConferenceEventHandler::subscribeReceived(const shared_ptr<EventSubscribe> &ev) {
	const auto &participantAddress = ev->getFrom();
	unsigned int lastNotify = conf->getLastNotify();

	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string conferenceAddressString =
	    conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string("<unknown>");

	shared_ptr<Participant> participant = getConferenceParticipant(participantAddress);
	if (!participant) {
		lError() << "Received SUBSCRIBE corresponds to no participant of the conference [" << conferenceAddressString
		         << "], no NOTIFY sent";
		ev->deny(LinphoneReasonDeclined);
		return -1;
	}

	const auto &contactAddr = ev->getRemoteContact();
	shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
	const auto deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
	if (!device ||
	    ((deviceState != ParticipantDevice::State::Present) && (deviceState != ParticipantDevice::State::Joining))) {
		lError() << "Received SUBSCRIBE for conference [" << conferenceAddressString << "], device sending subscribe ["
		         << *contactAddr << "] is not known, no NOTIFY sent";
		ev->deny(LinphoneReasonDeclined);
		return -1;
	}

	ev->accept();
	if (ev->getState() == LinphoneSubscriptionActive) {
		unsigned int evLastNotify = static_cast<unsigned int>(Utils::stoi(ev->getCustomHeader("Last-Notify-Version")));
		auto oldEv = device->getConferenceSubscribeEvent();
		device->setConferenceSubscribeEvent(ev);
		if (oldEv) {
			oldEv->terminate();
		}
		if ((evLastNotify == 0) || (deviceState == ParticipantDevice::State::Joining)) {
			lInfo() << "Sending initial notify of conference [" << conf->getConferenceAddress()
			        << "] to: " << device->getAddress() << " with last notif set to " << conf->getLastNotify();
			if (deviceState == ParticipantDevice::State::Present) {
				lInfo() << "Participant " << device->getAddress() << " is already part of conference ["
				        << conf->getConferenceAddress()
				        << "] hence send full state to be sure the client and the server are on the same page";
			} else {
				conf->setLastNotify(lastNotify + 1);
			}
			notifyFullState(createNotifyFullState(ev), device);
			// Do not notify everybody that a particiant has been added if it was already part of the conference. It may
			// mean that the client and the server wanted to synchronize to each other
			if (deviceState != ParticipantDevice::State::Present) {
				// Notify everybody that a participant device has been added and its capabilities after receiving the
				// SUBSCRIBE
				const auto notify =
				    createNotifyParticipantDeviceDataChanged(participant->getAddress(), device->getAddress());
				notifyAllExceptDevice(makeContent(notify), device);
			}
		} else if (evLastNotify < lastNotify) {
			lInfo() << "Sending all missed notify [" << evLastNotify << "-" << lastNotify << "] for conference ["
			        << conf->getConferenceAddress() << "] to: " << participant->getAddress();

			const int fullStateTrigger =
			    linphone_config_get_int(linphone_core_get_config(conf->getCore()->getCCore()), "misc",
			                            "full_state_trigger_due_to_missing_updates", 10);
			bool forceFullState = static_cast<int>(lastNotify - evLastNotify) > fullStateTrigger;
			// FIXME: Temporary workaround until chatrooms and conference will be one single class with different
			// capabilities. Every subscribe sent for a conference will be answered by a notify full state as events are
			// not stored in the database
			const auto &audioVideoConference = conf->getCore()->findAudioVideoConference(conf->getConferenceId());
			if (audioVideoConference || forceFullState) {
				notifyFullState(createNotifyFullState(ev), device);
			} else {
				notifyParticipantDevice(createNotifyMultipart(static_cast<int>(evLastNotify)), device);
			}
		} else if (evLastNotify > lastNotify) {
			lWarning() << "Last notify received by client [" << evLastNotify << "] for conference ["
			           << conf->getConferenceAddress() << "] should not be higher than last notify sent by server ["
			           << lastNotify << "] - sending a notify full state in an attempt to recover from this situation";
			notifyFullState(createNotifyFullState(ev), device);
		} else {
			notifyParticipantDevice(Content(), device);
		}
	}

	return 0;
}

void LocalConferenceEventHandler::subscriptionStateChanged(const shared_ptr<EventSubscribe> &ev,
                                                           LinphoneSubscriptionState state) {
	if (state == LinphoneSubscriptionTerminated && conf) {
		const auto &participantAddress = ev->getFrom();
		shared_ptr<Participant> participant = getConferenceParticipant(participantAddress);
		if (!participant) return;
		const auto &contactAddr = ev->getRemoteContact();
		shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
		if (!device) return;
		if (ev == device->getConferenceSubscribeEvent()) {
			lInfo() << "End of subscription for device [" << device->getAddress() << "] of conference ["
			        << conf->getConferenceAddress() << "]";
			device->setConferenceSubscribeEvent(nullptr);
		}
	}
}

Content LocalConferenceEventHandler::getNotifyForId(int notifyId, const shared_ptr<EventSubscribe> &ev) {
	unsigned int lastNotify = conf->getLastNotify();

	const int fullStateTrigger = linphone_config_get_int(linphone_core_get_config(conf->getCore()->getCCore()), "misc",
	                                                     "full_state_trigger_due_to_missing_updates", 10);
	bool forceFullState =
	    (notifyId > static_cast<int>(lastNotify)) || (static_cast<int>(lastNotify) - notifyId) > fullStateTrigger;
	if ((notifyId == 0) || forceFullState) {
		auto content = createNotifyFullState(ev);
		list<Content *> contentPtrs;
		contentPtrs.push_back(&content);
		auto multipart = ContentManager::contentListToMultipart(contentPtrs);
		return multipart;
	} else if (notifyId < static_cast<int>(lastNotify)) {
		return createNotifyMultipart(notifyId);
	}

	return Content();
}

Content LocalConferenceEventHandler::makeContent(const std::string &xml) {
	Content content;
	content.setContentType(ContentType::ConferenceInfo);
	if (linphone_core_content_encoding_supported(conf->getCore()->getCCore(), "deflate")) {
		content.setContentEncoding("deflate");
	}
	if (!xml.empty()) {
		content.setBodyFromUtf8(xml);
	}
	return content;
}

void LocalConferenceEventHandler::onFullStateReceived() {
}

void LocalConferenceEventHandler::onParticipantAdded(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                     const std::shared_ptr<Participant> &participant) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAllExcept(makeContent(createNotifyParticipantAdded(participant->getAddress())), participant);
		conf->updateParticipantsInConferenceInfo(participant->getAddress());

		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_added(L_GET_C_BACK_PTR(chatRoom.get()), L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress()
		           << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantRemoved(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                       const std::shared_ptr<Participant> &participant) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAllExcept(makeContent(createNotifyParticipantRemoved(participant->getAddress())), participant);
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_removed(L_GET_C_BACK_PTR(chatRoom.get()),
				                                               L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress()
		           << " being removed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantSetAdmin(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                        const std::shared_ptr<Participant> &participant) {
	const bool isAdmin = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(makeContent(createNotifyParticipantAdminStatusChanged(participant->getAddress(), isAdmin)));
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_admin_status_changed(L_GET_C_BACK_PTR(chatRoom.get()),
				                                                            L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << participant->getAddress()
		           << " admin status changed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onSubjectChanged(const std::shared_ptr<ConferenceSubjectEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(makeContent(createNotifySubjectChanged(event->getSubject())));
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_subject_changed(L_GET_C_BACK_PTR(chatRoom.get()), L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__
		           << ": Not sending notification of conference subject change because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceIsSpeakingChanged(const std::shared_ptr<ParticipantDevice> &,
                                                                       BCTBX_UNUSED(bool isSpeaking)) {
}

void LocalConferenceEventHandler::onParticipantDeviceIsMuted(const std::shared_ptr<ParticipantDevice> &,
                                                             BCTBX_UNUSED(bool isMuted)) {
}

void LocalConferenceEventHandler::onAvailableMediaChanged(const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(makeContent(createNotifyAvailableMediaChanged(event->getAvailableMediaType())));
	} else {
		lWarning() << __func__
		           << ": Not sending notification of conference subject change because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceAdded(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		auto participant = device->getParticipant();
		// If the ssrc is not 0, send a NOTIFY to the participant being added in order to give him its own SSRC
		if ((device->getSsrc(LinphoneStreamTypeAudio) != 0) || (device->getSsrc(LinphoneStreamTypeVideo) != 0)) {
			notifyAll(makeContent(createNotifyParticipantDeviceAdded(participant->getAddress(), device->getAddress())));
		} else {
			notifyAllExceptDevice(
			    makeContent(createNotifyParticipantDeviceAdded(participant->getAddress(), device->getAddress())),
			    device);
		}
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_device_added(L_GET_C_BACK_PTR(chatRoom.get()),
				                                                    L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress()
		           << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceRemoved(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		auto participant = device->getParticipant();
		notifyAllExceptDevice(
		    makeContent(createNotifyParticipantDeviceRemoved(participant->getAddress(), device->getAddress())), device);
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_device_removed(L_GET_C_BACK_PTR(chatRoom.get()),
				                                                      L_GET_C_BACK_PTR(event));
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress()
		           << " being removed because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceStateChanged(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		auto participant = device->getParticipant();
		notifyAll(
		    makeContent(createNotifyParticipantDeviceDataChanged(participant->getAddress(), device->getAddress())));
		if (conf) {
			shared_ptr<Core> core = conf->getCore();
			std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
			ConferenceId conferenceId(conferenceAddress, conferenceAddress);

			// Enquire whether this conference belongs to a server group chat room
			std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
			if (chatRoom) {
				_linphone_chat_room_notify_participant_device_state_changed(
				    L_GET_C_BACK_PTR(chatRoom.get()), L_GET_C_BACK_PTR(event),
				    (LinphoneParticipantDeviceState)device->getState());
			}
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress()
		           << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onParticipantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		auto participant = device->getParticipant();
		notifyAll(
		    makeContent(createNotifyParticipantDeviceDataChanged(participant->getAddress(), device->getAddress())));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress()
		           << " being added because pointer to conference is null";
	}
}

void LocalConferenceEventHandler::onEphemeralModeChanged(
    const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(makeContent(createNotifyEphemeralMode(event->getType())));
	} else {
		lWarning() << __func__ << ": Not sending notification of ephemeral mode changed to " << event->getType();
	}
}

void LocalConferenceEventHandler::onEphemeralLifetimeChanged(
    const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	if (conf) {
		notifyAll(makeContent(createNotifyEphemeralLifetime(event->getEphemeralMessageLifetime())));
	} else {
		lWarning() << __func__ << ": Not sending notification of ephemeral lifetime changed to "
		           << event->getEphemeralMessageLifetime();
	}
}

void LocalConferenceEventHandler::onStateChanged(BCTBX_UNUSED(LinphonePrivate::ConferenceInterface::State state)) {
}

void LocalConferenceEventHandler::onActiveSpeakerParticipantDevice(
    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
}

shared_ptr<Participant>
LocalConferenceEventHandler::getConferenceParticipant(const std::shared_ptr<Address> &address) const {
	shared_ptr<Core> core = conf->getCore();
	std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
	ConferenceId conferenceId(conferenceAddress, conferenceAddress);

	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId);
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
