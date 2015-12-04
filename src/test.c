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
#include "test.h"


/**
 * Main test function
 * 
 * @param   argc  Number of elements in `argv`
 * @param   argv  Command line arguments
 * @return        Zero on success
 */
int main(int argc, char** argv)
{
  /* Variables for the passphrase */
  char* passphrase;
  char* passphrase_;
  
  /* Hide the passphrase */
  passphrase_disable_echo1(0);
  
  /* Do things needed before reading the passphrase */
  printf("Passphrase: ");
  fflush(stdout);
  
  /* Read the passphrase */
  passphrase = passphrase_read2(0, 0);
  if (passphrase == NULL)
    {
      /* Something went wrong, print what and exit */
      perror(*argv);
      return 1;
    }
  
  /* Use the passphrase */
  printf("You entered: %s\n", passphrase);
  
  /* Wipe and free the passphrase */
  passphrase_ = passphrase;
  while (*passphrase)
    *passphrase++ = 0;
  free(passphrase_);
  
  /* Stop hiding user input */
  passphrase_reenable_echo1(0);
  
  /* End of program */
  return 0;
  
  /* `argc` was never used */
  (void) argc;
}

