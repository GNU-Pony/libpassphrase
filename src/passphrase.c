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


#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
/**
 * The original TTY settings
 */
static struct termios saved_stty;
#endif


#ifndef PASSPHRASE_REALLOC
static inline char* xrealloc(char* array, size_t cur_size, size_t new_size)
{
  char* rc = malloc(new_size * sizeof(char));
  size_t i;
  if (rc)
    for (i = 0; i < cur_size; i++)
	*(rc + i) = *(array + i);
  for (i = 0; i < cur_size; i++)
    *(array + i) = 0;
  free(array);
  return rc;
}
#else
#define xrealloc(array, _cur_size, new_size)  realloc(array, (new_size) * sizeof(char))
#endif


#if defined(PASSPHRASE_MOVE) && !defined(PASSPHRASE_STAR) && !defined(PASSPHRASE_ECHO)
#  define xprintf(...)  /* do nothing */
#  define xflush()      /* do nothing */
#elif defined(PASSPHRASE_MOVE) || defined(PASSPHRASE_STAR)
#  define xprintf       printf
#  define xflush()      fflush(stdout)
#else
#  define xflush()      fflush(stdout)
#endif


#ifdef PASSPHRASE_MOVE
#  if defined(PASSPHRASE_STAR)
#    define xputchar(C)  ({ if ((c & 0xC0) != 0x80)  putchar('*'); })
#  elif defined(PASSPHRASE_ECHO)
#    define xputchar(C)  putchar(C)
#  else
#    define xputchar(C)  ({ /* be silent */ })
#  endif
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
  long len =  0;
#ifdef PASSPHRASE_MOVE
  long point = 0;
  long i = 0;
#  ifdef PASSPHRASE_OVERRIDE
#    if defined(PASSPHRASE_INSERT) && defined(DEFAULT_INSERT)
  char insert = 1;
#    elif defined(PASSPHRASE_INSERT)
  char insert = 0;
#    endif
#  endif
#endif
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
#if defined(PASSPHRASE_MOVE)
	  /* \e[1~  \eOH  ^A  -1  home   */
	  /* \e[2~            -2  insert */
	  /* \e[3~  ^D        -3  delete */
	  /* \e[4~  \eOF  ^E  -4  end    */
	  /* \8     \127      -5  erase  */
	  /* \e[C   ^F        -6  right  */
	  /* \e[D   ^B        -7  left   */
	  int cc = 0;
#ifdef PASSPHRASE_DEDICATED
	  if (c == '\033')
	    {
	      c = getchar();
	      if (c == 'O')
		{
		  c = getchar();
		  if      (c == 'H')  cc = -1;
		  else if (c == 'F')  cc = -4;
		}
	      else if (c == '[')
		{
		  c = getchar();
		  if      (c == 'C')  cc = -6;
		  else if (c == 'D')  cc = -7;
		  else if (('1' <= c) && (c <= '4') && (getchar() == '~'))
		    cc = -(c - '0');
		}
	    }
	  else
#endif
	    if ((c == 8) || (c == 127))
	      cc = -5;
	    else if ((c < 0) || (c >= ' '))
	      cc = ((int)c) & 255;
#ifdef PASSPHRASE_CONTROL
	    else if (c == 'A' - '@')  cc = -1;
	    else if (c == 'B' - '@')  cc = -7;
	    else if (c == 'D' - '@')  cc = -3;
	    else if (c == 'E' - '@')  cc = -4;
	    else if (c == 'F' - '@')  cc = -6;
#endif
	  
	  if (cc > 0)
	    {
	      c = (char)cc;
	      if (point == len)
		{
		  xputchar(c);
		  *(rc + len++) = c;
		}
#ifdef PASSPHRASE_INSERT
	      else
#ifdef PASSPHRASE_OVERRIDE
		if (insert)
#endif
		  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wempty-body"
		    if ((c & 0xC0) != 0x80)
		      xprintf("\033[@");
#pragma GCC diagnostic pop
		    xputchar(c);
		    for (i = point; i < len; i++)
		      *(rc + i + 1) = *(rc + i);
		    len++;
		    *(rc + point++) = c;
		  }
#endif
#ifdef PASSPHRASE_OVERRIDE
		else
		  {
		    long n = 1;
		    char cn = c;
		    while (*(rc + point + n))
		      n++;
		    for (i = point; i < len; i++)
		      *(rc + i) = *(rc + i + n);
		    for (i = len - n; i < len; i++)
		      *(rc + point + i) = 0;
		    len -= n;
		    n = 0;
		    while (cn & 0x80)
		      {
			cn <<= 1;
			n++;
		      }
		    n = n ?: 1;
		    if (len + n > size)
		      {
			if ((rc = xrealloc(rc, size, size << 1L)) == NULL)
			  return NULL;
			size <<= 1L;
		      }
		    for (i = point; i < len; i++)
		      *(rc + i + n) = *(rc + i);
		    len += n;
		    for (i = 0; i < n; i++)
		      {
			if (i)
			  c = getchar();
			xputchar(c);
			*(rc + point++) = c;
		      }
		  }
#endif
	    }
	  else if ((cc == -1) && point) /* home */
	    {
	      xprintf("\033[%liD", point);
	      point = 0;
	    }
#if defined(PASSPHRASE_INSERT) && defined(PASSPHRASE_OVERRIDE)
	  else if (cc == -2) /* insert */
	    insert ^= 1;
#endif
#ifdef PASSPHRASE_DELETE
	  else if ((cc == -3) && (len != point)) /* delete */
	    {
	      xprintf("\033[P");
#ifdef PASSPHRASE_INVALID
	      *(rc + len) = 0;
#endif
	      do
		{
		  for (i = point; i < len; i++)
		    *(rc + i) = *(rc + i + 1);
		  len--;
		}
	      while ((len != point) && ((*(rc + point) & 0xC0) == 0x80));
	    }
#endif
	  else if ((cc == -4) && (len != point)) /* end */
	    {
	      xprintf("\033[%liC", len - point);
	      point = len;
	    }
	  else if ((cc == -5) && point) /* erase */
	    {
	      char redo = 1;
	      xprintf("\033[D\033[P");
#ifdef PASSPHRASE_INVALID
	      *(rc + len) = 0;
#endif
	      while (redo)
		{
		  redo = (*(rc + point) & 0xC0) == 0x80;
		  for (i = point; i <= len; i++)
		    *(rc + i - 1) = *(rc + i);
		  point--;
		  len--;
		}
	    }
	  else if ((cc == -6) && (len != point)) /* right */
	    {
	      xprintf("\033[C");
	      do
		point++;
	      while ((len != point) && ((*(rc + point) & 0xC0) == 0x80));
	    }
	  else if ((cc == -7) && point) /* left */
	    {
	      char redo = 1;
	      xprintf("\033[D");
	      while (redo)
		redo = (*(rc + point--) & 0xC0) == 0x80;
	    }
	  
#elif defined(PASSPHRASE_STAR)
	  if ((c == 8) || (c == 127))
	    {
	      if (len == 0)
		continue;
	      xprintf("\033[D \033[D");
	      xflush();
	      *(rc + --len) = 0;
	      continue;
	    }
	  if ((c & 0xC0) != 0x80)
	    putchar('*');
	  *(rc + len++) = c;
#else
	  *(rc + len++) = c;
#endif
	  
	  xflush();
	  if (len == size)
	    {
	      if ((rc = xrealloc(rc, size, size << 1L)) == NULL)
		return NULL;
	      size <<= 1L;
	    }
	}
    }
  
  /* NUL-terminate passphrase */
  *(rc + len) = 0;
  
#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
  printf("\n");
#endif
  return rc;
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
  stty.c_lflag &= ~ECHO;
#if defined(PASSPHRASE_STAR) || defined(PASSPHRASE_MOVE)
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
#if !defined(PASSPHRASE_ECHO) || defined(PASSPHRASE_MOVE)
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty);
#endif
}

