
#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct QueueStruct {
	int read;				//read operate pointer
	int write;				//write operate pointer
	int length;
	void **data;	
	sem_t add;
	sem_t remov;
	pthread_mutex_t lock;
} Queue;


Queue *queue_alloc(int size) {
	Queue* queue = malloc(sizeof(Queue));
	queue->length = size;
	queue->read = 0;
	queue->write = 0;
	queue->data = malloc(sizeof(void*) * size);
	sem_init(&queue->add, 0, size);
	sem_init(&queue->remov, 0, 0);
	pthread_mutex_init(&queue->lock, NULL);
	
	return queue;
}

void queue_free(Queue *queue) {		//all allocated memory need to been free'd
	free(queue->data);
	free(queue);
}

void queue_put(Queue *queue, void *item) {
	sem_wait(&queue->add);					//wait for the adding signal
	pthread_mutex_lock(&queue->lock);		//mutex locked from here
	
	if (queue->write == 0){					//put the very first item into the right place
		queue->data[0] = item;
		queue->write = 1;					//move pointer
	}
	else{	
		queue->data[queue->write] = item;
		queue->write += 1;					//move write pointer
		queue->write %= queue->length;		//boundary check
	}
	
	pthread_mutex_unlock(&queue->lock);		//mutex unlocked here
	sem_post(&queue->remov);				
}



void *queue_get(Queue *queue) {
	sem_wait(&queue->remov);				//wait for the removing signal
	pthread_mutex_lock(&queue->lock);		//mutex locked from here
	
	void *buffer = queue->data[queue->read];//it won't miss position 0
	queue->read += 1;						//move read pointer
	queue->read %= queue->length;			//check boundary as well
	
	pthread_mutex_unlock(&queue->lock);		//mutex unlocked
	sem_post(&queue->add);
	
	return buffer;
}

