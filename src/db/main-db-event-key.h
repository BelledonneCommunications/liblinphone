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

#ifndef _L_MAIN_DB_EVENT_KEY_H_
#define _L_MAIN_DB_EVENT_KEY_H_

#include "main-db-key.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class EventLog;

class MainDbEventKey : public MainDbKey {
	friend class EventLog;
	friend class EventLogPrivate;

public:
	MainDbEventKey();
	MainDbEventKey(const std::shared_ptr<Core> &core, long long storageId);
	~MainDbEventKey();

	MainDbEventKey *clone() const override {
		return new MainDbEventKey(*this);
	}

private:
	L_DECLARE_PRIVATE(MainDbKey);
	void resetStorageId();
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAIN_DB_EVENT_KEY_H_
