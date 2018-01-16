/*
 * function-call.h
 * Copyright (C) 2010-2017 Belledonne Communications SARL
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

#ifndef _L_FUNCTION_CALL_H_
#define _L_FUNCTION_CALL_H_

#include "list.h"

// =============================================================================

#define MEMBER_CALL(QUALIFIER) \
	template<int... Index, typename... SignalArgs, typename... SlotArgs, typename Ret, typename Obj> \
	struct FunctionCall<IndexesList<Index...>, List<SignalArgs...>, Ret (Obj::*)(SlotArgs...) QUALIFIER> { \
		static void call (Ret (Obj::*function)(SlotArgs...) QUALIFIER, Obj *object, void **args) { \
			(object->*function)((*reinterpret_cast<typename std::remove_reference<SignalArgs>::type *>(args[Index])) ...); \
		} \
	};

LINPHONE_BEGIN_NAMESPACE

/*
 * Internal compile time function call object.
 */
namespace Private {
	template<int...>
	struct IndexesList {};

	template<typename, int>
	struct IndexesAppend;

	template<int... Left, int Right>
	struct IndexesAppend<IndexesList<Left...>, Right> {
		typedef IndexesList<Left..., Right> Value;
	};

	template<int N>
	struct Indexes {
		typedef typename IndexesAppend<typename Indexes<N - 1>::Value, N - 1>::Value Value;
	};

	template<>
	struct Indexes<0> {
		typedef IndexesList<> Value;
	};

	template<typename, typename, typename>
	struct FunctionCall;

	// Call to non-member function.
	template<int... Index, typename... SignalArgs, typename Func>
	struct FunctionCall<IndexesList<Index...>, List<SignalArgs...>, Func> {
		static void call (Func &function, void **args) {
			function((*reinterpret_cast<typename std::remove_reference<SignalArgs>::type *>(args[Index]))...);
		}
	};

	// Call to member function.
	MEMBER_CALL();

	// Call to member function with const qualifier.
	MEMBER_CALL(const);
}

LINPHONE_END_NAMESPACE

#undef MEMBER_CALL

#endif // ifndef _L_FUNCTION_CALL_H_
