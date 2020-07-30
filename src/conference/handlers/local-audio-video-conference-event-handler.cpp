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

#include "local-audio-video-conference-event-handler.h"
#include "logger/logger.h"
#include "conference_private.h"

// TODO: remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

LocalAudioVideoConferenceEventHandler::LocalAudioVideoConferenceEventHandler (MediaConference::Conference *conference) : LocalConferenceEventHandler (dynamic_cast<Conference *>(conference)) {

}

MediaConference::Conference *LocalAudioVideoConferenceEventHandler::getMediaConference() const {
	return dynamic_cast<MediaConference::Conference *>(conf);
}

void LocalAudioVideoConferenceEventHandler::onStateChanged (LinphonePrivate::ConferenceInterface::State state) {
	switch(state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationFailed:
		case ConferenceInterface::State::TerminationFailed:
			break;
		case ConferenceInterface::State::CreationPending:
			getMediaConference()->finalizeCreation();
			break;
		case ConferenceInterface::State::TerminationPending:
			if (getMediaConference()->getParticipantCount() == 0) getMediaConference()->setState(ConferenceInterface::State::Terminated);
			break;
		case ConferenceInterface::State::Terminated:
			getMediaConference()->resetLastNotify();
			getMediaConference()->onConferenceTerminated(getMediaConference()->getConferenceAddress());
			break;
		case ConferenceInterface::State::Deleted:
			break;
	}

}

void LocalAudioVideoConferenceEventHandler::setConference(Conference *conference) {
	conf = conference;
}

LINPHONE_END_NAMESPACE
