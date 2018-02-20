/*
 * clonable-shared-pointer.h
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

#ifndef _L_CLONABLE_SHARED_POINTER_H_
#define _L_CLONABLE_SHARED_POINTER_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class SharedObject {
template<typename T>
friend class ClonableSharedPointer;

public:
	SharedObject () : mRefCounter(1) {};
	SharedObject (const SharedObject &) : SharedObject() {}
	virtual ~SharedObject () = default;

	virtual SharedObject *clone () = 0;

	int getRefCount () {
		return mRefCounter;
	}

private:
	int mRefCounter;

	L_DISABLE_ASSIGNMENT(SharedObject);
};

// -----------------------------------------------------------------------------

template<typename T>
class ClonableSharedPointer {
public:
	explicit ClonableSharedPointer (T *pointer = nullptr) : mPointer(pointer) {}

	ClonableSharedPointer (const ClonableSharedPointer &other) : mPointer(other.mPointer) {
		ref();
	}

	ClonableSharedPointer (ClonableSharedPointer &&other) : mPointer(other.mPointer) {
		other.mPointer = nullptr;
	}

	~ClonableSharedPointer () {
		unref();
	}

	ClonableSharedPointer &operator= (const ClonableSharedPointer &other) {
		if (mPointer != other.mPointer) {
			unref();
			mPointer = other.mPointer;
			ref();
		}
		return *this;
	}

	ClonableSharedPointer &operator= (ClonableSharedPointer &&other) {
		std::swap(mPointer, other.mPointer);
		return *this;
	}

	bool operator== (const ClonableSharedPointer &other) const {
		return mPointer == other.mPointer;
	}

	bool operator!= (const ClonableSharedPointer &other) const {
		return mPointer != other.mPointer;
	}

	T &operator* () {
		N_ASSERT(mPointer);
		tryClone();
		return *mPointer;
	}

	const T &operator* () const {
		N_ASSERT(mPointer);
		return *mPointer;
	}

	T *operator-> () {
		tryClone();
		return mPointer;
	}

	const T *operator-> () const {
		return mPointer;
	}

	explicit operator bool () const {
		return mPointer;
	}

	bool operator! () const {
		return !mPointer;
	}

	T *get () {
		tryClone();
		return mPointer;
	}

	const T *get () const {
		return mPointer;
	}

protected:
	T *mPointer;

private:
	void ref () {
		if (mPointer)
			++mPointer->mRefCounter;
	}

	void unref () {
		if (mPointer && --mPointer->mRefCounter == 0) {
			delete mPointer;
			mPointer = nullptr;
		}
	}

	void tryClone () {
		if (mPointer && mPointer->mRefCounter > 1) {
			--mPointer->mRefCounter;
			mPointer = mPointer->clone();
		}
	}
};

LINPHONE_END_NAMESPACE

#endif // _L_CLONABLE_SHARED_POINTER_H_
