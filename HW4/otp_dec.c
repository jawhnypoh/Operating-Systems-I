/* Assignment 4: OTP
 *
 * otp_dec.c
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
void printResults(int encryptSize, char encryptBuffer[]);

int main(int argc, char *argv[]) {
  // Initialize Variables
  int i;
  int socketFD;
  int portNum;
  int inputFP;
  int encryptSize;
  int keySize;
  struct sockaddr_in serverAddress;
  struct hostent *server;
  char encryptBuffer[SIZE];
  char keyBuffer[SIZE];
  char theBuffer[1];
  int sent;
  int received;

  // Verify that user input is actuall 4
  if(argc != 4) {
    printf("Usage: otp_dec cipherText key port \n");
    exit(1);
  }

  portNum = atoi(argv[3]);

  // Initiate encrypted file
  inputFP = open(argv[1], O_RDONLY);

  if(inputFP < 0) {
    fprintf(stderr, "ERROR: Cannot open file %s\n", argv[1]);
    exit(1);
  }

  // Get contents and read the length of file
  encryptSize = read(inputFP, encryptBuffer, SIZE);

  // Check to see if file has bad characters
  for(i=0; i<encryptSize - 1; i++) {
    if((int) encryptBuffer[i] > 90 || (int) encryptBuffer[i] < 65 && (int) encryptBuffer[i] != 32) {
      printf("ERROR: File contains bad characters! \n");
      exit(1);
    }
  }

  close(inputFP);


  // Initiate key file
  inputFP = open(argv[2], O_RDONLY);

  if(inputFP < 0) {
    fprintf(stderr, "ERROR: Cannot open key file %s\n", argv[2]);
    exit(1);
  }

  // Get contents and read the length of the file
  keySize = read(inputFP, keyBuffer, SIZE);

  // Check to see if file has bad characters;
  for(i=0; i<keySize - 1; i++) {
    if((int) keyBuffer[i] > 90 || (int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32) {
      printf("ERROR: File contains bad characters! \n");
      exit(1);
    }
  }

  close(inputFP);

  // Key size must be greater than Encrypted Input size
  if(encryptSize > keySize) {
    fprintf(stderr, "ERROR: Key '%s' is shorter than encrypted input! \n", argv[2]);
  }

  // Start the Socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if(socketFD < 0) {
    fprintf(stderr, "ERROR: Could not get otp_dec_d on port %d\n", portNum);
    exit(1);
  }

  memset(&serverAddress, '\0', sizeof(serverAddress));

  server = gethostbyname("localhost");
  if(server == NULL) {
    printf("ERROR: Could not connect to otp_dec_d\n");
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

  // Send file to server for decryption
  sent = write(socketFD, encryptBuffer, encryptSize - 1);
  if(sent < encryptSize - 1) {
    fprintf(stderr, "ERROR: Could not send cipherText to otp_dec_d on port %d\n", portNum);
    exit(1);
  }

  memset(theBuffer, 0, 1);

  // Did the server get the message? Confirm
  received = read(socketFD, theBuffer, 1);
  if(received < 0) {
    printf("ERROR: No response from server! \n");
    exit(1);
  }

  // Send key file to server for decryption
  sent = write(socketFD, keyBuffer, keySize - 1);
  if(sent < keySize - 1) {
    fprintf(stderr, "ERROR: Could not send key file to otp_dec_d on port %d\n", portNum);
    exit(1);
  }

  memset(encryptBuffer, 0, SIZE);

  do {
    // Read the contents
    received = read(socketFD, encryptBuffer, encryptSize - 1);
  }
  while(received > 0);

  if(received < 0) {
    printf("ERROR: Could not recieved ciphertext from otp_dec_d\n");
    exit(1);
  }

  printResults(encryptSize, encryptBuffer);

  close(socketFD);

  return 0;
}

void printResults(int encryptSize, char encryptBuffer[]) {
  int i;
  for(i=0; i<encryptSize - 1; i++) {
    printf("%c", encryptBuffer[i]);
  }
  printf("\n");
}

/*************** END OF FILE ****************/
