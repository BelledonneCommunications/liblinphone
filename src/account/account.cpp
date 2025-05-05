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

#include <bctoolbox/defs.h>

#include "account.h"

#include "chat/ics/ics.h"
#include "conference/client-conference.h"
#include "content/content.h"
#include "core/core.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/utils/algorithm.h"
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
#include "friend/friend.h"
#include "linphone/core.h"
#include "presence/presence-service.h"
#include "private.h"
#include "utils/custom-params.h"
#include "utils/xml-utils.h"
#include "xml/xcon-ccmp.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::XconCcmp;
using namespace Xsd::XconConferenceInfo;

Account::Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params)
    : CoreAccessor(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr) {
	mParams = params;
	mMissedCalls = 0;
	applyParamsChanges();
	lInfo() << *this << " created with params";
}

Account::Account(LinphoneCore *lc, std::shared_ptr<AccountParams> params, LinphoneProxyConfig *config)
    : Account(lc, params) {
	setConfig(config, false);
	lInfo() << *this << " created with params and proxy config";
}

Account::~Account() {
	lInfo() << *this << " destroyed";
	cancelDeletion();
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
	mPreviousPublishParamsHash[0] = strtoull(hash, (char **)nullptr, 16);
	hash[16] = saved;
	mPreviousPublishParamsHash[1] = strtoull(&hash[16], (char **)nullptr, 16);
	return previous_hash[0] != mPreviousPublishParamsHash[0] || previous_hash[1] != mPreviousPublishParamsHash[1];
}

LinphoneAccountAddressComparisonResult Account::compareLinphoneAddresses(const std::shared_ptr<const Address> &a,
                                                                         const std::shared_ptr<const Address> &b) {
	if (a == nullptr && b == nullptr) return LinphoneAccountAddressEqual;
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
	    oldParams != nullptr && !oldParams->mProxy.empty() ? Address::create(oldParams->mProxy) : nullptr;
	std::shared_ptr<Address> newProxy = !newParams->mProxy.empty() ? Address::create(newParams->mProxy) : nullptr;
	LinphoneAccountAddressComparisonResult result_identity;
	LinphoneAccountAddressComparisonResult result;

	result = compareLinphoneAddresses(oldParams != nullptr ? oldParams->mIdentityAddress : nullptr,
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
	}
	if (mOldParams == nullptr || mOldParams->mLimeAlgo != mParams->mLimeAlgo) {
		onLimeAlgoChanged(mParams->mLimeAlgo);
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

void Account::triggerDeletion() {
	cancelDeletion();
	weak_ptr<Account> weakZis = getSharedFromThis();
	// Timeout is in ms
	unsigned int accountDeletionTimeout = getCore()->getAccountDeletionTimeout() * 1000;
	mDeletionTimer = getCore()->createTimer(
	    [weakZis]() -> bool {
		    auto zis = weakZis.lock();
		    if (zis) {
			    try {
				    zis->getCore()->removeDeletedAccount(zis);
			    } catch (const bad_weak_ptr &) {
				    // ignored
			    }
		    }
		    return false;
	    },
	    accountDeletionTimeout, "Account deletion");
}

void Account::cancelDeletion() {
	if (mDeletionTimer) {
		try {
			getCore()->destroyTimer(mDeletionTimer);
		} catch (const bad_weak_ptr &) {
			// ignored.
		}
		mDeletionTimer = nullptr;
	}
}

bool Account::deletionPending() const {
	return mDeletionTimer != nullptr;
}

void Account::setSipEtag(const std::string &sipEtag) {
	mSipEtag = sipEtag;
}

void Account::setErrorInfo(LinphoneErrorInfo *errorInfo) {
	mErrorInfo = errorInfo;
}

void Account::setContactAddress(const std::shared_ptr<const Address> &contact) {
	mContactAddress = nullptr;
	if (contact) {
		mContactAddress = contact->clone()->toSharedPtr();
	}
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

void Account::handleDeletion() {
	switch (mState) {
		case LinphoneRegistrationOk:
			unregister();
			break;
		case LinphoneRegistrationCleared:
			cancelDeletion();
			getCore()->removeDeletedAccount(getSharedFromThis());
			break;
		case LinphoneRegistrationNone:
		default:
			// In all other states, un-registration is aborted.
			setState(LinphoneRegistrationNone, "Registration disabled");
			break;
	}
}

void Account::setState(LinphoneRegistrationState state, const std::string &message) {
	/*allow multiple notification of LinphoneRegistrationOk for refreshing*/
	if ((mState != state) || (state == LinphoneRegistrationOk)) {
		auto core = getCCore();
		const auto identity = (mParams) ? mParams->getIdentityAddress()->toString() : std::string("sip:");
		if (!mParams) lWarning() << "AccountParams not set for " << *this;
		lInfo() << *this << " moving from state [" << linphone_registration_state_to_string(mState) << "] to ["
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

		mPreviousState = mState;
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
		    mPreviousState != state) {
			linphone_core_update_friends_subscriptions(core);
		}
		if (state == LinphoneRegistrationOk && mPreviousState != state) {
			subscribeToMessageWaitingIndication();
		}

		triggerUpdate();

		if (deletionPending()) {
			handleDeletion();
		}

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
				lError() << "Couldn't compute automatic push notifications parameters on " << *this
				         << " because account params do not have available push notifications";
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
				for (const auto &pushParam : newParams->mPushNotificationConfig->getPushParamsMap()) {
					string paramName = pushParam.first;
					if (!contactParamsWrapper->getUriParamValue(paramName).empty()) {
						contactParamsWrapper->removeUriParam(paramName);
						didRemoveParams = true;
						lError() << "Removing '" << paramName << "' from " << *this
						         << " contact uri parameters because it will be generated automatically since core "
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

					lWarning() << *this << " contact uri parameters changed from '" << newParams->mContactUriParameters
					           << "' to '" << newContactUriParams << "'";
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
			        << "' added to " << *this;
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
			lError() << "Can't register " << *this << " without a proxy address";
			return;
		}
		if (mParams->mIdentityAddress == nullptr) {
			lError() << "Can't register " << *this << " without an identity address";
			return;
		}
		lInfo() << *this << " about to register (LinphoneCore version: " << linphone_core_get_version() << ")";

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
	}
}

void Account::refreshRegister() {
	if (!mParams) {
		lWarning() << "refreshRegister is called but no AccountParams is set on " << *this;
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
	if (mOp) {
		if (mState == LinphoneRegistrationOk || mState == LinphoneRegistrationFailed) {
			unsubscribeFromChatRooms();
			unsubscribeFromMessageWaitingIndication();
			lInfo() << *this << " unregistering.";
			mOp->unregister();
			mIsUnregistering = true;
		}
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
		lWarning() << "isAvpfEnabled is called but no AccountParams is set on " << *this;
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
		lWarning() << "findAuthInfo is called but no AccountParams is set on " << *this;
		return nullptr;
	}

	const std::string username = mParams->mIdentityAddress ? mParams->mIdentityAddress->getUsername() : std::string();
	const std::string domain = mParams->mIdentityAddress ? mParams->mIdentityAddress->getDomain() : std::string();
	return linphone_core_find_auth_info(getCCore(), mParams->mRealm.c_str(), username.c_str(), domain.c_str());
}

int Account::getUnreadChatMessageCount() const {
	if (!mParams) {
		lWarning() << "getUnreadMessageCount is called but no AccountParams is set on " << *this;
		return -1;
	}

	return getCore()->getUnreadChatMessageCount(mParams->mIdentityAddress);
}

void Account::unsubscribeFromChatRooms() {
	// Unsubscribe from chatrooms
	lInfo() << "Unsubscribing from a chatrooms linked to account " << *this;
	for (const auto &chatRoom : getChatRooms()) {
		const auto &conference = chatRoom->getConference();
		if (conference) {
			const auto &clientConference = dynamic_pointer_cast<LinphonePrivate::ClientConference>(conference);
			if (clientConference) {
				clientConference->unsubscribe();
			}
		}
	}
	mChatRoomList.mList.clear();
}

void Account::updateChatRoomList() const {
	list<shared_ptr<AbstractChatRoom>> results;
	if (!mParams) {
		lWarning() << "getChatRooms is called but no AccountParams is set on " << *this;
		mChatRoomList.mList.clear();
		return;
	}

	auto localAddress = mParams->mIdentityAddress;
	const list<shared_ptr<AbstractChatRoom>> chatRooms = getCore()->getChatRooms();
	for (const auto &chatRoom : chatRooms) {
		if (localAddress->weakEqual(chatRoom->getLocalAddress())) {
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

list<shared_ptr<AbstractChatRoom>> Account::filterChatRooms(const string &filter) const {
	updateChatRoomList();
	if (filter.empty()) {
		return mChatRoomList.mList;
	}

	list<shared_ptr<AbstractChatRoom>> oneToOneResults;
	list<shared_ptr<AbstractChatRoom>> groupResults;
	const auto &friendLists = getCore()->getFriendLists();

	for (const auto &chatRoom : mChatRoomList.mList) {
		auto capabilities = chatRoom->getCapabilities();
		if (capabilities & ChatRoom::Capabilities::Basic) {
			auto peerAddress = chatRoom->getPeerAddress();
			if (Utils::containsInsensitive(peerAddress->asStringUriOnly(), filter)) {
				oneToOneResults.push_back(chatRoom);
				continue;
			}

			for (const auto &friendList : friendLists) {
				const auto buddy = friendList->findFriendByAddress(peerAddress);
				if (buddy != nullptr && Utils::containsInsensitive(buddy->getName(), filter)) {
					oneToOneResults.push_back(chatRoom);
					break;
				}
			}
		} else {
			bool oneToOne = capabilities & ChatRoom::Capabilities::OneToOne;
			if (!oneToOne) {
				const string &subject = chatRoom->getSubject();
				if (Utils::containsInsensitive(subject, filter)) {
					groupResults.push_back(chatRoom);
					continue;
				}
			}

			for (const auto &participant : chatRoom->getParticipants()) {
				const auto peerAddress = participant->getAddress();
				if (Utils::containsInsensitive(peerAddress->asStringUriOnly(), filter)) {
					if (oneToOne) {
						oneToOneResults.push_back(chatRoom);
					} else {
						groupResults.push_back(chatRoom);
					}
					break;
				}

				for (const auto &friendList : friendLists) {
					const auto buddy = friendList->findFriendByAddress(peerAddress);
					if (buddy != nullptr && Utils::containsInsensitive(buddy->getName(), filter)) {
						if (oneToOne) {
							oneToOneResults.push_back(chatRoom);
						} else {
							groupResults.push_back(chatRoom);
						}
						break;
					}
				}
			}
		}
	}

	oneToOneResults.splice(oneToOneResults.end(), groupResults);
	return oneToOneResults;
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
		lWarning() << "getCallLogs is called but no AccountParams is set on " << *this;
		return {};
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
		lWarning() << "getCallLogsForAddress is called but no AccountParams is set on " << *this;
		return {};
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
		lWarning() << "deleteCallLogs is called but no AccountParams is set on " << *this;
		return;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "deleteCallLogs is called but core is not running on " << *this;
		return;
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	mainDb->deleteCallHistoryForLocalAddress(localAddress);
}

list<shared_ptr<ConferenceInfo>> Account::getConferenceInfos(const std::list<LinphoneStreamType> capabilities) const {
	if (!mParams) {
		lWarning() << "getConferenceInfos is called but no AccountParams is set on " << *this;
		return {};
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn) {
		lWarning() << "getConferenceInfos is called but core is not running on Account [" << this << "]";
		return {};
	}

	auto localAddress = mParams->mIdentityAddress;
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	mConferenceInfos = mainDb->getConferenceInfosWithParticipant(localAddress, capabilities);

	const auto ccmpServerUrl = mParams->getCcmpServerUrl();
	if (!ccmpServerUrl.empty()) {
		updateConferenceInfoListWithCcmp();
	}
	// At first, the list of locally stored conferences is returned.
	// The updated list will be available upon receiving the notification that the conference information list has been
	// updated (callback: conference_information_updated)
	return mConferenceInfos;
}

void Account::addConferenceInfo(const std::shared_ptr<ConferenceInfo> &info) {
	auto it = std::find_if(mConferenceInfos.begin(), mConferenceInfos.end(),
	                       [&info](const auto &cachedInfo) { return *info->getUri() == *cachedInfo->getUri(); });
	if (it != mConferenceInfos.end()) {
		mConferenceInfos.erase(it);
	}
	mConferenceInfos.push_back(info);
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

	snprintf(key, sizeof(key), "proxy_%i", index);
	linphone_config_clean_section(config, key);
	if (account == nullptr) {
		return;
	}

	account->writeToConfigFile(index);
}

void Account::writeToConfigFile(int index) {
	if (!mParams) {
		lWarning() << "writeToConfigFile is called but no AccountParams is set on " << *this;
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
	if (getState() == LinphoneRegistrationNone && deletionPending()) {
		// Account is removed, but never registered before.
		// This case happens when doing a removeAccount() while network is off.
		return false;
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
				mOp->setUserPointer(nullptr); /*we don't want to receive status for this un register*/
				mOp->unref();                 /*but we keep refresher to handle authentication if needed*/
				mOp = nullptr;
			}
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

	// if (mNeedToRegister) {
	//	pauseRegister();
	// }

	if (computePublishParamsHash()) {
		lInfo() << "Publish params have changed on " << *this;

		if (mPresencePublishEvent) {
			/*publish is terminated*/
			mPresencePublishEvent->terminate();
		}
		if (mParams->mPublishEnabled) setSendPublish(true);
	} else {
		lInfo() << "Publish params have not changed on " << *this;
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
			shared_ptr<const Address> sip = getAccountParams()->getIdentityAddress();
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

shared_ptr<EventPublish> Account::createPublish(const std::string &event, int expires) {
	if (!getCore()) {
		lError() << "Cannot create publish from " << *this << " not attached to any core";
		return nullptr;
	}
	return dynamic_pointer_cast<EventPublish>(
	    (new EventPublish(getCore(), getSharedFromThis(), nullptr, event, expires))->toSharedPtr());
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
		mPresencePublishEvent->setUserData(this);

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

		if (linphone_presence_model_get_presentity(mPresenceModel) == nullptr) {
			lInfo() << "No presentity set for model [" << mPresenceModel << "], using identity from account [" << this
			        << "]: " << *identityAddress;
			linphone_presence_model_set_presentity(mPresenceModel, identityAddress->toC());
		}

		const auto currentPresentity = linphone_presence_model_get_presentity(mPresenceModel);
		std::shared_ptr<const Address> presentityAddress = nullptr;
		char *contact = nullptr;
		if (!linphone_address_equal(currentPresentity, identityAddress->toC())) {
			lInfo() << "Presentity for model [" << mPresenceModel << "] differs account [" << this
			        << "], using account " << *identityAddress;
			presentityAddress = Address::getSharedFromThis(currentPresentity); /*saved, just in case*/
			if (linphone_presence_model_get_contact(mPresenceModel)) {
				contact = bctbx_strdup(linphone_presence_model_get_contact(mPresenceModel));
			}
			linphone_presence_model_set_presentity(mPresenceModel, identityAddress->toC());
			linphone_presence_model_set_contact(mPresenceModel, nullptr); /*it will be automatically computed*/
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
		lWarning() << "No proxy given for " << *this;
		return false;
	}
	if (mParams->mIdentityAddress == nullptr) {
		lWarning() << "Identity address of " << *this << " has not been set";
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
			if (dependentAccount == nullptr) {
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
	std::shared_ptr<const Address> mwiServerAddress = mParams->getMwiServerAddress();
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
		return;
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
void Account::onLimeAlgoChanged(const std::string &limeAlgo) {
#ifdef HAVE_LIME_X3DH
	auto core = getCCore();
	if (!core) return;
	if (!limeAlgo.empty()) {
		linphone_core_add_linphone_spec(core, "lime");
	} else {
		// If LIME algo is still set in the Core, do not remove the spec
		const char *core_lime_algo = linphone_config_get_string(core->config, "lime", "curve", "");
		if (core_lime_algo && strlen(core_lime_algo)) {
			return;
		}

		bool remove = true;
		// Check that no other account needs the spec before removing it
		for (const auto &account : getCore()->getAccounts()) {
			if (account != getSharedFromThis()) {
				const auto &params = account->getAccountParams();
				const std::string &accountLimeAlgo = params->getLimeAlgo();
				if (!accountLimeAlgo.empty()) {
					remove = false;
					break;
				}
			}
		}
		if (remove) {
			linphone_core_remove_linphone_spec(core, "lime");
		}
		return;
	}

	// the lime algo has changed, propagate the change to the encryption engine
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine && (encryptionEngine->getEngineType() == EncryptionEngine::EngineType::LimeX3dh)) {
		// just call the create lime user function, it will get the info from the account
		shared_ptr<Address> addr = mContactAddress;
		if (!addr) {
			auto sip = getAccountParams()->getIdentityAddress();
			if (sip) {
				auto gr = getCCore()->sal->getUuid();
				if (gr.empty()) return;
				addr = sip->clone()->toSharedPtr();
				addr->setUriParam("gr", "urn:uuid:" + gr);
			}
		}
		auto account = this->getSharedFromThis();
		if (addr) encryptionEngine->createLimeUser(account, addr->asStringUriOnly());
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

void Account::ccmpConferenceInformationRequestSent() {
	mCcmpConferenceInformationRequestsCounter++;
}

void Account::ccmpConferenceInformationResponseReceived() {
	if (mCcmpConferenceInformationRequestsCounter == 0) {
		lFatal() << *this << " Receiving more responses than HTTP requests sent";
	}
	mCcmpConferenceInformationRequestsCounter--;
	if (mCcmpConferenceInformationRequestsCounter == 0) {
		// Notify the application that all responses have been received and therefore the core holds a list of
		// conference informations that is up to date for this account
		auto infosCList = Utils::listToCBctbxList<LinphoneConferenceInfo, ConferenceInfo>(mConferenceInfos);
		_linphone_account_notify_conference_information_updated(toC(), infosCList);
		bctbx_list_free(infosCList);
	}
}

void Account::handleCCMPResponseConferenceList(const HttpResponse &response) {
	switch (response.getStatus()) {
		case HttpResponse::Status::Valid:
			handleResponseConferenceList(this, response);
			break;
		case HttpResponse::Status::Timeout:
			handleTimeoutConferenceList(this, response);
			break;
		case HttpResponse::Status::IOError:
		case HttpResponse::Status::InvalidRequest:
			handleIoErrorConferenceList(this, response);
			break;
	}
}

void Account::handleCCMPResponseConferenceInformation(const HttpResponse &response) {
	switch (response.getStatus()) {
		case HttpResponse::Status::Valid:
			handleResponseConferenceInformation(this, response);
			break;
		case HttpResponse::Status::Timeout:
			handleTimeoutConferenceInformation(this, response);
			break;
		case HttpResponse::Status::IOError:
		case HttpResponse::Status::InvalidRequest:
			handleIoErrorConferenceInformation(this, response);
			break;
	}
}

void Account::updateConferenceInfoListWithCcmp() const {
	const auto ccmpServerUrl = mParams->getCcmpServerUrl();
	if (ccmpServerUrl.empty()) {
		lError() << "Unable to get the list of conferences on an unknown CCMP server where account [" << this
		         << "] is a participant";
		return;
	}

	ConfsRequestType confsRequest = ConfsRequestType();
	CcmpConfsRequestMessageType requestBody = CcmpConfsRequestMessageType(confsRequest);
	const auto &identity = mParams->getIdentityAddress();
	std::string identityXconUserId = Utils::getXconId(identity);
	if (identityXconUserId.empty()) {
		lError() << "Aborting creation of body of POST to request the list of conferences on the CCMP server "
		         << ccmpServerUrl << " where account [" << this
		         << "] is a participant because its CCMP User ID is unknwon";
		return;
	}
	requestBody.setConfUserID(identityXconUserId);

	stringstream httpBody;
	Xsd::XmlSchema::NamespaceInfomap map;
	map["conference-info"].name = "urn:ietf:params:xml:ns:conference-info";
	map["xcon-conference-info"].name = "urn:ietf:params:xml:ns:xcon-conference-info";
	map["xcon-ccmp"].name = "urn:ietf:params:xml:ns:xcon-ccmp";
	serializeCcmpRequest(httpBody, requestBody, map);
	const auto body = httpBody.str();

	// Send request to retrieve the list of all conferences on the CCMP server
	auto *weakThis = const_cast<Account *>(this);
	if (!XmlUtils::sendCcmpRequest(getCore(), ccmpServerUrl, identity, body, [weakThis](const HttpResponse &response) {
		    weakThis->handleCCMPResponseConferenceList(response);
	    })) {
		lError() << "An error occurred when requesting informations list linked to " << *this << " to server "
		         << ccmpServerUrl;
	}
}

void Account::handleResponseConferenceList(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	int code = event.getHttpStatusCode();
	std::shared_ptr<Address> conferenceAddress;
	if (code >= 200 && code < 300) {
		const auto &body = event.getBody();
		auto content = body.getBodyAsString();
		if (!content.empty()) {
			try {
				istringstream data(content);
				auto responseType = parseCcmpResponse(data, Xsd::XmlSchema::Flags::dont_validate);
				auto &response = dynamic_cast<CcmpConfsResponseMessageType &>(responseType->getCcmpResponse());
				const auto responseCodeType = response.getResponseCode();
				code = static_cast<int>(responseCodeType);
				if (code >= 200 && code < 300) {
					auto &confsResponse = response.getConfsResponse();
					auto &confsInfo = confsResponse.getConfsInfo();
					if (!confsInfo.present()) return;
					auto &infos = confsInfo->getEntry();
					// For every conference that has been retrieved, send one more request to get all the details
					for (auto &info : infos) {
						ConfRequestType confRequest = ConfRequestType();
						auto confObjId = info.getUri();

						CcmpConfRequestMessageType requestBody = CcmpConfRequestMessageType(confRequest);
						// CCMP URI (conference object ID) if update or delete
						if (!confObjId.empty()) {
							requestBody.setConfObjID(confObjId);
						}

						// Conference user ID
						const auto &accountParams = account->getAccountParams();
						const auto &identity = accountParams->getIdentityAddress();
						const auto ccmpServerUrl = accountParams->getCcmpServerUrl();
						std::string identityXconUserId = Utils::getXconId(identity);
						if (identityXconUserId.empty()) {
							lError() << "Aborting creation of body of POST to request the list of conferences on "
							            "the CCMP server "
							         << ccmpServerUrl << " where account [" << account
							         << "] is a participant because its CCMP User ID is unknwon";
							return;
						}
						requestBody.setConfUserID(identityXconUserId);
						requestBody.setOperation(OperationType::retrieve);

						stringstream httpBody;
						Xsd::XmlSchema::NamespaceInfomap map;
						map["conference-info"].name = "urn:ietf:params:xml:ns:conference-info";
						map["xcon-conference-info"].name = "urn:ietf:params:xml:ns:xcon-conference-info";
						map["xcon-ccmp"].name = "urn:ietf:params:xml:ns:xcon-ccmp";
						serializeCcmpRequest(httpBody, requestBody, map);
						const auto stringBody = httpBody.str();

						auto weakThis = account;
						if (XmlUtils::sendCcmpRequest(account->getCore(), ccmpServerUrl, identity, stringBody,
						                              [weakThis](const HttpResponse &response) {
							                              weakThis->handleCCMPResponseConferenceInformation(response);
						                              })) {
							account->ccmpConferenceInformationRequestSent();
						} else {
							lError() << "An error occurred when requesting informations of conference " << confObjId
							         << " linked to Account [" << account << "] (" << *identity << ") to server "
							         << ccmpServerUrl;
						}
					}
				}
			} catch (const std::bad_cast &e) {
				lError() << "Error while casting parsed CCMP response (CcmpConfsResponseMessageType) in " << *account
				         << ": " << e.what();
			} catch (const exception &e) {
				lError() << "Error while parsing CCMP response (CcmpConfsResponseMessageType) in " << *account << ": "
				         << e.what();
			}
		}
	}
}

void Account::handleIoErrorConferenceList(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	const auto ccmpServerUrl = account->getAccountParams()->getCcmpServerUrl();
	const auto &body = event.getBody();
	auto content = body.getBodyAsString();
	lInfo() << "I/O error on retrieving the conference list " << *account << " belongs to from the CCMP server "
	        << ccmpServerUrl << ": " << content;
}

void Account::handleTimeoutConferenceList(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	const auto ccmpServerUrl = account->getAccountParams()->getCcmpServerUrl();
	const auto &body = event.getBody();
	auto content = body.getBodyAsString();
	lInfo() << "Timeout error on retrieving the conference list " << *account << " belongs to from the CCMP server "
	        << ccmpServerUrl << ": " << content;
}

void Account::handleResponseConferenceInformation(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	int code = event.getHttpStatusCode();
	std::shared_ptr<Address> conferenceAddress;
	if (code >= 200 && code < 300) {
		const auto &body = event.getBody();
		auto content = body.getBodyAsString();
		if (!content.empty()) {
			try {
				istringstream data(content);
				auto responseType = parseCcmpResponse(data, Xsd::XmlSchema::Flags::dont_validate);
				auto &response = dynamic_cast<CcmpConfResponseMessageType &>(responseType->getCcmpResponse());
				const auto responseCodeType = response.getResponseCode();
				code = static_cast<int>(responseCodeType);
				if (code >= 200 && code < 300) {
					auto &confResponse = response.getConfResponse();
					auto &confInfo = confResponse.getConfInfo();
					if (!confInfo.present()) return;
					std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
					info->setCcmpUri(confInfo->getEntity());
					auto &confDescription = confInfo->getConferenceDescription();
					if (confDescription.present()) {
						auto &description = confDescription->getFreeText();
						if (description.present() && !description.get().empty()) {
							info->setDescription(description.get());
						}
						auto &subject = confDescription->getSubject();
						if (subject.present() && !subject.get().empty()) {
							info->setSubject(subject.get());
						}
						auto &confUris = confDescription->getConfUris();
						auto &uris = confUris->getEntry();
						for (auto &uri : uris) {
							info->setUri(Address::create(uri.getUri()));
						}
						const auto &availableMedia = confDescription->getAvailableMedia();
						if (availableMedia.present()) {
							for (auto &mediaEntry : availableMedia->getEntry()) {
								const std::string &mediaType = mediaEntry.getType();
								const LinphoneMediaDirection mediaDirection =
								    XmlUtils::mediaStatusToMediaDirection(mediaEntry.getStatus().get());
								LinphoneStreamType streamType = LinphoneStreamTypeUnknown;
								if (mediaType == "audio") {
									streamType = LinphoneStreamTypeAudio;
								} else if (mediaType == "video") {
									streamType = LinphoneStreamTypeVideo;
								} else if (mediaType == "text") {
									streamType = LinphoneStreamTypeText;
								} else {
									lError() << "Unrecognized media type " << mediaType;
								}
								info->setCapability(streamType, (mediaDirection == LinphoneMediaDirectionSendRecv));
							}
						}
						auto &anySequence(confDescription->getAny());
						for (const auto &anyElement : anySequence) {
							auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
							auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
							auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
							if (nodeName == "xcon:conference-time") {
								ConferenceTimeType conferenceTime{anyElement};
								for (auto &conferenceTimeEntry : conferenceTime.getEntry()) {
									const std::string base = conferenceTimeEntry.getBase();
									if (!base.empty()) {
										auto ics = Ics::Icalendar::createFromString(base);
										if (ics) {
											auto timeInfo = ics->toConferenceInfo();
											info->setDateTime(timeInfo->getDateTime());
											info->setDuration(timeInfo->getDuration());
										}
									}
								}
							}
						}
					}
					auto &users = confInfo->getUsers();
					if (users.present()) {
						auto &anySequence(users.get().getAny());
						for (const auto &anyElement : anySequence) {
							auto name = xsd::cxx::xml::transcode<char>(anyElement.getLocalName());
							auto nodeName = xsd::cxx::xml::transcode<char>(anyElement.getNodeName());
							auto nodeValue = xsd::cxx::xml::transcode<char>(anyElement.getNodeValue());
							if (nodeName == "xcon:allowed-users-list") {
								ConferenceInfo::participant_list_t participantInfos;
								AllowedUsersListType allowedUserList{anyElement};
								auto allowedUsers = allowedUserList.getTarget();
								for (auto &user : allowedUsers) {
									auto participantInfo = ParticipantInfo::create(user.getUri());
									participantInfos.push_back(participantInfo);
								}
								info->setParticipants(participantInfos);
							}
						}
						for (auto &user : users->getUser()) {
							auto &roles = user.getRoles();
							bool isOrganizer = false;
							auto ccmpUri = user.getEntity().get();
							auto participantInfo = info->findParticipant(ccmpUri);
							if (roles) {
								auto &entry = roles->getEntry();
								isOrganizer = (find(entry, "organizer") != entry.end() ? true : false);
							}
							// The server CCMP sends responses using the XCON-USERID and, unfortunately, they are
							// generated by the CCMP server and the SDK has no knowledge of them. RFC4475
							// fortunately details the tag <associated-aors> that can be used to communicate
							// additional URIs that should be associated to the entity attribute. For example the
							// following XML code will allow to associated the participant with address
							// sip:bob@sip.example.org to the XCON-USERID bob:
							//<user entity="xcon-userid:bob">
							//  <associated-aors>
							//    <entry>
							//      <uri>sip:bob@sip.example.com</uri>
							//    </entry>
							//   </associated-aors>
							//</user>
							auto &associatedAors = user.getAssociatedAors();
							std::shared_ptr<Address> address;
							if (associatedAors.present()) {
								for (auto &aor : associatedAors->getEntry()) {
									auto tmpAddress = Address::create(aor.getUri());
									if (tmpAddress) {
										address = tmpAddress;
									}
								}
							}
							decltype(participantInfo) updatedParticipantInfo;
							if (participantInfo) {
								updatedParticipantInfo = participantInfo->clone()->toSharedPtr();
							} else {
								updatedParticipantInfo = ParticipantInfo::create(ccmpUri);
							}
							if (address) {
								updatedParticipantInfo->setAddress(address);
							}
							if (participantInfo) {
								info->updateParticipant(updatedParticipantInfo);
							} else {
								info->addParticipant(updatedParticipantInfo);
							}
							if (isOrganizer) {
								info->setOrganizer(updatedParticipantInfo);
							}
						}
					}
					account->addConferenceInfo(info);
#ifdef HAVE_DB_STORAGE
					auto &mainDb = account->getCore()->getPrivate()->mainDb;
					if (mainDb) {
						mainDb->insertConferenceInfo(info);
					}
#endif // HAVE_DB_STORAGE
				}
			} catch (const std::bad_cast &e) {
				lError() << "Error while casting parsed CCMP response (CcmpConfResponseMessageType) in account ["
				         << *account << ": " << e.what();
			} catch (const exception &e) {
				lError() << "Error while parsing CCMP response (CcmpConfResponseMessageType) in " << *account << ": "
				         << e.what();
			}
		}
	}
	account->ccmpConferenceInformationResponseReceived();
}

void Account::handleIoErrorConferenceInformation(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	const auto ccmpServerUrl = account->getAccountParams()->getCcmpServerUrl();
	const auto &body = event.getBody();
	auto content = body.getBodyAsString();
	lInfo() << *account << ": I/O error on retrieving the information of a conference from the CCMP server "
	        << ccmpServerUrl << ": " << content;
	account->ccmpConferenceInformationResponseReceived();
}

void Account::handleTimeoutConferenceInformation(void *ctx, const HttpResponse &event) {
	auto account = static_cast<Account *>(ctx);
	const auto ccmpServerUrl = account->getAccountParams()->getCcmpServerUrl();
	const auto &body = event.getBody();
	auto content = body.getBodyAsString();
	lInfo() << *account << ": Timeout error on retrieving the information of a conference from the CCMP server "
	        << ccmpServerUrl << ": " << content;
	account->ccmpConferenceInformationResponseReceived();
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

LinphoneAccountCbsConferenceInformationUpdatedCb AccountCbs::getConferenceInformationUpdated() const {
	return mConferenceInformationUpdatedCb;
}

void AccountCbs::setConferenceInformationUpdated(LinphoneAccountCbsConferenceInformationUpdatedCb cb) {
	mConferenceInformationUpdatedCb = cb;
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

LINPHONE_END_NAMESPACE
