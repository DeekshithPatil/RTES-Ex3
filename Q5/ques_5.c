/*-------------------------------------------------------------------------------------------------------------------------------
File Name   : Question_2.c
Author      : Kevin Tom
              Real-Time Embedded Systems
              University of Colorado Boulder
Email       : keto9919@colorado.edu
Paltform    : Ubuntu 20.04
IDE Used    : Visual Studio code
Date        : 26 Feb 2022
Version     : 1.0
            
Description : This program creates a process and waits for semaphores from another program
              
Reference   : timespec : https://www.educative.io/edpresso/what-is-timespec-in-c
              pthread  : https://www.geeksforgeeks.org/multithreading-c-2/
                         https://www.educative.io/edpresso/how-to-create-a-simple-thread-in-c - Excellent demo in the end
              mutex    : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/


---------------------------------------------------------------------------------------------------------------------------------*/


#include <stdio.h>
#include <pthread.h> //For creating and managing threads
#include <time.h>    //For timespec time
#include <stdlib.h>
#include <unistd.h> 


#define NUM_OF_THREADS               (2)
#define ACCELEROMETER_MAX_VALUE      (250)
#define GYROSCOPE_MAX_DEGREE_VALUE   (360)


//MUTEX for locking 
pthread_mutex_t attitude_state_lock;



//Attitude structure for threads to use
typedef struct sensor_readings
{
    //Acceleration readings on all axes
    double acc_X;
    double acc_Y;
    double acc_Z;

    //Roll,Pitch and Yaw readings
    double sensor_ROLL;
    double sensor_PITCH;
    double sensor_YAW;

    //This variable will have the time that the measurement was taken
    struct timespec time;

}sensor;


//Sensor
sensor sensor_1;



void* update_sensor_struct(void *arg)
{
    pthread_mutex_lock(&attitude_state_lock);

    printf("\n\n\n\rTHREAD-1 Before Execution\
            \n\r-------------------------");
    printf("\n\racc_X :%f\
            \n\racc_Y :%f\
            \n\racc_Z :%f\
            \n\rROLL  :%f\
            \n\rPITCH :%f\
            \n\rYAW   :%f\
            \n\rTime  :%ld S and %ld nS\n\r",sensor_1.acc_X,sensor_1.acc_Y,sensor_1.acc_Z,\
            sensor_1.sensor_ROLL,sensor_1.sensor_PITCH,sensor_1.sensor_YAW,sensor_1.time.tv_sec,\
            sensor_1.time.tv_nsec);
    
    sleep(13);

    //generating a random number
    time_t t;
    srand((unsigned) time(&t));

    //assigning random values to variables
    sensor_1.acc_X        = rand()%ACCELEROMETER_MAX_VALUE; //acclerometer values can vary from 0-250G
    sensor_1.acc_Y        = rand()%ACCELEROMETER_MAX_VALUE;
    sensor_1.acc_Z        = rand()%ACCELEROMETER_MAX_VALUE;

    sensor_1.sensor_ROLL  = rand()%GYROSCOPE_MAX_DEGREE_VALUE; //Roll pitch Yaw values can vary from 0-360 degrees per second
    sensor_1.sensor_PITCH = rand()%GYROSCOPE_MAX_DEGREE_VALUE;
    sensor_1.sensor_YAW   = rand()%GYROSCOPE_MAX_DEGREE_VALUE;

    clock_gettime(CLOCK_REALTIME,&(sensor_1.time)); //updating time of measurement


    printf("\n\n\n\rTHREAD-1 After Execution\
            \n\r------------------------");
    printf("\n\racc_X :%f\
            \n\racc_Y :%f\
            \n\racc_Z :%f\
            \n\rROLL  :%f\
            \n\rPITCH :%f\
            \n\rYAW   :%f\
            \n\rTime:%ld S and %ld nS\n\r",sensor_1.acc_X,sensor_1.acc_Y,sensor_1.acc_Z,\
            sensor_1.sensor_ROLL,sensor_1.sensor_PITCH,sensor_1.sensor_YAW,sensor_1.time.tv_sec,\
            sensor_1.time.tv_nsec);
    printf("\n\r----------------------------------------------------------------------------------------------");

    pthread_mutex_unlock(&attitude_state_lock);
    return NULL;
}


//this function will access 
void* access_sensor_print(void *arg)
{
    struct timespec *wait;
    struct timespec timestamp;
    int ret;
    wait=(struct timespec *)(malloc(sizeof(struct timespec)));
    wait->tv_sec=10;
    wait->tv_nsec=0;
    ret=pthread_mutex_timedlock(&attitude_state_lock,wait);
    if(ret!=0)
    {
        clock_gettime(CLOCK_REALTIME,&(timestamp));
        printf("\n\n\n\rNo data available at %ld S and %ld nS",timestamp.tv_sec,timestamp.tv_nsec); 
        return NULL;
    }
    pthread_mutex_unlock(&attitude_state_lock);
    return NULL;
}



//Entry point of the program
int main()
{
    //pthread_t is a integer used to recognize the thread (it is just a number)
    pthread_t thread[NUM_OF_THREADS];

    //initialzing mutex
    pthread_mutex_init(&attitude_state_lock, NULL);

    int pthread_call_status = 0;

    while(1)
    {
    pthread_call_status = pthread_create(&(thread[0]),NULL,update_sensor_struct,NULL);
    if(pthread_call_status != 0)
        printf("Call to pthread_create failed with ERROR CODE %d", pthread_call_status);
    
    
    pthread_call_status = pthread_create(&(thread[1]),NULL,access_sensor_print,NULL);
    if(pthread_call_status != 0)
        printf("Call to pthread_create failed with ERROR CODE %d", pthread_call_status);
    


    //main thread (which is executing main function) will wait till the thread finished execution
    pthread_call_status = pthread_join(thread[0],NULL);
    if(pthread_call_status != 0)
        printf("Call to pthread_join failed with ERROR CODE %d", pthread_call_status);

    
    pthread_call_status = pthread_join(thread[1],NULL);
    if(pthread_call_status != 0)
        printf("Call to pthread_join failed with ERROR CODE %d", pthread_call_status);
    }
    
    pthread_mutex_destroy(&attitude_state_lock);

    return 0;
}
