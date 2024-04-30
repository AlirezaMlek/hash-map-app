#ifndef COMMON_H
#define COMMON_H


#define MAX_VALUE_SIZE 100

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
};




typedef struct { // Request structure
    char operation;
    int key;
    char value[MAX_VALUE_SIZE];
} request_t;

typedef struct { // Response structure
    char flag;
    char value[MAX_VALUE_SIZE];
} response_t;



#endif // COMMON_H