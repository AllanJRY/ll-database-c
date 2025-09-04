#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif // _WIN32

#ifdef __unix__
// empty for now.
#endif // __unix__

#include <stdio.h>

#include "common.h"

#include "file.c"
#include "employee.c"

void print_usage(char* bin_name) {
    printf("Usage: %s -f <database file>\n", bin_name);
    printf("\t -f -- (required) path to database file\n");
    printf("\t -n -- create new database file\n");
}

int main(int argc, char** argv) {
    bool new_file   = false;
    char* file_path = NULL;
    char* add_str   = NULL;

    for (int arg_idx = 1; arg_idx < argc; arg_idx += 1) {
        char* flag = argv[arg_idx];

        if (flag[0] != '-') {
            printf("Invalid option '%s'\n", flag);
            return -1;
        }

        switch(flag[1]) {
            case 'a': {
                arg_idx += 1;
                add_str = argv[arg_idx];
                break;
            }
            case 'f': {
                arg_idx += 1;
                file_path = argv[arg_idx];
                break;
            }
            case 'n': {
                new_file = true;
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
        "precessing file: %s (v=%d count=%d size=%d is_new=%s)\n\n",
        file_path, header.version, header.count, header.file_size, new_file ? "true" : "false"
    );

    // NOTE: Because employees is not a growable list yet, we assume that add can only append 1 customer,
    // so we pre-allocate a slot for him.
    u32 employees_capacity = header.count;
    if (add_str != NULL) {
        employees_capacity += 1; 
    }
    Employee* employees = calloc(employees_capacity, sizeof(Employee));

    if(employees == NULL) {
        perror("main");
        fclose(db_file);
        return -1;
    }

    if (!employees_read(db_file, header.count, employees)) {
        fclose(db_file);
        return -1;
    }

    if (add_str != NULL) {
        // employees should grow
        if (employees_create(&employees[header.count], add_str)) {
            header.count     += 1;
            header.file_size += sizeof(Employee);
        } else {
            printf("unable to create employee");
            fclose(db_file);
            return -1;
        }
    }

    for (int i = 0; i < header.count; i += 1) {
        Employee* employee = &employees[i];
        printf("[%d]: name=%s address=%s hours=%d\n", i, employee->name, employee->address, employee->hours);
    }


    // TODO: extract into a sanitizing function ? And use fn pointer in file_write to sanitize the data before writing on file ?
    for (u32 i = 0; i < header.count; i += 1) {
        // Only for multi bytes type. char is 1 byte si name, and address not be changed, 
        // but hours is 4 bytes so the endianness impact this value.
        employees[i].hours = ntohl(employees[i].hours);
    }

    if (!file_write(db_file, &header, employees, sizeof(Employee))) {
        return -1;
    }

    fclose(db_file);

    return 0;
}

