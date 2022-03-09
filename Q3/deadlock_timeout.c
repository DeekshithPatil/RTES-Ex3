#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define NUM_THREADS 2
#define THREAD_1 1
#define THREAD_2 2

typedef struct
{
    int threadIdx;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
pthread_t threads[NUM_THREADS];
struct sched_param nrt_param;

pthread_mutex_t rsrcA, rsrcB;

volatile int rsrcACnt=0, rsrcBCnt=0, noWait=0;


void *grabRsrcs(void *threadp)
{
   struct timespec timeNow;
   struct timespec rsrcA_timeout;
   struct timespec rsrcB_timeout;
   int rc;
   threadParams_t *threadParams = (threadParams_t *)threadp;
   int threadIdx = threadParams->threadIdx;

   if(threadIdx == THREAD_1) printf("Thread 1 started\n");
   else if(threadIdx == THREAD_2) printf("Thread 2 started\n");
   else printf("Unknown thread started\n");

   int timeout1, timeout2;
   time_t t;
   
   srand((unsigned)time(&t));
   timeout1 = rand() % 5;
   timeout2 = rand() % 5;

   if(threadIdx == THREAD_1)
   {
     if((rc=pthread_mutex_lock(&rsrcA)) != 0)
     {
         printf("Thread 1 ERROR\n");
         pthread_exit(NULL);
     }
     else
     {
         printf("Thread 1 GOT A\n");
     }

     // if unsafe test, immediately try to acquire rsrcB
     if(!noWait) usleep(1000000);

     printf("Thread 1 random value = %d\n",timeout1);
     clock_gettime(CLOCK_REALTIME, &timeNow);
     rsrcB_timeout.tv_sec = timeNow.tv_sec + timeout1;
     rsrcB_timeout.tv_nsec = timeNow.tv_nsec;

     rc=pthread_mutex_timedlock(&rsrcB, &rsrcB_timeout);

     if(rc == 0)
     {
         clock_gettime(CLOCK_REALTIME, &timeNow);
         printf("Thread 1 GOT B\n");
     }
     else if(rc == ETIMEDOUT)
     {
         printf("Thread 1 TIMEOUT ERROR\n");
         pthread_mutex_unlock(&rsrcA);
         pthread_mutex_lock(&rsrcB);
         pthread_mutex_lock(&rsrcA);
         printf("THREAD 1 got A\n");
     }
     else
     {
         printf("Thread 1 ERROR\n");
         pthread_mutex_unlock(&rsrcA);
         pthread_exit(NULL);
     }

     printf("THREAD 1 got A and B\n");
     pthread_mutex_unlock(&rsrcB);
     pthread_mutex_unlock(&rsrcA);
     printf("THREAD 1 done\n");
   }

   else
   {
     if((rc=pthread_mutex_lock(&rsrcB)) != 0)
     {
         printf("Thread 2 ERROR\n");
         pthread_exit(NULL);
     }
     else
     {
         printf("Thread 2 GOT B\n");
     }

     // if unsafe test, immediately try to acquire rsrcB
     if(!noWait) usleep(1000000);
     
     printf("Thread 2 random value = %d\n",timeout2);
     clock_gettime(CLOCK_REALTIME, &timeNow);
     rsrcA_timeout.tv_sec = timeNow.tv_sec + timeout2;
     rsrcA_timeout.tv_nsec = timeNow.tv_nsec;

     rc=pthread_mutex_timedlock(&rsrcA, &rsrcA_timeout);

     if(rc == 0)
     {
         printf("Thread 2 GOT A\n");
     }
     else if(rc == ETIMEDOUT)
     {
         printf("Thread 2 TIMEOUT ERROR\n");
         pthread_mutex_unlock(&rsrcB);
         pthread_mutex_lock(&rsrcA);
         pthread_mutex_lock(&rsrcB);
         printf("THREAD 2 got A\n");
     }
     else
     {
         printf("Thread 2 ERROR\n");
         pthread_mutex_unlock(&rsrcB);
         pthread_exit(NULL);
     }

     printf("THREAD 2 got B and A\n");
     pthread_mutex_unlock(&rsrcA);
     pthread_mutex_unlock(&rsrcB);
     printf("THREAD 2 done\n");
   }
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   int rc, safe=0;

   rsrcACnt=0, rsrcBCnt=0, noWait=0;

   if(argc < 2)
   {
    //  printf("Will set up unsafe deadlock scenario\n");
   }
   else if(argc == 2)
   {
     if(strncmp("safe", argv[1], 4) == 0)
       safe=1;
     else if(strncmp("race", argv[1], 4) == 0)
       noWait=1;
     else
       printf("Will set up unsafe deadlock scenario\n");
   }
   else
   {
     printf("Usage: deadlock [safe|race|unsafe]\n");
   }

   // Set default protocol for mutex which is unlocked to start
   pthread_mutex_init(&rsrcA, NULL);
   pthread_mutex_init(&rsrcB, NULL);

  //  printf("Creating thread %d\n", THREAD_1);
   threadParams[THREAD_1].threadIdx=THREAD_1;
   rc = pthread_create(&threads[0], NULL, grabRsrcs, (void *)&threadParams[THREAD_1]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}

   if(safe) // Make sure Thread 1 finishes with both resources first
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1 joined to main\n");
     else
       perror("Thread 1");
   }

  //  printf("Creating thread %d\n", THREAD_2);
   threadParams[THREAD_2].threadIdx=THREAD_2;
   rc = pthread_create(&threads[1], NULL, grabRsrcs, (void *)&threadParams[THREAD_2]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}

  //  printf("will try to join both CS threads unless they deadlock\n");

   if(!safe)
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1 joined to main\n");
     else
       perror("Thread 1");
   }

   if(pthread_join(threads[1], NULL) == 0)
     printf("Thread 2 joined to main\n");
   else
     perror("Thread 2");

   if(pthread_mutex_destroy(&rsrcA) != 0)
     perror("mutex A destroy");

   if(pthread_mutex_destroy(&rsrcB) != 0)
     perror("mutex B destroy");

   printf("All done\n");

   exit(0);
}
