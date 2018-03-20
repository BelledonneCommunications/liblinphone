/*
 * c-core.cpp
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

#include "linphone/wrapper_utils.h"

#include "c-wrapper/c-wrapper.h"
#include "core/core.h"

#include "private_structs.h"

#include "chat/encryption/lime-backwards-compatible.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void _linphone_core_constructor (LinphoneCore *lc);
static void _linphone_core_destructor (LinphoneCore *lc);

L_DECLARE_C_OBJECT_IMPL_WITH_XTORS(
	Core,
	_linphone_core_constructor, _linphone_core_destructor,
	LINPHONE_CORE_STRUCT_FIELDS
)

static void _linphone_core_constructor (LinphoneCore *lc) {
}

static void _linphone_core_destructor (LinphoneCore *lc) {
	if (lc->callsCache)
		bctbx_list_free_with_data(lc->callsCache, (bctbx_list_free_func)linphone_call_unref);
	_linphone_core_uninit(lc);
}

void linphone_core_set_im_encryption_engine (LinphoneCore *lc, LinphoneImEncryptionEngine *imee) {
    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setEncryptionEngine(new LimeBackwardsCompatible);

	if (lc->im_encryption_engine) {
		linphone_im_encryption_engine_unref(lc->im_encryption_engine);
		lc->im_encryption_engine = NULL;
	}
	if (imee) {
		imee->lc = lc;
		lc->im_encryption_engine = linphone_im_encryption_engine_ref(imee);
	}
}

void linphone_core_enable_lime_v2 (LinphoneCore *lc, bool_t enable) {
// 	if (L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeV2Available()) {
// 		cout << "LIMEv2 is available, enabling" << endl;
// 	} else {
// 		cout << "LIMEv2 is unavailable, not enabling" << endl;
// 	}
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableLimeV2(enable);
}

bool_t linphone_core_update_lime_v2 (const LinphoneCore *lc) {
	bool isUpdated = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->updateLimeV2();
	return isUpdated?TRUE:FALSE;
}

bool_t linphone_core_lime_v2_enabled (const LinphoneCore *lc) {
	bool isEnabled = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeV2Enabled();
	return isEnabled?TRUE:FALSE;
}

bool_t linphone_core_lime_v2_available (const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeV2Available();
}
