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

#ifndef _L_DICTIONARY_H_
#define _L_DICTIONARY_H_

#include "belle-sip/object++.hh"

#include "buffer/buffer.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"
#include "object/property-container.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class LINPHONE_PUBLIC Dictionary : public bellesip::HybridObject<LinphoneDictionary, Dictionary>,
                                   public PropertyContainer {
public:
	Dictionary *clone() const override;

	float getFloat(const std::string &name) const;
	void setProperty(const std::string &name, const float value);

	const std::string &getString(const std::string &name) const;
	void setProperty(const std::string &name, const std::string &value);

	int getInt(const std::string &name) const;
	void setProperty(const std::string &name, const int value);

	long long getLongLong(const std::string &name) const;
	void setProperty(const std::string &name, const long long value);

	std::shared_ptr<Buffer> getBuffer(const std::string &name) const;
	void setProperty(const std::string &name, const std::shared_ptr<Buffer> &value);

	const std::list<std::string> getKeys() const;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_DICTIONARY_H_
