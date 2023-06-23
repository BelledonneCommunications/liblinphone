/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "linphone/utils/utils.h"

#include "logger/logger.h"

#include "address-parser.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static void sal_address_ext_set_scheme(SalAddress *addr, const std::string &scheme) {
	if (scheme == "sips") sal_address_set_secure(addr, TRUE);
}

static void sal_address_ext_set_gr(SalAddress *addr, const char *value) {
	sal_address_set_uri_param(addr, "gr", value);
}

static void sal_address_ext_set_username_escaped(SalAddress *addr, const char *value) {
	/* In the SalAddress (aka belle_sip_header_address_t), the user name is stored "unescaped".
	 * Our fast Address parser gives us escaped strings, so we have to convert*/
	char *tmp = belle_sip_to_unescaped_string(value);
	sal_address_set_username(addr, tmp);
	belle_sip_free(tmp);
}

unique_ptr<AddressParser> AddressParser::sInstance;

AddressParser &AddressParser::get() {
	if (!sInstance) sInstance.reset(new AddressParser());
	return *sInstance.get();
}

AddressParser::AddressParser() {

	shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(IdentityGrammar);
	if (!grammar) lFatal() << "Unable to load Identity Address grammar.";
	mParser = make_shared<belr::Parser<void *>>(grammar);

	mParser->setHandler("address", belr::make_fn(&sal_address_new_empty))
	    ->setCollector("scheme", belr::make_fn(&sal_address_ext_set_scheme))
	    ->setCollector("user", belr::make_fn(&sal_address_ext_set_username_escaped))
	    ->setCollector("host", belr::make_fn(&sal_address_set_domain))
	    ->setCollector("gruu-value", belr::make_fn(&sal_address_ext_set_gr));
}

// -----------------------------------------------------------------------------

SalAddress *AddressParser::parseAddress(const string &input) {
	size_t parsedSize;
	SalAddress *identityAddress = (SalAddress *)mParser->parseInput("address", input, &parsedSize);
	if (!identityAddress || parsedSize < input.size()) {
		lDebug() << "Unable to parse identity address from " << input;
		if (identityAddress) sal_address_unref(identityAddress);
		return nullptr;
	}
	return identityAddress;
}

LINPHONE_END_NAMESPACE
