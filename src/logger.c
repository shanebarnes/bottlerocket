/**
 * @file      logger.c
 * @brief     Logger implementation.
 * @author    Shane Barnes
 * @date      02 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_date.h"
#include "util_string.h"
#include "mutex_obj.h"

#include <stdarg.h>
#include <stdio.h>

static enum logger_level     static_level = LOGGER_LEVEL_ALL;
static struct mutexobj      *lock         = NULL;
static struct output_if_ops *output_if    = NULL;
static int32_t               refcount     = 0;

/**
 * @see See header file for interface comments.
 */
bool logger_create(void)
{
    bool retval = false;

    //@todo Replace with a read/write lock for multiple threads?
    lock = (struct mutexobj *)malloc(sizeof(struct mutexobj));

    if (lock != NULL)
    {
        retval = mutexobj_create(lock);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool logger_destroy(void)
{
    bool retval = false;

    if (lock != NULL)
    {
        retval = mutexobj_destroy(lock);
        free(lock);
        lock = NULL;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool logger_set_output(struct output_if_ops * const obj)
{
    bool retval = false;

    if (obj != NULL)
    {
        output_if = obj;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool logger_set_level(const enum logger_level level)
{
    bool retval = false;

    if ((lock != NULL) && (level <= LOGGER_LEVEL_MAX))
    {
        mutexobj_lock(lock);
        static_level = level;
        mutexobj_unlock(lock);

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
        case LOGGER_LEVEL_ALL:
            retval = (char*)"ALL";
            break;
        case LOGGER_LEVEL_TRACE:
            retval = (char*)"TRACE";
            break;
        case LOGGER_LEVEL_DEBUG:
            retval = (char*)"DEBUG";
            break;
        case LOGGER_LEVEL_INFO:
            retval = (char*)"INFO";
            break;
        case LOGGER_LEVEL_WARN:
            retval = (char*)"WARN";
            break;
        case LOGGER_LEVEL_ERROR:
            retval = (char*)"ERROR";
            break;
        case LOGGER_LEVEL_OFF:
            retval = (char*)"OFF";
            break;
        default:
            retval = NULL;
            break;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
void logger_printf(const enum logger_level level, const char *format, ...)
{
    int32_t error = 0, len;
    char msgbuf[128], outbuf[256], timebuf[32];
    va_list args;
    uint64_t sec = 0, nsec = 0;
    enum logger_level setlevel = LOGGER_LEVEL_OFF;
    bool dropmsg = false;

    if ((lock != NULL) &&
        (output_if != NULL))
    {
        mutexobj_lock(lock);
        // At the expense of more processing overhead, stop logging messages if
        // the number of incomplete logging transactions exceeds 10 in the event
        // that the high number of concurrent transactions is due to an
        // unintended circular dependency.
        if (refcount > 10)
        {
            dropmsg = true;
            fprintf(stderr,
                    "%s: preventing unintended recursion\n",
                    __FUNCTION__);
        }
        else
        {
            setlevel = static_level;
            refcount++;
        }
        mutexobj_unlock(lock);

        if ((dropmsg == false) &&
            (level >= setlevel) &&
            (level < LOGGER_LEVEL_OFF))
        {
            va_start(args, format);
            error = vsnprintf(msgbuf, sizeof(msgbuf), format, args);
            va_end(args);

            if (error >= 0)
            {
                utildate_gettvtime(DATE_CLOCK_REALTIME, &sec, &nsec);
                utildate_gettsformat(sec,
                                     UNIT_TIME_SEC,
                                     "%Y-%m-%dT%H:%M:%S",
                                     timebuf,
                                     sizeof(timebuf));

                len = utilstring_concat(outbuf,
                                        sizeof(outbuf),
                                        "%s.%06u [%-5s]: %s",
                                        timebuf,
                                        (uint32_t)nsec / 1000,
                                        logger_get_level_string(level),
                                        msgbuf);

                if (len > 0)
                {
                    output_if->oio_send(outbuf, len);
                }
            }
        }

        if (dropmsg == false)
        {
            mutexobj_lock(lock);
            refcount--;
            mutexobj_unlock(lock);
        }
    }
}
