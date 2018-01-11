/*
 * signal-emitter.h
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

#ifndef _L_SIGNAL_EMITTER_H_
#define _L_SIGNAL_EMITTER_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Private {
	void activateSignal (Object *sender, int signalIndex, void **args);

	template<typename Func, int Index>
	struct SignalEmitter {};

	template<typename Obj, typename... Args, int Index>
	struct SignalEmitter<void (Obj::*)(Args...), Index> {
		Obj *self;
		inline void operator () (Args... args) {
			void *argsPack[] = { (&args)... };
			activateSignal(self, Index, argsPack);
		}
	};
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SIGNAL_EMITTER_H_
