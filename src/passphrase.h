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
#ifndef PASSPHRASE_H
#define PASSPHRASE_H

#include <stddef.h>

#if defined(__GNUC__) && !defined(PASSPHRASE_USE_DEPRECATED)
# define PASSPHRASE_DEPRECATED(MSG)  __attribute__((__deprecated__(MSG)))
#else
# define PASSPHRASE_DEPRECATED(MSG)  /* ignore */
#endif



/**
 * `passphrase_read2` shall not do any thing
 * special, just accept the passphrase. This should
 * be used when getting authentication.
 * Should not be combined with `PASSPHRASE_READ_NEW`.
 */
#define PASSPHRASE_READ_EXISTING  0

/**
 * `passphrase_read2` shall draw a pasphrase
 * strength meter if such capability is
 * available. This should be used when create
 * a new passphrase.
 * Should not be combined with `PASSPHRASE_READ_EXISTING`.
 */
#define PASSPHRASE_READ_NEW  1

/**
 * `passphrase_read2` may do as it please with the
 * the screen. This is only used if combined with
 * `PASSPHRASE_READ_NEW` and not with
 * `PASSPHRASE_READ_BELOW_FREE`. `passphrase_read2`
 * will create make a line below the new current
 * line and use that line to draw the passphrase
 * strength meter if such capability is available.
 */
#define PASSPHRASE_READ_SCREEN_FREE  2

/**
 * `passphrase_read2` may do as it please with the
 * line below the current line. This is only used
 * if combined with `PASSPHRASE_READ_NEW`.
 * `passphrase_read2` will draw the passphrase
 * strength meter on the line below if such
 * capability is available.
 */
#define PASSPHRASE_READ_BELOW_FREE  4



/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be wiped and `free`:ed, `NULL` on error
 */
PASSPHRASE_DEPRECATED("Please use 'passphrase_read2' instead.")
char* passphrase_read(void);

/**
 * Reads the passphrase
 * 
 * @param   fdin   File descriptor for input
 * @param   flags  Settings, a combination of the constants:
 *                 * PASSPHRASE_READ_EXISTING
 *                 * PASSPHRASE_READ_NEW
 *                 * PASSPHRASE_READ_SCREEN_FREE
 *                 * PASSPHRASE_READ_BELOW_FREE
 *                 Invalid input is ignored, to make use the
 *                 application will work.
 * @return         The passphrase, should be wiped and `free`:ed, `NULL` on error
 */
char* passphrase_read2(int, int);

/**
 * Forcefully write NUL characters to a passphrase
 * 
 * @param  ptr  The password to wipe
 * @param  n    The number of characters to wipe
 */
void passphrase_wipe(char*, size_t);

/**
 * Forcefully write NUL characters to a passphrase
 * 
 * @param  ptr  The password to wipe
 */
void passphrase_wipe1(char*);

/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read` requires
 */
PASSPHRASE_DEPRECATED("Please use 'passphrase_disable_echo1' instead.")
void passphrase_disable_echo(void);

/**
 * Undo the actions of `passphrase_disable_echo`
 */
PASSPHRASE_DEPRECATED("Please use 'passphrase_reenable_echo1' instead.")
void passphrase_reenable_echo(void);

/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read2` requires
 * 
 * @param  fdin  File descriptor for input
 */
void passphrase_disable_echo1(int);

/**
 * Undo the actions of `passphrase_disable_echo1`
 * 
 * @param  fdin  File descriptor for input
 */
void passphrase_reenable_echo1(int);



#undef PASSPHRASE_DEPRECATED

#endif

