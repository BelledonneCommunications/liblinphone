/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include <jsoncpp/json/json.h>
#include <functional>

using namespace LinphonePrivate;
using namespace std;

class FlexiAPIClient {
    public:
        class Response {
            public:
                int code = 0;
                const char* body = "";

                Json::Value json() {
                    Json::Reader reader;
                    Json::Value obj;
                    reader.parse(body, obj);
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
                function<void (Response)> success;
                function<void (Response)> error;
                LinphoneCore *core;
        };

        FlexiAPIClient(LinphoneCore *lc);

        // Endpoints
        FlexiAPIClient* ping();
        FlexiAPIClient* emailChange(string email);
        FlexiAPIClient* me();
        FlexiAPIClient* passwordChange(string algorithm, string password);
        FlexiAPIClient* passwordChange(string algorithm, string password, string oldPassword);

        // Admin endpoints
        FlexiAPIClient* createAccount(string username, string password, string algorithm);
        FlexiAPIClient* createAccount(string username, string password, string algorithm, string domain);
        FlexiAPIClient* createAccount(string username, string password, string algorithm, bool activated);
        FlexiAPIClient* createAccount(string username, string password, string algorithm, string domain, bool activated);
        FlexiAPIClient* accounts();
        FlexiAPIClient* accountDelete(int id);
        FlexiAPIClient* account(int id);
        FlexiAPIClient* accountActivate(int id);
        FlexiAPIClient* accountDeactivate(int id);

        // Authentication
        FlexiAPIClient* setApiKey(const char* key);

        // Callbacks handlers
        FlexiAPIClient* then(function<void (Response)> success);
        FlexiAPIClient* error(function<void (Response)> error);

    private:
        LinphoneCore *mCore;
        Callbacks mRequestCallbacks;
        const char* apiKey;

        void prepareRequest(const char *path);
        void prepareRequest(const char *path, string type);
        void prepareRequest(const char *path, string type, JsonParams params);
        static void processResponse(void *ctx, const belle_http_response_event_t *event);
        static void processAuthRequested(void *ctx, belle_sip_auth_event_t *event);
};