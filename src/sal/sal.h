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

#ifndef _L_SAL_H_
#define _L_SAL_H_

#include <list>
#include <vector>

#include "linphone/types.h"
#include "linphone/utils/general.h"
#include "sal/offeranswer.h"
#include "sal/sal_stream_configuration.h"

#include "c-wrapper/internal/c-sal.h"
#include "logger/logger.h"

LINPHONE_BEGIN_NAMESPACE

class SalOp;
class SalCallOp;
class SalMessageOp;
class SalSubscribeOp;
class SalPublishOp;
class SalPresenceOp;
class SalReferOp;

class SalResolverContext {
public:
	using Callback = std::function<void(belle_sip_resolver_results_t *)>;
	SalResolverContext() = default;
	SalResolverContext(belle_sip_resolver_context_t *ctx, Callback *cb);
	// Evalulates whether the resolver context is not empty.
	// Not empty means that resolution may be pending (hence cancellable) or already completed.
	operator bool() const {
		return mCtx != nullptr;
	}
	SalResolverContext(const SalResolverContext &other) = delete;
	SalResolverContext(SalResolverContext &&other) = default;
	SalResolverContext &operator=(const SalResolverContext &other) = delete;
	SalResolverContext &operator=(SalResolverContext &&other);

	void cancel();
	/* Reset the resolver context. If a resolution is still pending, it is not cancelled.*/
	void reset();
	~SalResolverContext();

private:
	belle_sip_resolver_context_t *mCtx = nullptr;
	Callback *mCallback = nullptr;
};

class Sal {
public:
	using OnCallReceivedCb = void (*)(SalCallOp *op);
	using OnCallRingingCb = void (*)(SalOp *op);
	using OnCallAcceptedCb = void (*)(SalOp *op);
	using OnCallAckReceivedCb = void (*)(SalOp *op, SalCustomHeader *ack);
	using OnCallAckBeingSentCb = void (*)(SalOp *op, SalCustomHeader *ack);
	using OnCallUpdatingCb = void (*)(SalOp *op, bool_t isUpdate); // Called when a reINVITE/UPDATE is received
	using OnCallRefreshedCb = void (*)(SalOp *op);
	using OnCallRefreshingCb = void (*)(SalOp *op);
	using OnCallTerminatedCb = void (*)(SalOp *op, const char *from);
	using OnCallFailureCb = void (*)(SalOp *op);
	using OnCallReleasedCb = void (*)(SalOp *op);
	using OnCallCancelDoneCb = void (*)(SalOp *op);
	using OnAuthRequestedLegacyCb = void (*)(SalOp *op, const char *realm, const char *username);
	using OnAuthRequestedCb = bool_t (*)(Sal *sal, SalAuthInfo *info);
	using OnAuthFailureCb = void (*)(SalOp *op, SalAuthInfo *info);
	using OnRegisterSuccessCb = void (*)(SalOp *op, bool_t registered);
	using OnRegisterFailureCb = void (*)(SalOp *op);
	using OnVfuRequestCb = void (*)(SalOp *op);
	using OnDtmfReceivedCb = void (*)(SalOp *op, char dtmf);
	using OnCallReferCb = void (*)(SalOp *op,
	                               const SalAddress *referTo,
	                               const SalCustomHeader *custom_headers,
	                               const SalBodyHandler *body);
	using OnReferCb = void (*)(SalOp *op,
	                           const SalAddress *referTo,
	                           const SalCustomHeader *custom_headers,
	                           const SalBodyHandler *body);
	using OnMessageReceivedCb = void (*)(SalOp *op, const SalMessage *msg);
	using OnMessageDeliveryUpdateCb = void (*)(SalOp *op, SalMessageDeliveryStatus status);
	using OnNotifyReferCb = void (*)(SalOp *op, SalReferStatus status);
	using OnSubscribeResponseCb = void (*)(SalOp *op, SalSubscribeStatus status, int willRetry);
	using OnNotifyCb = void (*)(SalSubscribeOp *op, SalSubscribeStatus status, const char *event, SalBodyHandler *body);
	using OnSubscribeReceivedCb = void (*)(SalSubscribeOp *op, const char *event, const SalBodyHandler *body);
	using OnIncomingSubscribeClosedCb = void (*)(SalOp *op);
	using OnParsePresenceRequestedCb = void (*)(
	    SalOp *op, const char *contentType, const char *contentSubtype, const char *content, SalPresenceModel **result);
	using OnConvertPresenceToXMLRequestedCb = void (*)(SalOp *op,
	                                                   SalPresenceModel *presence,
	                                                   const char *contact,
	                                                   char **content);
	using OnNotifyPresenceCb = void (*)(SalOp *op, SalSubscribeStatus ss, SalPresenceModel *model, const char *msg);
	using OnRedirectCb = int (*)(SalOp *op);
	using OnSubscribePresenceReceivedCb = void (*)(SalPresenceOp *op, const char *from);
	using OnSubscribePresenceClosedCb = void (*)(SalPresenceOp *op, const char *from);
	using OnPingReplyCb = void (*)(SalOp *op);
	using OnInfoReceivedCb = void (*)(SalOp *op, SalBodyHandler *body);
	using OnPublishReceivedCb = void (*)(SalPublishOp *op, const char *event, const SalBodyHandler *body);
	using OnPublishResponseCb = void (*)(SalOp *op);
	using OnIncomingPublishClosedCb = void (*)(SalOp *op);
	using OnNotifyResponseCb = void (*)(SalOp *op);
	using OnExpireCb = void (*)(SalOp *op);

	struct Callbacks {
		OnCallReceivedCb call_received;
		OnCallReceivedCb call_rejected;
		OnCallRingingCb call_ringing;
		OnCallAcceptedCb call_accepted;
		OnCallAckReceivedCb call_ack_received;
		OnCallAckBeingSentCb call_ack_being_sent;
		OnCallUpdatingCb call_updating;
		OnCallRefreshedCb call_refreshed;
		OnCallRefreshingCb call_refreshing;
		OnCallTerminatedCb call_terminated;
		OnCallFailureCb call_failure;
		OnCallReleasedCb call_released;
		OnCallCancelDoneCb call_cancel_done;
		OnCallReferCb call_refer_received;
		OnAuthFailureCb auth_failure;
		OnRegisterSuccessCb register_success;
		OnRegisterFailureCb register_failure;
		OnVfuRequestCb vfu_request;
		OnDtmfReceivedCb dtmf_received;

		OnMessageReceivedCb message_received;
		OnMessageDeliveryUpdateCb message_delivery_update;
		OnNotifyReferCb notify_refer;
		OnSubscribeReceivedCb subscribe_received;
		OnIncomingSubscribeClosedCb incoming_subscribe_closed;
		OnSubscribeResponseCb subscribe_response;
		OnNotifyCb notify;
		OnSubscribePresenceReceivedCb subscribe_presence_received;
		OnSubscribePresenceClosedCb subscribe_presence_closed;
		OnParsePresenceRequestedCb parse_presence_requested;
		OnConvertPresenceToXMLRequestedCb convert_presence_to_xml_requested;
		OnNotifyPresenceCb notify_presence;
		OnPingReplyCb ping_reply;
		OnAuthRequestedCb auth_requested;
		OnInfoReceivedCb info_received;
		OnPublishReceivedCb publish_received;
		OnPublishResponseCb on_publish_response;
		OnIncomingPublishClosedCb incoming_publish_closed;
		OnExpireCb on_expire;
		OnNotifyResponseCb on_notify_response;
		OnReferCb refer_received; // For out of dialog refer
		OnRedirectCb process_redirect;
	};

	Sal(MSFactory *factory);
	~Sal();
	void setFactory(MSFactory *value);

	void setUserPointer(void *value) {
		mUserPointer = value;
	}
	void *getUserPointer() const {
		return mUserPointer;
	}

	void setCallbacks(const Callbacks *cbs);

	void *getStackImpl() const {
		return mStack;
	}

	int iterate() {
		belle_sip_stack_sleep(mStack, 0);
		return 0;
	}

	void setSendError(int value) {
		belle_sip_stack_set_send_error(mStack, value);
	}
	void setRecvError(int value) {
		belle_sip_provider_set_recv_error(mProvider, value);
	}
	void setClientBindPort(int port) {
		belle_sip_stack_set_client_bind_port(mStack, port);
	}

	// ---------------------------------------------------------------------------
	// SIP parameters
	// ---------------------------------------------------------------------------
	void setSupportedTags(const std::string &tags);
	const std::string &getSupportedTags() const;
	void addSupportedTag(const std::string &tag);
	void removeSupportedTag(const std::string &tag);

	static void setWellKnownPort(int value);
	static void setTLSWellKnownPort(int value);

	void setUserAgent(const std::string &value);
	const std::string &getUserAgent() const;
	void appendStackStringToUserAgent();

	bool isContentEncodingAvailable(const std::string &contentEncoding) const;
	bool isContentTypeSupported(const std::string &contentType) const;
	void addContentTypeSupport(const std::string &contentType);
	void removeContentTypeSupport(const std::string &contentType);

	void setDefaultSdpHandling(SalOpSDPHandling sdpHandlingMethod);

	void setUuid(const std::string &value) {
		mUuid = value;
	}
	const std::string &getUuid() const {
		return mUuid;
	}
	std::string createUuid();
	static std::string generateUuid();

	void enableNatHelper(bool value);
	bool natHelperEnabled() const {
		return mNatHelperEnabled;
	}

	bool pendingTransactionCheckingEnabled() const {
		return mPendingTransactionChecking;
	}
	void enablePendingTransactionChecking(bool value) {
		mPendingTransactionChecking = value;
	}

	void setRefresherRetryAfter(int value) {
		mRefresherRetryAfter = value;
	}
	int getRefresherRetryAfter() const {
		return mRefresherRetryAfter;
	}

	void enableSipUpdateMethod(bool value) {
		mEnableSipUpdate = value;
	}

	// RFC 4028
	void setSessionTimersEnabled(bool value) {
		mSessionExpiresEnabled = value;
	}
	void setSessionTimersValue(int expires) {
		mSessionExpiresValue = expires;
	}
	int getSessionTimersExpire() {
		return mSessionExpiresValue;
	}
	void setSessionTimersRefresher(LinphoneSessionExpiresRefresher refresher) {
		mSessionExpiresRefresher = static_cast<belle_sip_header_session_expires_refresher_t>(refresher);
	}
	void setSessionTimersMin(int min) {
		mSessionExpiresMin = min;
	}

	void useDates(bool value) {
		mUseDates = value;
	}
	void useRport(bool value);
	void enableAutoContacts(bool value) {
		mAutoContacts = value;
	}
	void enableTestFeatures(bool value) {
		mEnableTestFeatures = value;
	}
	bool isEnabledTestFeatures() {
		return mEnableTestFeatures;
	}
	void useNoInitialRoute(bool value) {
		mNoInitialRoute = value;
	}
	void enableUnconditionalAnswer(int value) {
		belle_sip_provider_enable_unconditional_answer(mProvider, value);
	}
	void setUnconditionalAnswer(unsigned short value) {
		belle_sip_provider_set_unconditional_answer(mProvider, value);
	}
	void enableReconnectToPrimaryAsap(bool value) {
		belle_sip_stack_enable_reconnect_to_primary_asap(mStack, value);
	}

	const std::list<SalOp *> &getPendingAuths() const {
		return mPendingAuths;
	}

	void setContactLinphoneSpecs(const std::string &value) {
		mLinphoneSpecs = value;
	}

	// ---------------------------------------------------------------------------
	// Network parameters
	// ---------------------------------------------------------------------------
	int setListenPort(const std::string &addr, int port, SalTransport tr, bool isTunneled);
	int getListeningPort(SalTransport tr);
	bool isTransportAvailable(SalTransport tr);

	void setTransportTimeout(int value) {
		belle_sip_stack_set_transport_timeout(mStack, value);
	}
	int getTransportTimeout() const {
		return belle_sip_stack_get_transport_timeout(mStack);
	}
	void setPongTimeout(int value) {
		belle_sip_stack_set_pong_timeout(mStack, value);
	}
	void enablePingPongVerification(bool value) {
		belle_sip_stack_enable_ping_pong_verification(mStack, value ? TRUE : FALSE);
	}

	void setUnreliableConnectionTimeout(int value) {
		belle_sip_stack_set_unreliable_connection_timeout(mStack, value);
	}
	int getUnreliableConnectionTimeout() const {
		return belle_sip_stack_get_unreliable_connection_timeout(mStack);
	}

	void setKeepAlivePeriod(unsigned int value);
	unsigned int getKeepAlivePeriod() const {
		return mKeepAlive;
	}
	void useTcpTlsKeepAlive(bool value) {
		mUseTcpTlsKeepAlive = value;
	}
	void forceNameAddr(bool value);
	void sendKeepAlive();

	void setDscp(int dscp) {
		belle_sip_stack_set_default_dscp(mStack, dscp);
	}

	int setTunnel(void *tunnelClient);

	void setHttpProxyHost(const std::string &value);
	const std::string &getHttpProxyHost() const;

	void setHttpProxyPort(int value) {
		belle_sip_stack_set_http_proxy_port(mStack, value);
	}
	int getHttpProxyPort() const {
		return belle_sip_stack_get_http_proxy_port(mStack);
	}

	void unlistenPorts();
	void resetTransports();
	void cleanUnreliableConnections();

	// ---------------------------------------------------------------------------
	// TLS parameters
	// ---------------------------------------------------------------------------
	void setSslConfig(void *sslConfig);
	void setRootCa(const std::string &value);
	void setRootCaData(const std::string &value);
	const std::string &getRootCa() const {
		return mRootCa;
	}

	void verifyServerCertificates(bool value);
	void verifyServerCn(bool value);
	void setTlsPostcheckCallback(int (*cb)(void *, const bctbx_x509_certificate_t *), void *data);

	// ---------------------------------------------------------------------------
	// DNS resolution
	// ---------------------------------------------------------------------------
	void setDnsTimeout(int value) {
		belle_sip_stack_set_dns_timeout(mStack, value);
	}
	int getDnsTimeout() const {
		return belle_sip_stack_get_dns_timeout(mStack);
	}

	void setDnsServers(const bctbx_list_t *servers);

	void enableDnsSearch(bool value) {
		belle_sip_stack_enable_dns_search(mStack, (unsigned char)value);
	}
	bool dnsSearchEnabled() const {
		return !!belle_sip_stack_dns_search_enabled(mStack);
	}

	void enableDnsSrv(bool value) {
		belle_sip_stack_enable_dns_srv(mStack, (unsigned char)value);
	}
	bool dnsSrvEnabled() const {
		return !!belle_sip_stack_dns_srv_enabled(mStack);
	}

	void setIpv6Preference(bool value) {
		belle_sip_stack_set_ip_version_preference(mStack, value ? AF_INET6 : AF_INET);
	}

	void setDnsUserHostsFile(const std::string &value);
	const std::string &getDnsUserHostsFile() const;

	/* to deprecate, use prototype with lambda function, more convenient*/
	belle_sip_resolver_context_t *
	resolveA(const std::string &name, int port, int family, belle_sip_resolver_callback_t cb, void *data);

	belle_sip_resolver_context_t *resolve(const std::string &service,
	                                      const std::string &transport,
	                                      const std::string &name,
	                                      int port,
	                                      int family,
	                                      belle_sip_resolver_callback_t cb,
	                                      void *data);
	SalResolverContext
	resolveA(const std::string &name, int port, int family, const SalResolverContext::Callback &onResults);

	SalResolverContext resolve(const std::string &service,
	                           const std::string &transport,
	                           const std::string &name,
	                           int port,
	                           int family,
	                           const SalResolverContext::Callback &onResults);
	// ---------------------------------------------------------------------------
	// Timers
	// ---------------------------------------------------------------------------
	belle_sip_source_t *
	createTimer(const std::function<bool()> &something, unsigned int milliseconds, const std::string &name);
	belle_sip_source_t *
	createTimer(belle_sip_source_func_t func, void *data, unsigned int timeoutValueMs, const std::string &timerName);
	void cancelTimer(belle_sip_source_t *timer);

	// Media
	void disableMedia(bool enable);
	bool mediaDisabled() const;

	// utils
	static int findCryptoIndexFromAlgo(const std::vector<SalSrtpCryptoAlgo> &crypto, const MSCryptoSuite suite);
	OfferAnswerEngine &getOfferAnswerEngine() {
		return mOfferAnswerEngine;
	}

	void setRefreshWindow(const int min_value, const int max_value);

private:
	static void onResolverResults(void *lambda, belle_sip_resolver_results_t *results);
	struct SalUuid {
		unsigned int timeLow;
		unsigned short timeMid;
		unsigned short timeHiAndVersion;
		unsigned char clockSeqHiAndReserved;
		unsigned char clockSeqLow;
		unsigned char node[6];
	};

	void setTlsProperties();
	int addListenPort(SalAddress *addr, bool isTunneled);
	static belle_sip_header_t *createSupportedHeader(const std::list<std::string> &tags);
	void makeSupportedHeader();
	void addPendingAuth(SalOp *op);
	void removePendingAuth(SalOp *op);
	belle_sip_response_t *createResponseFromRequest(belle_sip_request_t *req, int code);

	static void unimplementedStub() {
		lWarning() << "Unimplemented SAL callback";
	}
	static void removeListeningPoint(belle_sip_listening_point_t *lp, belle_sip_provider_t *prov) {
		belle_sip_provider_remove_listening_point(prov, lp);
	}

	// Internal callbacks
	static void processDialogTerminatedCb(void *userCtx, const belle_sip_dialog_terminated_event_t *event);
	static void processIoErrorCb(void *userCtx, const belle_sip_io_error_event_t *event);
	static void processRequestEventCb(void *userCtx, const belle_sip_request_event_t *event);
	static void processResponseEventCb(void *userCtx, const belle_sip_response_event_t *event);
	static void processTimeoutCb(void *userCtx, const belle_sip_timeout_event_t *event);
	static void processTransactionTerminatedCb(void *userCtx, const belle_sip_transaction_terminated_event_t *event);
	static void processAuthRequestedCb(void *userCtx, belle_sip_auth_event_t *event);

	MSFactory *mFactory = nullptr;
	Callbacks mCallbacks = {0};
	std::list<SalOp *> mPendingAuths;
	belle_sip_stack_t *mStack = nullptr;
	belle_sip_provider_t *mProvider = nullptr;
	belle_sip_header_user_agent_t *mUserAgentHeader = nullptr;
	belle_sip_listener_t *mListener = nullptr;
	void *mTunnelClient = nullptr;
	void *mUserPointer = nullptr; // User pointer

	// RFC 4028
	bool mSessionExpiresEnabled = false;
	int mSessionExpiresValue =
	    0; // disabled = 0, or not lower than mSessionExpiresMin, https://tools.ietf.org/html/rfc4028#page-16
	int mSessionExpiresMin = 0; // disabled = 0, min 90, max 86400
	belle_sip_header_session_expires_refresher_t mSessionExpiresRefresher =
	    BELLE_SIP_HEADER_SESSION_EXPIRES_UNSPECIFIED; // 0 = auto, 1 = uas, 2 = uac

	unsigned int mKeepAlive = 0;
	std::string mRootCa;
	std::string mRootCaData;
	std::string mUuid;
	int mRefresherRetryAfter = 60000; // Retry after value for refresher
	std::vector<std::string> mSupportedTags;
	belle_sip_header_t *mSupportedHeader = nullptr;
	bool mUseTcpTlsKeepAlive = false;
	bool mNatHelperEnabled = false;
	bool mTlsVerify = true;
	bool mTlsVerifyCn = true;
	bool mUseDates = false;
	bool mAutoContacts = true;
	bool mEnableTestFeatures = false;
	bool mNoInitialRoute = false;
	bool mEnableSipUpdate = true;
	bool mDisableMedia = false;
	SalOpSDPHandling mDefaultSdpHandling = SalOpSDPNormal;
	bool mPendingTransactionChecking = true; // For testing purposes
	void *mSslConfig = nullptr;
	std::vector<std::string> mSupportedContentTypes;
	std::string mLinphoneSpecs;
	belle_tls_crypto_config_postcheck_callback_t mTlsPostcheckCb;
	void *mTlsPostcheckCbData;
	OfferAnswerEngine mOfferAnswerEngine;

	// Cache values
	mutable std::string mDnsUserHostsFile;
	mutable std::string mHttpProxyHost;
	mutable std::string mSupported;
	mutable std::string mUserAgent;

	// Publish
	std::map<std::string, SalOp *> mOpByCallId;

	friend class SalOp;
	friend class SalCallOp;
	friend class SalRegisterOp;
	friend class SalMessageOp;
	friend class SalPresenceOp;
	friend class SalSubscribeOp;
	friend class SalPublishOp;
	friend class SalReferOp;
};

int toSipCode(SalReason reason);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_H_
