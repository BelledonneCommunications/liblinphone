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

#include "http-server-utils.h"

HttpServerWithBearerAuth::HttpServerWithBearerAuth(const std::string &realm, const std::string &validToken) {
	mToken = validToken;
	mBaseUrl = "http://127.0.0.1:" + mListeningPort;
	mRealm = realm;
	BCTBX_SLOGI << " Waiting for http request on " << mBaseUrl;
}
void HttpServerWithBearerAuth::challenge(httplib::Response &res) {
	res.status = 401;
	std::ostringstream headerValue;
	headerValue << "Bearer realm=\"" << mRealm << "\""
	            << ", authz_server=\"auth.example.org\"";
	res.set_header("WWW-Authenticate", headerValue.str());
}
void HttpServerWithBearerAuth::addResource(const std::string &urlPath,
                                           const std::string &contentType,
                                           const std::string &resourceName) {
	Get(urlPath, [this, contentType, resourceName](const httplib::Request &req, httplib::Response &res) {
		if (req.has_header("Authorization")) {
			// ok
			belle_sip_header_authorization_t *authorisation = belle_sip_header_authorization_parse(
			    ("Authorization: " + req.get_header_value("Authorization")).c_str());
			BC_ASSERT_STRING_EQUAL(belle_sip_header_authorization_get_scheme(authorisation), "Bearer");
			belle_sip_param_pair_t *paramPair = (belle_sip_param_pair_t *)bctbx_list_get_data(
			    belle_sip_parameters_get_parameters(BELLE_SIP_PARAMETERS(authorisation)));
			std::string token = paramPair ? paramPair->name : "";
			belle_sip_object_unref(authorisation);
			if (token == mToken) {
				res.set_content(loadFile(resourceName), contentType);
			} else if (token.find("expired") != std::string::npos) {
				challenge(res);
				return;
			} else {
				res.status = 403;
			}
		} else {
			challenge(res);
			return;
		}
	});
}
const std::string &HttpServerWithBearerAuth::getUrl() const {
	return mBaseUrl;
}

std::string HttpServerWithBearerAuth::loadFile(const std::string &resource) {
	char *file = bc_tester_res(resource.c_str());
	if (file) {
		std::ifstream ifst(file);
		if (!ifst.is_open()) bctbx_error("Cannot open [%s]", file);
		bc_free(file);
		std::ostringstream ostr;
		ostr << ifst.rdbuf();
		bctbx_message("File content is %s", ostr.str().c_str());
		return ostr.str();
	} else {
		bctbx_error("No resource file [%s]", resource.c_str());
	}
	return "";
}
