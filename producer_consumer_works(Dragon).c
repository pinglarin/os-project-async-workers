#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <time.h> 

#include "common.h"
#include "common_threads.h"

#ifdef linux
#include <semaphore.h>
#elif __APPLE__
#include "zemaphore.h"
#endif

int max;
int loops; // N of producer
int *buffer;

int use  = 0;
int fill = 0;

sem_t empty;
sem_t full;
sem_t mutex;



#define CMAX (10)
int consumers = 1;

//Use to set value in buffer
void do_fill(int value) {
    buffer[fill] = value;
    fill++;
    if (fill == max) //When full go back to 0
	fill = 0;
}

//Used to bring value out from buffer
int do_get() {
    int tmp = buffer[use];
    use++;
    if (use == max)
	use = 0;
    return tmp;
}

void *producer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        Sem_wait(&empty); //When empty is Max  = 4 [-- empty]
        Sem_wait(&mutex); //Use Critical region to call request by change mutex = 0 [-- mutex]
        do_fill(i);//Create Request
        Sem_post(&mutex);// ++ mutex (use to exit Critical region)
        Sem_post(&full);// ++ full
    }

    // end case [To stop do_fill stop fucking loop in line 34]
    for (i = 0; i < consumers; i++) {
        Sem_wait(&empty);
        Sem_wait(&mutex);
        do_fill(-1); //When do-
        Sem_post(&mutex);
        Sem_post(&full);
    }

    return NULL;
}
                                                                               
void *consumer(void *arg) {
    int tmp = 0; //Copy value from buffer
    while (tmp != -1) {
        Sem_wait(&full);//When full is Max  = 4 [-- full]
        Sem_wait(&mutex);//Use Critical region to call request by change mutex = 0 [-- mutex]
        tmp = do_get(); //Send out value in buffer
        Sem_post(&mutex);// ++ mutex (use to exit Critical region)
        Sem_post(&empty);// ++ empty
        printf("%lld %d\n", (long long int) arg, tmp);//Try to print
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // if (argc != 4) { //argc  = N useable argc + 1 
    //     fprintf(stderr, "usage: %s <buffersize> <loops> <consumers>\n", argv[0]);
    //     exit(1);
    // }
    // max   = atoi(argv[1]);
    // loops = atoi(argv[2]);
    // consumers = atoi(argv[3]);//Use 3 then + 1 on line 81

    int processes = 1;   
    int devices = 1;

    assert(consumers <= CMAX);

    buffer = (int *) malloc(max * sizeof(int));
    assert(buffer != NULL);
    int i;
    //Use to defalut data
    for (i = 0; i < max; i++) {
        buffer[i] = 0;
    }


    //Set up value
    Sem_init(&empty, max); // max are empty 
    Sem_init(&full, 0);    // 0 are full
    Sem_init(&mutex, 1);   // mutex (Critical region)


    //Keep pthred value on Customer and producer
    pthread_t pid, cid[CMAX];
    int Max = 500, Min = 100; Count = 5;

    for (int i = 0; i < count; i++)
    {
      int num = (rand() % (upper - lower + 1)) + lower;
      delay(num);
      Pthread_create(&pid, NULL, producer, NULL);       
    }
    
    for (i = 0; i < consumers; i++) {
        Pthread_create(&cid[i], NULL, consumer, (void *) (long long int) i); //Create Data
    }
    Pthread_join(pid, NULL); 
    for (i = 0; i < consumers; i++) {
	    Pthread_join(cid[i], NULL); 
    }
    return 0;
}

