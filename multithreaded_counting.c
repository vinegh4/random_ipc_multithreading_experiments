#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

#define COUNT_TO 10000
#define NUM_THREADS 8

uint32_t i = 0;
pthread_mutex_t mutex;

void *count(void *arg) {
    for(;;) {
        pthread_mutex_lock(&mutex);
        if(i >= COUNT_TO) {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        i++;
        printf("%u\n", i);
        pthread_mutex_unlock(&mutex);
        //sleep to simulate each thraed doing some additional non-critical work
        usleep(50);
    }
}

int main() {
    pthread_t *threads = malloc(sizeof(pthread_t)*NUM_THREADS);
    pthread_mutex_init(&mutex, NULL);
    
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, count, NULL);
    }

    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    } 

    return 0;
}