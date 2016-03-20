/**
 * @file   logger.h
 * @author Shane Barnes
 * @date   02 Mar 2016
 * @brief  Logger interface.
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "output_if_instance.h"
#include "system_types.h"

enum logger_level
{
    LOGGER_LEVEL_MIN   = 0,
    LOGGER_LEVEL_OFF   = 0,
    LOGGER_LEVEL_ALL   = 1,
    LOGGER_LEVEL_TRACE = 2,
    LOGGER_LEVEL_DEBUG = 3,
    LOGGER_LEVEL_INFO  = 4,
    LOGGER_LEVEL_WARN  = 5,
    LOGGER_LEVEL_ERROR = 6,
    LOGGER_LEVEL_MAX   = 6
};

/**
 * @brief Create a logger singleton.
 *
 * @return True if a logger singleton was created.
 */
bool logger_create(void);

/**
 * @brief Destroy a logger singleton.
 *
 * @return True if a logger singleton was destroyed.
 */
bool logger_destroy(void);

/**
 * @brief Set the logger output interface instance.
 *
 * @param[in] instance A pointer to the output interface instance.
 *
 * @return True if the new logger output interface instance was set.
 */
bool logger_set_output(struct output_if_ops * const instance);

/**
 * @brief Set the logger level.
 *
 * @param[in] level The new logger level.
 *
 * @return True if the new logger level was set.
 */
bool logger_set_level(const enum logger_level level);

/**
 * @brief Print a log message to an output stream based on a format string and a
 *        variable number of message arguments. Logger information will be
 *        prepended to the log message.
 *
 * @param[in] level  The severity level of the log message.
 * @param[in] format The log message format for the message arguments.
 * @param[in] ...    A variable number of message arguments.
 *
 * @return Void.
 */
void logger_printf(const enum logger_level level, const char *format, ...);

#endif // _LOGGER_H_
