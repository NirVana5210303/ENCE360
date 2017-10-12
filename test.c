#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 4

sem_t read;
sem_t signal;

void *run( void *ptr )
{
  char **message = (char **)ptr;

  sem_wait(&read);
  
  while(*message) {
    printf("%s\n", *message);  
    sem_post(&signal);
////////////////////////////    
    sem_wait(&read);
////////////////////////////    
  }

  sem_post(&signal);

  pthread_exit(0);
}



int main()
{
  int i = 0;
  pthread_t thread[NUM_THREADS];
  char *message = "";
  
  // initialise semaphores:
////////////////////////////   
  sem_init(&read, 0, 0);
  sem_init(&signal, 0, 0);
////////////////////////////   
  
  // Create a bunch of threads to perform some withdrawals
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_create( &thread[i], NULL, run, &message);
  }
  
  char *messages[] = {"hello", "world", NULL};
  char **m = messages;
  
  do {
    message = *m++;
    printf("sending: %s\n", message);

	// enable all the threads to print
//////////////////////////// 
    for (i = 0; i < NUM_THREADS; ++i) {
      sem_post(&read);
    }
//////////////////////////// 

    for (i = 0; i < NUM_THREADS; ++i) {
      sem_wait(&signal);
    }

  } while(message);

    
	// wait for all the threads we created to finish:
////////////////////////////////
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join( thread[i], NULL);
  }
////////////////////////////////
  
  printf("done!\n");

  
	return 0;
}
