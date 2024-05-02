#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_N 100


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



    for (int i = 0; i < N; i++) {
        int key = i;
        char insert_cmd[50];
        snprintf(insert_cmd, sizeof(insert_cmd), "./client insert %d test-%d &", key, key);
        system(insert_cmd);
    }


    usleep(100000); // Wait for 100 milliseconds


    // Create threads for search
    for (int i = 0; i < N; i++) {
        int key = i;
        char search_cmd[50];
        snprintf(search_cmd, sizeof(search_cmd), "./client search %d &", key);
        system(search_cmd);
    }


    return 0;
}


