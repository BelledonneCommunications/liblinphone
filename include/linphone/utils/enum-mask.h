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

#ifndef _L_ENUM_MASK_H_
#define _L_ENUM_MASK_H_

#include <initializer_list>

#include "linphone/utils/general.h"

// =============================================================================

/*
 * The purpose of this class seems to be able to manipulate flag values that are declared as part of an enum.
 * The C++ compiler forbids OR/AND operator on enums, however this can be easily workaround by either not using enum for mask but constexpr
 * or by doing a basic cast to unsigned int.
 * There are below 130 hardly readable lines of code for class doing this, but ~operator doesn't work as expected.
 * (just try a &= EnumValue; to convince). TODO: question the interest of keeping this class and switch to simple const unsigned int.
 */
LINPHONE_BEGIN_NAMESPACE

template<typename T>
class EnumMask {
	static_assert(std::is_enum<T>::value, "EnumMask must be used with enum type. Logic no?");
	static_assert(sizeof(T) <= sizeof(int), "EnumMask supports only int, short or char values.");

public:
	// EnumMask's type. Can be int or unsigned int.
	// See: http://en.cppreference.com/w/cpp/types/underlying_type
	typedef typename std::conditional<
		std::is_signed<typename std::underlying_type<T>::type>::value, int, unsigned int
	>::type StorageType;

	constexpr EnumMask (int mask = 0) : mMask(StorageType(mask)) {}
	constexpr EnumMask (T value) : mMask(StorageType(value)) {}
	constexpr EnumMask (std::initializer_list<T> mask) : mMask(init(mask.begin(), mask.end())) {}

	constexpr operator StorageType () const {
		return mMask;
	}

	constexpr bool isSet (T value) const {
		return isSet(StorageType(value));
	}

	inline EnumMask &set (T value) {
		*this |= value;
		return *this;
	}

	inline EnumMask &unset (T value) {
		*this &= ~StorageType(value);
		return *this;
	}

	constexpr bool operator! () const {
		return !mMask;
	}

	inline EnumMask &operator&= (int mask) {
		mMask &= mask;
		return *this;
	}

	inline EnumMask &operator&= (unsigned int mask) {
		mMask &= mask;
		return *this;
	}

	inline EnumMask &operator&= (T mask) {
		mMask &= StorageType(mask);
		return *this;
	}

	inline EnumMask &operator|= (EnumMask mask) {
		mMask |= mask.mMask;
		return *this;
	}

	inline EnumMask &operator|= (T mask) {
		mMask |= StorageType(mask);
		return *this;
	}

	inline EnumMask &operator^= (EnumMask mask) {
		mMask ^= mask.mMask;
		return *this;
	}

	inline EnumMask &operator^= (T mask) {
		mMask ^= StorageType(mask);
		return *this;
	}

	constexpr EnumMask operator& (int mask) const {
		return mMask & mask;
	}

	constexpr EnumMask operator& (unsigned int mask) const {
		return mMask & mask;
	}

	constexpr EnumMask operator& (T mask) const {
		return int(mMask & StorageType(mask));
	}

	constexpr EnumMask operator| (EnumMask mask) const {
		return mMask | mask.mMask;
	}

	constexpr EnumMask operator| (T mask) const {
		return int(mMask | StorageType(mask));
	}

	constexpr EnumMask operator^ (EnumMask mask) const {
		return mMask ^ mask.mMask;
	}

	constexpr EnumMask operator^ (T mask) const {
		return int(mMask ^ StorageType(mask));
	}

	constexpr EnumMask operator~ () const {
		return ~mMask;
	}

private:
	constexpr bool isSet (StorageType value) const {
		return (mMask & value) == value && (value || mMask == 0);
	}

// GCC versions prior to 5.0 have issues with array-bounds.
#if defined(__GNUC__) && (__GNUC__ < 5)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

	static constexpr StorageType init (
		typename std::initializer_list<T>::const_iterator begin,
		typename std::initializer_list<T>::const_iterator end
	) {
		return begin != end ? (StorageType(*begin) | init(begin + 1, end)) : StorageType(0);
	}

#if defined(__GNUC__) && (__GNUC__ < 5)
	#pragma GCC diagnostic pop
#endif

	StorageType mMask;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ENUM_MASK_H_
