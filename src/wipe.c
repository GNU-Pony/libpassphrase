/**
 * libpassphrase – Personalisable library for TTY passphrase reading
 * 
 * Copyright © 2013, 2014, 2015  Mattias Andrée (maandree@member.fsf.org)
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>

#define PASSPHRASE_USE_DEPRECATED
#include "passphrase.h"
#include "passphrase_helper.h"



#ifdef __GNUC__
# pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
#endif
/* Must positively absolutely not be flagged as possible to optimise away as it depends on configurations,
   and programs that uses this library must not be forced to be recompiled if the library is reconfigured. */


/**
 * `memset`, except calls to it cannot be removed by the compiler.
 */
void* (*volatile passphrase_explicit_memset________________)(void*, int, size_t) = memset;


/**
 * Forcefully write NUL characters to a passphrase
 * 
 * @param  ptr  The password to wipe
 * @param  n    The number of characters to wipe
 */
#ifdef __GNUC__
__attribute__((optimize("-O0")))
#endif
void passphrase_wipe(char* ptr, size_t n)
{

  passphrase_explicit_memset________________(ptr, 0, n);
}

/**
 * Forcefully write NUL characters to a passphrase
 * 
 * @param  ptr The password to wipe
 */
#ifdef __GNUC__
__attribute__((optimize("-O0")))
#endif
void passphrase_wipe1(char* ptr)
{

  passphrase_explicit_memset________________(ptr, 0, strlen(ptr));
}


#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif

