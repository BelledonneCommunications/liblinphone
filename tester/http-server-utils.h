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

#include "belle_sip_tester_utils.h"

/*
 * Fake Http server class simulating bearer authentication.
 * A token considered as valid is supplied at constructor.
 * Any token containing the key word "expired" is considered as expired.
 */
class HttpServerWithBearerAuth : public bellesip::HttpServer {
public:
	/* Initialize the http server, providing a realm and a valid bearer token to accept. */
	HttpServerWithBearerAuth(const std::string &realm, const std::string &validToken);
	// Add a resource file to serve at given url
	void addResource(const std::string &urlPath, const std::string &contentType, const std::string &resourceName);
	/* Returns the base url of the server */
	const std::string &getUrl() const;

private:
	void challenge(httplib::Response &res);
	std::string loadFile(const std::string &resource);
	std::string mToken;
	std::string mBaseUrl;
	std::string mRealm;
};
