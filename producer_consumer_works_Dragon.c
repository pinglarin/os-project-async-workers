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

int Max_DelayTime = 400;
int Min_DelayTime = 200;



#define CMAX (10)
//int consumers = 1;

//Use to set value in buffer
void do_fill(int value) {
    printf("Check do_fill\n");
    buffer[fill] = value;
    fill++;
    if (fill == max) //When full go back to 0
	fill = 0;
}

//Used to bring value out from buffer
int do_get() {
    printf("Check do_get\n");
    int tmp = buffer[use];
    use++;
    if (use == max)
	use = 0;
    return tmp;
}

void *producer(void *arg) {
    int i;
    int Max = 500, Min = 100, devices = 1;     
    for (i = 0; i < loops; i++) {

        printf("producer Checking round: %d \n",i);

        int num = (rand() % (Max - Min + 1)) + Min; // Random time range (100-500)
        usleep(num); //Like Delay but better


        Sem_wait(&empty); //When empty is Max  = 4 [-- empty]
        Sem_wait(&mutex); //Use Critical region to call request by change mutex = 0 [-- mutex]
        do_fill(i);//Create Request
        Sem_post(&mutex);// ++ mutex (use to exit Critical region)
        Sem_post(&full);// ++ full
    }

    // end case [To stop do_fill stop fucking loop in line 34]
    for (i = 0; i < devices; i++) {
        printf("Stopper\n");
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
    int i =0;
    while (tmp != -1) {

        printf("consumer Checking round: %d \n",i);
        int num = (rand() % (Max_DelayTime - Min_DelayTime + 1)) + Min_DelayTime; // Random time range (100-500)
        usleep(num); //Like Delay but better        
        Sem_wait(&full);//When full is Max  = 4 [-- full]
        Sem_wait(&mutex);//Use Critical region to call request by change mutex = 0 [-- mutex]
        tmp = do_get(); //Send out value in buffer
        Sem_post(&mutex);// ++ mutex (use to exit Critical region)
        Sem_post(&empty);// ++ empty
        printf("%lld %d\n", (long long int) arg, tmp);//Try to print
        i++;
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

    

    assert(devices<= CMAX);

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
    pthread_t pid[CMAX], cid[CMAX];
      

    for (int i = 0; i < processes; i++)//Added
    {     
      Pthread_create(&pid[i], NULL, producer, NULL);       
    }
    
    for (i = 0; i < devices; i++) {
        Pthread_create(&cid[i], NULL, consumer, (void *) (long long int) i); //Create Data
    }

    for (int i = 0; i < processes; i++)//Added
    {     
      Pthread_join(pid[i], NULL);       
    }
    
    for (i = 0; i < devices; i++) {
	    Pthread_join(cid[i], NULL); 
    }
    return 0;
}

//gcc -Wall -Werror -I../include -pthread -o out producer_consumer_works_Dragon.c
//./out
//cd Desktop/OS_project/os-project-async-workers