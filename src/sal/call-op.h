/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_SAL_CALL_OP_H_
#define _L_SAL_CALL_OP_H_

#include <optional>

#include "sal/message-op-interface.h"
#include "sal/op.h"

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC SalCallOp : public SalOp, public SalMessageOpInterface {
public:
	SalCallOp(Sal *sal, const bool capabilityNegotiation = false);
	virtual ~SalCallOp();

	void enableCapabilityNegotiation(const bool enable);
	bool capabilityNegotiationEnabled() const;

	std::shared_ptr<SalMediaDescription> getLocalMediaDescription() const {
		return mLocalMedia;
	}
	int setLocalMediaDescription(std::shared_ptr<SalMediaDescription> desc);
	void addLocalBody(const Content &content);
	const std::list<Content> &getLocalBodies() const {
		return mLocalBodies;
	}
	void setLocalBodies(const std::list<Content> &contents) {
		mLocalBodies = contents;
	}
	const std::list<Content> &getRemoteBodies() const;
	bool isContentInRemote(const ContentType &contentType) const;
	std::optional<std::reference_wrapper<const Content>> getContentInLocal(const ContentType &contentType) const;
	std::optional<std::reference_wrapper<const Content>> getContentInRemote(const ContentType &contentType) const;

	std::shared_ptr<SalMediaDescription> getSalMediaDescriptionFromContent(const Content &content);

	const std::shared_ptr<SalMediaDescription> &getRemoteMediaDescription() {
		return mRemoteMedia;
	}
	std::shared_ptr<SalMediaDescription> &getFinalMediaDescription();

	int call(const SalAddress *from, const SalAddress *to, const std::string &subject);
	int notifyRinging(bool earlyMedia, const LinphoneSupportLevel supportLevel100Rel);
	int accept();
	int decline(SalReason reason, const std::string &redirectionUri = "");

	void haltSessionTimersTimer();
	void restartSessionTimersTimer(belle_sip_response_t *response, int delta);
	bool canSendRequest(bool noUserConsent, bool logError = false);
	int update(const std::string &subject, bool noUserConsent);
	int update(const std::string &subject, bool noUserConsent, bool withSDP, int delta);
	int cancelInvite(const SalErrorInfo *info = nullptr);

	int refer(const std::string &referToUri);
	int referWithReplaces(SalCallOp *otherCallOp);
	int setReferrer(SalCallOp *referredCall);
	const SalAddress *getReferredBy() const;
	SalCallOp *getReplaces() const;
	int sendDtmf(char dtmf);
	int terminate(const SalErrorInfo *info = nullptr);
	bool autoAnswerAsked() const {
		return mAutoAnswerAsked;
	}
	void sendVfuRequest();
	int isOfferer() const {
		return mSdpOffering;
	}
	int notifyReferState(SalCallOp *newCallOp);
	bool compareOp(const SalCallOp *otherCallOp) const;
	bool dialogRequestPending() const {
		return (belle_sip_dialog_request_pending(mDialog) != 0);
	}
	const char *getLocalTag();
	const char *getRemoteTag();
	void setReplaces(const std::string &callId, const std::string &fromTag, const std::string &toTag);
	void setSdpHandling(SalOpSDPHandling handling);

	// Implementation of SalMessageOpInterface
	int sendMessage(const Content &content) override;
	int reply(SalReason reason) override {
		return SalOp::replyMessage(reason);
	}
	void setNotifyAllRingings(bool yesno) {
		mNotifyAllRingings = yesno;
	}
	bool getNotifyAllRingings() const {
		return mNotifyAllRingings;
	}

	// This is for testing only!
	void simulateLostAckOnDialog(bool enable);

private:
	void fillCallbacks() override;
	void setReleased();

	void setError(belle_sip_response_t *response, bool fatal);
	void callTerminated(belle_sip_server_transaction_t *serverTransaction,
	                    int statusCode,
	                    belle_sip_request_t *cancelRequest);
	void resetDescriptions();

	int parseSdpBody(const Content &body, belle_sdp_session_description_t **sessionDesc, SalReason *error);
	void sdpProcess();
	void fillRemoteBodies(const Content &body);
	void handleBodyFromResponse(belle_sip_response_t *response);
	void handleSessionTimersFromResponse(belle_sip_response_t *response);
	SalReason processBodyForInvite(belle_sip_request_t *invite);
	SalReason processBodyForAck(belle_sip_request_t *ack);
	void handleOfferAnswerResponse(belle_sip_response_t *response);

	void fillInvite(belle_sip_request_t *invite);
	void fillSessionExpiresHeaders(belle_sip_request_t *invite);
	void fillSessionExpiresHeaders(belle_sip_request_t *invite, belle_sip_header_session_expires_refresher_t refresher);
	void fillSessionExpiresHeaders(belle_sip_request_t *invite,
	                               belle_sip_header_session_expires_refresher_t refresher,
	                               int delta);
	void fillSessionExpiresMinSEHeader(belle_sip_request_t *invite);
	void cancellingInvite(const SalErrorInfo *info);
	int referTo(belle_sip_header_refer_to_t *referToHeader, belle_sip_header_referred_by_t *referredByHeader);
	int sendNotifyForRefer(int code,
	                       const std::string &reason,
	                       const std::string &subscription_state = BELLE_SIP_SUBSCRIPTION_STATE_ACTIVE,
	                       const std::string &subscription_reason = "");
	void notifyLastResponse(SalCallOp *newCallOp);
	void processRefer(const belle_sip_request_event_t *event, belle_sip_server_transaction_t *serverTransaction);
	void processNotify(const belle_sip_request_event_t *event, belle_sip_server_transaction_t *serverTransaction);
	bool checkForOrphanDialogOn2xx(belle_sip_dialog_t *dialog);

	static std::string setAddrTo0000(const std::string &value);
	static bool isMediaDescriptionAcceptable(std::shared_ptr<SalMediaDescription> &md);
	static bool isAPendingIncomingInviteTransaction(belle_sip_transaction_t *transaction);
	static void setCallAsReleased(SalCallOp *op);
	static void unsupportedMethod(belle_sip_server_transaction_t *serverTransaction, belle_sip_request_t *request);
	static belle_sip_header_allow_t *createAllow(bool enableUpdate);
	static std::vector<uint8_t> marshalMediaDescription(belle_sdp_session_description_t *sessionDesc,
	                                                    belle_sip_error_code &error);

	// belle_sip_message handlers
	static int setSdp(belle_sip_message_t *message, belle_sdp_session_description_t *sessionDesc);
	static int setSdpFromDesc(belle_sip_message_t *message, const std::shared_ptr<SalMediaDescription> &desc);
	static void processIoErrorCb(void *userCtx, const belle_sip_io_error_event_t *event);
	static Content extractBody(belle_sip_message_t *message);

	// Callbacks
	static int vfuRetryCb(void *userCtx, unsigned int events);
	static void processResponseCb(void *userCtx, const belle_sip_response_event_t *event);
	static void processTimeoutCb(void *userCtx, const belle_sip_timeout_event_t *event);
	static void processTransactionTerminatedCb(void *userCtx, const belle_sip_transaction_terminated_event_t *event);
	static void processRequestEventCb(void *userCtx, const belle_sip_request_event_t *event);
	static void processDialogTerminatedCb(void *userCtx, const belle_sip_dialog_terminated_event_t *event);

	// Private constants
	static const int MIN_SE = 1800; // Min-Session-Expires, in seconds

	// Attributes
	std::shared_ptr<SalMediaDescription> mLocalMedia = nullptr;
	std::shared_ptr<SalMediaDescription> mRemoteMedia = nullptr;
	std::list<Content> mLocalBodies;
	std::list<Content> mRemoteBodies;
	bool capabilityNegotiation = false;
	bool mNotifyAllRingings = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_CALL_OP_H_
