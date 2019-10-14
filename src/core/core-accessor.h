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

#ifndef _L_CORE_ACCESSOR_H_
#define _L_CORE_ACCESSOR_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;
class CoreAccessorPrivate;

// Decorator to get a valid core instance.
class LINPHONE_PUBLIC CoreAccessor {
public:
	CoreAccessor (const std::shared_ptr<Core> &core);
	virtual ~CoreAccessor () = 0;

	// Returns a valid core instance. Or throw one std::bad_weak_ptr exception if core is destroyed.
	std::shared_ptr<Core> getCore () const;

private:
	CoreAccessorPrivate *mPrivate = nullptr;

	L_DISABLE_COPY(CoreAccessor);
	L_DECLARE_PRIVATE(CoreAccessor);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_ACCESSOR_H_
