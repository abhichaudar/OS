#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define MAX_BUFFER_SIZE 10
int buffer[MAX_BUFFER_SIZE];
int buffer_size = 0;
sem_t empty, full;
pthread_mutex_t mutex;
int num_producers, num_consumers;
void *producer(void *arg) {
    int item;
    for (int i = 0; i < 5; i++) { 
        item = rand(); 
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        buffer[buffer_size++] = item;
        printf("Producer %ld produced item %d. Buffer size: %d\n", (long)arg, item, buffer_size);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    pthread_exit(NULL);
}
void *consumer(void *arg) {
    int item;
    for (int i = 0; i < 5; i++) { 
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        item = buffer[--buffer_size];
        printf("Consumer %ld consumed item %d. Buffer size: %d\n", (long)arg, item, buffer_size);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
    pthread_exit(NULL);
}
int main() {
    printf("Enter the number of producers: ");
    scanf("%d", &num_producers);
    printf("Enter the number of consumers: ");
    scanf("%d", &num_consumers);
    sem_init(&empty, 0, MAX_BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    pthread_t producer_threads[num_producers];
    pthread_t consumer_threads[num_consumers];
    for (long i = 0; i < num_producers; i++)
        pthread_create(&producer_threads[i], NULL, producer, (void *)i);
    for (long i = 0; i < num_consumers; i++)
        pthread_create(&consumer_threads[i], NULL, consumer, (void *)i);    
    for (int i = 0; i < num_producers; i++)
        pthread_join(producer_threads[i], NULL);    
    for (int i = 0; i < num_consumers; i++)
        pthread_join(consumer_threads[i], NULL);    
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);    
    return 0;
}

