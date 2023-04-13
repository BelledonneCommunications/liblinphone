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

#ifndef _L_ABSTRACT_DB_H_
#define _L_ABSTRACT_DB_H_

#include "object/object.h"
#include "utils/general-internal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractDbPrivate;

class LINPHONE_INTERNAL_PUBLIC AbstractDb : public Object {
public:
	enum Backend { Mysql, Sqlite3 };

	virtual ~AbstractDb() = default;

	/*
	 * Connect to the database, using specified backend.
	 * 'nameParams' must be start either with:
	 * - a file path if sqlite3 backend is used. If the path contains spaces, it must be enclosed within quotes.
	 * - a database name if using mysql.
	 * Then optional parameters can be added, in the form "param-name=param-value", separated with spaces.
	 * The meaning of these optional parameters is implementation dependant, refer to SOCI documentation for more
	 * details.
	 */
	bool connect(Backend backend, const std::string &nameParams);
	void disconnect();

	bool forceReconnect();

	Backend getBackend() const;

	virtual bool import(Backend backend, const std::string &parameters);

	bool isInitialized() const;
	/* This function is to initialize soci backends when used with static linking. */
	static void registerBackend(Backend backend);

protected:
	explicit AbstractDb(AbstractDbPrivate &p);

	virtual void init();

private:
	L_DECLARE_PRIVATE(AbstractDb);
	L_DISABLE_COPY(AbstractDb);
};

std::ostream &operator<<(std::ostream &os, AbstractDb::Backend b);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ABSTRACT_DB_H_
