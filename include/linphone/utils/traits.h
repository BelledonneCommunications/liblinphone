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

#ifndef _L_TRAITS_H_
#define _L_TRAITS_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Private {
	// See: http://en.cppreference.com/w/cpp/types/void_t
	template<typename... T> struct MakeVoid {
		typedef void type;
	};
	template<typename... T>
	using void_t = typename MakeVoid<T...>::type;

	template<typename T, typename U = void>
	struct IsMapContainerImpl : std::false_type {};

	template<typename T>
	struct IsMapContainerImpl<
		T,
		void_t<
			typename T::key_type,
			typename T::mapped_type,
			decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])
		>
	> : std::true_type {};
};

// Check if a type is a std container like map, unordered_map...
template<typename T>
struct IsMapContainer : Private::IsMapContainerImpl<T>::type {};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_TRAITS_H_
