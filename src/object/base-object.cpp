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

#include "base-object-p.h"
#include "base-object.h"
#include "c-wrapper/internal/c-tools.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

L_OBJECT_IMPL(BaseObject);

BaseObject::BaseObject (BaseObjectPrivate &p) : mPrivate(&p) {
	mPrivate->mPublic = this;
}

BaseObject::~BaseObject () {
	Wrapper::handleObjectDestruction(this);
	delete mPrivate;
}

LINPHONE_END_NAMESPACE
