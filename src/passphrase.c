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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define PASSPHRASE_USE_DEPRECATED
#include "passphrase.h"
#include "passphrase_helper.h"


#ifndef START_PASSPHRASE_LIMIT
# define START_PASSPHRASE_LIMIT  32
#endif
#ifndef DEFAULT_PASSPHRASE_METER
# define DEFAULT_PASSPHRASE_METER  "passcheck"
#endif



#ifdef PASSPHRASE_METER
struct passcheck_state
{
  int pipe_rw[2];
  pid_t pid;
  int flags;
};
#endif /* PASSPHRASE_METER */



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


#ifdef PASSPHRASE_METER
static void passcheck_start(struct passcheck_state* state, int flags)
{
  const char* command;
  int pipe_rw[2] = { -1, -1 };
  int exec_rw[2] = { -1, -1 };
  pid_t pid;
  ssize_t n;
  int i = 0;
  
  state->pid = -1;
  state->flags = (flags & PASSPHRASE_READ_NEW) ? (flags ^ PASSPHRASE_READ_NEW) : 0;
  if (state->flags == 0)
    return;
  
  command = getenv("LIBPASSPHRASE_METER");
  if (!command || !*command)
    command = DEFAULT_PASSPHRASE_METER;
  
  xpipe(state->pipe_rw);
  xpipe(pipe_rw);
  xpipe(exec_rw);
  /* ‘Their integer values shall be the two lowest available at the time of the pipe() call’ [man 3p pipe]
   * This guarantees (unless the application is doing something stupid) that the file desriptors
   * in exec_rw[1] is not stdin, stdout, stderr, or 0 (required by FD_CLOEXEC to take affect), assuming
   * stdin, stdout, and stderr are 0, 1, and 2, respectively, as specified in `man 3p stdin`. */
  
  if (fcntl(exec_rw[1], F_SETFD, FD_CLOEXEC) == -1)
    goto fail;
  
  pid = fork();
  if (pid == -1)
    {
      state->flags = 0;
      goto fail;
    }
  
  close(exec_rw[!!pid]), exec_rw[!!pid] = -1;
  close(state->pipe_rw[!!pid]), state->pipe_rw[!!pid] = -1;
  close(pipe_rw[!pid]), pipe_rw[!pid] = -1;
  state->pipe_rw[!!pid] = pipe_rw[!!pid], pipe_rw[!!pid] = -1;
  
  if (pid == 0)
    {
      gid_t gid = getgid(), egid = getegid();
      uid_t uid = getuid(), euid = geteuid();
      
      if ((state->pipe_rw[0] != STDIN_FILENO) && (state->pipe_rw[1] == STDIN_FILENO))
	{
	  pipe_rw[1] = dup(state->pipe_rw[1]);
	  if (pipe_rw[1] == -1)
	    goto child_fail;
	  state->pipe_rw[1] = pipe_rw[1], pipe_rw[1] = -1;
	}
      for (i = 0; i <= 1; i++)
	if (state->pipe_rw[i] != i)
	  {
	    close(i);
	    pipe_rw[i] = dup2(state->pipe_rw[i], i);
	    if (pipe_rw[i] == -1)
	      goto child_fail;
	    close(state->pipe_rw[i]), state->pipe_rw[i] = pipe_rw[i];
	  }
      
      if (egid != gid)
	if (setregid(gid, gid) && gid)
	  goto child_fail;
      if (euid != uid)
	if (setreuid(uid, uid) && uid)
	  goto child_fail;
      
      execlp(command, command, "-r", NULL);
    child_fail:
      n = write(exec_rw[1], &i, sizeof(i));
      _exit(!!n);
    }
  
 rewait:
  n = read(exec_rw[0], &i, sizeof(i));
  if ((n < 0) && (errno == EINTR))
    goto rewait;
  if (n)
    {
    rereap:
      if ((waitpid(pid, &i, 0) == -1) && (errno == EINTR))
	goto rereap;
      goto fail;
    }
  
  close(exec_rw[0]);
  state->pid = pid;
  return;
 fail:
  if (state->pipe_rw[0] >= 0)  close(state->pipe_rw[0]);
  if (state->pipe_rw[1] >= 0)  close(state->pipe_rw[1]);
  if (pipe_rw[0] >= 0)  close(pipe_rw[0]);
  if (pipe_rw[1] >= 0)  close(pipe_rw[1]);
  if (exec_rw[0] >= 0)  close(exec_rw[0]);
  if (exec_rw[1] >= 0)  close(exec_rw[1]);
  state->pid = -1;
  state->flags = 0;
}

static void passcheck_stop(struct passcheck_state* state)
{
  int _status;
  
  if (state->flags == 0)
    return;
  
  close(state->pipe_rw[0]);
  close(state->pipe_rw[1]);
  
rereap:
  if ((waitpid(state->pid, &_status, 0) == -1) && (errno == EINTR))
    goto rereap;
  
  /* TODO cleanup */
}

static void passcheck_update(struct passcheck_state* state, char* passphrase, size_t len)
{
  if (state->flags == 0)
    return;
  
  /* TODO */
}
#endif /* PASSPHRASE_METER */


static int fdgetc(int fd)
{
  unsigned char c;
  if (read(fd, &c, sizeof(c)) <= 0)
    return -1;
  return (int)c;
}


#if defined(PASSPHRASE_DEDICATED) && defined(PASSPHRASE_MOVE)
static int get_dedicated_control_key(int fdin)
{
  int c = fdgetc(fdin);
  if (c == 'O')
    {
      c = fdgetc(fdin);
      if (c == 'H')  return KEY_HOME;
      if (c == 'F')  return KEY_END;
    }
  else if (c == '[')
    {
      c = fdgetc(fdin);
      if (c == 'C')  return KEY_RIGHT;
      if (c == 'D')  return KEY_LEFT;
      if (('1' <= c) && (c <= '4') && (fdgetc(fdin) == '~'))
	return -(c - '0');
    }
  return 0;
}
#endif /* PASSPHRASE_DEDICATED && PASSPHRASE_MOVE */



#ifdef PASSPHRASE_MOVE
static int get_key(int c, int fdin)
{
# ifdef PASSPHRASE_DEDICATED
  if (c == '\033')             return get_dedicated_control_key(fdin);
# else /* PASSPHRASE_DEDICATED */
  (void) fdin;
# endif /* PASSPHRASE_DEDICATED */
  if ((c == 8) || (c == 127))  return KEY_ERASE;
  if ((c < 0) || (c >= ' '))   return c & 255;
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
char* passphrase_read2(int fdin, int flags)
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
#ifdef PASSPHRASE_METER
  struct passcheck_state passcheck;
#endif /* PASSPHRASE_METER */
  
  if (rc == NULL)
    return NULL;
  
#ifdef PASSPHRASE_METER
  passcheck_start(&passcheck, flags);
#endif /* PASSPHRASE_METER */
  
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
      c = fdgetc(fdin);
      if ((c < 0) || (c == '\n'))
	{
#ifdef PASSPHRASE_METER
	  passcheck_stop(&passcheck);
	  xflush();
#endif /* PASSPHRASE_METER */
	  break;
	}
      if (c == 0)
	continue;
      
#if defined(PASSPHRASE_MOVE)
      cc = get_key(c, fdin);
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
	  
#ifdef PASSPHRASE_METER
	  passcheck_update(&passcheck, rc, len);
#endif /* PASSPHRASE_METER */
	  
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
      
#ifdef PASSPHRASE_METER
      passcheck_update(&passcheck, rc, len);
#endif /* PASSPHRASE_METER */
      
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
  
#ifndef PASSPHRASE_METER
  (void) flags;
#endif /* !PASSPHRASE_METER */
}


/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be wiped and `free`:ed, `NULL` on error
 */
char* passphrase_read(void)
{
  return passphrase_read2(STDIN_FILENO, 0);
}

