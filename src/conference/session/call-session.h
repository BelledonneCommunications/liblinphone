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

#ifndef _L_CALL_SESSION_H_
#define _L_CALL_SESSION_H_

#include "object/object.h"
#include "address/address.h"
#include "conference/conference.h"
#include "conference/params/call-session-params.h"
#include "core/core-listener.h"
#include "sal/call-op.h"
#include "linphone/misc.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSessionPrivate;
class Content;
class Core;

class LINPHONE_PUBLIC CallSession : public Object, public CoreAccessor {
	friend class Call;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class Conference;
	friend class CorePrivate;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;
	friend class ParticipantDevice;

public:
	L_OVERRIDE_SHARED_FROM_THIS(CallSession);

	enum class State{
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
		EarlyUpdating
	};
	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)CallSession::State::EarlyUpdating == (int)LinphoneCallStateEarlyUpdating, "LinphoneCallState and CallSession::State are not synchronized, fix this !");

	CallSession (const std::shared_ptr<Core> &core, const CallSessionParams *params, CallSessionListener *listener);
	void setListener(CallSessionListener *listener);
	~CallSession ();

	// This virtual is a dirty hack until CallSession/MediaSession are refactored.
	virtual void acceptDefault();
	LinphoneStatus accept (const CallSessionParams *csp = nullptr);
	LinphoneStatus acceptUpdate (const CallSessionParams *csp = nullptr);
	virtual void configure (LinphoneCallDir direction, LinphoneProxyConfig *cfg, SalCallOp *op, const Address &from, const Address &to);
	void configure (LinphoneCallDir direction, const std::string &callid);
	bool isOpConfigured ();
	LinphoneStatus decline (LinphoneReason reason);
	LinphoneStatus decline (const LinphoneErrorInfo *ei);
	LinphoneStatus declineNotAnswered (LinphoneReason reason);
	virtual LinphoneStatus deferUpdate ();
	bool hasTransferPending ();
	virtual void initiateIncoming ();
	virtual bool initiateOutgoing ();
	virtual void iterate (time_t currentRealTime, bool oneSecondElapsed);
	LinphoneStatus redirect (const std::string &redirectUri);
	LinphoneStatus redirect (const Address &redirectAddr);
	virtual void startIncomingNotification (bool notifyRinging = true);
	void startBasicIncomingNotification (bool notifyRinging = true);
	void startPushIncomingNotification ();
	virtual int startInvite (const Address *destination, const std::string &subject = "", const Content *content = nullptr);
	LinphoneStatus terminate (const LinphoneErrorInfo *ei = nullptr);
	LinphoneStatus transfer (const std::shared_ptr<CallSession> &dest);
	LinphoneStatus transfer (const std::string &dest);
	LinphoneStatus update (const CallSessionParams *csp, const std::string &subject = "", const Content *content = nullptr);

	CallSessionParams *getCurrentParams () const;
	LinphoneCallDir getDirection () const;
	const Address &getDiversionAddress () const;
	int getDuration () const;
	const LinphoneErrorInfo * getErrorInfo () const;
	const Address &getLocalAddress () const;
	LinphoneCallLog *getLog () const;
	virtual const CallSessionParams *getParams () const;
	LinphoneReason getReason () const;
	std::shared_ptr<CallSession> getReferer () const;
	const std::string &getReferTo () const;
	const Address *getRemoteAddress () const;
	const std::string &getRemoteContact () const;
	const Address *getRemoteContactAddress () const;
	const CallSessionParams *getRemoteParams ();
	const std::string &getRemoteUserAgent () const;
	std::shared_ptr<CallSession> getReplacedCallSession () const;
	CallSession::State getState () const;
	const Address &getToAddress () const;
	CallSession::State getTransferState () const;
	std::shared_ptr<CallSession> getTransferTarget () const;
	const char *getToHeader (const std::string &name) const;

	static bool isEarlyState (CallSession::State state);
	void accepting ();

protected:
	explicit CallSession (CallSessionPrivate &p, const std::shared_ptr<Core> &core);
	CallSession::State getPreviousState () const;

private:
	bool mIsDeclining = false;
	bool mIsAccepting = false;
	LinphoneErrorInfo *mErrorCache = nullptr;
	L_DECLARE_PRIVATE(CallSession);
	L_DISABLE_COPY(CallSession);
};

inline std::ostream & operator << (std::ostream & str, CallSession::State state){
	str << linphone_call_state_to_string(static_cast<LinphoneCallState>(state));
	return str;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_H_
