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
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "passphrase.h"
#include "passphrase_helper.h"


#define START_PASSPHRASE_LIMIT  32




#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
/**
 * The original TTY settings
 */
static struct termios saved_stty;
#endif /* !PASSPHRASE_ECHO || PASSPHRASE_MOVE */



#ifndef PASSPHRASE_REALLOC
static char* xrealloc(char* array, size_t cur_size, size_t new_size)
{
  char* rc = malloc(new_size * sizeof(char));
  size_t i;
  if (rc)
    for (i = 0; i < cur_size; i++)
	*(rc + i) = *(array + i);
  passphrase_wipe(array, cur_size);
  free(array);
  return rc;
}
#else /* !PASSPHRASE_REALLOC */
# define xrealloc(array, _cur_size, new_size)  realloc(array, (new_size) * sizeof(char))
#endif /* !PASSPHRASE_REALLOC */



#if defined(PASSPHRASE_DEDICATED) && defined(PASSPHRASE_MOVE)
static int get_dedicated_control_key(void)
{
  int c = getchar();
  if (c == 'O')
    {
      c = getchar();
      if (c == 'H')  return KEY_HOME;
      if (c == 'F')  return KEY_END;
    }
  else if (c == '[')
    {
      c = getchar();
      if (c == 'C')  return KEY_RIGHT;
      if (c == 'D')  return KEY_LEFT;
      if (('1' <= c) && (c <= '4') && (getchar() == '~'))
	return -(c - '0');
    }
  return 0;
}
#endif /* PASSPHRASE_DEDICATED && PASSPHRASE_MOVE */



#ifdef PASSPHRASE_MOVE
static int get_key(int c)
{
# ifdef PASSPHRASE_DEDICATED
  if (c == '\033')             return get_dedicated_control_key();
# endif /* PASSPHRASE_DEDICATED */
  if ((c == 8) || (c == 127))  return KEY_ERASE;
  if ((c < 0) || (c >= ' '))   return ((int)c) & 255;
# ifdef PASSPHRASE_CONTROL
  if (c == 'A' - '@')          return KEY_HOME;
  if (c == 'B' - '@')          return KEY_LEFT;
  if (c == 'D' - '@')          return KEY_DELETE;
  if (c == 'E' - '@')          return KEY_END;
  if (c == 'F' - '@')          return KEY_RIGHT;
# endif /* PASSPHRASE_CONTROL */
  return 0;
}
#endif /* PASSPHRASE_MOVE */



/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be wiped `free`:ed, `NULL` on error
 */
char* passphrase_read(void)
{
  char* rc = malloc(START_PASSPHRASE_LIMIT * sizeof(char));
  size_t size = START_PASSPHRASE_LIMIT;
  size_t len =  0;
#ifdef PASSPHRASE_MOVE
  size_t point = 0;
  size_t i = 0;
# if defined(PASSPHRASE_OVERRIDE) && defined(PASSPHRASE_INSERT)
  char insert = DEFAULT_INSERT_VALUE;
# endif /* PASSPHRASE_OVERRIDE && PASSPHRASE_INSERT */
#endif /* PASSPHRASE_MOVE */
#ifdef PASSPHRASE_TEXT
  size_t printed_len = 0;
#endif /* PASSPHRASE_TEXT */
  int c;
#ifdef PASSPHRASE_MOVE
  int cc;
#endif
  
  if (rc == NULL)
    return NULL;
  
#ifdef PASSPHRASE_TEXT
  xprintf("%s%zn", PASSPHRASE_TEXT_EMPTY, &printed_len);
  if (printed_len)
    xprintf("\e[%zuD", printed_len);
#endif /* PASSPHRASE_TEXT */
  
  /* Read password until EOF or Enter, skip all \0 as that
     is probably not a part of the passphrase (good luck typing
     that in X.org) and can be echoed into stdin by the kernel. */
  for (;;)
    {
      c = getchar();
      if ((c < 0) || (c == '\n'))  break;
      if (c == 0)                  continue;
      
#if defined(PASSPHRASE_MOVE)
      cc = get_key(c);
      if (cc > 0)
	{
	  c = (char)cc;
	  if (point == len)
	    append_char();
# ifdef PASSPHRASE_INSERT
	  else
#  ifdef PASSPHRASE_OVERRIDE
	    if (insert)
#  endif /* PASSPHRASE_OVERRIDE */
	      insert_char();
# endif /* PASSPHRASE_INSERT */
# ifdef PASSPHRASE_OVERRIDE
	    else
	      override_char();
# endif /* PASSPHRASE_OVERRIDE */
	}
# if defined(PASSPHRASE_INSERT) && defined(PASSPHRASE_OVERRIDE)
      else if (cc == KEY_INSERT)                      insert ^= 1;
# endif /* PASSPHRASE_INSERT && PASSPHRASE_OVERRIDE */
# ifdef PASSPHRASE_DELETE
      else if ((cc == KEY_DELETE) && (len != point))  { delete_next(); print_delete(); }
# endif /* PASSPHRASE_DELETE */
      else if ((cc == KEY_ERASE) && point)            { erase_prev(); print_erase(); }
      else if ((cc == KEY_HOME)  && (point != 0))     move_home();
      else if ((cc == KEY_END)   && (point != len))   move_end();
      else if ((cc == KEY_RIGHT) && (point != len))   move_right();
      else if ((cc == KEY_LEFT)  && (point != 0))     move_left();
      
#elif defined(PASSPHRASE_STAR) || defined(PASSPHRASE_TEXT) /* PASSPHRASE_MOVE */
      if ((c == 8) || (c == 127))
	{
	  if (len == 0)
	    continue;
	  erase_prev();
	  print_erase();
	  xflush();
# ifdef DEBUG
	  goto debug;
# else /* DEBUG */
	  continue;
# endif /* DEBUG */
	}
      append_char();
      
#else /* PASSPHRASE_MOVE, PASSPHRASE_STAR || PASSPHRASE_TEXT */
      append_char();
#endif /* PASSPHRASE_MOVE, PASSPHRASE_STAR || PASSPHRASE_TEXT */
      
      xflush();
      if (len == size)
	{
	  if ((rc = xrealloc(rc, (size_t)size, (size_t)size << 1)) == NULL)
	    return NULL;
	  size <<= 1L;
	}
      
#ifdef DEBUG
# ifdef __GNUC__
#  pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-label"
# endif
    debug:
      {
	size_t n = 0;
	for (i = point; i < len; i++)
	  if ((*(rc + i) & 0xC0) != 0x80)
	    n++;
	*(rc + len) = 0;
	if (n)
	  fprintf(stderr, "\033[s\033[H\033[K%s\033[%zuD\033[01;34m%s\033[00m\033[u", rc, n, rc + point);
	else
	  fprintf(stderr, "\033[s\033[H\033[K%s\033[01;34m%s\033[00m\033[u", rc, rc + point);
	fflush(stderr);
      }
#endif /* DEBUG */
    }
  
  /* NUL-terminate passphrase */
  *(rc + len) = 0;
  
#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
  fprintf(stderr, "\n");
#endif /* !PASSPHRASE_ECHO || PASSPHRASE_MOVE */
  return rc;
}


#ifdef __GNUC__
# pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
#endif
/* Must positively absolutely not be flagged as possible to optimise away as it depends on configurations,
   and programs that uses this library must not be forced to be recompiled if the library is reconfigured. */


/**
 * Used to make sure that `passphrase_wipe` is not optimised away even within this library
 */
volatile sig_atomic_t passphrase_wipe_volatile________________ = 1;

/**
 * Forcable write NUL characters to a passphrase
 * 
 * @param  ptr  The password to wipe
 * @param  n    The number of characters to wipe
 */
#ifdef __GNUC__
__attribute__((optimize("-O0")))
#endif
void passphrase_wipe(volatile char* ptr, size_t n)
{
  size_t i;
  for (i = 0; (i < n) && passphrase_wipe_volatile________________; i++)
    *(ptr + i) = 0;
}

/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read` requires
 */
void passphrase_disable_echo(void)
{
#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
  struct termios stty;
  
  tcgetattr(STDIN_FILENO, &stty);
  saved_stty = stty;
  stty.c_lflag &= (tcflag_t)~ECHO;
# if defined(PASSPHRASE_STAR) || defined(PASSPHRASE_TEXT) || defined(PASSPHRASE_MOVE)
  stty.c_lflag &= (tcflag_t)~ICANON;
# endif /* PASSPHRASE_STAR || PASSPHRASE_TEXT || PASSPHRASE_MOVE */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty);
#endif /* !PASSPHRASE_ECHO || PASSPHRASE_MOVE */
}


/**
 * Undo the actions of `passphrase_disable_echo`
 */
void passphrase_reenable_echo(void)
{
#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty);
#endif /* !PASSPHRASE_ECHO || !PASSPHRASE_MOVE */
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif

