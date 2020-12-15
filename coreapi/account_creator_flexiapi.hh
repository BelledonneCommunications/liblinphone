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

#include <functional>

using namespace LinphonePrivate;
using namespace std;

class AccountCreatorFlexiAPI {
    public:
        typedef struct response {
            int code;
            const char* body;
        } reponse_t;

        AccountCreatorFlexiAPI(LinphoneCore *lc);

        // Endpoints
        AccountCreatorFlexiAPI* ping();

        // Callbacks handlers
        AccountCreatorFlexiAPI* then(function<void (response)> success);
        AccountCreatorFlexiAPI* error(function<void (response)> error);

    private:
        typedef struct callbacks {
            function<void (response)> success;
            function<void (response)> error;
        } callbacks_t;

        LinphoneCore *mCore;
        callbacks mRequestCallbacks;

        void prepareRequest(const char* path);
        static void processResponse(void *ctx, const belle_http_response_event_t *event);
};