#include "hash-table.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a hash table
    size_t size_of_hash_table = 100;
    HashTable hashTable;
    init_hash_table(&hashTable, size_of_hash_table);

    // Add an entry to the hash map
    int key = 123; 
    char* value = "test";
    insert_entity(&hashTable, key, value);

    printf("[OK] Data added. key: %d, value: %s\n", key, value);

    

    int search_key1 = 123;
    response_t result1 = search_entity(&hashTable, search_key1);
    if (result1.flag == FOUND) {
        printf("[OK] Data found for key %d: %s\n", search_key1, result1.value);
    } else if(result1.flag == NOT_FOUND){
        printf("[FAILED] Data not found with key %d\n", search_key1);
    } else {
        printf("[FAILED] Unkown error");
    }



    response_t result2 = delete_entity(&hashTable, search_key1);
    if (result2.flag == DELETE_SUCCESS) {
        printf("[OK] Data deleted. key %d\n", search_key1);
    } else if(result2.flag == DELETE_ABORT){
        printf("[FAILED] Cannot delete data with key %d\n", search_key1);
    } else {
        printf("[FAILED] Unkown error");
    }



    response_t result3 = search_entity(&hashTable, search_key1);
    if (result3.flag == FOUND) {
        printf("[FAILED] Value found: %s\n", result3.value);
    } else if(result3.flag == NOT_FOUND){
        printf("[OK] Data not found with key %d\n", search_key1);
    } else {
        printf("[FAILED] Unkown error");
    }



    // Cleanup: Destroy the hash table
    cleanup_hash_table(&hashTable);

    return 0;
}
