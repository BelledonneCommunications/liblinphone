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

#include "linphone/api/c-types.h"
#include "object/property-container.h"
#include <belle-sip/object++.hh>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class LINPHONE_PUBLIC Dictionary : public bellesip::HybridObject<LinphoneDictionary, Dictionary>,
                                   public PropertyContainer {
public:
	Dictionary *clone() const override;

	float getFloat(const string &name) const;
	void setProperty(const string &name, const float value);

	const string &getString(const string &name) const;
	void setProperty(const string &name, const string &value);

	int getInt(const string &name) const;
	void setProperty(const string &name, const int value);

	long long getLongLong(const string &name) const;
	void setProperty(const string &name, const long long value);
};

LINPHONE_END_NAMESPACE