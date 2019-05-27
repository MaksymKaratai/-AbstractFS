#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "Flash_FS.h"

/*
 * Start of the formatting functions block;
 * Every function formatting fnd write our FS structure into the specific file
 * argv 1 - FS size in bytes
 * argv 2 - blockSize in bytes
 * *////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SuperBlock makeSuperBlock(char *fsSize, char *blockSize) {
    struct SuperBlock block;
    block.sizeOfFS = atoi(fsSize);
    block.blockSizeInBytes = atoi(blockSize);
    block.numberOfBlocks = block.sizeOfFS / block.blockSizeInBytes;
    block.numberOfFreeBlocks = block.numberOfBlocks - 1; // reserve area for super block
    block.numberOfID = (block.numberOfBlocks - block.numberOfBlocks / 5) /
                       2; // two data block on single file descriptor, besides I reserve fifth part of all blocks to write needed structure elements
    block.firstFreeID = 0;
    block.bmidBlocs = (size_t) ceil((double) block.numberOfID / block.blockSizeInBytes);
    block.midBlocs = (size_t) ceil((double) block.numberOfID * sizeof(struct IndexDescriptor) / block.blockSizeInBytes);
    block.bmdbBlocs = (size_t) ceil((double) block.numberOfBlocks / block.blockSizeInBytes);
    block.mdbBlocs = block.numberOfBlocks - block.bmidBlocs - block.midBlocs - block.bmdbBlocs - 1;
    return block;
}

void writeSuperBlock(struct SuperBlock *sp, FILE *fs) {
    void *m = calloc(1, sp->blockSizeInBytes);
    *((struct SuperBlock *) m) = *sp;
    fwrite(m, sp->blockSizeInBytes, 1, fs);
    free(m);
}

void writeBitMapID(struct SuperBlock *sb, FILE *fs) {
    void *bitMap = calloc(1, sb->blockSizeInBytes);
    for (size_t i = 0; i < sb->blockSizeInBytes; i++) {
        ((char *) bitMap)[i] = '0';
    }
    for (int i = 0; i < sb->bmidBlocs; i++) {
        fwrite(bitMap, sb->blockSizeInBytes, 1, fs);
    }
    sb->numberOfFreeBlocks -= sb->bmidBlocs;//remove number of blocks that will be engaged
    free(bitMap);
}

void writeMID(struct SuperBlock *sb, FILE *fs) {
    void *mid = calloc(1, sb->blockSizeInBytes);
    for (int i = 0; i < sb->midBlocs; ++i) {
        fwrite(mid, sb->blockSizeInBytes, 1, fs);
    }
    sb->numberOfFreeBlocks -= sb->midBlocs;
    free(mid);
}

void writeBitMapDB(struct SuperBlock *sb, FILE *fs) {
    void *bitMapDB = calloc(1, sb->blockSizeInBytes);
    for (int i = 0; i < sb->blockSizeInBytes; ++i) {
        ((char *) bitMapDB)[i] = '0';
    }
    for (int i = 0; i < sb->bmdbBlocs; ++i) {
        fwrite(bitMapDB, sb->blockSizeInBytes, 1, fs);
    }
    sb->numberOfFreeBlocks -= sb->bmdbBlocs;
    free(bitMapDB);
}

void writeMDB(struct SuperBlock *sb, FILE *fs) {
    void *MDB = calloc(1, sb->numberOfFreeBlocks * sb->blockSizeInBytes);
    fwrite(MDB, sb->numberOfFreeBlocks * sb->blockSizeInBytes, 1, fs);
    sb->numberOfFreeBlocks -= sb->mdbBlocs;
    free(MDB);
}

FILE *formatFS(char *fsSize, char *blockSize) {
    struct SuperBlock superBlock = makeSuperBlock(fsSize, blockSize);
    FILE *fs;
    if ((fs = fopen("fs.txt", "wb+")) == NULL) {
        printf("Error file opening when creating file system\n");
        exit(1);
    }
    writeSuperBlock(&superBlock, fs);
    writeBitMapID(&superBlock, fs);
    writeMID(&superBlock, fs);
    writeBitMapDB(&superBlock, fs);
    writeMDB(&superBlock, fs);
    return fs;
}
/*
 * End of formatting function block!!!
 */////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Start of FS function block
 * Every function at this block may help to work with our simple FS
 *//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int createFile(char *fileName, FILE *fs) {
    struct SuperBlock sb;
    char c = '1';
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    fseek(fs, sb.blockSizeInBytes + sb.firstFreeID, SEEK_SET);
    fwrite(&c, 1, 1, fs);
    struct IndexDescriptor id = makeID(fileName, sb);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (sb.firstFreeID * sizeof(struct IndexDescriptor)), SEEK_SET);
    fwrite(&id, sizeof(struct IndexDescriptor), 1, fs);
    ///////////
    sb.firstFreeID = findFreeID(sb, fs);
    rewind(fs);
    fwrite(&sb, sizeof(struct SuperBlock), 1, fs);
    return id.index;
}

struct IndexDescriptor makeID(char *fileName, struct SuperBlock sb) {
    time_t t;
    struct tm *tm;
    struct IndexDescriptor newFile;
    newFile.index = sb.firstFreeID;
    newFile.amountOfBlocks = 0;
    newFile.numberOfBytes = 0;
    strcpy(newFile.fileName, fileName);
    time(&t);
    tm = localtime(&t);
    strftime(newFile.date, 11, "%Y:%m:%d", tm);
    return newFile;
}


int find(char *fileName, FILE *fs) {
    struct SuperBlock sb;
    struct IndexDescriptor tmp;
    char c;
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    for (int i = 0; i < sb.numberOfID; ++i) {
        fseek(fs, sb.blockSizeInBytes + i, SEEK_SET);
        fread(&c, 1, 1, fs);
        if (c == '1') {
            fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (i * sizeof(struct IndexDescriptor)), SEEK_SET);
            fread(&tmp, sizeof(struct IndexDescriptor), 1, fs);
            if (0 == strcmp(fileName, tmp.fileName)) {
                return i;
            }
        }
    }
    return -1;
}

int findFreeID(struct SuperBlock sb, FILE *fs) {
    int id = -1;
    char c;
    for (int i = 0; i < sb.numberOfID; ++i) {
        fseek(fs, sb.blockSizeInBytes + i, SEEK_SET);
        fread(&c, 1, 1, fs);
        if (c == '0') {
            id = i;
            break;
        }
    }
    return id;
}

short findFreeDB(struct SuperBlock sb, FILE *fs) {
    short id = -1;
    char c;
    char e = '1';
    for (short i = 0; i < sb.numberOfID; ++i) {
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs) + i, SEEK_SET);
        fread(&c, 1, 1, fs);
        if (c == '0') {
            id = i;
            fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs) + i, SEEK_SET);
            fwrite(&e, 1, 1, fs);
            break;
        }
    }
    return id;
}

int open(char *fileName, FILE *fs) {
    int res = find(fileName, fs);
    if (res != -1) {
        return res;
    } else {
        createFile(fileName, fs);
    }
}

void _write(char *text, char *fileName, FILE *fs) {//very hard method
    struct SuperBlock sb;
    struct IndexDescriptor id;
    int idNum = find(fileName, fs);
    char c = '1';
    if (idNum == -1) {
        printf("ERROR occurred trying write in file : '%s' , file does not exist!\n", fileName);
        return;
    }
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fread(&id, sizeof(struct IndexDescriptor), 1, fs);
    if (id.amountOfBlocks == 0) {// file was empty
        id.startBlockID = findFreeDB(sb, fs);
        id.amountOfBlocks++;
    }
    short next = id.startBlockID;
    int difference = abs(
            id.amountOfBlocks * sb.blockSizeInBytes - (id.numberOfBytes + ((id.amountOfBlocks - 1) * sizeof(short))));
    for (int i = 1; i < id.amountOfBlocks; ++i) {
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next)
                  + (sb.blockSizeInBytes - sizeof(short)), SEEK_SET);
        fread(&next, sizeof(short), 1, fs);
    }
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next)
              + (sb.blockSizeInBytes - difference), SEEK_SET);
    char buf[sizeof(short) + 1];
    short new;
    if (difference == 0) {
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next)
                  + (sb.blockSizeInBytes - sizeof(short)), SEEK_SET);
        fread(&buf, sizeof(short), 1, fs);
        new = findFreeDB(sb, fs);
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next)
                  + (sb.blockSizeInBytes - sizeof(short)), SEEK_SET);
        next = new;
        fwrite(&next, sizeof(short), 1, fs);
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next), SEEK_SET);
        fwrite(&buf, sizeof(short), 1, fs);
        id.amountOfBlocks++;
    }
    if (id.numberOfBytes + strlen(text) > id.amountOfBlocks * sb.blockSizeInBytes) { // check file overflowing
        while (strlen(text) > difference) {
            printf("---");
            int offset = difference - sizeof(short);
            fwrite(text, offset, 1, fs);
            new = findFreeDB(sb, fs);
            fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next)
                      + (sb.blockSizeInBytes - difference) + offset, SEEK_SET);
            next = new;
            fwrite(&next, sizeof(short), 1, fs);
            text += offset;
            id.amountOfBlocks++;
            id.numberOfBytes += offset;
            difference = sb.blockSizeInBytes;
        }
    }
    //else simple write
    fwrite(text, strlen(text), 1, fs);
    id.numberOfBytes += strlen(text);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fwrite(&id, sizeof(struct IndexDescriptor), 1, fs);
}

char *_read(char *fileName, FILE *fs) {
    struct SuperBlock sb;
    struct IndexDescriptor id;
    int idNum = find(fileName, fs);
    if (idNum == -1) {
        printf("File : '%s' does not exist!\n", fileName);
        return NULL;
    }
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fread(&id, sizeof(struct IndexDescriptor), 1, fs);
    if (id.numberOfBytes == 0) {
        printf("File '%s' is empty, nothing to read !\n", fileName);
        return NULL;
    }
    char *result = malloc(id.numberOfBytes);
    short next = id.startBlockID;
    int size = sb.blockSizeInBytes - sizeof(short);
    char buff[size];
    for (int i = 0; i < id.amountOfBlocks; ++i) {
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next), SEEK_SET);
        fread(buff, size, 1, fs);
        strcat(result, buff);
        fread(&next, sizeof(short), 1, fs);
    }
    return result;
}

void delete(char *fileName, FILE *fs) {
    int idNum = find(fileName, fs);
    if (idNum == -1) {
        printf("File '%s' does not exist in file system !\n", fileName);
        return;
    }
    char c = '0';
    struct SuperBlock sb;
    struct IndexDescriptor id;
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fread(&id, sizeof(struct IndexDescriptor), 1, fs);
    fseek(fs, sb.blockSizeInBytes + id.index, SEEK_SET);
    fwrite(&c, 1, 1, fs);
    void *idMemory = calloc(1, sizeof(struct IndexDescriptor));
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (id.index * sizeof(struct IndexDescriptor)), SEEK_SET);
    fwrite(idMemory, sizeof(struct IndexDescriptor), 1, fs);
    void *bdMemory = calloc(1, sb.blockSizeInBytes);
    short next = id.startBlockID;
    short new;
    for (int i = 0; i < id.amountOfBlocks; ++i) {
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs) + next, SEEK_SET);
        fwrite(&c, 1, 1, fs);
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next) + (sb.blockSizeInBytes -
                                                                                                  sizeof(short)),
              SEEK_SET);
        fread(&new, sizeof(short), 1, fs);
        fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + next), SEEK_SET);
        fwrite(bdMemory, sb.blockSizeInBytes, 1, fs);
        next = new;
    }

    sb.firstFreeID = findFreeID(sb, fs);
    rewind(fs);
    fwrite(&sb, sizeof(struct SuperBlock), 1, fs);
    free(idMemory);
    free(bdMemory);
}

void ls(FILE *fs) {
    struct SuperBlock sb;
    struct IndexDescriptor tmp;
    char c;
    int j = 1;
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    for (int i = 0; i < sb.numberOfID; ++i) {
        fseek(fs, sb.blockSizeInBytes + i, SEEK_SET);
        fread(&c, 1, 1, fs);
        if (c == '1') {
            fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (i * sizeof(struct IndexDescriptor)), SEEK_SET);
            fread(&tmp, sizeof(struct IndexDescriptor), 1, fs);
            printf(" %d) ------- %s ------- %d bytes ------- %s ----ID = %d\n", j, tmp.fileName, tmp.numberOfBytes, tmp.date, tmp.index+1);
            j++;
        }
    }
}
/*
* End of FS function block!!!
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * void _write(char *text, char *fileName, FILE *fs) {
    struct SuperBlock sb;
    struct IndexDescriptor id;
    int idNum = find(fileName, fs);
    if (idNum == -1) {
        printf("ERROR occurred trying write in file : '%s' , file does not exist!\n", fileName);
        return;
    }
    rewind(fs);
    fread(&sb, sizeof(struct SuperBlock), 1, fs);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fread(&id, sizeof(struct IndexDescriptor), 1, fs);
    if (id.numberOfBytes + strlen(text) > id.amountOfBlocks * sb.blockSizeInBytes) { // check file overflowing
        printf("ERROR occurred trying write in file : '%s' , file overflow!\n", fileName);
        return;
    }
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs + sb.midBlocs + sb.bmdbBlocs + id.startBlockID) + id.numberOfBytes, SEEK_SET);
    fwrite(text, strlen(text), 1, fs);
    id.numberOfBytes += strlen(text);
    fseek(fs, sb.blockSizeInBytes * (1 + sb.bmidBlocs) + (idNum * sizeof(struct IndexDescriptor)), SEEK_SET);
    fwrite(&id, sizeof(struct IndexDescriptor), 1, fs);
}
 * */