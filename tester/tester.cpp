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

#include <array>
#include <exception>
#include <string>
#include <vector>

#include <bctoolbox/crypto.h>
#include <bctoolbox/utils.hh>
#include <bctoolbox/vfs_encrypted.hh>
#include <belr/grammarbuilder.h>

#ifdef HAVE_SOCI
#include <soci/soci.h>
#endif // HAVE_SOCI

#include "liblinphone_tester.h"
#include "logger/logger.h"

#ifdef HAVE_LIME_X3DH
#include "bctoolbox/crypto.hh"
#include "lime/lime.hpp"
#endif // HAVE_LIME_X3DH

bool is_filepath_encrypted(const char *filepath) {
	bool ret = false;
	// if encryption openCallback is not set, file cannot be encrypted
	if (bctoolbox::VfsEncryption::openCallbackGet() == nullptr) {
		return false;
	}
	auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filepath, "r");
	if (fp != NULL) {
		ret = (bctbx_file_is_encrypted(fp) == TRUE);
		bctbx_file_close(fp);
	}
	return ret;
}

/* */
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void lime_delete_DRSessions(const char *limedb, const char *requestOption) {
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		if (requestOption != NULL) {
			sql << "DELETE FROM DR_sessions " << std::string(requestOption) << ";";
		} else {
			// Delete all sessions from the DR_sessions table
			sql << "DELETE FROM DR_sessions;";
		}
	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot delete DRSessions in database " << limedb << ". Error is " << e.what();
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
		lWarning() << "Cannot setback in time the lime users update ts on database " << limedb << ". Error is "
		           << e.what();
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
		lWarning() << "Cannot fetch the lime users update ts on database " << limedb << ". Error is " << e.what();
	}
#endif
	return ret;
}

char *lime_get_userIk(LinphoneCoreManager *mgr, char *gruu, uint8_t curveId) {
	char *ret = NULL;
#ifdef HAVE_SOCI
#ifdef HAVE_LIME_X3DH
	const char *limedb = mgr->lime_database_path;
	try {
		soci::session sql("sqlite3", limedb); // open the DB
		soci::blob ik_blob(sql);
		const std::string userGruu(gruu);
		sql << "SELECT Ik FROM Lime_LocalUsers WHERE UserId = :UserId AND curveId = :curveId LIMIT 1;",
		    soci::into(ik_blob), soci::use(userGruu), soci::use(curveId);
		if (sql.got_data()) { // Found it, it is stored in one buffer Public || Private
			std::array<unsigned char, BCTBX_EDDSA_448_PUBLIC_SIZE> ikRaw;
			const size_t public_key_size = ik_blob.get_len() / 2;
			ik_blob.read(0, (char *)(ikRaw.data()), public_key_size); // Read the public key
			std::vector<uint8_t> ik(ikRaw.cbegin(), ikRaw.cbegin() + public_key_size);
			std::string ikStr = bctoolbox::encodeBase64(ik);
			if (!ikStr.empty()) {
				ret = ms_strdup(ikStr.c_str());
			}
		}
	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot fetch the lime users to get the Identity Key value on database " << limedb
		           << ". Error is " << e.what();
	}
#endif // HAVE_LIME_X3DH
#endif
	return ret;
}

void delete_all_in_zrtp_table(const char *zrtpdb) {
#ifdef HAVE_SOCI
	try {
		soci::session sql("sqlite3", zrtpdb); // open the DB
		sql << "DELETE FROM zrtp;";           // Delete all in the zrtp table

	} catch (std::exception &e) { // swallow any error on DB
		lWarning() << "Cannot delete zrtp in database " << zrtpdb << ". Error is " << e.what();
	}
#endif
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

bool_t liblinphone_tester_is_executable_installed(const char *executable, const char *resource) {
	return bctoolbox::Utils::isExecutableInstalled(std::string(executable), std::string(resource));
}

void liblinphone_tester_add_grammar_loader_path(const char *path) {
	belr::GrammarLoader::get().addPath(std::string(path));
}

#ifdef HAVE_SOCI
void liblinphone_tester_add_soci_search_path(const char *path) {
	soci::dynamic_backends::search_paths().emplace_back(path);
}
#endif

#ifdef HAVE_LIME_X3DH
bool_t liblinphone_tester_is_lime_PQ_available(void) {
	return lime::lime_is_PQ_available() ? TRUE : FALSE;
}
#else
/* We should not need to define this function when LIME_X3DH is not built */
bool_t liblinphone_tester_is_lime_PQ_available(void) {
	return FALSE;
}
#endif
