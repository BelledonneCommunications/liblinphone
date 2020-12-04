/*
 * Copyright (c) 2020 Belledonne Communications SARL.
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

#ifdef HAVE_SOCI
#include <soci/soci.h>
#endif
#include "liblinphone_tester.h"
#include "logger/logger.h"
#include <exception>

/* */
void lime_delete_DRSessions(const char *limedb) {
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		// Delete all sessions from the DR_sessions table
		sql<<"DELETE FROM DR_sessions;";
	} catch (std::exception &e) { // swallow any error on DB
		lWarning()<<"Cannot delete DRSessions in base "<<limedb<<". Error is "<<e.what();
	}
#endif
}


