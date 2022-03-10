#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
// #include <windows.h>

// #define MAX_THREADS 5
#define BUFFER_SIZE 10

sem_t *empty, *full; //, *mutex;
pthread_mutex_t mutex;

double secs = 0;

int buffer[BUFFER_SIZE];
int in = 0, out = 0;
int nHead = 0;

static volatile int keepRunning = 1;
int nprocess, ndevice, nrequest, mintime, maxtime, currRequest = 0, finRequest = 0, producerType;

void intHandler(int dummy) {
    keepRunning = 0;
}

typedef struct node {
    int val;
    struct node *next;
} node_t;

node_t *head = NULL;
node_t *timeArrive = NULL;
node_t *timeStart = NULL;

int ret1, ret2;

void enqueue(node_t **head, int val);
int dequeue(node_t **head);
void print_list(node_t *head, char c);
void sleep_ms(int milliseconds);
int random_int(int min, int max);


void *producer_wait(void * id_ptr) {
    int ID = *((int *) id_ptr);
    static int nextProduced = 0;
    struct timeval start;
    
    while (currRequest < nrequest) {
        printf("%d %d %d\n", ID, currRequest, nrequest);
        currRequest++;
        sem_wait(empty);
        // (void) sem_wait(mutex);
        pthread_mutex_lock(&mutex);


       /* Check to see if Overwriting unread slot */
        if (buffer[in] != -1) {
            fprintf(stderr, "Synchronization Error: Producer %d Just overwrote %d from Slot %d\n", ID, buffer[in], in);
            exit(1);
        }

        
        
        sleep_ms(random_int(100, 500));
        gettimeofday(&start, NULL);
        
        nextProduced++; // Producing Integers
        
        enqueue(&head, nextProduced);
        enqueue(&timeArrive, start.tv_sec);
        nHead++;
        buffer[in] = nextProduced;
        printf("*%d %d %d %d %d\n", ID, currRequest, nrequest, buffer[in], nextProduced);
        /* Looks like we are OK */
        
        // printf("ID: %d, curr: %d, buffer: %d\n", ID, currRequest, buffer[in]);
        // printf("queue: %d\n", nHead);
        printf("Process %d has issued a request %d at slot %d, start: %ld\n", ID, nextProduced, in, start.tv_sec);
        in = (in + 1) % BUFFER_SIZE;
        
        // printf("incremented in!\n");

        pthread_mutex_unlock(&mutex);
        
        // (void) sem_post(mutex);
        sem_post(full);
    }

    return NULL;
}

void *producer_drop(void * id_ptr) {
    int ID = *((int *) id_ptr);
    static int nextProduced = 0;
    struct timeval start;
    
    while (currRequest < nrequest) {
        // printf("%d %d %d\n", ID, currRequest, nrequest);
        currRequest++;
        sem_wait(empty);
        // (void) sem_wait(mutex);
        pthread_mutex_lock(&mutex);


       /* Check to see if Overwriting unread slot */
        if (buffer[in] != -1) {
            fprintf(stderr, "Synchronization Error: Producer %d Just overwrote %d from Slot %d\n", ID, buffer[in], in);
            exit(1);
        }

        
        
        sleep_ms(random_int(100, 500));
        gettimeofday(&start, NULL);
        
        nextProduced++; // Producing Integers
        
        enqueue(&head, nextProduced);
        enqueue(&timeArrive, start.tv_sec);
        nHead++;
        // printf("*%d %d %d\n", ID, currRequest, nrequest);
        /* Looks like we are OK */
        buffer[in] = nextProduced;
        printf("queue: %d\n", nHead);
        printf("Process %d has issued a request %d at slot %d, start: %ld\n", ID, nextProduced, in, start.tv_sec);
        in = (in + 1) % BUFFER_SIZE;
        
        // printf("incremented in!\n");

        pthread_mutex_unlock(&mutex);
        
        // (void) sem_post(mutex);
        sem_post(full);
    }

    return NULL;
}

void *producer_replace(void * id_ptr) {
    int ID = *((int *) id_ptr);
    static int nextProduced = 0;
    struct timeval start;
    
    while (currRequest < nrequest) {
        // printf("%d %d %d\n", ID, currRequest, nrequest);
        currRequest++;
        sem_wait(empty);
        // (void) sem_wait(mutex);
        pthread_mutex_lock(&mutex);


       /* Check to see if Overwriting unread slot */
        if (buffer[in] != -1) {
            fprintf(stderr, "Synchronization Error: Producer %d Just overwrote %d from Slot %d\n", ID, buffer[in], in);
            exit(1);
        }

        
        
        sleep_ms(random_int(100, 500));
        gettimeofday(&start, NULL);
        
        nextProduced++; // Producing Integers'

        // dequeue(&head);
        // enqueue(&head, nextProduced)
        
        enqueue(&head, nextProduced);
        enqueue(&timeArrive, start.tv_sec);
        nHead++;
        // printf("*%d %d %d\n", ID, currRequest, nrequest);
        /* Looks like we are OK */
        buffer[in] = nextProduced;
        printf("queue: %d\n", nHead);
        printf("Process %d has issued a request %d at slot %d, start: %ld\n", ID, nextProduced, in, start.tv_sec);
        in = (in + 1) % BUFFER_SIZE;
        
        // printf("incremented in!\n");

        pthread_mutex_unlock(&mutex);
        
        // (void) sem_post(mutex);
        sem_post(full);
    }

    return NULL;
}

void *consumer (void *id_ptr) {
    int ID = *((int *) id_ptr);
    static int nextConsumed = 0;
    struct timeval stop;
    
    while (finRequest < nrequest) {
        printf("\t%d %d %d %d\n", ID, finRequest, nrequest, buffer[ID]);
        finRequest++;
        if (buffer[ID] == 0) {
            break;
        }
        sem_wait(full);
        // (void) sem_wait(mutex);

        pthread_mutex_lock(&mutex);

        

        gettimeofday(&stop, NULL);
        enqueue(&timeStart, stop.tv_sec);
        nextConsumed = buffer[out];
        sleep_ms(random_int(mintime, maxtime));

        // dequeue(&head, nextConsumed);

        /* Check to make sure we did not read from an empty slot */
        if (nextConsumed == -1) {
            fprintf(stderr, "Synch Error: Consumer %d Just Read from empty slot %d\n", ID, out);
            exit(1);
        }

        /* We must be OK */
        // printf("queue: %d\n", nHead);
        printf("\tDevice %d Just finished request %d from slot %d, stop: %ld\n", ID, nextConsumed, out, stop.tv_sec);
        buffer[out] = -1;
        out = (out + 1) % BUFFER_SIZE;
        
        // printf("incremented out!\n");

        pthread_mutex_unlock(&mutex);

        // (void) sem_post(mutex);
        sem_post(empty);
    }

    return NULL;
}

int random_int(int min, int max)
{
   return min + rand() % (max+1 - min);
}

void sleep_ms(int milliseconds){ // cross-platform sleep function
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}

int main() {
    // printf("A");
    scanf("%d %d %d %d %d %d", &nprocess, &ndevice, &nrequest, &mintime, &maxtime, &producerType);
    float timeWait[nrequest];
    int MAX_THREADS = nprocess + ndevice;
    int ID[MAX_THREADS];
    pthread_t TID[MAX_THREADS];

    clock_t t;
    t = clock();

    empty = sem_open("/empty", O_CREAT, 0644, BUFFER_SIZE);
    full = sem_open("/full", O_CREAT, 0644, 0);
    // mutex = sem_open("/mutex", O_CREAT, 0644, 1);
    pthread_mutex_init(&mutex,NULL);

    signal(SIGINT, intHandler);

    for (int i = 0; i < MAX_THREADS; i++) {
        ID[i] = i;
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = -1;
    }
    if (producerType == 1){
        for (int i = 0; i < nprocess; i++)
        {
            pthread_create(&TID[i], NULL, producer_wait, (void *) &ID[i]);
            printf("Process ID = %d created!\n", i);
        }
    }
    else if (producerType == 2){
        for (int i = 0; i < nprocess; i++)
        {
            pthread_create(&TID[i], NULL, producer_drop, (void *) &ID[i]);
            printf("Process ID = %d created!\n", i);
        }
    }
    else if (producerType == 3){
        for (int i = 0; i < nprocess; i++)
        {
            pthread_create(&TID[i], NULL, producer_replace, (void *) &ID[i]);
            printf("Process ID = %d created!\n", i);
        }
    }
    // int k = nprocess;
    // printf("A: device: %d", ndevice);
    for (int i = nprocess; i < MAX_THREADS; i++)
    {
        pthread_create(&TID[i], NULL, consumer, (void *) &ID[i]);
        printf("Device ID = %d created!\n", i);
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(TID[i], NULL);
    }

    // print_list(timeArrive, 'a');
    // print_list(timeStart, 's'); 

    int k = 0;
    float sum = 0;
    while (k < nrequest && (ret1=dequeue(&timeArrive)) > 0 && (ret2=dequeue(&timeStart)) > 0) {
        timeWait[k] = ret2 - ret1;
        // printf("wait: %f\n", timeWait[k]);
        sum = sum + timeWait[k];
        k++;
    }
    float avg = sum/nrequest;
    printf("Average wait time: %f seconds\n", avg);


    sem_unlink("/empty");
    sem_unlink("/full");
    // (void) sem_unlink("/mutex");

    clock_t tEnd = clock();
    double time_taken = (tEnd-t)/1000.0;//((double)t)/CLOCKS_PER_SEC;
    // printf("The program took %llu milliseconds to execute", delta_us);
    printf("\nTotal elapsed time: %f seconds", time_taken); //totalStart.tv_usec-totalEnd.tv_usec
    

    // print_list(head, '-');
    // print_list(timeArrive, 'a');
    // print_list(timeStart, 's');

    return 0;
}

void enqueue(node_t **head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
    // nqueue++;
}

int dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);
    
    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    // nqueue--;
    return retval;
}

void print_list(node_t *head, char c) {
    node_t *current = head;

    while (current != NULL) {
        printf("%c %d\n", c, current->val);
        current = current->next;
    }
}

