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

#include "account-manager-services-request.h"
#include "c-wrapper/internal/c-tools.h"
#include "dictionary/dictionary.h"
#include "linphone/api/c-account-manager-services-request-cbs.h"
#include "linphone/enums/c-enums.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AccountManagerServicesRequest::AccountManagerServicesRequest(shared_ptr<Core> core,
                                                             LinphoneAccountManagerServicesRequestType type,
                                                             const string &url,
                                                             const string &httpRequestType,
                                                             Json::Value params,
                                                             bool testerEnv)
    : CoreAccessor(core) {
	mType = type;
	mUrl = url;
	mHttpRequestType = httpRequestType;
	mJsonParams = params;
	mTesterEnv = testerEnv;
}

AccountManagerServicesRequest::~AccountManagerServicesRequest() {
}

// =============================================================================

void AccountManagerServicesRequest::submit() {
	try {
		auto &httpRequest =
		    getCore()->getHttpClient().createRequest(mHttpRequestType, mUrl).addHeader("Accept", "application/json");

		if (!mFrom.empty()) {
			httpRequest.addHeader("From", mFrom);
		} else if (mTesterEnv) {
			lWarning() << "[Account Manager Services] Tester env flag set, altering headers!";
			httpRequest.addHeader("From", "sip:admin_test@sip.example.org");
			httpRequest.addHeader("x-api-key", "no_secret_at_all");
		}

		if (!mLanguage.empty()) {
			httpRequest.addHeader("Accept-Language", mLanguage);
		}

		if (!mJsonParams.empty()) {
			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";
			auto content =
			    Content(ContentType("application/json"), std::string(Json::writeString(builder, mJsonParams)));
			httpRequest.setBody(content);
		}

		auto request = getSharedFromThis();
		httpRequest.execute([request](const HttpResponse &response) {
			try {
				int code = response.getHttpStatusCode();
				if (code >= 200 && code < 300) {
					request->handleSuccess(response);
				} else {
					request->handleError(response);
				}
			} catch (const std::exception &e) {
				lError() << e.what();
			}
		});
	} catch (const std::invalid_argument &e) {
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(AccountManagerServicesRequest, this,
		                                  linphone_account_manager_services_request_cbs_get_request_error, 0, e.what(),
		                                  nullptr);
	}
}

// =============================================================================

LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb
AccountManagerServicesRequestCbs::getRequestSuccessful() const {
	return mRequestSuccessfulCb;
}

void AccountManagerServicesRequestCbs::setRequestSuccessful(
    LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb cb) {
	mRequestSuccessfulCb = cb;
}

LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb AccountManagerServicesRequestCbs::getRequestError() const {
	return mRequestErrorCb;
}

void AccountManagerServicesRequestCbs::setRequestError(LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb cb) {
	mRequestErrorCb = cb;
}

LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb
AccountManagerServicesRequestCbs::getDevicesListFetched() const {
	return mDevicesListFetchedCb;
}

void AccountManagerServicesRequestCbs::setDevicesListFetched(
    LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb cb) {
	mDevicesListFetchedCb = cb;
}

// =============================================================================

string AccountManagerServicesRequest::requestTypeToString(LinphoneAccountManagerServicesRequestType type) {
	switch (type) {
		case LinphoneAccountManagerServicesRequestTypeSendAccountCreationTokenByPush:
			return "SendAccountCreationTokenByPush";
		case LinphoneAccountManagerServicesRequestTypeSendAccountRecoveryTokenByPush:
			return "SendAccountRecoveryTokenByPush";
		case LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken:
			return "RequestAccountCreationRequestToken";
		case LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken:
			return "GetAccountCreationTokenFromRequestToken";
		case LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken:
			return "CreateAccountUsingToken";
		case LinphoneAccountManagerServicesRequestTypeSendPhoneNumberLinkingCodeBySms:
			return "SendPhoneNumberLinkingCodeBySms";
		case LinphoneAccountManagerServicesRequestTypeLinkPhoneNumberUsingCode:
			return "LinkPhoneNumberUsingCode";
		case LinphoneAccountManagerServicesRequestTypeSendEmailLinkingCodeByEmail:
			return "SendEmailLinkingCodeByEmail";
		case LinphoneAccountManagerServicesRequestTypeLinkEmailUsingCode:
			return "LinkEmailUsingCode";
		case LinphoneAccountManagerServicesRequestTypeGetDevicesList:
			return "GetDevicesList";
		case LinphoneAccountManagerServicesRequestTypeDeleteDevice:
			return "DeleteDevice";
		case LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin:
			return "GetCreationTokenAsAdmin";
		case LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin:
			return "GetAccountInfoAsAdmin";
		case LinphoneAccountManagerServicesRequestTypeDeleteAccountAsAdmin:
			return "DeleteAccountAsAdmin";
	}
	return string("Unexpected: " + std::to_string((int)type));
}

Json::Value AccountManagerServicesRequest::parseResponseAsJson(const HttpResponse &response) {
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;
	Json::Value obj;

	string contentType = response.getHeaderValue("Content-Type");
	if (contentType != "application/json") {
		lWarning() << "[Account Manager Services] Invalid content type [" << contentType
		           << "] in response ('application/json' was expected), not parsing it";
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(AccountManagerServicesRequest, this,
		                                  linphone_account_manager_services_request_cbs_get_request_error,
		                                  response.getHttpStatusCode(), "Invalid Content-Type in response", nullptr);
		return Json::Value::nullSingleton();
	}

	auto content = response.getBody();
	string body = content.getBodyAsString();
	if (body.empty()) {
		return Json::Value::nullSingleton();
	}

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(body.c_str(), body.c_str() + body.length(), &obj, &err)) {
		lError() << err;
		return Json::Value::nullSingleton();
	}

	return obj;
}

string AccountManagerServicesRequest::extractDataFromSuccessfulRequest(const HttpResponse &response) {
	string data = "";
	if (mType == LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken ||
	    mType == LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken ||
	    mType == LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken ||
	    mType == LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin ||
	    mType == LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin) {
		auto json = parseResponseAsJson(response);
		if (json != Json::Value::nullSingleton()) {
			if (mType == LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken) {
				if (json.isMember("validation_url")) {
					data = json["validation_url"].asString();
				} else if (json.isMember("token")) {
					data = json["token"].asString();
				} else {
					lError()
					    << "[Account Manager Services Request] Failed to find either 'validation_url' or 'token' in "
					       "returned JSON!";
				}
			} else if (mType ==
			           LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken) {
				if (json.isMember("token")) {
					data = json["token"].asString();
				} else {
					lError() << "[Account Manager Services Request] Failed to find either 'token' in "
					            "returned JSON!";
				}
			} else if (mType == LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken) {
				if (mTesterEnv) {
					lWarning() << "[Account Manager Services Request] Tester env flag set, altering data!";
					data = json["id"].asString();
				} else if (json.isMember("username") && json.isMember("domain")) {
					string username = json["username"].asString();
					string domain = json["domain"].asString();
					data = string("sip:" + username + "@" + domain).c_str();
				} else {
					lError() << "[Account Manager Services Request] Failed to find 'username' and 'domain' in returned "
					            "JSON!";
				}
			} else if (mType == LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin) {
				if (json.isMember("token")) {
					data = json["token"].asString();
				} else {
					lError() << "[Account Manager Services Request] Failed to find 'token' in returned JSON!";
				}
			} else if (mType == LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin) {
				if (json.isMember("phone_change_code") || json.isMember("email_change_code")) {
					auto phoneCode = json["phone_change_code"];
					auto emailCode = json["email_change_code"];
					if (phoneCode != Json::Value::nullSingleton()) {
						data = phoneCode["code"].asString();
					} else if (emailCode != Json::Value::nullSingleton()) {
						data = emailCode["code"].asString();
					}
				} else {
					lError() << "[Account Manager Services Request] Failed to find either 'phone_change_code' or "
					            "'email_change_code' in returned JSON!";
				}
			}
		} else {
			lError() << "[Account Manager Services Request] Failed to parse response body as JSON!";
		}
	}
	return data;
}

void AccountManagerServicesRequest::handleSuccess(const HttpResponse &response) {
	string data = extractDataFromSuccessfulRequest(response);
	lDebug() << "[Account Manager Services Request] " << requestTypeToString(mType) << " success ("
	         << response.getHttpStatusCode() << "), data is [" << data << "]";

	if (mType == LinphoneAccountManagerServicesRequestTypeGetDevicesList) {
		auto json = parseResponseAsJson(response);
		if (json != Json::Value::nullSingleton()) {
			list<shared_ptr<AccountDevice>> devices;
			for (auto const &uuid : json.getMemberNames()) {
				auto device = json[uuid];
				string updateTime = device["update_time"].asString();
				string userAgent = device["user_agent"].asString();
				shared_ptr<AccountDevice> accountDevice =
				    (new AccountDevice(uuid, updateTime, userAgent))->toSharedPtr();
				devices.push_back(accountDevice);
			}

			mDevicesList.mList = devices;
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS(AccountManagerServicesRequest, this,
			                                  linphone_account_manager_services_request_cbs_get_devices_list_fetched,
			                                  mDevicesList.getCList());
		}
	}

	const char *c_data = nullptr;
	if (!data.empty()) c_data = data.c_str();
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(AccountManagerServicesRequest, this,
	                                  linphone_account_manager_services_request_cbs_get_request_successful, c_data);
}

void AccountManagerServicesRequest::handleError(const HttpResponse &response) {
	string errorMessage = response.getReason();
	shared_ptr<Dictionary> dictionary = nullptr;

	auto json = parseResponseAsJson(response);
	if (json != Json::Value::nullSingleton()) {
		errorMessage = json["message"].asString();

		if (json.isMember("errors")) {
			dictionary = (new Dictionary())->toSharedPtr();
			auto fieldErrors = json["errors"];
			for (auto const &key : fieldErrors.getMemberNames()) {
				string fieldErrorMessage = fieldErrors[key][0].asString();
				lWarning() << "[Account Manager Services Request] Found field [" << key << "] error message ["
				           << fieldErrorMessage << "]";
				dictionary->setProperty(key, fieldErrorMessage);
			}
		}
	} else {
		lError() << "[Account Manager Services Request] Failed to parse response body as JSON!";
	}

	lWarning() << "[Account Manager Services Request] " << requestTypeToString(mType) << " error: [" << errorMessage
	           << "] (" << response.getHttpStatusCode() << ")";
	LinphoneDictionary *dict = nullptr;
	if (dictionary != nullptr) {
		dict = dictionary->toC();
	}
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(AccountManagerServicesRequest, this,
	                                  linphone_account_manager_services_request_cbs_get_request_error,
	                                  response.getHttpStatusCode(), errorMessage.c_str(), dict);
}

LINPHONE_END_NAMESPACE
