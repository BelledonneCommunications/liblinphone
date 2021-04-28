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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LPCONFIG_H
#define LPCONFIG_H

#include "linphone/types.h"

/**
 * @addtogroup misc
 * @{
 */

/**
 * Safely downcast a belle_sip_object into #LinphoneConfig
 */
#define LINPHONE_CONFIG(obj) BELLE_SIP_CAST(obj, LinphoneConfig);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Instantiates a #LinphoneConfig object from a user config file.
 * The caller of this constructor owns a reference. linphone_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param filename the filename of the config file to read to fill the instantiated #LinphoneConfig @maybenil
 * @see linphone_config_new_with_factory()
 * @return a #LinphoneConfig object @maybenil
 */
LINPHONE_PUBLIC LinphoneConfig * linphone_config_new(const char *filename);

/**
 * Instantiates a #LinphoneConfig object from a user provided buffer.
 * The caller of this constructor owns a reference. linphone_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param buffer the buffer from which the #LinphoneConfig will be retrieved. We expect the buffer to be null-terminated. @notnil
 * @see linphone_config_new_with_factory()
 * @see linphone_config_new()
 * @return a #LinphoneConfig object @notnil
 */
LINPHONE_PUBLIC LinphoneConfig * linphone_config_new_from_buffer(const char *buffer);

/**
 * Instantiates a #LinphoneConfig object from a user config file and a factory config file.
 * The caller of this constructor owns a reference. linphone_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param config_filename the filename of the user config file to read to fill the instantiated #LinphoneConfig @maybenil
 * @param factory_config_filename the filename of the factory config file to read to fill the instantiated #LinphoneConfig @maybenil
 * @see linphone_config_new()
 * @return a #LinphoneConfig object @maybenil
 *
 * The user config file is read first to fill the #LinphoneConfig and then the factory config file is read.
 * Therefore the configuration parameters defined in the user config file will be overwritten by the parameters
 * defined in the factory config file.
 */
LINPHONE_PUBLIC LinphoneConfig * linphone_config_new_with_factory(const char *config_filename, const char *factory_config_filename);

/**
 * Instantiates a #LinphoneConfig object from a user config file name, group id and a factory config file.
 * The "group id" is the string that identify the "App group" capability of the iOS application.
 * App group gives access to a shared file system where all the configuration files for shared core are stored.
 * Both iOS application and iOS app extension that need shared core must activate the "App group" capability with the SAME
 * "group id" in the project settings.
 * The caller of this constructor owns a reference. linphone_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param app_group_id used to compute the path of the config file in the file system shared by the shared Cores @notnil
 * @param config_filename the filename of the user config file to read to fill the instantiated #LinphoneConfig @maybenil
 * @param factory_config_filename the filename of the factory config file to read to fill the instantiated #LinphoneConfig @maybenil
 * @see linphone_config_new()
 * @return a #LinphoneConfig object @maybenil
 *
 * The user config file is read first to fill the #LinphoneConfig and then the factory config file is read.
 * Therefore the configuration parameters defined in the user config file will be overwritten by the parameters
 * defined in the factory config file.
 */
LINPHONE_PUBLIC LinphoneConfig * linphone_config_new_for_shared_core(const char *app_group_id, const char* config_filename, const char *factory_config_filename);

/**
 * Reads a user config file and fill the #LinphoneConfig with the read config values.
 * @ingroup misc
 * @param config The #LinphoneConfig object to fill with the content of the file @notnil
 * @param filename The filename of the config file to read to fill the #LinphoneConfig @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_config_read_file(LinphoneConfig *config, const char *filename);

/**
 * Reads a xml config file and fill the #LinphoneConfig with the read config dynamic values.
 * @ingroup misc
 * @param config The #LinphoneConfig object to fill with the content of the file @notnil
 * @param filename The filename of the config file to read to fill the #LinphoneConfig @notnil
 */
LINPHONE_PUBLIC const char* linphone_config_load_from_xml_file(LinphoneConfig *config, const char *filename);

/**
 * Reads a xml config string and fill the #LinphoneConfig with the read config dynamic values.
 * @ingroup misc
 * @param config The #LinphoneConfig object to fill with the content of the file @notnil
 * @param buffer The string of the config file to fill the #LinphoneConfig @notnil
 * @return 0 in case of success
 */
LINPHONE_PUBLIC LinphoneStatus linphone_config_load_from_xml_string(LinphoneConfig *config, const char *buffer);

/**
 * Retrieves a configuration item as a string, given its section, key, and default value.
 *
 * The default value string is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_string The default value to return if not found. @maybenil
 * @return the found value or the default one if not found. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_config_get_string(const LinphoneConfig *config, const char *section, const char *key, const char *default_string);

/**
 * Retrieves a configuration item as a list of strings, given its section, key, and default value.
 * The default value is returned if the config item is not found.
 * @param config A #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_list The list to return when the key doesn't exist. \bctbx_list{const char *} @maybenil
 * @return A list of strings. \bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t * linphone_config_get_string_list(const LinphoneConfig *config, const char *section, const char *key, bctbx_list_t *default_list);

/**
 * Retrieves a configuration item as a range, given its section, key, and default min and max values.
 *
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param[out] min The min value found or default_min @notnil
 * @param[out] max The max value found or default_max @notnil
 * @param default_min the default min value to return if not found
 * @param default_max the default max value to return if not found.
 * @return TRUE if the value is successfully parsed as a range, FALSE otherwise.
 * If FALSE is returned, min and max are filled respectively with default_min and default_max values.
 */
LINPHONE_PUBLIC bool_t linphone_config_get_range(const LinphoneConfig *config, const char *section, const char *key, int *min, int *max, int default_min, int default_max);

/**
 * Retrieves a configuration item as an integer, given its section, key, and default value.
 *
 * The default integer value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found value or default_value if not found.
**/
LINPHONE_PUBLIC int linphone_config_get_int(const LinphoneConfig *config, const char *section, const char *key, int default_value);

/**
 * Retrieves a configuration item as a boolean, given its section, key, and default value.
 *
 * The default boolean value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found value or default_value if not found.
**/
LINPHONE_PUBLIC bool_t linphone_config_get_bool(const LinphoneConfig *config, const char *section, const char *key, bool_t default_value);

/**
 * Retrieves a configuration item as a 64 bit integer, given its section, key, and default value.
 *
 * The default integer value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found value or default_value if not found.
**/
LINPHONE_PUBLIC int64_t linphone_config_get_int64(const LinphoneConfig *config, const char *section, const char *key, int64_t default_value);

/**
 * Retrieves a configuration item as a float, given its section, key, and default value.
 *
 * The default float value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found value or default_value if not found.
**/
LINPHONE_PUBLIC float linphone_config_get_float(const LinphoneConfig *config, const char *section, const char *key, float default_value);

/**
 * Sets a string config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value The value to set @maybenil
**/
LINPHONE_PUBLIC void linphone_config_set_string(LinphoneConfig *config,const char *section, const char *key, const char *value);

/**
 * Sets a string list config item
 * @param config A #LinphoneConfig object @notnil
 * @param section The name of the section to put the configuration item into @notnil
 * @param key The name of the configuration item to set @notnil
 * @param value The value to set. \bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC void linphone_config_set_string_list(LinphoneConfig *config, const char *section, const char *key, const bctbx_list_t *value);

/**
 * Sets a range config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param min_value the min value to set
 * @param max_value the max value to set
 */
LINPHONE_PUBLIC void linphone_config_set_range(LinphoneConfig *config, const char *section, const char *key, int min_value, int max_value);

/**
 * Sets an integer config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value the value to set
**/
LINPHONE_PUBLIC void linphone_config_set_int(LinphoneConfig *config,const char *section, const char *key, int value);

/**
 * Sets a boolean config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value the value to set
**/
LINPHONE_PUBLIC void linphone_config_set_bool(LinphoneConfig *config,const char *section, const char *key, bool_t value);

/**
 * Sets an integer config item, but store it as hexadecimal
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value the value to set
**/
LINPHONE_PUBLIC void linphone_config_set_int_hex(LinphoneConfig *config,const char *section, const char *key, int value);

/**
 * Sets a 64 bits integer config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value the value to set
**/
LINPHONE_PUBLIC void linphone_config_set_int64(LinphoneConfig *config,const char *section, const char *key, int64_t value);

/**
 * Sets a float config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve a configuration item @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param value the value to set
**/
LINPHONE_PUBLIC void linphone_config_set_float(LinphoneConfig *config,const char *section, const char *key, float value);

/**
 * Writes the config file to disk.
 * @param config The #LinphoneConfig object @notnil
 * @return 0 if successful, -1 otherwise
**/
LINPHONE_PUBLIC LinphoneStatus linphone_config_sync(LinphoneConfig *config);

/**
 * Reload the config from the file.
 * @param config The #LinphoneConfig object @notnil
**/
LINPHONE_PUBLIC void linphone_config_reload(LinphoneConfig *config);

/**
 * Returns if a given section is present in the configuration.
 * @param config The #LinphoneConfig object @notnil
 * @param section the section to check if exists @notnil
 * @return 1 if it exists, 0 otherwise 
**/
LINPHONE_PUBLIC int linphone_config_has_section(const LinphoneConfig *config, const char *section);

/**
 * Removes every pair of key,value in a section and remove the section.
 * @param config The #LinphoneConfig object @notnil
 * @param section the section to clean @notnil
**/
LINPHONE_PUBLIC void linphone_config_clean_section(LinphoneConfig *config, const char *section);

/**
 * Returns if a given section  with a given key is present in the configuration.
 * @param config The #LinphoneConfig object @notnil
 * @param section to check if the given entry exists @notnil
 * @param key to check if it exists @notnil
 * @return 1 if it exists, 0 otherwise
 **/
LINPHONE_PUBLIC int linphone_config_has_entry(const LinphoneConfig *config, const char *section, const char *key);

/**
 * Removes entries for key,value in a section.
 * @param config The #LinphoneConfig object @notnil
 * @param section the section for which to clean the key entry @notnil
 * @param key the key to clean @notnil
 **/
LINPHONE_PUBLIC void linphone_config_clean_entry(LinphoneConfig *config, const char *section, const char *key);

/**
 * Returns the list of sections' names in the LinphoneConfig.
 * @param config The #LinphoneConfig object @notnil
 * @return A list of strings. \bctbx_list{char *} @maybenil
**/
LINPHONE_PUBLIC const bctbx_list_t * linphone_config_get_sections_names_list(LinphoneConfig *config);

/**
 * Returns the list of keys' names for a section in the LinphoneConfig.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section name @notnil
 * @return A list of strings. \bctbx_list{char *} @maybenil
**/
LINPHONE_PUBLIC const bctbx_list_t * linphone_config_get_keys_names_list(LinphoneConfig *config, const char *section );

/**
 * @brief Call a function for each section present in the configuration.
 * @donotwrap
**/
void linphone_config_for_each_section(const LinphoneConfig *config, void (*callback)(const char *section, void *ctx), void *ctx);

/**
 * @brief Call a function for each entry present in a section configuration.
 * @donotwrap
**/
void linphone_config_for_each_entry(const LinphoneConfig *config, const char *section, void (*callback)(const char *entry, void *ctx), void *ctx);

/*tells whether uncommited (with linphone_config_sync()) modifications exist*/
bool_t linphone_config_needs_commit(const LinphoneConfig *config);

LINPHONE_PUBLIC void linphone_config_destroy(LinphoneConfig *cfg);

/**
 * Retrieves a default configuration item as an integer, given its section, key, and default value.
 * The default integer value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the default value @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found default value or default_value if not found.
**/
LINPHONE_PUBLIC int linphone_config_get_default_int(const LinphoneConfig *config, const char *section, const char *key, int default_value);

/**
 * Retrieves a default configuration item as a 64 bit integer, given its section, key, and default value.
 * The default integer value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the default value @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found default value or default_value if not found.
**/
LINPHONE_PUBLIC int64_t linphone_config_get_default_int64(const LinphoneConfig *config, const char *section, const char *key, int64_t default_value);

/**
 * Retrieves a default configuration item as a float, given its section, key, and default value.
 * The default float value is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the default value @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found default value or default_value if not found.
**/
LINPHONE_PUBLIC float linphone_config_get_default_float(const LinphoneConfig *config, const char *section, const char *key, float default_value);

/**
 * Retrieves a default configuration item as a string, given its section, key, and default value.
 * The default value string is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the default value @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found
 * @return the found default value or default_value if not found.
**/
LINPHONE_PUBLIC const char* linphone_config_get_default_string(const LinphoneConfig *config, const char *section, const char *key, const char *default_value);

/**
 * Retrieves a section parameter item as a string, given its section and key.
 * The default value string is returned if the config item isn't found.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the default value @notnil
 * @param key The name of the configuration item to retrieve @notnil
 * @param default_value The default value to return if not found. @maybenil
 * @return the found default value or default_value if not found. @maybenil
**/
LINPHONE_PUBLIC const char* linphone_config_get_section_param_string(const LinphoneConfig *config, const char *section, const char *key, const char *default_value);

/**
 * increment reference count
 * @param config The #LinphoneConfig object @notnil
 * @return the same #LinphoneConfig object @notnil
**/
LINPHONE_PUBLIC LinphoneConfig *linphone_config_ref(LinphoneConfig *config);

/**
 * Decrement reference count, which will eventually free the object.
 * @param config The #LinphoneConfig object @notnil
**/
LINPHONE_PUBLIC void linphone_config_unref(LinphoneConfig *config);

/**
 * Write a string in a file placed relatively with the Linphone configuration file.
 * @param config #LinphoneConfig instance used as a reference @notnil
 * @param filename Name of the file where to write data. The name is relative to the place of the config file @notnil
 * @param data String to write @notnil
 */
LINPHONE_PUBLIC void linphone_config_write_relative_file(const LinphoneConfig *config, const char *filename, const char *data);

/**
 * Read a string from a file placed beside the Linphone configuration file
 * @param config #LinphoneConfig instance used as a reference
 * @param filename Name of the file where data will be read from. The name is relative to the place of the config file
 * @param data Buffer where read string will be stored
 * @param max_length Length of the buffer
 * @return 0 on success, -1 on failure
 * @donotwrap
 */
LINPHONE_PUBLIC LinphoneStatus linphone_config_read_relative_file(const LinphoneConfig *config, const char *filename, char *data, size_t max_length);

/**
 * Check if given file name exists relatively to the current location
 * @param config The #LinphoneConfig object @notnil
 * @param filename The file name to check if exists @notnil
 * @return TRUE if file exists relative to the to the current location
**/
LINPHONE_PUBLIC bool_t linphone_config_relative_file_exists(const LinphoneConfig *config, const char *filename);

/**
 * Dumps the #LinphoneConfig as XML into a buffer
 * @param config The #LinphoneConfig object @notnil
 * @return The buffer that contains the XML dump @notnil
**/
LINPHONE_PUBLIC char* linphone_config_dump_as_xml(const LinphoneConfig *config);

/**
 * Dumps the #LinphoneConfig as INI into a buffer
 * @param config The #LinphoneConfig object @notnil
 * @return The buffer that contains the config dump @notnil
**/
LINPHONE_PUBLIC char* linphone_config_dump(const LinphoneConfig *config);

/**
 * Retrieves the overwrite flag for a config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the overwrite flag @notnil
 * @param key The name of the configuration item to retrieve the overwrite flag from. @notnil
 * @return TRUE if overwrite flag is set, FALSE otherwise.
**/
LINPHONE_PUBLIC bool_t linphone_config_get_overwrite_flag_for_entry(const LinphoneConfig *config, const char *section, const char *key);

/**
 * Sets the overwrite flag for a config item (used when dumping config as xml)
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to set the overwrite flag @notnil
 * @param key The name of the configuration item to set the overwrite flag from @notnil
 * @param value The overwrite flag value to set
**/
LINPHONE_PUBLIC void linphone_config_set_overwrite_flag_for_entry(LinphoneConfig *config, const char *section, const char *key, bool_t value);

/**
 * Retrieves the overwrite flag for a config section
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the overwrite flag @notnil
 * @return TRUE if overwrite flag is set, FALSE otherwise.
**/
LINPHONE_PUBLIC bool_t linphone_config_get_overwrite_flag_for_section(const LinphoneConfig *config, const char *section);

/**
 * Sets the overwrite flag for a config section (used when dumping config as xml)
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to set the overwrite flag @notnil
 * @param value The overwrite flag value to set
**/
LINPHONE_PUBLIC void linphone_config_set_overwrite_flag_for_section(LinphoneConfig *config, const char *section, bool_t value);

/**
 * Retrieves the skip flag for a config item
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the skip flag @notnil
 * @param key The name of the configuration item to retrieve the skip flag from
 * @return TRUE if skip flag is set, FALSE otherwise.
**/
LINPHONE_PUBLIC bool_t linphone_config_get_skip_flag_for_entry(const LinphoneConfig *config, const char *section, const char *key);

/**
 * Sets the skip flag for a config item (used when dumping config as xml)
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to set the skip flag @notnil
 * @param key The name of the configuration item to set the skip flag from @notnil
 * @param value The skip flag value to set
**/
LINPHONE_PUBLIC void linphone_config_set_skip_flag_for_entry(LinphoneConfig *config, const char *section, const char *key, bool_t value);

/**
 * Retrieves the skip flag for a config section
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to retrieve the skip flag @notnil
 * @return TRUE if skip flag is set, FALSE otherwise.
**/
LINPHONE_PUBLIC bool_t linphone_config_get_skip_flag_for_section(const LinphoneConfig *config, const char *section);

/**
 * Sets the skip flag for a config section (used when dumping config as xml)
 * @param config The #LinphoneConfig object @notnil
 * @param section The section from which to set the skip flag @notnil
 * @param value The skip flag value to set
**/
LINPHONE_PUBLIC void linphone_config_set_skip_flag_for_section(LinphoneConfig *config, const char *section, bool_t value);

/**
 * Converts a config section into a dictionary.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section to dump as dictionary @notnil
 * @return a dictionary with all the keys from a section, or NULL if the section doesn't exist. @maybenil
 * @donotwrap LinphoneDictionary can't be wrapped
 */
LINPHONE_PUBLIC LinphoneDictionary * linphone_config_section_to_dict(const LinphoneConfig* config, const char* section);

/**
 * Loads a dictionary into a section of the #LinphoneConfig. If the section doesn't exist it is created.
 * Overwrites existing keys, creates non-existing keys.
 * @param config The #LinphoneConfig object @notnil
 * @param section The section to dump as dictionary @notnil
 * @param dict the dictionnary to load into the section @notnil
 * @donotwrap LinphoneDictionary can't be wrapped
 */
LINPHONE_PUBLIC void linphone_config_load_dict_to_section(LinphoneConfig* config, const char* section, const LinphoneDictionary* dict);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Returns the list of sections' names in the LinphoneConfig.
 * @param config The #LinphoneConfig object
 * @return a null terminated static array of strings
 * @deprecated 12/10/2017 use linphone_config_get_sections_names_list instead
 * @donotwrap
**/
LINPHONE_PUBLIC const char** linphone_config_get_sections_names(LinphoneConfig *config);

#ifdef __cplusplus
}
#endif

// 08/07/2020 Define old function names for backward compatibility
#define lp_config_section_to_dict linphone_config_section_to_dict
#define lp_config_load_dict_to_section linphone_config_load_dict_to_section

/**
 * @}
 */

#endif
