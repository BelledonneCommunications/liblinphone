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

#include <mediastreamer2/msrtt4103.h>

#include "conference/session/call-session.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Alert;
class CallSession;

class LINPHONE_PUBLIC CallSessionListener {
public:
	virtual ~CallSessionListener() = default;

	virtual void onAckBeingSent(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                            BCTBX_UNUSED(LinphoneHeaders *headers)) {
	}
	virtual void onAckReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                           BCTBX_UNUSED(LinphoneHeaders *headers)) {
	}
	virtual void onBackgroundTaskToBeStarted(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onBackgroundTaskToBeStopped(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCallSessionAccepting(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCallSessionAccepted(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCallSessionEarlyFailed(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                      BCTBX_UNUSED(LinphoneErrorInfo *ei)) {
	}
	virtual void onCallSessionSetReleased(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCallSessionSetTerminated(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCallSessionStateChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                       BCTBX_UNUSED(CallSession::State state),
	                                       BCTBX_UNUSED(const std::string &message)) {
	}
	virtual void onCallSessionTransferStateChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                               BCTBX_UNUSED(CallSession::State state)) {
	}
	virtual void onCheckForAcceptation(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onDtmfReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), BCTBX_UNUSED(char dtmf)) {
	}
	virtual void onIncomingCallSessionNotified(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onIncomingCallSessionStarted(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onIncomingCallSessionTimeoutCheck(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                               BCTBX_UNUSED(int elapsed),
	                                               BCTBX_UNUSED(bool oneSecondElapsed)) {
	}
	virtual void onPushCallSessionTimeoutCheck(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                           BCTBX_UNUSED(int elapsed)) {
	}
	virtual void onInfoReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                            BCTBX_UNUSED(const LinphoneInfoMessage *im)) {
	}
	virtual void onLossOfMediaDetected(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onTmmbrReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                             BCTBX_UNUSED(int streamIndex),
	                             BCTBX_UNUSED(int tmmbr)) {
	}
	virtual void onSnapshotTaken(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                             BCTBX_UNUSED(const char *file_path)) {
	}
	virtual void onStartRingtone(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onRemoteRecording(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                               BCTBX_UNUSED(bool recording)) {
	}

	virtual void onSecurityLevelDowngraded(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onEncryptionChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                 BCTBX_UNUSED(bool activated),
	                                 BCTBX_UNUSED(const std::string &authToken)) {
	}
	virtual void onAuthenticationTokenVerified(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                           BCTBX_UNUSED(bool verified)) {
	}
	virtual void onSendMasterKeyChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                    BCTBX_UNUSED(const std::string &masterKey)){};
	virtual void onReceiveMasterKeyChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                       BCTBX_UNUSED(const std::string &masterKey)){};
	virtual void onGoClearAckSent() {
	}

	virtual void onCallSessionStateChangedForReporting(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onRtcpUpdateForReporting(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                      BCTBX_UNUSED(SalStreamType type)) {
	}
	virtual void onStatsUpdated(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                            BCTBX_UNUSED(const std::shared_ptr<CallStats> &stats)) {
	}
	virtual void onUpdateMediaInfoForReporting(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                           BCTBX_UNUSED(int statsType)) {
	}

	virtual void onResetCurrentSession(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onSetCurrentSession(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}

	virtual void onFirstVideoFrameDecoded(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onResetFirstVideoFrameDecoded(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	}
	virtual void onCameraNotWorking(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                BCTBX_UNUSED(const char *camera_name)) {
	}
	virtual void onVideoDisplayErrorOccurred(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                         BCTBX_UNUSED(int error_code)) {
	}

	virtual bool areSoundResourcesAvailable(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
		return true;
	}
	virtual bool isPlayingRingbackTone(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
		return false;
	}

	virtual void onRealTimeTextCharacterReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                             BCTBX_UNUSED(RealtimeTextReceivedCharacter *data)) {
	}

#ifdef HAVE_BAUDOT
	virtual void onBaudotCharacterReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                                       BCTBX_UNUSED(char receivedCharacter)) {
	}
	virtual void onBaudotDetected(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
	                              BCTBX_UNUSED(MSBaudotStandard standard)) {
	}
#endif /* HAVE_BAUDOT */

	virtual void confirmGoClear() const {};

	virtual void onAlertNotified(BCTBX_UNUSED(std::shared_ptr<Alert> &alert)){};

	virtual std::unique_ptr<LogContextualizer> getLogContextualizer() {
		return nullptr;
	};
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_LISTENER_H_
