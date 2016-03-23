/**
 * @file   util_string.h
 * @author Shane Barnes
 * @date   07 Mar 2016
 * @brief  String utility interface.
 */

#ifndef _UTIL_STRING_H_
#define _UTIL_STRING_H_

#include "system_types.h"

/**
 * @brief Compare two strings.
 *
 * @param[in] str1       The first string.
 * @param[in] str2       The second string.
 * @param[in] ignoreCase True if the case of characters should not influence the
 *                       result of the comparison.
 *
 * @return True if the strings match.
 */
bool util_string_compare(const char * const str1,
                         const char * const str2,
                         const bool ignorecase);

/**
 * @brief Concatenate strings, integers, etc. and copy into a string buffer.
 *
 * @param[in,out] buf    A string buffer to store the concatenated strings,
 *                       integers, etc.
 * @param[in]     len    The size of the string buffer in bytes.
 * @param[in]     format A string containing the concatenation format.
 * @param[in]      ...   A variable size argument list.
 *
 * @return The length of the string (if the buffer had been sufficiently large)
 *         containing the concatenated strings, integers, etc (-1 on error).
 */
int32_t util_string_concat(char * const str,
                           const size_t len,
                           const char * const format,
                           ...);

/**
 * @brief Parse a string based on a string containing the match format. All
 *        matches are inserted into a variable size argument list.
 *
 * @param[in] str    The string to parse.
 * @param[in] format A string containing the match format.
 * @param[in]  ...   A variable size argument list.
 *
 * @return The number of matches found (-1 on error).
 */
int32_t util_string_parse(const char * const str,
                          const char * const format,
                          ...);

/**
 * @brief Convert a string (must not be a literal) to lower case.
 *
 * @param[in,out] str A string to convert to lower case.
 *
 * @return Void.
 */
void util_string_to_lower(char * const str);

/**
 * @brief Convert a string (must not be a literal) to upper case.
 *
 * @param[in,out] str A string to convert to upper case.
 *
 * @return Void.
 */
void util_string_to_upper(char * const str);

#endif // _UTIL_STRING_H_