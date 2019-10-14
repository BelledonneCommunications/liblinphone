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

#ifndef _L_DB_SESSION_H_
#define _L_DB_SESSION_H_

#include <soci/soci.h>

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class DbSessionPrivate;

class DbSession {
public:
	DbSession ();
	explicit DbSession (const std::string &uri);
	DbSession (DbSession &&other);
	~DbSession ();

	DbSession &operator= (DbSession &&other);

	operator bool () const;

	soci::session *getBackendSession () const;

	std::string primaryKeyStr (const std::string &type = "INT") const;
	std::string primaryKeyRefStr (const std::string &type = "INT") const;
	std::string varcharPrimaryKeyStr (int length) const;

	std::string currentTimestamp () const;
	std::string timestampType () const;

	std::string noLimitValue () const;

	long long getLastInsertId () const;

	void enableForeignKeys (bool status);

	bool checkTableExists (const std::string &table) const;

	long long resolveId (const soci::row &row, int col) const;

	std::time_t getTime (const soci::row &row, int col) const;

private:
	DbSessionPrivate *mPrivate;

	L_DECLARE_PRIVATE(DbSession);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_DB_SESSION_H_
