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

#pragma once

#include "conference/session/call-session.h"
#include "core/core-accessor.h"
#include "private.h"
#include "mediastreamer2/dtmfgen.h"
#include "core/core.h"
#include "tester_utils.h"
#include <map>

LINPHONE_BEGIN_NAMESPACE

class ToneManager : public CoreAccessor {
    public:
        ToneManager(std::shared_ptr<Core> core);
        ~ToneManager();

        // public entrypoints for tones
        void startRingbackTone(const std::shared_ptr<CallSession> &session);
        void startRingtone(const std::shared_ptr<CallSession> &session);
        void startErrorTone(const std::shared_ptr<CallSession> &session, LinphoneReason reason);
        void startNamedTone(const std::shared_ptr<CallSession> &session, LinphoneToneID toneId);
        void goToCall(const std::shared_ptr<CallSession> &session);
        void stop(const std::shared_ptr<CallSession> &session);
        void removeSession(const std::shared_ptr<CallSession> &session);
        void update(const std::shared_ptr<CallSession> &session);

        // linphone core public API entrypoints
        void linphoneCorePlayDtmf(char dtmf, int duration);
        void linphoneCoreStopDtmf();
        LinphoneStatus linphoneCorePlayLocal(const char *audiofile);
        void linphoneCoreStartDtmfStream();
        void linphoneCoreStopRinging();
        void linphoneCoreStopDtmfStream();

        // callback file player
        void onFilePlayerEnd(unsigned int eventId);

        // tester
        LinphoneCoreToneManagerStats *getStats();
        void resetStats();

        // timer
        void deleteTimer();

        //tones setup
        LinphoneToneDescription *getToneFromReason(LinphoneReason reason);
        LinphoneToneDescription *getToneFromId(LinphoneToneID id);
        void setTone(LinphoneReason reason, LinphoneToneID id, const char *audiofile);

    private:
        using AudioResourceType = enum {
            ToneGenerator,
            LocalPlayer
        };

        using State = enum {
            None,     // No tone played, not in call (the session just started or will end soon)
            Call,     // Running call
            Ringback, // Play Ringback tone
            Ringtone, // Play Ringtone or play a tone over the current call
            Tone      // Play a DTMF tone or a tone file
        };
        std::string stateToString(ToneManager::State state);
        void printDebugInfo(const std::shared_ptr<CallSession> &session);

        std::map<std::shared_ptr<CallSession>, ToneManager::State> mSessions;
        belle_sip_source_t *mTimer = nullptr;
        LinphoneCoreToneManagerStats *mStats;

        // timer
        void createTimerToCleanTonePlayer(unsigned int delay);

        //sessions
        void setState(const std::shared_ptr<CallSession> &session, ToneManager::State newState);
        ToneManager::State getState(const std::shared_ptr<CallSession> &session);
        bool isAnotherSessionInState(const std::shared_ptr<CallSession> &me, ToneManager::State state);
        bool getSessionInState(ToneManager::State state, std::shared_ptr<CallSession> &session);
        bool isThereACall();

        // start
        void doStartRingbackTone(const std::shared_ptr<CallSession> &session);
        void doStartRingtone(const std::shared_ptr<CallSession> &session);
        void doStartErrorTone(const std::shared_ptr<CallSession> &session, LinphoneReason reason);
        void doStartNamedTone(const std::shared_ptr<CallSession> &session, LinphoneToneID toneId);

        // stop
        void doStopRingbackTone();
        void doStopTone();
        void doStopToneToPlaySomethingElse(const std::shared_ptr<CallSession> &session);
        void doStopRingtone(const std::shared_ptr<CallSession> &session);
        void doStop(const std::shared_ptr<CallSession> &session, ToneManager::State newState);

        // sound
        MSFilter *getAudioResource(AudioResourceType rtype, MSSndCard *card, bool create);
        LinphoneStatus playFile(const char *audiofile);
        void playTone(const std::shared_ptr<CallSession> &session, MSDtmfGenCustomTone dtmf);
        MSDtmfGenCustomTone generateToneFromId(LinphoneToneID toneId);
};

LINPHONE_END_NAMESPACE
