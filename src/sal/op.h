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

#ifndef _L_SAL_OP_H_
#define _L_SAL_OP_H_

#include <bctoolbox/defs.h>
#include <bctoolbox/list.h>
#include <bctoolbox/ownership.hh>
#include <bctoolbox/port.h>
#include <belle-sip/types.h>

#include "c-wrapper/internal/c-sal.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "content/content.h"
#include "logger/logger.h"
#include "sal/sal.h"
#include "sal/sal_media_description.h"

using namespace ownership;

LINPHONE_BEGIN_NAMESPACE

class SalMessageOpInterface;

class LINPHONE_PUBLIC SalOp {
public:
	enum class Dir { Incoming = 0, Outgoing = 1 };

	SalOp(Sal *sal);
	virtual ~SalOp();

	SalOp *ref();
	void *unref();

	Sal *getSal() const {
		return mRoot;
	}

	void setUserPointer(void *value) {
		mUserPointer = value;
	}
	void *getUserPointer() const {
		return mUserPointer;
	}

	void setSubject(const std::string &value) {
		mSubject = value;
	}
	const std::string &getSubject() const {
		return mSubject;
	}

	const Dir &getDir() const {
		return mDir;
	}

	void setFrom(const std::string &value);
	void setFromAddress(const SalAddress *value);
	const std::string &getFrom() const {
		return mFrom;
	}
	const SalAddress *getFromAddress() const {
		return mFromAddress;
	}

	void setTo(const std::string &value);
	void setToAddress(const SalAddress *value);
	const std::string &getTo() const {
		return mTo;
	}
	const SalAddress *getToAddress() const {
		return mToAddress;
	}

	const SalAddress *getRequestAddress() {
		return mRequestAddress.borrow();
	}

	void setRequestUri(const std::string &value) {
		mRequestUri = value;
	}
	const std::string &getRequestUri() const {
		return mRequestUri;
	}

	void setContactAddress(const SalAddress *value);
	/* The contact address returned here is be pub-gruu provided by the proxy during REGISTER transaction, if "gruu"
	 is supported by client and server, otherwise the low-level transport address.*/
	const SalAddress *getContactAddress() const {
		return mContactAddress;
	}

	void setRoute(const std::string &value);
	void setRouteAddress(const SalAddress *value);
	const std::list<SalAddress *> &getRouteAddresses() const {
		return mRouteAddresses;
	}
	void addRouteAddress(const SalAddress *address);

	void setDiversionAddress(const SalAddress *value);
	const SalAddress *getDiversionAddress() const {
		return mDiversionAddress;
	}

	void setServiceRoute(const SalAddress *value);
	const SalAddress *getServiceRoute() const {
		return mServiceRoute;
	}

	void setManualRefresherMode(bool value) {
		mManualRefresher = value;
	}

	void setEntityTag(const std::string &value) {
		mEntityTag = value;
	}
	const std::string &getEntityTag() const {
		return mEntityTag;
	}

	void setEvent(const std::string &eventName);

	void setPrivacy(SalPrivacyMask value) {
		mPrivacy = value;
	}
	SalPrivacyMask getPrivacy() const {
		return mPrivacy;
	}

	void setRealm(const std::string &value) {
		mRealm = value;
	}

	void setSentCustomHeaders(SalCustomHeader *ch);

	void enableCnxIpTo0000IfSendOnly(bool value) {
		mCnxIpTo0000IfSendOnlyEnabled = value;
	}
	bool cnxIpTo0000IfSendOnlyEnabled() const {
		return mCnxIpTo0000IfSendOnlyEnabled;
	}

	const std::string &getProxy() const {
		return mRoute;
	}
	const std::string &getNetworkOrigin() const {
		return mOrigin;
	}
	const SalAddress *getNetworkOriginAddress() const {
		return mOriginAddress;
	}
	const std::string &getCallId() const {
		return mCallId;
	}
	std::string getDialogId() const;
	int getAddressFamily() const;
	void setRecvCustomHeaders(SalCustomHeader *ch);
	const SalCustomHeader *getRecvCustomHeaders() const {
		return mRecvCustomHeaders;
	}
	// overrideRemoteContact: Used by testers. Do not use in production code
	void overrideRemoteContact(const std::string &value);
	const std::string &getRemoteContact() const {
		return mRemoteContact;
	}
	const SalAddress *getRemoteContactAddress() const {
		return mRemoteContactAddress;
	}
	const std::string &getRemoteUserAgent() const {
		return mRemoteUserAgent;
	}

	const char *getPublicAddress(int *port) {
		return mRefresher ? belle_sip_refresher_get_public_address(mRefresher, port) : nullptr;
	}
	const char *getLocalAddress(int *port) {
		return mRefresher ? belle_sip_refresher_get_local_address(mRefresher, port) : nullptr;
	}

	const SalErrorInfo *getErrorInfo() const {
		return &mErrorInfo;
	}
	const SalErrorInfo *getReasonErrorInfo() const {
		return &mReasonErrorInfo;
	}

	bool isForkedOf(const SalOp *op) const {
		return !mCallId.empty() && !op->mCallId.empty() && (mCallId == op->mCallId);
	}
	bool isIdle() const;

	void stopRefreshing() {
		if (mRefresher) belle_sip_refresher_stop(mRefresher);
	}
	int refresh();
	bool hasDialog() const {
		return mDialog != nullptr;
	}
	bool isDialogEstablished() {
		return mDialog != nullptr && belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_CONFIRMED;
	}
	void killDialog();
	// Returns the remote CSeq, only in case the op has a dialog.
	unsigned int getRemoteCSeq();
	// Release means let the op finish its life but we don't want to use it anymore, and don't want to be called in
	// callbacks for this op
	void release();

	virtual void authenticate() {
		processAuthentication();
	}
	void cancelAuthentication() {
		lFatal() << "SalOp::cancelAuthentication not implemented yet";
	}
	SalAuthInfo *getAuthRequested() {
		return mAuthInfo;
	}

	int ping(const SalAddress *from, const SalAddress *to);
	int sendInfo(const SalBodyHandler *bodyHandler);

	int replyMessage(SalReason reason);
	int
	replyWithErrorInfo(const SalErrorInfo *info, const SalAddress *redirectionAddr = nullptr, const time_t expire = 0);
	/* Set a function to be called whenever an operation encouters a "491 request pending" response.
	 * The function shall retry the operation, based on the new context. */
	void setRetryFunction(const std::function<void()> &retryFunc);
	bool hasRetryFunction() const;
	void resetRetryFunction();
	int processRedirect();
	/* to restrict the use of connections for this op, see in belle-sip. */
	void setChannelBankIdentifier(const std::string &identifier);

	belle_sip_dialog_t *getDialog() const {
		return mDialog;
	}

	void makeSupportedHeader(const std::list<std::string> &supportedTags);

protected:
	enum class State {
		Early = 0,
		Active = 1,
		Terminating = 2, // This state is used to wait until a proceeding state, so we can send the cancel
		Terminated = 3
	};

	static std::string toString(const State value);

	enum class Type {
		Unknown = 0,
		Register = 1,
		Call = 2,
		Message = 3,
		Presence = 4,
		Publish = 5,
		Subscribe = 6,
		Refer = 7 // For out of dialog refer only
	};

	static std::string toString(const Type type);

	using ReleaseCb = void (*)(SalOp *op);

	virtual void fillCallbacks() {
	}
	void releaseImpl();
	void processAuthentication();

	belle_sip_request_t *buildRequest(const std::string &method);
	int sendRequest(belle_sip_request_t *request);
	int sendRequestWithContact(belle_sip_request_t *request, bool addContact);
	int sendRequestWithExpires(belle_sip_request_t *request, int expires);
	void resendRequest(belle_sip_request_t *request);
	int
	sendRequestAndCreateRefresher(belle_sip_request_t *request, int expires, belle_sip_refresher_listener_t listener);

	void setReasonErrorInfo(belle_sip_message_t *message);
	void setErrorInfoFromResponse(belle_sip_response_t *response);
	void resetErrorInfo();

	void setReferredBy(belle_sip_header_referred_by_t *referredByHeader);
	void setReplaces(belle_sip_header_replaces_t *replacesHeader);

	void setRemoteContact(const std::string &value);
	void setNetworkOrigin(const std::string &value);
	void setNetworkOriginAddress(SalAddress *value);
	void setPrivacyFromMessage(belle_sip_message_t *message);
	void setRemoteUserAgent(belle_sip_message_t *message);
	void setRequestAddress(BorrowedMut<SalAddress> value);

	belle_sip_response_t *createResponseFromRequest(belle_sip_request_t *request, int code) {
		return mRoot->createResponseFromRequest(request, code);
	}
	belle_sip_header_contact_t *createContact(bool forceSipInstance = false);
	belle_sip_header_t *createWarningHeader(const SalErrorInfo *ei, const SalAddress *serverAddr);

	void setOrUpdateDialog(belle_sip_dialog_t *dialog);
	belle_sip_dialog_t *linkOpWithDialog(belle_sip_dialog_t *dialog);
	void unlinkOpFromDialog(belle_sip_dialog_t *dialog);

	SalBodyHandler *getBodyHandler(belle_sip_message_t *message);

	void assignRecvHeaders(belle_sip_message_t *message);

	bool isSecure() const;
	void addHeaders(belle_sip_header_t *h, belle_sip_message_t *message);
	void addCustomHeaders(belle_sip_message_t *message);
	int unsubscribe();

	void processIncomingMessage(const belle_sip_request_event_t *event);
	void addMessageAccept(belle_sip_message_t *message);

	/* Handling of 491 Request pending. */
	bool runRetryFunc();
	bool handleRetry();

	static int setCustomBody(belle_sip_message_t *msg, const Content &body);

	static bool isExternalBody(belle_sip_header_content_type_t *contentType);

	static void assignAddress(SalAddress **address, std::string &addressStr, const std::string &value);
	static void assignAddress(SalAddress **address, std::string &addressStr, const SalAddress *value);
	static void addInitialRouteSet(belle_sip_request_t *request, const std::list<SalAddress *> &routeAddresses);

	static belle_sip_header_reason_t *makeReasonHeader(const SalErrorInfo *info);

	// SalOpBase
	Sal *mRoot = nullptr;
	std::string mRoute; // Or request-uri for REGISTER
	std::list<SalAddress *> mRouteAddresses;
	SalAddress *mContactAddress = nullptr;
	std::string mSubject;
	std::string mFrom;
	SalAddress *mFromAddress = nullptr;
	std::string mTo;
	SalAddress *mToAddress = nullptr;
	Owned<SalAddress> mRequestAddress = nullptr;
	std::string mRequestUri;
	std::string mOrigin;
	SalAddress *mOriginAddress = nullptr;
	SalAddress *mDiversionAddress = nullptr;
	std::string mRemoteUserAgent;
	SalAddress *mRemoteContactAddress = nullptr;
	std::string mRemoteContact;
	void *mUserPointer = nullptr;
	std::string mCallId = std::string();
	std::string mRealm;
	SalAddress *mServiceRoute = nullptr; // As defined by rfc3608, might be a list
	SalCustomHeader *mSentCustomHeaders = nullptr;
	SalCustomHeader *mRecvCustomHeaders = nullptr;
	std::string mEntityTag; // As defined by rfc3903 (I.E publih)
	std::string mChannelBankIdentifier;
	ReleaseCb mReleaseCb = nullptr;
	belle_sip_header_t *mSupportedHeader = nullptr;

	const belle_sip_listener_callbacks_t *mCallbacks = nullptr;
	SalErrorInfo mErrorInfo;
	SalErrorInfo mReasonErrorInfo;
	belle_sip_client_transaction_t *mPendingAuthTransaction = nullptr;
	belle_sip_server_transaction_t *mPendingServerTransaction = nullptr;
	belle_sip_server_transaction_t *mPendingUpdateServerTransaction = nullptr;
	belle_sip_client_transaction_t *mPendingClientTransaction = nullptr;
	SalAuthInfo *mAuthInfo = nullptr;
	belle_sip_dialog_t *mDialog = nullptr;
	belle_sip_header_replaces_t *mReplaces = nullptr;
	belle_sip_header_referred_by_t *mReferredBy = nullptr;
	std::shared_ptr<SalMediaDescription> mResult = nullptr;
	belle_sdp_session_description_t *mSdpAnswer = nullptr;
	State mState = State::Early;
	Dir mDir = Dir::Incoming;
	belle_sip_refresher_t *mRefresher = nullptr;
	int mRef = 0;
	Type mType = Type::Unknown;
	SalPrivacyMask mPrivacy = SalPrivacyNone;
	belle_sip_header_event_t *mEvent = nullptr; // Used by SalOpSubscribe kinds
	SalOpSDPHandling mSdpHandling = SalOpSDPNormal;
	int mAuthRequests = 0; // number of auth requested for this op
	belle_sip_source_t *mSessionTimersTimer = nullptr;
	std::function<void()> mRetryFunc;
	bool mCnxIpTo0000IfSendOnlyEnabled = false;
	bool mAutoAnswerAsked = false;
	bool mSdpOffering = false;
	bool mCallReleased = false;
	bool mManualRefresher = false;
	bool mHasAuthPending = false;
	bool mSupportsSessionTimers = false;
	bool mOpReleased = false;
	bool mOwnsDialog = true;
	bool mUseSupportedTags = false;

	friend class Sal;
	friend class Call;
	friend class SalMessageOpInterface;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_OP_H_
