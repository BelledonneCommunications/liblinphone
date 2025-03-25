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

#ifndef _L_ACCOUNT_H_
#define _L_ACCOUNT_H_

#include <memory>

#include "account-params.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-sal.h"
#include "call/call-log.h"
#include "conference/conference-info.h"
#include "http/http-client.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"
#include "sal/register-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

typedef enum _LinphoneAccountAddressComparisonResult {
	LinphoneAccountAddressDifferent,
	LinphoneAccountAddressEqual,
	LinphoneAccountAddressWeakEqual
} LinphoneAccountAddressComparisonResult;

enum class LimeUserAccountStatus {
	LimeUserAccountNone,
	LimeUserAccountNeedCreation,
	LimeUserAccountIsCreating,
	LimeUserAccountCreated,
	LimeUserAccountCreationSkiped // To send the register even if Lime user creation failed
};

class AccountCbs;
class Address;
class Event;
class AbstractChatRoom;
class EventPublish;
class HttpResponse;

class LINPHONE_PUBLIC Account : public bellesip::HybridObject<LinphoneAccount, Account>,
                                public UserDataAccessor,
                                public CallbacksHolder<AccountCbs>,
                                public CoreAccessor {
public:
	Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params);
	Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params, LinphoneProxyConfig *config);
	Account(const Account &other) = delete;
	virtual ~Account();

	Account *clone() const override;

	// Account params configuration
	LinphoneStatus setAccountParams(std::shared_ptr<AccountParams> params);
	std::shared_ptr<const AccountParams> getAccountParams() const;

	// Setters
	void setAuthFailure(int authFailure);
	void setRegisterChanged(bool registerChanged);
	void setSendPublish(bool sendPublish);
	void setNeedToRegister(bool needToRegister);
	void triggerDeletion();
	void cancelDeletion();
	bool deletionPending() const;
	void setSipEtag(const std::string &sipEtag);
	void setErrorInfo(LinphoneErrorInfo *errorInfo);
	void setContactAddress(const std::shared_ptr<const Address> &contact);
	void setContactAddressWithoutParams(const std::shared_ptr<const Address> &contact);
	void setPendingContactAddress(std::shared_ptr<Address> contact);
	void setServiceRouteAddress(std::shared_ptr<Address> serviceRoute);
	void setState(LinphoneRegistrationState state, const std::string &message);
	void setOp(SalRegisterOp *op);
	void setCustomheader(const std::string &headerName, const std::string &headerValue);
	void setPresencePublishEvent(const std::shared_ptr<EventPublish> &presencePublishEvent);
	void setDependency(std::shared_ptr<Account> dependency);
	void setDependee(std::shared_ptr<Account> dependency);
	void setLimeUserAccountStatus(LimeUserAccountStatus status);

	// Getters
	int getAuthFailure() const;
	bool getRegisterChanged() const;
	const std::string &getSipEtag() const;
	const LinphoneErrorInfo *getErrorInfo();
	const std::shared_ptr<Address> &getContactAddress() const;
	const std::shared_ptr<Address> &getContactAddressWithoutParams() const;
	const std::shared_ptr<Address> &getPendingContactAddress() const;
	const std::shared_ptr<Address> getServiceRouteAddress() const;
	LinphoneRegistrationState getState() const;
	LinphoneRegistrationState getPreviousState() const;
	SalRegisterOp *getOp() const;
	const char *getCustomHeader(const std::string &headerName) const;
	std::shared_ptr<EventPublish> getPresencePublishEvent() const;
	std::shared_ptr<Account> getDependency();
	std::shared_ptr<Account> getDependee();
	LimeUserAccountStatus getLimeUserAccountStatus() const;

	int getUnreadChatMessageCount() const;
	const std::list<std::shared_ptr<AbstractChatRoom>> &getChatRooms() const;
	const bctbx_list_t *getChatRoomsCList() const;
	std::list<std::shared_ptr<AbstractChatRoom>> filterChatRooms(const std::string &filter) const;

	int getMissedCallsCount() const;
	std::list<std::shared_ptr<CallLog>> getCallLogs() const;
	std::list<std::shared_ptr<CallLog>> getCallLogsForAddress(const std::shared_ptr<const Address> &) const;
	std::list<std::shared_ptr<ConferenceInfo>>
	getConferenceInfos(const std::list<LinphoneStreamType> capabilities = {}) const;
	void addConferenceInfo(const std::shared_ptr<ConferenceInfo> &info);
	void ccmpConferenceInformationResponseReceived();
	void ccmpConferenceInformationRequestSent();

	// Other
	void resetMissedCallsCount();
	void setMissedCallsCount(int count);
	void deleteCallLogs() const;
	bool isUnregistering() const {
		return mIsUnregistering;
	}
	bool check();
	bool isAvpfEnabled() const;
	void setPresenceModel(LinphonePresenceModel *presence);
	int sendPublish();
	void apply(LinphoneCore *lc);
	void notifyPublishStateChanged(LinphonePublishState state);
	void pauseRegister();
	void refreshRegister();
	void registerAccount();
	void releaseOps();
	void stopRefreshing();
	void unpublish();
	void unregister();
	void update();
	void addCustomParam(const std::string &key, const std::string &value);
	const std::string &getCustomParam(const std::string &key) const;
	void writeToConfigFile(int index);
	const LinphoneAuthInfo *findAuthInfo() const;
	std::shared_ptr<EventPublish> createPublish(const std::string &event, int expires);
	LinphoneReason getError();
	LinphoneTransportType getTransport();
	std::shared_ptr<Event> getMwiEvent() const;
	void subscribeToMessageWaitingIndication();
	void unsubscribeFromMessageWaitingIndication();

	// Utils
	static LinphoneAccountAddressComparisonResult compareLinphoneAddresses(const std::shared_ptr<const Address> &a,
	                                                                       const std::shared_ptr<const Address> &b);

	static void writeAllToConfigFile(const std::shared_ptr<Core> core);
	static void writeToConfigFile(LpConfig *config, const std::shared_ptr<Account> &account, int index);

	// To be removed when not using proxy config anymore
	LinphoneProxyConfig *getConfig() const;
	void setConfig(LinphoneProxyConfig *config, bool takeProxyConfigRef = true);
	LinphoneAccountAddressComparisonResult isServerConfigChanged();

	void updateConferenceInfoListWithCcmp() const;

	void handleCCMPResponseConferenceList(const HttpResponse &response);
	void handleCCMPResponseConferenceInformation(const HttpResponse &response);
	// CCMP request callback (conference list)
	static void handleResponseConferenceList(void *ctx, const HttpResponse &event);
	static void handleTimeoutConferenceList(void *ctx, const HttpResponse &event);
	static void handleIoErrorConferenceList(void *ctx, const HttpResponse &event);
	static void handleResponseConferenceInformation(void *ctx, const HttpResponse &event);
	static void handleTimeoutConferenceInformation(void *ctx, const HttpResponse &event);
	static void handleIoErrorConferenceInformation(void *ctx, const HttpResponse &event);

private:
	LinphoneCore *getCCore() const;
	bool canRegister();
	bool computePublishParamsHash();
	int done();
	void applyParamsChanges();
	void resolveDependencies();
	void updateDependentAccount(LinphoneRegistrationState state, const std::string &message);
	LinphoneAccountAddressComparisonResult isServerConfigChanged(std::shared_ptr<AccountParams> oldParams,
	                                                             std::shared_ptr<AccountParams> newParams);
	std::shared_ptr<Address> guessContactForRegister();

	void onInternationalPrefixChanged();
	void onConferenceFactoryAddressChanged(const std::shared_ptr<Address> &conferenceFactoryAddress);
	void
	onAudioVideoConferenceFactoryAddressChanged(const std::shared_ptr<Address> &audioVideoConferenceFactoryAddress);
	void onNatPolicyChanged(const std::shared_ptr<NatPolicy> &policy);
	void onLimeServerUrlChanged(const std::string &limeServerUrl);
	void onLimeAlgoChanged(const std::string &limeAlgo);
	void onMwiServerAddressChanged();
	bool customContactChanged();
	std::list<SalAddress *> getOtherContacts();
	void unsubscribeFromChatRooms();
	void updateChatRoomList() const;

	void triggerUpdate();
	void handleDeletion();

	std::shared_ptr<AccountParams> mParams;

	int mAuthFailure;

	LimeUserAccountStatus mLimeUserAccountStatus = LimeUserAccountStatus::LimeUserAccountNone;
	bool mNeedToRegister = false;
	bool mRegisterChanged = false;
	bool mSendPublish = false;
	bool mIsUnregistering = false;
	bool hasProxyConfigRef = false;

	belle_sip_source_t *mDeletionTimer = nullptr;

	std::string mSipEtag;

	LinphoneErrorInfo *mErrorInfo = nullptr;

	std::shared_ptr<Address> mContactAddress = nullptr;
	std::shared_ptr<Address> mContactAddressWithoutParams = nullptr;
	std::shared_ptr<Address> mPendingContactAddress = nullptr;
	mutable std::shared_ptr<Address> mServiceRouteAddress = nullptr;

	LinphoneRegistrationState mState = LinphoneRegistrationNone;
	LinphoneRegistrationState mPreviousState = LinphoneRegistrationNone;

	SalRegisterOp *mOp = nullptr;
	SalCustomHeader *mSentHeaders = nullptr;

	std::shared_ptr<EventPublish> mPresencePublishEvent = nullptr;
	LinphonePresenceModel *mPresenceModel = nullptr;

	std::shared_ptr<Account> mDependency = nullptr;
	std::weak_ptr<Account> mDependee;

	unsigned long long mPreviousPublishParamsHash[2] = {0};
	std::shared_ptr<AccountParams> mOldParams;

	mutable std::list<std::shared_ptr<ConferenceInfo>> mConferenceInfos;
	mutable ListHolder<AbstractChatRoom> mChatRoomList;

	// This is a back pointer intended to keep both LinphoneProxyConfig and Account
	// api to be usable at the same time. This should be removed as soon as
	// proxy configs can be replaced.
	LinphoneProxyConfig *mConfig = nullptr;

	int mMissedCalls = 0;
	unsigned int mCcmpConferenceInformationRequestsCounter = 0;

	std::shared_ptr<Event> mMwiEvent;
};

class AccountCbs : public bellesip::HybridObject<LinphoneAccountCbs, AccountCbs>, public Callbacks {
public:
	LinphoneAccountCbsRegistrationStateChangedCb getRegistrationStateChanged() const;
	void setRegistrationStateChanged(LinphoneAccountCbsRegistrationStateChangedCb cb);
	LinphoneAccountCbsMessageWaitingIndicationChangedCb getMessageWaitingIndicationChanged() const;
	void setMessageWaitingIndicationChanged(LinphoneAccountCbsMessageWaitingIndicationChangedCb cb);
	LinphoneAccountCbsConferenceInformationUpdatedCb getConferenceInformationUpdated() const;
	void setConferenceInformationUpdated(LinphoneAccountCbsConferenceInformationUpdatedCb cb);

private:
	LinphoneAccountCbsRegistrationStateChangedCb mRegistrationStateChangedCb = nullptr;
	LinphoneAccountCbsMessageWaitingIndicationChangedCb mMessageWaitingIndicationChangedCb = nullptr;
	LinphoneAccountCbsConferenceInformationUpdatedCb mConferenceInformationUpdatedCb = nullptr;
};

class AccountLogContextualizer : public CoreLogContextualizer {
public:
	AccountLogContextualizer(const LinphoneAccount *account)
	    : CoreLogContextualizer(account ? Account::toCpp(account) : nullptr) {
	}
};

inline std::ostream &operator<<(std::ostream &ostr, const Account &account) {
	const auto &params = account.getAccountParams();
	const auto identity =
	    (params && params->getIdentityAddress()) ? params->getIdentityAddress()->toString() : std::string("sip:");
	ostr << "Account [" << (void *)&account << "]  (" << identity << ")";
	return ostr;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_H_
