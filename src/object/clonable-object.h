/*
 * clonable-object.h
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

#ifndef _L_CLONABLE_OBJECT_H_
#define _L_CLONABLE_OBJECT_H_

#include "clonable-shared-pointer.h"
#include "object-head.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

/*
 * Clonable Object of Linphone. Generally it's just a data object with no
 * intelligence.
 */
class LINPHONE_PUBLIC ClonableObject {
	L_OBJECT;

public:
	virtual ~ClonableObject ();

protected:
	explicit ClonableObject (ClonableObjectPrivate &p);

	ClonableSharedPointer<SharedObject> mPrivate;

private:
	L_DECLARE_PRIVATE(ClonableObject);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLONABLE_OBJECT_H_
