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

#include <sstream>

#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"

#include "conference/participant.h"
#include "conference/remote-conference.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "logger/logger.h"
#include "remote-conference-event-handler.h"

#include <xsd/cxx/xml/string.hxx>

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::ConferenceInfoLinphoneExtension;

// -----------------------------------------------------------------------------

RemoteConferenceEventHandler::RemoteConferenceEventHandler (Conference *remoteConference, ConferenceListener * listener) {
	conf = remoteConference;
	confListener = listener;
	conf->getCore()->getPrivate()->registerListener(this);
}

RemoteConferenceEventHandler::~RemoteConferenceEventHandler () {

	try {
		conf->getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}


// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::conferenceInfoNotifyReceived (const string &xmlBody) {
	istringstream data(xmlBody);
	unique_ptr<ConferenceType> confInfo;
	try {
		confInfo = parseConferenceInfo(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing conference-info notify for: " << getConferenceId();
		return;
	}

	IdentityAddress entityAddress(confInfo->getEntity().c_str());

	if (entityAddress != getConferenceId().getPeerAddress()) {
		lError() << "Unable to process received NOTIFY because the entity address " << entityAddress << " doesn't match the peer address " << getConferenceId().getPeerAddress();
		return;
	}

	auto &confDescription = confInfo->getConferenceDescription();

	// 1. Compute event time.
	time_t creationTime = time(nullptr);
	{
		auto &freeText = confDescription->getFreeText();
		if (freeText.present())
			creationTime = static_cast<time_t>(Utils::stoll(freeText.get()));
	}

	// 2. Update last notify.
	{
		auto &version = confInfo->getVersion();
		if (version.present()) {
			unsigned int notifyVersion = version.get();
			if (getLastNotify() >= notifyVersion) {
				lWarning() << "Ignoring conference notify for: " << getConferenceId() << ", notify version received is: "
					<< notifyVersion << ", should be stricly more than last notify id of conference: " << getLastNotify();
				return;
			}
			conf->setLastNotify(version.get());
		}
	}

	bool isFullState = confInfo->getState() == StateType::full;

	// 3. Notify ephemeral settings, media, subject and keywords.
	if (confDescription.present()) {
		auto &subject = confDescription.get().getSubject();
		if (subject.present() && !subject.get().empty()) {
			if (conf->getSubject() != subject.get()) {
				conf->Conference::setSubject(subject.get());
				if (!isFullState) {
					conf->notifySubjectChanged(
						creationTime,
						isFullState,
						subject.get()
					);
				}
			}
		}

		auto &keywords = confDescription.get().getKeywords();
		if (keywords.present() && !keywords.get().empty()) {
			KeywordsType xmlKeywords = keywords.get();
			confListener->onConferenceKeywordsChanged(
				vector<string>(xmlKeywords.begin(), xmlKeywords.end())
			);
		}


		const auto &availableMedia = confDescription.get().getAvailableMedia();
		if (availableMedia.present()) {
			for (auto &mediaEntry : availableMedia.get().getEntry()) {
				const std::string mediaType = mediaEntry.getType();
				const LinphoneMediaDirection mediaDirection = RemoteConferenceEventHandler::mediaStatusToMediaDirection(mediaEntry.getStatus().get());
				const bool enabled = (mediaDirection == LinphoneMediaDirectionSendRecv);
				if (mediaType.compare("audio") == 0) {
					conf->confParams->enableAudio(enabled);
				} else if (mediaType.compare("video") == 0) {
					conf->confParams->enableVideo(enabled);
				} else if (mediaType.compare("text") == 0) {
					conf->confParams->enableChat(enabled);
				} else {
					lError() << "Unrecognized media type " << mediaType;
				}
			}
			if (!isFullState) {
				conf->notifyAvailableMediaChanged (creationTime, isFullState, conf->getMediaCapabilities());
			}
		}

		auto & anySequence (confDescription.get().getAny());

		for (const auto &anyElement : anySequence) {
			auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
			auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
			auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
			if (nodeName == "linphone-cie:ephemeral") {
				Ephemeral ephemeral{anyElement};
				auto ephemeralLifetime = ephemeral.getLifetime();
				auto ephemeralMode = ephemeral.getMode();

				const auto & core = conf->getCore();
				auto chatRoom = core->findChatRoom(getConferenceId());
				std::shared_ptr<LinphonePrivate::ClientGroupChatRoom> cgcr = nullptr;
				if (chatRoom && (chatRoom->getConference().get() == conf)) {
					cgcr = dynamic_pointer_cast<LinphonePrivate::ClientGroupChatRoom>(chatRoom);
				}
				if (cgcr) {
					if (ephemeralMode.empty() || (ephemeralMode.compare("admin-managed") == 0)) {
						cgcr->getCurrentParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::AdminManaged);
						if (!ephemeralLifetime.empty()) {
							const auto lifetime = std::stol(ephemeralLifetime);
							cgcr->getCurrentParams()->setEphemeralLifetime(lifetime);
							cgcr->getPrivate()->enableEphemeral((lifetime != 0));
							if (!isFullState) {
								conf->notifyEphemeralLifetimeChanged(
									creationTime,
									isFullState,
									lifetime
								);

								conf->notifyEphemeralMessageEnabled(
									creationTime,
									isFullState,
									(lifetime != 0)
								);
							}


						}
					} else if (ephemeralMode.compare("device-managed") == 0) {
						cgcr->getCurrentParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged);
					}
				}
			}
		}

	}

	if (isFullState)
		confListener->onParticipantsCleared();

	auto &users = confInfo->getUsers();
	if (!users.present()) return;

	// 4. Notify changes on users.
	for (auto &user : users->getUser()) {
		Address address(conf->getCore()->interpretUrl(user.getEntity().get()));
		StateType state = user.getState();

		shared_ptr<Participant> participant = conf->findParticipant(address);

		if (state == StateType::deleted) {
			if (conf->isMe(address)) {
				lInfo() << "Participant " << address.asString() << " requested to be deleted is me.";
				continue;
			} else if (participant) {
				conf->participants.remove(participant);

				if (!isFullState && participant) {
					conf->notifyParticipantRemoved(
						creationTime,
						isFullState,
						participant
					);
				}

				continue;
			} else {
				lWarning() << "Participant " << address.asString() << " removed but not in the list of participants!";
			}
		} else if (state == StateType::full) {
			if (conf->isMe(address)) {
				lInfo() << "Participant " << address.asString() << " requested to be added is me.";
			} else if (participant) {
				lWarning() << "Participant " << *participant << " added but already in the list of participants!";
			} else {
				participant = Participant::create(conf,address);
				conf->participants.push_back(participant);
				conf->updateParticipantsInConferenceInfo(address);
				lInfo() << "Participant " << *participant << " is successfully added - conference " << conf->getConferenceAddress().asString() << " has " << conf->getParticipantCount() << " participants";

				if (!isFullState) {
					conf->notifyParticipantAdded(
						creationTime,
						isFullState,
						participant
					);
				}
			}
		}

		// Try to get participant again as it may have been added or removed earlier on
		if (conf->isMe(address))
			participant = conf->getMe();
		else
			participant = conf->findParticipant(address);

		if (!participant) {
			lWarning() << "Participant " << address.asString() << " is not in the list of participants however it is trying to change the list of devices or change role!";
		} else {

			auto &roles = user.getRoles();
			if (roles) {

				auto &entry = roles->getEntry();
				bool isAdmin = (find(entry, "admin") != entry.end()
						? true
						: false);

				if (participant->isAdmin() != isAdmin) {

					participant->setAdmin(isAdmin);

					if (!isFullState) {
						conf->notifyParticipantSetAdmin(
							creationTime,
							isFullState,
							participant,
							isAdmin
						);
					}
				}
			}

			for (const auto &endpoint : user.getEndpoint()) {
				if (!endpoint.getEntity().present())
					continue;

				Address gruu(endpoint.getEntity().get());
				StateType state = endpoint.getState();

				shared_ptr<ParticipantDevice> device = nullptr;
				if (state == StateType::full) {
					device = participant->addDevice(gruu);
				} else {
					device = participant->findDevice(gruu);
				}

				const auto previousDeviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;

				if ((state != StateType::deleted) && (device)) {
/*
					auto & deviceAnySequence (endpoint.get().getAny());

					for (auto anyElementIt = deviceAnySequence.begin(); anyElementIt != deviceAnySequence.end (); ++anyElementIt) {
						const xercesc_3_1::DOMElement& anyElement (*anyElementIt);
						string name (xsd::cxx::xml::transcode<char>(anyElement.getLocalName()));
						string nodeName (xsd::cxx::xml::transcode<char>(anyElement.getNodeName()));
						string nodeValue (xsd::cxx::xml::transcode<char>(anyElement.getNodeValue()));
						if (nodeName.compare("linphone-cie:service-description") == 0) {
							const auto service = ServiceDescription(anyElement);
							const auto serviceId = service.getServiceId();
							const auto serviceVersion = Utils::Version(service.getVersion());
							if (serviceId.compare("ephemeral") == 0) {
								device->enableAdminModeSupport((serviceVersion > Utils::Version(1,1)));
							}
						}
					}
*/
					bool mediaCapabilityChanged = false;
					for (const auto &media : endpoint.getMedia()) {
						const std::string mediaType = media.getType().get();
						LinphoneMediaDirection mediaDirection = RemoteConferenceEventHandler::mediaStatusToMediaDirection(media.getStatus().get());
						if (mediaType.compare("audio") == 0) {
							mediaCapabilityChanged |= device->setStreamCapability(mediaDirection, LinphoneStreamTypeAudio);
							if (media.getSrcId()) {
								const std::string srcId = media.getSrcId().get();
								unsigned long ssrc = std::stoul(srcId);
								device->setSsrc((uint32_t) ssrc);
							}
						} else if (mediaType.compare("video") == 0) {
							mediaCapabilityChanged |= device->setStreamCapability(mediaDirection, LinphoneStreamTypeVideo);
							if (media.getLabel()) {
								const std::string label = media.getLabel().get();
								if (!label.empty()) {
									device->setLabel(label);
								}
							}
						} else if (mediaType.compare("text") == 0) {
							mediaCapabilityChanged |= device->setStreamCapability(mediaDirection, LinphoneStreamTypeText);
						} else {
							lError() << "Unrecognized media type " << mediaType;
						}
					}

					// Do not notify media capability changed during full states and participant addition because it is already done by the listener method onFullStateReceived
					if(mediaCapabilityChanged && !isFullState && (state != StateType::full)) {
						conf->notifyParticipantDeviceMediaCapabilityChanged(
							creationTime,
							isFullState,
							participant,
							device);
					}

					bool mediaAvailabilityChanged = device->updateStreamAvailabilities();

					// Do not notify availability changed during full states and participant addition because it is already done by the listener method onFullStateReceived
					if(mediaAvailabilityChanged && !isFullState && (state != StateType::full)) {
						conf->notifyParticipantDeviceMediaAvailabilityChanged(
							creationTime,
							isFullState,
							participant,
							device);
					}

					if (endpoint.getStatus().present()) {
						const auto & status = endpoint.getStatus().get();
						if ((status == EndpointStatusType::dialing_in) || (status == EndpointStatusType::dialing_out)) {
							device->setState(ParticipantDevice::State::Joining);
						} else if (status == EndpointStatusType::pending) {
							device->setState(ParticipantDevice::State::ScheduledForJoining);
						} else if (status == EndpointStatusType::connected) {
							device->setState(ParticipantDevice::State::Present);
						} else if (status == EndpointStatusType::on_hold) {
							device->setState(ParticipantDevice::State::OnHold);
						} else if (status == EndpointStatusType::disconnecting) {
							device->setState(ParticipantDevice::State::Leaving);
						} else if (status == EndpointStatusType::disconnected) {
							device->setState(ParticipantDevice::State::Left);
						}
					}
				}

				if (state == StateType::deleted) {

					participant->removeDevice(gruu);

					if (!isFullState && device && participant) {
						conf->notifyParticipantDeviceRemoved(
							creationTime,
							isFullState,
							participant,
							device
						);
					}

				} else if (device) {

					lInfo() << "Participant device " << gruu.asString() << " is successfully added";

					const string &name = endpoint.getDisplayText().present() ? endpoint.getDisplayText().get() : "";

					if (!name.empty())
						device->setName(name);

					if (conf->isMe(address) && conf->getMainSession())
						device->setSession(conf->getMainSession());

					if(!isFullState && (state == StateType::full)) {
						conf->notifyParticipantDeviceAdded(
							creationTime,
							isFullState,
							participant,
							device
						);
					}

					if (endpoint.getStatus().present()) {
						const auto & status = endpoint.getStatus().get();
						if ((status == EndpointStatusType::on_hold) && ((previousDeviceState != ParticipantDevice::State::OnHold) || (state == StateType::full))) {
							conf->notifyParticipantDeviceLeft(
								creationTime,
								isFullState,
								participant,
								device);
						} else if ((status == EndpointStatusType::connected) && ((previousDeviceState != ParticipantDevice::State::Present) || (state == StateType::full))) {
							conf->notifyParticipantDeviceJoined(
								creationTime,
								isFullState,
								participant,
								device);
						}
					}

				} else {
					lError() << "Unable to update media direction of device " << gruu << " because it has not been found in conference " << conf->getConferenceAddress();
				}

			}
		}
	}

	if (isFullState) {
		confListener->onFirstNotifyReceived(getConferenceId().getPeerAddress());
		conf->notifyFullState();
	}
}

LinphoneMediaDirection RemoteConferenceEventHandler::mediaStatusToMediaDirection (MediaStatusType status) {
	switch (status) {
		case MediaStatusType::inactive:
			return LinphoneMediaDirectionInactive;
		case MediaStatusType::sendonly:
			return LinphoneMediaDirectionSendOnly;
		case MediaStatusType::recvonly:
			return LinphoneMediaDirectionRecvOnly;
		case MediaStatusType::sendrecv:
			return LinphoneMediaDirectionSendRecv;
	}
	return LinphoneMediaDirectionSendRecv;
}

// -----------------------------------------------------------------------------

bool RemoteConferenceEventHandler::alreadySubscribed() const {
	return (!lev && subscriptionWanted);
}

void RemoteConferenceEventHandler::subscribe () {
	if (!alreadySubscribed())
		return; // Already subscribed or application did not request subscription


	const string &peerAddress = getConferenceId().getPeerAddress().asString();
	const string &localAddress = getConferenceId().getLocalAddress().asString();

	LinphoneAddress *lAddr = linphone_address_new(localAddress.c_str());
	LinphoneAddress *peerAddr = linphone_address_new(peerAddress.c_str());
	LinphoneCore *lc = conf->getCore()->getCCore();
	LinphoneProxyConfig *cfg = linphone_core_lookup_proxy_by_identity(lc, lAddr);

	if (!cfg || (linphone_proxy_config_get_state(cfg) != LinphoneRegistrationOk)) {
		linphone_address_unref(lAddr);
		linphone_address_unref(peerAddr);
		return;
	}

	lev = linphone_core_create_subscribe_2(conf->getCore()->getCCore(), peerAddr, cfg, "conference", 600);
	lev->op->setFrom(localAddress);
	const string &lastNotifyStr = Utils::toString(getLastNotify());
	linphone_event_add_custom_header(lev, "Last-Notify-Version", lastNotifyStr.c_str());
	linphone_address_unref(lAddr);
	linphone_address_unref(peerAddr);
	linphone_event_set_internal(lev, TRUE);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(lev), "event-handler-private", this, NULL);
	lInfo() << localAddress << " is subscribing to chat room or conference: " << peerAddress << " with last notify: " << lastNotifyStr;
	linphone_event_send_subscribe(lev, nullptr);
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::unsubscribePrivate () {
	if (lev){
		/* The following tricky code is to break a cycle. Indeed linphone_event_terminate() will chage the event's state,
		 * which will be notified to the core, that will call us immediately in invalidateSubscription(), which resets 'lev'
		 * while we still have to unref it.*/
		LinphoneEvent *tmpEv = lev;
		lev = nullptr;
		linphone_event_terminate(tmpEv);
		linphone_event_unref(tmpEv);
	}
}

void RemoteConferenceEventHandler::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	if (!sipNetworkReachable)
		unsubscribePrivate();
}

void RemoteConferenceEventHandler::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message) {
	if (state == LinphoneRegistrationOk)
		subscribe();
}

void RemoteConferenceEventHandler::onEnteringBackground () {
	unsubscribePrivate();
}

void RemoteConferenceEventHandler::onEnteringForeground () {
	subscribe();
}

void RemoteConferenceEventHandler::invalidateSubscription () {
	if (lev){
		if ((lev->subscription_state == LinphoneSubscriptionError) && (conf->getState() == ConferenceInterface::State::CreationPending)) {
			// The conference received an answer to its SUBSCRIBE and the server is not supporting the conference event package
			conf->setState(ConferenceInterface::State::Created);
		}
		linphone_event_unref(lev);
		lev = nullptr;
	}
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::subscribe (const ConferenceId &conferenceId) {
	conf->setConferenceId(conferenceId);
	subscriptionWanted = true;
	subscribe();
}

void RemoteConferenceEventHandler::unsubscribe () {
	unsubscribePrivate();
	subscriptionWanted = false;
}

void RemoteConferenceEventHandler::notifyReceived (const Content &content) {
	lInfo() << "NOTIFY received for conference: " << getConferenceId() << " - Content type " << content.getContentType().getType() << " subtype " << content.getContentType().getSubType();
	const ContentType &contentType = content.getContentType();
	if (contentType == ContentType::ConferenceInfo) {
		conferenceInfoNotifyReceived(content.getBodyAsUtf8String());
	}
}

void RemoteConferenceEventHandler::multipartNotifyReceived (const Content &content) {
	lInfo() << "multipart NOTIFY received for conference: " << getConferenceId();
	for (const auto &content : ContentManager::multipartToContentList(content)) {
		notifyReceived(content);
	}
}

const ConferenceId &RemoteConferenceEventHandler::getConferenceId() const {
	return conf->getConferenceId();
};

unsigned int RemoteConferenceEventHandler::getLastNotify() const {
	return conf->getLastNotify();
};

LINPHONE_END_NAMESPACE
