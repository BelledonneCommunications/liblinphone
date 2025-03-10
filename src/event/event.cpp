/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "event.h"

#include "core/core.h"
#include "core_private.h"
#include "linphone/error_info.h"
#include "private_functions.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Event::Event(const shared_ptr<Core> &core) : CoreAccessor(core) {
	mEi = linphone_error_info_new();
}

Event::~Event() {
	lInfo() << "Destroying event [" << this << "]";

	if (mEi) linphone_error_info_unref(mEi);
	try {
		auto core = getCore();
		if (core) {
			LinphoneCore *lc = core->getCCore();
			if (lc != NULL && linphone_core_get_global_state(lc) != LinphoneGlobalOff) {
				if (mOp) mOp->release();
			}
		}
	} catch (const bad_weak_ptr &) {
	}

	if (mSendCustomHeaders) sal_custom_header_free(mSendCustomHeaders);
}

void Event::fillOpFields() {
	if (!mOp) return;
	auto dir = mOp->getDir();
	auto resourceSalAddress = (dir == SalOp::Dir::Incoming) ? mOp->getToAddress() : mOp->getFromAddress();
	auto accountIdentity = Address::create(resourceSalAddress);
	auto account = getCore()->findAccountByIdentityAddress(accountIdentity);
	if (account) {
		// Set the contact address to avoid putting down a local address
		auto state = account->getState();
		auto contactAddress = account->getContactAddress();
		if (contactAddress && contactAddress->isValid() && (state == LinphoneRegistrationOk)) {
			// setContactAddress clones the SalAddress
			Address contactUri(contactAddress->getUri());
			contactUri.merge(*accountIdentity);
			mOp->setContactAddress(contactUri.getImpl());
		}
		auto realm = account->getAccountParams()->getRealm();
		if (!realm.empty()) {
			mOp->setRealm(L_STRING_TO_C(realm));
		}
	}
}

LinphoneReason Event::getReason() const {
	return linphone_error_info_get_reason(getErrorInfo());
}

const LinphoneErrorInfo *Event::getErrorInfo() const {
	if (!mEi) mEi = linphone_error_info_new();
	linphone_error_info_from_sal_op(mEi, mOp);
	return mEi;
}

void Event::setInternal(bool internal) {
	mInternal = internal;
}
bool Event::isInternal() {
	return mInternal;
}

void Event::addCustomHeader(const string &name, const string &value) {
	mSendCustomHeaders = sal_custom_header_append(mSendCustomHeaders, name.c_str(), value.c_str());
}

void Event::removeCustomHeader(const string &name) {
	mSendCustomHeaders = sal_custom_header_remove(mSendCustomHeaders, name.c_str());
}

const char *Event::getCustomHeaderCstr(const std::string &name) {
	return sal_custom_header_find(mOp->getRecvCustomHeaders(), name.c_str());
}

string Event::getCustomHeader(const std::string &name) {
	return L_C_TO_STRING(getCustomHeaderCstr(name));
}

const string &Event::getName() const {
	return mName;
}

const std::shared_ptr<Address> Event::getFrom() const {
	return cacheFrom();
}

void Event::setFrom(const std::shared_ptr<Address> &fromAddress) {
	mFromAddress = fromAddress->clone()->toSharedPtr();
}

const std::shared_ptr<Address> Event::getTo() const {
	return cacheTo();
}

void Event::setTo(const std::shared_ptr<Address> &toAddress) {
	mToAddress = toAddress->clone()->toSharedPtr();
}

const std::string &Event::getCallId() const {
	return mOp->getCallId();
}

std::shared_ptr<Address> Event::getResource() const {
	return cacheTo();
}

std::shared_ptr<Address> Event::getRequestAddress() const {
	return cacheRequestAddress();
}

void Event::setRequestAddress(const std::shared_ptr<const Address> &requestAddress) {
	mRequestAddress = requestAddress->clone()->toSharedPtr();
}

std::shared_ptr<Address> Event::getRemoteContact() const {
	if (!mRemoteContactAddress) {
		mRemoteContactAddress = Address::create();
	}
	if (mOp) {
		mRemoteContactAddress->setImpl(mOp->getRemoteContactAddress());
	}
	return mRemoteContactAddress;
}

std::shared_ptr<Address> Event::cacheFrom() const {
	if (!mFromAddress) {
		mFromAddress = Address::create();
	}
	if (mOp) {
		mFromAddress->setImpl(mOp->getFromAddress());
	}
	return mFromAddress;
}

std::shared_ptr<Address> Event::cacheTo() const {
	if (!mToAddress) {
		mToAddress = Address::create();
	}
	if (mOp) {
		mToAddress->setImpl(mOp->getToAddress());
	}
	return mToAddress;
}

std::shared_ptr<Address> Event::cacheRequestAddress() const {
	if (!mRequestAddress) {
		mRequestAddress = Address::create();
	}
	if (mOp) {
		mRequestAddress->setImpl(mOp->getRequestAddress());
	}
	return mRequestAddress;
}

void Event::release() {
	try {
		if (mOp) {
			/*this will stop the refresher*/
			mOp->stopRefreshing();
		}
	} catch (const bad_weak_ptr &) {
	}
	if (mUnrefWhenTerminated) {
		unref();
	}
}

LinphonePrivate::SalEventOp *Event::getOp() const {
	return mOp;
}

int Event::getExpires() const {
	return mExpires;
}

void Event::setExpires(int expires) {
	mExpires = expires;
}

void Event::setUnrefWhenTerminated(bool unrefWhenTerminated) {
	mUnrefWhenTerminated = unrefWhenTerminated;
}

void Event::setManualRefresherMode(bool manual) {
	mOp->setManualRefresherMode(manual);
}

LINPHONE_END_NAMESPACE
