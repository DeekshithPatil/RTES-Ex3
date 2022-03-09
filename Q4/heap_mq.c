/****************************************************************************/
/*                                                                          */
/* Sam Siewert - 10/14/97                                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
                                                                    
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>   
#include <time.h>
#include <sys/time.h>

#define ERROR -1

#define SNDRCV_MQ "/send_receive_mq"

struct mq_attr mq_attr;
static mqd_t mymq;

/* receives pointer to heap, reads it, and deallocate heap memory */

void * receiver(void *arg)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr; 
  int prio;
  int nbytes;
  int count = 0;
  int id;
  int itr=0;
  while(1) {

    /* read oldest, highest priority msg from the message queue */

    printf("Reading %ld bytes\n", sizeof(void *));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == ERROR)
/*
    if((nbytes = mq_receive(mymq, (void *)&buffptr, (size_t)sizeof(void *), &prio)) == ERROR)
*/
    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("itr = %d, receive: ptr msg 0x%X received with priority = %d, length = %d, id = %d\n", itr,buffptr, prio, nbytes, id);

      printf("contents of ptr = \n%s\n", (char *)buffptr);

      free(buffptr);

      printf("heap space memory freed\n");

    }

    itr++;

    if(itr == 5)
    {
      break;
    }
    
  }

}


static char imagebuff[4096];

void * sender(void * arg)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;
  int itr=0;

  while(1) {

    /* send malloc'd message with priority=30 */

    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("Message to send = %s\n", (char *)buffptr);

    printf("Sending %ld bytes\n", sizeof(buffptr));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == ERROR)
    {
      perror("mq_send");
    }
    else
    {
      printf("send: message ptr 0x%X successfully sent\n", buffptr);
    }

    sleep(3);

    itr++;

    if(itr == 5)
    {
      break;
    }

  }
  
}


static int sid, rid;

void main(void)
{
  pthread_t send_thread, recieve_thread;
  pthread_attr_t send_thread_attr, recieve_thread_attr;

  struct sched_param rt_param;

  int i, j;
  char pixel = 'A';

  for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);

  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  /* receiver runs at a higher priority than the sender */
//   if((rid=taskSpawn("Receiver", 90, 0, 4000, receiver, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == ERROR) {
//     printf("Receiver task spawn failed\n");
//   }
//   else
//     printf("Receiver task spawned\n");

//   if((sid=taskSpawn("Sender", 100, 0, 4000, sender, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == ERROR) {
//     printf("Sender task spawn failed\n");
//   }
//   else
//     printf("Sender task spawned\n");

  pthread_attr_init(&recieve_thread_attr);
  pthread_attr_setinheritsched(&recieve_thread_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&recieve_thread_attr, SCHED_FIFO);

  pthread_attr_init(&send_thread_attr);
  pthread_attr_setinheritsched(&send_thread_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&send_thread_attr, SCHED_FIFO);

  int rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  int rt_min_prio = sched_get_priority_min(SCHED_FIFO);


  rt_param.sched_priority = rt_max_prio - 10;
  pthread_attr_setschedparam(&recieve_thread_attr, &rt_param);

  int ret = pthread_create(&recieve_thread,&recieve_thread_attr,receiver,NULL);

  if(ret !=0)
  {
    perror("pthread_create()");
  }

  rt_param.sched_priority = rt_max_prio - 20;
  pthread_attr_setschedparam(&send_thread_attr, &rt_param);

  ret = pthread_create(&send_thread,&send_thread_attr,sender,NULL);

  if(ret !=0)
  {
    perror("pthread_create()");
  }

  void * res;

  pthread_join(recieve_thread,&res);
  pthread_join(send_thread,&res);

  mq_close(mymq);
  mq_unlink(SNDRCV_MQ);

  printf("Program Terminating!\n");
}
