/*
 * arguments.h
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

#ifndef _L_ARGUMENTS_H_
#define _L_ARGUMENTS_H_

#include "list.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

/*
 * Internal compile time consistent arguments checker.
 */
namespace Private {
	template<typename, typename>
	struct ArgsConsistent {
		enum {
			Value = false
		};
	};

	template<typename A>
	struct ArgsConsistent<A, A> {
		enum {
			Value = true
		};
	};

	template<typename A>
	struct ArgsConsistent<A &, A &> {
		enum {
			Value = true
		};
	};

	template<typename A>
	struct ArgsConsistent<void, A> {
		enum {
			Value = true
		};
	};

	template<typename A>
	struct ArgsConsistent<A, void> {
		enum {
			Value = true
		};
	};

	template<>
	struct ArgsConsistent<void, void> {
		enum {
			Value = true
		};
	};

	// ---------------------------------------------------------------------------

	template<typename, typename>
	struct ArgsListConsistent {
		enum {
			Value = false
		};
	};

	template<>
	struct ArgsListConsistent<List<>, List<>> {
		enum {
			Value = true
		};
	};

	template<typename L>
	struct ArgsListConsistent<L, List<>> {
		enum {
			Value = true
		};
	};

	template<typename Arg1, typename Arg2, typename... Tail1, typename... Tail2>
	struct ArgsListConsistent<List<Arg1, Tail1...>, List<Arg2, Tail2...>> {
		typedef ArgsConsistent<Arg1, Arg2> Check;
		typedef ArgsListConsistent<List<Tail1...>, List<Tail2...>> CheckList;

		enum {
			Value = Check::Value && CheckList::Value
		};
	};
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ARGUMENTS_H_
