/**
 * @file   logger.c
 * @author Shane Barnes
 * @date   02 Mar 2016
 * @brief  Logger implementation.
 */

#include "logger.h"
#include "manip_date.h"
#include "rwlock_instance.h"

#include <stdarg.h>
#include <stdio.h>

static enum logger_level      static_level = LOGGER_LEVEL_ALL;
static struct rwlock_instance lock;


/**
 * @see See header file for interface comments.
 */
bool logger_create(void)
{
    return rwlock_instance_create(&lock);
}

/**
 * @see See header file for interface comments.
 */
bool logger_destroy(void)
{
    return rwlock_instance_destroy(&lock);
}

/**
 * @see See header file for interface comments.
 */
bool logger_set_level(const enum logger_level level)
{
    bool retval = false;

    if ((level >= LOGGER_LEVEL_MIN) && (level <= LOGGER_LEVEL_MAX))
    {
        rwlock_instance_wrlock(&lock);
        static_level = level;
        rwlock_instance_unlock(&lock);
        retval = true;
    }

    return retval;
}

/**
 * @brief Get a character string representation of an enumerated log level.
 *
 * @param[in] level The enumerated log level.
 *
 * @return A character string representation of an enumerated log level (NULL on
 *         error).
 */
static char *logger_get_level_string(const enum logger_level level)
{
    char *retval = NULL;

    switch (level)
    {
        case LOGGER_LEVEL_OFF:
            retval = "OFF";
            break;
        case LOGGER_LEVEL_ALL:
            retval = "ALL";
            break;
        case LOGGER_LEVEL_TRACE:
            retval = "TRACE";
            break;
        case LOGGER_LEVEL_DEBUG:
            retval = "DEBUG";
            break;
        case LOGGER_LEVEL_INFO:
            retval = "INFO";
            break;
        case LOGGER_LEVEL_WARN:
            retval = "WARN";
            break;
        case LOGGER_LEVEL_ERROR:
            retval = "ERROR";
            break;
        default:
            retval = NULL;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
void logger_printf(const enum logger_level level, const char *format, ...)
{
    int32_t error = 0;
    char msgbuf[1024], timebuf[512];
    va_list args;
    uint64_t sec = 0, nsec = 0;
    rwlock_instance_rdlock(&lock);
    enum logger_level setlevel = static_level;
    rwlock_instance_unlock(&lock);

    if ((setlevel != LOGGER_LEVEL_OFF) &&
        ((level >= setlevel) || (setlevel == LOGGER_LEVEL_ALL)))
    {
        va_start(args, format);
        error = vsnprintf(msgbuf, sizeof(msgbuf), format, args);
        va_end(args);

        if (error >= 0)
        {
            manip_date_time(DATE_CLOCK_REALTIME, &sec, &nsec);
            manip_date_time_format(sec,
                                   UNIT_TIME_SEC,
                                   "%Y-%m-%dT%H:%M:%S",
                                   timebuf,
                                   sizeof(timebuf));

            fprintf(stdout,
                    "%s.%06u [%-5s]: %s",
                    timebuf,
                    (uint32_t)nsec / 1000,
                    logger_get_level_string(level),
                    msgbuf);
        }
    }
}
