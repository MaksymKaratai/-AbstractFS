#include <stdio.h>

struct SuperBlock {
    int sizeOfFS;
    int numberOfBlocks;
    int numberOfFreeBlocks;
    int numberOfID;
    int firstFreeID;
    int blockSizeInBytes;
    size_t bmidBlocs;
    size_t midBlocs;
    size_t bmdbBlocs;
    size_t mdbBlocs;
};

struct IndexDescriptor {
    int index;
    char fileName[20];
    char date[11];
    int amountOfBlocks;
    short startBlockID;
    int numberOfBytes;
};


// function that uses to format FS
struct SuperBlock makeSuperBlock(char *fsSize, char *blockSize);
void writeSuperBlock(struct SuperBlock* sp, FILE* fs);
void writeBitMapID(struct SuperBlock* sb, FILE* fs);
void writeMID(struct SuperBlock *sb, FILE *fs);
void writeBitMapDB(struct SuperBlock *sb, FILE *fs);
void writeMDB(struct SuperBlock *sb, FILE *fs);
FILE* formatFS(char *fsSize, char *blockSize);

//function to work with FS
int createFile(char *fileName, FILE *fs);
struct IndexDescriptor makeID(char *fileName, struct SuperBlock sb);
int findFreeID(struct SuperBlock sb, FILE *fs);
short findFreeDB(struct SuperBlock sb, FILE *fs);
void normalization(struct SuperBlock, struct IndexDescriptor, FILE *fs);
int find(char *fileName, FILE *fs);
int open(char *fileName, FILE *fs);
void delete(char *fileName, FILE *fs);
void _write(char *text, char *fileName, FILE *fs);
char* _read(char *fileName, FILE *fs); // function allocate memory for information in file, after using you must free it by self!
void ls(FILE *fs); // print all files in file system
