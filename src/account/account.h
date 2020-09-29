/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _L_ACCOUNT_H_
#define _L_ACCOUNT_H_

#include <memory>

#include <belle-sip/object++.hh>

#include "account-params.h"
#include "c-wrapper/internal/c-sal.h"
#include "linphone/api/c-types.h"
#include "sal/register-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

typedef enum _LinphoneAccountAddressComparisonResult {
	LinphoneAccountAddressDifferent,
	LinphoneAccountAddressEqual,
	LinphoneAccountAddressWeakEqual
} LinphoneAccountAddressComparisonResult;

class Account : public bellesip::HybridObject<LinphoneAccount, Account> {
public:
	Account (LinphoneCore *lc, std::shared_ptr<AccountParams> params);
	Account (LinphoneCore *lc, std::shared_ptr<AccountParams> params, LinphoneProxyConfig *config);
	Account (const Account &other);
	~Account ();

	Account* clone () const override;

	// Account params configuration
	LinphoneStatus setAccountParams (std::shared_ptr<AccountParams> params);
	std::shared_ptr<const AccountParams> getAccountParams () const;

	// Setters
	void setAuthFailure (int authFailure);
	void setRegisterChanged (bool registerChanged);
	void setSendPublish (bool sendPublish);
	void setNeedToRegister (bool needToRegister);
	void setDeletionDate (time_t deletionDate);
	void setSipEtag (const std::string& sipEtag);
	void setUserData (void *userData);
	void setCore (LinphoneCore *lc);
	void setErrorInfo (LinphoneErrorInfo *errorInfo);
	void setContactAddress (LinphoneAddress *contact);
	void setContactAddressWithoutParams (LinphoneAddress *contact);
	void setPendingContactAddress (LinphoneAddress *contact);
	void setServiceRouteAddress (LinphoneAddress *serviceRoute);
	void setState (LinphoneRegistrationState state, const std::string& message);
	void setOp (SalRegisterOp *op);
	void setCustomheader (const std::string& headerName, const std::string& headerValue);
	void setPresencePublishEvent (LinphoneEvent *presencePublishEvent);
	void setDependency (std::shared_ptr<Account> dependency);

	// Getters
	int getAuthFailure () const;
	bool getRegisterChanged () const;
	time_t getDeletionDate () const;
	const std::string& getSipEtag () const;
	void* getUserData () const;
	LinphoneCore* getCore () const;
	LinphoneErrorInfo* getErrorInfo ();
	LinphoneAddress* getContactAddress () const;
	LinphoneAddress* getContactAddressWithoutParams () const;
	LinphoneAddress* getPendingContactAddress () const;
	LinphoneAddress* getServiceRouteAddress ();
	LinphoneRegistrationState getState () const;
	SalRegisterOp* getOp() const;
	const char* getCustomHeader (const std::string& headerName) const;
	LinphoneEvent* getPresencePublishEvent () const;
	std::shared_ptr<Account> getDependency ();

	// Other
	void refreshRegister ();
	void pauseRegister ();
	void stopRefreshing ();
	void registerAccount ();
	void unregister ();
	LinphoneReason getError ();
	LinphoneTransportType getTransport ();
	bool isAvpfEnabled () const;
	const LinphoneAuthInfo* findAuthInfo () const;
	int getUnreadChatMessageCount () const;
	void writeToConfigFile (int index);
	void update ();
	void apply (LinphoneCore *lc);
	LinphoneEvent *createPublish (const char *event, int expires);
	int sendPublish (LinphonePresenceModel *presence);
	void unpublish ();
	void notifyPublishStateChanged (LinphonePublishState state);
	bool check ();
	void releaseOps ();

	// Callbacks
	void addCallbacks (LinphoneAccountCbs *callbacks);
	void removeCallbacks (LinphoneAccountCbs *callbacks);
	void setCurrentCallbacks (LinphoneAccountCbs *callbacks);
	LinphoneAccountCbs* getCurrentCallbacks () const;

	const bctbx_list_t* getCallbacksList () const;

	// Listener
	void onPushNotificationAllowedChanged (bool allow);
	void onInternationalPrefixChanged ();
	void onConferenceFactoryUriChanged (const std::string &conferenceFactoryUri);
	void onNatPolicyChanged (LinphoneNatPolicy *policy);

	// To be removed when not using proxy config anymore
	LinphoneProxyConfig *getConfig () const;
	void setConfig (LinphoneProxyConfig *config);
	LinphoneAccountAddressComparisonResult isServerConfigChanged ();

private:
	bool computePublishParamsHash();
	LinphoneAccountAddressComparisonResult isServerConfigChanged (std::shared_ptr<AccountParams> oldParams, std::shared_ptr<AccountParams> newParams);
	bool canRegister ();
	LinphoneAddress *guessContactForRegister ();
	void updateDependentAccount(LinphoneRegistrationState state, const std::string &message);
	int done ();
	void resolveDependencies ();

	std::shared_ptr<AccountParams> mParams;

	int mAuthFailure;

	bool mNeedToRegister = false;
	bool mRegisterChanged = false;
	bool mSendPublish = false;

	time_t mDeletionDate;

	std::string mSipEtag;

	void *mUserData;

	LinphoneCore *mCore = nullptr;

	LinphoneErrorInfo *mErrorInfo = nullptr;

	LinphoneAddress *mContactAddress = nullptr;
	LinphoneAddress *mContactAddressWithoutParams = nullptr;
	LinphoneAddress *mPendingContactAddress = nullptr;
	LinphoneAddress *mServiceRouteAddress = nullptr;

	LinphoneRegistrationState mState = LinphoneRegistrationNone;

	SalRegisterOp *mOp = nullptr;
	SalCustomHeader *mSentHeaders = nullptr;

	LinphoneEvent *mPresencePublishEvent = nullptr;

	std::shared_ptr<Account> mDependency = nullptr;

	bctbx_list_t *mCallbacksList = nullptr;
	LinphoneAccountCbs *mCurrentCallbacks = nullptr;

	unsigned long long mPreviousPublishParamsHash[2] = {0};
	std::shared_ptr<AccountParams> mOldParams;

	// This is a back pointer intended to keep both LinphoneProxyConfig and Account
	// api to be usable at the same time. This should be removed as soon as 
	// proxy configs can be replaced.
	LinphoneProxyConfig *mConfig = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_H_
