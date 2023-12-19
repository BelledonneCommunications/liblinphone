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

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Instantiates a new dictionary with values from source.
 * @param src The #LinphoneDictionary object to be cloned. @notnil
 * @return The newly created #LinphoneDictionary object. @notnil
 */
LINPHONE_PUBLIC LinphoneDictionary * linphone_dictionary_clone(const LinphoneDictionary* src);

/**
 * Take a reference on a #LinphoneDictionary.
 * @param dict The #LinphoneDictionary object. @notnil
 * @return the same #LinphoneDictionary object.
 */
LINPHONE_PUBLIC LinphoneDictionary * linphone_dictionary_ref(LinphoneDictionary* dict);

/**
 * Release a #LinphoneDictionary.
 * @param dict The #LinphoneDictionary object. @notnil
 */
LINPHONE_PUBLIC void linphone_dictionary_unref(LinphoneDictionary* dict);

/**
 * Sets a float value to a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @param value The int value.
**/
LINPHONE_PUBLIC void linphone_dictionary_set_float(LinphoneDictionary* dict, const char* key, float value);

/**
 * Gets the float value of a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return The username. @maybenil
 */
LINPHONE_PUBLIC float linphone_dictionary_get_float(LinphoneDictionary* dict, const char* key);

/**
 * Sets a int value to a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @param value The int value.
**/
LINPHONE_PUBLIC void linphone_dictionary_set_int(LinphoneDictionary* dict, const char* key, int value);

/**
 * Gets the int value of a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return The username. @maybenil
 */
LINPHONE_PUBLIC int linphone_dictionary_get_int(LinphoneDictionary* dict, const char* key);

/**
 * Sets a char* value to a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @param value The char* value.
**/
LINPHONE_PUBLIC void linphone_dictionary_set_string(LinphoneDictionary* dict, const char* key, const char* value);

/**
 * Gets the char* value of a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return The username. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_dictionary_get_string(LinphoneDictionary* dict, const char* key);

/**
 * Sets a int64 value to a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @param value The int64 value.
**/
LINPHONE_PUBLIC void linphone_dictionary_set_int64(LinphoneDictionary* dict, const char* key, int64_t value);

/**
 * Gets the int64 value of a key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return The username. @maybenil
 */
LINPHONE_PUBLIC int64_t linphone_dictionary_get_int64(LinphoneDictionary* dict, const char* key);

/**
 * Removes the pair of the key.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return 
*/
LINPHONE_PUBLIC LinphoneStatus linphone_dictionary_remove(LinphoneDictionary* dict, const char* key);

/**
 * Clears the dictionary.
 * @param dict The #LinphoneDictionary object. @notnil
*/
LINPHONE_PUBLIC void linphone_dictionary_clear(LinphoneDictionary* dict);

/**
 * Search if the key is present in the dictionary.
 * @param dict The #LinphoneDictionary object. @notnil
 * @param key The key. @maybenil
 * @return
*/
LINPHONE_PUBLIC LinphoneStatus linphone_dictionary_has_key(const LinphoneDictionary* dict, const char* key);

#ifdef __cplusplus
}
#endif