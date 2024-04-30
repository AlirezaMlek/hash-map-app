#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "common.h"

#define SHM_NAME "./app_shm"

enum RequestType string_to_enum(const char *str) {
    if (strcmp(str, "insert") == 0) {
        return INSERT;
    } else if (strcmp(str, "delete") == 0) {
        return DELETE;
    } else if (strcmp(str, "search") == 0) {
        return SEARCH;
    } else {
        printf("Invalid operation\n");
        exit(EXIT_FAILURE);
    }
}



bool checkFlag(char flag){
    if (flag==ADDED_SUCCESS || flag==ADDED_ABORT || flag==DELETE_SUCCESS || flag==DELETE_ABORT || flag==FOUND || 
    flag==NOT_FOUND || flag==ERROR){
        return true;
    } else{
        return false;
    }
}




int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <hash_table_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Map shared memory for request
    request_t* request = mmap(NULL, sizeof(request_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (request == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    request->operation = string_to_enum(argv[1]);
    request->key = atoi(argv[2]);
    
    if (request->operation == INSERT) {
        strncpy(request->value, argv[3], MAX_VALUE_SIZE - 1); // Copy the value to request
        request->value[MAX_VALUE_SIZE - 1] = '\0'; // Ensure null termination
    } else {
        // For other operations, set value to NULL
        request->value[0] = '\0';
    }

    // Wait for the response
    bool response_received = false;
    response_t* response;
    while (!response_received) {
        // Map shared memory for response
        response = mmap(NULL, sizeof(response_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (response == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        if (checkFlag(response->flag)) {
            response_received = true;
        } else {
            // Response not received yet, wait and retry
            munmap(response, sizeof(response_t));
            usleep(100000); // Wait for 100 milliseconds before retrying
        }
    }

    switch (response->flag) {
    case ADDED_SUCCESS:
        printf("Data added successfully\n");
        break;

    case ADDED_ABORT:
        printf("Insertion operation aborted\n");
        break;

    case DELETE_SUCCESS:
        printf("Data deleted successfully\n");
        break;

    case DELETE_ABORT:
        printf("Delete operation aborted\n");
        break;

    case FOUND:
        printf("Received data: %s\n", response->value);
        break;

    case NOT_FOUND:
        printf("Data not found\n");
        break;

    default:
        printf("Unknown error\n");
        break;
    }

    // Close shared memory
    munmap(request, sizeof(request_t));
    munmap(response, sizeof(response_t));
    close(shm_fd);

    return 0;
}
