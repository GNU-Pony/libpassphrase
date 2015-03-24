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
#ifndef PASSPHRASE_HELPER_H
#define PASSPHRASE_HELPER_H


/* Fix conflicting configurations */
#if defined(PASSPHRASE_TEXT) && defined(PASSPHRASE_STAR)
#  warning You cannot have both PASSPHRASE_TEXT and PASSPHRASE_STAR
#  undef PASSPHRASE_TEXT
#endif
#if defined(PASSPHRASE_STAR) && defined(PASSPHRASE_ECHO)
#  warning You cannot have both PASSPHRASE_STAR and PASSPHRASE_ECHO
#  undef PASSPHRASE_ECHO
#endif
#if defined(PASSPHRASE_TEXT) && defined(PASSPHRASE_ECHO)
#  warning You cannot have both PASSPHRASE_TEXT and PASSPHRASE_ECHO
#  undef PASSPHRASE_ECHO
#endif


/* Default texts */
#ifndef PASSPHRASE_STAR_CHAR
#  define PASSPHRASE_STAR_CHAR  "*"
#endif
#ifndef PASSPHRASE_TEXT_EMPTY
#  define PASSPHRASE_TEXT_EMPTY  "(empty)"
#endif
#ifndef PASSPHRASE_TEXT_NOT_EMPTY
#  define PASSPHRASE_TEXT_NOT_EMPTY  "(not empty)"
#endif



/* Control keys. */

/**
 * Home-key.
 * Character sequences: \e[1~  \eOH
 * Control-key combination: ^A
 */
#define KEY_HOME    -1

/**
 * Insert-key.
 * Character sequences: \e[2~
 */
#define KEY_INSERT  -2

/**
 * Delete-key.
 * Character sequences: \e[3~
 * Control-key combination: ^D
 */
#define KEY_DELETE  -3

/**
 * End-key.
 * Character sequences: \e[4~  \eOF
 * Control-key combination: ^E
 */
#define KEY_END     -4

/**
 * Erase-key, also known as backspace.
 * Character sequences: \d8  \d127
 */
#define KEY_ERASE   -5

/**
 * Right-key.
 * Character sequences: \e[C
 * Control-key combination: ^F
 */
#define KEY_RIGHT   -6

/**
 * Left-key.
 * Character sequences: \e[D
 * Control-key combination: ^B
 */
#define KEY_LEFT    -7



/* Custom fflush and fprintf */
#if defined(PASSPHRASE_STAR) || defined(PASSPHRASE_TEXT)
#  define xprintf(...)  fprintf(stderr, __VA_ARGS__)
#  define xflush()      fflush(stderr)
#elif defined(PASSPHRASE_MOVE) && !defined(PASSPHRASE_ECHO)
#  define xprintf(...)  ({ /* do nothing */ })
#  define xflush()      ({ /* do nothing */ })
#elif defined(PASSPHRASE_MOVE)
#  define xprintf(...)  fprintf(stderr, __VA_ARGS__)
#  define xflush()      fflush(stderr)
#else
#  define xflush()      fflush(stderr)
#endif



/* Custom putchar */
#if defined(PASSPHRASE_STAR)
#  define xputchar(C)  (((C & 0xC0) != 0x80) ? fprintf(stderr, "%s", PASSPHRASE_STAR_CHAR) : 0)
#elif defined(PASSPHRASE_ECHO) && defined(PASSPHRASE_MOVE)
#  define xputchar(C)  fputc(C, stderr)
#else
#  define xputchar(C)  ({ /* be silent */ })
#endif



/* Is insert active by default? */
#if defined(PASSPHRASE_OVERRIDE) && defined(PASSPHRASE_INSERT)
#  if defined(DEFAULT_INSERT)
#    define DEFAULT_INSERT_VALUE  1
#  else
#    define DEFAULT_INSERT_VALUE  0
#  endif
#endif



/* PASSPHRASE_INVALID's affect */
#if defined(PASSPHRASE_INVALID)
#  define null_terminate()  (*(rc + len) = 0)
#else
#  define null_terminate()  ({ /* do nothing*/ })
#endif



/* Implementation of the right-key's action */
#if defined(PASSPHRASE_TEXT)
#  define move_right()							\
  ({									\
    do									\
      point++;								\
    while ((len != point) && ((*(rc + point) & 0xC0) == 0x80));		\
  })
#else
#  define move_right()							\
  ({									\
    xprintf("\033[C");							\
    do									\
      point++;								\
    while ((len != point) && ((*(rc + point) & 0xC0) == 0x80));		\
  })
#endif


/* Implementation of the left-key's action */
#if defined(PASSPHRASE_TEXT)
#  define move_left()							\
  ({									\
    point--;								\
    while (point && ((*(rc + point) & 0xC0) == 0x80))			\
      point--;								\
  })
#else
#  define move_left()							\
  ({									\
    xprintf("\033[D");							\
    point--;								\
    while (point && ((*(rc + point) & 0xC0) == 0x80))			\
      point--;								\
  })
#endif


/* Implementation of the home-key's action */
#if defined(PASSPHRASE_TEXT)
#  define move_home()  (point = 0)
#else
#  define move_home()				\
  ({						\
    size_t n = 0;				\
    for (i = 0; i < point; i++)			\
      if ((*(rc + i) & 0xC0) != 0x80)		\
	n++;					\
    xprintf("\033[%zuD", n);			\
    point = 0;					\
  })
#endif


/* Implementation of the end-key's action */
#if defined(PASSPHRASE_TEXT)
#  define move_end()  (point = len)
#else
#  define move_end()				\
  ({						\
    size_t n = 0;				\
    for (i = point; i < len; i++)		\
      if ((*(rc + i) & 0xC0) != 0x80)		\
	n++;					\
    xprintf("\033[%zuC", n);			\
    point = len;				\
  })
#endif


/* Implementation of the delete-key's action upon the passphrase buffer */
#define delete_next()						\
  ({								\
    null_terminate();						\
    do								\
      {								\
	for (i = point; i < len; i++)				\
	  *(rc + i) = *(rc + i + 1);				\
	len--;							\
      }								\
    while ((len != point) && ((*(rc + point) & 0xC0) == 0x80));	\
  })


/* Implementation of the erase-key's action upon the passphrase buffer */
#if defined(PASSPHRASE_MOVE)
#define erase_prev()						\
  ({								\
    char redo = 1;						\
    null_terminate();						\
    while (redo)						\
      {								\
	redo = (*(rc + point - 1) & 0xC0) == 0x80;		\
	for (i = point; i < len; i++)				\
	  *(rc + i - 1) = *(rc + i);				\
	if (point <= len)					\
	  *(rc + len - 1) = *(rc + len);			\
	point--;						\
	len--;							\
      }								\
  })
#else
#  define erase_prev()  (*(rc + --len) = 0)
#endif


#ifdef PASSPHRASE_MOVE
#  define move_point()  (point++)
#else
#  define move_point()  ({ /* do nothing*/ })
#endif


#if defined(PASSPHRASE_TEXT)
#  define append_char()							\
  ({									\
    if (len == 0)							\
      {									\
    	xprintf("\033[K");						\
	xprintf("%s%zn", PASSPHRASE_TEXT_NOT_EMPTY, &printed_len);	\
	if (printed_len)						\
	  xprintf("\033[%zuD", printed_len);				\
      }									\
    *(rc + len++) = (char)c;						\
    move_point();							\
  })
#else
#  define append_char()	 (xputchar(c), *(rc + len++) = (char)c, move_point())
#endif


#if defined(PASSPHRASE_TEXT)
#  define insert_char()					\
  ({							\
    for (i = len; i > point; i--)			\
      *(rc + i) = *(rc + i - 1);			\
    len++;						\
    *(rc + point++) = (char)c;				\
  })
#else
#  define insert_char()					\
  ({							\
    if ((c & 0xC0) != 0x80)				\
      xprintf("\033[@");				\
    xputchar(c);					\
    for (i = len; i > point; i--)			\
      *(rc + i) = *(rc + i - 1);			\
    len++;						\
    *(rc + point++) = (char)c;				\
  })
#endif


#define override_char()						\
  ({								\
    size_t n = 1;						\
    char cn = (char)c;						\
    while ((*(rc + point + n) & 0xC0) == 0x80)			\
      n++;							\
    for (i = point + n; i < len; i++)				\
      *(rc + i - n) = *(rc + i);				\
    passphrase_wipe(rc + len - n, n);				\
    len -= n;							\
    n = 0;							\
    while (cn & 0x80)						\
      {								\
	cn = (char)(cn << 1);					\
	n++;							\
      }								\
    n = n ?: 1;							\
    if (len + n > size)						\
      {								\
	if ((rc = xrealloc(rc, size, size << 1L)) == NULL)	\
	  return NULL;						\
	size <<= 1L;						\
      }								\
    len += n;							\
    for (i = len - 1; i > point + n; i--)			\
      *(rc + i) = *(rc + i - n);				\
    if (len - 1 >= point + n)					\
      *(rc + point + n) = *(rc + point);			\
    for (i = 0; i < n; i++)					\
      {								\
	if (i)							\
	  c = getchar();					\
	xputchar(c);						\
	*(rc + point++) = (char)c;				\
      }								\
  })


/* Implementation of the delete-key's action upon the display */
#if defined(PASSPHRASE_TEXT)
#  define print_delete()							\
  (len ? 0 : (xprintf("\033[K%s%zn", PASSPHRASE_TEXT_EMPTY, &printed_len),	\
	      (printed_len - 3 ? xprintf("\033[%zuD", printed_len - 3) : 0)))
#else
#  define print_delete()  xprintf("\033[P")
#endif


/* Implementation of the erase-key's action upon the display */
#if defined(PASSPHRASE_TEXT)
#  define print_erase()								\
  (len ? 0 : (xprintf("\033[K%s%zn", PASSPHRASE_TEXT_EMPTY, &printed_len),	\
	      (printed_len - 3 ? xprintf("\033[%zuD", printed_len - 3) : 0)))
#elif defined(PASSPHRASE_MOVE)
#  define print_erase()  xprintf("\033[D\033[P")
#elif defined(PASSPHRASE_STAR)
#  define print_erase()  xprintf("\033[D \033[D")
#endif



#endif

