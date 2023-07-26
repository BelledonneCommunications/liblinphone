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

#include "remote-conference-event-handler.h"

#include <sstream>

#include <xsd/cxx/xml/string.hxx>

#include <bctoolbox/defs.h>

#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"

#include "conference.h"
#include "conference/participant.h"
#include "conference/remote-conference.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "event/event-subscribe.h"
#include "logger/logger.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::ConferenceInfoLinphoneExtension;

// -----------------------------------------------------------------------------

RemoteConferenceEventHandler::RemoteConferenceEventHandler(Conference *remoteConference, ConferenceListener *listener) {
	conf = remoteConference;
	confListener = listener;
	conf->getCore()->getPrivate()->registerListener(this);
}

RemoteConferenceEventHandler::~RemoteConferenceEventHandler() {

	try {
		conf->getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::setInitialSubscriptionUnderWayFlag(bool on) {
	initialSubscriptionUnderWay = on;
}

bool RemoteConferenceEventHandler::getInitialSubscriptionUnderWayFlag() const {
	return initialSubscriptionUnderWay;
}

void RemoteConferenceEventHandler::conferenceInfoNotifyReceived(const string &xmlBody) {
	istringstream data(xmlBody);
	unique_ptr<ConferenceType> confInfo;
	try {
		confInfo = parseConferenceInfo(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing conference-info notify for: " << getConferenceId();
		return;
	}

	const auto &core = conf->getCore();
	const auto &conferenceAddress = conf->getConferenceAddress();
	const std::string conferenceAddressString =
	    conferenceAddress ? conferenceAddress->toString() : std::string("<unknown>");
	auto chatRoom = core->findChatRoom(getConferenceId());

	std::shared_ptr<Address> entityAddress = Address::create(confInfo->getEntity());
	const auto &peerAddress = getConferenceId().getPeerAddress();

	if (!peerAddress || (*entityAddress != *peerAddress)) {
		const std::string peerAddressString = peerAddress ? peerAddress->toString() : std::string("<unknown>");
		lError() << "Unable to process received NOTIFY because the entity address " << entityAddress->toString()
		         << " doesn't match the peer address " << peerAddressString;
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
			conf->setLastNotify(version.get());
			if (chatRoom) {
				// Update last notify ID in the DB just in case the notify does not generate any further event
				conf->getCore()->getPrivate()->mainDb->updateNotifyId(chatRoom, getLastNotify());
			}
		}
	}

	// 3. Notify ephemeral settings, media, subject and keywords.
	if (confDescription.present()) {
		auto &subject = confDescription.get().getSubject();
		if (subject.present() && !subject.get().empty()) {
			if (conf->getUtf8Subject() != subject.get()) {
				conf->Conference::setSubject(Utils::utf8ToLocale(subject.get()));
				if (!isFullState) {
					// Subject must be stored in the system locale
					conf->notifySubjectChanged(creationTime, isFullState, subject.get());
				}
			}
		}

		auto &keywords = confDescription.get().getKeywords();
		if (keywords.present() && !keywords.get().empty()) {
			KeywordsType xmlKeywords = keywords.get();
			confListener->onConferenceKeywordsChanged(vector<string>(xmlKeywords.begin(), xmlKeywords.end()));
		}

		const auto &availableMedia = confDescription.get().getAvailableMedia();
		if (availableMedia.present()) {
			for (auto &mediaEntry : availableMedia.get().getEntry()) {
				const std::string mediaType = mediaEntry.getType();
				const LinphoneMediaDirection mediaDirection =
				    RemoteConferenceEventHandler::mediaStatusToMediaDirection(mediaEntry.getStatus().get());
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
				conf->notifyAvailableMediaChanged(creationTime, isFullState, conf->getMediaCapabilities());
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
								conf->notifyEphemeralLifetimeChanged(creationTime, isFullState, lifetime);

								conf->notifyEphemeralMessageEnabled(creationTime, isFullState, (lifetime != 0));
							}
						}
					} else if (ephemeralMode.compare("device-managed") == 0) {
						cgcr->getCurrentParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged);
					}
				}
			} else if (nodeName == "linphone-cie:crypto-security-level") {
				CryptoSecurityLevel cryptoSecurityLevel{anyElement};
				auto securityLevelString = cryptoSecurityLevel.getLevel();
				auto securityLevel = ConferenceParams::getSecurityLevelFromAttribute(securityLevelString);
				conf->confParams->setSecurityLevel(securityLevel);
				conf->updateSecurityLevelInConferenceInfo(securityLevel);
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

	auto oldParticipants = conf->getParticipants();
	auto oldMeDevices = conf->getMe()->getDevices();
	if (isFullState) {
		confListener->onParticipantsCleared();
	}

	auto &users = confInfo->getUsers();
	if (!users.present()) return;

	// 4. Notify changes on users.
	for (auto &user : users->getUser()) {
		std::shared_ptr<Address> address = core->interpretUrl(user.getEntity().get(), false);
		StateType state = user.getState();

		shared_ptr<Participant> participant = nullptr;
		if (conf->isMe(address)) participant = conf->getMe();
		else participant = conf->findParticipant(address);

		const auto &pIt =
		    std::find_if(oldParticipants.cbegin(), oldParticipants.cend(), [&address](const auto &currentParticipant) {
			    return (*address == *currentParticipant->getAddress());
		    });

		if (state == StateType::deleted) {
			if (conf->isMe(address)) {
				lInfo() << "Participant " << address->toString() << " requested to be deleted is me.";
				continue;
			} else if (participant) {
				conf->participants.remove(participant);
				lInfo() << "Participant " << *participant << " is successfully removed - conference "
				        << conferenceAddressString << " has " << conf->getParticipantCount() << " participants";
				if (!isFullState) {
					conf->notifyParticipantRemoved(creationTime, isFullState, participant);
				}

				continue;
			} else {
				lWarning() << "Participant " << address->toString() << " removed but not in the list of participants!";
			}
		} else if (state == StateType::full) {
			if (conf->isMe(address)) {
				lInfo() << "Participant " << address->toString() << " requested to be added is me.";
			} else if (participant) {
				lWarning() << "Participant " << *participant << " added but already in the list of participants!";
			} else {
				participant = Participant::create(conf, address);
				conf->participants.push_back(participant);
				conf->updateParticipantsInConferenceInfo(address);
				lInfo() << "Participant " << *participant << " is successfully added - conference "
				        << conferenceAddressString << " has " << conf->getParticipantCount() << " participants";
				if (!isFullState ||
				    (!oldParticipants.empty() && (pIt == oldParticipants.cend()) && !conf->isMe(address))) {
					conf->notifyParticipantAdded(creationTime, isFullState, participant);
				}
			}
		}

		if (!participant) {
			// Try to get participant again as it may have been added or removed earlier on
			if (conf->isMe(address)) participant = conf->getMe();
			else participant = conf->findParticipant(address);
		}

		if (!participant) {
			lDebug() << "Participant " << address->toString()
			         << " is not in the list of participants however it is trying to change the list of devices or "
			            "change role! Resubscribing to conference "
			         << conferenceAddressString << " to clear things up.";
			requestFullState();
		} else {

			auto &roles = user.getRoles();
			if (roles) {

				auto &entry = roles->getEntry();
				bool isAdmin = (find(entry, "admin") != entry.end() ? true : false);

				if (participant->isAdmin() != isAdmin) {

					participant->setAdmin(isAdmin);

					if (!isFullState) {
						conf->notifyParticipantSetAdmin(creationTime, isFullState, participant, isAdmin);
					}
				}
			}

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
							conf->notifyParticipantDeviceRemoved(creationTime, isFullState, participant, device);
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
					bool mediaCapabilityChanged = false;
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

						LinphoneMediaDirection mediaDirection =
						    RemoteConferenceEventHandler::mediaStatusToMediaDirection(media.getStatus().get());
						mediaCapabilityChanged |= device->setStreamCapability(mediaDirection, streamType);
						if (media.getLabel()) {
							const std::string label = media.getLabel().get();
							if (!label.empty()) {
								device->setLabel(label, streamType);
							}
						}
						if (mediaDirection == LinphoneMediaDirectionInactive) {
							device->setSsrc(streamType, 0);
						} else {
							if (media.getSrcId()) {
								const std::string srcId = media.getSrcId().get();
								unsigned long ssrc = std::stoul(srcId);
								device->setSsrc(streamType, (uint32_t)ssrc);
							}
						}
					}

					bool mediaAvailabilityChanged = device->updateStreamAvailabilities();
					// Do not notify availability changed during full states and participant addition because it is
					// already done by the listener method onFullStateReceived
					if (!isFullState && (state != StateType::full) &&
					    (previousDeviceState != ParticipantDevice::State::ScheduledForJoining) &&
					    (previousDeviceState != ParticipantDevice::State::Joining) &&
					    (previousDeviceState != ParticipantDevice::State::Alerting)) {
						if (mediaAvailabilityChanged) {
							conf->notifyParticipantDeviceMediaAvailabilityChanged(creationTime, isFullState,
							                                                      participant, device);
						}

						if (mediaCapabilityChanged) {
							conf->notifyParticipantDeviceMediaCapabilityChanged(creationTime, isFullState, participant,
							                                                    device);
						}
					}

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

					if (conf->isMe(address) && conf->getMainSession()) device->setSession(conf->getMainSession());

					if (state == StateType::full) {
						lInfo() << "Participant device " << gruu->toString() << " has been successfully added";
						bool sendNotify =
						    (!oldParticipants.empty() && (pIt == oldParticipants.cend())) && !conf->isMe(address);
						if (pIt != oldParticipants.cend()) {
							const auto &oldDevices = (*pIt)->getDevices();
							const auto &dIt =
							    std::find_if(oldDevices.cbegin(), oldDevices.cend(), [&gruu](const auto &oldDevice) {
								    return (*gruu == *oldDevice->getAddress());
							    });
							sendNotify = (dIt == oldDevices.cend()) && !conf->isMe(address);
						}
						if (!isFullState || sendNotify) {
							conf->notifyParticipantDeviceAdded(creationTime, isFullState, participant, device);
						}
					} else {
						lInfo() << "Participant device " << gruu->toString() << " has been successfully updated";
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
		auto currentParticipants = conf->getParticipants();
		auto currentMeDevices = conf->getMe()->getDevices();
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
					conf->notifyParticipantDeviceRemoved(creationTime, isFullState, p, d);
				}
			}
			if (pIt == currentParticipants.cend()) {
				lInfo() << "Participant " << *p->getAddress() << " is no longer a member of chatroom or conference "
				        << conferenceAddressString;
				conf->notifyParticipantRemoved(creationTime, isFullState, p);
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
				conf->notifyParticipantDeviceRemoved(creationTime, isFullState, conf->getMe(), d);
			}
		}
		conf->notifyFullState();
		if (!synchronizing) {
			confListener->onFirstNotifyReceived(getConferenceId().getPeerAddress());
		}
	}
}

void RemoteConferenceEventHandler::requestFullState() {
	lInfo() << "Requesting full state for conference "
	        << (conf->getConferenceAddress() ? conf->getConferenceAddress()->toString()
	                                         : std::string("<unknown conference address>"));
	unsubscribe();
	conf->setLastNotify(0);
	subscribe(getConferenceId());
	fullStateRequested = true;
}

LinphoneMediaDirection RemoteConferenceEventHandler::mediaStatusToMediaDirection(MediaStatusType status) {
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
	return (!ev && subscriptionWanted);
}

void RemoteConferenceEventHandler::subscribe() {
	if (!alreadySubscribed()) return; // Already subscribed or application did not request subscription

	const auto localAddress = getConferenceId().getLocalAddress();

	LinphoneCore *lc = conf->getCore()->getCCore();
	LinphoneAccount *acc = linphone_core_lookup_account_by_identity(lc, localAddress->toC());

	if (!acc || (linphone_account_get_state(acc) != LinphoneRegistrationOk)) {
		return;
	}

	const auto &peerAddress = getConferenceId().getPeerAddress();
	ev = dynamic_pointer_cast<EventSubscribe>(
	    (new EventSubscribe(conf->getCore(), peerAddress, Account::toCpp(acc)->getConfig(), "conference", 600))
	        ->toSharedPtr());
	ev->getOp()->setFromAddress(localAddress->getImpl());
	setInitialSubscriptionUnderWayFlag(true);
	const string &lastNotifyStr = Utils::toString(getLastNotify());
	ev->addCustomHeader("Last-Notify-Version", lastNotifyStr.c_str());
	ev->setInternal(true);
	ev->setProperty("event-handler-private", this);
	lInfo() << *localAddress << " is subscribing to chat room or conference: " << *peerAddress
	        << " with last notify: " << lastNotifyStr;
	ev->send(nullptr);
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::unsubscribePrivate() {
	if (ev) {
		/* The following tricky code is to break a cycle. Indeed linphone_event_terminate() will change the event's
		 * state, which will be notified to the core, that will call us immediately in invalidateSubscription(), which
		 * resets 'ev' while we still have to unref it.*/
		shared_ptr<EventSubscribe> tmpEv = ev;
		ev = nullptr;
		tmpEv->terminate();
	}
}

void RemoteConferenceEventHandler::onNetworkReachable(bool sipNetworkReachable,
                                                      BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe();
	} else {
		unsubscribePrivate();
	}
}

void RemoteConferenceEventHandler::onAccountRegistrationStateChanged(BCTBX_UNUSED(std::shared_ptr<Account> account),
                                                                     LinphoneRegistrationState state,
                                                                     BCTBX_UNUSED(const std::string &message)) {
	if (state == LinphoneRegistrationOk) subscribe();
}

void RemoteConferenceEventHandler::onEnteringBackground() {
	unsubscribePrivate();
}

void RemoteConferenceEventHandler::onEnteringForeground() {
	subscribe();
}

void RemoteConferenceEventHandler::invalidateSubscription() {
	if (ev) {
		if ((ev->getState() == LinphoneSubscriptionError) &&
		    (conf->getState() == ConferenceInterface::State::CreationPending)) {
			// The conference received an answer to its SUBSCRIBE and the server is not supporting the conference event
			// package
			conf->setState(ConferenceInterface::State::Created);
		}
		ev = nullptr;
	}
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::subscribe(const ConferenceId &conferenceId) {
	conf->setConferenceId(conferenceId);
	subscriptionWanted = true;
	waitingFullState = (getLastNotify() == 0);
	subscribe();
}

void RemoteConferenceEventHandler::unsubscribe() {
	unsubscribePrivate();
	subscriptionWanted = false;
}

void RemoteConferenceEventHandler::updateInitialSubcriptionUnderWay(std::shared_ptr<Event> notifyLev) {
	if (getInitialSubscriptionUnderWayFlag()) {
		;
		setInitialSubscriptionUnderWayFlag((ev != notifyLev));
	}
}

void RemoteConferenceEventHandler::notifyReceived(std::shared_ptr<Event> notifyLev, const Content &content) {
	updateInitialSubcriptionUnderWay(notifyLev);
	notifyReceived(content);
}

void RemoteConferenceEventHandler::notifyReceived(const Content &content) {
	lInfo() << "NOTIFY received for conference: " << getConferenceId() << " - Content type "
	        << content.getContentType().getType() << " subtype " << content.getContentType().getSubType();
	const ContentType &contentType = content.getContentType();
	if (contentType == ContentType::ConferenceInfo) {
		conferenceInfoNotifyReceived(content.getBodyAsUtf8String());
	}
}

void RemoteConferenceEventHandler::multipartNotifyReceived(std::shared_ptr<Event> notifyLev, const Content &content) {
	updateInitialSubcriptionUnderWay(notifyLev);
	multipartNotifyReceived(content);
}

void RemoteConferenceEventHandler::multipartNotifyReceived(const Content &content) {
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

time_t RemoteConferenceEventHandler::dateTimeToTimeT(const Xsd::XmlSchema::DateTime &xsdTime) const {
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
