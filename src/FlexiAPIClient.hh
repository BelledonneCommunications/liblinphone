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

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

#include "c-wrapper/c-wrapper.h"

#include <functional>
#include <json/json.h>

using namespace LinphonePrivate;
using namespace std;

class LINPHONE_PUBLIC FlexiAPIClient : public enable_shared_from_this<FlexiAPIClient> {
  public:
	class Response {
	  public:
		int code = 0;
		string body = "";

		Json::Value json() {
			JSONCPP_STRING err;
			Json::CharReaderBuilder builder;
			Json::Value obj;

			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

			if (!reader->parse(body.c_str(), body.c_str() + body.length(), &obj, &err)) {
				lError() << err;
			}

			return obj;
		};
	};

	class JsonParams {
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

	class Callbacks {
	  public:
		function<void(const Response &)> success;
		function<void(const Response &)> error;
		LinphoneCore *core;
		shared_ptr<FlexiAPIClient> mSelf;
	};

	FlexiAPIClient(LinphoneCore *lc);

	// Public endpoinds
	FlexiAPIClient *ping();
	FlexiAPIClient *sendToken(string pnProvider, string pnParam, string pnPrid);
	FlexiAPIClient *accountCreate(string username, string password, string algorithm, string token);
	FlexiAPIClient *accountCreate(string username, string domain, string password, string algorithm, string token);
	FlexiAPIClient *accountInfo(string sip);
	FlexiAPIClient *accountActivateEmail(string sip, string code);
	FlexiAPIClient *accountActivatePhone(string sip, string code);

	// Authenticated endpoints
	FlexiAPIClient *me();
	FlexiAPIClient *accountDelete();
	FlexiAPIClient *accountPasswordChange(string algorithm, string password);
	FlexiAPIClient *accountPasswordChange(string algorithm, string password, string oldPassword);
	FlexiAPIClient *accountDevices();
	FlexiAPIClient *accountDevice(string uuid);
	FlexiAPIClient *accountEmailChangeRequest(string email);
	FlexiAPIClient *accountPhoneChangeRequest(string phone);
	FlexiAPIClient *accountPhoneChange(string code);

	// Admin endpoints
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, string domain);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, bool activated);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, string domain,
									   bool activated);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, string domain,
									   bool activated, string email);
	FlexiAPIClient *adminAccountCreate(string username, string password, string algorithm, string domain,
									   bool activated, string email, string phone);
	FlexiAPIClient *adminAccounts();
	FlexiAPIClient *adminAccount(int id);
	FlexiAPIClient *adminAccountDelete(int id);
	FlexiAPIClient *adminAccountActivate(int id);
	FlexiAPIClient *adminAccountDeactivate(int id);

	// Authentication
	FlexiAPIClient *setApiKey(const char *key);
	FlexiAPIClient *setTest(bool test);

	// Callbacks handlers
	FlexiAPIClient *then(function<void(Response)> success);
	FlexiAPIClient *error(function<void(Response)> error);

  private:
	LinphoneCore *mCore;
	Callbacks mRequestCallbacks;
	const char *mApiKey;
	bool mTest;

	void prepareRequest(string path);
	void prepareRequest(string path, string type);
	void prepareRequest(string path, string type, JsonParams params);
	static void processResponse(void *ctx, const belle_http_response_event_t *event) noexcept;
	static void processAuthRequested(void *ctx, belle_sip_auth_event_t *event) noexcept;
	string urlEncode(const string &value);
};