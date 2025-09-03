#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif // _WIN32

#ifdef __unix__
// empty for now.
#endif // __unix__

#include <stdio.h>

#include "common.h"

#include "file.c"

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

    FILE* db_file         = NULL;
    Db_File_Header header = {0};

    if (new_file) {
        if (!file_create(file_path, &db_file)) {
            // TODO: log ?
            return -1;
        }

        if (!file_header_init(db_file, &header)) {
            // TODO: log ?
            return -1;
        }
    } else {
        if (!file_open(file_path, &db_file)) {
            // TODO: log ?
            return -1;
        }

        if (!file_header_read_and_validate(db_file, &header)) {
            // TODO: log ?
            return -1;
        }

    }

    printf(
        "precessing file: %s (v=%d count=%d size=%d is_new=%s)",
        file_path, header.version, header.count, header.file_size, new_file ? "true" : "false"
    );

    if (!file_write_header(db_file, &header)) {
        return -1;
    }

    fclose(db_file);

    return 0;
}

