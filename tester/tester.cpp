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

#ifdef HAVE_SOCI
#include <soci/soci.h>
#endif
#include "liblinphone_tester.h"
#include "logger/logger.h"
#include <exception>

/* */
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void lime_delete_DRSessions(const char *limedb) {
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		// Delete all sessions from the DR_sessions table
		sql << "DELETE FROM DR_sessions;";
	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot delete DRSessions in base " << limedb << ". Error is " << e.what();
	}
#endif
}

void lime_setback_usersUpdateTs(const char *limedb, int days) {
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		// Set back in time the users updateTs by the given number of days
		sql << "UPDATE Lime_LocalUsers SET updateTs = date (updateTs, '-" << days << " day');";
	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot setback in time the lime users update ts on base " << limedb << ". Error is " << e.what();
	}
#endif
}
uint64_t lime_get_userUpdateTs(const char *limedb) {
	uint64_t ret = 0;
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		// get the users updateTs in unixepoch form - we may have more than one, just return the first one
		sql << "SELECT strftime('%s', updateTs) as t FROM Lime_LocalUsers LIMIT 1;", soci::into(ret);
	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot fetch the lime users update ts on base " << limedb << ". Error is " << e.what();
	}
#endif
	return ret;
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER
