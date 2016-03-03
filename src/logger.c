/**
 * @file   logger.c
 * @author Shane Barnes
 * @date   02 Mar 2016
 * @brief  Logger implementation.
 */

#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static enum logger_level static_level = LOGGER_LEVEL_ALL;

/**
 * @see See header file for interface comments.
 */
bool logger_set_level(const enum logger_level level)
{
    bool retval = false;

    if ((level >= LOGGER_LEVEL_MIN) && (level <= LOGGER_LEVEL_MAX))
    {
        static_level = level;
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
    struct timeval tv;
    struct tm *tm = NULL;
    time_t time;

    if ((static_level != LOGGER_LEVEL_OFF) &&
        ((level >= static_level) || (static_level == LOGGER_LEVEL_ALL)))
    {
        va_start(args, format);
        error = vsprintf(msgbuf, format, args);
        va_end(args);

        if (error >= 0)
        {
            error = gettimeofday(&tv, NULL);
            time = tv.tv_sec;
            tm = localtime(&time);
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%S", tm);
            fprintf(stdout,
                    "%s.%06u [%-5s]: %s",
                    timebuf,
                    (uint32_t)tv.tv_usec,
                    logger_get_level_string(level),
                    msgbuf);
        }
    }
}
