#ifndef SRVPOLL_H
#define SRVPOLL_H

#include "poll.h"
#include <stdatomic.h>

#define MAX_CLIENTS 256
#define PORT 8080
#define BUFF_SIZE 4096

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} state_e;

typedef struct {
    int fd;
    state_e state;
    char buf[BUFF_SIZE];
} clientState_t;

void init_clients(clientState_t *states);

int find_free_slot(clientState_t *states);

int find_slots_by_id(clientState_t *states, int fd);

#endif
