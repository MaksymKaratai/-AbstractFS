#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Flash_FS.h"

void help();
int isArg(char *s);
char* getString(char *argv[], int argc);

int main(int argc, char* argv[]) {
    FILE *fs = fopen("fs.txt", "rb+");
    char *opts = "fcwrdlh";
    char *tmp;
    switch (getopt(argc, argv, opts)) {
            case 'f':
                fclose(fs);
                fs = formatFS(argv[2], argv[3]);
                break;
            case 'c':
                open(argv[2], fs);
                break;
            case 'w':
                tmp = getString(argv,argc);
                _write(tmp, argv[2], fs);
                free(tmp);
                break;
            case 'r':
                tmp = _read(argv[2], fs);
                printf("File '%s' : \n%s\n", argv[2], tmp);
                free(tmp);
                break;
            case 'd':
                delete(argv[2], fs);
                break;
            case 'l':
                ls(fs);
                break;
            case 'h':
                help();
                break;
            default: printf("Wrong input !!!\n");
    };

    fclose(fs);
    return 0;
}

void help() {
    printf("-f 'fsSize' 'blockSize'   (format and create file system, all parameters are in bytes)\n");
    printf("-c 'fileName'    (create file in FS with name 'fileName', if file exist do not rewrite him)\n");
    printf("-w 'fileName' 'text'   (write 'text' in file 'fileName', if file does not exist or overflow - ERROR occurred)\n");
    printf("-r 'fileName' (read all information from file 'fileName')\n");
    printf("-d 'fileName' (delete file 'fileName' from file system)\n");
    printf("-l  (show all files in file system)\n");
    printf("-h   (help)\n");
}

int isArg(char *s) {
    if (s == NULL) {
        return -1;
    }
    if (strcmp(s,"-a") == 0 || strcmp(s,"-d") == 0 || strcmp(s,"-h") == 0 || strcmp(s,"-c") == 0) {
        return -1;
    } else return 1;
}

char* getString(char *argv[], int argc) {
    char *tmp = malloc(1000);
    tmp[0] = '\0';
    for (int i = 3; i < argc; ++i) {
        strcat(tmp, argv[i]);
        if (i != argc-1)
            strcat(tmp," ");
    }
    return tmp;
}


//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv