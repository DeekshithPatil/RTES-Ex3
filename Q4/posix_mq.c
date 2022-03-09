/****************************************************************************/
/* Function: Basic POSIX message queue demo from VxWorks Prog. Guide p. 78  */
/*                                                                          */
/* Sam Siewert - 9/24/97                                                    */
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


#define ERROR     -1

#define SNDRCV_MQ "/send_receive_mq"
#define MAX_MSG_SIZE 128

struct mq_attr mq_attr;

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * receiver(void * param)
{
  mqd_t mymq;
  char buffer[MAX_MSG_SIZE];
  int prio;
  int nbytes;

  printf("Inside reciever!\n");

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, S_IRWXU, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  
  if((nbytes = mq_receive(mymq, buffer, MAX_MSG_SIZE, &prio)) == ERROR)
  {
    perror("mq_receive");
  }
  else
  {
    buffer[nbytes] = '\0';
    printf("receive: msg %s\nreceived with priority = %d, length = %d\n",
           buffer, prio, nbytes);
  }

  mq_close(mymq);
    
}

static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

void * sender(void * param) 
{
  mqd_t mymq;
  int prio;
  int nbytes;

  printf("Inside sender!\n");
  mymq = mq_open(SNDRCV_MQ, O_RDWR, S_IRWXU, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  /* send message with priority=30 */
  if((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg), 30)) == ERROR)
  {
    perror("mq_send");
  }
  else
  {
    printf("send: message successfully sent\n");
  }
  
}


void main(void)
{

  pthread_t send_thread, recieve_thread;
  pthread_attr_t send_thread_attr, recieve_thread_attr;

  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = MAX_MSG_SIZE;

  mq_attr.mq_flags = 0;

  struct sched_param rt_param;

  /* receiver runs at a higher priority than the sender */
  // if(taskSpawn("Receiver", 90, 0, 4000, receiver, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR) {
  //   printf("Receiver task spawn failed\n");
  // }
  // else
  //   printf("Receiver task spawned\n");

  // if(taskSpawn("Sender", 100, 0, 4000, sender, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR) {
  //   printf("Sender task spawn failed\n");
  // }
  // else
  //   printf("Sender task spawned\n");

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


}