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
		if (getCore()) {
			LinphoneCore *lc = this->getCore()->getCCore();
			if (lc != NULL && linphone_core_get_global_state(lc) != LinphoneGlobalOff) {
				if (mOp) mOp->release();
			}
		}
	} catch (const bad_weak_ptr &) {
	}

	if (mSendCustomHeaders) sal_custom_header_free(mSendCustomHeaders);
	if (mToAddress) linphone_address_unref(mToAddress);
	if (mFromAddress) linphone_address_unref(mFromAddress);
	if (mRemoteContactAddress) linphone_address_unref(mRemoteContactAddress);
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

const LinphoneAddress *Event::getFrom() const {
	return cacheFrom();
}

void Event::setFrom(const LinphoneAddress *fromAddress) {
	if (mFromAddress) {
		linphone_address_unref(mFromAddress);
		mFromAddress = nullptr;
	}
	if (fromAddress) {
		mFromAddress = linphone_address_clone(fromAddress);
	}
}

const LinphoneAddress *Event::getTo() const {
	return cacheTo();
}

void Event::setTo(const LinphoneAddress *toAddress) {
	if (mToAddress) {
		linphone_address_unref(mToAddress);
		mToAddress = nullptr;
	}
	if (toAddress) {
		mToAddress = linphone_address_clone(toAddress);
	}
}

const LinphoneAddress *Event::getResource() const {
	return cacheTo();
}

const LinphoneAddress *Event::cacheFrom() const {
	if (mFromAddress) linphone_address_unref(mFromAddress);
	char *buf = sal_address_as_string(mOp->getFromAddress());
	mFromAddress = linphone_address_new(buf);
	ms_free(buf);
	return mFromAddress;
}

const LinphoneAddress *Event::cacheTo() const {
	if (mToAddress) linphone_address_unref(mToAddress);
	char *buf = sal_address_as_string(mOp->getToAddress());
	mToAddress = linphone_address_new(buf);
	ms_free(buf);
	return mToAddress;
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

bool Event::getUnrefWhenTerminated() const {
	return mUnrefWhenTerminated;
}
void Event::setUnrefWhenTerminated(bool unrefWhenTerminated) {
	mUnrefWhenTerminated = unrefWhenTerminated;
}

LINPHONE_END_NAMESPACE