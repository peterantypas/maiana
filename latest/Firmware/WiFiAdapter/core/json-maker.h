
/*
<https://github.com/rafagafe/tiny-json>

  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c) 2018 Rafa Garcia <rafagarcia77@gmail.com>.
  Permission is hereby  granted, free of charge, to any  person obtaining a copy
  of this software and associated  documentation files (the "Software"), to deal
  in the Software  without restriction, including without  limitation the rights
  to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
  copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
  IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
  FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
  AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
  LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <stddef.h>

#ifndef MAKE_JSON_H
#define	MAKE_JSON_H

#ifdef	__cplusplus
extern "C" {
#endif

/** @defgroup makejoson Make JSON.
  * @{ */

/** Open a JSON object in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_objOpen( char* dest, char const* name, size_t* remLen );

/** Close a JSON object in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_objClose( char* dest, size_t* remLen );

/** Used to finish the root JSON object. After call json_objClose().
  * @param dest Pointer to the end of JSON under construction.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_end( char* dest, size_t* remLen );

/** Open an array in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_arrOpen( char* dest, char const* name, size_t* remLen );

/** Close an array in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_arrClose( char* dest, size_t* remLen );
/** Add a text property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value A valid null-terminated string with the value.
  *              Backslash escapes will be added for special characters.
  * @param len Max length of value. < 0 for unlimit.  
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */  
char* json_nstr( char* dest, char const* name, char const* value, int len, size_t* remLen );

/** Add a text property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value A valid null-terminated string with the value.
  *              Backslash escapes will be added for special characters.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
static inline char* json_str( char* dest, char const* name, char const* value, size_t* remLen ) {
    return json_nstr( dest, name, value, -1, remLen );  
}

/** Add a boolean property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Zero for false. Non zero for true.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_bool( char* dest, char const* name, int value, size_t* remLen );

/** Add a null property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_null( char* dest, char const* name, size_t* remLen );

/** Add an integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_int( char* dest, char const* name, int value, size_t* remLen );

/** Add an unsigned integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_uint( char* dest, char const* name, unsigned int value, size_t* remLen );

/** Add a long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_long( char* dest, char const* name, long int value, size_t* remLen );

/** Add an unsigned long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_ulong( char* dest, char const* name, unsigned long int value, size_t* remLen );

/** Add a long long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_verylong( char* dest, char const* name, long long int value, size_t* remLen );

/** Add a double precision number property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @param remLen Pointer to remaining length of dest
  * @return Pointer to the new end of JSON under construction. */
char* json_double( char* dest, char const* name, double value, size_t* remLen );

/** @ } */

#ifdef	__cplusplus
}
#endif

#endif	/* MAKE_JSON_H */

