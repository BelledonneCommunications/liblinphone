/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _SAL_STREAM_BUNDLE_H_
#define _SAL_STREAM_BUNDLE_H_

#include "sal/sal_stream_description.h"

typedef struct SalStreamBundle{
	bctbx_list_t *mids; /* List of mids corresponding to streams associated in the bundle. The first one is the "tagged" one. */
} SalStreamBundle;

#endif // ifndef _SAL_STREAM_BUNDLE_H_
