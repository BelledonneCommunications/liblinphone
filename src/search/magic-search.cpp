/*
 * magic-search.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
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

#include "magic-search.h"

#include <bctoolbox/list.h>

#include "c-wrapper/internal/c-tools.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const unsigned int MagicSearch::sMaxWeight = 10000;

static unsigned int getWeight(const std::string world, const std::string &filter, const unsigned int weightMax = MagicSearch::sMaxWeight) {
	return 0;
}

void MagicSearch::getContactListFromFilter(const std::string &filter) {
	/*list<LinphoneFriend> friendList =
		L_GET_RESOLVED_CPP_LIST_FROM_C_LIST(
			linphone_core_get_default_friend_list(this->getCore()->getCCore())->friends,
			Friend
			);*/
	list<LinphoneFriend*> friendList = list<LinphoneFriend*>();
	LinphoneFriendList* list = linphone_core_get_default_friend_list(this->getCore()->getCCore());
	for (bctbx_list_t *f = list->friends ; f != nullptr; f = bctbx_list_next(f)) {
		//unsigned int weight = 0;
		// NOM
		// PRENOM
		// NUMERO
		/*weight = */getWeight("", "");

		friendList.push_back((LinphoneFriend*)f->data);
	}
}

LINPHONE_END_NAMESPACE
