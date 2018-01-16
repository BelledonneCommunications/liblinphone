/*
 * object.cpp
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

#include "logger/logger.h"
#include "object-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Object::Object (ObjectPrivate &p) : BaseObject(p) {}

// -----------------------------------------------------------------------------

shared_ptr<Object> Object::getSharedFromThis () {
	return const_pointer_cast<Object>(static_cast<const Object *>(this)->getSharedFromThis());
}

shared_ptr<const Object> Object::getSharedFromThis () const {
	try {
		return shared_from_this();
	} catch (const exception &) {
		lFatal() << "Object " << this << " was not created with make_shared.";
	}

	// Unable to reach this point.
	L_ASSERT(false);
	return nullptr;
}

const Object::Lock &Object::getLock () const {
	L_D();
	return d->getLock();
}

// -----------------------------------------------------------------------------

#define CHECK_CONNECT_PARAM(PARAM) \
	do { \
		if (!PARAM) { \
			lError() << "No " #PARAM " given!"; \
			slotObject->call(Private::SlotObject::Delete, nullptr, nullptr); \
			return Connection(); \
		} \
	} while (false)

Connection Object::connectInternal (
	const Object *sender,
	void **signal,
	const Object *receiver,
	void **slot,
	Private::SlotObject *slotObject
) {
	// Note: `receiver` can be null with non-member function slot.
	CHECK_CONNECT_PARAM(sender);
	CHECK_CONNECT_PARAM(signal);
	CHECK_CONNECT_PARAM(slot);
	CHECK_CONNECT_PARAM(slotObject);

	return Connection();
}

#undef CHECK_CONNECT_PARAM

LINPHONE_END_NAMESPACE
