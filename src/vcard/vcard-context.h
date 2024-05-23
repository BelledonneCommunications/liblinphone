/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_VCARD_CONTEXT_H_
#define _L_VCARD_CONTEXT_H_

#include "belle-sip/object++.hh"

#ifdef VCARD_ENABLED
#include <belcard/belcard.hpp>
#endif /* VCARD_ENABLED */

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"
#include "tester_utils.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Vcard;

class LINPHONE_PUBLIC VcardContext : public bellesip::HybridObject<LinphoneVcardContext, VcardContext>,
                                     public UserDataAccessor {
public:
	VcardContext(bool useVCard3Grammar = false);
	VcardContext(const VcardContext &other) = delete;
	virtual ~VcardContext() = default;

	VcardContext *clone() const override;

	bool isUsingVCard3Grammar() const {
#ifdef VCARD_ENABLED
		return mParser->isUsingV3Grammar();
#else
		return false;
#endif
	}

	// Getters
	std::shared_ptr<Vcard> getVcardFromBuffer(const std::string &buffer) const;
	std::list<std::shared_ptr<Vcard>> getVcardListFromBuffer(const std::string &buffer) const;
	std::list<std::shared_ptr<Vcard>> getVcardListFromFile(const std::string &filename) const;

private:
#ifdef VCARD_ENABLED
	std::shared_ptr<belcard::BelCardParser> mParser;
#endif /* VCARD_ENABLED */
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_VCARD_CONTEXT_H_
