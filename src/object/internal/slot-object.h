/*
 * slot-object.h
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

#ifndef _L_SLOT_OBJECT_H_
#define _L_SLOT_OBJECT_H_

#include "arguments.h"
#include "function-pointer.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Private {
	// ---------------------------------------------------------------------------
	// Abstract slot object.
	// ---------------------------------------------------------------------------

	class SlotObject {
	public:
		enum Action {
			Call,
			Delete
		};

	private:
		typedef void (*CallFunction)(Action action, SlotObject *slotObject, Object *receiver, void **args);

	public:
		explicit SlotObject (CallFunction function) : mCallFunction(function) {}

		inline void call (Action action, Object *receiver, void **args) {
			mCallFunction(action, this, receiver, args);
		}

	protected:
		~SlotObject () = default;

	private:
		const CallFunction mCallFunction;

		L_DISABLE_COPY(SlotObject)
	};

	// ---------------------------------------------------------------------------
	// Slot object with non-member function.
	// ---------------------------------------------------------------------------

	template<typename Func, typename Args>
	class SlotObjectFunction : public SlotObject {
	public:
		explicit SlotObjectFunction (Func function) : SlotObject(&callImpl), mFunction(function) {}

	private:
		typedef Private::FunctionPointer<Func> FuncType;

		static void callImpl (Action action, SlotObject *slotObject, Object *receiver, void **args) {
			SlotObjectFunction *slotObjectFunction = static_cast<SlotObjectFunction *>(slotObject);
			switch (action) {
				case Call:
					FuncType::template call<Args>(slotObjectFunction->mFunction, receiver, args);
					break;
				case Delete:
					delete slotObjectFunction;
			}
		}

		Func mFunction;

		L_DISABLE_COPY(SlotObjectFunction)
	};

	// ---------------------------------------------------------------------------
	// Slot object with member function.
	// ---------------------------------------------------------------------------

	template<typename Func, typename Args>
	class SlotObjectMemberFunction : public SlotObject {
	public:
		explicit SlotObjectMemberFunction (Func function) : SlotObject(&callImpl), mFunction(function) {}

	private:
		typedef Private::FunctionPointer<Func> FuncType;

		static void callImpl (Action action, SlotObject *slotObject, Object *receiver, void **args) {
			SlotObjectMemberFunction *slotObjectMemberFunction = static_cast<SlotObjectMemberFunction *>(slotObject);
			switch (action) {
				case Call:
					FuncType::template call<Args>(
						slotObjectMemberFunction->mFunction,
						static_cast<typename FuncType::Object *>(receiver),
						args
					);
					break;
				case Delete:
					delete slotObjectMemberFunction;
					break;
			}
		}

		Func mFunction;

		L_DISABLE_COPY(SlotObjectMemberFunction)
	};
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SLOT_OBJECT_H_
