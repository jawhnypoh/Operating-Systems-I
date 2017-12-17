#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>

// Booleans
typedef int bool;
enum { false, true };

// Global Variables
int numRooms = 7;

// Structs
struct Room {
  char *theName;
  struct Room **roomConnections;
  char **connectionNames;
  int roomTotalConnections;
  int roomMaxConnections;
  int roomType; /* 1: START_ROOM    2: MID_ROOM     3: END_ROOM */
};

// Function Declarations
char* createGame();
char* getDirectoryName();
void getRoomNames(struct Room*, char*);
void getRoomFeatures(struct Room*, char*, char*, int);
void addToRoom(struct Room*, char*, int);
int getRoomType(struct Room*, char*, int);
struct Room* getConnections(struct Room*, char*, int);
char* getNameFromConnection(char*);
void connectAllRooms(struct Room*);
void theGame(struct Room*);
struct Room **expandRecordSize(struct Room**, int);
struct Room *moveAround(struct Room*, struct Room*);
void printWin(struct Room**, int);
void printTheTime();


char* getDirectoryName() {
  DIR *myDirectory = opendir(".");
  assert(myDirectory != NULL);

  struct dirent *directoryEntry;
  struct stat fileStat;

  char *newestName;
  char *pathName;
  char *directoryPathName = malloc(sizeof(char) * 25);

  int latest;

  while((directoryEntry = readdir(myDirectory)) != NULL ) {
    // Only use directories with poj.rooms
    if(strncmp(directoryEntry->d_name, "poj.rooms.", 9) == 0) {
      pathName = malloc(sizeof(char) * 25);

      // Copy name into pathName
      strcpy(pathName, directoryEntry->d_name);
      stat(pathName, &fileStat);
      // If first time, assign
      if(latest == 0) {
        latest = fileStat.st_mtime;
        newestName = malloc(sizeof(char) * 25);
        strcpy(newestName, pathName);
      }
      else {
        // If there's a newer directory, assign that name instead
        if(latest < fileStat.st_mtime) {
          latest = fileStat.st_mtime;
          newestName = malloc(sizeof(char) * 25);
          strcpy(newestName, pathName);
        }
      }
    }
  }

  sprintf(directoryPathName, "./%s", newestName);

  return directoryPathName;
}

void getRoomNames(struct Room *gameRooms, char *directoryPathName) {
  DIR *myDirectory = opendir(directoryPathName);
  char *roomName;

  struct dirent *directoryEntry;
  int numRooms = 0;

  // Read files from the directory
  while((directoryEntry = readdir(myDirectory)) != NULL ) {
    // Skip "." and ".."
    if(strcmp(directoryEntry->d_name, ".") != 0 && strcmp(directoryEntry->d_name, "..") != 0 ) {
      // Copy roomName to struct->roomName
      roomName = malloc(sizeof(char) * 15);
      strcpy(roomName, directoryEntry->d_name);
      gameRooms[numRooms].theName = roomName;

      // Get the room features and put them into the room
      getRoomFeatures(gameRooms, directoryPathName, roomName, numRooms);

      numRooms++;
    }
  }
  closedir(myDirectory);
  connectAllRooms(gameRooms);
}

void getRoomFeatures(struct Room *gameRooms, char *directoryPathName, char *roomName, int index) {
  char *pathToFile = malloc(sizeof(char) * 50);
  char *line = malloc(sizeof(char) * 50);
  int i;

  // Join directoryPathName and roomName to get pathToFile
  sprintf(pathToFile, "%s/%s", directoryPathName, roomName);
  FILE *myFile = fopen(pathToFile, "r");
  if(myFile == NULL) {
    printf("ERROR: fopen failed due to %s\n", myFile);
  }

  while(fgets(line, 50, myFile)) {
    // Don't need to add ROOM NAME again, so if find, skip it
    if(strncmp(line, "ROOM NAME", 9) == 0){
      continue;
    }

    // If line is at ROOM TYPE, check if:
    // 1. START_ROOM
    // 2. END_ROOM
    else if(strncmp(line, "ROOM TYPE", 9) == 0) {
      getRoomType(gameRooms, line, index);
    }

    else {
      getConnections(gameRooms, line, index);
    }
  }

  free(line);
  fclose(myFile);
}

int getRoomType(struct Room *gameRooms, char *line, int index) {
  if(line[11] == 'S') {
    gameRooms[index].roomType = 1;
  }
  if(line[11] == 'E') {
    gameRooms[index].roomType = 3;
  }

  return gameRooms[index].roomType;
}

struct Room *getConnections(struct Room *gameRooms, char *line, int index) {
  int i;
  // If array space allocated is too small, reallocate and expand
  if(gameRooms[index].roomTotalConnections == gameRooms[index].roomMaxConnections) {
    char **tempConnections = malloc(sizeof(char* ) * gameRooms[index].roomMaxConnections * 2);
    for(i=0; i<gameRooms[index].roomTotalConnections; i++) {
      tempConnections[i] = gameRooms[index].connectionNames[i];
    }
    free(gameRooms[index].connectionNames);
    gameRooms[index].connectionNames = tempConnections;
    gameRooms[index].roomMaxConnections *= 2;
  }

  // Add string to our connection
  gameRooms[index].connectionNames[gameRooms[index].roomTotalConnections] = getNameFromConnection(line);
  gameRooms[index].roomTotalConnections++;

  return gameRooms;
}

char* getNameFromConnection(char *line) {
  int i;
  // Get the name from the Connections
  char *connectedRoom = malloc(sizeof(char) * 15);

  // Get all room names after "CONNECTION 1: "
  for(i=14; i<strlen(line) - 1; i++) {
    connectedRoom[i - 14] = line[i];
  }
  // Null terminator
  connectedRoom[i - 14] = '\0';

  return connectedRoom;
}

struct Room* createRooms() {
  int i;

  struct Room* gameRooms = malloc(sizeof(struct Room) * numRooms);
  for(i=0; i<numRooms; i++) {
    gameRooms[i].roomTotalConnections = 0;
    gameRooms[i].roomMaxConnections = 3;
    gameRooms[i].roomType = 2;

    // Set up string pointers
    gameRooms[i].connectionNames = malloc(sizeof(char*) * gameRooms[i].roomMaxConnections);
  }

  return gameRooms;
}

void connectAllRooms(struct Room* gameRooms) {
  // Connect all the rooms together
  int i=0;
  int j, k;
  while(i<numRooms) {
    gameRooms[i].roomConnections = malloc(sizeof(struct Room*) * gameRooms[i].roomTotalConnections);
    for(j=0; j<gameRooms[i].roomTotalConnections; j++) {
      for(k=0; k<numRooms; k++) {
        // If any rooms name is the same as name in connectionNames, connect the rooms
        if(strcmp(gameRooms[i].connectionNames[j], gameRooms[k].theName) == 0) {
          gameRooms[i].roomConnections[j] = &(gameRooms[k]);
        }
      }
    }
    i++;
  }
}

void theGame(struct Room* gameRooms) {
  int i;
  int numSteps = 0;
  int recordSize = 5;

  struct Room *currentRoom;

  // Array of room structs to record the steps the user took
  struct Room **stepRooms = malloc(sizeof(struct Room*) * recordSize);

  // Look for the first room
  for(i=0; i<numRooms; i++) {
    if(gameRooms[i].roomType == 1) {
      currentRoom = &(gameRooms[i]);
      break;
    }
  }

  // Continue the game until user finds the end room
  while(currentRoom->roomType != 3) {
    currentRoom = moveAround(gameRooms, currentRoom);
    //Increment numSteps when user makes another move
    if(currentRoom != stepRooms[numSteps - 1]) {
      if(numSteps == recordSize) {
        stepRooms = expandRecordSize(stepRooms, recordSize);
      }
      stepRooms[numSteps] = currentRoom;
      numSteps++;
    }
  }

  // Once the user reaches the end room, print the result
  printWin(stepRooms, numSteps);
}

struct Room **expandRecordSize(struct Room **stepRooms, int recordSize) {
  int i;

  struct Room **temporaryRooms = malloc(sizeof(struct Room*) * recordSize * 2);
  for(i=0; i<recordSize; i++) {
    temporaryRooms[i] = stepRooms[i];
  }
  free(stepRooms);
  stepRooms = temporaryRooms;
  recordSize *= 2;

  return stepRooms;
}

struct Room* moveAround(struct Room *gameRooms, struct Room *currentRoom) {
  int i;
  int roomExists = 0;
  char userInput[15];

  // Tell the user where we are
  printf("CURRENT LOCATION: %s\n", currentRoom->theName);
  printf("POSSIBLE CONNECTIONS: ");
  for(i=0; i<currentRoom->roomTotalConnections; i++) {
    printf("%s", currentRoom->roomConnections[i]->theName);
    if(i == currentRoom->roomTotalConnections - 1) {
      printf(" ");
    }
    else {
      printf(", ");
    }
  }

  // Ask the user where they want to go
  printf("\n");
  printf("WHERE TO? > ");
  fgets(userInput, 15, stdin);
  printf("\n");

  int rmNewLine = strlen(userInput);
  if(rmNewLine > 1 && userInput[rmNewLine - 1] == '\n') {
    userInput[rmNewLine - 1] = '\0';
  }

  // TODO: If user types in "time", run that function
  if(strncmp(userInput, "time", 4) == 0) {
    printTheTime();

    return currentRoom;
  }

  // If user inputs a room name that exists and can be moved to, move to that room
  for(i=0; i<currentRoom->roomTotalConnections; i++) {
    if(strcmp(userInput, currentRoom->roomConnections[i]->theName) == 0) {
      roomExists = 1;
      currentRoom = currentRoom->roomConnections[i];
      break;
    }
  }

  // If room doesn't exist or userInput doesn't make sense
  if(!roomExists) {
    printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN. \n\n");
  }

  return currentRoom;
}

void printWin(struct Room **stepRooms, int numRooms) {
  int i;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS! \n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS: \n", numRooms);
  for(i=0; i<numRooms; i++) {
    printf("%s\n", stepRooms[i]->theName);
  }
}

void printTheTime() {
  int i;
  FILE *timeFile;
  time_t rawTime;
  struct tm* timeInfo;
  char timeBuffer[80];
  time(&rawTime);
  timeInfo = localtime(&rawTime);

  // The format of time output
  strftime(timeBuffer, 80, "%l:%M%P, %A, %B %d, %Y\n", timeInfo);


  // Take out the space at the beginning
  for(i=0; i<strlen(timeBuffer); i++) {
    timeBuffer[i] = timeBuffer[i+1];
  }

  // Open our file and print the time to the text file
  timeFile = fopen("currentTime.txt", "w");
  fprintf(timeFile, "%s", timeBuffer);

  // Before closing, read the file to the terminal
  fgets(timeBuffer, 80, timeFile);
  printf("%s\n", timeBuffer);
  fclose(timeFile);
}

int main() {
  // Initialize the rooms
  struct Room *gameRooms = createRooms();
  char *pathName = getDirectoryName();
  getRoomNames(gameRooms, pathName);
  theGame(gameRooms);

 return 0;
}
