/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_MAGIC_MACROS_H_
#define _L_MAGIC_MACROS_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

// Concat in depth context.
#define L_CONCAT__(A, B) A##B
#define L_CONCAT_(A, B) L_CONCAT__(A, B)
#define L_CONCAT(A, B) L_CONCAT_(A, B)

// Get argument numbers from variadic.
#define L_ARG_N(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, N,     \
                ...)                                                                                                   \
	N

#define L_EXPAND(X) X

// Useful in depth context.
#define L_STRINGIFY(X) #X

#define L_GET_N_ARGS(...)                                                                                              \
	L_EXPAND(L_ARG_N(__VA_ARGS__, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// Get argument numbers - 1 from variadic.
#define L_GET_N_ARGS_SUB(X, ...) L_GET_N_ARGS(__VA_ARGS__)

// Get argument from variadic macro.
#define L_GET_ARG_1(A1, ...) A1
#define L_GET_ARG_2(A1, A2, ...) A2
#define L_GET_ARG_3(A1, A2, A3, ...) A3
#define L_GET_ARG_4(A1, A2, A3, A4, ...) A4
#define L_GET_ARG_5(A1, A2, A3, A4, A5, ...) A5
#define L_GET_ARG_6(A1, A2, A3, A4, A5, A6, ...) A6
#define L_GET_ARG_7(A1, A2, A3, A4, A5, A6, A7, ...) A7
#define L_GET_ARG_8(A1, A2, A3, A4, A5, A6, A7, A8, ...) A8
#define L_GET_ARG_9(A1, A2, A3, A4, A5, A6, A7, A8, A9, ...) A9
#define L_GET_ARG_10(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, ...) A10
#define L_GET_ARG_11(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, ...) A11
#define L_GET_ARG_12(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, ...) A12
#define L_GET_ARG_13(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, ...) A13
#define L_GET_ARG_14(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, ...) A14
#define L_GET_ARG_15(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, ...) A15
#define L_GET_ARG_16(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, ...) A16
#define L_GET_ARG_17(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, ...) A17
#define L_GET_ARG_18(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, ...) A18
#define L_GET_ARG_19(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, ...) A19
#define L_GET_ARG_20(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, ...) A20
#define L_GET_ARG_21(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21,   \
                     ...)                                                                                              \
	A21

// Get left part of variadic.
#define L_GET_HEAP_1(A1, ...) A1
#define L_GET_HEAP_2(A1, A2, ...) A1, A2
#define L_GET_HEAP_3(A1, A2, A3, ...) A1, A2, A3
#define L_GET_HEAP_4(A1, A2, A3, A4, ...) A1, A2, A3, A4
#define L_GET_HEAP_5(A1, A2, A3, A4, A5, ...) A1, A2, A3, A4, A5
#define L_GET_HEAP_6(A1, A2, A3, A4, A5, A6, ...) A1, A2, A3, A4, A5, A6
#define L_GET_HEAP_7(A1, A2, A3, A4, A5, A6, A7, ...) A1, A2, A3, A4, A5, A6, A7
#define L_GET_HEAP_8(A1, A2, A3, A4, A5, A6, A7, A8, ...) A1, A2, A3, A4, A5, A6, A7, A8
#define L_GET_HEAP_9(A1, A2, A3, A4, A5, A6, A7, A8, A9, ...) A1, A2, A3, A4, A5, A6, A7, A8, A9
#define L_GET_HEAP_10(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, ...) A1, A2, A3, A4, A5, A6, A7, A8, A9, A10
#define L_GET_HEAP_11(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, ...) A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11
#define L_GET_HEAP_12(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, ...)                                          \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12
#define L_GET_HEAP_13(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, ...)                                     \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13
#define L_GET_HEAP_14(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, ...)                                \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14
#define L_GET_HEAP_15(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, ...)                           \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#define L_GET_HEAP_16(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, ...)                      \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16
#define L_GET_HEAP_17(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, ...)                 \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17
#define L_GET_HEAP_18(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, ...)            \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18
#define L_GET_HEAP_19(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, ...)       \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19
#define L_GET_HEAP_20(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, ...)  \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20
#define L_GET_HEAP_21(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21,  \
                      ...)                                                                                             \
	A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21

#define L_GET_HEAP(...) L_EXPAND(L_CONCAT(L_GET_HEAP_, L_GET_N_ARGS_SUB(__VA_ARGS__))(__VA_ARGS__))

// Call a macro on args.
#define L_CALL(MACRO, ARGS) MACRO ARGS
#define L_CALL_HELPER(MACRO, ARGS) MACRO ARGS

// Map each variadic args.
#define L_APPLY_1(MACRONAME, DATA, A1) L_CALL_HELPER(MACRONAME, (DATA, A1))

#define L_APPLY_2(MACRONAME, DATA, A1, A2) L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_1(MACRONAME, DATA, A2)

#define L_APPLY_3(MACRONAME, DATA, A1, A2, A3) L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_2(MACRONAME, DATA, A2, A3)

#define L_APPLY_4(MACRONAME, DATA, A1, A2, A3, A4)                                                                     \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_3(MACRONAME, DATA, A2, A3, A4)

#define L_APPLY_5(MACRONAME, DATA, A1, A2, A3, A4, A5)                                                                 \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_4(MACRONAME, DATA, A2, A3, A4, A5)

#define L_APPLY_6(MACRONAME, DATA, A1, A2, A3, A4, A5, A6)                                                             \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_5(MACRONAME, DATA, A2, A3, A4, A5, A6)

#define L_APPLY_7(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7)                                                         \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_6(MACRONAME, DATA, A2, A3, A4, A5, A6, A7)

#define L_APPLY_8(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8)                                                     \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_7(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8)

#define L_APPLY_9(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9)                                                 \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_8(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9)

#define L_APPLY_10(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)                                           \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_9(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10)

#define L_APPLY_11(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)                                      \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_10(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)

#define L_APPLY_12(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)                                 \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_11(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)

#define L_APPLY_13(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)                            \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_12(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)

#define L_APPLY_14(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)                       \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_13(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)

#define L_APPLY_15(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)                  \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_14(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)

#define L_APPLY_16(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16)             \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_15(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16)

#define L_APPLY_17(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17)        \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_16(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17)

#define L_APPLY_18(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18)   \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_17(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18)

#define L_APPLY_19(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18,   \
                   A19)                                                                                                \
	L_CALL_HELPER(MACRONAME, (DATA, A1)),                                                                              \
	    L_APPLY_18(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19)

#define L_APPLY_20(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18,   \
                   A19, A20)                                                                                           \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_19(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12,   \
	                                                 A13, A14, A15, A16, A17, A18, A19, A20)

#define L_APPLY_21(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18,   \
                   A19, A20, A21)                                                                                      \
	L_CALL_HELPER(MACRONAME, (DATA, A1)), L_APPLY_20(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12,   \
	                                                 A13, A14, A15, A16, A17, A18, A19, A20, A21)

#define L_APPLY(MACRONAME, DATA, ...)                                                                                  \
	L_CALL(L_CONCAT(L_APPLY_, L_GET_N_ARGS(__VA_ARGS__)), (MACRONAME, DATA, __VA_ARGS__))

// Map each variadic args without comma separator.
#define L_APPLY_WITHOUT_COMMA_1(MACRONAME, DATA, A1) L_CALL_HELPER(MACRONAME, (DATA, A1))

#define L_APPLY_WITHOUT_COMMA_2(MACRONAME, DATA, A1, A2)                                                               \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_1(MACRONAME, DATA, A2)

#define L_APPLY_WITHOUT_COMMA_3(MACRONAME, DATA, A1, A2, A3)                                                           \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_2(MACRONAME, DATA, A2, A3)

#define L_APPLY_WITHOUT_COMMA_4(MACRONAME, DATA, A1, A2, A3, A4)                                                       \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_3(MACRONAME, DATA, A2, A3, A4)

#define L_APPLY_WITHOUT_COMMA_5(MACRONAME, DATA, A1, A2, A3, A4, A5)                                                   \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_4(MACRONAME, DATA, A2, A3, A4, A5)

#define L_APPLY_WITHOUT_COMMA_6(MACRONAME, DATA, A1, A2, A3, A4, A5, A6)                                               \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_5(MACRONAME, DATA, A2, A3, A4, A5, A6)

#define L_APPLY_WITHOUT_COMMA_7(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7)                                           \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_6(MACRONAME, DATA, A2, A3, A4, A5, A6, A7)

#define L_APPLY_WITHOUT_COMMA_8(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8)                                       \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_7(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8)

#define L_APPLY_WITHOUT_COMMA_9(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9)                                   \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_8(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9)

#define L_APPLY_WITHOUT_COMMA_10(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)                             \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_9(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10)

#define L_APPLY_WITHOUT_COMMA_11(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)                        \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_10(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)

#define L_APPLY_WITHOUT_COMMA_12(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)                   \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_11(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)

#define L_APPLY_WITHOUT_COMMA_13(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)              \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_12(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)

#define L_APPLY_WITHOUT_COMMA_14(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)         \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_13(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)

#define L_APPLY_WITHOUT_COMMA_15(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)    \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_14(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)

#define L_APPLY_WITHOUT_COMMA_16(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16)                                                                                  \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_15(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16)

#define L_APPLY_WITHOUT_COMMA_17(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16, A17)                                                                             \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_16(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17)

#define L_APPLY_WITHOUT_COMMA_18(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16, A17, A18)                                                                        \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_17(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17,  \
	                         A18)

#define L_APPLY_WITHOUT_COMMA_19(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16, A17, A18, A19)                                                                   \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_18(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17,  \
	                         A18, A19)

#define L_APPLY_WITHOUT_COMMA_20(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16, A17, A18, A19, A20)                                                              \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_19(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17,  \
	                         A18, A19, A20)

#define L_APPLY_WITHOUT_COMMA_21(MACRONAME, DATA, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15,    \
                                 A16, A17, A18, A19, A20, A21)                                                         \
	L_CALL_HELPER(MACRONAME, (DATA, A1))                                                                               \
	L_APPLY_WITHOUT_COMMA_20(MACRONAME, DATA, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17,  \
	                         A18, A19, A20, A21)

#define L_APPLY_WITHOUT_COMMA(MACRONAME, DATA, ...)                                                                    \
	L_CALL(L_CONCAT(L_APPLY_WITHOUT_COMMA_, L_GET_N_ARGS(__VA_ARGS__)), (MACRONAME, DATA, __VA_ARGS__))

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAGIC_MACROS_H_
