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
#include <unistd.h>
#include <termios.h>

#define PASSPHRASE_USE_DEPRECATED
#include "passphrase.h"
#include "passphrase_helper.h"



#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE) || defined(PASSPHRASE_METER)
# define NEED_TERMIOS
#else
# ifdef NEED_TERMIOS
#  undef NEED_TERMIOS
# endif
#endif /* !PASSPHRASE_ECHO || PASSPHRASE_MOVE || PASSPHRASE_METER */



#if defined(NEED_TERMIOS)
/**
 * The original TTY settings
 */
static struct termios saved_stty;
#endif /* NEED_TERMIOS */



/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read` requires
 */
#if defined(__GNUC__) && !defined(NEED_TERMIOS)
__attribute__((const))
#endif /* __GNUC__ && !NEED_TERMIOS */
void passphrase_disable_echo(void)
{
  passphrase_disable_echo1(STDIN_FILENO);
}


/**
 * Undo the actions of `passphrase_disable_echo`
 */
#if defined(__GNUC__) && !defined(NEED_TERMIOS)
__attribute__((const))
#endif /* __GNUC__ && !NEED_TERMIOS */
void passphrase_reenable_echo(void)
{
  passphrase_reenable_echo1(STDIN_FILENO);
}

/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read2` requires
 * 
 * @param  fdin  File descriptor for input
 */
#if defined(__GNUC__) && !defined(NEED_TERMIOS)
__attribute__((const))
#endif /* __GNUC__ && !NEED_TERMIOS */
void passphrase_disable_echo1(int fdin)
{
#if defined(NEED_TERMIOS)
  struct termios stty;
  
  tcgetattr(fdin, &stty);
  saved_stty = stty;
  stty.c_lflag &= (tcflag_t)~ECHO;
# if defined(PASSPHRASE_STAR) || defined(PASSPHRASE_TEXT) || defined(PASSPHRASE_MOVE) || defined(PASSPHRASE_METER)
  stty.c_lflag &= (tcflag_t)~ICANON;
# endif /* PASSPHRASE_STAR || PASSPHRASE_TEXT || PASSPHRASE_MOVE || PASSPHRASE_METER */
  tcsetattr(fdin, TCSAFLUSH, &stty);
#else /* NEED_TERMIOS */
  (void) fdin;
#endif /* NEED_TERMIOS */
}


/**
 * Undo the actions of `passphrase_disable_echo1`
 * 
 * @param  fdin  File descriptor for input
 */
#if defined(__GNUC__) && !defined(NEED_TERMIOS)
__attribute__((const))
#endif /* __GNUC__ && !NEED_TERMIOS */
void passphrase_reenable_echo1(int fdin)
{
#if defined(NEED_TERMIOS)
  tcsetattr(fdin, TCSAFLUSH, &saved_stty);
#else /* NEED_TERMIOS */
  (void) fdin;
#endif /* NEED_TERMIOS */
}

