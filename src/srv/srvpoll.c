#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "file.h"
#include "parse.h"
#include "srvpoll.h"

void init_clients(clientstate_t *states){
    for(int i = 0; i < MAX_CLIENTS; i++){
        states[i].fd = -1;
        states[i].state = STATE_NEW;
        memset(&states[i].buf, '\0', BUFF_SIZE);
    }
}

int find_free_slot(clientstate_t *states){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(states[i].fd == -1){
            return i;
        }
    }
    return -1;
}

int find_slot_by_fd(clientstate_t *states, int fd){
   for(int i = 0; i < MAX_CLIENTS; i++){
        if(states[i].fd == fd){
            return i;
       }
    }
    return -1;
}
