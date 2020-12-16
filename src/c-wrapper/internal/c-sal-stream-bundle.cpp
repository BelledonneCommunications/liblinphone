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

#include "c-wrapper/internal/c-tools.h"
#include "c-wrapper/internal/c-sal-stream-bundle.h"

SalStreamBundle *sal_stream_bundle_new(void){
	return ms_new0(SalStreamBundle, 1);
}

void sal_stream_bundle_add_stream(SalStreamBundle *bundle, SalStreamDescription *stream, const char *mid){
	stream->mid = L_C_TO_STRING(mid);
	bundle->mids = bctbx_list_append(bundle->mids, ms_strdup(mid));
}

void sal_stream_bundle_destroy(SalStreamBundle *bundle){
	bctbx_list_free_with_data(bundle->mids, (void (*)(void*)) ms_free);
	ms_free(bundle);
}

SalStreamBundle *sal_stream_bundle_clone(const SalStreamBundle *bundle){
	SalStreamBundle *ret = sal_stream_bundle_new();
	ret->mids = bctbx_list_copy_with_data(bundle->mids, (bctbx_list_copy_func)bctbx_strdup);
	return ret;
}

const char *sal_stream_bundle_get_mid_of_transport_owner(const SalStreamBundle *bundle){
	return (const char*)bundle->mids->data; /* the first one is the transport owner*/
}
