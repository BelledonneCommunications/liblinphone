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

#ifndef _L_CALL_SESSION_LISTENER_H_
#define _L_CALL_SESSION_LISTENER_H_

#include "conference/session/call-session.h"

#include <mediastreamer2/msrtt4103.h>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;

class LINPHONE_PUBLIC CallSessionListener {
public:
	virtual ~CallSessionListener() = default;

	virtual void onAckBeingSent (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) {}
	virtual void onAckReceived (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) {}
	virtual void onBackgroundTaskToBeStarted (const std::shared_ptr<CallSession> &session) {}
	virtual void onBackgroundTaskToBeStopped (const std::shared_ptr<CallSession> &session) {}
	virtual bool onCallSessionAccepted (const std::shared_ptr<CallSession> &session) { return false; }
	virtual void onCallSessionEarlyFailed (const std::shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) {}
	virtual void onCallSessionSetReleased (const std::shared_ptr<CallSession> &session) {}
	virtual void onCallSessionSetTerminated (const std::shared_ptr<CallSession> &session) {}
	virtual void onCallSessionStartReferred (const std::shared_ptr<CallSession> &session) {}
	virtual void onCallSessionStateChanged (const std::shared_ptr<CallSession> &session, CallSession::State state, const std::string &message) {}
	virtual void onCallSessionTransferStateChanged (const std::shared_ptr<CallSession> &session, CallSession::State state) {}
	virtual void onCheckForAcceptation (const std::shared_ptr<CallSession> &session) {}
	virtual void onDtmfReceived (const std::shared_ptr<CallSession> &session, char dtmf) {}
	virtual void onIncomingCallSessionNotified (const std::shared_ptr<CallSession> &session) {}
	virtual void onIncomingCallSessionStarted (const std::shared_ptr<CallSession> &session) {}
	virtual void onIncomingCallSessionTimeoutCheck (const std::shared_ptr<CallSession> &session, int elapsed, bool oneSecondElapsed) {}
	virtual void onInfoReceived (const std::shared_ptr<CallSession> &session, const LinphoneInfoMessage *im) {}
	virtual void onLossOfMediaDetected (const std::shared_ptr<CallSession> &session) {}
	virtual void onTmmbrReceived (const std::shared_ptr<CallSession> &session, int streamIndex, int tmmbr) {}
	virtual void onSnapshotTaken(const std::shared_ptr<CallSession> &session, const char *file_path) {}

	virtual void onEncryptionChanged (const std::shared_ptr<CallSession> &session, bool activated, const std::string &authToken) {}

	virtual void onCallSessionStateChangedForReporting (const std::shared_ptr<CallSession> &session) {}
	virtual void onRtcpUpdateForReporting (const std::shared_ptr<CallSession> &session, SalStreamType type) {}
	virtual void onStatsUpdated (const std::shared_ptr<CallSession> &session, const LinphoneCallStats *stats) {}
	virtual void onUpdateMediaInfoForReporting (const std::shared_ptr<CallSession> &session, int statsType) {}

	virtual void onResetCurrentSession (const std::shared_ptr<CallSession> &session) {}
	virtual void onSetCurrentSession (const std::shared_ptr<CallSession> &session) {}

	virtual void onFirstVideoFrameDecoded (const std::shared_ptr<CallSession> &session) {}
	virtual void onResetFirstVideoFrameDecoded (const std::shared_ptr<CallSession> &session) {}
	virtual void onCameraNotWorking (const std::shared_ptr<CallSession> &session, const char *camera_name) {}


	virtual bool areSoundResourcesAvailable (const std::shared_ptr<CallSession> &session) { return true; }
	virtual bool isPlayingRingbackTone (const std::shared_ptr<CallSession> &session) { return false; }

	virtual void onRealTimeTextCharacterReceived (const std::shared_ptr<CallSession> &session, RealtimeTextReceivedCharacter *data) {}
	
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_LISTENER_H_
