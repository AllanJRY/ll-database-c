#include <stdio.h>

#include "common.h"

/* 
 * note: for a better code with more controls on files permission etc. OS specific calls should be 
 * used. For examples: 
 * - on __unix__: open(...), stat(...) etc.
 * - on _WIN32: CreateFileA(...), GetFileAttributes(...), OpenFile(...) etc.
 */

// TODO: could be a single method with a mode in first arg (like OPEN, CREATE, etc.) ?
//       Also, should logs should be removed from here and return an int with a specific value for
//       different kinf of error ? like (OK, KO, KO_ALREADY_EXISTS etc...)
bool file_create(char* file_path, FILE* new_file) {
    // This check can be better by using OS specific functions.
    FILE* check = fopen(file_path, "rb");
    if (check != NULL) {
        printf("file_create: File already exists");
        fclose(check);
        return false;
    }
    
    new_file = fopen(file_path, "a");

    if (new_file == NULL) {
        perror("file_create");
        return false;
    }

    return true;
}

bool file_open(char* file_path, FILE* opened_file) {
    opened_file = fopen(file_path, "rb");
    
    if (opened_file == NULL) {
        perror("file_open");
        return false;
    }

    return true;
}
