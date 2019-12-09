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

#include "bctoolbox/defs.h"
#include "sal/register-op.h"
#include "bellesip_sal/sal_impl.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

int SalRegisterOp::sendRegister (const string &proxy, const string &from, int expires, const SalAddress *oldContact) {
	if (mRefresher) {
		belle_sip_refresher_stop(mRefresher);
		belle_sip_object_unref(mRefresher);
		mRefresher = nullptr;
	}

	setFrom(from);
	setTo(from);
	setRoute(proxy);
	auto request = buildRequest("REGISTER");
	auto requestUri = belle_sip_request_get_uri(request);
	belle_sip_uri_set_user(requestUri, nullptr); // Remove userinfo if there is any
	if (mRoot->mUseDates) {
		time_t curtime = time(nullptr);
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(belle_sip_header_date_create_from_time(&curtime)));
	}
	auto acceptHeader = belle_sip_header_create(
		"Accept",
		"application/sdp, text/plain, application/vnd.gsma.rcs-ft-http+xml"
	);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), acceptHeader);
	belle_sip_message_set_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(createContact()));

	if (oldContact) {
		auto contactHeader = belle_sip_header_contact_create(BELLE_SIP_HEADER_ADDRESS(oldContact));
		if (contactHeader) {
			belle_sip_header_contact_set_expires(contactHeader, 0); // Remove old aor
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(contactHeader));
			char *tmp = belle_sip_object_to_string(contactHeader);
			lInfo() << "Clearing contact [" << tmp << "] for op [" << this << "]";
			ms_free(tmp);
		} else {
			lError() << "Cannot add old contact header to op [" << this << "]";
		}
	}

	return sendRequestAndCreateRefresher(request, expires, registerRefresherListener);
}

void SalRegisterOp::registerRefresherListener (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry) {
	auto op = static_cast<SalRegisterOp *>(userCtx);
	auto response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(belle_sip_refresher_get_transaction(refresher)));

	lInfo() << "Register refresher [" << statusCode << "] reason [" << reasonPhrase << "] for proxy [" << op->getProxy() << "]";

	if (belle_sip_refresher_get_auth_events(refresher)) {
		if (op->mAuthInfo)
			sal_auth_info_delete(op->mAuthInfo);
		// Only take first one for now
		op->mAuthInfo = sal_auth_info_create(reinterpret_cast<belle_sip_auth_event_t *>(belle_sip_refresher_get_auth_events(refresher)->data));
	}
	sal_error_info_set(&op->mErrorInfo, SalReasonUnknown, "SIP", (int)statusCode, reasonPhrase, nullptr);

	if (statusCode >= 200)
		op->assignRecvHeaders(BELLE_SIP_MESSAGE(response));

	if (statusCode == 200) {
		// Check service route rfc3608
		auto contactHeader = belle_sip_refresher_get_contact(refresher);
		auto serviceRouteHeader = belle_sip_message_get_header_by_type(response, belle_sip_header_service_route_t);
		belle_sip_header_address_t *serviceRouteAddressHeader = nullptr;
		if (serviceRouteHeader) {
			serviceRouteAddressHeader = belle_sip_header_address_create(
				nullptr,
				belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(serviceRouteHeader))
			);
		}
		op->setServiceRoute(reinterpret_cast<const SalAddress *>(serviceRouteAddressHeader));
		if (serviceRouteAddressHeader)
			belle_sip_object_unref(serviceRouteAddressHeader);

		op->mRoot->removePendingAuth(op); // Just in case

		if (contactHeader) {
			auto p = BELLE_SIP_PARAMETERS(contactHeader);
			const char *gruu = belle_sip_parameters_get_parameter(p, "pub-gruu");
			if (gruu) {
				char *unquotedGruu = belle_sip_unquote_strdup(gruu);
				op->setContactAddress(reinterpret_cast<SalAddress *>(belle_sip_header_address_parse(unquotedGruu)));
				bctbx_free(unquotedGruu);
				belle_sip_parameters_remove_parameter(p, "pub-gruu");
			} else {
				// Update contact with real value
				op->setContactAddress(reinterpret_cast<SalAddress *>(BELLE_SIP_HEADER_ADDRESS(contactHeader)));
			}
		}
		op->mRoot->mCallbacks.register_success(op, belle_sip_refresher_get_expires(op->mRefresher) > 0);
	} else if (statusCode >= 400) {
		// From rfc3608, 6.1.:
		//     If the UA refreshes the registration, the stored value of the Service-
		//     Route is updated according to the Service-Route header field of the
		//     latest 200 class response.  If there is no Service-Route header field
		//     in the response, the UA clears any service route for that address-
		//     of-record previously stored by the UA.  If the re-registration
		//     request is refused or if an existing registration expires and the UA
		//     chooses not to re-register, the UA SHOULD discard any stored service
		//     route for that address-of-record.
		op->setServiceRoute(nullptr);
		op->ref(); // Take a ref while invoking the callback to make sure the operations done after are valid
		op->mRoot->mCallbacks.register_failure(op);
		if ((op->mState != State::Terminated) && op->mAuthInfo) {
			switch (statusCode){
				case 401:
					BCTBX_NO_BREAK;
				case 407:
					// Add pending auth for both 401 and 407, as we got a non-working authentication.
					op->mRoot->addPendingAuth(op);
					BCTBX_NO_BREAK;
					// In any case notify the failure.
				case 403:
					op->mRoot->mCallbacks.auth_failure(op, op->mAuthInfo);
					break;
			}
		}
		op->unref();
	}
}

LINPHONE_END_NAMESPACE
