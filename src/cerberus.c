/**
 * cerberus – Minimal login program
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
#include "cerberus.h"


#ifndef USE_TTY_GROUP
#define tty_group  0
#endif


/**
 * The environment variables
 */
extern char** environ;

/**
 * Mane method
 * 
 * @param   argc  The number of command line arguments
 * @param   argv  The command line arguments
 * @return        Return code
 */
int main(int argc, char** argv)
{
  char* username = NULL;
  char* hostname = NULL;
  char* passphrase = NULL;
  char preserve_env = 0;
  char skip_auth = 0;
  #ifdef USE_TTY_GROUP
  gid_t tty_group = 0;
  struct group* group;
  #endif
  struct passwd* entry;
  
  
  /* Disable echoing */
  disable_echo();
  /* This should be done as early and quickly as possible so as little
     as possible of the passphrase gets leaked to the output if the user
     begins entering the passphrase directly after the username. */
  
  
  /* Set process group ID */
  setpgrp();
  
  
  /* Parse command line arguments */
  {
    char double_dashed = 0;
    char hostname_on_next = 0;
    int i;
    for (i = 1; i < argc; i++)
      {
	char *arg = *(argv + i);
	char c;
	
	if (*arg == 0)
	  ;
	else if ((*arg == '-') && (double_dashed == 0))
	  while ((c = *(++arg)))
	    if ((c == 'V') || (c == 'H'))
	      ;
	    else if (c == 'p')
	      preserve_env = 1;
	    else if (c == 'h')
	      {
		if (*(arg + 1))
		  hostname = arg + 1;
		else
		  hostname_on_next = 1;
		break;
	      }
	    else if (c == 'f')
	      {
		if (*(arg + 1))
		  username = arg + 1;
		skip_auth = 1;
		break;
	      }
	    else if (c == '-')
	      {
		double_dashed = 1;
		break;
	      }
	    else
	      printf("%s: unrecognised options: -%c\n", *argv, c);
	else if (hostname_on_next)
	  {
	    hostname = arg;
	    hostname_on_next = 0;
	  }
	else
	  username = arg;
      }
  }
  
  
  /* Change that a username has been specified */
  if (username == 0)
    {
      printf("%s: no username specified\n", *argv);
      sleep(ERROR_SLEEP);
      return 2;
    }
  
  
  /* Print ant we want a passphrase, if -f has not been used */
  if (skip_auth == 0)
    {
      printf("Passphrase: ");
      fflush(stdout);
    }
  /* Done early to make to program look like it is even faster than it is */
  
  
  /* Make sure nopony is spying */
  #ifdef USE_TTY_GROUP
  if ((group = getgrnam(TTY_GROUP)))
    tty_group = group->gr_gid;
  #endif
  secure_tty(tty_group);
  
  
  /* Set up clean quiting and time out */
  signal(SIGALRM, timeout_quit);
  signal(SIGQUIT, user_quit);
  signal(SIGINT, user_quit);
  siginterrupt(SIGALRM, 1);
  siginterrupt(SIGQUIT, 1);
  siginterrupt(SIGINT, 1);
  alarm(TIMEOUT_SECONDS);
  
  
  /* Get user information */
  if ((entry = getpwnam(username)) == NULL)
    {
      if (errno)
	perror("getpwnam");
      else
	printf("User does not exist\n");
      sleep(ERROR_SLEEP);
      return 1;
    }
  username = entry->pw_name;
  
  
  /* Get the passphrase, if -f has not been used */
  if (skip_auth == 0)
    {
      passphrase = get_passphrase();
      printf("\n");
    }
  
  /* Passphrase entered, turn off timeout */
  alarm(0);
  
  /* TODO verify passphrase */
  
  /* Wipe and free the passphrase from the memory */
  if (skip_auth == 0)
    {
      long i;
      for (i = 0; *(passphrase + i); i++)
	*(passphrase + i) = 0;
      free(passphrase);
    }
  
  
  /* Reset terminal settings */
  reenable_echo();
  
  
  /* TODO login */
  
  /* Change directory */
  if (chdir(entry->pw_dir))
    {
      perror("chdir");
      if (chdir(DEFAULT_HOME))
	{
	  perror("chdir");
	  sleep(ERROR_SLEEP);
	  return 1;
	}
      entry->pw_dir = DEFAULT_HOME;
    }
  
  /* Make sure the shell to use is definied */
  if ((entry->pw_shell && *(entry->pw_shell)) == 0)
    entry->pw_shell = DEFAULT_SHELL;
  
  /* Set environment variables */
  {
    char* _term = getenv("TERM");
    char* term = NULL;
    if (_term)
      {
	int n = 0, i;
	while (*(_term + n++))
	  ;
	term = malloc(n * sizeof(char));
	if (term == NULL)
	  {
	    perror("malloc");
	    sleep(ERROR_SLEEP);
	    return 1;
	  }
	for (i = 0; i < n; i++)
	  *(term + i) = *(_term + i);
      }
    
    if (preserve_env == 0)
      {
	environ = malloc(sizeof(char*));
	if (environ == NULL)
	  {
	    perror("malloc");
	    sleep(ERROR_SLEEP);
	    return 1;
	  }
	*environ = NULL;
      }
    
    setenv("HOME", entry->pw_dir, 1);
    setenv("USER", entry->pw_name, 1);
    setenv("LOGUSER", entry->pw_name, 1);
    setenv("SHELL", entry->pw_shell, 1);
    setenv("TERM", term ?: DEFAULT_TERM, 1);
    if (term)
      free(term);
    
    /* TODO set PATH */
  }
  
  
  /* Reset terminal ownership and mode */
  chown_tty(0, tty_group, 0);
  
  return 0;
}

