#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Global variable accessible by all threads
int shared_variable = 0;

// Thread function with no arguments
void *threadFunction(void *arg) {
    // Access the global variable
    shared_variable++;

    // Perform some work in the thread
    printf("Thread: Incremented shared_variable to %d\n", shared_variable);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Create threads
    if (pthread_create(&thread1, NULL, threadFunction, NULL) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread2, NULL, threadFunction, NULL) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("All threads have finished.\n");

    return 0;
}

