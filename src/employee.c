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

bool employees_create(Employee* employee, char* add_str) {
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

    return true;
}


