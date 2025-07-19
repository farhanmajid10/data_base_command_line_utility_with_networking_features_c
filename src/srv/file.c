#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"


int create_db_file(char *filename) {
    int fd = open(filename, O_RDWR);
    if(fd != -1){
        close(fd);//if something is open that souldn't be open, please remember to close it instead of just complaining about it.
        printf("File already exists.\n");
        return STATUS_ERROR;
    }
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if(fd == -1){
        perror("open");
        printf("Could not open file. Error Detected.\n");
        return STATUS_ERROR;
    }
    return fd;
}

int open_db_file(char *filename) {
    int fd = open(filename, O_RDWR,0664);
    if(fd == -1){
        perror("open");
        printf("Unable to open the file. Error Detected.\n");
        return STATUS_ERROR;
    }
    return fd;
}

/*
* Our goal is to handle input and output here, meaning opening and closing a file.
*/
