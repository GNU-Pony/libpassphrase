/**
 * libpassphrase – Personalisable library for TTY passphrase reading
 * 
 * Copyright © 2013, 2014  Mattias Andrée (maandree@member.fsf.org)
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
#ifndef __PASSPHRASE_H__
#define __PASSPHRASE_H__


/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be wiped `free`:ed, `NULL` on error
 */
extern char* passphrase_read(void);

/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read` requires
 */
extern void passphrase_disable_echo(void);

/**
 * Undo the actions of `passphrase_disable_echo`
 */
extern void passphrase_reenable_echo(void);


#endif

