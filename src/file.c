#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __unix__
#include <arpa/inet.h>
#endif

#include <stdio.h>

#include "common.h"

/* 
 * note: for a better code with more controls on files permission etc. OS specific calls should be 
 * used. For examples: 
 * - on __unix__: open(...), stat(...) etc.
 * - on _WIN32: CreateFileA(...), GetFileAttributes(...), OpenFile(...) etc.
 */

// Help making sure the file that where are reading is a valid file.
#define HEADER_MAGIC 0x4c4c4144

typedef struct Db_File_Header {
    u32 magic;
    u16 version;
    u16 count;
    u32 file_size;
} Db_File_Header;

// TODO: could be a single method with a mode in first arg (like OPEN, CREATE, etc.) ?
//       Also, should logs should be removed from here and return an int with a specific value for
//       different kinf of error ? like (OK, KO, KO_ALREADY_EXISTS etc...)
bool file_create(char* file_path, FILE** new_file) {
    // This check can be better by using OS specific functions (GetFileAttributes on windows and stat like fn on unix).
    FILE* check = fopen(file_path, "rb");
    if (check != NULL) {
        printf("file_create: File already exists");
        fclose(check);
        return false;
    }
    
    *new_file = fopen(file_path, "ab");

    if (new_file == NULL) {
        perror("file_create");
        return false;
    }

    return true;
}

bool file_open(char* file_path, FILE** opened_file) {
    *opened_file = fopen(file_path, "rb+");
    
    if (*opened_file == NULL) {
        perror("file_open");
        return false;
    }

    return true;
}

bool file_write(FILE* db_file, Db_File_Header* header, void* data, size_t data_bloc_size) {
    if (db_file == NULL) {
        printf("file_write_header: Db file is null");
        return false;
    }

    if (header == NULL) {
        printf("file_write_header: Header is null");
        return false;
    }
    
    // keep the count before endianness conversion for later use.
    u32 data_count = header->count;

    header->version   = htons(header->version);
    header->count     = htons(header->count);
    header->magic     = htonl(header->magic);
    header->file_size = htonl(header->file_size);

    rewind(db_file);

    if (fwrite(header, sizeof(Db_File_Header), 1, db_file) != 1) {
        printf("file_write_header: Error while writing file header\n");
        perror("file_write_header");
        return false;
    }

    if(data_count > 0) {
        if (fwrite(data, data_bloc_size, data_count, db_file) != data_count) {
            printf("file_write_header: Error while writing file's data\n");
            perror("file_write_header");
            return false;
        }
    }

    return true;
}

bool file_header_init(FILE* db_file, Db_File_Header* header_to_init) {
    if (header_to_init == NULL) {
        printf("file_header_init: Header is null");
        return false;
    }

    header_to_init->version   = 0x1;
    header_to_init->count     = 0;
    header_to_init->magic     = HEADER_MAGIC;
    header_to_init->file_size = sizeof(Db_File_Header);
    return true;
}

bool file_header_read_and_validate(FILE* db_file, Db_File_Header* header) {
    if (db_file == NULL) {
        printf("file_header_read_and_validate: Db file is null");
        return false;
    }

    if (header == NULL) {
        printf("file_header_read_and_validate: Header is null");
        return false;
    }

    // FIXME: could silently fail if the file is empty (like when new file is created, but no header written).
    fseek(db_file, 0, SEEK_END);
    size_t file_size = ftell(db_file);
    rewind(db_file);

    if(fread(header, sizeof(Db_File_Header), 1, db_file) != 1) {
        perror("file_header_read_and_validate");
        return false;
    }

    header->version   = ntohs(header->version);
    header->count     = ntohs(header->count);
    header->magic     = ntohl(header->magic);
    header->file_size = ntohl(header->file_size);

    if (header->magic != HEADER_MAGIC) {
        printf("Invalid header magic");
        return false;
    }

    if (header->version != 1) {
        printf("Invalid header version");
        return false;
    }

    if (header->file_size != file_size) {
        printf("Invalid header file size");
        return false;
    }

    return true;
}
