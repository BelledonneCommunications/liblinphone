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

#ifndef _L_ACCOUNT_MANAGER_SERVICES_REQUEST_H_
#define _L_ACCOUNT_MANAGER_SERVICES_REQUEST_H_

#include <json/json.h>

#include "account/account-device.h"
#include "belle-sip/object++.hh"
#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "http/http-client.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AccountManagerServicesRequestCbs;
class AccountManagerServicesRequest
    : public bellesip::HybridObject<LinphoneAccountManagerServicesRequest, AccountManagerServicesRequest>,
      public CoreAccessor,
      public CallbacksHolder<AccountManagerServicesRequestCbs> {

public:
	AccountManagerServicesRequest(std::shared_ptr<Core> core,
	                              LinphoneAccountManagerServicesRequestType type,
	                              const std::string &url,
	                              const std::string &httpRequestType,
	                              Json::Value params,
	                              bool testerEnv = false);
	AccountManagerServicesRequest(const AccountManagerServicesRequest &other) = delete;
	virtual ~AccountManagerServicesRequest();

	void submit();

	LinphoneAccountManagerServicesRequestType getType() const {
		return mType;
	}

	const std::string &getFrom() const {
		return mFrom;
	}
	void setFrom(const std::string &from) {
		mFrom = from;
	}

	const std::string &getLanguage() const {
		return mLanguage;
	}
	void setLanguage(const std::string &language) {
		mLanguage = language;
	}

private:
	std::string requestTypeToString(LinphoneAccountManagerServicesRequestType type);
	Json::Value parseResponseAsJson(const HttpResponse &response);
	std::string extractDataFromSuccessfulRequest(const HttpResponse &response);

	void handleSuccess(const HttpResponse &response);
	void handleError(const HttpResponse &response);

	LinphoneAccountManagerServicesRequestType mType;
	std::string mUrl;
	std::string mHttpRequestType;
	Json::Value mJsonParams;
	bool mTesterEnv = false;

	std::string mFrom;
	std::string mLanguage;

	std::list<std::shared_ptr<AccountDevice>> mDevicesList;
};

class AccountManagerServicesRequestCbs
    : public bellesip::HybridObject<LinphoneAccountManagerServicesRequestCbs, AccountManagerServicesRequestCbs>,
      public Callbacks {
public:
	LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb getRequestSuccessful() const;
	void setRequestSuccessful(LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb cb);
	LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb getRequestError() const;
	void setRequestError(LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb cb);
	LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb getDevicesListFetched() const;
	void setDevicesListFetched(LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb cb);

private:
	LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb mRequestSuccessfulCb = nullptr;
	LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb mRequestErrorCb = nullptr;
	LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb mDevicesListFetchedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_MANAGER_SERVICES_REQUEST_H_
