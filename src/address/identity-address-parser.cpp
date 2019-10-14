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

namespace {
	string IdentityGrammar("identity_grammar");
}

// -----------------------------------------------------------------------------

class IdentityAddressParserPrivate : public ObjectPrivate {
public:
	shared_ptr<belr::Parser<shared_ptr<IdentityAddress> >> parser;
    unordered_map<string, shared_ptr<IdentityAddress >> cache;
};

IdentityAddressParser::IdentityAddressParser () : Singleton(*new IdentityAddressParserPrivate) {
	L_D();

	shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(IdentityGrammar);
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
            lDebug() << "Unable to parse identity address from " << input;
            return nullptr;
        }
        d->cache[input] = identityAddress;
        return identityAddress;
    } else {
        return it->second;
    }
}

LINPHONE_END_NAMESPACE