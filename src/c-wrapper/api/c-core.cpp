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
#include "linphone/utils/utils.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core.h"

#include "private_structs.h"

#include "chat/encryption/encryption-engine.h"

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
	lc->state = LinphoneGlobalOff;
}

static void _linphone_core_destructor (LinphoneCore *lc) {
	if (lc->callsCache)
		bctbx_list_free_with_data(lc->callsCache, (bctbx_list_free_func)linphone_call_unref);
	_linphone_core_uninit(lc);
}

void linphone_core_set_im_encryption_engine (LinphoneCore *lc, LinphoneImEncryptionEngine *imee) {
	if (lc->im_encryption_engine) {
		linphone_im_encryption_engine_unref(lc->im_encryption_engine);
		lc->im_encryption_engine = NULL;
	}
	if (imee) {
		imee->lc = lc;
		lc->im_encryption_engine = linphone_im_encryption_engine_ref(imee);
	}
}

void linphone_core_enable_lime_x3dh (LinphoneCore *lc, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enableLimeX3dh(enable ? true : false);
}

bool_t linphone_core_lime_x3dh_enabled (const LinphoneCore *lc) {
	bool isEnabled = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhEnabled();
	return isEnabled ? TRUE : FALSE;
}

bool_t linphone_core_lime_x3dh_available (const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->limeX3dhAvailable();
}

void linphone_core_set_lime_x3dh_server_url(LinphoneCore *lc, const char *url) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setX3dhServerUrl(L_C_TO_STRING(url));
}

void linphone_core_delete_local_encryption_db (const LinphoneCore *lc) {
	auto encryptionEngine = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getEncryptionEngine();
	if (encryptionEngine)
		encryptionEngine->cleanDb();
}

//Deprecated
const char *linphone_core_get_linphone_specs (const LinphoneCore *lc) {
	return lp_config_get_string(linphone_core_get_config(lc), "sip", "linphone_specs", NULL);
}

//Deprecated
void linphone_core_set_linphone_specs (LinphoneCore *lc, const char *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecs(Utils::cStringToCppString(specs));
}

void linphone_core_set_linphone_specs_list (LinphoneCore *lc, const bctbx_list_t *specs) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecsList(L_GET_CPP_LIST_FROM_C_LIST(specs, const char *, string));
}

void linphone_core_add_linphone_spec (LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addSpec(Utils::cStringToCppString(spec));
}

void linphone_core_remove_linphone_spec (LinphoneCore *lc, const char *spec) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->removeSpec(Utils::cStringToCppString(spec));
}

const bctbx_list_t *linphone_core_get_linphone_specs_list (LinphoneCore *lc) {
	return L_GET_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getSpecsList());
}
