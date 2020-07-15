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
#include "xml/conference-info.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;

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

void RemoteConferenceEventHandler::simpleNotifyReceived (const string &xmlBody) {
	istringstream data(xmlBody);
	unique_ptr<ConferenceType> confInfo;
	try {
		confInfo = parseConferenceInfo(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing conference notify for: " << getConferenceId();
		return;
	}

	IdentityAddress entityAddress(confInfo->getEntity().c_str());
	if (entityAddress != getConferenceId().getPeerAddress())
		return;

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

	// 3. Notify subject and keywords.
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
			if (!participant) {
				lWarning() << "Participant " << address.asString() << " removed but not in the list of participants!";
			} else {
				conf->participants.remove(participant);
				if (!isFullState) {
					conf->notifyParticipantRemoved(
						creationTime,
						isFullState,
						address
					);
				}

				continue;
			}
		}

		if (state == StateType::full) {
			if (conf->isMe(address)) {
				lWarning() << "Participant " << participant << " is the conference itself!";
			} else if (participant) {
				lWarning() << "Participant " << participant << " added but already in the list of participants!";
			} else {
				participant = Participant::create(conf,address);
				conf->participants.push_back(participant);

				if (!isFullState) {
					conf->notifyParticipantAdded(
						creationTime,
						isFullState,
						address
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
							address,
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

				if (state == StateType::deleted) {

					participant->removeDevice(gruu);

					if (!isFullState) {
					conf->notifyParticipantDeviceRemoved(
						creationTime,
						isFullState,
						address,
						gruu
					);
					}
				} else if (state == StateType::full) {

					shared_ptr<ParticipantDevice> device = participant->addDevice(gruu);

					const string &name = endpoint.getDisplayText().present() ? endpoint.getDisplayText().get() : "";

					if (!name.empty())
						device->setName(name);

					if (!isFullState) {
						conf->notifyParticipantDeviceAdded(
							creationTime,
							isFullState,
							address,
							gruu,
							name
						);
					}
				}
			}
		}
	}

	if (isFullState) {
		confListener->onFirstNotifyReceived(getConferenceId().getPeerAddress());
		conf->notifyFullState();
	}
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::subscribe () {
	if (lev || !subscriptionWanted)
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
	linphone_event_set_user_data(lev, this);
	lInfo() << "Subscribing to chat room or conference: " << peerAddress << "with last notify: " << lastNotifyStr;
	linphone_event_send_subscribe(lev, nullptr);
}

// -----------------------------------------------------------------------------

void RemoteConferenceEventHandler::unsubscribePrivate () {
	if (lev) {
		linphone_event_terminate(lev);
		lev = nullptr;
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
	lev = nullptr;
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

void RemoteConferenceEventHandler::notifyReceived (const string &xmlBody) {

	lInfo() << "NOTIFY received for conference: " << getConferenceId();

	simpleNotifyReceived(xmlBody);
}

void RemoteConferenceEventHandler::multipartNotifyReceived (const string &xmlBody) {

	lInfo() << "multipart NOTIFY received for conference: " << getConferenceId();

	Content multipart;
	multipart.setBodyFromUtf8(xmlBody);
	ContentType contentType(ContentType::Multipart);
	contentType.addParameter("boundary", MultipartBoundary);
	multipart.setContentType(contentType);

	for (const auto &content : ContentManager::multipartToContentList(multipart))
		simpleNotifyReceived(content.getBodyAsUtf8String());
}

const ConferenceId &RemoteConferenceEventHandler::getConferenceId() const {
	return conf->getConferenceId();
};

unsigned int RemoteConferenceEventHandler::getLastNotify() const {
	return conf->getLastNotify();
};

LINPHONE_END_NAMESPACE
