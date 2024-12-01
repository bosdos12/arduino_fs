
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;


const int chipSelect = 4;

char userInput;

char userInputLine[32] = "";
char command[16] = "";
char target[16] = "";



// Function Declarations
void getSystemData();
void listFilesFunction();




// Create instances of the SdFat library
File theFile;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print(F("\nInitializing SD card..."));

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(chipSelect)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    Serial.println(F("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!"));
    while (1)
      ;
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  Serial.println(F("\nWelcome to Adak Celina arduino file system manager v1.0"));
  Serial.println(F("Type `help;` to get help."));

  // getSystemData();

  // listFilesFunction();
}

void loop(void) {
  if (Serial.available()) {
    userInput = Serial.read();
    if (userInput != ';') {
      userInputLine[strlen(userInputLine)] = userInput;
    } else {

      int i;
      for (i = 0; i < strlen(userInputLine); i++) {

        if (command[strlen(command) - 1] != ' ') {
          command[strlen(command)] = userInputLine[i];
        } else {
          target[strlen(target)] = userInputLine[i];
        }
      }



      Serial.print("userInputLine: ");
      Serial.println(userInputLine);
      Serial.print("Command: ");
      Serial.println(command);
      Serial.print("Target: ");
      Serial.println(target);

      if (strcmp(command, "help ") == 0 || strcmp(command, "help") == 0) {
        Serial.println(F("\nAdak Celina arduino file system manager v1.0"));
        Serial.println(F("Write commands in `command target;` format."));
        Serial.println(F("Remember to end each line with a semicolon (;)."));
        Serial.println(F("Certain commands can be run without a target."));
        Serial.println(F("help    | command to get help. Optionally write 'help commandname` for specific commands to get extra information."));
        Serial.println(F("sysinfo | get information about your storage system."));
        Serial.println(F("ls      | list all the files in your system."));
        Serial.println(F("touch   | Usage: 'touch filename'. Creates new file with the name given after the command touch."));
        Serial.println(F("read    | Usage: 'read filename.'. Displays the contents of a file."));
        Serial.println(F("del     | Usage: 'del filename.'. Deletes a file."));
      } else if (strcmp(command, "ls") == 0) {
        listFilesFunction(SD.open("/"), 0);
      } else if (strcmp(command, "sysinfo ") == 0 || strcmp(command, "sysinfo") == 0) {
        getSystemData();
      } else if (strcmp(command, "touch ") == 0) {
        if (SD.exists(target)) {
          Serial.println(F("A file with this name already exists."));
        } else {
          if (strlen(target) <= 13 && strlen(target) > 0) {
            Serial.print(F("Creating "));
            Serial.print(target);
            Serial.println(F("..."));

            theFile = SD.open(target, FILE_WRITE);
            theFile.close();

            Serial.print(target);
            Serial.println(F(" Created."));
          } else Serial.println(F("The file name must be be between 1 and 13 characters."));
        }
      } else if (strcmp(command, "read ") == 0) {
        if (SD.exists(target)) {
          theFile = SD.open(target);
          while (theFile.available()) {
            Serial.write(theFile.read());
          }
          
          theFile.close();
        } else Serial.println("File doesn't exist.");
      }else if (strcmp(command, "del ") == 0) {
        if (SD.exists(target)) { 
          SD.remove(target);
          Serial.println("File deleted.");
        } else Serial.println("File doesn't exist.");
      }

      memset(userInputLine, '\0', sizeof(userInputLine));
      memset(command, '\0', sizeof(command));
      memset(target, '\0', sizeof(target));
    }
  }
}


void getSystemData() {


  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    while (1)
      ;
  }

  // print the type of card
  Serial.println();
  Serial.print(F("Card type:         "));
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println(F("SD1"));
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println(F("SD2"));
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println(F("SDHC"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
    while (1)
      ;
  }

  Serial.print(F("Clusters:          "));
  Serial.println(volume.clusterCount());
  Serial.print(F("Blocks x Cluster:  "));
  Serial.println(volume.blocksPerCluster());

  Serial.print(F("Total Blocks:      "));
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();  // clusters are collections of blocks
  volumesize *= volume.clusterCount();     // we'll have a lot of clusters
  volumesize /= 2;                         // SD card blocks are always 512 bytes (2 blocks are 1 KB)
  Serial.print("Volume size (KB):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (MB):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (GB):  ");
  Serial.println((float)volumesize / 1024.0);
}


void listFilesFunction(File dir, int numTabs) {

  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    };
    entry.close();
  }
}
