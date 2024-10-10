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

#ifndef _L_CALL_SESSION_P_H_
#define _L_CALL_SESSION_P_H_

#include <functional>
#include <queue>

#include "object/object-p.h"

#include "call-session.h"
#include "sal/call-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_INTERNAL_PUBLIC CallSessionPrivate : public ObjectPrivate, public CoreListener {
public:
	int computeDuration() const;
	void handleIncoming(bool tryStartRingtone);
	virtual void initializeParamsAccordingToIncomingCallParams();
	void notifyReferState();
	virtual void setState(CallSession::State newState, const std::string &message);
	void restorePreviousState();
	void setTransferState(CallSession::State newState);
	void startIncomingNotification();
	bool startPing();
	void setPingTime(int value) {
		pingTime = value;
	}

	void createOp();
	CallSessionParams *getCurrentParams() const {
		return currentParams;
	}
	const std::shared_ptr<Account> &getDestAccount() const {
		return account;
	}
	SalCallOp *getOp() const {
		return op;
	}
	bool isBroken() const {
		return broken;
	}
	bool reportEvents() const;
	bool isInConference() const;
	const std::string getConferenceId() const;
	void setConferenceId(const std::string id);
	void setParams(CallSessionParams *csp);
	void setReferPending(bool value) {
		referPending = value;
	}
	void setTransferTarget(std::shared_ptr<CallSession> session) {
		transferTarget = session;
	}

	virtual void abort(const std::string &errorMsg);
	virtual void accepted();
	void ackBeingSent(LinphoneHeaders *headers);
	virtual void ackReceived(LinphoneHeaders *headers);
	void cancelDone();
	virtual bool failure();
	void infoReceived(SalBodyHandler *bodyHandler);
	void pingReply();
	void referred(const std::shared_ptr<Address> &referToAddr);
	virtual void remoteRinging();
	virtual void replaceOp(SalCallOp *newOp);
	virtual void terminated();
	void updated(bool isUpdate);
	void updatedByRemote();
	virtual void updating(bool isUpdate);
	virtual void refreshed(); /* Called when an incoming UPDATE is received (for session timers)*/

	void init();

	void accept(const CallSessionParams *params);
	void acceptOrTerminateReplacedSessionInIncomingNotification();
	virtual LinphoneStatus
	acceptUpdate(const CallSessionParams *csp, CallSession::State nextState, const std::string &stateInfo);
	LinphoneStatus checkForAcceptation();
	virtual void handleIncomingReceivedStateInIncomingNotification();
	virtual bool isReadyForInvite() const;
	bool isUpdateAllowed(CallSession::State &nextState) const;
	virtual int restartInvite();
	virtual void setReleased();
	virtual void setTerminated();
	virtual LinphoneStatus startAcceptUpdate(CallSession::State nextState, const std::string &stateInfo);
	virtual LinphoneStatus startUpdate(const CallSession::UpdateMethod method = CallSession::UpdateMethod::Default,
	                                   const std::string &subject = "");
	virtual void terminate();
	virtual void updateCurrentParams() const;
	virtual void
	setDestAccount(const std::shared_ptr<Account> &account); // Set destProxy and update the proxy of currentParams

	void setBroken();
	void setContactOp();

	virtual void reinviteToRecoverFromConnectionLoss();
	virtual void repairByNewInvite(bool withReplaces);

	// CoreListener
	void onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onRegistrationStateChanged(LinphoneProxyConfig *cfg,
	                                LinphoneRegistrationState cstate,
	                                const std::string &message) override;
	void onCallStateChanged(LinphoneCall *call, LinphoneCallState state, const std::string &message) override;
	void updateToFromAssertedIdentity();

protected:
	std::list<std::weak_ptr<CallSessionListener>> listeners;
	CallSessionParams *params = nullptr;
	mutable CallSessionParams *currentParams = nullptr;
	mutable CallSessionParams *remoteParams = nullptr;
	mutable std::shared_ptr<Address> diversionAddress;

	std::string subject;
	LinphoneCallDir direction = LinphoneCallOutgoing;
	CallSession::State state = CallSession::State::Idle;
	std::string messageState;
	CallSession::State prevState = CallSession::State::Idle;
	std::string prevMessageState;
	CallSession::State lastStableState = CallSession::State::Idle;
	std::string lastStableMessageState;
	CallSession::State transferState = CallSession::State::Idle;
	std::shared_ptr<Account> account = nullptr;
	LinphoneErrorInfo *ei = nullptr;
	std::shared_ptr<CallLog> log = nullptr;
	mutable std::string referTo;
	std::shared_ptr<Address> referToAddress;
	mutable std::shared_ptr<Address> requestAddress;
	mutable std::shared_ptr<Address> mRemoteContactAddress;
	mutable std::shared_ptr<Address> mReferredBy;
	// This counter is used to keep active track of reINVITEs and UPDATEs under processing at any given time.
	// In fact Linphone can have multiple active transaction at the same time on the same dialog as the transaction
	// queue is popped after receiving the 100 Trying and not the 200 Ok
	int nbProcessingUpdates = 0;

	SalCallOp *op = nullptr;

	SalOp *pingOp = nullptr;
	bool pingReplied = false;
	int pingTime = 0;

	std::shared_ptr<CallSession> referer;
	std::shared_ptr<CallSession> transferTarget;

	bool broken = false;
	bool deferIncomingNotification = false;
	bool deferUpdate = false;
	bool deferUpdateInternal = false;
	mutable bool needLocalIpRefresh = false;
	bool nonOpError = false; /* Set when the LinphoneErrorInfo was set at higher level than sal */
	bool notifyRinging = true;
	bool referPending = false;
	bool reinviteOnCancelResponseRequested = false;
	std::queue<std::function<LinphoneStatus()>> pendingActions;

private:
	void completeLog();
	void createOpTo(const std::shared_ptr<Address> &to);
	void executePendingActions();
	void refreshContactAddress();

	std::shared_ptr<Address> getFixedContact() const;

	void repairIfBroken();

	L_DECLARE_PUBLIC(CallSession);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_P_H_
