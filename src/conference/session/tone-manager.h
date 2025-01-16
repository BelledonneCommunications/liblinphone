/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#pragma once

#include "conference/session/call-session.h"
#include "core/core-accessor.h"
#include "core/core.h"
#include "mediastreamer2/dtmfgen.h"
#include "private.h"
#include "tester_utils.h"
#include <map>

LINPHONE_BEGIN_NAMESPACE

/*
 * The ToneManager is in charge of scheduling rings, ringbacks, and various tones that come along with the life
 * of calls.
 */
class ToneManager {
public:
	ToneManager(Core &core);
	ToneManager(const ToneManager &other) = delete;
	~ToneManager() = default;

	/*
	 * The CallSession's state change notification are sufficient to trigger rings and tones.
	 * The ToneManager also needs to be informed about a transition to a future state, that's why
	 * there are to entry points:
	 * - prepareForNextState()
	 * - notifyState();
	 * They are driven by the MediaSession.
	 */
	void prepareForNextState(const std::shared_ptr<CallSession> &session, CallSession::State nextState);
	void notifyState(const std::shared_ptr<CallSession> &session, CallSession::State nextState);

	void notifySecurityAlert(const std::shared_ptr<CallSession> &session);
	void stopSecurityAlert();

	/* Below are a few accessors required by other parts of liblinphone.*/
	void playDtmf(char dtmf, int duration);
	void stopDtmf();
	LinphoneStatus playLocal(const char *audiofile);
	void startDtmfStream();
	void stopDtmfStream();

	/* Used to temporarily override the audio output device. */
	void setOutputDevice(const std::shared_ptr<CallSession> &session, const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getOutputDevice(const std::shared_ptr<CallSession> &session) const;

	/* Request the tone manager to immediately abandon any sound card usage. All running rings or tones are dropped. */
	void freeAudioResources();

	// tester
	const LinphoneCoreToneManagerStats *getStats() const;
	void resetStats();

	// Tone configuration.
	LinphoneToneDescription *getTone(LinphoneToneID id);
	void setTone(LinphoneToneID id, const char *audiofile);

private:
	using AudioResourceType = enum { ToneGenerator = 0, LocalPlayer = 1 };

	LinphoneCoreToneManagerStats mStats;

	void notifyIncomingCall(const std::shared_ptr<CallSession> &session);
	void notifyOutgoingCallRinging(const std::shared_ptr<CallSession> &session);
	void notifyToneIndication(LinphoneReason reason);
	void destroyRingStream();

	// start
	void startRingbackTone();
	void startRingtone();
	void startErrorTone(LinphoneReason reason);
	void startNamedTone(LinphoneToneID toneId);

	// stop
	void stopRingbackTone();
	void stopTone();
	void stopRingtone();

	void scheduleRingStreamDestruction();
	void updateRingingSessions(const std::shared_ptr<CallSession> &callSession, CallSession::State state);

	Core &getCore() const {
		return mCore;
	}

	// sound
	MSFilter *getAudioResource(AudioResourceType rtype, MSSndCard *card, bool create);
	LinphoneStatus playFile(const char *audiofile);
	void playTone(const MSDtmfGenCustomTone &tone);
	MSDtmfGenCustomTone generateToneFromId(LinphoneToneID toneId);
	void cleanPauseTone();
	bool inCallOrConference() const;
	bool shouldPlayWaitingTone(const std::shared_ptr<CallSession> &session);
	std::shared_ptr<CallSession> lookupRingingSession() const;
	Core &mCore;
	RingStream *mRingStream = nullptr;
	std::shared_ptr<CallSession> mSessionRinging;
	std::function<void()> mSessionRingingStopFunction;
	std::shared_ptr<CallSession> mSessionRingingBack;
	std::shared_ptr<CallSession> mSessionPaused;
	belle_sip_source_t *mRingStreamTimer = nullptr;
	belle_sip_source_t *mSecurityAlertTimer = nullptr;
	bool mDtmfStreamStarted = false;
};

LINPHONE_END_NAMESPACE
