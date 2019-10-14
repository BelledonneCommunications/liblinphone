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

#ifndef _L_CONTENT_P_H_
#define _L_CONTENT_P_H_

#include "content-disposition.h"
#include "content-type.h"
#include "content.h"
#include "object/clonable-object-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Header;

class ContentPrivate : public ClonableObjectPrivate {
private:
	std::vector<char> body;
	ContentType contentType;
	ContentDisposition contentDisposition;
	std::string contentEncoding;
	std::list<Header> headers;

	const std::list<std::pair<std::string, std::string>>::const_iterator findHeader (const std::string &headerName) const;

	L_DECLARE_PUBLIC(Content);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONTENT_P_H_
