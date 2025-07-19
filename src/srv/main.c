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

    init_clients(&clientStates);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket");
        return;
    }

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
        return -1;
    }

    if(listen(listen_fd, 10) == -1){
        perror("listen");
        return -1;
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
            return -1;
        }

        if(fds[0].revents & POLLIN){
            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len );
            if(conn_fd == -1){
                perror("accept");
                return -1;
            }

            printf("New connection from %s : %d \n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            free_slot = find_free_slot();
            if(free_slot == -1){
                printf("Server Full. Closing Connection.\n");
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
                int slot = find_free_slot(fd);

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
    return 0;
}



int main(int argc, char *argv[]) {
  int c = 0;
  bool newfile = false;
  char *file_path = NULL;
  int dbfd = -1;
  struct employee_t *employees;
  struct employee_t *employeesOut;
  char *employee_string = NULL;
  bool list = false;
  bool search_employee = false;
  bool remove_employee = false;
  bool change_hours = false;
  char *to_be_removed = NULL;
  char *to_be_searched = NULL;
  char *new_hours = NULL;
  struct dbheader_t *headerOut = NULL;

  while ((c = getopt(argc, argv, "nf:a:ls:r:h:")) != -1) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      file_path = optarg;
      break;
    case 'a':
      employee_string = optarg;
      break;
    case 'l':
      list = true;
      break;
    case 's':
      search_employee = true;
      to_be_searched = optarg;
      break;
    case 'r':
      remove_employee = true;
      to_be_removed = optarg;
      break;
    case 'h':
      change_hours = true;
      new_hours = optarg;
      break;
    case '?':
      printf("-%s is unkown option\n", optarg);
      break;
    default:
      printf("Error detected\n");
      return -1;
    }
  }
  if (file_path == NULL) {
    printf("File Path is a required argument.\n");
    print_usage(argv);
    return 0;
  }

  if (newfile) {
    dbfd = create_db_file(file_path);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file.\n");
      return -1;
    }
    int header_create = create_db_header(dbfd, &headerOut);
    if (header_create == STATUS_ERROR) {
      printf("Was not able to create the header.\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(file_path);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file.\n");
      return -1;
    }
    if (validate_db_header(dbfd, &headerOut) == STATUS_ERROR) {
      printf("Failed to validate the database header.\n");
      return STATUS_ERROR;
    }
  }

  if (read_employees(dbfd, headerOut, &employeesOut) != STATUS_SUCCESS) {
    printf("Reading employees failed. \n");
    return -1;
  }

  if (employee_string) {
    headerOut->count++;
    employeesOut = realloc(employeesOut, headerOut->count * (sizeof(struct employee_t)));
    if (employeesOut == NULL) {
      printf("Realloc for adding employee_failed.\n");
      return STATUS_ERROR;
    }
    add_employee(headerOut, employeesOut, employee_string);
  }

  if (search_employee) {
    search_employees(headerOut, employeesOut, to_be_searched);
  }

  if (remove_employee) {
    int remove_code = -1;
    remove_code = remove_employees(headerOut, employeesOut, to_be_removed);
        if(remove_code == 0){
            printf("could not find employee in database.\n");
        }else if(remove_code == 1){
            headerOut->count--;
            free(employeesOut);
            employeesOut = NULL;
        }else if(remove_code == 2){
            headerOut->count--;
            employeesOut = realloc(employeesOut, (headerOut->count * (sizeof(struct employee_t))));
            if(employeesOut == NULL){
                printf("Realloc failed for removing employee.\n");
            return STATUS_ERROR;
            }
        }
    }

  if(change_hours){
    employee_hour_change(headerOut, employeesOut, new_hours);
    }

  if (list) {
    list_employees(headerOut, employeesOut);
  }

  output_file(dbfd, headerOut, employeesOut);

  printf("newfile : %d\n", newfile);
  printf("file_path: %s\n", file_path);
  return 0;
}

/*
 * main function and some utility functions are going to be in this file.
 */
