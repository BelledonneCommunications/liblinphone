/*
 * function-pointer.h
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

#ifndef _L_FUNCTION_POINTER_H_
#define _L_FUNCTION_POINTER_H_

#include "function-call.h"

// =============================================================================

#define MEMBER_FUNCTION(QUALIFIER) \
	template<typename Obj, typename Ret, typename... Args> \
	struct FunctionPointer<Ret (Obj::*)(Args...) QUALIFIER> { \
		typedef Obj Object; \
		typedef Ret ReturnType; \
		typedef Ret (Obj::*Function)(Args...) QUALIFIER; \
		typedef List<Args...> Arguments; \
		enum { \
			ArgumentsNumber = sizeof ...(Args), \
			IsMemberFunction = true \
		}; \
		template<typename SignalArgs> \
		static void call (Function function, Obj *object, void **args) { \
			FunctionCall<typename Indexes<ArgumentsNumber>::Value, SignalArgs, Function>::call(function, object, args); \
		} \
	}

LINPHONE_BEGIN_NAMESPACE

/*
 * Internal compile time function pointer object.
 */
namespace Private {
	// Not a valid function.
	template<typename Func>
	struct FunctionPointer {
		enum {
			ArgumentsNumber = -1,
			IsMemberFunction = false
		};
	};

	// A non-member function.
	template<typename Ret, typename... Args>
	struct FunctionPointer<Ret (*)(Args...)> {
		typedef Ret ReturnType;
		typedef Ret (*Function)(Args...);
		typedef List<Args...> Arguments;

		enum {
			ArgumentsNumber = sizeof ...(Args),
			IsMemberFunction = false
		};

		template<typename SignalArgs>
		static void call (Function function, void *, void **args) {
			FunctionCall<typename Indexes<ArgumentsNumber>::Value, SignalArgs, Function>::call(function, args);
		}
	};

	// A member function.
	MEMBER_FUNCTION();

	// A member function with const qualifier.
	MEMBER_FUNCTION(const);
}

LINPHONE_END_NAMESPACE

#undef MEMBER_FUNCTION

#endif // ifndef _L_FUNCTION_POINTER_H_
