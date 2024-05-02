#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "common.h"


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



bool check_flag(char flag){
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

    
    int shm_fd_req;
    int shm_fd_res;

    // Open shared memory
    shm_fd_req = shm_open(SHM_NAME_REQ, O_CREAT | O_RDWR, 0777);
    if (shm_fd_req == -1)
    {
        perror("shm_open request");
        exit(EXIT_FAILURE);
    }

    shm_fd_res = shm_open(SHM_NAME_RES, O_CREAT | O_RDWR, 0777);
    if (shm_fd_res == -1)
    {
        perror("shm_open response");
        exit(EXIT_FAILURE);
    }


    // Map shared memory for request
    request_t* request = mmap(NULL, sizeof(request_t)*NUM_CACHE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd_req, 0);
    if (request == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


    response_t* response = mmap(NULL, sizeof(response_t)*NUM_CACHE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd_res, 0);
    if (response == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


    int tid = 0;
    int i;
    for (i = 0; i < NUM_CACHE; i++)
    {
        if(atomic_load(&request[i].free) && request[i].mode==SERVER_READY){
            atomic_store(&request[i].free, false);
            request[i].operation = string_to_enum(argv[1]);
            request[i].key = atoi(argv[2]);
            
            if (request[i].operation == INSERT) {
                strncpy(request[i].value, argv[3], MAX_VALUE_SIZE - 1); // Copy the value to request
                request[i].value[MAX_VALUE_SIZE - 1] = '\0'; // Ensure null termination
            } else {
                // For other operations, set value to NULL
                request[i].value[0] = '\0';
            }
            request[i].mode = CLIENT_READY;

            break;
        }
        tid ++;
    }
    
    if(tid==NUM_CACHE){
        perror("max-req");
        exit(EXIT_FAILURE);
    }
    
    

    // Wait for the response
    bool response_received = false;
    while (!response_received) {

        if (check_flag(response[tid].flag) && response[tid].mode==SERVER_READY) {
            // printf("hiii %d %c\n", tid, response[tid].flag);
            response_received = true;
        } else {
            // Response not received yet, wait and retry
            usleep(100000); // Wait for 100 milliseconds before retrying
        }
    }
    

    switch (response[tid].flag) {
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
        printf("Received data: %s\n", response[tid].value);
        break;

    case NOT_FOUND:
        printf("Data not found\n");
        break;

    default:
        printf("Unknown error\n");
        break;
    }

    
    response[tid].flag = DEFAULT;
    response[tid].mode = CLIENT_READY;

    // Close shared memory
    // munmap(request, sizeof(request_t)*NU);
    // munmap(response, sizeof(response_t));
    close(shm_fd_req);
    close(shm_fd_res);

    return 0;
}
