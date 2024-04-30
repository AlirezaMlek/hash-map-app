#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hash-table.h"
#include "common.h"

#define SHM_NAME "./app_shm"

HashTable hash_table;

typedef struct {
    pthread_rwlock_t lock;
} shared_data_t;

shared_data_t* shared_data;





void generate_response(int shm_fd, request_t* request, response_t response){
    response_t* shm_response = mmap(NULL, sizeof(response_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_response == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    *shm_response = response;

    // Unmap shared memory for response
    munmap(shm_response, sizeof(response_t));

    // Unmap shared memory for request
    munmap(request, sizeof(request_t)); 
}




void* server_thread(void* arg) {
    int shm_fd = *(int*)arg;
    response_t response;

    // Server logic
    while (1) {
        // Read from shared memory
        request_t *request = mmap(NULL, sizeof(request_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        if (request == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        

        switch (request->operation) {
            case INSERT:
                printf("New insert operation\n");
                pthread_rwlock_wrlock(&shared_data->lock);
                response = insert_entity(&hash_table, request->key, request->value);
                generate_response(shm_fd, request, response);
                pthread_rwlock_unlock(&shared_data->lock);
                break;

            case DELETE:
                printf("New delete operation\n");
                pthread_rwlock_wrlock(&shared_data->lock);
                response = delete_entity(&hash_table, request->key);
                generate_response(shm_fd, request, response);
                pthread_rwlock_unlock(&shared_data->lock);
                break;

            case SEARCH:
                printf("New search operation\n");
                pthread_rwlock_wrlock(&shared_data->lock);
                response = search_entity(&hash_table, request->key);
                generate_response(shm_fd, request, response);
                pthread_rwlock_unlock(&shared_data->lock);
                break;

            default:
                break;
        }

        
        usleep(100000); // Wait for 100 milliseconds before retrying

    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <hash_table_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Initialize shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data_t));
    shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Initialize hash table
    size_t table_size = atoi(argv[1]);
    init_hash_table(&hash_table, table_size);

    // Initialize readers-writer lock
    pthread_rwlock_init(&shared_data->lock, NULL);

    // Create server thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, &shm_fd);

    // Wait for the server thread to finish (which should never happen in this case)
    pthread_join(server_tid, NULL);

    // Cleanup
    pthread_rwlock_destroy(&shared_data->lock);
    cleanup_hash_table(&hash_table);
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    return 0;
}
