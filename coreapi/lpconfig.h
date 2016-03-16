/***************************************************************************
 *            lpconfig.h
 *
 *  Thu Mar 10 15:02:49 2005
 *  Copyright  2005  Simon Morlat
 *  Email simon.morlat@linphone.org
 ****************************************************************************/

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
#include <mediastreamer2/mscommon.h>
#include <ortp/port.h>

#ifndef LINPHONE_PUBLIC
	#define LINPHONE_PUBLIC MS2_PUBLIC
#endif

/**
 * The LpConfig object is used to manipulate a configuration file.
 *
 * @ingroup misc
 * The format of the configuration file is a .ini like format:
 * - sections are defined in []
 * - each section contains a sequence of key=value pairs.
 *
 * Example:
 * @code
 * [sound]
 * echocanceler=1
 * playback_dev=ALSA: Default device
 *
 * [video]
 * enabled=1
 * @endcode
**/
typedef struct _LpConfig LpConfig;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Instantiates a LpConfig object from a user config file.
 * The caller of this constructor owns a reference. lp_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param filename the filename of the config file to read to fill the instantiated LpConfig
 * @see lp_config_new_with_factory
 */
LINPHONE_PUBLIC LpConfig * lp_config_new(const char *filename);

/**
 * Instantiates a LpConfig object from a user provided buffer.
 * The caller of this constructor owns a reference. lp_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param buffer the buffer from which the lpconfig will be retrieved. We expect the buffer to be null-terminated.
 * @see lp_config_new_with_factory
 * @see lp_config_new
 */
LINPHONE_PUBLIC LpConfig * lp_config_new_from_buffer(const char *buffer);

/**
 * Instantiates a LpConfig object from a user config file and a factory config file.
 * The caller of this constructor owns a reference. lp_config_unref() must be called when this object is no longer needed.
 * @ingroup misc
 * @param config_filename the filename of the user config file to read to fill the instantiated LpConfig
 * @param factory_config_filename the filename of the factory config file to read to fill the instantiated LpConfig
 * @see lp_config_new
 *
 * The user config file is read first to fill the LpConfig and then the factory config file is read.
 * Therefore the configuration parameters defined in the user config file will be overwritten by the parameters
 * defined in the factory config file.
 */
LINPHONE_PUBLIC LpConfig * lp_config_new_with_factory(const char *config_filename, const char *factory_config_filename);

/**
 * Reads a user config file and fill the LpConfig with the read config values.
 * @ingroup misc
 * @param lpconfig The LpConfig object to fill with the content of the file
 * @param filename The filename of the config file to read to fill the LpConfig
 */
LINPHONE_PUBLIC int lp_config_read_file(LpConfig *lpconfig, const char *filename);

/**
 * Retrieves a configuration item as a string, given its section, key, and default value.
 *
 * @ingroup misc
 * The default value string is returned if the config item isn't found.
**/
LINPHONE_PUBLIC const char *lp_config_get_string(const LpConfig *lpconfig, const char *section, const char *key, const char *default_string);

/**
 * Retrieves a configuration item as a range, given its section, key, and default min and max values.
 *
 * @ingroup misc
 * @return TRUE if the value is successfully parsed as a range, FALSE otherwise.
 * If FALSE is returned, min and max are filled respectively with default_min and default_max values.
 */
LINPHONE_PUBLIC bool_t lp_config_get_range(const LpConfig *lpconfig, const char *section, const char *key, int *min, int *max, int default_min, int default_max);

/**
 * Retrieves a configuration item as an integer, given its section, key, and default value.
 *
 * @ingroup misc
 * The default integer value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC int lp_config_get_int(const LpConfig *lpconfig,const char *section, const char *key, int default_value);

/**
 * Retrieves a configuration item as a 64 bit integer, given its section, key, and default value.
 *
 * @ingroup misc
 * The default integer value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC int64_t lp_config_get_int64(const LpConfig *lpconfig,const char *section, const char *key, int64_t default_value);

/**
 * Retrieves a configuration item as a float, given its section, key, and default value.
 *
 * @ingroup misc
 * The default float value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC float lp_config_get_float(const LpConfig *lpconfig,const char *section, const char *key, float default_value);

/**
 * Sets a string config item
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_string(LpConfig *lpconfig,const char *section, const char *key, const char *value);

/**
 * Sets a range config item
 *
 * @ingroup misc
 */
LINPHONE_PUBLIC void lp_config_set_range(LpConfig *lpconfig, const char *section, const char *key, int min_value, int max_value);

/**
 * Sets an integer config item
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_int(LpConfig *lpconfig,const char *section, const char *key, int value);

/**
 * Sets an integer config item, but store it as hexadecimal
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_int_hex(LpConfig *lpconfig,const char *section, const char *key, int value);

/**
 * Sets a 64 bits integer config item
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_int64(LpConfig *lpconfig,const char *section, const char *key, int64_t value);

/**
 * Sets a float config item
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_float(LpConfig *lpconfig,const char *section, const char *key, float value);

/**
 * Writes the config file to disk.
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC int lp_config_sync(LpConfig *lpconfig);

/**
 * Returns 1 if a given section is present in the configuration.
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC int lp_config_has_section(const LpConfig *lpconfig, const char *section);

/**
 * Removes every pair of key,value in a section and remove the section.
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_clean_section(LpConfig *lpconfig, const char *section);

/**
 * Returns the list of sections' names in the LpConfig.
 * @param[in] lpconfig The LpConfig object
 * @return a null terminated static array of strings
**/
LINPHONE_PUBLIC const char** lp_config_get_sections_names(LpConfig *lpconfig);

/**
 * Call a function for each section present in the configuration.
 *
 * @ingroup misc
**/
void lp_config_for_each_section(const LpConfig *lpconfig, void (*callback)(const char *section, void *ctx), void *ctx);

/**
 * Call a function for each entry present in a section configuration.
 *
 * @ingroup misc
**/
void lp_config_for_each_entry(const LpConfig *lpconfig, const char *section, void (*callback)(const char *entry, void *ctx), void *ctx);

/*tells whether uncommited (with lp_config_sync()) modifications exist*/
int lp_config_needs_commit(const LpConfig *lpconfig);

LINPHONE_PUBLIC void lp_config_destroy(LpConfig *cfg);

/**
 * Retrieves a default configuration item as an integer, given its section, key, and default value.
 *
 * @ingroup misc
 * The default integer value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC int lp_config_get_default_int(const LpConfig *lpconfig, const char *section, const char *key, int default_value);

/**
 * Retrieves a default configuration item as a 64 bit integer, given its section, key, and default value.
 *
 * @ingroup misc
 * The default integer value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC int64_t lp_config_get_default_int64(const LpConfig *lpconfig, const char *section, const char *key, int64_t default_value);

/**
 * Retrieves a default configuration item as a float, given its section, key, and default value.
 *
 * @ingroup misc
 * The default float value is returned if the config item isn't found.
**/
LINPHONE_PUBLIC float lp_config_get_default_float(const LpConfig *lpconfig, const char *section, const char *key, float default_value);

/**
 * Retrieves a default configuration item as a string, given its section, key, and default value.
 *
 * @ingroup misc
 * The default value string is returned if the config item isn't found.
**/
LINPHONE_PUBLIC const char* lp_config_get_default_string(const LpConfig *lpconfig, const char *section, const char *key, const char *default_value);

/**
 * Retrieves a section parameter item as a string, given its section and key.
 *
 * @ingroup misc
 * The default value string is returned if the config item isn't found.
**/
LINPHONE_PUBLIC const char* lp_config_get_section_param_string(const LpConfig *lpconfig, const char *section, const char *key, const char *default_value);


/**
 * increment reference count
 * @ingroup misc
**/
LINPHONE_PUBLIC LpConfig *lp_config_ref(LpConfig *lpconfig);

/**
 * Decrement reference count, which will eventually free the object.
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_unref(LpConfig *lpconfig);

/**
 * @brief Write a string in a file placed relatively with the Linphone configuration file.
 * @param lpconfig LpConfig instance used as a reference
 * @param filename Name of the file where to write data. The name is relative to the place of the config file
 * @param data String to write
 */
LINPHONE_PUBLIC void lp_config_write_relative_file(const LpConfig *lpconfig, const char *filename, const char *data);

/**
 * @brief Read a string from a file placed beside the Linphone configuration file
 * @param lpconfig LpConfig instance used as a reference
 * @param filename Name of the file where data will be read from. The name is relative to the place of the config file
 * @param data Buffer where read string will be stored
 * @param max_length Length of the buffer
 * @return 0 on success, -1 on failure
 */
LINPHONE_PUBLIC int lp_config_read_relative_file(const LpConfig *lpconfig, const char *filename, char *data, size_t max_length);

/**
 * @return TRUE if file exists relative to the to the current location
**/
LINPHONE_PUBLIC bool_t lp_config_relative_file_exists(const LpConfig *lpconfig, const char *filename);

/**
 * Dumps the LpConfig as XML into a buffer
 * @param[in] lpconfig The LpConfig object
 * @return The buffer that contains the XML dump
 * 
 * @ingroup misc
**/
LINPHONE_PUBLIC char* lp_config_dump_as_xml(const LpConfig *lpconfig);

/**
 * Retrieves the overwrite flag for a config item
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC bool_t lp_config_get_overwrite_flag_for_entry(const LpConfig *lpconfig, const char *section, const char *key);

/**
 * Sets the overwrite flag for a config item (used when dumping config as xml)
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_overwrite_flag_for_entry(LpConfig *lpconfig, const char *section, const char *key, bool_t value);

/**
 * Retrieves the overwrite flag for a config section
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC bool_t lp_config_get_overwrite_flag_for_section(const LpConfig *lpconfig, const char *section);

/**
 * Sets the overwrite flag for a config section (used when dumping config as xml)
 *
 * @ingroup misc
**/
LINPHONE_PUBLIC void lp_config_set_overwrite_flag_for_section(LpConfig *lpconfig, const char *section, bool_t value);

#ifdef __cplusplus
}
#endif

#endif
