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

#ifndef _L_CALL_SESSION_H_
#define _L_CALL_SESSION_H_

#include <mediastreamer2/msrtt4103.h>

#include "address/address.h"
#include "call/call-stats.h"
#include "conference/conference-id.h"
#include "conference/params/call-session-params.h"
#include "core/core-accessor.h"
#include "core/core-listener.h"
#include "linphone/misc.h"
#include "object/object.h"
#include "sal/call-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Alert;
class CallLog;
class CallSessionPrivate;
class CallSessionListener;
class Content;
class Core;
class Conference;

class LINPHONE_PUBLIC CallSession : public Object, public CoreAccessor {
	friend class Call;
	friend class Conference;
	friend class Core;
	friend class CorePrivate;
	friend class ServerConference;
	friend class ClientConference;
	friend class ClientChatRoom;
	friend class ServerChatRoom;
	friend class ParticipantDevice;

public:
	L_OVERRIDE_SHARED_FROM_THIS(CallSession);

	enum class State {
		Idle = LinphoneCallStateIdle,
		IncomingReceived = LinphoneCallStateIncomingReceived,
		PushIncomingReceived = LinphoneCallStatePushIncomingReceived,
		OutgoingInit = LinphoneCallStateOutgoingInit,
		OutgoingProgress = LinphoneCallStateOutgoingProgress,
		OutgoingRinging = LinphoneCallStateOutgoingRinging,
		OutgoingEarlyMedia = LinphoneCallStateOutgoingEarlyMedia,
		Connected = LinphoneCallStateConnected,
		StreamsRunning = LinphoneCallStateStreamsRunning,
		Pausing = LinphoneCallStatePausing,
		Paused = LinphoneCallStatePaused,
		Resuming = LinphoneCallStateResuming,
		Referred = LinphoneCallStateReferred,
		Error = LinphoneCallStateError,
		End = LinphoneCallStateEnd,
		PausedByRemote = LinphoneCallStatePausedByRemote,
		UpdatedByRemote = LinphoneCallStateUpdatedByRemote,
		IncomingEarlyMedia = LinphoneCallStateIncomingEarlyMedia,
		Updating = LinphoneCallStateUpdating,
		Released = LinphoneCallStateReleased,
		EarlyUpdatedByRemote = LinphoneCallStateEarlyUpdatedByRemote,
		EarlyUpdating = LinphoneCallStateEarlyUpdating
	};
	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert((int)CallSession::State::EarlyUpdating == (int)LinphoneCallStateEarlyUpdating,
	              "LinphoneCallState and CallSession::State are not synchronized, fix this !");

	enum class UpdateMethod { Default = 0, Invite = 1, Update = 2 };

	enum class PredefinedSubjectType {
		Conference = 0,
		InternalUpdate = 1,
		Refresh = 2,
		MediaChange = 3,
		CallOnHold = 4,
		BothPartiesOnHold = 5,
		Resuming = 6
	};

	static const std::map<PredefinedSubjectType, std::string> predefinedSubject;

	CallSession(const std::shared_ptr<Core> &core, const CallSessionParams *params);
	void addListener(std::shared_ptr<CallSessionListener> listener);
	void removeListener(const std::shared_ptr<CallSessionListener> &listener);
	void clearListeners();
	void setStateToEnded();
	~CallSession();

	// This virtual is a dirty hack until CallSession/MediaSession are refactored.
	virtual void acceptDefault();
	LinphoneStatus accept(const CallSessionParams *csp = nullptr);
	LinphoneStatus acceptUpdate(const CallSessionParams *csp = nullptr);
	virtual void configure(LinphoneCallDir direction,
	                       const std::shared_ptr<Account> &account,
	                       SalCallOp *op,
	                       const std::shared_ptr<const Address> &from,
	                       const std::shared_ptr<const Address> &to);
	virtual void configure(LinphoneCallDir direction, const std::string &callid);
	bool isOpConfigured();
	LinphoneStatus decline(LinphoneReason reason);
	LinphoneStatus decline(const LinphoneErrorInfo *ei);
	LinphoneStatus declineNotAnswered(LinphoneReason reason);
	virtual LinphoneStatus deferUpdate();
	bool hasTransferPending();
	bool sdpFoundInLocalBody() const;
	bool sdpFoundInRemoteBody() const;
	bool isCapabilityNegotiationEnabled() const;
	const std::list<LinphoneMediaEncryption> getSupportedEncryptions() const;
	virtual void initiateIncoming();
	virtual bool initiateOutgoing(const std::string &subject = "",
	                              const std::shared_ptr<const Content> content = nullptr);
	virtual void iterate(time_t currentRealTime, bool oneSecondElapsed);
	LinphoneStatus redirect(const std::string &redirectUri);
	LinphoneStatus redirect(const Address &redirectAddr);
	virtual void startIncomingNotification(bool notifyRinging = true);
	void startBasicIncomingNotification(bool notifyRinging = true);
	void startPushIncomingNotification();
	virtual int startInvite(const std::shared_ptr<Address> &destination,
	                        const std::string &subject = "",
	                        const std::shared_ptr<const Content> content = nullptr);
	LinphoneStatus terminate(const LinphoneErrorInfo *ei = nullptr);
	LinphoneStatus transfer(const std::shared_ptr<CallSession> &dest);
	LinphoneStatus transfer(const Address &dest);
	LinphoneStatus update(const CallSessionParams *csp,
	                      const UpdateMethod method = UpdateMethod::Default,
	                      const std::string &subject = "",
	                      const std::shared_ptr<Content> content = nullptr);

	CallSessionParams *getCurrentParams() const;
	LinphoneCallDir getDirection() const;
	Address getDiversionAddress() const;
	int getDuration() const;
	const LinphoneErrorInfo *getErrorInfo() const;
	const std::shared_ptr<Address> getLocalAddress() const;
	const std::shared_ptr<Address> getContactAddress() const;
	std::shared_ptr<CallLog> getLog() const;
	virtual const CallSessionParams *getParams() const;
	LinphoneReason getReason() const;
	std::shared_ptr<CallSession> getReferer() const;
	const std::string &getReferTo() const;
	const std::shared_ptr<Address> getReferToAddress() const;
	std::shared_ptr<const Address> getReferredBy() const;
	const std::shared_ptr<Address> getRemoteAddress() const;
	const std::string &getRemoteContact() const;
	const std::shared_ptr<Address> getRemoteContactAddress() const;
	const CallSessionParams *getRemoteParams();
	const std::string &getRemoteUserAgent() const;
	std::shared_ptr<CallSession> getReplacedCallSession() const;
	CallSession::State getState() const;
	const std::shared_ptr<Address> getToAddress() const;
	const std::shared_ptr<Address> getRequestAddress() const;
	CallSession::State getTransferState() const;
	std::shared_ptr<CallSession> getTransferTarget() const;
	const char *getToHeader(const std::string &name) const;

	const std::string getFromTag() const;
	const std::string getToTag() const;

	void updateContactAddressInOp();

	static bool isEarlyState(CallSession::State state);
	void accepting();
	// bool isDelinedEarly ();
	// const LinphoneErrorInfo * getErrorInfoCache () const;

	void addPendingAction(std::function<LinphoneStatus()> f);

	static bool isPredefinedSubject(const std::string &subject);

	void notifyCameraNotWorking(const char *cameraName);
	void notifyResetFirstVideoFrameDecoded();
	void notifyFirstVideoFrameDecoded();
	void notifySnapshotTaken(const char *filepath);
	void notifyRealTimeTextCharacterReceived(RealtimeTextReceivedCharacter *data);
#ifdef HAVE_BAUDOT
	void notifyBaudotCharacterReceived(char receivedCharacter);
	void notifyBaudotDetected(MSBaudotStandard standard);
#endif /* HAVE_BAUDOT */
	void notifySendMasterKeyChanged(const std::string key);
	void notifyReceiveMasterKeyChanged(const std::string key);
	void notifyUpdateMediaInfoForReporting(const int type);
	void notifyRtcpUpdateForReporting(SalStreamType type);
	void notifyStatsUpdated(const std::shared_ptr<CallStats> &stats);
	void notifyTmmbrReceived(const int index, const int max_bitrate);
	void notifyAlert(std::shared_ptr<Alert> &alert);
	void notifyCallSessionStateChanged(CallSession::State newState, const std::string &message);
	void notifyCallSessionTransferStateChanged(CallSession::State newState);
	void notifyCallSessionStateChangedForReporting();
	void notifyStartRingtone();
	void notifyIncomingCallSessionTimeoutCheck(int elapsed, bool oneSecondElapsed);
	void notifyPushCallSessionTimeoutCheck(int elapsed);
	void notifyIncomingCallSessionNotified();
	void notifyIncomingCallSessionStarted();
	void notifyCallSessionAccepting();
	void notifyCallSessionAccepted();
	void notifyCallSessionEarlyFailed(LinphoneErrorInfo *ei);
	void notifyCallSessionSetTerminated();
	void notifyCallSessionSetReleased();
	void notifyCheckForAcceptation();
	void notifyGoClearAckSent();
	void notifyAckBeingSent(LinphoneHeaders *headers);
	void notifyAckReceived(LinphoneHeaders *headers);
	void notifyBackgroundTaskToBeStarted();
	void notifyBackgroundTaskToBeStopped();
	void notifyInfoReceived(LinphoneInfoMessage *info);
	bool isPlayingRingbackTone();
	bool areSoundResourcesAvailable();
	void notifyLossOfMediaDetected();
	void notifySetCurrentSession();
	void notifyResetCurrentSession();
	void notifyDtmfReceived(char dtmf);
	void notifyRemoteRecording(bool isRecording);
	void notifySecurityLevelDowngraded();
	void notifyEncryptionChanged(bool activated, const std::string &authToken);
	void notifyAuthenticationTokenVerified(bool verified);
	void notifyVideoDisplayErrorOccurred(int errorCode);

protected:
	explicit CallSession(CallSessionPrivate &p, const std::shared_ptr<Core> &core);
	CallSession::State getPreviousState() const;
	CallSession::State getLastStableState() const;
	void updateContactAddress(Address &contactAddress) const;
	void assignAccount(const std::shared_ptr<Account> &account);

private:
	// bool mIsDeclining = false;
	bool mIsAccepting = false;
	// LinphoneErrorInfo *mErrorCache = nullptr;
	L_DECLARE_PRIVATE(CallSession);
	L_DISABLE_COPY(CallSession);
};

inline std::ostream &operator<<(std::ostream &str, CallSession::State state) {
	str << linphone_call_state_to_string(static_cast<LinphoneCallState>(state));
	return str;
}

inline std::ostream &operator<<(std::ostream &str, const CallSession &callSession) {
	str << "CallSession [" << &callSession << "]";
	return str;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_H_
