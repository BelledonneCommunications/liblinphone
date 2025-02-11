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

#include "main-db-event-key.h"
#include "core/core-p.h"
#include "main-db-key-p.h"
#include "main-db-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

MainDbEventKey::MainDbEventKey() : MainDbKey() {};

MainDbEventKey::MainDbEventKey(const shared_ptr<Core> &core, long long storageId) : MainDbKey(core, storageId) {
}

MainDbEventKey::~MainDbEventKey() {
	resetStorageId();
}

void MainDbEventKey::resetStorageId() {
	L_D();

	if (isValid()) {
		auto core = d->core.lock();
		if (core) {
			auto &db = core->getPrivate()->mainDb;
			if (db) {
				db->getPrivate()->storageIdToEvent.erase(d->storageId);
			}
		}
	}

	d->storageId = -1;
}

LINPHONE_END_NAMESPACE
