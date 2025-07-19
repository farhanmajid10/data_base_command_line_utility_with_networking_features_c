#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <file_path>\n", argv[0]);
  printf("\t-n : create new database file.\n");
  printf("\t-f : (required) path to database file.\n");
  return;
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
