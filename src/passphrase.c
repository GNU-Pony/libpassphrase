/**
 * libpassphrase – Personalisable library for TTY passphrase reading
 * 
 * Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
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

#include "passphrase.h"


#define START_PASSPHRASE_LIMIT  32


#ifndef PASSPHRASE_ECHO
/**
 * The original TTY settings
 */
static struct termios saved_stty;
#endif


#ifndef PASSPHRASE_REALLOC
static inline char* xrealloc(char* array, size_t size)
{
  char* rc = malloc(size);
  int i;
  if (rc)
    for (i = 0; *(array + i); i++)
      {
	*(rc + i) = *(array + i);
	*(array + i) = 0;
      }
  else
    for (i = 0; *(array + i); i++)
      *(array + i) = 0;
  free(array);
  return rc;
}
#else
#define  xrealloc  realloc
#endif


/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be wiped `free`:ed, `NULL` on error
 */
char* passphrase_read(void)
{
  char* rc = malloc(START_PASSPHRASE_LIMIT * sizeof(char));
  long size = START_PASSPHRASE_LIMIT;
  long len = 0;
  int c;
  
  if (rc == NULL)
    return NULL;
  
  /* Read password until EOF or Enter, skip all \0 as that
     is probably not a part of the passphrase (good luck typing
     that in X.org) and can be echoed into stdin by the kernel. */
  for (;;)
    {
      c = getchar();
      if ((c < 0) || (c == '\n'))
	break;
      if (c != 0)
        {
#ifdef PASSPHRASE_STAR
	  if ((c == 8) || (c == 127))
	    {
	      if (len == 0)
		continue;
	      printf("\033[D \033[D");
	      fflush(stdout);
	      *(rc + --len) = 0;
	      continue;
	    }
	  putchar('*');
#endif
	  *(rc + len++) = c;
	  if (len == size)
	    if ((rc = xrealloc(rc, (size <<= 1L) * sizeof(char))) == NULL)
	      return rc;
	}
    }
  
  /* NUL-terminate passphrase */
  *(rc + len) = 0;
  
#ifndef PASSPHRASE_ECHO
  printf("\n");
#endif
  return rc;
}


/**
 * Disable echoing and do anything else to the terminal settnings `passphrase_read` requires
 */
void passphrase_disable_echo(void)
{
#ifndef PASSPHRASE_ECHO
  struct termios stty;
  
  tcgetattr(STDIN_FILENO, &stty);
  saved_stty = stty;
  stty.c_lflag &= ~ECHO;
#ifdef PASSPHRASE_STAR
  stty.c_lflag &= ~ICANON;
#endif
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty);
#endif
}


/**
 * Undo the actions of `passphrase_disable_echo`
 */
void passphrase_reenable_echo(void)
{
#ifndef PASSPHRASE_ECHO
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty);
#endif
}

