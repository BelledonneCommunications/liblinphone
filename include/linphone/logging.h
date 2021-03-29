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

#ifndef _LINPHONE_LOG_H_
#define _LINPHONE_LOG_H_

#include "types.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup logging
 * @{
 */

/**
 * @brief Singleton class giving access to logging features.
 * 
 * It supports custom domain, writing into a file as well as several verbosity levels.
 * The #LinphoneLoggingServiceCbs listener allows you to be notified each time a log is printed.
 * 
 * As the #LinphoneLoggingService is a singleton, use linphone_logging_service_get() to get it.
 */
typedef struct _LinphoneLoggingService LinphoneLoggingService;

/**
 * @brief Listener for #LinphoneLoggingService.
 */
typedef struct _LinphoneLoggingServiceCbs LinphoneLoggingServiceCbs;

/**
 * @brief Verbosity levels of log messages.
 */
typedef enum _LinphoneLogLevel {
	LinphoneLogLevelDebug   = 1<<0, /**< @brief Level for debug messages. */
	LinphoneLogLevelTrace   = 1<<1, /**< @brief Level for traces. */
	LinphoneLogLevelMessage = 1<<2, /**< @brief Level for information messages. */
	LinphoneLogLevelWarning = 1<<3, /**< @brief Level for warning messages. */
	LinphoneLogLevelError   = 1<<4, /**< @brief Level for error messages. */
	LinphoneLogLevelFatal   = 1<<5  /**< @brief Level for fatal error messages. */
} LinphoneLogLevel;

/**
 * @brief Type of callbacks called each time liblinphone write a log message.
 * 
 * @param log_service A pointer on the logging service singleton. @notnil
 * @param domain A string describing which sub-library of liblinphone the message is coming from. @notnil
 * @param level Verbosity #LinphoneLogLevel of the message.
 * @param message Content of the message. @notnil
 */
typedef void (*LinphoneLoggingServiceCbsLogMessageWrittenCb)(LinphoneLoggingService *log_service, const char *domain, LinphoneLogLevel level, const char *message);

/**
 * @brief Gets the singleton logging service object.
 * 
 * The singleton is automatically instantiated if it hasn't
 * been done yet.
 * 
 * @return A pointer on the #LinphoneLoggingService singleton. @notnil
 */
LINPHONE_PUBLIC LinphoneLoggingService *linphone_logging_service_get(void);

/**
 * @brief Increases the reference counter.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @return the same #LinphoneLoggingService object @notnil
 */
LINPHONE_PUBLIC LinphoneLoggingService *linphone_logging_service_ref(LinphoneLoggingService *log_service);

/**
 * @brief Decreases the reference counter and destroy the object if the counter reaches 0.
 * @param log_service the #LinphoneLoggingService object @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_unref(LinphoneLoggingService *log_service);

/**
 * Adds a callback object to the list of listeners
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param cbs the #LinphoneLoggingServiceCbs to add @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_add_callbacks(LinphoneLoggingService *log_service, LinphoneLoggingServiceCbs *cbs);

/**
 * Removes a callback object from the list of listeners
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param cbs the #LinphoneLoggingServiceCbs to remove @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_remove_callbacks(LinphoneLoggingService *log_service, LinphoneLoggingServiceCbs *cbs);

/**
 * Returns the current callbacks being called while iterating on callbacks
 * @param log_service the #LinphoneLoggingService object @notnil
 * @return A pointer to the current #LinphoneLoggingServiceCbs object @maybenil
 */
LINPHONE_PUBLIC LinphoneLoggingServiceCbs *linphone_logging_service_get_current_callbacks(const LinphoneLoggingService *log_service);

/**
 * @brief Set the verbosity of the log.
 * 
 * For instance, a level of #LinphoneLogLevelMessage will let pass fatal, error, warning and message-typed messages
 * whereas trace and debug messages will be dumped out.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param level the #LinphoneLogLevel to set
 */
LINPHONE_PUBLIC void linphone_logging_service_set_log_level(LinphoneLoggingService *log_service, LinphoneLogLevel level);

/**
 * @brief Sets the types of messages that will be authorized to be written in the log.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param mask Example: #LinphoneLogLevelMessage|#LinphoneLogLevelError will ONLY let pass message-typed and error messages.
 * @note Calling that function reset the log level that has been specified by #linphone_logging_service_set_log_level().
 */
LINPHONE_PUBLIC void linphone_logging_service_set_log_level_mask(LinphoneLoggingService *log_service, unsigned int mask);

/**
 * @brief Gets the log level mask.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @return the log level mask
 */
LINPHONE_PUBLIC unsigned int linphone_logging_service_get_log_level_mask(const LinphoneLoggingService *log_service);

/**
 * @brief Enables logging in a file.
 * 
 * That function enables an internal log handler that writes log messages in
 * log-rotated files.
 * 
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param dir Directory where to create the distinct parts of the log. @notnil
 * @param filename Name of the log file. @notnil
 * @param max_size The maximal size of each part of the log. The log rotating is triggered
 * each time the currently opened log part reach that limit.
 */
LINPHONE_PUBLIC void linphone_logging_service_set_log_file(const LinphoneLoggingService *log_service, const char *dir, const char *filename, size_t max_size);

/**
 * @brief Set the domain where application logs are written (for example with #linphone_logging_service_message()).
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param domain The domain. @maybenil
 * @note The domain is mandatory to write logs. This needs to be set before setting the log level.
 */
LINPHONE_PUBLIC void linphone_logging_service_set_domain(LinphoneLoggingService *log_service, const char *domain);

/**
 * @brief Get the domain where application logs are written (for example with #linphone_logging_service_message()).
 * @param log_service the #LinphoneLoggingService object @maybenil
 * @return The domain where application logs are written. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_logging_service_get_domain(LinphoneLoggingService *log_service);

/**
 * @brief Write a LinphoneLogLevelDebug message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_debug(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Write a LinphoneLogLevelTrace message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_trace(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Write a LinphoneLogLevelMessage message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_message(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Write a LinphoneLogLevelWarning message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_warning(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Write a LinphoneLogLevelError message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_error(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Write a LinphoneLogLevelFatal message to the logs.
 * @param log_service the #LinphoneLoggingService object @notnil
 * @param message The log message. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_fatal(LinphoneLoggingService *log_service, const char *message);

/**
 * @brief Increases the reference counter.
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 * @return the same #LinphoneLoggingServiceCbs object @notnil
 */
LINPHONE_PUBLIC LinphoneLoggingServiceCbs *linphone_logging_service_cbs_ref(LinphoneLoggingServiceCbs *cbs);

/**
 * @brief Decreases the reference counter.
 * 
 * The object is automatically destroyed once the counter reach 0.
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 */
LINPHONE_PUBLIC void linphone_logging_service_cbs_unref(LinphoneLoggingServiceCbs *cbs);

/**
 * @brief Sets the callback to call each time liblinphone writes a log message.
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 * @param cb the #LinphoneLoggingServiceCbsLogMessageWrittenCb to set
 */
LINPHONE_PUBLIC void linphone_logging_service_cbs_set_log_message_written(LinphoneLoggingServiceCbs *cbs, LinphoneLoggingServiceCbsLogMessageWrittenCb cb);

/**
 * @brief Gets the value of the message event callback.
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 * @return the current #LinphoneLoggingServiceCbsLogMessageWrittenCb
 */
LINPHONE_PUBLIC LinphoneLoggingServiceCbsLogMessageWrittenCb linphone_logging_service_cbs_get_log_message_written(const LinphoneLoggingServiceCbs *cbs);

/**
 * @brief Pass a pointer on a custom object.
 * 
 * That pointer can be get back by callbacks by using #linphone_logging_service_get_cbs() and #linphone_logging_service_cbs_get_user_data().
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 * @param user_data the user data pointer. @maybenil
 */
LINPHONE_PUBLIC void linphone_logging_service_cbs_set_user_data(LinphoneLoggingServiceCbs *cbs, void *user_data);

/**
 * @brief Gets the user_data pointer back.
 * @param cbs the #LinphoneLoggingServiceCbs object. @notnil
 * @return the user data pointer. @maybenil
 */
LINPHONE_PUBLIC void *linphone_logging_service_cbs_get_user_data(const LinphoneLoggingServiceCbs *cbs);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * @brief Gets the logging service listener.
 * @param log_service the #LinphoneLoggingService object
 * @deprecated 19/02/2019 Use add_callbacks / remove_callbacks instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneLoggingServiceCbs *linphone_logging_service_get_callbacks(const LinphoneLoggingService *log_service);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // _LINPHONE_LOG_H_
