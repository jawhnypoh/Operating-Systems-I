/* Assignment 2: Adventure
   By Johnny Po
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Booleans
typedef int bool;
enum { false, true };

// Global Variables
int numRooms = 7;
int minimumConnections = 3;

// Structs
struct Room {
  char* theName;
  struct Room** roomConnections;
  int roomTotalConnections;
  int roomMaxConnections;
  int roomType; /* 1: START_ROOM    2: MID_ROOM     3: END_ROOM */
};

// Function Declarations
void createFiles();
struct Room * makeRooms();
void createRoomNames(struct Room*);
struct Room * setRoomTypes(struct Room*);
struct Room * createRoomConnections(struct Room*);
void addRoomConnections(struct Room*, int, int);
void addMoreConnections(struct Room*, int);
void freeRooms(struct Room*);

void main(){
  srand(time(NULL));

  createFiles();
  struct Room *listofRooms = makeRooms();
  freeRooms(listofRooms);
}

 /************************************************************
  * Function createFiles()
  * Creates the room files and the directory to store
  * the rooms in
  * Param: None
  ************************************************************/
void createFiles() {
  struct Room *listofRooms = makeRooms(7);

  int i, j;
  int pid = getpid();
  char myDirectory[60];
  char readBuffer[50];
  char myRoomType[12];
  sprintf(myDirectory, "poj.rooms.%d", pid);
  mkdir(myDirectory, 0777);

  FILE *myFile;

  // Create room info for each room
  for(i=0; i<numRooms; i++) {
    // Open file and add the name

    // Clear out the array before using it
    memset(readBuffer, '\0', sizeof(myDirectory));
    sprintf(readBuffer, "./%s/%s", myDirectory, listofRooms[i].theName);
    myFile = fopen(readBuffer, "w");
    if(myFile == NULL) {
      printf("fopen failed due to %s\n", myFile);
      printf("readBuffer: %s\n", readBuffer);
    }

    fprintf(myFile, "ROOM NAME: %s\n", listofRooms[i].theName);

    // Add connections
    for(j=0; j<listofRooms[i].roomTotalConnections; j++) {
      fprintf(myFile, "CONNECTION %d: %s\n", j+1, listofRooms[i].roomConnections[j]->theName);
    }

    // Switch case for choosing room type
    switch (listofRooms[i].roomType) {
      case 1:
        strcpy(myRoomType, "START_ROOM");
        break;
      case 2:
        strcpy(myRoomType, "MID_ROOM");
        break;
      case 3:
        strcpy(myRoomType, "END_ROOM");
        break;
    }

    // Add the room type and close the file
    fprintf(myFile, "ROOM TYPE: %s\n", myRoomType);
    fclose(myFile);
  }
}

/************************************************************
 * Function makeRooms()
 * Creates the array of 7 rooms and set the room infos
 * Param: numRooms
 ************************************************************/
struct Room * makeRooms() {
  struct Room *listofRooms = malloc(sizeof(struct Room) * numRooms);

  createRoomNames(listofRooms);
  setRoomTypes(listofRooms);
  createRoomConnections(listofRooms);

  return listofRooms;
}

/************************************************************
 * Function createRoomNames()
 * Creates hardcoded names for the rooms
 * Param: listofRooms, numRooms
 ************************************************************/
void createRoomNames(struct Room* listofRooms) {
  // Create 10 total roomNames
  int i, j, size;
  int k = 0;
  int randomNum = 0;
  int tempNum;

  char** roomNames = malloc(sizeof(char*) * 10);
  int* order = malloc(sizeof(int*) * numRooms);
  for(i=0; i<numRooms; i++) {
    order[i] = -1;
  }

  // Allocate memory for each element of roomNames
  for(i=0; i<10; i++) {
    roomNames[i] = malloc(sizeof(char*) * 20);
  }

  // Hardcode name into each element of array
  strcpy(roomNames[0], "Memorial Union");
  strcpy(roomNames[1], "LINC");
  strcpy(roomNames[2], "Valley Library");
  strcpy(roomNames[3], "Wiegand Hall");
  strcpy(roomNames[4], "KEC");
  strcpy(roomNames[5], "Kidder Hall");
  strcpy(roomNames[6], "Gilbert Hall");
  strcpy(roomNames[7], "Austin Hall");
  strcpy(roomNames[8], "Waldo Hall");
  strcpy(roomNames[9], "Johnson Hall");

  // Randomly assign roomNames to listofRooms.theName
  bool chosen[10];
  for(i=0; i<10; i++) {
    chosen[i] = false;
  }

  for(k=0; k<numRooms; k++) {
    randomNum = rand() % 10 + 1;

    while(chosen[randomNum]) {
      randomNum = rand() % 10 + 1;
    }
    chosen[randomNum] = true;

    order[k] = randomNum;
    listofRooms[k].theName = malloc(strlen(roomNames[randomNum] + 1));
    strcpy(listofRooms[k].theName, roomNames[randomNum]);
  }

  // Free the memory allocated for roomNames
  for(i=0; i<10; i++) {
    free(roomNames[i]);
  }
  free(roomNames);
}

/************************************************************
 * Function setRoomTypes()
 * Sets the type for each room
 * Param: listofRooms, numRooms
 ************************************************************/
struct Room * setRoomTypes(struct Room *listofRooms) {
  int i;
  int randomNum1, randomNum2;

  randomNum1 = rand() % numRooms;
  randomNum2 = rand() % numRooms;

  // First, set all the room types to MID_ROOM
  for(i=0; i<numRooms; i++) {
    listofRooms[i].roomType = 2;

    // Randomly assign a room to START_ROOM
    if(listofRooms[randomNum1].roomType == 2) {
      listofRooms[randomNum1].roomType = 1;
    }

    // Randomly assign a room to END_ROOM
    else if(listofRooms[randomNum2].roomType == 2) {
      listofRooms[randomNum2].roomType = 3;
    }
  }

  return listofRooms;
}

/************************************************************
 * Function createRoomConnections()
 * Creates the connections for each room
 * Params: listofRooms, numRooms
 ************************************************************/
struct Room * createRoomConnections(struct Room *listofRooms) {
  // // Create connections for each room
  int i, j;
  for(i=0; i<numRooms; i++) {
    // Malloc for the room connections
    listofRooms[i].roomConnections = malloc(sizeof(struct Room) * 3);
    listofRooms[i].roomTotalConnections = 0;
    listofRooms[i].roomMaxConnections = 3;

    // For now, fill each Rooms connections with null values
    for(j=0; j<3; j++) {
      listofRooms[i].roomConnections[j] = NULL;
    }
  }

  for(i=0; i<numRooms; i++) {
    while(listofRooms[i].roomTotalConnections < numRooms - 1) {

      int j, randomRoom, connectionExists = 0;
      // Generate a random room name, and exit loop only when the
      // name doesn't exist in the current connections
      do {
        connectionExists = 0;
        randomRoom = rand() % numRooms;
        for(j = 0; j < listofRooms[i].roomTotalConnections; j++) {
          if(strcmp(listofRooms[i].roomConnections[j]->theName, listofRooms[randomRoom].theName) == 0) {
            connectionExists = 1;
          }
        }
      } while(connectionExists);
      // If the room generated is not itself
      if(i != randomRoom){
        addRoomConnections(listofRooms, i, randomRoom);
      }
      // If the room has more than the minimum number of connections, exit loop
      if(listofRooms[i].roomTotalConnections >= 3) break;
    }
  }
  return listofRooms;
}

/************************************************************
 * Function addRoomConnections()
 * Adds the connections for each room
 * Params: listofRooms, idx1, idx2
 ************************************************************/
 void addRoomConnections(struct Room *listofRooms, int Idx1, int Idx2) {
   // If necessary, add more connections to roomMaxConnections
   if(listofRooms[Idx1].roomTotalConnections == listofRooms[Idx1].roomMaxConnections) {
     addMoreConnections(listofRooms, Idx1);
   }
   if(listofRooms[Idx2].roomTotalConnections == listofRooms[Idx2].roomMaxConnections) {
     addMoreConnections(listofRooms, Idx2);
   }

   // Connect the rooms
   listofRooms[Idx1].roomConnections[listofRooms[Idx1].roomTotalConnections] = &(listofRooms[Idx2]);
   listofRooms[Idx2].roomConnections[listofRooms[Idx2].roomTotalConnections] = &(listofRooms[Idx1]);
   listofRooms[Idx1].roomTotalConnections++;
   listofRooms[Idx2].roomTotalConnections++;
 }

 /************************************************************
  * Function addMoreConnections()
  * If necessary, add more connection possibilities into rooms
  * Params: listofRooms, Index
  ************************************************************/
 void addMoreConnections(struct Room *listofRooms, int Index) {
   int i;
   struct Room **temporaryRoom = malloc(sizeof(struct Room*) * listofRooms[Index].roomMaxConnections * 2);
   for(i=0; i<listofRooms[Index].roomMaxConnections; i++) {
     temporaryRoom[i] = listofRooms[Index].roomConnections[i];
   }
   free(listofRooms[Index].roomConnections);
   listofRooms[Index].roomConnections = temporaryRoom;
   listofRooms[Index].roomMaxConnections *= 2;
 }

 /************************************************************
  * Function freeRooms()
  * Free everything when done
  * Params: listofRooms
  ************************************************************/
 void freeRooms(struct Room *listofRooms) {
   int i;
   for(i=0; i<numRooms; i++) {
     free(listofRooms[i].theName);
     free(listofRooms[i].roomConnections);
   }
   free(listofRooms);
 }
