#ifndef COMMON_H
#define COMMON_H
#include <stdatomic.h>


#define MAX_VALUE_SIZE 100

#define NUM_CACHE 10 // Change it to increase maximum parallel processes

#define SHM_NAME_REQ "/req_shm"
#define SHM_NAME_RES "/res_shm"

enum RequestType {
    INSERT = 'I',
    DELETE = 'D',
    SEARCH = 'S',
};



enum ResponseType {
    ADDED_SUCCESS = 'a',
    ADDED_ABORT = 'b',
    DELETE_SUCCESS = 'c',
    DELETE_ABORT = 'd',
    FOUND = 'e',
    NOT_FOUND = 'f',
    ERROR = 'g',
    DEFAULT =  'x'
};


enum DateMode{
    SERVER_READY = 'A',
    CLIENT_READY = 'B',
};



typedef struct { // Request structure
    char operation;
    char mode;
    int key;
    char value[MAX_VALUE_SIZE];
    atomic_bool free;
} request_t;

typedef struct { // Response structure
    char flag;
    char mode;
    char value[MAX_VALUE_SIZE];
} response_t;



#endif // COMMON_H