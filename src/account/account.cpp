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
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "account.h"

#include <bctoolbox/defs.h>
#include "linphone/api/c-account.h"
#include "linphone/api/c-account-params.h"
#include "linphone/core.h"
#include "private.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Account::Account (LinphoneCore *lc, std::shared_ptr<AccountParams> params) {
	mCore = lc;
	mParams = params;
	applyParamsChanges();
}

Account::Account (LinphoneCore *lc, std::shared_ptr<AccountParams> params, LinphoneProxyConfig *config)
	: Account(lc, params) {
	mConfig = config;
}

Account::Account (const Account &other) : HybridObject(other) {
}

Account::~Account () {
	if (mSentHeaders) sal_custom_header_free(mSentHeaders);
	if (mPendingContactAddress) linphone_address_unref(mPendingContactAddress);
	setDependency(nullptr);
	if (mErrorInfo) linphone_error_info_unref(mErrorInfo);

	if (mServiceRouteAddress) linphone_address_unref(mServiceRouteAddress);
	if (mContactAddress) linphone_address_unref(mContactAddress);
	if (mContactAddressWithoutParams) linphone_address_unref(mContactAddressWithoutParams);

	bctbx_list_for_each(mCallbacksList, (bctbx_list_iterate_func)linphone_account_cbs_unref);

	releaseOps();
}

Account* Account::clone () const {
	return new Account(*this);
}

// -----------------------------------------------------------------------------

static char * appendLinphoneAddress(LinphoneAddress *addr,char *out) {
	char *res = out;
	if (addr) {
		char *tmp;
		tmp = linphone_address_as_string(addr);
		res = ms_strcat_printf(out, "%s",tmp);
		ms_free(tmp);
	}
	return res;
}

static char * appendString(const char * string,char *out) {
	char *res = out;
	if (string) {
		res = ms_strcat_printf(out, "%s",string);
	}
	return res;
}

bool Account::computePublishParamsHash() {
	char *source = NULL;
	char hash[33];
	char saved;
	unsigned long long previous_hash[2];
	bctbx_list_t *routes_iterator = mParams->mRoutes;
	previous_hash[0] = mPreviousPublishParamsHash[0];
	previous_hash[1] = mPreviousPublishParamsHash[1];

	source = ms_strcat_printf(source, "%i", mParams->mPrivacy);
	source = appendLinphoneAddress(mParams->mIdentityAddress, source);
	source = appendString(mParams->mProxy.c_str(), source);
	while (routes_iterator) {
		const char *route = (const char *)bctbx_list_get_data(routes_iterator);
		source = appendString(route, source);
		routes_iterator = bctbx_list_next(routes_iterator);
	}
	source = appendString(mParams->mRealm.c_str(), source);
	source = ms_strcat_printf(source, "%i", mParams->mPublishExpires);
	source = ms_strcat_printf(source, "%i", mParams->mPublishEnabled);
	belle_sip_auth_helper_compute_ha1(source, "dummy", "dummy", hash);
	ms_free(source);
	saved = hash[16];
	hash[16] = '\0';
	mPreviousPublishParamsHash[0] = strtoull(hash, (char **)NULL, 16);
	hash[16] = saved;
	mPreviousPublishParamsHash[1] = strtoull(&hash[16], (char **)NULL, 16);
	return previous_hash[0] != mPreviousPublishParamsHash[0] || previous_hash[1] != mPreviousPublishParamsHash[1];
}

LinphoneAccountAddressComparisonResult Account::compareLinphoneAddresses (const LinphoneAddress *a, const LinphoneAddress *b) {
	if (a == NULL && b == NULL)
		return LinphoneAccountAddressEqual;
	else if (!a || !b)
		return LinphoneAccountAddressDifferent;

	if (linphone_address_equal(a,b))
		return LinphoneAccountAddressEqual;
	if (linphone_address_weak_equal(a,b)) {
		/*also check both transport and uri */
		if (linphone_address_get_secure(a) == linphone_address_get_secure(b) && linphone_address_get_transport(a) == linphone_address_get_transport(b))
			return LinphoneAccountAddressWeakEqual;
		else
			return LinphoneAccountAddressDifferent;
	}
	return LinphoneAccountAddressDifferent; /*either username, domain or port ar not equals*/
}

LinphoneAccountAddressComparisonResult Account::isServerConfigChanged (std::shared_ptr<AccountParams> oldParams, std::shared_ptr<AccountParams> newParams) {
	LinphoneAddress *oldProxy = oldParams != nullptr && !oldParams->mProxy.empty() ? linphone_address_new(oldParams->mProxy.c_str()) : NULL;
	LinphoneAddress *newProxy = !newParams->mProxy.empty() ? linphone_address_new(newParams->mProxy.c_str()) : NULL;
	LinphoneAccountAddressComparisonResult result_identity;
	LinphoneAccountAddressComparisonResult result;

	result = compareLinphoneAddresses(oldParams != nullptr ? oldParams->mIdentityAddress : NULL, newParams->mIdentityAddress);
	if (result == LinphoneAccountAddressDifferent) goto end;
	result_identity = result;

	result = compareLinphoneAddresses(oldProxy, newProxy);

	if (mContactAddress != nullptr) {
		IdentityAddress addr(*L_GET_CPP_PTR_FROM_C_OBJECT(mContactAddress));
		if (!addr.getGruu().empty() && result != LinphoneAccountAddressEqual) {
			// Returning weak equal to be sure no unregister will be done
			result = LinphoneAccountAddressWeakEqual;
			goto end;
		}
	}

	// This is the legacy mode, if there is no gruu and result is different,
	// then an unregister will be triggered.
	if (result == LinphoneAccountAddressDifferent) goto end;
	/** If the proxies are equal use the result of the difference between the identities,
	  * otherwise the result is weak-equal and so weak-equal must be returned even if the
	  * identities were equal.
	  */
	if (result == LinphoneAccountAddressEqual) result = result_identity;

	end:
	if (oldProxy) linphone_address_unref(oldProxy);
	if (newProxy) linphone_address_unref(newProxy);
	lInfo() << "linphoneAccountIsServerConfigChanged : " << result;

	return result;
}

LinphoneStatus Account::setAccountParams (std::shared_ptr<AccountParams> params) {
	mOldParams = mParams ? mParams : nullptr;

	// Equivalent of the old proxy_config_edit
	computePublishParamsHash();

	if (mParams->mPublishEnabled && mPresencePublishEvent){
		linphone_event_pause_publish(mPresencePublishEvent);
	}

	// Replacing the old params by the updated one
	mParams = params;

	// Some changes in AccountParams needs a special treatment in Account
	applyParamsChanges();

	// Equivalent of the old proxy_config_done
	return done();
}

std::shared_ptr<const AccountParams> Account::getAccountParams () const {
	return mParams;
}

void Account::applyParamsChanges () {
	if (mOldParams == nullptr
		|| mOldParams->mPushNotificationAllowed != mParams->mPushNotificationAllowed
		|| mOldParams->mRemotePushNotificationAllowed != mParams->mRemotePushNotificationAllowed
		|| !linphone_push_notification_config_is_equal(mOldParams->mPushNotificationConfig, mParams->mPushNotificationConfig))
		onPushNotificationAllowedChanged(false);

	if (mOldParams == nullptr || mOldParams->mInternationalPrefix != mParams->mInternationalPrefix)
		onInternationalPrefixChanged();

	if (mOldParams == nullptr || mOldParams->mConferenceFactoryUri != mParams->mConferenceFactoryUri)
		onConferenceFactoryUriChanged(mParams->mConferenceFactoryUri);

	if (mOldParams == nullptr || mOldParams->mNatPolicy != mParams->mNatPolicy)
		if (mParams->mNatPolicy != nullptr) onNatPolicyChanged(mParams->mNatPolicy);

	if (mOldParams == nullptr
		|| mOldParams->mRegisterEnabled != mParams->mRegisterEnabled
		|| mOldParams->mExpires != mParams->mExpires
		|| mOldParams->mContactParameters != mParams->mContactParameters
		|| mOldParams->mContactUriParameters != mParams->mContactUriParameters) {
		mRegisterChanged = true;
	}
}

// -----------------------------------------------------------------------------

void Account::setAuthFailure (int authFailure) {
	mAuthFailure = authFailure;
}

void Account::setRegisterChanged (bool registerChanged) {
	mRegisterChanged = registerChanged;
}

void Account::setSendPublish (bool sendPublish) {
	mSendPublish = sendPublish;
}

void Account::setNeedToRegister (bool needToRegister) {
	mNeedToRegister = needToRegister;
}

void Account::setDeletionDate (time_t deletionDate) {
	mDeletionDate = deletionDate;
}

void Account::setSipEtag (const std::string& sipEtag) {
	mSipEtag = sipEtag;
}

void Account::setUserData (void *userData) {
	mUserData = userData;
}

void Account::setCore (LinphoneCore *lc) {
	mCore = lc;
}

void Account::setErrorInfo (LinphoneErrorInfo *errorInfo) {
	mErrorInfo = errorInfo;
}

void Account::setContactAddress (LinphoneAddress *contact) {
	if (mContactAddress) {
		linphone_address_unref(mContactAddress);
		mContactAddress = nullptr;
	}

	if (contact) mContactAddress = linphone_address_clone(contact);

	setContactAddressWithoutParams(contact);
}

void Account::setContactAddressWithoutParams (LinphoneAddress *contact) {
	if (mContactAddressWithoutParams) {
		linphone_address_unref(mContactAddressWithoutParams);
		mContactAddressWithoutParams = nullptr;
	}

	if (contact) {
		mContactAddressWithoutParams = linphone_address_clone(contact);
		linphone_address_clean(mContactAddressWithoutParams);
		linphone_address_set_port(mContactAddressWithoutParams, -1);
		linphone_address_set_domain(mContactAddressWithoutParams, nullptr);
		linphone_address_set_display_name(mContactAddressWithoutParams, nullptr);
	}
}

void Account::setPendingContactAddress (LinphoneAddress *contact) {
	if (mPendingContactAddress) {
		linphone_address_unref(mPendingContactAddress);
		mPendingContactAddress = nullptr;
	}

	if (contact) mPendingContactAddress = linphone_address_clone(contact);
}

void Account::setServiceRouteAddress (LinphoneAddress *serviceRoute) {
	if (mServiceRouteAddress) {
		linphone_address_unref(mServiceRouteAddress);
		mServiceRouteAddress = nullptr;
	}

	if (serviceRoute) mServiceRouteAddress = linphone_address_clone(serviceRoute);
}

//Enable register on account dependent on the given one (if any).
//Also force contact address of dependent account to the given one
void Account::updateDependentAccount(LinphoneRegistrationState state, const std::string &message) {
	if (!mCore) return;
	bctbx_list_t *it = mCore->sip_conf.accounts;

	for (;it;it = it->next) {
		LinphoneAccount *tmp = reinterpret_cast<LinphoneAccount *>(it->data);
		auto params = Account::toCpp(tmp)->mParams;
		lInfo() << "updateDependentAccount(): " << this << " is registered, checking for [" << tmp
			<< "] ->dependency=" << linphone_account_get_dependency(tmp);
		
		if (tmp != this->toC() && linphone_account_get_dependency(tmp) == this->toC()) {
			auto tmpCpp = Account::toCpp(tmp);
			if (!params->mRegisterEnabled) {
				lInfo() << "Dependant account [" << tmp << "] has registration disabled, so it will not register.";
				continue;
			}
			auto copyParams = params->clone()->toSharedPtr();
			if (state == LinphoneRegistrationOk) {
				// Force dependent account to re-register
				params->mRegisterEnabled = false;
				copyParams->mRegisterEnabled = true;
				const SalAddress *salAddr = mOp->getContactAddress();

				if (salAddr) {
					if (mContactAddress) {
						linphone_address_unref(mContactAddress);
					}
					char *sal_addr = sal_address_as_string(salAddr);
					mContactAddress = linphone_address_new(sal_addr);
					bctbx_free(sal_addr);
				}
			} else if (state == LinphoneRegistrationCleared || state == LinphoneRegistrationFailed) {
				tmpCpp->pauseRegister();
				tmpCpp->setState(state, message);
			}
			tmpCpp->setAccountParams(copyParams);
			tmpCpp->update();
		}
	}
}

void Account::setState (LinphoneRegistrationState state, const std::string& message) {
	if (mState != state || state == LinphoneRegistrationOk) { /*allow multiple notification of LinphoneRegistrationOk for refreshing*/
		const char *identity = mParams ? mParams->mIdentity.c_str() : "";
		if (!mParams) lWarning() << "AccountParams not set for Account [" << this->toC() << "]";
		lInfo() << "Account [" << this << "] for identity [" << identity << "] moving from state ["
			<< linphone_registration_state_to_string(mState) << "] to ["
			<< linphone_registration_state_to_string(state) << "] on core [" << mCore << "]";

		if (state == LinphoneRegistrationOk) {
			const SalAddress *salAddr = mOp->getContactAddress();
			if (salAddr) L_GET_CPP_PTR_FROM_C_OBJECT(mContactAddress)->setInternalAddress(salAddr);
		}

		if (linphone_core_should_subscribe_friends_only_when_registered(mCore) && mState != state && state == LinphoneRegistrationOk) {
			lInfo() << "Updating friends for identity [" << identity << "] on core [" << mCore << "]";
			/* state must be updated before calling linphone_core_update_friends_subscriptions*/
			mState = state;
			linphone_core_update_friends_subscriptions(mCore);
		} else {
			/*at this point state must be updated*/
			mState = state;
		}

		if (!mDependency) {
			updateDependentAccount(state, message);
		}

		_linphone_account_notify_registration_state_changed(this->toC(), state, message.c_str());
		if (mCore) linphone_core_notify_account_registration_state_changed(mCore, this->toC(), state, message.c_str());
		if (mConfig && mCore) {
			// Compatibility with proxy config
			linphone_core_notify_registration_state_changed(mCore, mConfig, state, message.c_str());
		}
	} else {
		/*state already reported*/
	}
}

void Account::setOp (SalRegisterOp *op) {
	mOp = op;
}

void Account::setCustomheader (const std::string& headerName, const std::string& headerValue) {
	mSentHeaders = sal_custom_header_append(mSentHeaders, headerName.c_str(), headerValue.c_str());
	mRegisterChanged = true;
}

void Account::setPresencePublishEvent (LinphoneEvent *presencePublishEvent) {
	mPresencePublishEvent = presencePublishEvent;
}

void Account::setDependency (std::shared_ptr<Account> dependency) {
	if (!mParams) {
		lWarning() << "setDependency is called but no AccountParams is set on Account [" << this->toC() << "]";
		return;
	}

	if (dependency) {
		mDependency = dependency;
		mParams->mDependsOn = dependency->mParams->mIdKey;
	} else {
		mDependency = nullptr;
		mParams->mDependsOn = "";
	}
}

// -----------------------------------------------------------------------------

int Account::getAuthFailure () const {
	return mAuthFailure;
}

bool Account::getRegisterChanged () const {
	return mRegisterChanged;
}

time_t Account::getDeletionDate () const {
	return mDeletionDate;
}

const std::string& Account::getSipEtag () const {
	return mSipEtag;
}

void *Account::getUserData () const {
	return mUserData;
}

LinphoneCore *Account::getCore () const {
	return mCore;
}

LinphoneErrorInfo *Account::getErrorInfo () {
	if (!mErrorInfo) mErrorInfo = linphone_error_info_new();
	linphone_error_info_from_sal_op(mErrorInfo, mOp);
	return mErrorInfo;
}

LinphoneAddress *Account::getContactAddress () const {
	return mContactAddress;
}

LinphoneAddress* Account::getContactAddressWithoutParams () const {
	return mContactAddressWithoutParams;
}

LinphoneAddress* Account::getPendingContactAddress () const {
	return mPendingContactAddress;
}

LinphoneAddress* Account::getServiceRouteAddress () {
	if (!mOp) return nullptr;

	const SalAddress *salAddr = mOp->getServiceRoute();
	if (!salAddr) return nullptr;

	if (mServiceRouteAddress) {
		L_GET_CPP_PTR_FROM_C_OBJECT(mServiceRouteAddress)->setInternalAddress(const_cast<SalAddress *>(salAddr));
	} else {
		char *buf = sal_address_as_string(salAddr);
		mServiceRouteAddress = linphone_address_new(buf);
		ms_free(buf);
	}

	return mServiceRouteAddress;
}

LinphoneRegistrationState Account::getState () const {
	return mState;
}

SalRegisterOp* Account::getOp() const {
	return mOp;
}

const char* Account::getCustomHeader (const std::string& headerName) const {
	if (!mOp) return nullptr;

	return sal_custom_header_find(mOp->getRecvCustomHeaders(), headerName.c_str());
}

LinphoneEvent* Account::getPresencePublishEvent () const {
	return mPresencePublishEvent;
}

std::shared_ptr<Account> Account::getDependency () {
	return mDependency;
}

// -----------------------------------------------------------------------------

LinphoneAddress *Account::guessContactForRegister () {
	LinphoneAddress *result = nullptr;

	if (mDependency) {
		//In case of dependent account, force contact of 'master' account, but only after a successful register
		return linphone_address_clone(mDependency->mContactAddress);
	}
	LinphoneAddress *proxy = linphone_address_new(mParams->mProxy.c_str());
	if (!proxy)
		return nullptr;
	const char *host = linphone_address_get_domain(proxy);
	if (host) {
		result = linphone_address_clone(mParams->mIdentityAddress);
		if (!mParams->mContactParameters.empty()) {
			// We want to add a list of contacts params to the linphone address
			linphone_address_set_params(result, mParams->mContactParameters.c_str());
		}
		if (!mParams->mContactUriParameters.empty())
			linphone_address_set_uri_params(result, mParams->mContactUriParameters.c_str());
	}
	linphone_address_unref(proxy);
	return result;
}

void Account::registerAccount () {
	if (mParams->mRegisterEnabled) {
		LinphoneAddress* proxy = linphone_address_new(mParams->mProxy.c_str());
		char *proxy_string;
		char *from = linphone_address_as_string(mParams->mIdentityAddress);
		lInfo() << "LinphoneAccount [" << this << "] about to register (LinphoneCore version: " << linphone_core_get_version() << ")";
		proxy_string=linphone_address_as_string_uri_only(proxy);
		linphone_address_unref(proxy);
		if (mOp)
			mOp->release();
		mOp = new SalRegisterOp(mCore->sal.get());

		linphone_configure_op(mCore, mOp, mParams->mIdentityAddress, mSentHeaders, FALSE);

		LinphoneAddress *contactAddress = guessContactForRegister();
		if (contactAddress) {
			mOp->setContactAddress(L_GET_CPP_PTR_FROM_C_OBJECT(contactAddress)->getInternalAddress());
			if (!mContactAddress) {
				mContactAddress = linphone_address_clone(contactAddress);
			}
			linphone_address_unref(contactAddress);
		}
		mOp->setUserPointer(this->toC());

		if (mOp->sendRegister(
			proxy_string,
			mParams->mIdentity,
			mParams->mExpires,
			mPendingContactAddress ? L_GET_CPP_PTR_FROM_C_OBJECT(mPendingContactAddress)->getInternalAddress() : NULL
		)==0) {
			if (mPendingContactAddress) {
				linphone_address_unref(mPendingContactAddress);
				mPendingContactAddress = nullptr;
			}
			setState(LinphoneRegistrationProgress, "Registration in progress");
		} else {
			setState(LinphoneRegistrationFailed, "Registration failed");
		}
		ms_free(proxy_string);
		ms_free(from);
	} else {
		/* unregister if registered*/
		unregister();
		if (mState == LinphoneRegistrationProgress) {
			setState(LinphoneRegistrationCleared, "Registration cleared");
		}
	}
}

void Account::refreshRegister () {
	if (!mParams) {
		lWarning() << "refreshRegister is called but no AccountParams is set on Account [" << this->toC() << "]";
		return;
	}

	if (mParams->mRegisterEnabled && mOp && mState != LinphoneRegistrationProgress) {
		if (mOp->refreshRegister(mParams->mExpires) == 0) {
			setState(LinphoneRegistrationProgress, "Refresh registration");
		}
	}
}

void Account::pauseRegister () {
	if (mOp) mOp->stopRefreshing();
}

void Account::unregister () {
	if (mOp && (mState == LinphoneRegistrationOk ||
					(mState == LinphoneRegistrationProgress && mParams->mExpires != 0))) {
		mOp->unregister();
	}
}

void Account::unpublish () {
	if (mPresencePublishEvent
		&& (linphone_event_get_publish_state(mPresencePublishEvent) == LinphonePublishOk ||
					(linphone_event_get_publish_state(mPresencePublishEvent)  == LinphonePublishProgress && mParams->mPublishExpires != 0))) {
		linphone_event_unpublish(mPresencePublishEvent);
	}
	if (!mSipEtag.empty()) {
		mSipEtag = "";
	}
}

void Account::notifyPublishStateChanged (LinphonePublishState state) {
	if (mPresencePublishEvent != nullptr) {
		switch (state) {
			case LinphonePublishCleared:
				setSipEtag("");
				BCTBX_NO_BREAK;
			case LinphonePublishError:
				linphone_event_unref(mPresencePublishEvent);
				mPresencePublishEvent = NULL;
				break;
			case LinphonePublishOk:
				setSipEtag(linphone_event_get_custom_header(mPresencePublishEvent, "SIP-ETag"));
				break;
			default:
				break;
		}
	}
}

void Account::stopRefreshing () {
	LinphoneAddress *contact_addr = nullptr;
	const SalAddress *sal_addr = mOp && mState == LinphoneRegistrationOk ? mOp->getContactAddress() : nullptr;
	if (sal_addr) {
		char *buf = sal_address_as_string(sal_addr);
		contact_addr = buf ? linphone_address_new(buf) : nullptr;
		ms_free(buf);
	}

	/*with udp, there is a risk of port reuse, so I prefer to not do anything for now*/
	if (contact_addr) {
		if (linphone_address_get_transport(contact_addr) != LinphoneTransportUdp && linphone_config_get_int(mCore->config, "sip", "unregister_previous_contact", 0)) {
			if (mPendingContactAddress)
				linphone_address_unref(mPendingContactAddress);
			mPendingContactAddress = contact_addr;
		} else {
			linphone_address_unref(contact_addr);
		}
	}

	if (mPresencePublishEvent) { /*might probably do better*/
		linphone_event_set_publish_state(mPresencePublishEvent, LinphonePublishNone);
		linphone_event_unref(mPresencePublishEvent);
		mPresencePublishEvent = nullptr;
	}

	if (mOp) {
		mOp->release();
		mOp = nullptr;
	}
}

LinphoneReason Account::getError () {
	return linphone_error_info_get_reason(getErrorInfo());
}

static LinphoneTransportType salTransportToLinphoneTransport(SalTransport sal) {
	switch(sal) {
		case SalTransportUDP:
			return LinphoneTransportUdp;
		
		case SalTransportTCP:
			return LinphoneTransportTcp;

		case SalTransportDTLS:
			return LinphoneTransportDtls;

		case SalTransportTLS:
			return LinphoneTransportTls;
	}

	return LinphoneTransportUdp;
}

LinphoneTransportType Account::getTransport () {
	std::string addr;
	LinphoneTransportType ret = LinphoneTransportUdp; /*default value*/
	const SalAddress* route_addr = nullptr;
	bool destroy_route_addr = false;

	if (getServiceRouteAddress()) {
		route_addr = L_GET_CPP_PTR_FROM_C_OBJECT(getServiceRouteAddress())->getInternalAddress();
	} else if (mParams && mParams->getRoutes()) {
		// get first route
		char *tmp =  linphone_address_as_string((LinphoneAddress *)bctbx_list_get_data(mParams->getRoutes()));
		addr = tmp;
		bctbx_free(tmp);
	} else if (mParams && !mParams->getServerAddressAsString().empty()) {
		addr = mParams->getServerAddressAsString();
	} else {
		lError() << "Cannot guess transport for account with identity [" << this->toC() << "]";
		return ret;
	}

	if (!route_addr) {
		if (!((*(SalAddress **)&route_addr) = sal_address_new(addr.c_str())))
			return ret;
		destroy_route_addr = true;
	}

	ret = salTransportToLinphoneTransport(sal_address_get_transport(route_addr));

	if (destroy_route_addr)
		sal_address_unref((SalAddress *)route_addr);

	return ret;
}

bool Account::isAvpfEnabled () const {
	if (!mParams) {
		lWarning() << "isAvpfEnabled is called but no AccountParams is set on Account [" << this->toC() << "]";
		return false;
	}

	if (mParams->mAvpfMode == LinphoneAVPFDefault && mCore) {
		return linphone_core_get_avpf_mode(mCore) == LinphoneAVPFEnabled;
	}

	return mParams->mAvpfMode == LinphoneAVPFEnabled;
}

const LinphoneAuthInfo* Account::findAuthInfo () const {
	if (!mParams) {
		lWarning() << "findAuthInfo is called but no AccountParams is set on Account [" << this->toC() << "]";
		return nullptr;
	}

	const char *username = mParams->mIdentityAddress ? linphone_address_get_username(mParams->mIdentityAddress) : NULL;
	const char *domain =  mParams->mIdentityAddress ? linphone_address_get_domain(mParams->mIdentityAddress) : NULL;
	return linphone_core_find_auth_info(mCore, mParams->mRealm.c_str(), username, domain);
}

int Account::getUnreadChatMessageCount () const {
	if (!mParams) {
		lWarning() << "getUnreadMessageCount is called but no AccountParams is set on Account [" << this->toC() << "]";
		return -1;
	}

	return L_GET_CPP_PTR_FROM_C_OBJECT(mCore)->getUnreadChatMessageCount(
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(mParams->mIdentityAddress))
	);
}

void Account::writeToConfigFile (int index) {
	if (!mParams) {
		lWarning() << "writeToConfigFile is called but no AccountParams is set on Account [" << this->toC() << "]";
		return;
	}

	mParams->writeToConfigFile(mCore->config, index);
}

bool Account::canRegister(){
	if (mCore->sip_conf.register_only_when_network_is_up && !mCore->sip_network_state.global_state) {
		return false;
	}
	if (mDependency) {
		return mDependency->getState() == LinphoneRegistrationOk;
	}
	return true;
}

int Account::done () {
	if (!check())
		return -1;

	/*check if server address has changed*/
	LinphoneAccountAddressComparisonResult res = isServerConfigChanged(mOldParams, mParams);
	if (res != LinphoneAccountAddressEqual) {
		/* server config has changed, need to unregister from previous first*/
		if (mOp) {
			if (res == LinphoneAccountAddressDifferent) {
				unregister();
			}
			mOp->setUserPointer(NULL); /*we don't want to receive status for this un register*/
			mOp->unref(); /*but we keep refresher to handle authentication if needed*/
			mOp = nullptr;
		}
		if (mPresencePublishEvent) {
			if (res == LinphoneAccountAddressDifferent) {
				unpublish();
			}
		}
		mNeedToRegister = true;
	}

	if (mRegisterChanged) {
		mNeedToRegister = true;
		mRegisterChanged = false;
	}

	if (mNeedToRegister){
		pauseRegister();
	}

	if (computePublishParamsHash()) {
		lInfo() << "Publish params have changed on account [" << this->toC() << "]";
		if (mPresencePublishEvent) {
			/*publish is terminated*/
			linphone_event_terminate(mPresencePublishEvent);
		}
		if (mParams->mPublishEnabled) mSendPublish = true;
	} else {
		lInfo() << "Publish params have not changed on account [" << this->toC() << "]";
	}

	if (mCore) {
		linphone_proxy_config_write_all_to_config_file(mCore); // TODO: change it when removing all proxy_config
	}

	return 0;
}

void Account::update () {
	if (mNeedToRegister){
		if (canRegister()){
			registerAccount();
			mNeedToRegister = false;
		}
	}
	if (mSendPublish && (mState == LinphoneRegistrationOk || mState == LinphoneRegistrationCleared)){
		sendPublish(mCore->presence_model);
		mSendPublish = false;
	}
}

string Account::getComputedPushNotificationParameters () {
	if (!mCore
		|| !mCore->push_notification_enabled
		|| !mParams->isPushNotificationAvailable()
	) {
		return string("");
	}

	string provider = L_C_TO_STRING(linphone_push_notification_config_get_provider(mParams->mPushNotificationConfig));
	bool_t use_legacy_params = !!linphone_config_get_int(mCore->config, "net", "use_legacy_push_notification_params", FALSE);
	if (provider.empty()) {
		// Can this be improved ?
		bool tester_env = !!linphone_config_get_int(mCore->config, "tester", "test_env", FALSE);
		if (tester_env) provider = "liblinphone_tester";
		// End of improvement zone
	#ifdef __ANDROID__
			if (use_legacy_params)
				provider = "firebase";
			else
				provider = "fcm";
	#elif TARGET_OS_IPHONE
			provider = tester_env ? "apns.dev" : "apns";
	#endif
	}
	if (provider.empty()) return NULL;

	bool basicPushAllowed = mParams->mPushNotificationAllowed;
	bool remotePushAllowed = mParams->mRemotePushNotificationAllowed;
	string voipToken = L_C_TO_STRING(linphone_push_notification_config_get_voip_token(mParams->mPushNotificationConfig));
	string remoteToken = L_C_TO_STRING(linphone_push_notification_config_get_remote_token(mParams->mPushNotificationConfig));
	string param = L_C_TO_STRING(linphone_push_notification_config_get_param(mParams->mPushNotificationConfig));
	if (param.empty()) {
		string param_format = "%s.%s.%s";
		string team_id = linphone_push_notification_config_get_team_id(mParams->mPushNotificationConfig);
		string bundle_identifer = linphone_push_notification_config_get_bundle_identifier(mParams->mPushNotificationConfig);
		char services[100];
		memset(services, 0, sizeof(services));
		if (basicPushAllowed) {
			strcat(services, "voip");
			if (remotePushAllowed) {
				strcat(services, "&");
			}
		}
		if (remotePushAllowed) {
			strcat(services, "remote");
		}
		char pn_param[200];
		memset(pn_param, 0, sizeof(pn_param));
		snprintf(pn_param, sizeof(pn_param), param_format.c_str(), team_id.c_str(), bundle_identifer.c_str(), services);
		param = pn_param;
	}

	string prid = L_C_TO_STRING(linphone_push_notification_config_get_prid(mParams->mPushNotificationConfig));
	
	string oldVoipToken;
	string oldRemoteToken;
	if (mOldParams) {
		oldVoipToken = L_C_TO_STRING(linphone_push_notification_config_get_voip_token(mOldParams->mPushNotificationConfig));
		oldRemoteToken = L_C_TO_STRING(linphone_push_notification_config_get_remote_token(mOldParams->mPushNotificationConfig));
	}
	if (prid.empty() || oldVoipToken != voipToken || oldRemoteToken != remoteToken) {
		char pn_prid[300];
		memset(pn_prid, 0, sizeof(pn_prid));
		if (basicPushAllowed) {
			strcat(pn_prid, voipToken.c_str());
			if (remotePushAllowed && !remoteToken.empty()) {
				strcat(pn_prid, "&");
			}
		}
		if (remotePushAllowed) {
			strcat(pn_prid, remoteToken.c_str());
		}
		prid = pn_prid;
	}

	string format = "pn-provider=%s;pn-param=%s;pn-prid=%s;pn-timeout=0;pn-silent=1";
	if (use_legacy_params) {
		format = "pn-type=%s;app-id=%s;pn-tok=%s;pn-timeout=0;pn-silent=1";
	}
	char computedPushParams[512];
	memset(computedPushParams, 0, sizeof(computedPushParams));
	snprintf(computedPushParams, sizeof(computedPushParams), format.c_str(), provider.c_str(), param.c_str(), prid.c_str());

	if (remotePushAllowed) {
		string remoteFormat = ";pn-msg-str=%s;pn-call-str=%s;pn-groupchat-str=%s;pn-call-snd=%s;pn-msg-snd=%s";
		string msg_str = L_C_TO_STRING(linphone_push_notification_config_get_msg_str(mParams->mPushNotificationConfig));
		string call_str = L_C_TO_STRING(linphone_push_notification_config_get_call_str(mParams->mPushNotificationConfig));
		string groupchat_str = L_C_TO_STRING(linphone_push_notification_config_get_group_chat_str(mParams->mPushNotificationConfig));
		string call_snd = L_C_TO_STRING(linphone_push_notification_config_get_call_snd(mParams->mPushNotificationConfig));
		string msg_snd = L_C_TO_STRING(linphone_push_notification_config_get_msg_str(mParams->mPushNotificationConfig));

		char remoteSpecificParams[512];
		memset(remoteSpecificParams, 0, sizeof(remoteSpecificParams));
		snprintf(remoteSpecificParams, sizeof(remoteSpecificParams), remoteFormat.c_str(), msg_str.c_str(), call_str.c_str(), groupchat_str.c_str(), call_snd.c_str(), msg_snd.c_str());
		strcat(computedPushParams, remoteSpecificParams);
	}

	return string(computedPushParams);
}

void Account::updatePushNotificationParameters () {
	onPushNotificationAllowedChanged(true);
}

void Account::apply (LinphoneCore *lc) {
	mOldParams = nullptr; // remove old params to make sure we will register since we only call apply when adding accounts to core
	mCore = lc;

	if (mDependency != nullptr) {
		//disable register if master account is not yet registered
		if (mDependency->mState != LinphoneRegistrationOk) {
			if (mParams->mRegisterEnabled != FALSE) {
				mRegisterChanged = TRUE;
			}
			//We do not call enableRegister on purpose here
			//Explicitely disabling register on a dependent config puts it in a disabled state (see cfg->reg_dependent_disabled) to avoid automatic re-enable if masterCfg reach LinphoneRegistrationOk
		}
	}

	done();
}

LinphoneEvent *Account::createPublish (const char *event, int expires) {
	if (!mCore){
		lError() << "Cannot create publish from account [" << this->toC() << "] not attached to any core";
		return nullptr;
	}
	return _linphone_core_create_publish(mCore, this->toC(), NULL, event, expires);
}

int Account::sendPublish (LinphonePresenceModel *presence) {
	int err=0;
	LinphoneAddress *presentity_address = NULL;
	char* contact = NULL;

	if (mState == LinphoneRegistrationOk || mState == LinphoneRegistrationCleared){
		LinphoneContent *content;
		char *presence_body;
		if (mPresencePublishEvent == NULL){
			mPresencePublishEvent = createPublish("presence", mParams->getPublishExpires());
		}
		mPresencePublishEvent->internal = TRUE;

		if (linphone_presence_model_get_presentity(presence) == NULL) {
			lInfo() << "No presentity set for model [" << presence << "], using identity from account [" << this->toC() << "]";
			linphone_presence_model_set_presentity(presence, mParams->getIdentityAddress());
		}

		if (!linphone_address_equal(linphone_presence_model_get_presentity(presence), mParams->getIdentityAddress())) {
			lInfo() << "Presentity for model [" << presence << "] differ account [" << this->toC() << "], using account";
			presentity_address = linphone_address_clone(linphone_presence_model_get_presentity(presence)); /*saved, just in case*/
			if (linphone_presence_model_get_contact(presence)) {
				contact = bctbx_strdup(linphone_presence_model_get_contact(presence));
			}
			linphone_presence_model_set_presentity(presence, mParams->getIdentityAddress());
			linphone_presence_model_set_contact(presence,NULL); /*it will be automatically computed*/

		}
		if (!(presence_body = linphone_presence_model_to_xml(presence))) {
			lError() << "Cannot publish presence model [" << presence << "] for account [" << this->toC() << "] because of xml serialization error";
			return -1;
		}

		content = linphone_content_new();
		linphone_content_set_buffer(content, (const uint8_t *)presence_body,strlen(presence_body));
		linphone_content_set_type(content, "application");
		linphone_content_set_subtype(content,"pidf+xml");
		if (!mSipEtag.empty()) {
			linphone_event_add_custom_header(mPresencePublishEvent, "SIP-If-Match", mSipEtag.c_str());
			mSipEtag = "";
		}
		err = linphone_event_send_publish(mPresencePublishEvent, content);
		linphone_content_unref(content);
		ms_free(presence_body);
		if (presentity_address) {
			linphone_presence_model_set_presentity(presence,presentity_address);
			linphone_address_unref(presentity_address);
		}
		if (contact) {
			linphone_presence_model_set_contact(presence,contact);
			bctbx_free(contact);
		}

	}else mSendPublish = true; /*otherwise do not send publish if registration is in progress, this will be done later*/
	return err;
}

bool Account::check () {
	if (mParams->mProxy.empty())
		return false;
	if (mParams->mIdentityAddress == NULL)
		return false;
	resolveDependencies();
	return TRUE;
}

void Account::releaseOps () {
	if (mOp) {
		mOp->release();
		mOp = nullptr;
	}

	if (mPresencePublishEvent){
		linphone_event_terminate(mPresencePublishEvent);
		linphone_event_unref(mPresencePublishEvent);
		mPresencePublishEvent = nullptr;
	}

	if (mDependency) {
		mDependency = nullptr;
	}
}

void Account::resolveDependencies () {
	if (!mCore) return;

	const bctbx_list_t *elem;
	for(elem = mCore->sip_conf.accounts; elem != NULL; elem = elem->next) {
		LinphoneAccount *account = (LinphoneAccount*)elem->data;
		
		LinphoneAccount *dependency = linphone_account_get_dependency(account);
		string dependsOn = Account::toCpp(account)->mParams->mDependsOn;
		if (dependency != NULL && !dependsOn.empty()) {
			LinphoneAccount *master_account = linphone_core_get_account_by_idkey(mCore, dependsOn.c_str());
			if (master_account != NULL && master_account != dependency) {
				lError() << "LinphoneAccount has a dependency but idkeys do not match: [" << dependsOn << "] != ["
					<< linphone_account_params_get_idkey(linphone_account_get_params(dependency)) << "], breaking dependency now.";
				linphone_account_unref(dependency);
				linphone_account_set_dependency(account, NULL);
				return;
			}else if (master_account == NULL){
				lWarning() << "LinphoneAccount [" << account << "] depends on account [" << dependency << "], which is not currently in the list.";
			}
		}
		if (!dependsOn.empty() && dependency == NULL) {
			LinphoneAccount *dependency_acc = linphone_core_get_account_by_idkey(mCore, dependsOn.c_str());

			if (dependency_acc == NULL) {
				lWarning() << "LinphoneAccount marked as dependent but no account found for idkey [" << dependsOn << "]";
				return;
			} else {
				lInfo() << "LinphoneAccount [" << account << "] now depends on master LinphoneAccount [" << dependency_acc << "]";
				linphone_account_set_dependency(account, dependency_acc);
			}
		}
	}
}

// -----------------------------------------------------------------------------

void Account::addCallbacks (LinphoneAccountCbs *callbacks) {
	mCallbacksList = bctbx_list_append(mCallbacksList, callbacks);
}

void Account::removeCallbacks (LinphoneAccountCbs *callbacks) {
	bctbx_list_remove(mCallbacksList, callbacks);
}

void Account::setCurrentCallbacks (LinphoneAccountCbs *callbacks) {
	mCurrentCallbacks = callbacks;
}

LinphoneAccountCbs* Account::getCurrentCallbacks () const {
	return mCurrentCallbacks;
}

const bctbx_list_t* Account::getCallbacksList () const {
	return mCallbacksList;
}

// -----------------------------------------------------------------------------

void Account::onPushNotificationAllowedChanged (bool callDone) {
	string computedPushParams = getComputedPushNotificationParameters();
	string contactUriParams = mParams->mContactUriParameters;

	// Do not alter contact uri params for account without push notification allowed
	if (!computedPushParams.empty() && mCore && mCore->push_notification_enabled && (mParams->mPushNotificationAllowed || mParams->mRemotePushNotificationAllowed)) {
		if (contactUriParams.empty() || contactUriParams != computedPushParams) {
			mParams->setContactUriParameters(computedPushParams);

			// If callDone is false then this is called from setAccountParams and there is no need to call done()
			if (callDone) {
				mNeedToRegister = true;
				done();
			}

			lInfo() << "Push notification information [" << computedPushParams << "] added to account [" << this->toC() << "]";
		}
	} else {
		if (!contactUriParams.empty()) {
			mParams->setContactUriParameters("");

			// If callDone is false then this is called from setAccountParams and there is no need to call done()
			if (callDone) {
				mNeedToRegister = true;
				done();
			}

			lInfo() << "Push notification information removed from account [" << this->toC() << "]";
		}
	}
}

void Account::onInternationalPrefixChanged () {
	/* Ensure there is a default account otherwise after invalidating friends maps we won't be able to recompute phone numbers */
	/* Also it is useless to do it if the account being edited isn't the default one */
	if (mCore && this->toC() == linphone_core_get_default_account(mCore)) {
		linphone_core_invalidate_friends_maps(mCore);
	}
}

void Account::onConferenceFactoryUriChanged (const std::string &conferenceFactoryUri) {
	if (!conferenceFactoryUri.empty()) {
		if (mCore) {
			linphone_core_add_linphone_spec(mCore, "groupchat/1.1");
			linphone_core_add_linphone_spec(mCore, "ephemeral");
		}
	} else if (mCore) {
		bool remove = true;
		//Check that no other account needs the specs before removing it
		for (bctbx_list_t *it = mCore->sip_conf.accounts; it; it = it->next) {
			if (it->data != this->toC()) {
				const char *confUri = linphone_account_params_get_conference_factory_uri(linphone_account_get_params((LinphoneAccount *) it->data));
				if (confUri && strlen(confUri)) {
					remove = false;
					break;
				}
			}
		}
		if (remove) {
			linphone_core_remove_linphone_spec(mCore, "groupchat/1.1");
			linphone_core_remove_linphone_spec(mCore, "ephemeral");
		}
	}
}

void Account::onNatPolicyChanged (LinphoneNatPolicy *policy) {
	policy->lc = mCore;
}

LinphoneProxyConfig *Account::getConfig () const {
	return mConfig;
}

void Account::setConfig (LinphoneProxyConfig *config) {
	mConfig = config;
}

LinphoneAccountAddressComparisonResult Account::isServerConfigChanged() {
	return isServerConfigChanged(mOldParams, mParams);
}

LINPHONE_END_NAMESPACE
