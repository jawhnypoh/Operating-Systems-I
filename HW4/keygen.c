/* Assignment 4: OTP
 *
 * keygen.c
 *
 * Johnny Po
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void createKey(int argc, char *argv[]);
int randomInteger(int minimum, int maximum);

int main(int argc, char *argv[]) {
  // Make sure number of arguments is correct
  if(argc < 2) {
    printf("Usage: keygen length \n");
    exit(1);
  }

    // Generate the key
    createKey(argc, argv);

  return 0;
}

void createKey(int argc, char *argv[]) {
  // Initialize Variables
  time_t theTime;
  int theLength;
  int i;
  char randomLetter;

  // Random seed
  srand((unsigned) time(&theTime));

  theLength = atoi(argv[1]);

  if(theLength < 1) {
    printf("ERROR: Key length not long enough. \n");
    exit(1);
  }

  // Assign random letters
  for(i=0; i<theLength; i++) {
    randomLetter = (char) randomInteger(64, 90);

    // Account for special cases
    if(randomLetter == '@') {
      randomLetter = ' ';
    }

    // Print out
    printf("%c", randomLetter);
  }
  printf("\n");
}

int randomInteger(int minimum, int maximum) {
  int randomVariable;

  randomVariable = rand()%(maximum - minimum + 1) + minimum;

  return randomVariable;
}
