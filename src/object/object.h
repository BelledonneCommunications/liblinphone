/*
 * object.h
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

#ifndef _L_OBJECT_H_
#define _L_OBJECT_H_

#include <memory>
#include <mutex>

#include "base-object.h"
#include "connection.h"
#include "internal/slot-object.h"
#include "property-container.h"

// =============================================================================

// -----------------------------------------------------------------------------
// Internal macros, do not use directly.
// -----------------------------------------------------------------------------

#define L_INTERNAL_SIGNAL_CONCAT_TYPE_ARG(TYPE, PARAM) TYPE PARAM

#define L_INTERNAL_CHECK_CONNECT_TYPES(SIGNAL_TYPE, SLOT_TYPE) \
	static_assert( \
		static_cast<int>(SIGNAL_TYPE::ArgumentsNumber) >= static_cast<int>(SLOT_TYPE::ArgumentsNumber), \
		"Slot requires less arguments." \
	); \
	static_assert( \
		(Private::ArgsListConsistent<typename SIGNAL_TYPE::Arguments, typename SLOT_TYPE::Arguments>::Value), \
		"Signal and slot args are not consistent." \
	); \
	static_assert( \
		(Private::ArgsConsistent<typename SLOT_TYPE::ReturnType, typename SIGNAL_TYPE::ReturnType>::Value), \
		"Return type of signal and slot are not consistent." \
	);

#define L_INTERNAL_SIGNAL_INDEX(NAME, LINE) L_CONCAT(lSignalIndexOf ## _, L_CONCAT(NAME ## _, LINE))

#define L_INTERNAL_STRINGIFY(NOOP, VALUE) #VALUE
#define L_INTERNAL_STRINGIFY_LIST(...) makeStringLiteralList(L_APPLY(L_INTERNAL_STRINGIFY, , __VA_ARGS__))

// -----------------------------------------------------------------------------
// Public macros API.
// -----------------------------------------------------------------------------

// Must be used in Object or ObjectPrivate.
#define L_SYNC() \
	static_assert( \
		!std::is_base_of<Object, decltype(this)>::value && !std::is_base_of<ObjectPrivate, decltype(this)>::value, \
		"Unable to lock. Instance is not an Object or ObjectPrivate." \
	); \
	const std::lock_guard<Object::Lock> synchronized(const_cast<Object::Lock &>(getLock()));

// Declare one Smart Object with Signals.
#define L_OBJECT(CLASS) \
	public: \
		typedef CLASS lType; \
		using lParentType = std::remove_reference< \
			decltype(LinphonePrivate::Private::getParentObject(&lType::getMetaObject)) \
		>::type; \
	private: \
		friend constexpr std::tuple<> lMetaSignals (LinphonePrivate::Private::MetaObjectCounter<0>, lType **) { return {}; } \
	public: \
		struct MetaObjectBuilder; \
		static const LinphonePrivate::MetaObject metaObject; \
		const MetaObject *getMetaObject () const override;

// Declare one signal method.
#define L_SIGNAL(NAME, TYPES, ...) \
	void NAME (L_APPLY_LIST(L_INTERNAL_SIGNAL_CONCAT_TYPE_ARG, TYPES, __VA_ARGS__)) { \
		typedef decltype(L_CALL(L_RESOLVE_OVERLOAD, TYPES)(&lType::NAME)) SignalType; \
		LinphonePrivate::Private::SignalEmitter<SignalType, L_INTERNAL_SIGNAL_INDEX(NAME, __LINE__)>{this}(__VA_ARGS__); \
	} \
	static constexpr int L_INTERNAL_SIGNAL_INDEX(NAME, __LINE__) = \
		std::tuple_size< \
			decltype(lMetaSignals(LinphonePrivate::Private::MetaObjectCounter<>(), static_cast<lType **>(nullptr))) \
		>::value; \
	friend constexpr auto lMetaSignals ( \
		LinphonePrivate::Private::MetaObjectCounter<L_INTERNAL_SIGNAL_INDEX(NAME, __LINE__) + 1> counter, \
		lType **context \
	) L_AUTO_CONSTEXPR_RETURN( \
		std::tuple_cat( \
			lMetaSignals(counter.prev(), context), \
			std::make_tuple(LinphonePrivate::Private::makeFunctionInfo( \
				L_CALL(L_RESOLVE_OVERLOAD, TYPES)(&lType::NAME), \
				#NAME, \
				L_INTERNAL_STRINGIFY_LIST(L_CALL(L_EXPAND_VARIADIC, TYPES)), \
				L_INTERNAL_STRINGIFY_LIST(__VA_ARGS__) \
			)) \
		) \
	)

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC MetaObject {
public:
	const char *getClassName () const;
	const MetaObject *getParent () const;
	int getSignalsNumber () const;
	int getSignalIndex (void **signal) const;

	static void activateSignal (Object *sender, const MetaObject *metaObject, int signalIndex, void **args);
};

namespace Private {
	// Compilation counter.
	template<int N = 255>
	struct MetaObjectCounter : MetaObjectCounter<N - 1> {
		static constexpr int value = N;
		static constexpr MetaObjectCounter<N - 1> prev () {
			return {};
		}
	};

	template<>
	struct MetaObjectCounter<0> {
		static constexpr int value = 0;
	};

	template<typename Func, int Index>
	struct SignalEmitter {};

	template<typename Obj, typename... Args, int Index>
	struct SignalEmitter<void (Obj::*)(Args...), Index> {
		Obj *self;
		inline void operator () (Args... args) {
			void *argsPack[] = { (&args)... };
			MetaObject::activateSignal(self, &Obj::metaObject, Index, argsPack);
		}
	};

	template<typename T>
	T &getParentObject (const MetaObject *(T::*)() const);
};

/*
 * Main Object of Linphone. Can be shared but is not Clonable.
 * Supports properties and shared from this.
 * Must be built with ObjectFactory.
 */
class LINPHONE_PUBLIC Object :
	public std::enable_shared_from_this<Object>,
	public BaseObject,
	public PropertyContainer {
	L_OBJECT(Object);

public:
	typedef std::recursive_mutex Lock;

	std::shared_ptr<Object> getSharedFromThis ();
	std::shared_ptr<const Object> getSharedFromThis () const;

	template<typename Func1, typename Func2>
	static typename std::enable_if<Private::FunctionPointer<Func2>::ArgumentsNumber >= 0, Connection>::type connect (
		const typename Private::FunctionPointer<Func1>::Object *sender,
		Func1 signal,
		Func2 slot
	) {
		typedef Private::FunctionPointer<Func1> SignalType;
		typedef Private::FunctionPointer<Func2> SlotType;

		L_INTERNAL_CHECK_CONNECT_TYPES(SignalType, SlotType)

		return connectInternal(
			sender, reinterpret_cast<void **>(&signal), sender, nullptr,
			new Private::SlotObjectFunction<
				Func2,
				typename Private::ListBuilder<typename SignalType::Arguments, SlotType::ArgumentsNumber>::Value
			>(slot),
			&SignalType::Object::metaObject
		);
	}

	template<typename Func1, typename Func2>
	static Connection connect (
		const typename Private::FunctionPointer<Func1>::Object *sender,
		Func1 signal,
		const typename Private::FunctionPointer<Func2>::Object *receiver,
		Func2 slot
	) {
		typedef Private::FunctionPointer<Func1> SignalType;
		typedef Private::FunctionPointer<Func2> SlotType;

		L_INTERNAL_CHECK_CONNECT_TYPES(SignalType, SlotType)

		return connectInternal(
			sender, reinterpret_cast<void **>(&signal), receiver, reinterpret_cast<void **>(&slot),
			new Private::SlotObjectMemberFunction<
				Func2,
				typename Private::ListBuilder<typename SignalType::Arguments, SlotType::ArgumentsNumber>::Value
			>(slot),
			&SignalType::Object::metaObject
		);
	}

	bool disconnect (const Connection &connection);

protected:
	explicit Object (ObjectPrivate &p);

	const Lock &getLock () const;

private:
	static Connection connectInternal (
		const Object *sender,
		void **signal,
		const Object *receiver,
		void **slot,
		Private::SlotObject *slotObject,
		const MetaObject *metaObject
	);

	L_DECLARE_PRIVATE(Object);
	L_DISABLE_COPY(Object);
};

LINPHONE_END_NAMESPACE

#undef L_INTERNAL_CHECK_CONNECT_TYPES

#endif // ifndef _L_OBJECT_H_
