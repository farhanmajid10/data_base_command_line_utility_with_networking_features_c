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
    return 0;
}

/*
 * main function and some utility functions are going to be in this file.
 */
