/*
 * identity-address-parser.cpp
 * Copyright (C) 2019 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <set>
#include <unordered_map>

#include <belr/abnf.h>
#include <belr/grammarbuilder.h>

#include "linphone/utils/utils.h"

#include "logger/logger.h"
#include "object/object-p.h"

#include "identity-address-parser.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class IdentityAddressParserPrivate : public ObjectPrivate {
public:
	shared_ptr<belr::Parser<shared_ptr<IdentityAddress> >> parser;
    unordered_map<string, shared_ptr<IdentityAddress >> cache;
};

IdentityAddressParser::IdentityAddressParser () : Singleton(*new IdentityAddressParserPrivate) {
	L_D();

    const char *identityAddressGrammar =
        "address = scheme \":\" [user] \"@\" host [ gruu-parameter ] \r\n"
        "scheme = \"sip\" / \"sips\" \r\n"
        "user = 1*( alphanum / escaped / \"-\" / \"+\" / \"_\" / \"~\" ) \r\n"
        "escaped = \"%\" HEXDIG HEXDIG \r\n"
        "host = 1*( alphanum / \".\" ) \r\n"
        "gruu-parameter = \";gr=\" gruu-value \r\n"
        "gruu-value = 1*( alphanum / \"-\" / \"_\" / \":\" ) \r\n"
        "alphanum = ALPHA / DIGIT \r\n";
	
	belr::ABNFGrammarBuilder builder;
    shared_ptr<belr::Grammar> defaultGrammar = make_shared<belr::Grammar>("");
    defaultGrammar->include(make_shared<belr::CoreRules>());
    shared_ptr<belr::Grammar> grammar = builder.createFromAbnf(identityAddressGrammar, defaultGrammar);

	//shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(IdentityAddressGrammar);
	if (!grammar)
		lFatal() << "Unable to load Identity Address grammar.";
	d->parser = make_shared<belr::Parser<shared_ptr<IdentityAddress>>>(grammar);
	
	d->parser->setHandler("address", belr::make_fn(make_shared<IdentityAddress>))
		->setCollector("scheme", belr::make_sfn(&IdentityAddress::setScheme))
		->setCollector("user", belr::make_sfn(&IdentityAddress::setUsername))
		->setCollector("host", belr::make_sfn(&IdentityAddress::setDomain))
		->setCollector("gruu-value", belr::make_sfn(&IdentityAddress::setGruu));
}

// -----------------------------------------------------------------------------

shared_ptr<IdentityAddress> IdentityAddressParser::parseAddress (const string &input) {
	L_D();

    auto it = d->cache.find(input);
    if (it == d->cache.end()) {
        size_t parsedSize;
        shared_ptr<IdentityAddress> identityAddress = d->parser->parseInput("Address", input, &parsedSize);
        if (!identityAddress) {
            lWarning() << "Unable to parse identity address from " << input;
            return nullptr;
        }
        d->cache[input] = identityAddress;
        return identityAddress;
    } else {
        return it->second;
    }
}

LINPHONE_END_NAMESPACE