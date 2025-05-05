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
#include "chat/chat-room/server-chat-room.h"
#include "conference/conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"
#include "linphone/api/c-content.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "server-conference-event-handler.h"
#include "utils/xml-utils.h"

#include <xsd/cxx/xml/dom/serialization-source.hxx>

// TODO: remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::ConferenceInfoLinphoneExtension;

// =============================================================================

ServerConferenceEventHandler::ServerConferenceEventHandler(std::shared_ptr<Conference> conf,
                                                           ConferenceListener *listener)
    : conference(conf), confListener(listener) {
}

// -----------------------------------------------------------------------------

void ServerConferenceEventHandler::notifyFullState(const std::shared_ptr<Content> &notify,
                                                   const shared_ptr<ParticipantDevice> &device) {
	notifyParticipantDevice(notify, device);
}

void ServerConferenceEventHandler::notifyOnlyAdmins(const std::shared_ptr<Content> &notify) {
	auto conf = getConference();
	if (!conf) {
		return;
	}
	for (const auto &participant : conf->getParticipants()) {
		if (participant->isAdmin()) {
			for (const auto &device : participant->getDevices()) {
				notifyParticipantDevice(notify, device);
			}
		}
	}
}

void ServerConferenceEventHandler::notifyAllExceptDevice(const std::shared_ptr<Content> &notify,
                                                         const shared_ptr<ParticipantDevice> &exceptDevice) {
	auto conf = getConference();
	if (!conf) {
		return;
	}
	for (const auto &participant : conf->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			if (device != exceptDevice) {
				/* Only notify to device that are present in the conference. */
				notifyParticipantDevice(notify, device);
			}
		}
	}
}

void ServerConferenceEventHandler::notifyAllExcept(const std::shared_ptr<Content> &notify,
                                                   const shared_ptr<Participant> &exceptParticipant) {
	auto conf = getConference();
	if (!conf) {
		return;
	}

	for (const auto &participant : conf->getParticipants()) {
		if (participant != exceptParticipant) {
			notifyParticipant(notify, participant);
		}
	}
}

void ServerConferenceEventHandler::notifyAll(const std::shared_ptr<Content> &notify) {
	auto conf = getConference();
	if (!conf) {
		return;
	}

	for (const auto &participant : conf->getParticipants()) {
		notifyParticipant(notify, participant);
	}
}

std::shared_ptr<Content> ServerConferenceEventHandler::createNotifyFullState(const shared_ptr<EventSubscribe> &ev) {
	auto conf = getConference();
	if (!conf) {
		return nullptr;
	}
	vector<string> acceptedContents = vector<string>();
	if (ev) {
		const auto message = (belle_sip_message_t *)ev->getOp()->getRecvCustomHeaders();
		for (belle_sip_header_t *acceptHeader = belle_sip_message_get_header(message, "Accept"); acceptHeader != NULL;
		     acceptHeader = belle_sip_header_get_next(acceptHeader)) {
			acceptedContents.push_back(L_C_TO_STRING(belle_sip_header_get_unparsed_value(acceptHeader)));
		}
	}

	std::shared_ptr<Address> conferenceAddress = conf->getConferenceAddress();
	ConferenceId conferenceId(conferenceAddress, conferenceAddress, conf->getCore()->createConferenceIdParams());
	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
	const bool oneToOne = chatRoom ? !!!chatRoom->getCurrentParams()->isGroup() : false;
	const bool ephemerable = chatRoom ? !!chatRoom->getCurrentParams()->getChatParams()->ephemeralEnabled() : false;
	string entity = conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string();
	string subject = conf->getUtf8Subject();
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	if (!subject.empty()) {
		confDescr.setSubject(subject);
	}
	const auto &conferenceParams = conf->getCurrentParams();
	const auto &audioEnabled = conferenceParams->audioEnabled();
	const LinphoneMediaDirection audioDirection =
	    audioEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto &videoEnabled = conferenceParams->videoEnabled();
	const LinphoneMediaDirection videoDirection =
	    videoEnabled ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive;
	const auto &textEnabled = conferenceParams->chatEnabled();
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

	auto &confDescrDOMDoc = confDescr.getDomDocument();
	auto startTime = conferenceParams->getStartTime();
	auto endTime = conferenceParams->getEndTime();
	if ((startTime > -1) || (endTime > -1)) {
		auto conferenceTimes = ConferenceTimes();
		if (startTime > -1) {
			conferenceTimes.setStart(timeTToDateTime(startTime));
		}

		if (endTime > -1) {
			conferenceTimes.setEnd(timeTToDateTime(endTime));
		}

		::xercesc::DOMElement *confTimesE(confDescrDOMDoc.createElementNS(
		    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
		    ::xsd::cxx::xml::string("linphone-cie:conference-times").c_str()));
		*confTimesE << conferenceTimes;
		confDescr.getAny().push_back(confTimesE);
	}

	if (ephemerable && chatRoom) {
		const ModeType mode = (chatRoom->getCurrentParams()->getChatParams()->getEphemeralMode() ==
		                       AbstractChatRoom::EphemeralMode::AdminManaged)
		                          ? "admin-managed"
		                          : "device-managed";

		const auto ephemeral =
		    Ephemeral(mode, std::to_string(chatRoom->getCurrentParams()->getChatParams()->getEphemeralLifetime()));

		::xercesc::DOMElement *ephemeralE(confDescrDOMDoc.createElementNS(
		    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
		    ::xsd::cxx::xml::string("linphone-cie:ephemeral").c_str()));
		*ephemeralE << ephemeral;

		confDescr.getAny().push_back(ephemeralE);
	}

	ConferenceParamsInterface::SecurityLevel securityLevel = conferenceParams->getSecurityLevel();
	if (conf->isChatOnly()) {
		securityLevel = (chatRoom->getCurrentParams()->getChatParams()->isEncrypted()
		                     ? ConferenceParamsInterface::SecurityLevel::EndToEnd
		                     : ConferenceParamsInterface::SecurityLevel::None);
	}
	const auto cryptoSecurityLevel = CryptoSecurityLevel(ConferenceParams::getSecurityLevelAttribute(securityLevel));
	::xercesc::DOMElement *cryptoSecurityLevelE(confDescrDOMDoc.createElementNS(
	    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
	    ::xsd::cxx::xml::string("linphone-cie:crypto-security-level").c_str()));
	*cryptoSecurityLevelE << cryptoSecurityLevel;
	confDescr.getAny().push_back(cryptoSecurityLevelE);

	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	UsersType users;
	confInfo.setUsers(users);

	std::list<std::shared_ptr<Participant>> participants(conf->getParticipants());

	// Add local participant only if it is enabled
	if (conferenceParams->localParticipantEnabled() && conf->isIn()) {
		std::shared_ptr<Participant> me = conf->getMe();
		if (me) {
			participants.push_front(me);
		}
	}

	auto organizer = conf->getOrganizer();

	for (const auto &participant : participants) {
		UserType user = UserType();
		UserRolesType roles;
		UserType::EndpointSequence endpoints;
		user.setRoles(roles);
		user.setEndpoint(endpoints);
		const auto &participantAddress = participant->getAddress();
		user.setEntity(participantAddress->asStringUriOnly());
		bool isOrganizer = organizer && organizer->weakEqual(*participantAddress);
		if (isOrganizer) {
			user.getRoles()->getEntry().push_back("organizer");
		}
		user.getRoles()->getEntry().push_back(participant->isAdmin() ? "admin" : "participant");
		user.getRoles()->getEntry().push_back(Participant::roleToText(participant->getRole()));
		user.setState(StateType::full);

		for (const auto &device : participant->getDevices()) {
			const string &gruu = device->getAddress()->asStringUriOnly();
			EndpointType endpoint = EndpointType();
			endpoint.setEntity(gruu);
			const string &displayName = device->getName();
			if (!displayName.empty()) endpoint.setDisplayText(displayName);

			addProtocols(device, endpoint);

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

void ServerConferenceEventHandler::addAvailableMediaCapabilities(const LinphoneMediaDirection audioDirection,
                                                                 const LinphoneMediaDirection videoDirection,
                                                                 const LinphoneMediaDirection textDirection,
                                                                 ConferenceDescriptionType &confDescr) {
	ConferenceMediaType mediaType;
	ConferenceMediumType audio("audio", "1");
	audio.setDisplayText("audio");
	audio.setStatus(XmlUtils::mediaDirectionToMediaStatus(audioDirection));
	mediaType.getEntry().push_back(audio);

	ConferenceMediumType video("video", "2");
	video.setDisplayText("video");
	video.setStatus(XmlUtils::mediaDirectionToMediaStatus(videoDirection));
	mediaType.getEntry().push_back(video);

	ConferenceMediumType text("text", "3");
	text.setDisplayText("text");
	text.setStatus(XmlUtils::mediaDirectionToMediaStatus(textDirection));
	mediaType.getEntry().push_back(text);
	confDescr.setAvailableMedia(mediaType);
}

void ServerConferenceEventHandler::addEndpointSessionInfo(const std::shared_ptr<ParticipantDevice> &device,
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
					lError() << "Focus owner device " << *device->getAddress()
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
		case ParticipantDevice::State::RequestingToJoin:
			// RFC4575: the on_hold state means that there is signaling between the conference and the client but the
			// latter is neither mixed in the conference nor being mixed. It is typically used when the endpoint has
			// requested to join the conference (state RequestingToJoin) or it left temporarely the conference (state
			// OnHold)
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
	if (joiningTime >= 0) {
		joiningInfoType.setWhen(timeTToDateTime(joiningTime));
	}

	std::string reason = std::string("Reason: SIP;text=") + reasonText;
	joiningInfoType.setReason(reason);
	endpoint.setJoiningInfo(joiningInfoType);
}

void ServerConferenceEventHandler::addEndpointCallInfo(const std::shared_ptr<ParticipantDevice> &device,
                                                       EndpointType &endpoint) {
	if (!device->getCallId().empty() || !device->getFromTag().empty() || !device->getToTag().empty()) {
		SipDialogIdType sipDialogIdType =
		    SipDialogIdType(device->getCallId(), device->getFromTag(), device->getToTag());
		CallType callInfoType = CallType();
		callInfoType.setSip(sipDialogIdType);
		endpoint.setCallInfo(callInfoType);
	}
}

void ServerConferenceEventHandler::addMediaCapabilities(const std::shared_ptr<ParticipantDevice> &device,
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
	if (!device->getStreamLabel(LinphoneStreamTypeAudio).empty()) {
		audio.setLabel(device->getStreamLabel(LinphoneStreamTypeAudio));
	}
	audio.setStatus(XmlUtils::mediaDirectionToMediaStatus(audioDirection));
	endpoint.getMedia().push_back(audio);

	const auto isScreenSharing = device->screenSharingEnabled();
	const auto &videoDirection =
	    isScreenSharing ? LinphoneMediaDirectionSendOnly : device->getStreamCapability(LinphoneStreamTypeVideo);
	MediaType video = MediaType("2");
	video.setDisplayText("video");
	video.setType("video");
	if (videoDirection != LinphoneMediaDirectionInactive) {
		if (!device->getStreamLabel(LinphoneStreamTypeVideo).empty()) {
			video.setLabel(device->getStreamLabel(LinphoneStreamTypeVideo));
		}
		if (device->getSsrc(LinphoneStreamTypeVideo) > 0) {
			video.setSrcId(std::to_string(device->getSsrc(LinphoneStreamTypeVideo)));
		}
	}
	video.setStatus(XmlUtils::mediaDirectionToMediaStatus(videoDirection));
	if (isScreenSharing) {
		const auto streamData = StreamData("slides");
		auto &videoDOMDoc = video.getDomDocument();
		::xercesc::DOMElement *e(videoDOMDoc.createElementNS(
		    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
		    ::xsd::cxx::xml::string("linphone-cie:stream-data").c_str()));
		*e << streamData;
		video.getAny().push_back(e);
	}
	endpoint.getMedia().push_back(video);

	const auto &textDirection = device->getStreamCapability(LinphoneStreamTypeText);
	MediaType text = MediaType("3");
	text.setDisplayText("text");
	text.setType("text");
	text.setStatus(XmlUtils::mediaDirectionToMediaStatus(textDirection));
	endpoint.getMedia().push_back(text);

	MediaType screenSharing = MediaType("4");
	screenSharing.setDisplayText("thumbnail");
	screenSharing.setType("video");

	const auto &thumbnailVideoDirection = device->getThumbnailStreamCapability();
	if (thumbnailVideoDirection != LinphoneMediaDirectionInactive) {
		if (!device->getThumbnailStreamLabel().empty()) {
			screenSharing.setLabel(device->getThumbnailStreamLabel());
		}
		if (device->getThumbnailStreamSsrc() > 0) {
			screenSharing.setSrcId(std::to_string(device->getThumbnailStreamSsrc()));
		}
	}
	screenSharing.setStatus(XmlUtils::mediaDirectionToMediaStatus(thumbnailVideoDirection));
	const auto streamData = StreamData("thumbnail");
	auto &screenSharingDOMDoc = screenSharing.getDomDocument();
	::xercesc::DOMElement *e(screenSharingDOMDoc.createElementNS(
	    ::xsd::cxx::xml::string("linphone:xml:ns:conference-info-linphone-extension").c_str(),
	    ::xsd::cxx::xml::string("linphone-cie:stream-data").c_str()));
	*e << streamData;
	screenSharing.getAny().push_back(e);
	endpoint.getMedia().push_back(screenSharing);
}

void ServerConferenceEventHandler::addProtocols(const std::shared_ptr<ParticipantDevice> &device,
                                                EndpointType &endpoint) {
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
}

std::shared_ptr<Content> ServerConferenceEventHandler::createNotifyMultipart(int notifyId) {
	auto conf = getConference();
	if (!conf) {
		return nullptr;
	}

	auto core = conf->getCore();
	list<shared_ptr<EventLog>> events = core->getPrivate()->mainDb->getConferenceNotifiedEvents(
	    ConferenceId(conf->getConferenceAddress(), conf->getConferenceAddress(), core->createConferenceIdParams()),
	    static_cast<unsigned int>(notifyId));

	list<shared_ptr<Content>> contents;
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

	if (contents.empty()) return Content::create();

	Content multipart = ContentManager::contentListToMultipart(contents);
	if (linphone_core_content_encoding_supported(conf->getCore()->getCCore(), "deflate"))
		multipart.setContentEncoding("deflate");
	return Content::create(multipart);
}

string ServerConferenceEventHandler::createNotifyParticipantAdded(const std::shared_ptr<Address> &pAddress) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);
	UserType user = UserType();
	UserRolesType roles;

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
	if (participant) {
		user.getRoles()->getEntry().push_back(Participant::roleToText(participant->getRole()));
	}
	user.setState(StateType::full);

	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string ServerConferenceEventHandler::createNotifyParticipantAdminStatusChanged(const std::shared_ptr<Address> &pAddress,
                                                                               bool isAdmin) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
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

string ServerConferenceEventHandler::createNotifyParticipantRemoved(const std::shared_ptr<Address> &pAddress) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
	user.setEntity(pAddress->asStringUriOnly());
	user.setState(StateType::deleted);
	confInfo.getUsers()->getUser().push_back(user);

	return createNotify(confInfo);
}

string ServerConferenceEventHandler::createNotifyParticipantDeviceAdded(const std::shared_ptr<Address> &pAddress,
                                                                        const std::shared_ptr<Address> &dAddress) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
	ConferenceType confInfo = ConferenceType(entity);
	UsersType users;
	confInfo.setUsers(users);

	UserType user = UserType();
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

			addProtocols(participantDevice, endpoint);

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

string ServerConferenceEventHandler::createNotifyParticipantDeviceRemoved(const std::shared_ptr<Address> &pAddress,
                                                                          const std::shared_ptr<Address> &dAddress) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
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
				if (timeOfDisconnection >= 0) {
					disconnectionInfoType.setWhen(timeTToDateTime(timeOfDisconnection));
				}
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

string
ServerConferenceEventHandler::createNotifyParticipantDeviceDataChanged(const std::shared_ptr<Address> &pAddress,
                                                                       const std::shared_ptr<Address> &dAddress) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
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

string ServerConferenceEventHandler::createNotifySubjectChanged() {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	return createNotifySubjectChanged(conf->getUtf8Subject());
}

// -----------------------------------------------------------------------------

void ServerConferenceEventHandler::notifyResponseCb(LinphoneEvent *lev) {
	auto ev = dynamic_pointer_cast<EventSubscribe>(Event::toCpp(lev)->getSharedFromThis());
	auto cbs = ev->getCurrentCallbacks();
	ServerConferenceEventHandler *handler = static_cast<ServerConferenceEventHandler *>(cbs->getUserData());
	cbs->setUserData(nullptr);
	cbs->notifyResponseCb = nullptr;

	if (ev->getReason() != LinphoneReasonNone) return;

	if (!handler) {
		lInfo() << "Unable to process event " << ev << " because no handler has been found";
		return;
	}

	auto conf = handler->getConference();
	if (conf) {
		LinphonePrivate::ConferenceInterface::State confState = conf->getState();
		if ((confState != ConferenceInterface::State::Deleted) &&
		    (confState != ConferenceInterface::State::Terminated)) {
			if (handler->confListener) {
				for (const auto &p : conf->getParticipants()) {
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

string ServerConferenceEventHandler::createNotify(ConferenceType confInfo, bool isFullState) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

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

string ServerConferenceEventHandler::createNotifySubjectChanged(const string &subject) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	confDescr.setSubject(subject);
	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	return createNotify(confInfo);
}

string ServerConferenceEventHandler::createNotifyEphemeralMode(const EventLog::Type &type) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string entity = conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string("sip:");
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	std::string keywordList;
	keywordList += "ephemeral";
	if (!keywordList.empty()) {
		KeywordsType keywords(sizeof(char), keywordList.c_str());
		confDescr.setKeywords(keywords);
	}

	ConferenceId conferenceId(conferenceAddress, conferenceAddress, conf->getCore()->createConferenceIdParams());
	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
	const ModeType mode =
	    (type == EventLog::Type::ConferenceEphemeralMessageManagedByAdmin) ? "admin-managed" : "device-managed";
	shared_ptr<Core> core = conf->getCore();
	long lifetime = linphone_core_get_default_ephemeral_lifetime(core->getCCore());
	if (chatRoom) {
		lifetime = chatRoom->getCurrentParams()->getChatParams()->getEphemeralLifetime();
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

string ServerConferenceEventHandler::createNotifyEphemeralLifetime(const long &lifetime) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string entity = conferenceAddress ? conferenceAddress->asStringUriOnly() : std::string("sip:");
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

	ConferenceId conferenceId(conferenceAddress, conferenceAddress, conf->getCore()->createConferenceIdParams());
	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
	shared_ptr<Core> core = conf->getCore();
	auto chatRoomMode = static_cast<AbstractChatRoom::EphemeralMode>(
	    linphone_core_chat_room_get_default_ephemeral_mode(core->getCCore()));
	if (chatRoom) {
		chatRoomMode = chatRoom->getCurrentParams()->getChatParams()->getEphemeralMode();
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

string ServerConferenceEventHandler::createNotifyAvailableMediaChanged(
    const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	auto conf = getConference();
	if (!conf) {
		return std::string();
	}

	string entity =
	    (conf->getConferenceAddress() ? conf->getConferenceAddress()->asStringUriOnly() : std::string("sip:"));
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

void ServerConferenceEventHandler::notifyParticipant(const std::shared_ptr<Content> &notify,
                                                     const shared_ptr<Participant> &participant) {
	for (const auto &device : participant->getDevices()) {
		/* Only notify to device that are present in the conference. */
		switch (device->getState()) {
			case ParticipantDevice::State::Joining:
			case ParticipantDevice::State::ScheduledForJoining:
			case ParticipantDevice::State::Alerting:
			case ParticipantDevice::State::Present:
			case ParticipantDevice::State::OnHold:
			case ParticipantDevice::State::RequestingToJoin:
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

void ServerConferenceEventHandler::notifyParticipantDevice(const shared_ptr<Content> &content,
                                                           const shared_ptr<ParticipantDevice> &device) {
	if (!device->isSubscribedToConferenceEventPackage()) return;
	auto conf = getConference();
	if (!conf) {
		return;
	}

	shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
	shared_ptr<EventCbs> cbs = EventCbs::create();
	cbs->setUserData(this);
	cbs->notifyResponseCb = notifyResponseCb;
	ev->addCallbacks(cbs);

	ev->notify(content);
	LinphoneContent *cContent = content->isEmpty() ? nullptr : content->toC();
	linphone_core_notify_notify_sent(conf->getCore()->getCCore(), ev->toC(), cContent);
}

// -----------------------------------------------------------------------------

LinphoneStatus ServerConferenceEventHandler::subscribeReceived(const shared_ptr<EventSubscribe> &ev) {
	auto conf = getConference();
	if (!conf) {
		return -1;
	}

	const auto &participantAddress = ev->getFrom();
	unsigned int lastNotify = conf->getLastNotify();

	shared_ptr<Participant> participant = getConferenceParticipant(participantAddress);
	if (!participant) {
		lError() << "Declining SUBSCRIBE because participant " << *participantAddress << " cannot be found in "
		         << *conf;
		ev->deny(LinphoneReasonDeclined);
		return -1;
	}

	const auto &contactAddr = ev->getRemoteContact();
	shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
	const auto deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
	if (!device ||
	    ((deviceState != ParticipantDevice::State::Present) && (deviceState != ParticipantDevice::State::Joining))) {
		lError() << "Received SUBSCRIBE for " << *conf << ", device sending subscribe [" << *contactAddr
		         << "] is not known, no NOTIFY sent";
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
		const auto &pAddress = participant->getAddress();
		const auto &dAddress = device->getAddress();
		if ((evLastNotify == 0) || (deviceState == ParticipantDevice::State::Joining)) {
			const auto &needToSyncDevice = device->isChangingSubscribeEvent();
			if (needToSyncDevice) {
				lInfo() << "Participant " << *dAddress << " is already part of " << *conf
				        << " hence send full state to be sure the client and the server are on the same page";
			}
			lInfo() << "Sending initial notify of " << *conf << " to: " << *dAddress
			        << " with last notify version set to " << conf->getLastNotify();
			notifyFullState(createNotifyFullState(ev), device);
			device->clearChangingSubscribeEvent();
		} else if (evLastNotify < lastNotify) {
			lInfo() << "Sending all missed notify [" << evLastNotify << "-" << lastNotify << "] for " << *conf
			        << " to: " << *pAddress;
			const int fullStateTrigger =
			    linphone_config_get_int(linphone_core_get_config(conf->getCore()->getCCore()), "misc",
			                            "full_state_trigger_due_to_missing_updates", 10);
			bool forceFullState = static_cast<int>(lastNotify - evLastNotify) > fullStateTrigger;
			// FIXME: Temporary workaround until chatrooms and conference will be one single class with different
			// capabilities. Every subscribe sent for a conference will be answered by a notify full state as events are
			// not stored in the database
			const auto &conference = conf->getCore()->findConference(conf->getConferenceId(), false);
			if ((conference && !conference->isChatOnly()) || forceFullState) {
				notifyFullState(createNotifyFullState(ev), device);
			} else {
				notifyParticipantDevice(createNotifyMultipart(static_cast<int>(evLastNotify)), device);
			}
		} else if (evLastNotify > lastNotify) {
			lWarning() << "Last notify received by client [" << evLastNotify << "] for " << *conf
			           << " should not be higher than last notify sent by server [" << lastNotify
			           << "] - sending a notify full state in an attempt to recover from this situation";
			notifyFullState(createNotifyFullState(ev), device);
		} else {
			notifyParticipantDevice(Content::create(), device);
		}
	}

	return 0;
}

void ServerConferenceEventHandler::subscriptionStateChanged(const shared_ptr<EventSubscribe> &ev,
                                                            LinphoneSubscriptionState state) {
	auto conf = getConference();
	if (state == LinphoneSubscriptionTerminated && conf) {
		const auto &participantAddress = ev->getFrom();
		shared_ptr<Participant> participant = getConferenceParticipant(participantAddress);
		if (!participant) return;
		const auto &contactAddr = ev->getRemoteContact();
		shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
		if (!device) return;
		if (ev == device->getConferenceSubscribeEvent()) {
			lInfo() << "End of subscription for device [" << *device->getAddress() << "] of " << *conf;
			device->setConferenceSubscribeEvent(nullptr);
		}
	}
}

std::shared_ptr<Content> ServerConferenceEventHandler::getNotifyForId(int notifyId,
                                                                      const shared_ptr<EventSubscribe> &ev) {
	auto conf = getConference();
	if (!conf) {
		return nullptr;
	}

	unsigned int lastNotify = conf->getLastNotify();

	const int fullStateTrigger = linphone_config_get_int(linphone_core_get_config(conf->getCore()->getCCore()), "misc",
	                                                     "full_state_trigger_due_to_missing_updates", 10);
	bool forceFullState =
	    (notifyId > static_cast<int>(lastNotify)) || (static_cast<int>(lastNotify) - notifyId) > fullStateTrigger;
	if ((notifyId == 0) || forceFullState) {
		auto content = createNotifyFullState(ev);
		auto multipart = ContentManager::contentListToMultipart({content});
		return Content::create(multipart);
	} else if (notifyId < static_cast<int>(lastNotify)) {
		return createNotifyMultipart(notifyId);
	}

	return Content::create();
}

std::shared_ptr<Content> ServerConferenceEventHandler::makeContent(const std::string &xml) {
	auto conf = getConference();
	if (!conf) {
		return nullptr;
	}

	auto content = Content::create();
	content->setContentType(ContentType::ConferenceInfo);
	if (linphone_core_content_encoding_supported(conf->getCore()->getCCore(), "deflate")) {
		content->setContentEncoding("deflate");
	}
	if (!xml.empty()) {
		content->setBodyFromUtf8(xml);
	}
	return content;
}

void ServerConferenceEventHandler::onFullStateReceived() {
}

void ServerConferenceEventHandler::onParticipantAdded(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                      const std::shared_ptr<Participant> &participant) {
	auto conf = getConference();
	if (!conf) {
		return;
	}

	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	const auto &pAddress = participant->getAddress();
	if (conf) {
		notifyAllExcept(makeContent(createNotifyParticipantAdded(pAddress)), participant);
		conf->updateParticipantInConferenceInfo(participant);

		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_added(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << *pAddress
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantRemoved(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                        const std::shared_ptr<Participant> &participant) {
	auto conf = getConference();
	if (!conf) {
		return;
	}

	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	const auto &pAddress = participant->getAddress();
	if (conf) {
		notifyAllExcept(makeContent(createNotifyParticipantRemoved(pAddress)), participant);
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_removed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << *pAddress
		           << " being removed because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantSetAdmin(const std::shared_ptr<ConferenceParticipantEvent> &event,
                                                         const std::shared_ptr<Participant> &participant) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	const bool isAdmin = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	const auto &pAddress = participant->getAddress();
	if (conf) {
		notifyAll(makeContent(createNotifyParticipantAdminStatusChanged(pAddress, isAdmin)));
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_admin_status_changed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant " << *pAddress
		           << " admin status changed because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onSubjectChanged(const std::shared_ptr<ConferenceSubjectEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	if (conf) {
		const auto &subject = event->getSubject();
		conf->updateSubjectInConferenceInfo(subject);
		notifyAll(makeContent(createNotifySubjectChanged(subject)));
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_subject_changed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of conference subject to " << event->getSubject()
		           << " change because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceIsSpeakingChanged(const std::shared_ptr<ParticipantDevice> &,
                                                                        BCTBX_UNUSED(bool isSpeaking)) {
}

void ServerConferenceEventHandler::onParticipantDeviceIsMuted(const std::shared_ptr<ParticipantDevice> &,
                                                              BCTBX_UNUSED(bool isMuted)) {
}

void ServerConferenceEventHandler::onAvailableMediaChanged(
    const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	if (!conf) {
		return;
	}

	if (conf) {
		notifyAll(makeContent(createNotifyAvailableMediaChanged(event->getAvailableMediaType())));
	} else {
		lWarning() << __func__
		           << ": Not sending notification of conference subject change because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceJoiningRequest(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the conference has been terminated
	auto conf = getConference();
	const auto &dAddress = device->getAddress();
	if (conf) {
		auto participant = device->getParticipant();
		const auto &pAddress = participant->getAddress();
		notifyAllExceptDevice(makeContent(createNotifyParticipantDeviceAdded(pAddress, dAddress)), device);
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << *dAddress
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceAdded(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	const auto &dAddress = device->getAddress();
	if (conf) {
		auto participant = device->getParticipant();
		const auto &pAddress = participant->getAddress();
		if (device->addedNotifySent()) {
			// If the ssrc is not 0, send a NOTIFY to the participant being added in order to give him its own SSRC
			if ((device->getSsrc(LinphoneStreamTypeAudio) != 0) || (device->getSsrc(LinphoneStreamTypeVideo) != 0)) {
				notifyAll(makeContent(createNotifyParticipantDeviceAdded(pAddress, dAddress)));
			} else {
				notifyAllExceptDevice(makeContent(createNotifyParticipantDeviceAdded(pAddress, dAddress)), device);
			}
		}
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_device_added(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << *dAddress
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceRemoved(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	const auto &dAddress = device->getAddress();
	if (conf) {
		auto participant = device->getParticipant();
		notifyAllExceptDevice(makeContent(createNotifyParticipantDeviceRemoved(participant->getAddress(), dAddress)),
		                      device);
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_device_removed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << *dAddress
		           << " being removed because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceStateChanged(
    const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	const auto &dAddress = device->getAddress();
	if (conf) {
		auto participant = device->getParticipant();
		notifyAll(makeContent(createNotifyParticipantDeviceDataChanged(participant->getAddress(), dAddress)));
		// Enquire whether this conference belongs to a server group chat room
		std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
		if (chatRoom) {
			_linphone_chat_room_notify_participant_device_state_changed(
			    chatRoom->toC(), L_GET_C_BACK_PTR(event), (LinphoneParticipantDeviceState)device->getState());
		}
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << *dAddress
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceScreenSharingChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	if (conf) {
		auto participant = device->getParticipant();
		notifyAll(
		    makeContent(createNotifyParticipantDeviceDataChanged(participant->getAddress(), device->getAddress())));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << device->getAddress()
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onParticipantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	const auto &dAddress = device->getAddress();
	if (conf) {
		auto participant = device->getParticipant();
		notifyAll(makeContent(createNotifyParticipantDeviceDataChanged(participant->getAddress(), dAddress)));
	} else {
		lWarning() << __func__ << ": Not sending notification of participant device " << *dAddress
		           << " being added because pointer to conference is null";
	}
}

void ServerConferenceEventHandler::onEphemeralModeChanged(
    const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	if (conf) {
		notifyAll(makeContent(createNotifyEphemeralMode(event->getType())));
	} else {
		lWarning() << __func__ << ": Not sending notification of ephemeral mode changed to " << event->getType();
	}
}

void ServerConferenceEventHandler::onEphemeralLifetimeChanged(
    const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) {
	// Do not send notify if conference pointer is null. It may mean that the confernece has been terminated
	auto conf = getConference();
	if (conf) {
		notifyAll(makeContent(createNotifyEphemeralLifetime(event->getEphemeralMessageLifetime())));
	} else {
		lWarning() << __func__ << ": Not sending notification of ephemeral lifetime changed to "
		           << event->getEphemeralMessageLifetime();
	}
}

void ServerConferenceEventHandler::onStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	auto conf = getConference();
	if (!conf) {
		return;
	}

	const auto &conferenceParams = conf->getCurrentParams();
	const auto &textEnabled = conferenceParams->chatEnabled();
	switch (state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Deleted:
			break;
		case ConferenceInterface::State::CreationFailed:
			conf->getCore()->doLater([conf] { dynamic_pointer_cast<ServerConference>(conf)->requestDeletion(); });
			break;
		case ConferenceInterface::State::CreationPending:
			conf->finalizeCreation();
			break;
		case ConferenceInterface::State::TerminationPending:
			if (conf->getParticipantDevices(false).size() == 0) conf->setState(ConferenceInterface::State::Terminated);
			break;
		case ConferenceInterface::State::Terminated:
			if (!textEnabled) conf->resetLastNotify();
			break;
	}
}

void ServerConferenceEventHandler::onActiveSpeakerParticipantDevice(
    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
}

shared_ptr<Participant>
ServerConferenceEventHandler::getConferenceParticipant(const std::shared_ptr<Address> &address) const {
	auto conf = getConference();
	if (!conf) {
		return nullptr;
	}

	// Enquire whether this conference belongs to a server group chat room
	std::shared_ptr<AbstractChatRoom> chatRoom = conf->getChatRoom();
	std::shared_ptr<LinphonePrivate::ServerChatRoom> sgcr = nullptr;
	if (chatRoom && (chatRoom->getConference() == conf) && !conf->supportsMedia()) {
		sgcr = dynamic_pointer_cast<LinphonePrivate::ServerChatRoom>(chatRoom);
	}

	shared_ptr<Participant> participant = nullptr;
	if (sgcr) {
		// If conference belongs to a server group chat room, then search in the cached participants
		participant = conf->findInvitedParticipant(address);
	} else {
		participant = conf->findParticipant(address);
	}

	return participant;
}

Xsd::XmlSchema::DateTime ServerConferenceEventHandler::timeTToDateTime(const time_t &unixTime) const {
	auto utcTimeStruct = Utils::getTimeTAsTm(unixTime);
	LinphonePrivate::Xsd::XmlSchema::DateTime utcTime(
	    (utcTimeStruct.tm_year + 1900), static_cast<short unsigned int>(utcTimeStruct.tm_mon + 1),
	    static_cast<short unsigned int>(utcTimeStruct.tm_mday), static_cast<short unsigned int>(utcTimeStruct.tm_hour),
	    static_cast<short unsigned int>(utcTimeStruct.tm_min), utcTimeStruct.tm_sec, 0, 0);
	return utcTime;
}

std::shared_ptr<Conference> ServerConferenceEventHandler::getConference() const {
	try {
		auto conferencePtr = std::shared_ptr<Conference>(conference);
		return conferencePtr;
	} catch (const bad_weak_ptr &) {
		lWarning() << "Unable to get valid Conference instance belonging to event handler " << this;
		return nullptr;
	}
}

LINPHONE_END_NAMESPACE
