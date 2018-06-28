/*
 * encryption-engine-enums.h
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

#ifndef _L_ENCRYPTION_ENGINE_ENUMS_H_
#define _L_ENCRYPTION_ENGINE_ENUMS_H_

// =============================================================================

#define L_ENUM_VALUES_ENCRYPTION_ENGINE_SECURITY_LEVEL(F) \
	F(Unsafe /**< Fail */) \
	F(ClearText /**< No encryption */) \
	F(Encrypted /**< Encrypted */) \
	F(Safe /**< Encrypted and Verified */)

#endif // ifndef _L_ENCRYPTION_ENGINE_ENUMS_H_
