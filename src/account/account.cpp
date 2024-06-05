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

#include <bctoolbox/defs.h>

#include "account.h"

#include "content/content.h"
#include "core/core.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "push-notification/push-notification-config.h"
#ifdef HAVE_ADVANCED_IM
#ifdef HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-encryption-engine.h"
#endif // HAVE_LIME_X3DH
#endif // HAVE_ADVANCED_IM
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "core/core-p.h"
#include "db/main-db-p.h"
#include "event/event-publish.h"
#include "linphone/core.h"
#include "presence/presence-service.h"
#include "private.h"
#include "utils/custom-params.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Account::Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params)
    : CoreAccessor(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr) {
	mParams = params;
	mMissedCalls = 0;
	applyParamsChanges();
	lInfo() << "Account [" << this << "] created with params";
}

Account::Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params, LinphoneProxyConfig *config)
    : Account(lc, params) {
	setConfig(config, false);
	lInfo() << "Account [" << this << "] created with params and proxy config";
}

Account::~Account() {
	lInfo() << "Account [" << this << "] destroyed";
	if (mSentHeaders) sal_custom_header_free(mSentHeaders);
	setDependency(nullptr);
	if (mErrorInfo) linphone_error_info_unref(mErrorInfo);
	if (mPresenceModel) linphone_presence_model_unref(mPresenceModel);

	setConfig(nullptr);
	releaseOps();
}

Account *Account::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

static std::string appendLinphoneAddress(const std::shared_ptr<Address> &addr, const std::string &out) {
	auto res = out;
	if (addr) {
		res.append(addr->toString());
	}
	return res;
}

static std::string appendString(const std::string &string, const std::string &out) {
	auto res = out;
	if (!string.empty()) {
		res.append(string);
	}
	return res;
}

bool Account::computePublishParamsHash() {
	std::string source;
	char hash[33];
	char saved;
	unsigned long long previous_hash[2];
	previous_hash[0] = mPreviousPublishParamsHash[0];
	previous_hash[1] = mPreviousPublishParamsHash[1];

	source.append(std::to_string(static_cast<int>(mParams->mPrivacy)));
	source = appendLinphoneAddress(mParams->mIdentityAddress, source);
	source = appendString(mParams->mProxy, source);
	const auto &routes = mParams->mRoutes;
	for (const auto &route : routes) {
		source = appendLinphoneAddress(route, source);
	}
	source = appendString(mParams->mRealm, source);
	source.append(std::to_string(mParams->mPublishExpires));
	source.append(std::to_string(mParams->mPublishEnabled ? 1 : 0));
	belle_sip_auth_helper_compute_ha1(source.c_str(), "dummy", "dummy", hash);
	saved = hash[16];
	hash[16] = '\0';
	mPreviousPublishParamsHash[0] = strtoull(hash, (char **)NULL, 16);
	hash[16] = saved;
	mPreviousPublishParamsHash[1] = strtoull(&hash[16], (char **)NULL, 16);
	return previous_hash[0] != mPreviousPublishParamsHash[0] || previous_hash[1] != mPreviousPublishParamsHash[1];
}

LinphoneAccountAddressComparisonResult Account::compareLinphoneAddresses(const std::shared_ptr<const Address> &a,
                                                                         const std::shared_ptr<const Address> &b) {
	if (a == NULL && b == NULL) return LinphoneAccountAddressEqual;
	else if (!a || !b) return LinphoneAccountAddressDifferent;

	if (*a == *b) return LinphoneAccountAddressEqual;
	if (a->weakEqual(*b)) {
		/*also check both transport and uri */
		if (a->getSecure() == b->getSecure() && a->getTransport() == b->getTransport())
			return LinphoneAccountAddressWeakEqual;
		else return LinphoneAccountAddressDifferent;
	}
	return LinphoneAccountAddressDifferent; /*either username, domain or port ar not equals*/
}

LinphoneAccountAddressComparisonResult Account::isServerConfigChanged(std::shared_ptr<AccountParams> oldParams,
                                                                      std::shared_ptr<AccountParams> newParams) {
	std::shared_ptr<Address> oldProxy =
	    oldParams != nullptr && !oldParams->mProxy.empty() ? Address::create(oldParams->mProxy) : NULL;
	std::shared_ptr<Address> newProxy = !newParams->mProxy.empty() ? Address::create(newParams->mProxy) : NULL;
	LinphoneAccountAddressComparisonResult result_identity;
	LinphoneAccountAddressComparisonResult result;

	result = compareLinphoneAddresses(oldParams != nullptr ? oldParams->mIdentityAddress : NULL,
	                                  newParams->mIdentityAddress);
	if (result == LinphoneAccountAddressDifferent) goto end;
	result_identity = result;

	result = compareLinphoneAddresses(oldProxy, newProxy);

	if (mContactAddress && !mContactAddress->getUriParamValue("gr").empty() && result != LinphoneAccountAddressEqual) {
		// Returning weak equal to be sure no unregister will be done
		result = LinphoneAccountAddressWeakEqual;
		goto end;
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
	lInfo() << "linphoneAccountIsServerConfigChanged : " << result;

	return result;
}

LinphoneStatus Account::setAccountParams(std::shared_ptr<AccountParams> params) {
	mOldParams = mParams ? mParams : nullptr;

	// Equivalent of the old proxy_config_edit
	computePublishParamsHash();

	if (mParams->mPublishEnabled && mPresencePublishEvent) {
		mPresencePublishEvent->pause();
	}

	// Replacing the old params by the updated one
	mParams = params;

	// Some changes in AccountParams needs a special treatment in Account
	applyParamsChanges();

	// Equivalent of the old proxy_config_done
	return done();
}

std::shared_ptr<const AccountParams> Account::getAccountParams() const {
	return mParams;
}

bool Account::customContactChanged() {
	if (!mOldParams) return false;
	if (mParams->mCustomContact == nullptr && mOldParams->mCustomContact == nullptr) return false;
	if (mParams->mCustomContact != nullptr && mOldParams->mCustomContact == nullptr) return true;
	if (mParams->mCustomContact == nullptr && mOldParams->mCustomContact != nullptr) return true;
	return ((*mOldParams->mCustomContact) != (*mParams->mCustomContact));
}

void Account::applyParamsChanges() {
	if (mOldParams == nullptr || mOldParams->mInternationalPrefix != mParams->mInternationalPrefix)
		onInternationalPrefixChanged();

	if (mOldParams == nullptr ||
	    ((mOldParams->mConferenceFactoryAddress != nullptr) ^ (mParams->mConferenceFactoryAddress != nullptr)) ||
	    ((mOldParams->mConferenceFactoryAddress != nullptr) && (mParams->mConferenceFactoryAddress != nullptr) &&
	     (*mOldParams->mConferenceFactoryAddress == *mParams->mConferenceFactoryAddress))) {
		onConferenceFactoryAddressChanged(mParams->mConferenceFactoryAddress);
	}

	if (mOldParams == nullptr ||
	    ((mOldParams->mAudioVideoConferenceFactoryAddress != nullptr) ^
	     (mParams->mAudioVideoConferenceFactoryAddress != nullptr)) ||
	    ((mOldParams->mAudioVideoConferenceFactoryAddress != nullptr) &&
	     (mParams->mAudioVideoConferenceFactoryAddress != nullptr) &&
	     (*mOldParams->mAudioVideoConferenceFactoryAddress == *mParams->mAudioVideoConferenceFactoryAddress))) {
		onAudioVideoConferenceFactoryAddressChanged(mParams->mAudioVideoConferenceFactoryAddress);
	}

	if (mOldParams == nullptr || mOldParams->mNatPolicy != mParams->mNatPolicy)
		if (mParams->mNatPolicy != nullptr) onNatPolicyChanged(mParams->mNatPolicy);

	if (mOldParams == nullptr || mOldParams->mLimeServerUrl != mParams->mLimeServerUrl) {
		onLimeServerUrlChanged(mParams->mLimeServerUrl);
		mRegisterChanged = true;
	}

	if (mOldParams == nullptr || mOldParams->mRegisterEnabled != mParams->mRegisterEnabled ||
	    mOldParams->mExpires != mParams->mExpires || mOldParams->mContactParameters != mParams->mContactParameters ||
	    mOldParams->mContactUriParameters != mParams->mContactUriParameters ||
	    mOldParams->mPushNotificationAllowed != mParams->mPushNotificationAllowed ||
	    mOldParams->mRemotePushNotificationAllowed != mParams->mRemotePushNotificationAllowed ||
	    !(mOldParams->mPushNotificationConfig->isEqual(*mParams->mPushNotificationConfig)) || customContactChanged()) {
		mRegisterChanged = true;
	}

	if (mOldParams == nullptr ||
	    ((mOldParams->mMwiServerAddress != nullptr) ^ (mParams->mMwiServerAddress != nullptr)) ||
	    ((mOldParams->mMwiServerAddress != nullptr) && (mParams->mMwiServerAddress != nullptr) &&
	     (*mOldParams->mMwiServerAddress != *mParams->mMwiServerAddress))) {
		onMwiServerAddressChanged();
	}
}

// -----------------------------------------------------------------------------

void Account::setAuthFailure(int authFailure) {
	mAuthFailure = authFailure;
}

void Account::setRegisterChanged(bool registerChanged) {
	mRegisterChanged = registerChanged;
}

void Account::setSendPublish(bool sendPublish) {
	mSendPublish = sendPublish;
	if (mSendPublish) {
		triggerUpdate();
	}
}

void Account::setNeedToRegister(bool needToRegister) {
	mNeedToRegister = needToRegister;
	if (mNeedToRegister) {
		try {
			auto engine = getCore()->getEncryptionEngine();
			if (engine && mParams) {
				if (!mParams->getLimeServerUrl().empty() || !getCore()->getX3dhServerUrl().empty()) {
					mLimeUserAccountStatus = LimeUserAccountStatus::LimeUserAccountNeedCreation;
				}
			}
		} catch (const bad_weak_ptr &) {
			// Core pointer is null
		}

		triggerUpdate();
	}
}

void Account::setDeletionDate(time_t deletionDate) {
	mDeletionDate = deletionDate;
}

void Account::setSipEtag(const std::string &sipEtag) {
	mSipEtag = sipEtag;
}

void Account::setErrorInfo(LinphoneErrorInfo *errorInfo) {
	mErrorInfo = errorInfo;
}

void Account::setContactAddress(const std::shared_ptr<const Address> &contact) {
	mContactAddress = nullptr;
	if (contact) mContactAddress = contact->clone()->toSharedPtr();
	setContactAddressWithoutParams(contact);
}

void Account::setContactAddressWithoutParams(const std::shared_ptr<const Address> &contact) {
	mContactAddressWithoutParams = nullptr;

	if (contact) {
		mContactAddressWithoutParams = contact->clone()->toSharedPtr();
		mContactAddressWithoutParams->clean();
		mContactAddressWithoutParams->setPort(-1);
		mContactAddressWithoutParams->setDomain(std::string());
		mContactAddressWithoutParams->setDisplayName(std::string());
	}
}

void Account::setPendingContactAddress(std::shared_ptr<Address> contact) {
	if (mPendingContactAddress) {
		mPendingContactAddress = nullptr;
	}

	if (contact) mPendingContactAddress = contact;
}

void Account::setServiceRouteAddress(std::shared_ptr<Address> serviceRoute) {
	mServiceRouteAddress = nullptr;

	if (serviceRoute) mServiceRouteAddress = serviceRoute->clone()->toSharedPtr();
}

// Enable register on account dependent on the given one (if any).
// Also force contact address of dependent account to the given one
void Account::updateDependentAccount(LinphoneRegistrationState state, const std::string &message) {
	auto core = getCCore();

	if (!core) return;
	auto dependee = getDependee();
	if (dependee) {
		auto params = dependee->mParams;
		if (!params->mRegisterEnabled) {
			lInfo() << "Dependant account [" << dependee << "] has registration disabled, so it will not register.";
			return;
		}
		auto copyParams = params->clone()->toSharedPtr();
		if (state == LinphoneRegistrationOk) {
			// Force dependent account to re-register
			params->mRegisterEnabled = false;
			copyParams->mRegisterEnabled = true;
			const SalAddress *salAddr = mOp->getContactAddress();

			if (!mContactAddress) {
				mContactAddress = Address::create();
			}
			if (salAddr) {
				mContactAddress->setImpl(salAddr);
			}
		} else if (state == LinphoneRegistrationCleared || state == LinphoneRegistrationFailed) {
			dependee->pauseRegister();
			dependee->setState(state, message);
		}
		dependee->setAccountParams(copyParams);
		dependee->update();
	}
}

void Account::setState(LinphoneRegistrationState state, const std::string &message) {
	auto core = getCCore();
	if (mState != state ||
	    state == LinphoneRegistrationOk) { /*allow multiple notification of LinphoneRegistrationOk for refreshing*/
		const auto identity = (mParams) ? mParams->getIdentityAddress()->toString() : std::string("sip:");
		if (!mParams) lWarning() << "AccountParams not set for Account [" << this << "]";
		lInfo() << "Account [" << this << "] for identity [" << identity << "] moving from state ["
		        << linphone_registration_state_to_string(mState) << "] to ["
		        << linphone_registration_state_to_string(state) << "] on core [" << core << "]";
		mIsUnregistering = false;
		if (state == LinphoneRegistrationOk) {

			const auto salAddr = mOp->getContactAddress();
			if (salAddr) {
				if (!mContactAddress) {
					mContactAddress = (new Address())->toSharedPtr();
				}
				mContactAddress->setImpl(salAddr);
			}
			mOldParams = nullptr; // We can drop oldParams, since last registration was successful.
		}

		LinphoneRegistrationState previousState = mState;
		mState = state;
		if (!mDependency) {
			updateDependentAccount(state, message);
		}

		_linphone_account_notify_registration_state_changed(this->toC(), state, message.c_str());
		if (core) linphone_core_notify_account_registration_state_changed(core, this->toC(), state, message.c_str());
		if (mConfig && core) {
			// Compatibility with proxy config
			linphone_core_notify_registration_state_changed(core, mConfig, state, message.c_str());
		}

		if (linphone_core_should_subscribe_friends_only_when_registered(core) && state == LinphoneRegistrationOk &&
		    previousState != state) {
			linphone_core_update_friends_subscriptions(core);
		}
		if (state == LinphoneRegistrationOk && previousState != state) {
			subscribeToMessageWaitingIndication();
		}

		triggerUpdate();
	} else {
		/*state already reported*/
	}
}

void Account::setOp(SalRegisterOp *op) {
	mOp = op;
}

void Account::setCustomheader(const std::string &headerName, const std::string &headerValue) {
	mSentHeaders = sal_custom_header_append(mSentHeaders, headerName.c_str(), headerValue.c_str());
	mRegisterChanged = true;
}

void Account::setPresencePublishEvent(const std::shared_ptr<EventPublish> &presencePublishEvent) {
	mPresencePublishEvent = presencePublishEvent;
}

void Account::setDependee(std::shared_ptr<Account> dependee) {
	mDependee = dependee;
}

void Account::setDependency(std::shared_ptr<Account> dependency) {
	if (!mParams) {
		lWarning() << "setDependency is called but no AccountParams is set on Account [" << this << "]";
		return;
	}

	if (dependency && (dependency != getSharedFromThis())) {
		mDependency = dependency;
		dependency->setDependee(getSharedFromThis());
		mParams->mDependsOn = dependency->mParams->mIdKey;
	} else {
		if (mDependency) {
			mDependency->setDependee(nullptr);
		}
		mDependency = nullptr;
		mParams->mDependsOn = "";
	}
}

void Account::setLimeUserAccountStatus(LimeUserAccountStatus status) {
	mLimeUserAccountStatus = status;

	if (mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountCreated ||
	    mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountCreationSkiped ||
	    mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountNone) {
		// Trigger the update again so that it can continue.
		triggerUpdate();
	}
}

// -----------------------------------------------------------------------------

int Account::getAuthFailure() const {
	return mAuthFailure;
}

bool Account::getRegisterChanged() const {
	return mRegisterChanged;
}

time_t Account::getDeletionDate() const {
	return mDeletionDate;
}

const std::string &Account::getSipEtag() const {
	return mSipEtag;
}

const LinphoneErrorInfo *Account::getErrorInfo() {
	if (!mErrorInfo) mErrorInfo = linphone_error_info_new();
	linphone_error_info_from_sal_op(mErrorInfo, mOp);
	return mErrorInfo;
}

const std::shared_ptr<Address> &Account::getContactAddress() const {
	return mContactAddress;
}

const std::shared_ptr<Address> &Account::getContactAddressWithoutParams() const {
	return mContactAddressWithoutParams;
}

const std::shared_ptr<Address> &Account::getPendingContactAddress() const {
	return mPendingContactAddress;
}

const std::shared_ptr<Address> Account::getServiceRouteAddress() const {
	if (!mOp) return nullptr;

	const auto salAddr = mOp->getServiceRoute();
	if (!salAddr) return nullptr;

	if (!mServiceRouteAddress) {
		mServiceRouteAddress = Address::create();
	}
	mServiceRouteAddress->setImpl(salAddr);

	return mServiceRouteAddress;
}

LinphoneRegistrationState Account::getPreviousState() const {
	return mPreviousState;
}

LinphoneRegistrationState Account::getState() const {
	return mState;
}

SalRegisterOp *Account::getOp() const {
	return mOp;
}

const char *Account::getCustomHeader(const std::string &headerName) const {
	if (!mOp) return nullptr;

	return sal_custom_header_find(mOp->getRecvCustomHeaders(), headerName.c_str());
}

std::shared_ptr<EventPublish> Account::getPresencePublishEvent() const {
	return mPresencePublishEvent;
}

std::shared_ptr<Account> Account::getDependee() {
	return mDependee.lock();
}

std::shared_ptr<Account> Account::getDependency() {
	return mDependency;
}

LimeUserAccountStatus Account::getLimeUserAccountStatus() const {
	return mLimeUserAccountStatus;
}

// -----------------------------------------------------------------------------

std::shared_ptr<Address> Account::guessContactForRegister() {
	std::shared_ptr<Address> result = nullptr;
	auto core = getCCore();

	if (mDependency) {
		// In case of dependent account, force contact of 'master' account, but only after a successful register
		return mDependency->mContactAddress;
	}
	std::shared_ptr<Address> proxy = Address::create(mParams->mProxy);
	if (!proxy) return nullptr;
	const auto host = proxy->getDomain();
	if (!host.empty()) {
		result = mParams->mIdentityAddress->clone()->toSharedPtr();
		if (!mParams->mContactParameters.empty()) {
			// We want to add a list of contacts params to the linphone address
			result->setParams(mParams->mContactParameters);
		}

		bool successfullyPreparedPushParameters = false;
		auto newParams = mParams->clone()->toSharedPtr();

		if (core && core->push_notification_enabled) {
			if (!newParams->isPushNotificationAvailable()) {
				lError() << "Couldn't compute automatic push notifications parameters on account [" << this
				         << "] because account params do not have available push notifications";
			} else if (newParams->mPushNotificationAllowed || newParams->mRemotePushNotificationAllowed) {
				if (newParams->mPushNotificationConfig->getProvider().empty()) {
					bool tester_env = !!linphone_config_get_int(core->config, "tester", "test_env", FALSE);
					if (tester_env) newParams->mPushNotificationConfig->setProvider("liblinphone_tester");
#if TARGET_OS_IPHONE
					if (tester_env) newParams->mPushNotificationConfig->setProvider("apns.dev");
#endif
				}
				newParams->mPushNotificationConfig->generatePushParams(newParams->mPushNotificationAllowed,
				                                                       newParams->mRemotePushNotificationAllowed);
				successfullyPreparedPushParameters = true;
			}
		}

		if (!newParams->mContactUriParameters.empty()) {
			if (successfullyPreparedPushParameters) {
				// build an Address to make use of useful param management functions
				std::shared_ptr<Address> contactParamsWrapper =
				    Address::create(string("sip:dummy;" + newParams->mContactUriParameters));
				bool didRemoveParams = false;
				for (auto pushParam : newParams->mPushNotificationConfig->getPushParamsMap()) {
					string paramName = pushParam.first;
					if (!contactParamsWrapper->getUriParamValue(paramName).empty()) {
						contactParamsWrapper->removeUriParam(paramName);
						didRemoveParams = true;
						lError() << "Removing '" << paramName << "' from account [" << this
						         << "] contact uri parameters because it will be generated automatically since core "
						            "has push notification enabled";
					}
				}

				if (didRemoveParams) {
					string newContactUriParams;
					const auto &uriParamMap = contactParamsWrapper->getUriParams();
					for (const auto &param : uriParamMap) {
						if (!param.second.empty()) {
							newContactUriParams = newContactUriParams + param.first + "=" + param.second + ";";
						}
					}

					lWarning() << "Account [" << this << "] contact uri parameters changed from '"
					           << newParams->mContactUriParameters << "' to '" << newContactUriParams << "'";
					newParams->mContactUriParameters = newContactUriParams;
				}
			}
			result->setUriParams(newParams->mContactUriParameters);
		}

		if (successfullyPreparedPushParameters) {
			result->setUriParam(PushConfigPridKey, newParams->getPushNotificationConfig()->getPrid());
			result->setUriParam(PushConfigProviderKey, newParams->getPushNotificationConfig()->getProvider());
			result->setUriParam(PushConfigParamKey, newParams->getPushNotificationConfig()->getParam());

			auto &pushParams = newParams->getPushNotificationConfig()->getPushParamsMap();
			result->setUriParam(PushConfigSilentKey, pushParams.at(PushConfigSilentKey));
			result->setUriParam(PushConfigTimeoutKey, pushParams.at(PushConfigTimeoutKey));

			if (mParams->mRemotePushNotificationAllowed) {
				result->setUriParam(PushConfigMsgStrKey, newParams->getPushNotificationConfig()->getMsgStr());
				result->setUriParam(PushConfigCallStrKey, newParams->getPushNotificationConfig()->getCallStr());
				result->setUriParam(PushConfigGroupChatStrKey,
				                    newParams->getPushNotificationConfig()->getGroupChatStr());
				result->setUriParam(PushConfigCallSoundKey, newParams->getPushNotificationConfig()->getCallSnd());
				result->setUriParam(PushConfigMsgSoundKey, newParams->getPushNotificationConfig()->getMsgSnd());
				if (!newParams->getPushNotificationConfig()->getRemotePushInterval().empty())
					result->setUriParam(PushConfigRemotePushIntervalKey,
					                    newParams->getPushNotificationConfig()->getRemotePushInterval());
			}
			lInfo() << "Added push notification informations '"
			        << newParams->getPushNotificationConfig()->asString(mParams->mRemotePushNotificationAllowed)
			        << "' added to account [" << this << "]";
			setAccountParams(newParams);
		}
	}
	return result;
}

std::list<SalAddress *> Account::getOtherContacts() {
	std::list<SalAddress *> ret;
	if (mPendingContactAddress) {
		SalAddress *toRemove = sal_address_clone(mPendingContactAddress->getImpl());
		sal_address_set_params(toRemove, "expires=0");
		ret.push_back(toRemove);
	}
	if (mParams->mCustomContact) {
		SalAddress *toAdd = sal_address_clone(mParams->mCustomContact->getImpl());
		ret.push_back(toAdd);
	}
	if (mOldParams && mOldParams->mCustomContact) {
		if (!mParams->mCustomContact || (*mOldParams->mCustomContact != *mParams->mCustomContact)) {
			/* need to remove previously used custom contact */
			SalAddress *toRemove = sal_address_clone(mOldParams->mCustomContact->getImpl());
			sal_address_set_params(toRemove, "expires=0");
			ret.push_back(toRemove);
		}
	}
	return ret;
}

void Account::registerAccount() {
	if (mParams->mRegisterEnabled) {
		if (mParams->mProxyAddress == nullptr) {
			lError() << "Can't register Account [" << this << "] without a proxy address";
			return;
		}
		if (mParams->mIdentityAddress == nullptr) {
			lError() << "Can't register Account [" << this << "] without an identity address";
			return;
		}
		lInfo() << "LinphoneAccount [" << this
		        << "] about to register (LinphoneCore version: " << linphone_core_get_version() << ")";

		if (mOp) mOp->release();
		mOp = new SalRegisterOp(getCCore()->sal.get());

		linphone_configure_op(getCCore(), mOp, mParams->mIdentityAddress->toC(), mSentHeaders, FALSE);

		std::shared_ptr<Address> contactAddress = guessContactForRegister();
		if (contactAddress) {
			mOp->setContactAddress(contactAddress->getImpl());
		}
		mOp->setUserPointer(this->toC());

		auto otherContacts = getOtherContacts();
		if (mOp->sendRegister(mParams->mProxyAddress->getImpl(), mParams->mIdentityAddress->getImpl(),
		                      mParams->mExpires, otherContacts) == 0) {
			if (mPendingContactAddress) {
				mPendingContactAddress = nullptr;
			}
			setState(LinphoneRegistrationProgress, "Registration in progress");
		} else {
			setState(LinphoneRegistrationFailed, "Registration failed");
		}

		for (auto ct : otherContacts)
			sal_address_unref(ct);
	} else {
		/* unregister if registered*/
		unregister();
		if (mState == LinphoneRegistrationProgress) {
			setState(LinphoneRegistrationCleared, "Registration cleared");
		}
	}
}

void Account::refreshRegister() {
	if (!mParams) {
		lWarning() << "refreshRegister is called but no AccountParams is set on Account [" << this << "]";
		return;
	}

	if (mParams->mRegisterEnabled && mOp && mState != LinphoneRegistrationProgress) {
		if (mOp->refreshRegister(mParams->mExpires) == 0) {
			setState(LinphoneRegistrationRefreshing, "Refresh registration");
		}
	}
}

void Account::pauseRegister() {
	if (mOp) mOp->stopRefreshing();
}

void Account::unregister() {
	if (mOp &&
	    (mState == LinphoneRegistrationOk || (mState == LinphoneRegistrationProgress && mParams->mExpires != 0))) {
		unsubscribeFromMessageWaitingIndication();
		mOp->unregister();
		mIsUnregistering = true;
	}
}

void Account::unpublish() {
	if (mPresencePublishEvent &&
	    (mPresencePublishEvent->getState() == LinphonePublishOk ||
	     (mPresencePublishEvent->getState() == LinphonePublishOutgoingProgress && mParams->mPublishExpires != 0))) {
		mPresencePublishEvent->unpublish();
	}
	if (!mSipEtag.empty()) {
		mSipEtag = "";
	}
}

void Account::notifyPublishStateChanged(LinphonePublishState state) {
	if (mPresencePublishEvent != nullptr) {
		switch (state) {
			case LinphonePublishCleared:
				setSipEtag("");
				BCTBX_NO_BREAK;
			case LinphonePublishError:
				mPresencePublishEvent = nullptr;
				break;
			case LinphonePublishOk: {
				const string &etag = mPresencePublishEvent->getCustomHeader("SIP-ETag");
				if (!etag.empty()) setSipEtag(etag);
				else {
					lWarning() << "SIP-Etag is missing in custom header. The server must provide it for PUBLISH.";
					setSipEtag("");
				}
				break;
			}
			default:
				break;
		}
	}
}

void Account::stopRefreshing() {
	std::shared_ptr<Address> contact_addr = nullptr;
	const SalAddress *sal_addr = mOp && mState == LinphoneRegistrationOk ? mOp->getContactAddress() : nullptr;
	if (sal_addr) {
		char *buf = sal_address_as_string(sal_addr);
		contact_addr = buf ? Address::create(buf) : nullptr;
		ms_free(buf);
	}

	/*with udp, there is a risk of port reuse, so I prefer to not do anything for now*/
	if (contact_addr) {
		if (contact_addr->getTransport() != LinphonePrivate::Transport::Udp &&
		    linphone_config_get_int(getCCore()->config, "sip", "unregister_previous_contact", 0)) {
			mPendingContactAddress = contact_addr;
		}
	}

	if (mPresencePublishEvent) { /*might probably do better*/
		mPresencePublishEvent->setState(LinphonePublishNone);
		mPresencePublishEvent = nullptr;
	}

	if (mOp) {
		mOp->release();
		mOp = nullptr;
	}
}

LinphoneReason Account::getError() {
	return linphone_error_info_get_reason(getErrorInfo());
}

static LinphoneTransportType salTransportToLinphoneTransport(SalTransport sal) {
	switch (sal) {
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

LinphoneTransportType Account::getTransport() {
	std::string addr;
	LinphoneTransportType ret = LinphoneTransportUdp; /*default value*/
	const SalAddress *route_addr = nullptr;
	if (getServiceRouteAddress()) {
		route_addr = getServiceRouteAddress()->getImpl();
	} else if (mParams && !mParams->getRoutes().empty()) {
		// get first route
		route_addr = mParams->getRoutes().front()->getImpl();
	} else if (mParams && !mParams->getServerAddressAsString().empty()) {
		route_addr = mParams->getServerAddress()->getImpl();
	} else {
		lError() << "Cannot guess transport for account with identity [" << this << "]";
		return ret;
	}
	ret = salTransportToLinphoneTransport(sal_address_get_transport(route_addr));

	return ret;
}

bool Account::isAvpfEnabled() const {
	if (!mParams) {
		lWarning() << "isAvpfEnabled is called but no AccountParams is set on Account [" << this << "]";
		return false;
	}
	auto core = getCCore();

	if (mParams->mAvpfMode == LinphoneAVPFDefault && core) {
		return linphone_core_get_avpf_mode(core) == LinphoneAVPFEnabled;
	}

	return mParams->mAvpfMode == LinphoneAVPFEnabled;
}

const LinphoneAuthInfo *Account::findAuthInfo() const {
	if (!mParams) {
		lWarning() << "findAuthInfo is called but no AccountParams is set on Account [" << this << "]";
		return nullptr;
	}

	const std::string username = mParams->mIdentityAddress ? mParams->mIdentityAddress->getUsername() : std::string();
	const std::string domain = mParams->mIdentityAddress ? mParams->mIdentityAddress->getDomain() : std::string();
	return linphone_core_find_auth_info(getCCore(), mParams->mRealm.c_str(), username.c_str(), domain.c_str());
}

int Account::getUnreadChatMessageCount() const {
	if (!mParams) {
		lWarning() << "getUnreadMessageCount is called but no AccountParams is set on Account [" << this << "]";
		return -1;
	}

	return getCore()->getUnreadChatMessageCount(mParams->mIdentityAddress);
}

void Account::updateChatRoomList() const {
	list<shared_ptr<AbstractChatRoom>> results;
	if (!mParams) {
		lWarning() << "updateChatRoomList is called but no AccountParams is set on Account [" << this << "]";
		mChatRoomList.mList = list<shared_ptr<AbstractChatRoom>>();
		return;
	}

	auto localAddress = mParams->mIdentityAddress;
	const list<shared_ptr<AbstractChatRoom>> chatRooms = getCore()->getChatRooms();
	for (auto chatRoom : chatRooms) {
		if (localAddress->weakEqual(*chatRoom->getLocalAddress())) {
			results.push_back(chatRoom);
		}
	}
	mChatRoomList.mList = results;
}

const list<shared_ptr<AbstractChatRoom>> &Account::getChatRooms() const {
	updateChatRoomList();
	return mChatRoomList.mList;
}

const bctbx_list_t *Account::getChatRoomsCList() const {
	updateChatRoomList();
	return mChatRoomList.getCList();
}

int Account::getMissedCallsCount() const {
	return mMissedCalls;
}

void Account::resetMissedCallsCount() {
	mMissedCalls = 0;
}

void Account::setMissedCallsCount(int count) {
	mMissedCalls = count;
}

list<shared_ptr<CallLog>> Account::getCallLogs() const {
	if (!mParams) {
		lWarning() << "getCallLogs is called but no AccountParams is set on Account [" << this << "]";
		list<shared_ptr<CallLog>> callLogs;
		return callLogs;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "getCallLogs is called but core is not running on Account [" << this << "]";
		return {};
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	return mainDb->getCallHistoryForLocalAddress(localAddress, linphone_core_get_max_call_logs(getCore()->getCCore()));
}

list<shared_ptr<CallLog>> Account::getCallLogsForAddress(const std::shared_ptr<const Address> &remoteAddress) const {
	if (!mParams) {
		lWarning() << "getCallLogsForAddress is called but no AccountParams is set on Account [" << this << "]";
		list<shared_ptr<CallLog>> callLogs;
		return callLogs;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "getCallLogsForAddress is called but core is not running on Account [" << this << "]";
		return {};
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	return mainDb->getCallHistory(remoteAddress, localAddress, linphone_core_get_max_call_logs(getCore()->getCCore()));
}

void Account::deleteCallLogs() const {
	if (!mParams) {
		lWarning() << "deleteCallLogs is called but no AccountParams is set on Account [" << this << "]";
		return;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "deleteCallLogs is called but core is not running on Account [" << this << "]";
		return;
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	mainDb->deleteCallHistoryForLocalAddress(localAddress);
}

list<shared_ptr<ConferenceInfo>> Account::getConferenceInfos(const std::list<LinphoneStreamType> capabilities) const {
	if (!mParams) {
		lWarning() << "getConferenceInfos is called but no AccountParams is set on Account [" << this << "]";
		list<shared_ptr<ConferenceInfo>> conferences;
		return conferences;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "getConferenceInfos is called but core is not running on Account [" << this << "]";
		return {};
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	return mainDb->getConferenceInfosWithParticipant(localAddress, capabilities);
}

void Account::writeAllToConfigFile(const std::shared_ptr<Core> core) {
	LinphoneCore *lc = core->getCCore();
	int i = 0;
	;
	if (!linphone_core_ready(lc)) return;

	if (linphone_config_is_readonly(lc->config)) {
		lInfo() << "Storage of accounts to config file is skipped.";
		return;
	}

	LpConfig *config = linphone_core_get_config(lc);
	for (const auto &account : core->getAccounts()) {
		Account::writeToConfigFile(config, account, i);
		i++;
	}
	/*to ensure removed configs are erased:*/
	Account::writeToConfigFile(config, nullptr, i);
	linphone_config_set_int(config, "sip", "default_proxy", core->getDefaultAccountIndex());
	core->getPrivate()->writeNatPolicyConfigurations();
}

void Account::writeToConfigFile(LpConfig *config, const std::shared_ptr<Account> &account, int index) {
	char key[50];

	sprintf(key, "proxy_%i", index);
	linphone_config_clean_section(config, key);
	if (account == nullptr) {
		return;
	}

	account->writeToConfigFile(index);
}

void Account::writeToConfigFile(int index) {
	if (!mParams) {
		lWarning() << "writeToConfigFile is called but no AccountParams is set on Account [" << this << "]";
		return;
	}

	mParams->writeToConfigFile(getCCore()->config, index);
}

void Account::addCustomParam(const std::string &key, const std::string &value) {
	mParams->addCustomParam(key, value);
}

const std::string &Account::getCustomParam(const std::string &key) const {
	return mParams->getCustomParam(key);
}

bool Account::canRegister() {
	LinphoneCore *core = getCCore();
	if (core->sip_conf.register_only_when_network_is_up && !core->sip_network_state.global_state) {
		return false;
	}
	if (mDependency) {
		return mDependency->getState() == LinphoneRegistrationOk;
	}
	return true;
}

int Account::done() {
	if (!check()) return -1;

	/*check if server address has changed*/
	LinphoneAccountAddressComparisonResult res = isServerConfigChanged(mOldParams, mParams);
	if (res != LinphoneAccountAddressEqual) {
		/* server config has changed, need to unregister from previous first*/
		if (mOp) {
			if (res == LinphoneAccountAddressDifferent) {
				unregister();
			}
			mOp->setUserPointer(NULL); /*we don't want to receive status for this un register*/
			mOp->unref();              /*but we keep refresher to handle authentication if needed*/
			mOp = nullptr;
		}
		if (mPresencePublishEvent) {
			if (res == LinphoneAccountAddressDifferent) {
				unpublish();
			}
		}
		setNeedToRegister(true);
	}

	if (mRegisterChanged) {
		setNeedToRegister(true);
		mRegisterChanged = false;
	}

	if (mNeedToRegister) {
		pauseRegister();
	}

	if (computePublishParamsHash()) {
		lInfo() << "Publish params have changed on account [" << this << "]";
		if (mPresencePublishEvent) {
			/*publish is terminated*/
			mPresencePublishEvent->terminate();
		}
		if (mParams->mPublishEnabled) setSendPublish(true);
	} else {
		lInfo() << "Publish params have not changed on account [" << this << "]";
	}

	try {
		if (getCore()) {
			Account::writeAllToConfigFile(getCore());
		}
	} catch (const bad_weak_ptr &) {
	}

	return 0;
}

LinphoneCore *Account::getCCore() const {
	try {
		return L_GET_C_BACK_PTR(getCore());
	} catch (...) {
	}
	return nullptr;
}

void Account::update() {
	auto engine = getCore()->getEncryptionEngine();
	if (engine && (!mParams->getLimeServerUrl().empty() || !getCore()->getX3dhServerUrl().empty()) &&
	    mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountNeedCreation) {
		shared_ptr<Address> addr = mContactAddress;
		if (!addr) {
			shared_ptr<Address> sip = getAccountParams()->getIdentityAddress();
			if (sip) {
				auto gr = getCCore()->sal->getUuid();
				if (gr.empty()) return;
				addr = sip->clone()->toSharedPtr();
				addr->setUriParam("gr", "urn:uuid:" + gr);
			}
		}
		auto account = this->getSharedFromThis();
		if (addr) engine->createLimeUser(account, addr->asStringUriOnly());
	} else if (!engine || (engine && (mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountCreated ||
	                                  mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountCreationSkiped ||
	                                  mLimeUserAccountStatus == LimeUserAccountStatus::LimeUserAccountNone))) {
		/* Either lime isn't enabled
		 * Or lime is enabled, and the Lime User Creation succeed or failed (so skip it and register) */
		if (mNeedToRegister) {
			if (canRegister()) {
				registerAccount();
				mNeedToRegister = false;
			}
		}
		if (mSendPublish && (mState == LinphoneRegistrationOk || mState == LinphoneRegistrationCleared)) {
			if (mPresenceModel == nullptr) {
				setPresenceModel(getCCore()->presence_model);
			}
			sendPublish();
			mSendPublish = false;
		}
	}
}

void Account::apply(LinphoneCore *lc) {
	mOldParams = nullptr; // remove old params to make sure we will register since we only call apply when adding
	                      // accounts to core
	setCore(L_GET_CPP_PTR_FROM_C_OBJECT(lc));

	if (mDependency != nullptr) {
		// disable register if master account is not yet registered
		if (mDependency->mState != LinphoneRegistrationOk) {
			if (mParams->mRegisterEnabled != false) {
				mRegisterChanged = true;
			}
			// We do not call enableRegister on purpose here
			// Explicitely disabling register on a dependent config puts it in a disabled state (see
			// cfg->reg_dependent_disabled) to avoid automatic re-enable if masterCfg reach LinphoneRegistrationOk
		}
	}

	done();
}

shared_ptr<EventPublish> Account::createPublish(const std::string event, int expires) {
	if (!getCore()) {
		lError() << "Cannot create publish from account [" << this << "] not attached to any core";
		return nullptr;
	}
	return dynamic_pointer_cast<EventPublish>(
	    (new EventPublish(getCore(), getSharedFromThis(), NULL, event, expires))->toSharedPtr());
}

void Account::setPresenceModel(LinphonePresenceModel *presence) {
	if (mPresenceModel) {
		linphone_presence_model_unref(mPresenceModel);
		mPresenceModel = nullptr;
	}
	if (presence) mPresenceModel = linphone_presence_model_ref(presence);
}

int Account::sendPublish() {
	if (mPresenceModel == nullptr) {
		lError() << "No presence model has been set for this account, can't send the PUBLISH";
		return -1;
	}

	int err = 0;
	if (mState == LinphoneRegistrationOk || mState == LinphoneRegistrationCleared) {
		int publishExpires = mParams->getPublishExpires();

		if (mPresencePublishEvent != nullptr) {
			LinphonePublishState state = mPresencePublishEvent->getState();
			if (state != LinphonePublishOk && state != LinphonePublishOutgoingProgress &&
			    state != LinphonePublishRefreshing) {
				lInfo() << "Presence publish state is [" << linphone_publish_state_to_string(state)
				        << "], destroying it and creating a new one instead";
				mPresencePublishEvent = nullptr;
			}
		}

		if (mPresencePublishEvent == nullptr) {
			mPresencePublishEvent = createPublish("presence", publishExpires);
		}

		mPresencePublishEvent->setInternal(true);
		if (publishExpires != 1) {
			// Force manual refresh mode so we can go through this method again
			// when PUBLISH is about to expire, so we can update the presence model timestamp
			mPresencePublishEvent->setManualRefresherMode(true);
		}
		const auto &identityAddress = mParams->getIdentityAddress();
		mPresencePublishEvent->setUserData(identityAddress->toC());

		LinphoneConfig *config = linphone_core_get_config(getCCore());
		if (linphone_config_get_bool(config, "sip", "update_presence_model_timestamp_before_publish_expires_refresh",
		                             FALSE)) {
			unsigned int nbServices = linphone_presence_model_get_nb_services(mPresenceModel);
			if (nbServices > 0) {
				LinphonePresenceService *latest_service =
				    linphone_presence_model_get_nth_service(mPresenceModel, nbServices - 1);
				PresenceService::toCpp(latest_service)->setTimestamp(ms_time(nullptr));
			}
		}

		if (linphone_presence_model_get_presentity(mPresenceModel) == NULL) {
			lInfo() << "No presentity set for model [" << mPresenceModel << "], using identity from account [" << this
			        << "]: " << *identityAddress;
			linphone_presence_model_set_presentity(mPresenceModel, identityAddress->toC());
		}

		const auto currentPresentity = linphone_presence_model_get_presentity(mPresenceModel);
		std::shared_ptr<const Address> presentityAddress = nullptr;
		char *contact = NULL;
		if (!linphone_address_equal(currentPresentity, identityAddress->toC())) {
			lInfo() << "Presentity for model [" << mPresenceModel << "] differs account [" << this
			        << "], using account " << *identityAddress;
			presentityAddress = Address::getSharedFromThis(currentPresentity); /*saved, just in case*/
			if (linphone_presence_model_get_contact(mPresenceModel)) {
				contact = bctbx_strdup(linphone_presence_model_get_contact(mPresenceModel));
			}
			linphone_presence_model_set_presentity(mPresenceModel, identityAddress->toC());
			linphone_presence_model_set_contact(mPresenceModel, NULL); /*it will be automatically computed*/
		}

		char *presence_body;
		if (!(presence_body = linphone_presence_model_to_xml(mPresenceModel))) {
			lError() << "Cannot publish presence model [" << mPresenceModel << "] for account [" << this
			         << "] because of xml serialization error";
			return -1;
		}

		if (!mSipEtag.empty()) {
			mPresencePublishEvent->addCustomHeader("SIP-If-Match", mSipEtag);
			mSipEtag = "";
		}

		auto content = Content::create(nullptr, true);
		content->setBody((const uint8_t *)presence_body, strlen(presence_body));
		ContentType contentType("application", "pidf+xml");
		content->setContentType(contentType);

		err = mPresencePublishEvent->send(content);
		ms_free(presence_body);

		if (presentityAddress) {
			lInfo() << "Restoring previous presentity address " << *presentityAddress << " for model ["
			        << mPresenceModel << "]";

			linphone_presence_model_set_presentity(mPresenceModel, presentityAddress->toC());
		}
		if (contact) {
			linphone_presence_model_set_contact(mPresenceModel, contact);
			bctbx_free(contact);
		}
	} else
		setSendPublish(true); /*otherwise do not send publish if registration is in progress, this will be done later*/

	return err;
}

bool Account::check() {
	if (mParams->mProxy.empty()) {
		lWarning() << "No proxy given for account " << this;
		return false;
	}
	if (mParams->mIdentityAddress == NULL) {
		lWarning() << "Identity address of account " << this << " has not been set";
		return false;
	}
	resolveDependencies();
	return true;
}

void Account::releaseOps() {
	if (mOp) {
		mOp->release();
		mOp = nullptr;
	}

	if (mPresencePublishEvent) {
		mPresencePublishEvent->terminate();
		mPresencePublishEvent = nullptr;
	}
}

void Account::resolveDependencies() {
	auto core = getCCore();
	if (!core) return;

	for (auto &account : getCore()->getAccounts()) {
		auto dependency = account->getDependency();
		string dependsOn = account->mParams->mDependsOn;
		auto dependentAccount = getCore()->getAccountByIdKey(dependsOn);
		if (dependency != nullptr && !dependsOn.empty()) {
			if (dependentAccount != nullptr && dependentAccount != dependency) {
				lError() << "Account has a dependency but idkeys do not match: [" << dependsOn << "] != ["
				         << dependency->getAccountParams()->getIdKey() << "], breaking dependency now.";
				account->setDependency(nullptr);
				return;
			} else if (dependentAccount == nullptr) {
				lWarning() << "Account [" << account << "] depends on account [" << dependency
				           << "], which is not currently in the list.";
			}
		}
		if (!dependsOn.empty() && dependency == nullptr) {
			if (dependentAccount == NULL) {
				lWarning() << "Account marked as dependent but no account found for idkey [" << dependsOn << "]";
				return;
			} else {
				lInfo() << "Account [" << account << "] now depends on master Account [" << dependentAccount << "]";
				account->setDependency(dependentAccount);
			}
		}
	}
}

std::shared_ptr<Event> Account::getMwiEvent() const {
	return mMwiEvent;
}

void Account::subscribeToMessageWaitingIndication() {
	const std::shared_ptr<Address> &mwiServerAddress = mParams->getMwiServerAddress();
	if (mwiServerAddress) {
		int expires = linphone_config_get_int(getCore()->getCCore()->config, "sip", "mwi_expires", 86400);
		if (mMwiEvent) mMwiEvent->terminate();
		auto subscribeEvent = new EventSubscribe(getCore(), mParams->getIdentityAddress(), "message-summary", expires);
		subscribeEvent->setRequestAddress(mwiServerAddress);
		mMwiEvent = subscribeEvent->toSharedPtr();
		mMwiEvent->setInternal(true);
		mMwiEvent->addCustomHeader("Accept", "application/simple-message-summary");
		linphone_event_send_subscribe(mMwiEvent->toC(), nullptr);
		mMwiEvent->setUserData(this);
	}
}

void Account::unsubscribeFromMessageWaitingIndication() {
	if (mMwiEvent) {
		mMwiEvent->terminate();
		mMwiEvent = nullptr;
	}
}

// -----------------------------------------------------------------------------

void Account::onInternationalPrefixChanged() {
	try {
		auto core = getCore();
		/* Ensure there is a default account otherwise after invalidating friends maps we won't be able to recompute
		 * phone numbers */
		/* Also it is useless to do it if the account being edited isn't the default one */
		if (core && (core->getDefaultAccount() == getSharedFromThis())) {
			linphone_core_invalidate_friends_maps(getCCore());
		}
	} catch (const bad_weak_ptr &) {
	}
}

void Account::onConferenceFactoryAddressChanged(const std::shared_ptr<Address> &conferenceFactoryAddress) {
	auto core = getCCore();
	if (!core) return;
	std::string conferenceSpec("conference/");
	conferenceSpec.append(Core::conferenceVersionAsString());
	std::string groupchatSpec("groupchat/");
	groupchatSpec.append(Core::groupChatVersionAsString());
	std::string ephemeralSpec("ephemeral/");
	ephemeralSpec.append(Core::ephemeralVersionAsString());

	if (conferenceFactoryAddress && conferenceFactoryAddress->isValid()) {
		linphone_core_add_linphone_spec(core, L_STRING_TO_C(conferenceSpec));
		linphone_core_add_linphone_spec(core, L_STRING_TO_C(groupchatSpec));
		linphone_core_add_linphone_spec(core, L_STRING_TO_C(ephemeralSpec));
	} else {
		bool remove = true;
		bool removeAudioVideoConfAddress = true;
		// Check that no other account needs the specs before removing it
		for (const auto &account : getCore()->getAccounts()) {
			if (account != getSharedFromThis()) {
				const auto &params = account->getAccountParams();
				const auto &confUri = params->getConferenceFactoryAddress();
				if (confUri) {
					remove = false;
					removeAudioVideoConfAddress = false;
					break;
				}
				const auto &audioVideoConfUri = params->getAudioVideoConferenceFactoryAddress();
				if (audioVideoConfUri) {
					removeAudioVideoConfAddress = false;
				}
			}
		}
		if (removeAudioVideoConfAddress) {
			linphone_core_remove_linphone_spec(core, L_STRING_TO_C(conferenceSpec));
		}
		if (remove) {
			linphone_core_remove_linphone_spec(core, L_STRING_TO_C(groupchatSpec));
			linphone_core_remove_linphone_spec(core, L_STRING_TO_C(ephemeralSpec));
		}
	}
}

void Account::onAudioVideoConferenceFactoryAddressChanged(
    const std::shared_ptr<Address> &audioVideoConferenceFactoryAddress) {
	auto core = getCCore();
	if (!core) return;

	std::string conferenceSpec("conference/");
	conferenceSpec.append(Core::conferenceVersionAsString());

	if (audioVideoConferenceFactoryAddress) {
		linphone_core_add_linphone_spec(core, L_STRING_TO_C(conferenceSpec));
	} else {
		bool remove = true;
		// Check that no other account needs the specs before removing it
		for (const auto &account : getCore()->getAccounts()) {
			if (account != getSharedFromThis()) {
				const auto &params = account->getAccountParams();
				const auto &confUri = params->getConferenceFactoryAddress();
				const auto &audioVideoConfUri = params->getAudioVideoConferenceFactoryAddress();
				if (confUri || audioVideoConfUri) {
					remove = false;
					break;
				}
			}
		}
		if (remove) {
			linphone_core_remove_linphone_spec(core, L_STRING_TO_C(conferenceSpec));
		}
	}
}

void Account::onNatPolicyChanged(BCTBX_UNUSED(const std::shared_ptr<NatPolicy> &policy)) {
}

LinphoneProxyConfig *Account::getConfig() const {
	return mConfig;
}

void Account::setConfig(LinphoneProxyConfig *config, bool takeProxyConfigRef) {
	if (mConfig && hasProxyConfigRef) {
		linphone_proxy_config_unref(mConfig);
	}
	mConfig = config;
	hasProxyConfigRef = takeProxyConfigRef;
	if (mConfig && hasProxyConfigRef) {
		linphone_proxy_config_ref(mConfig);
	}
}

LinphoneAccountAddressComparisonResult Account::isServerConfigChanged() {
	return isServerConfigChanged(mOldParams, mParams);
}

void Account::triggerUpdate() {
	try {
		auto account = getSharedFromThis();
		getCore()->doLater([account]() {
			// Avoid updating if the core is not running
			if (const auto *cCore = account->getCCore();
			    cCore != nullptr && linphone_core_get_global_state(cCore) == LinphoneGlobalOn)
				account->update();
		});
	} catch (const bad_weak_ptr &) {
		// Core pointer is null
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Account::onLimeServerUrlChanged(const std::string &limeServerUrl) {
#ifdef HAVE_LIME_X3DH
	auto core = getCCore();
	if (!core) return;
	if (!limeServerUrl.empty()) {
		linphone_core_add_linphone_spec(core, "lime");
	} else {
		// If LIME server URL is still set in the Core, do not remove the spec
		const char *core_lime_server_url = linphone_core_get_lime_x3dh_server_url(core);
		if (core_lime_server_url && strlen(core_lime_server_url)) {
			return;
		}

		bool remove = true;
		// Check that no other account needs the spec before removing it
		for (const auto &account : getCore()->getAccounts()) {
			if (account != getSharedFromThis()) {
				const auto &params = account->getAccountParams();
				const std::string &accountLimeServerUrl = params->getLimeServerUrl();
				if (!accountLimeServerUrl.empty()) {
					remove = false;
					break;
				}
			}
		}
		if (remove) {
			linphone_core_remove_linphone_spec(core, "lime");
		}
	}

	// If the lime server URL has changed, then propagate the change to the encryption engine
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine && (encryptionEngine->getEngineType() == EncryptionEngine::EngineType::LimeX3dh)) {
		auto account = this->getSharedFromThis();
		encryptionEngine->onServerUrlChanged(account, limeServerUrl);
	}

#else
	lWarning() << "Lime X3DH support is not available";
#endif
}

void Account::onMwiServerAddressChanged() {
	if (mState == LinphoneRegistrationOk) {
		subscribeToMessageWaitingIndication();
	}
}

// -----------------------------------------------------------------------------

LinphoneAccountCbsRegistrationStateChangedCb AccountCbs::getRegistrationStateChanged() const {
	return mRegistrationStateChangedCb;
}

void AccountCbs::setRegistrationStateChanged(LinphoneAccountCbsRegistrationStateChangedCb cb) {
	mRegistrationStateChangedCb = cb;
}

LinphoneAccountCbsMessageWaitingIndicationChangedCb AccountCbs::getMessageWaitingIndicationChanged() const {
	return mMessageWaitingIndicationChangedCb;
}

void AccountCbs::setMessageWaitingIndicationChanged(LinphoneAccountCbsMessageWaitingIndicationChangedCb cb) {
	mMessageWaitingIndicationChangedCb = cb;
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

LINPHONE_END_NAMESPACE
