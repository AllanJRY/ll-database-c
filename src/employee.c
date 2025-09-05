/*
 * NOTE: For the moment, this file is named employee.c, but once more business data type and storage
 * features will be added, this file could be rename or the project could be re-organized.
 */

#include <stdio.h>
#include <string.h>

#include "common.h"

typedef struct Employee { 
    char name[256];
    char address[256];
    u32  hours;
} Employee;

bool employees_read(FILE* db_file, u32 employee_count, Employee* employees) {
    if (db_file == NULL) {
        printf("employees_read: Db file is null");
        return false;
    }

    if (employees == NULL) {
        printf("employees_read: Employees is null");
        return false;
    }

    size_t employee_read_count = fread(employees, sizeof(Employee), employee_count, db_file);

    if(employee_read_count != employee_count) {
        printf("employees_read: Expected to read %d employees from file but read %zu\n", employee_count, employee_read_count);
        return false;
    }

    for (u32 i = 0; i < employee_count; i += 1) {
        // Only for multi bytes type. char is 1 byte si name, and address not be changed, 
        // but hours is 4 bytes so the endianness impact this value.
        employees[i].hours = htonl(employees[i].hours);
    }

    return true;
}

bool employees_create(u16* employee_count, Employee* employee, char* add_str) {
    if (employee == NULL) {
        printf("employees_create: Employee is null");
        return false;
    }

    if (add_str == NULL) {
        printf("employees_create: Add string is null");
        return false;
    }

    char* name  = strtok(add_str, ",");
    char* addr  = strtok(NULL, ",");
    char* hours = strtok(NULL, ",");

    strncpy(employee->name, name, sizeof(employee->name));
    strncpy(employee->address, addr, sizeof(employee->address));
    employee->hours = atoi(hours);

    *employee_count += 1;

    return true;
}

u16 employees_delete(u16* employee_count, Employee* employees, char* name) {
    u16 employee_deleted_count = 0;

    u16 i = 0;
    for (;i < *employee_count;) {
        Employee* employee = &employees[i];

        if (strcmp(employee->name, name) == 0) {
            employee_deleted_count += 1;
            *employee_count        -= 1;
            memcpy(employee, &employees[*employee_count], sizeof(Employee));
        } else {
            // We only increment if no deletion occured, because we replace the deleted one with
            // the last one to avoid shifting element, if we increment, the last element which has
            // been placed on the empty deleted slot will be skipped, but we need to check it in case
            // it has to be deleted too.
            i += 1;
        }
    }

    return employee_deleted_count;
}
