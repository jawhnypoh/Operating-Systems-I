/* Assignment 4: OTP
 *
 * otp_dec_d.c
 *
 * Johnny Po
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

// Definitions
#define SIZE 8192

// Function Declarations
void startServer(int socketFD, struct sockaddr_in serverAddress, int portNum);
void childProcess(int socketFD, int newSocketFD, int portNum, char encryptBuffer[], char keyBuffer[], char theBuffer[]);
void checkPoint(int encryptSize, int keySize, char encryptBuffer[], char keyBuffer[]);
char reverseEncryption(int encryptSize, char encryptBuffer[], char keyBuffer[], char theBuffer[]);


int main(int argc, char *argv[]) {
  // Initialize Variables
  int socketFD;
  int newSocketFD;
  int portNum;
  socklen_t clientLength;
  struct sockaddr_in serverAddress, clientAddress;
  char encryptBuffer[SIZE];
  char keyBuffer[SIZE];
  char theBuffer[SIZE];
  pid_t PID;

  // Check user arguments
  if(argc != 2) {
    printf("Usage: otp_enc_d listeningPort\n");
    exit(1);
  }
  // Check connection
  else if ((socketFD = socket(AF_INET, SOCK_STREAM, 0))  < 0) {
    printf("OTP_DEC_D ERROR: Could not open socket! \n");
    exit(1);
  }

  // Start port number and sockets
  portNum = atoi(argv[1]);

  // Set the server struct
  bzero((char*) &serverAddress, sizeof(serverAddress));

  // Zero IP Address memory
  memset(&serverAddress, '\0', sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNum);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Start up the server
  startServer(socketFD, serverAddress, portNum);

  // Open to listen for all connections: Don't stop until finished
  while(1) {
    clientLength = sizeof(clientAddress);
    newSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &clientLength);

    // Make sure there are no connection errors
    if(newSocketFD < 0) {
      printf("OTP_DEC_D ERROR: Could not accept connection! \n");
      continue;
    }

    // Fork
    PID = fork();

    // Check for any errors on Fork
    if(PID < 0) {
      printf("OTP_DEC_D ERROR: Could not fork! \n");
    }

    // Child processes
    if(PID == 0) {
      memset(encryptBuffer, 0, SIZE);

      childProcess(socketFD, newSocketFD, portNum, encryptBuffer, keyBuffer, theBuffer);

      exit(0);
    }
    // Otherwise, parent ends the process for new client
    else {
      close(newSocketFD);

      // Wait for the child process
      wait(NULL);
    }
  }

  // Close and clean up
  close(socketFD);

  return 0;
}

void startServer(int socketFD, struct sockaddr_in serverAddress, int portNum) {
  // Bind the socket to the port
  if(bind(socketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
    fprintf(stderr, "OTP_DEC_D ERROR: Could not bind socket to port %d\n", portNum);
    exit(1);
  }
  // Start server to listen, limit at 5
  else if (listen(socketFD, 5) < 0) {
    fprintf(stderr, "OTP_DEC_D ERROR: Connection listening on port %d! \n", portNum);
    exit(1);
  }
}

void childProcess(int socketFD, int newSocketFD, int portNum, char encryptBuffer[], char keyBuffer[], char theBuffer[]) {
  // Initialize Variables
  int i;
  int encryptSize;
  int keySize;
  int encryptChar;
  int keyChar;
  int decryptText;
  int sent;

  // Get length and contents of encrypt file
  encryptSize = read(newSocketFD, encryptBuffer, SIZE);
  if(encryptSize < 0) {
    fprintf(stderr, "OTP_DEC_D ERROR: Could not read cipherText on port %d\n", portNum);
    exit(1);
  }

  // Let client know
  sent = write(newSocketFD, "!", 1);
  if(sent < 0) {
    printf("OTP_DEC_D ERROR: Could not send to client! \n");
    exit(1);
  }

  // Clear keyBuffer
  memset(keyBuffer, 0, SIZE);

  // Get length and contents of key file
  keySize = read(newSocketFD, keyBuffer, SIZE);
  if(keySize < 0) {
    fprintf(stderr, "OTP_DEC_D ERROR: Could not read key on port %d\n", portNum);
    exit(1);
  }

  // Check characters and length
  checkPoint(encryptSize, keySize, encryptBuffer, keyBuffer);

  // Reverse the encryption of the file
  reverseEncryption(encryptSize, encryptBuffer, keyBuffer, theBuffer);

  // Send our encrypted text
  sent = write(newSocketFD, theBuffer, encryptSize);
  if(sent < encryptSize) {
    printf("OTP_DEC_D ERROR: Could not send to socket! \n");
    exit(1);
  }

  // Close and clean up
  close(newSocketFD);
  close(socketFD);
}

void checkPoint(int encryptSize, int keySize, char encryptBuffer[], char keyBuffer[]) {
  int i;

  // Make sure there are no bad characters
  for(i=0; i<encryptSize; i++) {
    if((int) encryptBuffer[i] > 90 || ((int) encryptBuffer[i] < 65 && (int) encryptBuffer[i] != 32)) {
      printf("OTP_DEC_D ERROR: cipherText contains bad characters! \n");
      exit(1);
    }
  }

  // Make sure key has no bad characters
  for(i=0; i<keySize; i++) {
    if((int) keyBuffer[i] > 90 || (int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32) {
      printf("OTP_DEC_D ERROR: Key contains bad characters! \n");
      exit(1);
    }
  }

  // Check length of the key
  if(keySize < encryptSize) {
    printf("OTP_DEC_D ERROR: The key is too short! \n");
    exit(1);
  }
}

char reverseEncryption(int encryptSize, char encryptBuffer[], char keyBuffer[], char theBuffer[]) {
  int i;
  int encryptChar;
  int keyChar;
  int decryptText;

  for(i=0; i<encryptSize; i++) {
    // Change all spaces
    if(encryptBuffer[i] == ' ') {
      encryptBuffer[i] = '@';
    }
    if(keyBuffer[i] == ' ') {
      keyBuffer[i] = '@';
    }

    // For easy ASCII manipulation
    encryptChar = (int) encryptBuffer[i];
    keyChar = (int) keyBuffer[i];

    // Change the ASCII Range
    encryptChar = encryptChar - 64;
    keyChar = keyChar - 64;

    // Update for decryption
    decryptText = encryptChar - keyChar;

    // For negative numbers
    if(decryptText < 0) {
      decryptText = decryptText + 27;
    }

    // For capital letters
    decryptText = decryptText + 64;

    // Convert back to characters
    theBuffer[i] = (char) decryptText + 0;

    // After finishing encryption, change back to spaces
    if(theBuffer[i] == '@') {
      theBuffer[i] = ' ';
    }
  }

  return *theBuffer;
}


/*************** END OF FILE ****************/
