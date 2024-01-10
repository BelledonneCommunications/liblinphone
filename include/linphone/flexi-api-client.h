/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include <functional>
#include <json/json.h>

#include <belle-sip/listener.h>

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

using namespace std;

struct belle_http_response_event;
struct belle_sip_auth_event;

typedef struct belle_http_response_event belle_http_response_event_t;
typedef struct belle_sip_auth_event belle_sip_auth_event_t;

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC FlexiAPIClient {
public:
	class LINPHONE_PUBLIC Response {
	public:
		int code = 0;
		string body = "";

		Json::Value json();
	};

	class LINPHONE_PUBLIC JsonParams {
	public:
		Json::Value jsonParameters;

		void push(string key, string value) {
			jsonParameters[key] = value;
		};

		bool empty() {
			return jsonParameters.empty();
		};

		string json() {
			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";

			return string(Json::writeString(builder, jsonParameters));
		};
	};

	class LINPHONE_PUBLIC Callbacks {
	public:
		function<void(const Response &)> success;
		function<void(const Response &)> error;
		LinphoneCore *core;
	};

	FlexiAPIClient(LinphoneCore *lc);

	// Public endpoinds
	FlexiAPIClient *ping();
	FlexiAPIClient *sendAccountCreationToken();
	FlexiAPIClient *sendAccountCreationTokenByPush(string pnProvider, string pnParam, string pnPrid);
	FlexiAPIClient *accountCreationRequestToken();
	FlexiAPIClient *accountCreationTokenUsingRequestToken(string token);
	FlexiAPIClient *
	accountCreateWithAccountCreationToken(string username, string password, string algorithm, string token);
	FlexiAPIClient *accountCreateWithAccountCreationToken(
	    string username, string domain, string password, string algorithm, string token);
	FlexiAPIClient *accountInfo(string sip);
	FlexiAPIClient *accountActivateEmail(string sip, string code);
	FlexiAPIClient *accountActivatePhone(string sip, string code);
	FlexiAPIClient *accountAuthTokenCreate();
	FlexiAPIClient *accountApiKeyFromAuthTokenGenerate(string authToken);

	// Public unsecure endpoints
	FlexiAPIClient *accountInfoByPhone(string phone);
	FlexiAPIClient *accountRecoverByPhone(string phone, string token);
	FlexiAPIClient *accountRecoverUsingRecoverKey(string sip, string recoverKey);
	FlexiAPIClient *accountCreate(string username, string password, string email);
	FlexiAPIClient *accountCreate(
	    string username, string password, string algorithm, string domain, string email, string phone, string token);

	// Authenticated endpoints
	FlexiAPIClient *me();
	FlexiAPIClient *accountDelete();
	FlexiAPIClient *accountPasswordChange(string algorithm, string password);
	FlexiAPIClient *accountPasswordChange(string algorithm, string password, string oldPassword);
	FlexiAPIClient *accountDevices();
	FlexiAPIClient *accountDevice(string uuid);
	FlexiAPIClient *accountContacts();
	FlexiAPIClient *accountContact(string sip);
	FlexiAPIClient *accountEmailChangeRequest(string email);
	FlexiAPIClient *accountPhoneChangeRequest(string phone);
	FlexiAPIClient *accountPhoneChange(string code);
	FlexiAPIClient *accountAuthTokenAttach(string authToken);
	FlexiAPIClient *accountProvision();
	FlexiAPIClient *accountProvisioningInformation(string provisioningToken);

	// Admin endpoints
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, string domain);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, bool activated);
	FlexiAPIClient *
	adminAccountCreate(string username, string password, string algorithm, string domain, bool activated);
	FlexiAPIClient *
	adminAccountCreate(string username, string password, string algorithm, string domain, bool activated, string email);
	FlexiAPIClient *adminAccountCreate(
	    string username, string password, string algorithm, string domain, bool activated, string email, string phone);
	FlexiAPIClient *adminAccountCreate(string username,
	                                   string password,
	                                   string algorithm,
	                                   string domain,
	                                   bool activated,
	                                   string email,
	                                   string phone,
	                                   string dtmfProtocol);
	FlexiAPIClient *adminAccounts();
	FlexiAPIClient *adminAccount(int id);
	FlexiAPIClient *adminAccountDelete(int id);
	FlexiAPIClient *adminAccountActivate(int id);
	FlexiAPIClient *adminAccountDeactivate(int id);
	FlexiAPIClient *adminAccountSearch(string sip);

	FlexiAPIClient *adminAccountContacts(int id);
	FlexiAPIClient *adminAccountContactAdd(int id, int contactId);
	FlexiAPIClient *adminAccountContactDelete(int id, int contactId);

	// Authentication
	FlexiAPIClient *setApiKey(const char *key);
	FlexiAPIClient *useTestAdminAccount(bool test);

	// Callbacks handlers
	FlexiAPIClient *then(const function<void(Response)> &success);
	FlexiAPIClient *error(const function<void(Response)> &error);

private:
	LinphoneCore *mCore;
	shared_ptr<Callbacks> mRequestCallbacks;
	const char *mApiKey;
	bool mUseTestAdminAccount;

	void prepareAndSendRequest(string path);
	void prepareAndSendRequest(string path, string type);
	void prepareAndSendRequest(string path, string type, JsonParams params);
	void prepareAndSendRootRequest(string path, string type, string contentSubtype);
	static void processResponse(void *ctx, const belle_http_response_event_t *event) noexcept;
	static void processIoError(void *ctx, const belle_sip_io_error_event_t *event) noexcept;
	static void processTimeout(void *ctx, const belle_sip_timeout_event_t *event) noexcept;
	static void processAuthRequested(void *ctx, belle_sip_auth_event_t *event) noexcept;
	string urlEncode(const string &value);
};

LINPHONE_END_NAMESPACE
