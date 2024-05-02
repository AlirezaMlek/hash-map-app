#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "hash-table.h"
#include "common.h"



HashTable hash_table;

typedef struct
{
    request_t* req;
    response_t* res;
} shared_data_t;


typedef struct
{
    shared_data_t cache;
    int tid;
} sever_thread_args_t;



request_t *shared_memory_request;
response_t *shared_memory_response;


void clear_memory(size_t size_shared_req, size_t size_shared_res, int shm_fd_req, int shm_fd_res)
{
    cleanup_hash_table(&hash_table);
    munmap(shared_memory_request, size_shared_req);
    close(shm_fd_req);
    shm_unlink(SHM_NAME_REQ);

    munmap(shared_memory_response, size_shared_res);
    close(shm_fd_res);
    shm_unlink(SHM_NAME_RES);
}



void generate_response(response_t response, int tid)
{
    strncpy(shared_memory_response[tid].value, response.value, MAX_VALUE_SIZE - 1);
    shared_memory_response[tid].flag = response.flag;
    shared_memory_response[tid].mode = SERVER_READY;

}


void *server_operation(int tid)
{

    response_t response;

    switch (shared_memory_request[tid].operation)
    {
    case INSERT:
        printf("New insert operation\n");
        response = insert_entity(&hash_table, shared_memory_request[tid].key, shared_memory_request[tid].value);
        generate_response(response, tid);
        break;

    case DELETE:
        printf("New delete operation\n");
        response = delete_entity(&hash_table, shared_memory_request[tid].key);
        generate_response(response, tid);
        break;

    case SEARCH:
        printf("New search operation\n");
        response = search_entity(&hash_table, shared_memory_request[tid].key);
        generate_response(response, tid);
        break;

    default:
        break;
    }

    return NULL;
}

void *server_thread(void *arg)
{

    int tid = *(int*) arg;

    // Server logic
    while (1)
    {
        usleep(100000); // Wait for 100 milliseconds before retrying

        response_t response;

        if (shared_memory_request[tid].mode == CLIENT_READY)
        {
            server_operation(tid);
            shared_memory_request[tid].mode = SERVER_READY;
            atomic_store(&shared_memory_request[tid].free, true);
        }
        
    }

    return NULL;
}





void init_cache_value()
{
    for (size_t i = 0; i < NUM_CACHE; i++)
    {
        shared_memory_request[i].operation = DEFAULT;
        shared_memory_request[i].mode = SERVER_READY;
        atomic_store(&shared_memory_request[i].free, true);

        shared_memory_response[i].flag = DEFAULT;
    }
}






void init_cache(int shm_fd_req, int shm_fd_res, size_t size_shared_req, size_t size_shared_res){
    // Resize the shared memory object
    if (ftruncate(shm_fd_req, size_shared_req) == -1)
    {
        clear_memory(size_shared_req, size_shared_res, shm_fd_req, shm_fd_res);
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd_res, size_shared_res) == -1)
    {
        clear_memory(size_shared_req, size_shared_res, shm_fd_req, shm_fd_res);
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }


    shared_memory_request = mmap(NULL, size_shared_req, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_req, 0);
    if (shared_memory_request == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    shared_memory_response = mmap(NULL, size_shared_res, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_res, 0);
    if (shared_memory_response == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


}






int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <hash_table_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }



    //------------- Initialize hash table
    size_t table_size = atoi(argv[1]);
    init_hash_table(&hash_table, table_size);



    //------------- Initialize shm

    int shm_fd_req;
    int shm_fd_res;

    // Size of the shared memory
    size_t size_shared_req = sizeof(request_t);
    size_shared_req *= NUM_CACHE;

    size_t size_shared_res = sizeof(response_t);
    size_shared_res *= NUM_CACHE;

    // Clear shared memory
    clear_memory(size_shared_req, size_shared_res, shm_fd_req, shm_fd_res);


    // Create or open the shared memory object
    shm_fd_req = shm_open(SHM_NAME_REQ, O_CREAT | O_RDWR, 0777);
    if (shm_fd_req == -1)
    {
        perror("shm_open sequest");
        exit(EXIT_FAILURE);
    }

    // Create or open the shared memory object
    shm_fd_res = shm_open(SHM_NAME_RES, O_CREAT | O_RDWR, 0777);
    if (shm_fd_res == -1)
    {
        perror("shm_open response");
        exit(EXIT_FAILURE);
    }
    

    init_cache(shm_fd_req, shm_fd_res, size_shared_req, size_shared_res);
    init_cache_value();





    //------------- Create server thread

    
    pthread_t server_tid[NUM_CACHE];

    int proc_id[NUM_CACHE];
    for (int i = 0; i < NUM_CACHE; i++)
    {
        proc_id[i] = i;
        pthread_create(&server_tid[i], NULL, server_thread, &proc_id[i]);
    }
    
    for (int i = 0; i < NUM_CACHE; i++)
    {
        pthread_join(server_tid[i], NULL);
    }



    //------------- Cleanup
    cleanup_hash_table(&hash_table);
    munmap(shared_memory_request, size_shared_req);
    close(shm_fd_req);
    shm_unlink(SHM_NAME_REQ);

    munmap(shared_memory_response, size_shared_res);
    close(shm_fd_res);
    shm_unlink(SHM_NAME_RES);

    return 0;
}
