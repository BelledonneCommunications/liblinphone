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

#ifndef _L_MWI_PARSER_H_
#define _L_MWI_PARSER_H_

#include "belr/abnf.h"
#include "belr/grammarbuilder.h"

#include "account/mwi/message-waiting-indication.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Mwi {
class Node {
public:
	virtual ~Node() = default;
};

class LINPHONE_PUBLIC Parser {
public:
	virtual ~Parser() = default;

	static Parser *getInstance() {
		static Parser instance;
		return &instance;
	}

	std::shared_ptr<MessageWaitingIndication> parseMessageSummary(const std::string &input);

private:
	Parser();
	L_DISABLE_COPY(Parser);

	std::shared_ptr<belr::Parser<std::shared_ptr<Node>>> mParser;
};
} // namespace Mwi

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MWI_PARSER_H_
