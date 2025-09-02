#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>

#include "common.h"

void print_usage(char* bin_name) {
    printf("Usage: %s -f <database file>\n", bin_name);
    printf("\t -f -- (required) path to database file\n");
    printf("\t -n -- create new database file\n");
}

int main(int argc, char** argv) {

    char* file_path = NULL;
    bool new_file = false;

    for (int arg_idx = 1; arg_idx < argc; arg_idx += 1) {
        char* flag = argv[arg_idx];

        if (flag[0] != '-') {
            printf("Invalid option '%s'\n", flag);
            return -1;
        }

        switch(flag[1]) {
            case 'n': {
                new_file = true;
                break;
            }
            case 'f': {
                arg_idx += 1;
                file_path = argv[arg_idx];
                break;
            }
            default: {
                printf("Unknown option '%s'\n", flag);
            }
        }
    }

    if (file_path == NULL) {
        printf("A file path must be given\n");
        print_usage(argv[0]);
        return -1;
    }

    printf("Db File: %s", file_path);
    if(new_file) {
        printf(" (new)");
    }
    printf("\n");

    return 0;
}

