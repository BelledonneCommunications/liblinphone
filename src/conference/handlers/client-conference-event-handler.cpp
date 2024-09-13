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

#include "client-conference-event-handler.h"

#include <sstream>

#include <xsd/cxx/xml/string.hxx>

#include <bctoolbox/defs.h>

#include "chat/chat-room/client-chat-room.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "conference/session/media-session.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "event/event-subscribe.h"
#include "linphone/api/c-account.h"
#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::ConferenceInfoLinphoneExtension;

// -----------------------------------------------------------------------------

ClientConferenceEventHandler::ClientConferenceEventHandler(const std::shared_ptr<Core> &core,
                                                           const std::shared_ptr<Conference> &clientConference,
                                                           ConferenceListener *listener)
    : CoreAccessor(core) {
	conf = clientConference;
	confListener = listener;
	getCore()->getPrivate()->registerListener(this);
}

ClientConferenceEventHandler::~ClientConferenceEventHandler() {

	try {
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

// -----------------------------------------------------------------------------

void ClientConferenceEventHandler::setManagedByListEventhandler(bool managed) {
	managedByListEventhandler = managed;
}

void ClientConferenceEventHandler::setInitialSubscriptionUnderWayFlag(bool on) {
	initialSubscriptionUnderWay = on;
}

bool ClientConferenceEventHandler::getInitialSubscriptionUnderWayFlag() const {
	return initialSubscriptionUnderWay;
}

void ClientConferenceEventHandler::fillParticipantAttributes(
    std::shared_ptr<Participant> &participant,
    xsd::cxx::tree::optional<LinphonePrivate::Xsd::ConferenceInfo::UserRolesType> &roles,
    StateType state,
    bool isFullState,
    bool notify) const {
	if (roles) {
		auto &entry = roles->getEntry();

		time_t creationTime = time(nullptr);

		// Admin
		bool isAdmin = (find(entry, "admin") != entry.end() ? true : false);
		if (participant->isAdmin() != isAdmin) {
			participant->setAdmin(isAdmin);
			if (!isFullState && notify) {
				getConference()->notifyParticipantSetAdmin(creationTime, isFullState, participant, isAdmin);
			}
		}

		// Role
		bool isSpeaker = (find(entry, "speaker") != entry.end() ? true : false);
		bool isListener = (find(entry, "listener") != entry.end() ? true : false);
		Participant::Role role = Participant::Role::Unknown;
		if (isListener) {
			role = Participant::Role::Listener;
		} else if (isSpeaker || (state == StateType::full)) {
			// When a participant is added, then set its role to speaker by default to be backward compatible
			role = Participant::Role::Speaker;
		}
		bool unknownRole = (role == Participant::Role::Unknown);
		if ((participant->getRole() != role) && (isFullState || !unknownRole)) {
			participant->setRole(role);
			if (!isFullState && notify) {
				getConference()->notifyParticipantSetRole(creationTime, isFullState, participant, role);
			}
		}
	}
}

void ClientConferenceEventHandler::conferenceInfoNotifyReceived(const string &xmlBody) {
	istringstream data(xmlBody);
	unique_ptr<ConferenceType> confInfo;
	try {
		confInfo = parseConferenceInfo(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing conference-info notify for: " << getConferenceId();
		return;
	}

	const auto &core = getCore();
	auto chatRoom = core->findChatRoom(getConferenceId());

	const auto &conferenceAddress = getConference()->getConferenceAddress();
	const std::string conferenceAddressString =
	    conferenceAddress ? conferenceAddress->toString() : std::string("sip:unknown");
	std::shared_ptr<Address> entityAddress = Address::create(confInfo->getEntity());
	if (!conferenceAddress || (*entityAddress != *conferenceAddress)) {
		lError() << "Unable to process received NOTIFY because the entity address " << *entityAddress
		         << " doesn't match the conference address " << conferenceAddressString;
		return;
	}

	bool isFullState = confInfo->getState() == StateType::full;

	if (isFullState) {
		setInitialSubscriptionUnderWayFlag(false);
	}
	bool synchronizing = fullStateRequested;
	if (isFullState && fullStateRequested) {
		fullStateRequested = false;
	}

	if (waitingFullState && !isFullState) {
		lError() << "Unable to process received NOTIFY because conference " << conferenceAddressString
		         << " is waiting a full state";
		return;
	} else {
		waitingFullState = false;
	}

	auto &confDescription = confInfo->getConferenceDescription();

	// 1. Compute event time.
	time_t creationTime = time(nullptr);
	{
		if (confDescription.present()) {
			auto &freeText = confDescription->getFreeText();
			if (freeText.present()) creationTime = static_cast<time_t>(Utils::stoll(freeText.get()));
		}
	}

	// 2. Update last notify.
	{
		const auto previousLastNotify = getLastNotify();
		auto &version = confInfo->getVersion();
		if (version.present()) {
			unsigned int notifyVersion = version.get();
			synchronizing |= isFullState && (previousLastNotify >= notifyVersion);
			if (!isFullState && (previousLastNotify >= notifyVersion)) {
				lWarning() << "Ignoring conference notify for: " << getConferenceId()
				           << ", notify version received is: " << notifyVersion
				           << ", should be stricly more than last notify id of conference: " << previousLastNotify;
				requestFullState();
				return;
			}
			getConference()->setLastNotify(version.get());
			if (chatRoom) {
				// Update last notify ID in the DB just in case the notify does not generate any further event
				core->getPrivate()->mainDb->updateNotifyId(chatRoom, getLastNotify());
			}
		}
	}

	// 3. Notify ephemeral settings, media, subject and keywords.
	if (confDescription.present()) {
		auto &subject = confDescription.get().getSubject();
		if (subject.present() && !subject.get().empty()) {
			if (getConference()->getUtf8Subject() != subject.get()) {
				getConference()->Conference::setSubject(Utils::utf8ToLocale(subject.get()));
				if (!isFullState) {
					// Subject must be stored in the system locale
					getConference()->notifySubjectChanged(creationTime, isFullState, subject.get());
				}
			}
		}

		auto &keywords = confDescription.get().getKeywords();
		if (keywords.present() && !keywords.get().empty()) {
			KeywordsType xmlKeywords = keywords.get();
			confListener->onConferenceKeywordsChanged(vector<string>(xmlKeywords.begin(), xmlKeywords.end()));
		}

		const auto &availableMedia = confDescription.get().getAvailableMedia();
		// Take into consideration available media only if the conference is not a chat room
		if (availableMedia.present() && !chatRoom) {
			for (auto &mediaEntry : availableMedia.get().getEntry()) {
				const std::string mediaType = mediaEntry.getType();
				const LinphoneMediaDirection mediaDirection =
				    ClientConferenceEventHandler::mediaStatusToMediaDirection(mediaEntry.getStatus().get());
				const bool enabled = (mediaDirection == LinphoneMediaDirectionSendRecv);
				if (mediaType.compare("audio") == 0) {
					getConference()->getCurrentParams()->enableAudio(enabled);
				} else if (mediaType.compare("video") == 0) {
					getConference()->getCurrentParams()->enableVideo(enabled);
				} else if (mediaType.compare("text") == 0) {
					// Do not allow to turn off chat capabilities in order to preserve backward compatibility
					getConference()->getCurrentParams()->enableChat(enabled ||
					                                                getConference()->getCurrentParams()->chatEnabled());
				} else {
					lError() << "Unrecognized media type " << mediaType;
				}
			}
			if (!isFullState) {
				getConference()->notifyAvailableMediaChanged(creationTime, isFullState,
				                                             getConference()->getMediaCapabilities());
			}
		}

		auto &anySequence(confDescription.get().getAny());

		for (const auto &anyElement : anySequence) {
			auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
			auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
			auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
			if (nodeName == "linphone-cie:ephemeral") {
				Ephemeral ephemeral{anyElement};
				auto ephemeralLifetime = ephemeral.getLifetime();
				auto ephemeralMode = ephemeral.getMode();

				std::shared_ptr<LinphonePrivate::ClientChatRoom> cgcr = nullptr;
				if (chatRoom && (chatRoom->getConference() == getConference())) {
					cgcr = dynamic_pointer_cast<LinphonePrivate::ClientChatRoom>(chatRoom);
				}
				if (cgcr) {
					if (ephemeralMode.empty() || (ephemeralMode.compare("admin-managed") == 0)) {
						cgcr->getCurrentParams()->getChatParams()->setEphemeralMode(
						    AbstractChatRoom::EphemeralMode::AdminManaged);
						if (!ephemeralLifetime.empty()) {
							const auto lifetime = std::stol(ephemeralLifetime);
							cgcr->getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);
							cgcr->enableEphemeral((lifetime != 0), false);
							if (!isFullState) {
								getConference()->notifyEphemeralLifetimeChanged(creationTime, isFullState, lifetime);

								getConference()->notifyEphemeralMessageEnabled(creationTime, isFullState,
								                                               (lifetime != 0));
							}
						}
					} else if (ephemeralMode.compare("device-managed") == 0) {
						cgcr->getCurrentParams()->getChatParams()->setEphemeralMode(
						    AbstractChatRoom::EphemeralMode::DeviceManaged);
					}
				}
			} else if (nodeName == "linphone-cie:conference-times") {
				ConferenceTimes conferenceTimes{anyElement};
				if (conferenceTimes.getStart().present()) {
					auto startTime = conferenceTimes.getStart().get();
					getConference()->getCurrentParams()->setStartTime(dateTimeToTimeT(startTime));
				}
				if (conferenceTimes.getEnd().present()) {
					auto endTime = conferenceTimes.getEnd().get();
					getConference()->getCurrentParams()->setEndTime(dateTimeToTimeT(endTime));
				}
			} else if (nodeName == "linphone-cie:crypto-security-level") {
				CryptoSecurityLevel cryptoSecurityLevel{anyElement};
				auto securityLevelString = cryptoSecurityLevel.getLevel();
				auto securityLevel = ConferenceParams::getSecurityLevelFromAttribute(securityLevelString);
				getConference()->getCurrentParams()->setSecurityLevel(securityLevel);
			}
		}
	}

	auto &confState = confInfo->getConferenceState();
	if (confState.present()) {
		auto &anySequence(confState.get().getAny());

		for (const auto &anyElement : anySequence) {
			auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
			auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
			auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
		}
	}

	auto oldParticipants = getConference()->getParticipants();
	auto oldMeDevices = getConference()->getMe()->getDevices();
	if (isFullState) {
		confListener->onParticipantsCleared();
	}

	auto &users = confInfo->getUsers();
	if (!users.present()) return;

	// 4. Notify changes on users.
	for (auto &user : users->getUser()) {
		std::shared_ptr<Address> address = core->interpretUrl(user.getEntity().get(), false);
		StateType state = user.getState();

		bool isMe = getConference()->isMe(address);

		shared_ptr<Participant> participant = nullptr;
		if (isMe) participant = getConference()->getMe();
		else participant = getConference()->findParticipant(address);

		const auto &pIt =
		    std::find_if(oldParticipants.cbegin(), oldParticipants.cend(), [&address](const auto &currentParticipant) {
			    return (*address == *currentParticipant->getAddress());
		    });

		auto &roles = user.getRoles();
		if (state == StateType::deleted) {
			if (isMe) {
				lInfo() << "Participant " << *address << " requested to be deleted is me.";
				continue;
			} else if (participant) {
				getConference()->mParticipants.remove(participant);
				lInfo() << "Participant " << *participant << " is successfully removed - conference "
				        << conferenceAddressString << " has " << getConference()->getParticipantCount()
				        << " participants";
				if (!isFullState) {
					getConference()->notifyParticipantRemoved(creationTime, isFullState, participant);

					// TODO FIXME: Remove later when devices for friends will be notified through presence
					lInfo() << "[Friend] Removing device with address [" << address->asStringUriOnly() << "]";
					getCore()->getPrivate()->mainDb->removeDevice(address);
				}

				continue;
			} else {
				lWarning() << "Participant " << *address << " removed but not in the list of participants!";
			}
		} else if (state == StateType::full) {
			if (isMe) {
				lInfo() << "Participant " << *address << " requested to be added is me.";
				fillParticipantAttributes(participant, roles, state, isFullState, false);
			} else if (participant) {
				lWarning() << "Participant " << *participant << " added but already in the list of participants!";
			} else {
				participant = Participant::create(getConference(), address);
				fillParticipantAttributes(participant, roles, state, isFullState, false);

				getConference()->mParticipants.push_back(participant);
				lInfo() << "Participant " << *participant << " is successfully added - conference "
				        << conferenceAddressString << " has " << getConference()->getParticipantCount()
				        << " participants";
				if (!isFullState || (!oldParticipants.empty() && (pIt == oldParticipants.cend()) && !isMe)) {
					getConference()->notifyParticipantAdded(creationTime, isFullState, participant);
				}
			}
		}

		if (!participant) {
			// Try to get participant again as it may have been added or removed earlier on
			if (isMe) participant = getConference()->getMe();
			else participant = getConference()->findParticipant(address);
		}

		if (!participant) {
			lDebug() << "Participant " << *address
			         << " is not in the list of participants however it is trying to change the list of devices or "
			            "change role! Resubscribing to conference "
			         << conferenceAddressString << " to clear things up.";
			requestFullState();
		} else {

			fillParticipantAttributes(participant, roles, state, isFullState, true);
			for (const auto &endpoint : user.getEndpoint()) {
				if (!endpoint.getEntity().present()) continue;

				std::shared_ptr<Address> gruu = Address::create(endpoint.getEntity().get());
				StateType state = endpoint.getState();

				shared_ptr<ParticipantDevice> device = nullptr;
				if (state == StateType::full) {
					device = participant->addDevice(gruu);
				} else {
					if (endpoint.getCallInfo().present()) {
						const auto &callInfo = endpoint.getCallInfo().get();
						if (callInfo.getSip().present()) {
							const auto &sip = callInfo.getSip().get();
							device = participant->findDeviceByCallId(sip.getCallId());
							if (device) {
								device->setAddress(gruu);
							}
						}
					}
					if (!device) {
						device = participant->findDevice(gruu);
					}
				}

				const auto previousDeviceState =
				    device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;

				if (state == StateType::deleted) {

					participant->removeDevice(gruu);

					if (device) {
						if (endpoint.getDisconnectionInfo().present()) {
							const auto &disconnectionInfo = endpoint.getDisconnectionInfo().get();
							if (disconnectionInfo.getWhen().present()) {
								auto disconnectionTime = disconnectionInfo.getWhen().get();
								device->setTimeOfDisconnection(dateTimeToTimeT(disconnectionTime));
							}

							if (disconnectionInfo.getReason().present()) {
								device->setDisconnectionReason(disconnectionInfo.getReason().get());
							}
						}

						if (endpoint.getDisconnectionMethod().present()) {
							const auto &disconnectionMethod = endpoint.getDisconnectionMethod().get();
							switch (disconnectionMethod) {
								case DisconnectionType::booted:
									device->setDisconnectionMethod(ParticipantDevice::DisconnectionMethod::Booted);
									break;
								case DisconnectionType::departed:
									device->setDisconnectionMethod(ParticipantDevice::DisconnectionMethod::Departed);
									break;
								case DisconnectionType::busy:
									device->setDisconnectionMethod(ParticipantDevice::DisconnectionMethod::Busy);
									break;
								case DisconnectionType::failed:
									device->setDisconnectionMethod(ParticipantDevice::DisconnectionMethod::Failed);
									break;
							}
						}

						// Set participant device state to left in case the application regularly checks its state
						device->setState(ParticipantDevice::State::Left);

						if (!isFullState && participant) {
							getConference()->notifyParticipantDeviceRemoved(creationTime, isFullState, participant,
							                                                device);
						}
					}
				} else if (device) {
					/*
					                    auto & deviceAnySequence (endpoint.get().getAny());
					                    for (auto anyElementIt = deviceAnySequence.begin(); anyElementIt !=
					   deviceAnySequence.end (); ++anyElementIt) { const xercesc_3_1::DOMElement& anyElement
					   (*anyElementIt); string name (xsd::cxx::xml::transcode<char>(anyElement.getLocalName())); string
					   nodeName (xsd::cxx::xml::transcode<char>(anyElement.getNodeName())); string nodeValue
					   (xsd::cxx::xml::transcode<char>(anyElement.getNodeValue())); if
					   (nodeName.compare("linphone-cie:service-description") == 0) { const auto service =
					   ServiceDescription(anyElement); const auto serviceId = service.getServiceId(); const auto
					   serviceVersion = Utils::Version(service.getVersion()); if (serviceId.compare("ephemeral") == 0) {
					                                device->enableAdminModeSupport((serviceVersion >
					   Utils::Version(1,1)));
					                            }
					                        }
					                    }
					*/

					bool isScreenSharing = false;
					std::set<LinphoneStreamType> mediaCapabilityChanged;
					bool thumbnailTagFound = false;
					for (const auto &media : endpoint.getMedia()) {
						const std::string mediaType = media.getType().get();
						LinphoneStreamType streamType = LinphoneStreamTypeUnknown;
						if (mediaType.compare("audio") == 0) {
							streamType = LinphoneStreamTypeAudio;
						} else if (mediaType.compare("video") == 0) {
							streamType = LinphoneStreamTypeVideo;
						} else if (mediaType.compare("text") == 0) {
							streamType = LinphoneStreamTypeText;
						} else {
							lError() << "Unrecognized media type " << mediaType;
						}

						std::string content;
						auto &mediaAnySequence(media.getAny());
						for (const auto &anyElement : mediaAnySequence) {
							auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
							auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
							auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
							if (nodeName == "linphone-cie:stream-data") {
								StreamData streamData{anyElement};
								content = streamData.getStreamContent();
							}
						}

						isScreenSharing |= (streamType == LinphoneStreamTypeVideo) && (content.compare("slides") == 0);
						LinphoneMediaDirection mediaDirection =
						    ClientConferenceEventHandler::mediaStatusToMediaDirection(media.getStatus().get());
						uint32_t ssrc = 0;
						if (media.getSrcId() && (mediaDirection != LinphoneMediaDirectionInactive)) {
							const std::string srcId = media.getSrcId().get();
							ssrc = (uint32_t)std::stoul(srcId);
						}
						std::string label;
						if (media.getLabel()) {
							label = media.getLabel().get();
						}
						bool isThumbnailStream =
						    (streamType == LinphoneStreamTypeVideo) && (content.compare("thumbnail") == 0);
						if (isThumbnailStream) {
							thumbnailTagFound = true;
							device->setThumbnailStreamSsrc(ssrc);
							if (!label.empty()) {
								device->setThumbnailStreamLabel(label);
							}
							if (device->setThumbnailStreamCapability(mediaDirection)) {
								mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
							}
						} else {
							device->setSsrc(streamType, ssrc);
							if (!label.empty()) {
								device->setLabel(label, streamType);
							}
							if (device->setStreamCapability(mediaDirection, streamType)) {
								mediaCapabilityChanged.insert(streamType);
							}
						}
					}
					const auto &mainSession = getConference()->getMainSession();
					if (!thumbnailTagFound) {
						lInfo() << "It seems that we are dealing with a legacy conference server that doesn't provide "
						           "device's thumbnail informations.";
						const auto &remoteAddress = mainSession ? mainSession->getRemoteAddress() : nullptr;
						bool thumbnailEnabled = false;
						if (isMe && remoteAddress && remoteAddress->uriEqual(*device->getAddress())) {
							const auto &ms = dynamic_pointer_cast<MediaSession>(mainSession);
							if (ms) {
								const auto &params = ms->getMediaParams();
								thumbnailEnabled = params->cameraEnabled();
							}
						} else {
							const auto &deviceCapability = device->getStreamCapability(LinphoneStreamTypeVideo);
							thumbnailEnabled = ((deviceCapability == LinphoneMediaDirectionSendOnly) ||
							                    (deviceCapability == LinphoneMediaDirectionSendRecv));
						}
						device->setThumbnailStreamLabel(device->getLabel(LinphoneStreamTypeVideo));
						if (device->setThumbnailStreamCapability(thumbnailEnabled ? LinphoneMediaDirectionSendOnly
						                                                          : LinphoneMediaDirectionInactive)) {
							mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
						}
					}
					getConference()->setCachedScreenSharingDevice();
					const auto screenSharingChanged = device->enableScreenSharing(isScreenSharing);
					if (!isFullState && (state != StateType::full) && screenSharingChanged) {
						getConference()->notifyParticipantDeviceScreenSharingChanged(creationTime, isFullState,
						                                                             participant, device);
					}

					auto mediaAvailabilityChanged = device->updateStreamAvailabilities();
					// Do not notify availability changed during full states and participant addition because it is
					// already done by the listener method onFullStateReceived
					if (!isFullState && (state != StateType::full) &&
					    (previousDeviceState != ParticipantDevice::State::ScheduledForJoining) &&
					    (previousDeviceState != ParticipantDevice::State::Joining) &&
					    (previousDeviceState != ParticipantDevice::State::Alerting)) {
						if (!mediaAvailabilityChanged.empty()) {
							getConference()->notifyParticipantDeviceMediaAvailabilityChanged(creationTime, isFullState,
							                                                                 participant, device);
						}

						if (!mediaCapabilityChanged.empty()) {
							getConference()->notifyParticipantDeviceMediaCapabilityChanged(creationTime, isFullState,
							                                                               participant, device);
						}
					}
					getConference()->resetCachedScreenSharingDevice();

					if (endpoint.getJoiningMethod().present()) {
						const auto &joiningMethod = endpoint.getJoiningMethod().get();
						switch (joiningMethod) {
							case JoiningType::dialed_in:
								device->setJoiningMethod(ParticipantDevice::JoiningMethod::DialedIn);
								break;
							case JoiningType::dialed_out:
								device->setJoiningMethod(ParticipantDevice::JoiningMethod::DialedOut);
								break;
							case JoiningType::focus_owner:
								device->setJoiningMethod(ParticipantDevice::JoiningMethod::FocusOwner);
								break;
						}
					}

					if (endpoint.getJoiningInfo().present()) {
						const auto &joiningInfo = endpoint.getJoiningInfo().get();
						if (joiningInfo.getWhen().present()) {
							auto joiningTime = joiningInfo.getWhen().get();
							device->setTimeOfJoining(dateTimeToTimeT(joiningTime));
						}
					}

					if (endpoint.getStatus().present()) {
						const auto &status = endpoint.getStatus().get();
						ParticipantDevice::State state = ParticipantDevice::State::Joining;
						switch (status) {
							case EndpointStatusType::dialing_in:
							case EndpointStatusType::dialing_out:
								state = ParticipantDevice::State::Joining;
								break;
							case EndpointStatusType::alerting:
								state = ParticipantDevice::State::Alerting;
								break;
							case EndpointStatusType::pending:
								state = ParticipantDevice::State::ScheduledForJoining;
								break;
							case EndpointStatusType::connected:
								state = ParticipantDevice::State::Present;
								break;
							case EndpointStatusType::on_hold:
								state = ParticipantDevice::State::OnHold;
								break;
							case EndpointStatusType::disconnecting:
								state = ParticipantDevice::State::Leaving;
								break;
							case EndpointStatusType::disconnected:
								state = ParticipantDevice::State::Left;
								break;
							case EndpointStatusType::muted_via_focus:
								state = ParticipantDevice::State::MutedByFocus;
								break;
						}
						device->setState(state, !isFullState);
					}

					if (endpoint.getCallInfo().present()) {
						const auto &callInfo = endpoint.getCallInfo().get();
						if (callInfo.getSip().present()) {
							const auto &sip = callInfo.getSip().get();
							device->setCallId(sip.getCallId());
							device->setFromTag(sip.getFromTag());
							device->setToTag(sip.getToTag());
						}
					}

					const string &name = endpoint.getDisplayText().present() ? endpoint.getDisplayText().get() : "";

					if (!name.empty()) device->setName(name);
					// TODO FIXME: Remove later when devices for friends will be notified through presence
					lInfo() << "[Friend] Inserting new device with name [" << name << "] and address ["
					        << gruu->asStringUriOnly() << "]";
					getCore()->getPrivate()->mainDb->insertDevice(gruu, name);

					// For chat rooms, the session is handled by the participant
					if (isMe && mainSession &&
					    (getConference()->getCurrentParams()->audioEnabled() ||
					     getConference()->getCurrentParams()->videoEnabled()))
						device->setSession(mainSession);

					if (state == StateType::full) {
						lInfo() << "Participant device " << *gruu << " has been successfully added";
						bool sendNotify = (!oldParticipants.empty() && (pIt == oldParticipants.cend())) && !isMe;
						if (pIt != oldParticipants.cend()) {
							const auto &oldDevices = (*pIt)->getDevices();
							const auto &dIt =
							    std::find_if(oldDevices.cbegin(), oldDevices.cend(), [&gruu](const auto &oldDevice) {
								    return (*gruu == *oldDevice->getAddress());
							    });
							sendNotify = (dIt == oldDevices.cend()) && !isMe;
						}
						if (!isFullState || sendNotify) {
							getConference()->notifyParticipantDeviceAdded(creationTime, isFullState, participant,
							                                              device);
						}
					} else {
						lInfo() << "Participant device " << *gruu << " has been successfully updated";
					}
				} else {
					lDebug() << "Unable to update media direction of device " << *gruu
					         << " because it has not been found in conference " << conferenceAddressString
					         << ". Resubscribing to conference " << conferenceAddressString << " to clear things up.";
					requestFullState();
				}
			}
		}
	}

	if (isFullState) {
		auto currentParticipants = getConference()->getParticipants();
		auto currentMeDevices = getConference()->getMe()->getDevices();
		// Send participant and participant device removed notifys if the full state has less participants than the
		// current chat room or conference
		for (const auto &p : oldParticipants) {
			const auto &pIt = std::find_if(currentParticipants.cbegin(), currentParticipants.cend(),
			                               [&p](const auto &currentParticipant) {
				                               return (*p->getAddress() == *currentParticipant->getAddress());
			                               });
			for (const auto &d : p->getDevices()) {
				bool deviceFound = false;
				if (pIt == currentParticipants.cend()) {
					deviceFound = false;
				} else {
					const auto &currentDevices = (*pIt)->getDevices();
					const auto &dIt =
					    std::find_if(currentDevices.cbegin(), currentDevices.cend(), [&d](const auto &currentDevice) {
						    return (*d->getAddress() == *currentDevice->getAddress());
					    });
					deviceFound = (dIt != currentDevices.cend());
				}
				if (!deviceFound) {
					lInfo() << "Device " << *d->getAddress() << " is no longer a member of chatroom or conference "
					        << conferenceAddressString;
					getConference()->notifyParticipantDeviceRemoved(creationTime, isFullState, p, d);
				}
			}
			if (pIt == currentParticipants.cend()) {
				lInfo() << "Participant " << *p->getAddress() << " is no longer a member of chatroom or conference "
				        << conferenceAddressString;
				getConference()->notifyParticipantRemoved(creationTime, isFullState, p);
			}
		}
		for (const auto &d : oldMeDevices) {
			const auto &dIt =
			    std::find_if(currentMeDevices.cbegin(), currentMeDevices.cend(), [&d](const auto &currentDevice) {
				    return (*d->getAddress() == *currentDevice->getAddress());
			    });
			bool deviceFound = (dIt != currentMeDevices.cend());
			if (!deviceFound) {
				lInfo() << "Device " << *d->getAddress() << " is no longer a member of chatroom or conference "
				        << conferenceAddressString;
				getConference()->notifyParticipantDeviceRemoved(creationTime, isFullState, getConference()->getMe(), d);
			}
		}
		getConference()->notifyFullState();
		if (!synchronizing) {
			confListener->onFirstNotifyReceived(getConferenceId().getPeerAddress());
		}
	}
}

void ClientConferenceEventHandler::requestFullState() {
	auto conference = getConference();
	lInfo() << "Requesting full state for conference "
	        << (conference->getConferenceAddress() ? conference->getConferenceAddress()->toString()
	                                               : std::string("sip:unknown"));
	unsubscribe();
	conference->setLastNotify(0);
	subscribe(getConferenceId());
	fullStateRequested = true;
}

LinphoneMediaDirection ClientConferenceEventHandler::mediaStatusToMediaDirection(MediaStatusType status) {
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

bool ClientConferenceEventHandler::notAlreadySubscribed() const {
	return !ev && subscriptionWanted;
}

bool ClientConferenceEventHandler::needToSubscribe() const {
	auto conference = getConference();
	return notAlreadySubscribed() && !managedByListEventhandler &&
	       (conference && (conference->getState() != ConferenceInterface::State::CreationFailed));
}

bool ClientConferenceEventHandler::subscribe() {
	if (!needToSubscribe()) return false; // Already subscribed or application did not request subscription

	const auto localAddress = getConferenceId().getLocalAddress();
	if (!localAddress) return false; // Unknown local address

	auto conference = getConference();
	auto account = conference->getAccount();
	if (!account || (account->getState() != LinphoneRegistrationOk)) {
		return false;
	}

	const auto &conferenceAddress = getConference()->getConferenceAddress();
	if (!conferenceAddress) return false; // Unknown peer address
	ev = dynamic_pointer_cast<EventSubscribe>(
	    (new EventSubscribe(getCore(), conferenceAddress, account, "conference", 600))->toSharedPtr());
	ev->getOp()->setFromAddress(localAddress->getImpl());
	setInitialSubscriptionUnderWayFlag(true);
	const string &lastNotifyStr = Utils::toString(getLastNotify());
	ev->addCustomHeader("Last-Notify-Version", lastNotifyStr.c_str());
	ev->setInternal(true);
	ev->setProperty("event-handler-private", this);
	lInfo() << *localAddress << " is subscribing to chat room or conference: " << *conferenceAddress
	        << " with last notify: " << lastNotifyStr;
	return (ev->send(nullptr) == 0);
}

// -----------------------------------------------------------------------------

void ClientConferenceEventHandler::unsubscribePrivate() {
	if (ev) {
		/* The following tricky code is to break a cycle. Indeed linphone_event_terminate() will change the event's
		 * state, which will be notified to the core, that will call us immediately in invalidateSubscription(), which
		 * resets 'ev' while we still have to unref it.*/
		shared_ptr<EventSubscribe> tmpEv = ev;
		ev = nullptr;
		tmpEv->terminate();
	}
}

void ClientConferenceEventHandler::onNetworkReachable(bool sipNetworkReachable,
                                                      BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe(getConferenceId());
	} else {
		unsubscribePrivate();
	}
}

void ClientConferenceEventHandler::onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
                                                                     LinphoneRegistrationState state,
                                                                     BCTBX_UNUSED(const std::string &message)) {
	const auto &conferenceId = getConferenceId();
	const auto &localAddress = conferenceId.getLocalAddress();
	const auto &params = account->getAccountParams();
	const auto &address = params->getIdentityAddress();
	if (localAddress && address->weakEqual(*localAddress) && (state == LinphoneRegistrationOk)) subscribe(conferenceId);
}

void ClientConferenceEventHandler::onEnteringBackground() {
	unsubscribePrivate();
}

void ClientConferenceEventHandler::onEnteringForeground() {
	subscribe();
}

void ClientConferenceEventHandler::invalidateSubscription() {
	if (ev) {
		if ((ev->getState() == LinphoneSubscriptionError) &&
		    (getConference()->getState() == ConferenceInterface::State::CreationPending)) {
			// The conference received an answer to its SUBSCRIBE and the server is not supporting the conference event
			// package
			getConference()->setState(ConferenceInterface::State::Created);
		}
		ev = nullptr;
	}
}

// -----------------------------------------------------------------------------

void ClientConferenceEventHandler::setSubscriptionWanted(bool wanted) {
	subscriptionWanted = wanted;
}

bool ClientConferenceEventHandler::subscribe(BCTBX_UNUSED(const ConferenceId &conferenceId)) {
	setSubscriptionWanted(true);
	auto ret = subscribe();
	if (ret) {
		waitingFullState = (getLastNotify() == 0);
	} else {
		setSubscriptionWanted(false);
	}
	return ret;
}

void ClientConferenceEventHandler::unsubscribe() {
	unsubscribePrivate();
	setSubscriptionWanted(false);
	setManagedByListEventhandler(false);
}

void ClientConferenceEventHandler::updateInitialSubcriptionUnderWay(std::shared_ptr<Event> notifyLev) {
	if (getInitialSubscriptionUnderWayFlag()) {
		setInitialSubscriptionUnderWayFlag((ev != notifyLev));
	}
}

void ClientConferenceEventHandler::notifyReceived(std::shared_ptr<Event> notifyLev, const Content &content) {
	updateInitialSubcriptionUnderWay(notifyLev);
	notifyReceived(content);
}

void ClientConferenceEventHandler::notifyReceived(const Content &content) {
	lInfo() << "NOTIFY received for conference: " << getConferenceId() << " - Content type "
	        << content.getContentType().getType() << " subtype " << content.getContentType().getSubType();
	const ContentType &contentType = content.getContentType();
	if (contentType == ContentType::ConferenceInfo) {
		conferenceInfoNotifyReceived(content.getBodyAsUtf8String());
	}
}

void ClientConferenceEventHandler::multipartNotifyReceived(std::shared_ptr<Event> notifyLev, const Content &content) {
	updateInitialSubcriptionUnderWay(notifyLev);
	multipartNotifyReceived(content);
}

void ClientConferenceEventHandler::multipartNotifyReceived(const Content &content) {
	lInfo() << "multipart NOTIFY received for conference: " << getConferenceId();
	for (const auto &c : ContentManager::multipartToContentList(content)) {
		notifyReceived(c);
	}
}

shared_ptr<Conference> ClientConferenceEventHandler::getConference() const {
	return conf.lock();
};

const ConferenceId &ClientConferenceEventHandler::getConferenceId() const {
	return getConference() ? getConference()->getConferenceId() : Utils::getEmptyConstRefObject<ConferenceId>();
};

unsigned int ClientConferenceEventHandler::getLastNotify() const {
	return getConference() ? getConference()->getLastNotify() : 0;
};

time_t ClientConferenceEventHandler::dateTimeToTimeT(const Xsd::XmlSchema::DateTime &xsdTime) const {
	tm timeStruct;
	timeStruct.tm_year = (xsdTime.year() - 1900), timeStruct.tm_mon = (xsdTime.month() - 1),
	timeStruct.tm_mday = xsdTime.day(), timeStruct.tm_hour = xsdTime.hours(), timeStruct.tm_min = xsdTime.minutes(),
	timeStruct.tm_sec = static_cast<int>(xsdTime.seconds());
	if (xsdTime.zone_present()) {
		timeStruct.tm_hour += xsdTime.zone_hours();
		timeStruct.tm_min += xsdTime.zone_minutes();
	}
	return Utils::getTmAsTimeT(timeStruct);
}

LINPHONE_END_NAMESPACE
