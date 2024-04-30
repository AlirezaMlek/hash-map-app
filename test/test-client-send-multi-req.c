#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_N 100

// Function to execute insert command
void *execute_insert(void *arg) {
    int key = *((int *)arg);
    char insert_cmd[50];
    snprintf(insert_cmd, sizeof(insert_cmd), "./client insert %d test-%d", key, key);
    system(insert_cmd);
    pthread_exit(NULL);
}

// Function to execute search command
void *execute_search(void *arg) {
    int key = *((int *)arg);
    char search_cmd[50];
    snprintf(search_cmd, sizeof(search_cmd), "./client search %d", key);
    system(search_cmd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    if (N < 1 || N > MAX_N) {
        printf("Invalid value for N\n");
        exit(EXIT_FAILURE);
    }

    // Create threads for insertions
    pthread_t insert_threads[MAX_N];
    int insert_keys[MAX_N];
    for (int i = 0; i < N; i++) {
        insert_keys[i] = i;
        if (pthread_create(&insert_threads[i], NULL, execute_insert, &insert_keys[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Create threads for searches
    pthread_t search_threads[MAX_N];
    int search_keys[MAX_N];
    for (int i = 0; i < N; i++) {
        search_keys[i] = i;
        if (pthread_create(&search_threads[i], NULL, execute_search, &search_keys[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all insert threads to complete
    for (int i = 0; i < N; i++) {
        if (pthread_join(insert_threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all search threads to complete
    for (int i = 0; i < N; i++) {
        if (pthread_join(search_threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
