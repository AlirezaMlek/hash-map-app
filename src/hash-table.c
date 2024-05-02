#include "hash-table.h"

int TABLE_SIZE;



int hash(int key) {
    return key % TABLE_SIZE;
}




void init_hash_table(HashTable *hashTable, int size) {
    TABLE_SIZE = size; // Set TABLE_SIZE to the provided size
    
    hashTable->table = (Entry**)malloc(TABLE_SIZE * sizeof(Entry*));
    if (hashTable->table == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    hashTable->rwlock = (pthread_rwlock_t*)malloc(TABLE_SIZE * sizeof(pthread_rwlock_t));
    if (hashTable->rwlock == NULL) {
        perror("Memory allocation failed");
        free(hashTable->table); // Free previously allocated memory
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < TABLE_SIZE; ++i) {
        hashTable->table[i] = NULL;
        pthread_rwlock_init(&hashTable->rwlock[i], NULL);
    }
}







response_t insert_entity(HashTable *hashTable, int key, char* value) {
    int index = hash(key);
    response_t response;
    
    pthread_rwlock_wrlock(&hashTable->rwlock[index]);
    
    Entry* newEntry = (Entry*)malloc(sizeof(Entry));
    if (!newEntry) {
        perror("Memory allocation failed");
        response.flag = ADDED_ABORT;
        pthread_rwlock_unlock(&hashTable->rwlock[index]);
        return response;
    }
    newEntry->key = key;
    strncpy(newEntry->value, value, MAX_VALUE_SIZE - 1);
    newEntry->value[MAX_VALUE_SIZE - 1] = '\0';
    newEntry->next = NULL;
    
    if (hashTable->table[index] == NULL) {
        hashTable->table[index] = newEntry;
    } else {
        Entry* current = hashTable->table[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newEntry;
    }
    
    pthread_rwlock_unlock(&hashTable->rwlock[index]);

    response.flag = ADDED_SUCCESS;
    return response;


}





response_t search_entity(HashTable *hashTable, int key) {
    int index = hash(key);

    response_t response;
    
    pthread_rwlock_rdlock(&hashTable->rwlock[index]);
    
    Entry* current = hashTable->table[index];
    while (current != NULL) {
        if (current->key == key) {
            pthread_rwlock_unlock(&hashTable->rwlock[index]);
            strncpy(response.value, current->value, MAX_VALUE_SIZE - 1); // Copy the value to request
            
            response.value[MAX_VALUE_SIZE - 1] = '\0';
            response.flag = FOUND;
            return response;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(&hashTable->rwlock[index]);

    response.flag = NOT_FOUND;
    return response;
}



response_t delete_entity(HashTable *hashTable, int key) {
    int index = hash(key);

    response_t response;
    
    pthread_rwlock_wrlock(&hashTable->rwlock[index]);
    
    Entry* current = hashTable->table[index];
    Entry* prev = NULL;
    while (current != NULL) {
        if (current->key == key) {
            if (prev == NULL) {
                hashTable->table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            pthread_rwlock_unlock(&hashTable->rwlock[index]);
            response.flag = DELETE_SUCCESS;
            return response;
        }
        prev = current;
        current = current->next;
    }
    

    response.flag = DELETE_ABORT;
    return response;

    pthread_rwlock_unlock(&hashTable->rwlock[index]);
}




void cleanup_hash_table(HashTable *hashTable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        pthread_rwlock_destroy(&hashTable->rwlock[i]);
        Entry* current = hashTable->table[i];
        while (current != NULL) {
            Entry* temp = current;
            current = current->next;
            free(temp);
        }
    }
}



