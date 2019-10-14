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

#ifndef LINPHONE_DEFS_H_
#define LINPHONE_DEFS_H_


#include "mediastreamer2/mscommon.h"


#define LINPHONE_IPADDR_SIZE 64
#define LINPHONE_HOSTNAME_SIZE 128

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
#define LINPHONE_DEPRECATED MS2_DEPRECATED
#endif


#endif /* LINPHONE_DEFS_H_ */
