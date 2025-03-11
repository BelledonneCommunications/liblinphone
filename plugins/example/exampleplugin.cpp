/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "linphone/core.h"
#include "logger/logger.h"

LINPHONE_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT
#endif
PLUGIN_EXPORT void liblinphone_exampleplugin_init(LinphoneCore *core) {
	lInfo() << "Example plugin for core " << std::string(linphone_core_get_identity(core))
	        << " has been successfully loaded";
}

#ifdef __cplusplus
}
#endif

LINPHONE_END_NAMESPACE
