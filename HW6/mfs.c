/*
    Happy Ndikumana
    1001641586

    To run:
        Type in the names of the files/directory as seen on the console
        add the extension to appropriate files

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BPBBytesPerSecOffset 11
#define BPBBytesPerSecSize 2

#define BPBSecPerClusOffset 13
#define BPBSecPerClusSize 1

#define BPBRsvdSecCntOffset 14
#define BPBRsvdSecCntSize 2

#define BPBNumFATsOffset 16
#define BPBNumFATsSize 1

#define BPBRootEntCntOffset 17
#define BPBRootEntCntSize 2

#define BPBFATSz32Offset 36
#define BPBFATSz32Size 4

#define FILENAMESIZE 20
#define DIR_NAME_LEN 11
#define COMMANDSIZE 200
#define MAXENTRIES 16 
#define INPUT_NUMS 5

struct __attribute__((__packed__)) directoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t unused[8];
    uint16_t clusterHigh;
    uint8_t unused2[4];
    uint16_t clusterLow; //address of the file
    uint32_t size;
};

uint16_t BPB_BytesPerSec;
uint8_t BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t BPB_NumFATs;
uint32_t BPB_FATSz32;
uint16_t BPB_RootEntCnt;
uint32_t rootCluster;
uint32_t currentCluster;

char * currentImage = NULL; // contains the name of currently open file
struct directoryEntry directory[MAXENTRIES];
FILE *fp;

int16_t nextLB(uint32_t sector) {
  uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector * 4);
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val, 2, 1, fp);
  return val;
}

void readInfoIn() 
{
    fseek(fp, BPBBytesPerSecOffset, SEEK_SET);
    fread(&BPB_BytesPerSec, BPBBytesPerSecSize, 1, fp);

    fseek(fp, BPBSecPerClusOffset, SEEK_SET);
    fread(&BPB_SecPerClus, BPBSecPerClusSize, 1, fp);

    fseek(fp, BPBRsvdSecCntOffset, SEEK_SET);
    fread(&BPB_RsvdSecCnt, BPBRsvdSecCntSize, 1, fp);

    fseek(fp, BPBNumFATsOffset, SEEK_SET);
    fread(&BPB_NumFATs, BPBNumFATsSize, 1, fp);

    fseek(fp, BPBRootEntCntOffset, SEEK_SET);
    fread(&BPB_RootEntCnt, BPBRootEntCntSize, 1, fp);

    fseek(fp, BPBFATSz32Offset, SEEK_SET);
    fread(&BPB_FATSz32, BPBFATSz32Size, 1, fp);

    rootCluster = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);
    currentCluster = rootCluster;
}
void printInfo() 
{
    if(currentImage == NULL)
    {
        printf("No file open at the moment\n");
        return;
    }
    printf("BPB_BytesPerSec: \t%d \t%X\n", BPB_BytesPerSec, BPB_BytesPerSec);
    printf("BPB_SecPerClus: \t%d\t %X\n", BPB_SecPerClus, BPB_SecPerClus);
    printf("BPB_RsvdSecCnt: \t%d \t%X\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
    printf("BPB_NumFATs: \t\t%d \t%X\n", BPB_NumFATs, BPB_NumFATs);
    printf("BPB_RootEntCnt: \t%d \t%X\n", BPB_RootEntCnt, BPB_RootEntCnt);
    printf("BPB_FATSz32: \t\t%d \t%X\n", BPB_FATSz32, BPB_FATSz32);

}
void readCurrentDirectory(int clusterAddress) 
{
    fseek( fp, clusterAddress, SEEK_SET);
    for (int i = 0; i < MAXENTRIES; i++) 
    {
        fread(&directory[i], sizeof(struct directoryEntry), 1, fp);
    }
}
int LBAToOffset(int32_t sector) 
{
    return ((sector -2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}
void openFile(char *file) 
{

    if (fp && strcmp(file, currentImage) == 0) 
    {
        printf("Error: File system image already open.\n");
        return;
    }
    if (currentImage != NULL) 
        free(currentImage);

    fp = fopen( file, "r");

    if (fp == NULL) 
    {
        printf("Error: File could not open\n");
        return;
    }
    currentImage = malloc(strlen(file) * sizeof(char));
    strcpy(currentImage, file);
    readInfoIn();

    return;
}
//closes file by settin fp to NULL and freeing the currentImage char pointer
void closeFile() 
{
    //if fp is NULL, no file is open, just return
    if(fp == NULL)
    {
        printf("Error: Open file system image first.\n");
        return;
    }

    fclose(fp);
    fp = NULL;
    currentImage = NULL;
    // free(currentImage);

}
int IndexOfFile(char *fileName) 
{
    if(!currentCluster)
        return -1;
    readCurrentDirectory(currentCluster);

    int fileNameLen = strlen(fileName);
    char *tempName = fileName;

    bool isFile = false;
    if(tempName[fileNameLen - 4] == '.')
        isFile = true;

    char *directoryName = malloc( DIR_NAME_LEN * sizeof(char) );

    if (!isFile) 
    {
        for (int i = 0; i < DIR_NAME_LEN; i++) 
        {
            if (i >= fileNameLen) 
            {
                directoryName[i] = ' ';
            }
            else 
            {
                directoryName[i] = tempName[i];
            }
        }
        fileName = directoryName;
    }
    else 
    {
        char *directoryName = malloc( DIR_NAME_LEN * sizeof(char) );
        int i = 0;
        while (tempName[i] != '.') 
        {
            directoryName[i] = tempName[i];
            i++;
        }

        while (i < DIR_NAME_LEN - 3) 
        {
            directoryName[i] = ' ';
            i++;
        }

        directoryName[DIR_NAME_LEN - 3] = tempName[fileNameLen - 3];
        directoryName[DIR_NAME_LEN - 2] = tempName[fileNameLen - 2];
        directoryName[DIR_NAME_LEN - 1] = tempName[fileNameLen - 1];

        fileName = directoryName;
    }
    for (int i = 0; i < MAXENTRIES; i++) 
    {
        char directoryName[12];
        memcpy(directoryName , directory[i].DIR_Name, 11);
        directoryName[11] = '\0';

        if (strcmp(fileName, directoryName) == 0) 
        {
            return i;
        }
    }
    return -1;
}
void printStat(char *fileName) 
{
    if (fp == NULL) 
        return;
    //in the current cluster, find the file passed in and print its stats
    int index = IndexOfFile(fileName);
    if (index != -1) 
    {
        printf("Attribute: \t\t\t%d\n", directory[index].DIR_Attr);
        printf("Size: \t\t\t\t%d\n", directory[index].size);
        printf("Starting Cluster Number: \t%d\n", directory[index].clusterLow);
    }
    else 
        printf("Error: File not found\n");
}
void copyFile(char *fileName) 
{
    if (!fp) 
        return;

    int fileNameLen = strlen(fileName);
    bool isFile = false;
    if(fileName[fileNameLen - 4] == '.')
        isFile = true;

    if (!isFile) 
    {
        printf("Error: directories cannot be copied. Try adding an extension to your file.\n");
        return;
    }

    int index = IndexOfFile(fileName);

    if (index == -1) 
    {
        printf("Error: File was not found\n");
        return;
    }

    //find address and size of the file
    int fileAddress = directory[index].clusterLow;
    int offset = LBAToOffset(fileAddress);
    int fileSize = directory[index].size;

    //create a new file in current computer directory
    char *newFileName = malloc( sizeof(char) * DIR_NAME_LEN + 1);
    memcpy(newFileName , directory[index].DIR_Name, 11);
    newFileName[11] = '\0';

    int nameLen = strlen(newFileName);
    char fileExt[3];
    fileExt[0] = newFileName[nameLen - 3];
    fileExt[1] = newFileName[nameLen - 2];
    fileExt[2] = newFileName[nameLen - 1];

    char *token = strtok(newFileName, " ");

    char *actualFile = malloc( ( sizeof(char) * strlen(token) + 4));
    int i;
    for(i = 0; i < strlen(token); i++) 
    {
        actualFile[i] = token[i];
    }
    actualFile[i] = '.';
    actualFile[i + 1] = fileExt[0];
    actualFile[i + 2] = fileExt[1];
    actualFile[i + 3] = fileExt[2];

    strcpy(newFileName, actualFile);
    fseek(fp, offset, SEEK_SET);
    //new file should be reable and writeable -> w+
    FILE *newFileptr = fopen(newFileName, "w+");

    //copy contents from other file into new file
    char buffer[512];

    while (fileSize >= 512) 
    {
        fread(buffer, 512, 1, fp);
        fwrite(buffer, 512, 1, newFileptr);
        fileAddress = nextLB(fileAddress);
        if (fileAddress == -1) 
            break;
        offset = LBAToOffset(fileAddress);
        fseek(fp, offset, SEEK_SET);
        fileSize -= 512;
    }
    if (fileSize > 0) 
    {
        fread(buffer, fileSize, 1, fp);
        fwrite(buffer, fileSize, 1, newFileptr);
    }

    fclose(newFileptr);

}
void readFile(char *file, char* StartPos, char* numberOfBytes) 
{
    //finds the file, and then reads the numberOfBytes of the contents starting at a given position
    if (!fp) 
        return;

    int index = IndexOfFile(file);

    if (index == -1) 
    {
        printf("Error: File not found. Try adding the file extension\n");
        return;
    }

    int position = atoi(StartPos);
    int bytes = atoi(numberOfBytes);

    int fileAddress = directory[index].clusterLow;
    int fileSize = directory[index].size;

    int offset = LBAToOffset(fileAddress);

    fseek(fp, offset, SEEK_SET);

    uint8_t buffer[position + bytes];

    fread(&buffer, position + bytes, 1, fp);

    for (int i = position; i < position + bytes; i++) {
        printf("0x%X ", buffer[i]);
    }
    printf("\n");

}
void printFiles() 
{
    if (!fp) 
        return;
    
    readCurrentDirectory(currentCluster);

    for (int i = 0; i < MAXENTRIES; i++) 
    {
        char fileName[12];
        memcpy(fileName , directory[i].DIR_Name, 11);
        fileName[11] = '\0';
        uint8_t attr = directory[i].DIR_Attr;
        if ((attr == 1 || attr == 16 || attr == 32) && (fileName[0] != (char)0xe5 && fileName[0] != (char)0x05 && fileName[0] != (char)0x00))
            printf("%s\n", fileName);
    }
}
void changeDirectory(char * directoryName)
{
    //check if directory name is  ".."
        //if yes, check if it's root directory
            //if yes, make the current directory = root directory
            //return
        //else
            //read the current directory and save parent directory
            //if parent directory == root
                //make the currentCluster = rootCluster
            //else
                //change the currentCluster to the cd'ed cluster with LBAToOffset
    //if dirNanem = ".", do nothing
    //if directoryName is anything else
        //find the index of the file in the cluster
        //using the that index, use the closter low and the LBAToOffset, make that the current directory

    if(strcmp(directoryName, "..") == 0)
    {
        //at lowest level
        if(currentCluster == rootCluster)
            return;
        readCurrentDirectory(currentCluster);
        int parentCluster = directory[1].clusterLow;
        if(parentCluster == 0)
        {
            currentCluster = rootCluster;
        }
        else
        {
            currentCluster = LBAToOffset(parentCluster);
        }
        return;
    }
    int fileIndex = IndexOfFile(directoryName);
    int dirAddress = directory[fileIndex].clusterLow;
    currentCluster = LBAToOffset(dirAddress);
}
int main() 
{
  char * userInput = malloc(COMMANDSIZE);

  while(true)
  {
    printf ("mfs> ");

    // fgets returns NULL when there is no input
    while(!fgets (userInput, COMMANDSIZE, stdin));

    char *inputs[INPUT_NUMS];
    char *strToParse  = strdup(userInput);
    char *strOriginal = strToParse;
    int inputsCount = 0;
    char *tokenPtr;

    while (((tokenPtr = strsep(&strToParse, " \t\n" ) ) != NULL) && (inputsCount < INPUT_NUMS)) 
    {
        inputs[inputsCount] = strndup( tokenPtr, COMMANDSIZE );
        if(strlen( inputs[inputsCount] ) == 0) 
        {
            inputs[inputsCount] = NULL;
        }
        inputsCount++;
    }

    if (inputs[0] != NULL) 
    {
        if ( strcmp(inputs[0], "open") == 0 ) 
        {
            if (inputs[1] != NULL) 
                openFile(inputs[1]);
        }
        else if ( strcmp(inputs[0], "close") == 0 ) 
            closeFile();
        else if (strcmp(inputs[0], "info") == 0) 
            printInfo();
        else if (strcmp(inputs[0], "stat") == 0)
        {
            if (inputs[1]) 
                printStat(inputs[1]);
        }
        else if (strcmp(inputs[0], "get") == 0) 
        {
            if (inputs[1]) 
                copyFile(inputs[1]);
        }
        else if (strcmp(inputs[0], "read") == 0) 
        {
            if (inputs[1] && inputs[2] && inputs[3]) 
                readFile(inputs[1], inputs[2], inputs[3]);
        }
        else if (strcmp(inputs[0], "ls") == 0) 
            printFiles();
        else if (strcmp(inputs[0], "cd") == 0) 
        {
            if (inputs[1]) 
                changeDirectory(inputs[1]);
        }
        else if (strcmp(inputs[0], "exit") == 0 || strcmp(inputs[0], "quit") == 0)
            exit(0);
        }
    }
  return 0;
}