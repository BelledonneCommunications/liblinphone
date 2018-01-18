/*
 * general.h
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

#ifndef _L_GENERAL_H_
#define _L_GENERAL_H_

#ifdef __cplusplus
	#include <tuple>
	#include <type_traits>
#endif

#include "linphone/utils/magic-macros.h"

// =============================================================================

#ifdef __cplusplus
	#define LINPHONE_BEGIN_NAMESPACE namespace LinphonePrivate {
	#define LINPHONE_END_NAMESPACE }
#else
	#define LINPHONE_BEGIN_NAMESPACE
	#define LINPHONE_END_NAMESPACE
#endif

// -----------------------------------------------------------------------------

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------
// Export.
// -----------------------------------------------------------------------------

#ifndef LINPHONE_PUBLIC
	#if defined(_MSC_VER)
		#ifdef LINPHONE_STATIC
			#define LINPHONE_PUBLIC
		#else
			#ifdef LINPHONE_EXPORTS
				#define LINPHONE_PUBLIC	__declspec(dllexport)
			#else
				#define LINPHONE_PUBLIC	__declspec(dllimport)
			#endif
		#endif
	#else
		#define LINPHONE_PUBLIC
	#endif
#endif

#ifndef LINPHONE_DEPRECATED
	#if defined(_MSC_VER)
		#define LINPHONE_DEPRECATED __declspec(deprecated)
	#else
		#define LINPHONE_DEPRECATED __attribute__((deprecated))
	#endif
#endif

// -----------------------------------------------------------------------------

#ifdef __cplusplus

// -----------------------------------------------------------------------------
// Debug.
// -----------------------------------------------------------------------------

void l_assert (const char *condition, const char *file, int line);

#ifdef DEBUG
	#define L_ASSERT(CONDITION) ((CONDITION) ? static_cast<void>(0) : LinphonePrivate::l_assert(#CONDITION, __FILE__, __LINE__))
#else
	#define L_ASSERT(CONDITION) static_cast<void>(false && (CONDITION))
#endif

// -----------------------------------------------------------------------------
// Optimization.
// -----------------------------------------------------------------------------

#ifndef _MSC_VER
	#define L_LIKELY(EXPRESSION) __builtin_expect(static_cast<bool>(EXPRESSION), true)
	#define L_UNLIKELY(EXPRESSION)  __builtin_expect(static_cast<bool>(EXPRESSION), false)
#else
	#define L_LIKELY(EXPRESSION) EXPRESSION
	#define L_UNLIKELY(EXPRESSION) EXPRESSION
#endif

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

// Define an integer version like: 0xXXYYZZ, XX=MAJOR, YY=MINOR, and ZZ=PATCH.
#define L_VERSION(MAJOR, MINOR, PATCH) (((MAJOR) << 16) | ((MINOR) << 8) | (PATCH))

#define L_AUTO_CONSTEXPR_RETURN(VALUE) -> decltype(VALUE) { return VALUE; }

// -----------------------------------------------------------------------------
// Data access.
// -----------------------------------------------------------------------------

class BaseObject;
class BaseObjectPrivate;
class ClonableObject;
class ClonableObjectPrivate;
class Object;
class ObjectPrivate;

#define L_INTERNAL_CHECK_OBJECT_INHERITANCE(CLASS) \
	static_assert( \
		!(std::is_base_of<BaseObject, CLASS>::value && std::is_base_of<ClonableObject, CLASS>::value), \
		"Multiple inheritance between BaseObject and ClonableObject is not allowed." \
	);

#define L_INTERNAL_GET_BETTER_PRIVATE_ANCESTOR(CLASS) \
	std::conditional< \
		std::is_base_of<BaseObject, CLASS>::value, \
		BaseObject, \
		std::conditional< \
			std::is_base_of<ClonableObject, CLASS>::value, \
			ClonableObject, \
			CLASS \
		>::type \
	>::type

#define L_INTERNAL_DECLARE_PRIVATE(CLASS) \
	inline CLASS ## Private *getPrivate () { \
		L_INTERNAL_CHECK_OBJECT_INHERITANCE(CLASS); \
		return reinterpret_cast<CLASS ## Private *>( \
			L_INTERNAL_GET_BETTER_PRIVATE_ANCESTOR(CLASS)::mPrivate \
		); \
	} \
	inline const CLASS ## Private *getPrivate () const { \
		L_INTERNAL_CHECK_OBJECT_INHERITANCE(CLASS); \
		return reinterpret_cast<const CLASS ## Private *>( \
			L_INTERNAL_GET_BETTER_PRIVATE_ANCESTOR(CLASS)::mPrivate \
		); \
	} \
	friend class CLASS ## Private; \
	friend class Wrapper;

// Allows access to private internal data.
// Gives a control to C Wrapper.
#ifndef LINPHONE_TESTER
	#define L_DECLARE_PRIVATE(CLASS) L_INTERNAL_DECLARE_PRIVATE(CLASS)
#else
	#define L_DECLARE_PRIVATE(CLASS) \
		L_INTERNAL_DECLARE_PRIVATE(CLASS) \
		friend class Tester;
#endif

// Generic public helper.
template<
	typename R,
	typename P,
	typename C
>
constexpr R *getPublicHelper (P *object, const C *) {
	return static_cast<R *>(object);
}

// Generic public helper. Deal with shared data.
template<
	typename R,
	typename P,
	typename C
>
inline R *getPublicHelper (const P &objectSet, const C *) {
	auto it = objectSet.cbegin();
	L_ASSERT(it != objectSet.cend());
	return static_cast<R *>(*it);
}

#define L_DECLARE_PUBLIC(CLASS) \
	inline CLASS *getPublic () { \
		return getPublicHelper<CLASS>(mPublic, this); \
	} \
	inline const CLASS *getPublic () const { \
		return getPublicHelper<const CLASS>(mPublic, this); \
	} \
	friend class CLASS;

#define L_DISABLE_COPY(CLASS) \
	CLASS (const CLASS &) = delete; \
	CLASS &operator= (const CLASS &) = delete;

// Get Private data.
#define L_D() decltype(getPrivate()) const d = getPrivate();

// Get Public data.
#define L_Q() decltype(getPublic()) const q = getPublic();

template<typename T, typename U>
struct AddConstMirror {
	typedef U type;
};

template<typename T, typename U>
struct AddConstMirror<const T, U> {
	typedef typename std::add_const<U>::type type;
};

// Get Private data of class in a multiple inheritance case.
#define L_D_T(CLASS, NAME) \
	auto const NAME = static_cast< \
		AddConstMirror< \
			std::remove_reference<decltype(*this)>::type, \
			CLASS ## Private \
		>::type * \
	>(CLASS::mPrivate);

// Get Private data of class in a multiple inheritance case.
#define L_Q_T(CLASS, NAME) \
	auto const NAME = static_cast< \
		AddConstMirror< \
			std::remove_reference<decltype(*this)>::type, \
			CLASS \
		>::type * \
	>(getPublic());

#define L_OVERRIDE_SHARED_FROM_THIS(CLASS) \
	inline std::shared_ptr<CLASS> getSharedFromThis () { \
		return std::static_pointer_cast<CLASS>(Object::getSharedFromThis()); \
	} \
	inline std::shared_ptr<const CLASS> getSharedFromThis () const { \
		return std::static_pointer_cast<const CLASS>(Object::getSharedFromThis()); \
	}

// -----------------------------------------------------------------------------
// Overload.
// -----------------------------------------------------------------------------

namespace Private {
	template<typename... Args>
	struct ResolveMemberFunctionOverload {
		template<typename Ret, typename Obj>
		constexpr auto operator() (Ret (Obj::*func)(Args...)) const -> decltype(func) {
			return func;
		}
	};

	template<typename... Args>
	struct ResolveConstMemberFunctionOverload {
		template<typename Ret, typename Obj>
		constexpr auto operator() (Ret (Obj::*func)(Args...) const) const -> decltype(func) {
			return func;
		}
	};

	template<typename... Args>
	struct ResolveOverload : ResolveConstMemberFunctionOverload<Args...>, ResolveMemberFunctionOverload<Args...> {
		using ResolveMemberFunctionOverload<Args...>::operator();
		using ResolveConstMemberFunctionOverload<Args...>::operator();

		template<typename Ret>
		constexpr auto operator() (Ret (*func)(Args...)) const -> decltype(func) {
			return func;
		}
	};
}

// Useful to select a specific overloaded function.
#define L_RESOLVE_OVERLOAD(...) \
	LinphonePrivate::Private::ResolveOverload<__VA_ARGS__>()

// -----------------------------------------------------------------------------
// Wrapper public.
// -----------------------------------------------------------------------------

#define L_DECL_C_STRUCT(STRUCT) typedef struct _ ## STRUCT STRUCT;
#define L_DECL_C_STRUCT_PREFIX_LESS(STRUCT) typedef struct STRUCT STRUCT;

// -----------------------------------------------------------------------------
// Index Sequence (C++11 impl).
// -----------------------------------------------------------------------------

template<std::size_t...>
struct IndexSequence {
	using type = IndexSequence;
};

namespace Private {
	template<class S1, class S2>
	struct ConcatSequence;

	template<std::size_t... S1, std::size_t... S2>
	struct ConcatSequence<IndexSequence<S1...>, IndexSequence<S2...>> :
		IndexSequence<S1..., (sizeof...(S1) + S2)...> {};

	template<std::size_t N>
	struct MakeIndexSequence : ConcatSequence<
		typename MakeIndexSequence<N / 2>::type,
		typename MakeIndexSequence<N - N / 2>::type
	> {};

	template<>
	struct MakeIndexSequence<0> : IndexSequence<> {};

	template<>
	struct MakeIndexSequence<1> : IndexSequence<0> {};
}

template<std::size_t N>
using MakeIndexSequence = typename Private::MakeIndexSequence<N>::type;

// -----------------------------------------------------------------------------
// Compile-time string.
// -----------------------------------------------------------------------------

namespace Private {
	constexpr bool equal (const char *a, const char *b) {
		return *a == *b ? (*a == '\0' || equal(a + 1, b + 1)) : false;
	}
}

// See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4121.pdf
template<std::size_t N> using RawStringLiteral = const char [N];

template<std::size_t N, typename = MakeIndexSequence<N>>
struct StringLiteral;

template<std::size_t N, std::size_t... Index>
struct StringLiteral<N, IndexSequence<Index...>> {
	constexpr StringLiteral (RawStringLiteral<N> &inRaw) : raw{ (inRaw[Index])... } {}

	constexpr char operator[] (std::size_t p) const {
		return raw[p];
	}

	template<std::size_t M>
	constexpr bool operator== (const StringLiteral<M> &other) const {
		return N != M ? false : Private::equal(raw, other.raw);
	}

	template<std::size_t M>
	constexpr bool operator!= (const StringLiteral<M> &other) const {
		return !(*this == other);
	}

	RawStringLiteral<N> raw;
};

template<std::size_t N>
constexpr StringLiteral<N> makeStringLiteral (RawStringLiteral<N> &raw) {
	return StringLiteral<N>(raw);
}

#endif // ifdef __cplusplus

LINPHONE_END_NAMESPACE

#endif // ifndef _L_GENERAL_H_
