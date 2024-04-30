#include "hash-table.h"



#define NUM_THREADS 4
#define ENTRIES_PER_THREAD 1000

void* insertEntries(void* arg) {
    HashTable* hashTable = (HashTable*)arg;
    
    for (int i = 0; i < ENTRIES_PER_THREAD; ++i) {
        // Generate unique keys for each thread
        int key = rand() % (ENTRIES_PER_THREAD * NUM_THREADS);
        char *value = "test";
        
        insert_entity(hashTable, key, value);
    }
    
    pthread_exit(NULL);
}


int countEntries(HashTable *hashTable, int TABLE_SIZE) {
    int count = 0;
    for (int i = 0; i < TABLE_SIZE; ++i) {
        pthread_rwlock_rdlock(&hashTable->rwlock[i]);
        Entry* current = hashTable->table[i];
        while (current != NULL) {
            count++;
            current = current->next;
        }
        pthread_rwlock_unlock(&hashTable->rwlock[i]);
    }
    return count;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <size_of_hash_table>\n", argv[0]);
        return 1;
    }

    int size_of_hash_table = atoi(argv[1]);
    if (size_of_hash_table <= 0) {
        printf("Invalid size of hash table\n");
        return 1;
    }
    
    HashTable hashTable;
    init_hash_table(&hashTable, size_of_hash_table); // Pass size_of_hash_table to initHashTable

    pthread_t threads[NUM_THREADS];
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, insertEntries, (void*)&hashTable)) {
            perror("pthread_create failed");
            return 1;
        }
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(threads[i], NULL)) {
            perror("pthread_join failed");
            return 1;
        }
    }

    printf("Number of records in the hash table: %d\n", countEntries(&hashTable, size_of_hash_table));


    cleanup_hash_table(&hashTable);
    
    return 0;
}