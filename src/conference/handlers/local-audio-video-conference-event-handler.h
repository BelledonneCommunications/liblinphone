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

#ifndef _L_LOCAL_AUDIO_VIDEO_CONFERENCE_EVENT_HANDLER_H_
#define _L_LOCAL_AUDIO_VIDEO_CONFERENCE_EVENT_HANDLER_H_

#include "local-conference-event-handler.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;

}



class LINPHONE_PUBLIC LocalAudioVideoConferenceEventHandler : public LocalConferenceEventHandler {
friend class LocalConferenceListEventHandler;
#ifdef LINPHONE_TESTER
	friend class Tester;
#endif
public:
	LocalAudioVideoConferenceEventHandler (MediaConference::Conference *conference);

	/*
	 * This fonction is called each time the conference transitions to a new state
	 * @param[in] state new state of the conference
	 */
	void onStateChanged (LinphonePrivate::ConferenceInterface::State state) override;
	void resetConference ();

	MediaConference::Conference *getMediaConference() const;
private:

	L_DISABLE_COPY(LocalAudioVideoConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_AUDIO_VIDEO_CONFERENCE_EVENT_HANDLER_H_
