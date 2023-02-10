/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_CALL_SESSION_LISTENER_H_
#define _L_CALL_SESSION_LISTENER_H_

#include <bctoolbox/defs.h>

#include "conference/session/call-session.h"

#include <mediastreamer2/msrtt4103.h>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;

class LINPHONE_PUBLIC CallSessionListener {
public:
	virtual ~CallSessionListener() = default;

	virtual void onAckBeingSent (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(LinphoneHeaders *headers)) {}
	virtual void onAckReceived (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(LinphoneHeaders *headers)) {}
	virtual void onBackgroundTaskToBeStarted (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onBackgroundTaskToBeStopped (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onCallSessionAccepting (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual bool onCallSessionAccepted (UNUSED(const std::shared_ptr<CallSession> &session)) { return false; }
	virtual void onCallSessionEarlyFailed (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(LinphoneErrorInfo *ei)) {}
	virtual void onCallSessionSetReleased (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onCallSessionSetTerminated (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onCallSessionStartReferred (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onCallSessionStateChanged (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(CallSession::State state), UNUSED(const std::string &message)) {}
	virtual void onCallSessionTransferStateChanged (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(CallSession::State state)) {}
	virtual void onCheckForAcceptation (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onDtmfReceived (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(char dtmf)) {}
	virtual void onIncomingCallSessionNotified (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onIncomingCallSessionStarted (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onIncomingCallSessionTimeoutCheck (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(int elapsed), UNUSED(bool oneSecondElapsed)) {}
	virtual void onPushCallSessionTimeoutCheck (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(int elapsed)) {}
	virtual void onInfoReceived (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(const LinphoneInfoMessage *im)) {}
	virtual void onLossOfMediaDetected (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onTmmbrReceived (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(int streamIndex), UNUSED(int tmmbr)) {}
	virtual void onSnapshotTaken(UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(const char *file_path)) {}
	virtual void onStartRingtone(UNUSED(const std::shared_ptr<CallSession> &session)){}
	virtual void onRemoteRecording(UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(bool recording)){}

	virtual void onEncryptionChanged (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(bool activated), UNUSED(const std::string &authToken)) {}
	virtual void onGoClearAckSent() {}

	virtual void onCallSessionStateChangedForReporting (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onRtcpUpdateForReporting (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(SalStreamType type)) {}
	virtual void onStatsUpdated (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(const LinphoneCallStats *stats)) {}
	virtual void onUpdateMediaInfoForReporting (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(int statsType)) {}

	virtual void onResetCurrentSession (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onSetCurrentSession (UNUSED(const std::shared_ptr<CallSession> &session)) {}

	virtual void onFirstVideoFrameDecoded (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onResetFirstVideoFrameDecoded (UNUSED(const std::shared_ptr<CallSession> &session)) {}
	virtual void onCameraNotWorking (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(const char *camera_name)) {}

	virtual bool areSoundResourcesAvailable (UNUSED(const std::shared_ptr<CallSession> &session)) { return true; }
	virtual bool isPlayingRingbackTone (UNUSED(const std::shared_ptr<CallSession> &session)) { return false; }

	virtual LinphoneConference * getCallSessionConference (UNUSED(const std::shared_ptr<CallSession> &session)) const { return nullptr; }

	virtual void onRealTimeTextCharacterReceived (UNUSED(const std::shared_ptr<CallSession> &session), UNUSED(RealtimeTextReceivedCharacter *data)) {}

	virtual void confirmGoClear() const {};
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_LISTENER_H_
