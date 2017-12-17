/* Assignment 4 - OTP
 *
 * otp_endc.c
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

// Function Declarations
void printResults(int inputSize, char inputBuffer[]);

// Definitions
#define SIZE 8192


int main(int argc, char *argv[]) {
  // Initialize variables
  int i;
  int socketFD;
  int portNum;
  int inputSize;
  int keySize;
  int sent;
  int received;
  char inputBuffer[SIZE];
  char keyBuffer[SIZE];
  char theBuffer[1];

  // Socket structs
  struct sockaddr_in serverAddress;
  struct hostent *server;

  // Verify that user input is actually 4
  if(argc != 4) {
    printf("Usage: otp_enc plaintext key port \n");
    exit(1);
  }

  portNum = atoi(argv[3]);

  // Initiate file for input
  int inputFP = open(argv[1], O_RDONLY);

  if(inputFP < 0) {
    fprintf(stderr, "ERROR: Cannot open plaintext file! %s\n", argv[1]);
    exit(1);
  }

  // Get size and contents of input
  inputSize = read(inputFP, inputBuffer, SIZE);

  // Make sure there's no bad characters
  for(i=0; i<inputSize - 1; i++) {
    if((int) inputBuffer[i] > 90 || (int) inputBuffer[i] < 65 && (int) inputBuffer[i] != 32) {
      printf("ERROR: Plaintext contains bad characters! \n");
      exit(1);
    }
  }

  close(inputFP);



  // Initiate file for key
  int keyFP = open(argv[2], O_RDONLY);

  if(keyFP < 0) {
    fprintf(stderr, "ERROR: Cannot open key file! %s\n", argv[2]);
    exit(1);
  }

  // Get size and contents of key
  keySize = read(keyFP, keyBuffer, SIZE);


  // Make sure there are no bad characters
  for(i=0; i<keySize - 1; i++) {
    if((int) keyBuffer[i] > 90 || (int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32) {
      printf("ERROR: Plaintext contains bad characters! \n");
      exit(1);
    }
  }

  close(keyFP);


  // Key size must be greater than Input size
  if(inputSize > keySize) {
    fprintf(stderr, "ERROR: Key '%s' is shorter than input! \n", argv[2]);
  }


  // Start the Socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if(socketFD < 0) {
    fprintf(stderr, "ERROR: Could not get otp_enc_d on port %d\n", portNum);
    exit(1);
  }

  memset(&serverAddress, '\0', sizeof(serverAddress));

  server = gethostbyname("localhost");
  if(server == NULL) {
    printf("ERROR: Could not connect to otp_enc_d! \n");
    exit(1);
  }


  // Housekeeping procedures
  bzero((char *)&serverAddress, sizeof(serverAddress));
  bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNum);

  // Try to connect, catch error if failed
  if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    fprintf(stderr, "ERROR: Could not connect to server on port %d\n", portNum);
    exit(1);
  }

  // Send input file to server
  sent = write(socketFD, inputBuffer, inputSize - 1);
  if(sent < inputSize - 1) {
    fprintf(stderr, "ERROR: Could not send the plaintext to otp_enc_d on port %d\n", portNum);
    exit(1);
  }

  memset(theBuffer, 0, 1);

  // Did the server get the message? Confirm
  received = read(socketFD, theBuffer, 1);
  if(received < 0) {
    printf("ERROR: No response from server! \n");
    exit(1);
  }

  // Send key file to server
  sent = write(socketFD, keyBuffer, keySize - 1);
  if(sent < keySize - 1) {
    fprintf(stderr, "ERROR: Could not send key to otp_enc_d on port %d\n", portNum);
    exit(1);
  }

  memset(inputBuffer, 0, 1);

  do {
    // Text should be received back as encrypted
    received = recv(socketFD, inputBuffer, inputSize - 1, 0);
  }
  while(received > 0);

  if(received < 0) {
    printf("ERROR: Could not recieved ciphertext from otp_enc_d\n");
    exit(1);
  }

  printResults(inputSize, inputBuffer);


  close(socketFD);

  return 0;
}

void printResults(int inputSize, char inputBuffer[]) {
  int i;
  for(i=0; i<inputSize - 1; i++) {
    printf("%c", inputBuffer[i]);
  }
  printf("\n");
}

/*************** END OF FILE ****************/
