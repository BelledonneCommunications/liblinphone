/*
 * main-db-event-key.h
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

#ifndef _L_MAIN_DB_EVENT_KEY_H_
#define _L_MAIN_DB_EVENT_KEY_H_

#include "main-db-key.h"
#include "main-db-key-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class MainDbEventKey : public MainDbKey {
	friend class ChatMessageKiller;
public:
	MainDbEventKey ();
	MainDbEventKey (const std::shared_ptr<Core> &core, long long storageId);
	~MainDbEventKey ();

	MainDbEventKey* clone () const override {
		return new MainDbEventKey(*this);
	}
	
	bool operator== (const MainDbEventKey &other) const;
	bool operator!= (const MainDbEventKey &other) const;
	bool operator< (const MainDbEventKey &other) const;
	
	const long long &getStorageId () const;

private:
	L_DECLARE_PRIVATE(MainDbKey);
};

LINPHONE_END_NAMESPACE

// Add map key support.
namespace std {
	template<>
	struct hash<LinphonePrivate::MainDbEventKey> {
		std::size_t operator() (const LinphonePrivate::MainDbEventKey &mainDbEventKey) const {
			return hash<long long>()(mainDbEventKey.getStorageId());
		}
	};
}

#endif // ifndef _L_MAIN_DB_EVENT_KEY_H_
