/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "linphone/api/c-dictionary.h"
#include "c-wrapper/c-wrapper.h"
#include "dictionary/dictionary.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneDictionary *linphone_dictionary_clone(const LinphoneDictionary *src) {
	if (src) {
		LinphoneDictionary *dict = Dictionary::toCpp(src)->clone()->toC();
		return dict;
	}
	return nullptr;
}

LinphoneDictionary *linphone_dictionary_ref(LinphoneDictionary *dict) {
	Dictionary::toCpp(dict)->ref();
	return dict;
}

void linphone_dictionary_unref(LinphoneDictionary *dict) {
	Dictionary::toCpp(dict)->unref();
}

void linphone_dictionary_set_float(LinphoneDictionary *dict, const char *key, float value) {
	Dictionary::toCpp(dict)->setProperty(L_C_TO_STRING(key), value);
}

float linphone_dictionary_get_float(const LinphoneDictionary *dict, const char *key) {
	return Dictionary::toCpp(dict)->getFloat(L_C_TO_STRING(key));
}

void linphone_dictionary_set_int(LinphoneDictionary *dict, const char *key, int value) {
	Dictionary::toCpp(dict)->setProperty(L_C_TO_STRING(key), value);
}

int linphone_dictionary_get_int(const LinphoneDictionary *dict, const char *key) {
	return Dictionary::toCpp(dict)->getInt(L_C_TO_STRING(key));
}

void linphone_dictionary_set_string(LinphoneDictionary *dict, const char *key, const char *value) {
	Dictionary::toCpp(dict)->setProperty(L_C_TO_STRING(key), value);
}

const char *linphone_dictionary_get_string(const LinphoneDictionary *dict, const char *key) {
	return Dictionary::toCpp(dict)->getString(L_C_TO_STRING(key)).c_str();
}

void linphone_dictionary_set_int64(LinphoneDictionary *dict, const char *key, int64_t value) {
	Dictionary::toCpp(dict)->setProperty(L_C_TO_STRING(key), static_cast<long long>(value));
}

int64_t linphone_dictionary_get_int64(const LinphoneDictionary *dict, const char *key) {
	return Dictionary::toCpp(dict)->getLongLong(L_C_TO_STRING(key));
}

void linphone_dictionary_set_buffer(LinphoneDictionary *dict, const char *key, LinphoneBuffer *value) {
	Dictionary::toCpp(dict)->setProperty(L_C_TO_STRING(key), Buffer::toCpp(value)->getSharedFromThis());
}

LinphoneBuffer *linphone_dictionary_get_buffer(const LinphoneDictionary *dict, const char *key) {
	auto buffer = Dictionary::toCpp(dict)->getBuffer(L_C_TO_STRING(key));
	if (buffer) return buffer->toC();
	return nullptr;
}

LinphoneStatus linphone_dictionary_remove(LinphoneDictionary *dict, const char *key) {
	return Dictionary::toCpp(dict)->remove(L_C_TO_STRING(key));
}

void linphone_dictionary_clear(LinphoneDictionary *dict) {
	Dictionary::toCpp(dict)->clear();
}

LinphoneStatus linphone_dictionary_has_key(const LinphoneDictionary *dict, const char *key) {
	if (Dictionary::toCpp(dict)->hasKey(L_C_TO_STRING(key))) {
		return 1;
	}
	return 0;
}

bctbx_list_t *linphone_dictionary_get_keys(const LinphoneDictionary *dict) {
	std::list<std::string> keys = Dictionary::toCpp(dict)->getKeys();
	bctbx_list_t *result = nullptr;
	for (auto i = keys.begin(); i != keys.end(); ++i) {
		result = bctbx_list_append(result, ms_strdup(i->c_str()));
	}
	return result;
}