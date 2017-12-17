/* Assignment 3: smallsh
 *
 * By Johnny Po
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


// File IO
char *inFile;
char *outFile;

// Global Variables
int permitBackground = 1;

// Signal Actions Structs
struct sigaction SIGINT_action = {0};
struct sigaction SIGTSTP_action = {0};

// Function Declarations
int changeDirectoryFunction(char**);
int exitFunction();
int statusFunction(char**, int);
void insertPID(char**, int);
void sigINT(int);
void sigTSTP(int);
void cleanUp(char**, int);
void printTheStatus(int);


int main() {
  char userLine[2048];
  char *userArgs[512];

  // Status Initialized as 0
  int status = 0;

  // To catch signals
  SIGINT_action.sa_handler = SIG_IGN; // Set to ignore
  sigaction(SIGINT, &SIGINT_action, NULL);

  SIGTSTP_action.sa_handler = SIG_IGN; // Set to ignore
  SIGTSTP_action.sa_handler = sigTSTP;
  SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  // Infinite while loop
  while(1) {
    int numArguments;
    int background = 0;

    printf(": ");
    fflush(stdout);

    // Get the user input
    if(fgets(userLine, 2048, stdin) == NULL) {
      // Reached end of file, close
      return;
    }

    /********** Parse the user input *********/
    int pos = 0;
    numArguments = 0;
    char *token;

    // To split the user command arguments into tokens by space
    token = strtok(userLine, " \n");
    while(token != NULL) {

      // Check the tokens for special characters < > &
      if(strcmp(token, "<") == 0) {
        // Get input file name
        token = strtok(NULL, " \n");

        // Copy token to inFile
        inFile = strdup(token);

        // Get the next argument
        token = strtok(NULL, " \n");
      }

      else if(strcmp(token, ">") == 0) {
        // Get input file name
        token = strtok(NULL, " \n");

        // Copy token to outFile
        outFile = strdup(token);

        // Get the next argument
        token = strtok(NULL, " \n");
      }

      else if(strcmp(token, "&") == 0) {
        // Background command: Set to show command is in the background
        if(permitBackground == 1) {
          background = 1;
        }
        // If permitBackground is not set to 1, we are in foreground-only mode
        // Make & irrelavent
        else {
          background = 0;
        }
        break;
      }

      else {
        // Store userArgs in an array
        userArgs[pos] = strdup(token);

        token = strtok(NULL, " \n");
        numArguments++;

        ++pos;
      }
    }
    userArgs[pos] = NULL;

    insertPID(userArgs, getpid());

    /********** Execute user command *********/
    if(userArgs[0] == NULL) {
      // Nothing was entered, so do nothing
    }
    else if(strcmp(userArgs[0], "cd") == 0) {
      changeDirectoryFunction(userArgs);
    }
    else if(strcmp(userArgs[0], "exit") == 0) {
      exitFunction();
    }
    else if(strcmp(userArgs[0], "status") == 0) {
      printTheStatus(status);
    }

    // Special handler for comments
    else if(!(strncmp(userArgs[0], "#", 1))) {
      // This is a comment, do nothing
    }

    // Otherwise, non built in shell functions
    else {
      int FD, i;

      // Call fork
      pid_t spawnPID = fork();

      if(spawnPID == 0) {
        // Child process
        // insertPID(userArgs, getpid());
        if(!background) {
          // If process isn't in background
          // Can be interrupted with Ctrl-C
          SIGINT_action.sa_handler = SIG_DFL; // Set to default
          SIGINT_action.sa_flags = 0;
          sigfillset(&SIGINT_action.sa_mask);
          sigaction(SIGINT, &SIGINT_action, NULL);
        }
        if(inFile != NULL) {
          // If input file exists
          // Open with readonly permissions
          FD = open(inFile, O_RDONLY);

          if(FD == -1) {
            // File is invalid or doesn't exist, exit
            fprintf(stderr, "File is invalid or doesn't exist: %s\n", inFile);
            exit(1);
          }
          else if(dup2(FD, 0) == -1) { // Redirect the input
            fprintf(stderr, "DUP2 error\n");
            exit(1);
          }
          // After finished, close the file
          close(FD);
        }

        /* Otherwise, if process is in the background, redirect the
           input to /dev/null if the input file is not specified */
        else if(background) {

          FD = open("/dev/null", O_RDONLY);

          if(FD == -1) {
            // Could not open file
            fprintf(stderr, "Could not open.\n");
            exit(1);
          }
          else if(dup2(FD, 0) == -1) {
            fprintf(stderr, "DUP2 error\n");
            exit(1);
          }
          // After finished, close the file
          close(FD);
        }

        // If output file exists
        else if(outFile != NULL) {
          // Open the file
          FD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

          if(FD == -1) {
            // Could not open file
            fprintf(stderr, "Could not open file %s\n", outFile);
            fflush(stdout);
            exit(1);
          }

          if(dup2(FD, 1) == -1) {
            // Error redirecting output
            fprintf(stderr, "DUP2 error\n");
            exit(1);
          }
          // After finished, close the file
          close(FD);
        }

        // Execute the command in userArguments[0]
        if(execvp(userArgs[0], userArgs)) {
          // Command not recognized
          fprintf(stderr, "Command not found: %s\n", userArgs[0]);
          fflush(stdout);
          exit(1);
        }
      }

      else if(spawnPID < 0) {
        // There was a fork() error
        fprintf(stderr, "There was an error with fork()\n");
        status = 1;
      }

      else {
        // The parent
        if(!background) {
          // If process isn't in the background, wait for foreground process to finish
          waitpid(spawnPID, &status, 0);
        }
        else {
          // If process is in background, print its PID
          printf("Background PID: %d\n", spawnPID);
        }
      }

      cleanUp(userArgs, numArguments);

      // Check to see if background processes have completed
      spawnPID = waitpid(-1, &status, WNOHANG);
      while(spawnPID > 0) {
        // Show that process is completed
        printf("Background PID %d is completed. \n", spawnPID);

        // Print the status
        printTheStatus(status);

        spawnPID = waitpid(-1, &status, WNOHANG);
      }

    }
  }

  return 0;
}

/******************** BUILT IN FUNCTIONS **************************/
int changeDirectoryFunction(char **userArgs) {
  // If user just enters 'cd', go to HOME
  if(userArgs[1] == NULL) {
    // Get home value
    char *home = getenv("HOME");
    // If exists, change to home
    if(home != NULL) {
      chdir(home);
    }
  }

  // Else, change to the user specified directory
  else {
    char cwd[256];
    char newDirectory[256];
    getcwd(cwd, sizeof(cwd));

    if(strcmp(userArgs[1], "..") != 0) {
      sprintf(newDirectory, "%s/%s", cwd, userArgs[1]);
    }
    else {
      strcpy(newDirectory, "..");
    }
    if(chdir(newDirectory) != 0) {
      printf("cd: No such file or directory. \n");
    }
  }
  return 0;
}

int exitFunction() {
  exit(0);
}


/******************** HELPER FUNCTIONS **************************/
void insertPID(char **userArgs, pid_t spawnPID) {
  // If "$$", change to PID
  char *pch;
  int i = 0;
  int j = 0;
  int argumentLength = 0;
  while(userArgs[i] != NULL) {
    pch = strstr(userArgs[i], "$$");
    if(pch != NULL) {
      argumentLength = strlen(userArgs[i]);
      char *buffer = malloc(sizeof(char) + (argumentLength + 5));
      char *tmp1 = malloc(sizeof(char) + (argumentLength + 5));
      char *tmp2 = malloc(sizeof(char) + (argumentLength + 5));

      while(userArgs[i] + sizeof(char) * j != pch) {
        tmp1[j] = userArgs[i][j];
        j++;
      }
      tmp1[j] = '\0';
      j += 2;
      int loc = j;

      while(userArgs[i][j] != '\0') {
        tmp2[j - loc] = userArgs[i][j];
        j++;
      }
      tmp2[j - loc] = '\0';

      sprintf(buffer, "%s%d%s", tmp1, (int) spawnPID, tmp2);
      free(tmp1);
      free(tmp2);
      free(userArgs[i]);
      userArgs[i] = buffer;
    }
    i++;
  }
}

void sigINT(int signo) {
  // This function checks for Ctrl-C
  printf("\nSignal Caught, sleeping for 5 seconds! %d\n", signo);
  //sleep(5);
  SIGINT_action.sa_handler = SIG_IGN; // Set to ignore
  sigaction(SIGINT, &SIGINT_action, NULL);
}

void sigTSTP(int signo) {
  // This function checks for Ctrl-Z
  char *zMessage = "\nSIGTSTP Detected! ";
  int zLen = strlen(zMessage);
  write(STDOUT_FILENO, zMessage, zLen);

  if(signo == SIGTSTP) {
    if(permitBackground == 1) {
      char *message = "Entering foreground only mode (& is ignored)...\n";
      int len = strlen(message);
      write(STDOUT_FILENO, message, len);
      permitBackground = 0; // This prevents background processes from being created
    }
    else if(permitBackground == 0) {
      char *message = "Exiting foreground only mode...\n";
      int len = strlen(message);
      write(STDOUT_FILENO, message, len);
      permitBackground = 1; // This allows background processes to be created
    }
  }
}

void cleanUp(char **userArgs, int numArguments) {
  // Empty the array of userArgs
  int i;

  for(i=0; i<numArguments; i++) {
    userArgs[i] = NULL;
  }

  // Files will be set to NULL for use again
  inFile = NULL;
  outFile = NULL;
  free(inFile);
  free(outFile);
}

void printTheStatus(int status) {
  // If the process has successfully finished
  if(WIFEXITED(status)) {
    printf("Exit Status: %i\n", WEXITSTATUS(status));
  }
  // If the process was terminated by a signal
  else {
    printf("Background process interrupted, terminated by signal %i\n", status);
  }
}

/************* END OF FILE ************/
