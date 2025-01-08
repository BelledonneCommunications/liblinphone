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

#ifndef _L_DIAL_PLAN_H_
#define _L_DIAL_PLAN_H_

#include <functional>
#include <list>
#include <optional>

#include "bctoolbox/list.h"
#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class DialPlan : public bellesip::HybridObject<LinphoneDialPlan, DialPlan> {
public:
	DialPlan(
	    const std::string &country = "",
	    const std::string &isoCountryCode = "",
	    const std::string &ccc = "",
	    int minNnl = 0,
	    int maxNnl = 0,
	    const std::string &icp = "",
	    const std::string &flag = "",
	    const std::optional<std::function<size_t(const std::string)>> &nationalNumberLengthFunction = std::nullopt);
	DialPlan(const DialPlan &other) = default;
	DialPlan &operator=(const DialPlan &other);

	DialPlan *clone() const override {
		return new DialPlan(*this);
	}

	const std::string &getCountry() const;
	const std::string &getIsoCountryCode() const;
	const std::string &getCountryCallingCode() const;
	void setCountryCallingCode(const std::string &ccc);
	int getMinNationalNumberLength() const;
	int getMaxNationalNumberLength() const;
	const std::string &getInternationalCallPrefix() const;
	const std::string &getFlag() const;
	bool isGeneric() const;

	std::string flattenPhoneNumber(const std::string &number) const;
	std::string formatPhoneNumber(const std::string &phoneNumber, bool escapePlus) const;
	std::string formatShortNumber(const std::string &phoneNumber) const;
	std::string getSignificantDigits(const std::string &phoneNumber) const;

	static const std::shared_ptr<DialPlan> MostCommon;

	static int lookupCccFromE164(const std::string &e164);
	static int lookupCccFromIso(const std::string &iso);
	static std::shared_ptr<DialPlan> findByCcc(int ccc);
	static std::shared_ptr<DialPlan> findByCcc(const std::string &ccc);
	static const std::list<std::shared_ptr<DialPlan>> &getAllDialPlans();
	static bool_t isShortNumber(const int ccc, const std::string &phoneNumber);
	static bool_t hasEnoughSignificantDigits(const int ccc, const std::string &phoneNumber);

private:
	std::string country;
	std::string isoCountryCode;          // ISO 3166-1 alpha-2 code, ex: FR for France.
	std::string countryCallingCode;      // Country calling code.
	int minNationalNumberLength = 0;     // Minimum national number length to search for prefixes.
	int maxNationalNumberLength = 8;     // Maximum national number length.
	std::string internationalCallPrefix; // International call prefix, ex: 00 in europe.
	std::string flag;
	std::function<size_t(const std::string)> mNationalNumberLengthFunction;

	static const std::list<std::shared_ptr<DialPlan>> sDialPlans;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_DIAL_PLAN_H_
