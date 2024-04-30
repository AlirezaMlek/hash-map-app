#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "common.h"



typedef struct Entry {
    int key;
    char value[MAX_VALUE_SIZE];
    struct Entry* next;
} Entry;

typedef struct {
    Entry** table;
    pthread_rwlock_t* rwlock;
} HashTable;

void init_hash_table(HashTable *hashTable, int size);
response_t insert_entity(HashTable *hashTable, int key, char* value);
response_t search_entity(HashTable *hashTable, int key);
response_t delete_entity(HashTable *hashTable, int key);
void cleanup_hash_table(HashTable *hashTable);

#endif /* HASH_TABLE_H */
