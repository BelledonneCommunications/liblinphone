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

#ifndef _L_IDENTITY_ADDRESS_PARSER_H_
#define _L_IDENTITY_ADDRESS_PARSER_H_

#include "belr/abnf.h"
#include "belr/belr.h"
#include "belr/grammarbuilder.h"

#include "c-wrapper/internal/c-sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

/**
 * The AddressParser is designed to efficiently parse
 * simple SIP uris whith only scheme, user, host, and gr parameter.
 */
class AddressParser {

public:
	SalAddress *parseAddress(const std::string &input);
	static AddressParser &get();

private:
	AddressParser();
	std::shared_ptr<belr::Parser<void *>> mParser;
	static std::unique_ptr<AddressParser> sInstance;
	static constexpr const char *IdentityGrammar = "identity_grammar.belr";
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_IDENTITY_ADDRESS_PARSER_H_
