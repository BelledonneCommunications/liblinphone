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

#ifndef _L_CLONABLE_OBJECT_P_H_
#define _L_CLONABLE_OBJECT_P_H_

#include <set>

#include "linphone/utils/general.h"

#include "object-head-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClonableObjectPrivate {
	L_OBJECT_PRIVATE;

public:
	ClonableObjectPrivate () = default;
	virtual ~ClonableObjectPrivate () = default;

protected:
	std::set<ClonableObject *> mPublic;

private:
	L_DECLARE_PUBLIC(ClonableObject);

	// It's forbidden to copy directly one Clonable object private.
	// To allow copy, you must define copy constructor in inherited object.
	L_DISABLE_COPY(ClonableObjectPrivate);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLONABLE_OBJECT_P_H_
