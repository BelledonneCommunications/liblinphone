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

#ifndef _L_BUFFER_H_
#define _L_BUFFER_H_

#include <belle-sip/object++.hh>

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class LINPHONE_PUBLIC Buffer : public bellesip::HybridObject<LinphoneBuffer, Buffer> {
public:
	Buffer() = default;
	explicit Buffer(const std::vector<uint8_t> &data);
	explicit Buffer(const std::string &data);
	explicit Buffer(std::vector<uint8_t> &&data);
	explicit Buffer(std::string &&data);

	void *getUserData() const;
	void setUserData(void *ud);

	const std::vector<uint8_t> &getContent() const;
	void setContent(const std::vector<uint8_t> &content);

	const std::string &getStringContent() const;
	void setStringContent(const std::string &content);

	size_t getSize() const;
	void setSize(size_t size);

	bool_t isEmpty() const;

private:
	void *mUserData;
	std::vector<uint8_t> mContent;
	mutable std::string mStringContent;
};

LINPHONE_END_NAMESPACE

#endif // _L_BUFFER_H_