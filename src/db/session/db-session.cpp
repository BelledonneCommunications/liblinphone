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

#include "linphone/utils/utils.h"

#include "db-session.h"
#include "logger/logger.h"
#include "sqlite3_bctbx_vfs.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class DbSessionPrivate {
public:
	enum class Backend { None, Mysql, Sqlite3 } backend = Backend::None;

	std::unique_ptr<soci::session> backendSession;
};

DbSession::DbSession() : mPrivate(new DbSessionPrivate) {
}

DbSession::DbSession(const string &uri) : DbSession() {
	try {
		L_D();
		auto sqlitePos = uri.find("sqlite3://");
		if (sqlitePos != std::string::npos) { // opening a sqlite3 db, force SOCI to use the bctbx_sqlite3_vfs
			// uri might be just the filepath, add a db= in front of it in that case
			std::string uriArgs{uri};
			if ((uri.find("db=") == std::string::npos) &&
			    (uri.find("dbname=") == std::string::npos)) { // db name parameter can be db= or dbname=
				// Add the db= prefix
				if (uriArgs[sqlitePos + 10] != '"') {        // file path is not enclosed in quotes, do it too
					uriArgs.insert(sqlitePos + 10, "db=\""); // insert just after "sqlite3://" position +10
					uriArgs.push_back('\"');
				} else { // file path is already enclosed in quotes
					uriArgs.insert(sqlitePos + 10,
					               "db="); // insert just after "sqlite3://" position +10, before the opening "
				}
			}
			uriArgs.append(" vfs=").append(BCTBX_SQLITE3_VFS);
			d->backendSession = makeUnique<soci::session>(uriArgs);
		} else {
			d->backendSession = makeUnique<soci::session>(uri);
		}
		d->backend = !uri.find("mysql") ? DbSessionPrivate::Backend::Mysql : DbSessionPrivate::Backend::Sqlite3;
	} catch (const exception &e) {
		lWarning() << "Unable to build db session with uri: " << e.what();
	}
}

DbSession::DbSession(DbSession &&other) : mPrivate(other.mPrivate) {
	other.mPrivate = nullptr;
}

DbSession::~DbSession() {
	delete mPrivate;
}

DbSession &DbSession::operator=(DbSession &&other) {
	std::swap(mPrivate, other.mPrivate);
	return *this;
}

DbSession::operator bool() const {
	L_D();
	return d->backend != DbSessionPrivate::Backend::None;
}

soci::session *DbSession::getBackendSession() const {
	L_D();
	return d->backendSession.get();
}

string DbSession::primaryKeyStr(const string &type) const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return " " + type + " AUTO_INCREMENT PRIMARY KEY";
		case DbSessionPrivate::Backend::Sqlite3:
			// See: ROWIDs and the INTEGER PRIMARY KEY
			// https://www.sqlite.org/lang_createtable.html
			return " INTEGER PRIMARY KEY ASC";
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

string DbSession::primaryKeyRefStr(const string &type) const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return " " + type;
		case DbSessionPrivate::Backend::Sqlite3:
			return " INTEGER";
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

string DbSession::varcharPrimaryKeyStr(int length) const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return " VARCHAR(" + Utils::toString(length) + ") PRIMARY KEY";
		case DbSessionPrivate::Backend::Sqlite3:
			return " VARCHAR(" + Utils::toString(length) + ") PRIMARY KEY";
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

string DbSession::currentTimestamp() const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return " CURRENT_TIMESTAMP";
		case DbSessionPrivate::Backend::Sqlite3: {
			// Ugly hack but Sqlite3 does not allow table alteration where we add a date column using a default value
			// of CURRENT_TIMESTAMP.
			const tm &now = Utils::getTimeTAsTm(std::time(nullptr));

			char buffer[128];
			snprintf(buffer, sizeof buffer, "'%d-%02d-%02d %02d:%02d:%02d'", now.tm_year + 1900, now.tm_mon + 1,
			         now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
			return buffer;
		}
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

string DbSession::timestampType() const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return " TIMESTAMP";
		case DbSessionPrivate::Backend::Sqlite3:
			return " DATE";
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

string DbSession::noLimitValue() const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return "9999999999999999999";
		case DbSessionPrivate::Backend::Sqlite3:
			return "-1";
		case DbSessionPrivate::Backend::None:
			return "";
	}

	L_ASSERT(false);
	return "";
}

long long DbSession::getLastInsertId() const {
	long long id = 0;

	L_D();

	string sql;
	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			sql = "SELECT LAST_INSERT_ID()";
			break;
		case DbSessionPrivate::Backend::Sqlite3:
			sql = "SELECT last_insert_rowid()";
			break;
		case DbSessionPrivate::Backend::None:
			break;
	}

	if (!sql.empty()) {
		*d->backendSession << sql, soci::into(id);
	}

	return id;
}

void DbSession::enableForeignKeys(bool status) {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			*d->backendSession << string("SET FOREIGN_KEY_CHECKS = ") + (status ? "1" : "0");
			break;
		case DbSessionPrivate::Backend::Sqlite3:
			*d->backendSession << string("PRAGMA foreign_keys = ") + (status ? "ON" : "OFF");
			break;
		case DbSessionPrivate::Backend::None:
			break;
	}
}

bool DbSession::checkTableExists(const string &table) const {
	L_D();

	soci::session *session = d->backendSession.get();
	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			*session << "SHOW TABLES LIKE :table", soci::use(table);
			return session->got_data();
		case DbSessionPrivate::Backend::Sqlite3:
			*session << "SELECT name FROM sqlite_master WHERE type='table' AND name=:table", soci::use(table);
			return session->got_data();
		case DbSessionPrivate::Backend::None:
			return false;
	}

	L_ASSERT(false);
	return false;
}

long long DbSession::resolveId(const soci::row &row, int col) const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return static_cast<long long>(row.get<unsigned long long>((std::size_t)col));
		case DbSessionPrivate::Backend::Sqlite3:
			return static_cast<long long>(row.get<int>((std::size_t)col));
		case DbSessionPrivate::Backend::None:
			return 0;
	}

	L_ASSERT(false);
	return 0;
}

unsigned int DbSession::getUnsignedInt(const soci::row &row, const std::size_t col, const unsigned int def) const {
	L_D();

	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return row.get<unsigned int>(col, def);
		case DbSessionPrivate::Backend::Sqlite3:
			return static_cast<unsigned int>(row.get<int>(col, static_cast<int>(def)));
		case DbSessionPrivate::Backend::None:
			return 0;
	}

	L_ASSERT(false);
	return 0;
}

time_t DbSession::getTime(const soci::row &row, int col) const {
	L_D();

	if (row.get_indicator(size_t(col)) == soci::i_null) {
		return -1;
	}
	tm t = row.get<tm>((size_t)col);
	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			return mktime(&t); // Local time to UTC. For Mysql the local time is used, not a problem.
		case DbSessionPrivate::Backend::Sqlite3:
			t.tm_isdst = 0; // Sqlite3 = UTC. Soci can set the DST field, it's not good for us...
			return Utils::getTmAsTimeT(t);
		case DbSessionPrivate::Backend::None:
			return 0;
	}

	L_ASSERT(false);
	return 0;
}

pair<tm, soci::indicator> DbSession::getTimeWithSociIndicator(time_t t) const {
	L_D();
	time_t time = (t <= 0) ? 0 : t;
	tm dateTime = {0};
	auto indicator = soci::i_ok;
	switch (d->backend) {
		case DbSessionPrivate::Backend::Mysql:
			// The backend MySql uses the TIMESTAMP datatype that needs the time information in the current timezone:
			// https://dev.mysql.com/doc/refman/8.4/en/datetime.html MySQL converts TIMESTAMP values from the current
			// time zone to UTC for storage, and back from UTC to the current time zone for retrieval.
			dateTime = *std::localtime(&time);
			indicator = (t < 0) ? soci::i_null : soci::i_ok;
			break;
		case DbSessionPrivate::Backend::Sqlite3:
			dateTime = Utils::getTimeTAsTm(time);
			indicator = (t < 0) ? soci::i_null : soci::i_ok;
			break;
		case DbSessionPrivate::Backend::None:
			dateTime = Utils::getTimeTAsTm(time);
			indicator = soci::i_ok;
			break;
	}
	pair<tm, soci::indicator> dataInDb(dateTime, indicator);
	return dataInDb;
}

LINPHONE_END_NAMESPACE
