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

#ifndef _L_CONTENT_MANAGER_H_
#define _L_CONTENT_MANAGER_H_

#include <list>

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Content;

namespace {
	constexpr const char MultipartBoundary[] = "---------------------------14737809831466499882746641449";
}

namespace ContentManager {
	LINPHONE_PUBLIC std::list<Content> multipartToContentList (const Content &content);
	LINPHONE_PUBLIC Content contentListToMultipart (const std::list<Content *> &contents, const std::string &boundary = MultipartBoundary, bool encrypted = false);
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONTENT_MANAGER_H_
