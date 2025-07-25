#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"
#include "srvpoll.h"

#include <asm-generic/socket.h>
#include <endian.h>
#include <sys/poll.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>

clientstate_t clientStates[MAX_CLIENTS] = {0};

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <file_path>\n", argv[0]);
  printf("\t-n : create new database file.\n");
  printf("\t-f : (required) path to database file.\n");
  printf("\t-p : (required) port to listen to. \n");
  return;
}

void poll_loop(unsigned short port, struct dbheader_t *dbhdr, struct employee_t *employees){
    int conn_fd, listen_fd, free_slot;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
     
    fd_set read_fds, write_fds;
    
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    int opt = 1;

    init_clients(clientStates);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket");
        return;
    }

   //might have an error. have to check the man page. 
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("setsockopt");
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        return;
    }

    if(listen(listen_fd, 10) == -1){
        perror("listen");
        return;
    }

    
    memset(fds, 0, sizeof(fds));
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    nfds = 1;

    while(1){

        int ii = 1;
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clientStates[i].fd != -1){
                fds[ii].fd = clientStates[i].fd;
                fds[ii].events = POLLIN;
                ii++;
            }
        }

        int n_events = poll(fds, nfds, -1);
        if(n_events == -1){
            perror("poll");
            return;
        }

        if(fds[0].revents & POLLIN){
            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len );
            if(conn_fd == -1){
                perror("accept");
                return;
            }

            printf("New connection from %s : %d \n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            free_slot = find_free_slot(clientStates);
            if(free_slot == -1){
                printf("Server Full. Closing new Connection.\n");
                close(conn_fd);
            }else{
                clientStates[free_slot].fd = conn_fd;
                clientStates[free_slot].state = STATE_CONNECTED;
                nfds++;
                printf("Slot %d has fd %d\n", free_slot, clientStates[free_slot].fd);
            }
            n_events--;
        }
            
        for(int i = 1; i < nfds && n_events > 0; i++){
            if(fds[i].revents & POLLIN){
                n_events--;

                int fd = fds[i].fd;
                int slot = find_slot_by_fd(clientStates, fd);

                ssize_t bytes_read = read(fd, &clientStates[slot].buf, sizeof(clientStates[slot].buf));
                
                if(bytes_read <= 0){
                    close(fd) ;
                    if(slot == -1){
                        printf("Tried to close fd but fd doesn't exist\n");
                    }else{
                        clientStates[i].state = STATE_DISCONNECTED;
                        clientStates[i].fd = -1;
                        printf("Client disconnected or error.\n");
                        nfds--;
                    }
                }else {
                    printf("Data Received from client : %s\n", clientStates[slot].buf);
                } 
            }
        }    
    }
    return;
}




int main(int argc, char *argv[]) {

    char* filepath = NULL;
    char* portarg = NULL;
    unsigned short port = 0;
    bool newfile = false;
    bool list = false;
    int c;

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;
    
    while((c = getopt(argc, argv, "nf:p:")) != -1){
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                if(port == 0){
                    printf("Bad Port\n");
                }
                break;
            case '?':
                printf("Unknown Option -%c\n", c);
            default:
                return -1;
        }
    }
        
    if(filepath == NULL){
        printf("Filepath is a required argument.\n");
        print_usage(argv);    
    }

    if(port == 0){
        printf("port is not set\n");
        print_usage(argv);    
    }

    if(newfile){
        dbfd = create_db_file(filepath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to create database file.\n.");
            return -1;
        }

        if(create_db_header(dbfd, &dbhdr)){
            printf("Unable to create database header.\n");
            return -1;
        }
    }else{
        dbfd = open_db_file(filepath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to open database file.\n");
            return -1;
        }
        if(validate_db_header(dbfd, &dbhdr) == STATUS_ERROR){
            printf("failed to validate database header.\n");
            return -1;
        }
    }
        if(read_employees(dbfd, dbhdr, &employees) !=STATUS_SUCCESS){
            printf("failed to read employees.\n");
            return 0;
        }
        poll_loop(port, dbhdr,  employees);
        
        output_file(dbfd, dbhdr, employees);

        return 0;
}

/*
 * main function and some utility functions are going to be in this file.
 */
