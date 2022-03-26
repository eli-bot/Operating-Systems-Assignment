/*  AUTHOR: Elijah Combes
    NAME: LiftSimulatorB.c
    PURPOSE: Multithreaded program to simulate the operations of three lifts/elevators running concurrently
            to service requests from different floors from a building. Writes the details of each operation
            and each request to a file( "sim_out" ) and terminates when all requests from the input file
            and in the buffer of requests have been serviced.
*/            

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "BufferOperations.h"
#include "LiftSimulator.h" 
#include "FileReader.h"


struct thread_info {    /* Used as argument to thread_start() */
           pthread_t thread_id;        /* ID returned by pthread_create() */
           int       thread_num;       /* Application-defined thread # */
           char     *argv_string;      /* From command-line argument */
       };

void *lift(void *ptr);       
int getThreadId(pthread_t thread);

Buffer *buf; // pointer to buffer
pthread_mutex_t lock1, lock2; // lock for access to the buffer
pthread_cond_t bufferEmpty;
pthread_cond_t bufferFull;
struct thread_info *tInfo;
int *t; // time
int *totalReq; // total number of requests
int *totalMove; // total number of movements by all lifts
int done = FALSE;

int main(int argc, char *argv[])
{
    int s,i,j;
    pthread_attr_t attr;
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_cond_init(&bufferEmpty, NULL);
    pthread_cond_init(&bufferFull, NULL);
    pthread_t tid[NUM_TASKS]; // stores thread id's
   // Request *requests;
    t = (int*)malloc(sizeof(int));

    if( argc != 3)
    {
        printf("Please provide the buffer size(m) and time(t)\n");
        printf("run using ./LiftSimulator m t\n");
    }
    else
    {
        buf = (Buffer*)malloc(sizeof(Buffer)); // create space for Buffer
        buf->size = atoi(argv[1]); 
        *t = atoi(argv[2]);
        totalMove = (int*)malloc(sizeof(int));
        totalReq = (int*)malloc(sizeof(int));
        if(buf->size < 1)
        {
            printf("Error: please ensure the buffer size is >= 1\n");
        }
        else if(*t < 0)
        {
            printf("Error: time must be >= 0\n");
        }
        else
        { 
            // initalising buffer 
            buf->requests = (Request*)malloc(sizeof(Request) * (buf->size)); // create space to hold each request 

            buf->numReq = 0; // initialise number of requests to 0
            buf->head = -1;
            buf->tail = -1;
            s = pthread_attr_init(&attr);

            tInfo = calloc(4,sizeof(struct thread_info)); // create space for each of the 4 threads info
            s = pthread_create(&tInfo[0].thread_id,NULL,&request, buf->requests); // create Lift_R request thread
            validateThreadCreation(s);
            
            for(i = 1; i < NUM_TASKS; i++)
            {
                s = pthread_create(&tInfo[i].thread_id,NULL,&lift, &tInfo[i]); // create 3 threads, one for each lift
                tInfo[i].thread_num = i;
                validateThreadCreation(s);
            }
            
            pthread_join(tInfo[0].thread_id,NULL); // join threads so main must wait for their completion
            
            pthread_join(tInfo[2].thread_id,NULL);
            pthread_join(tInfo[1].thread_id,NULL);
            pthread_join(tInfo[3].thread_id,NULL);
        }
       // pthread_exit(NULL);
        pthread_mutex_destroy(&lock1);
        free(buf);
        free(tInfo);
        writeTotals(totalMove,totalReq); // writes totdal to file
        free(totalMove);
        free(totalReq);
    }    
    free(t);
    printf("Simulation Complete\n");
    printf("Please refer to the file 'sim_out' for results\n");
    return 0;    
} 

/*  NAME: request
    IMPORTS: ptr(void*) - a pointer to a buffer to store data about requests
    EXPORTS: ptr(void*) 
    PURPOSE: thread function to open the input file "sim_input" to read in data about a request and save this data in a queue(buf.requests)
             stored in a Buffer. The request data is then written to a file "sim_out".If the buffer is full then it waits for a request to be removed from the queue and 
             then proceeds to read in another request and add it to the queue. Process repeats until the end of the 
             input file is reached and there are no more requests to add. */  
void *request(void *ptr)
{
    char* fileName = "sim_input";
    FILE *f;
    int i = 1;
    int requestNo;
    int numLines = getNumLines(fileName); // represents the number of lines left to read
   
    Request *newRequest;
    Request *req = (Request*)ptr; // convert the void pointer from thread creation to the requests buffer
    f = fopen(fileName,"r");
    printf("\n");
    if(f == NULL)
    {
        printf("Error opening file. \n");
    }
    else
    {
        // loop whilst the input file still has lines to be read
        while( numLines > 0 )
        {   
            // adds a request to the buffer
            pthread_mutex_lock(&lock1); // lock mutex to access CS
            if(!bufferIsFull(buf))    
            {
                addRequest(buf); // moves head/tail pointers in buffer
                printf("\n");
                readInputLine(f, &(*(req + buf->tail)) ); // reads in new request and adds to buffer
                requestNo++;
                requestWrite( &req[buf->tail], &requestNo ); // write the new request to the file
                numLines--; // decrement numLines when a line is read from the file
                
                pthread_cond_signal(&bufferFull); // allows lift's to take from the buffer when numReq > 0 && numReq <= 5
                pthread_mutex_unlock(&lock1); // unlock mutex so lift's can acquire it         
            }
            else
            {
                while(bufferIsFull(buf))
                {
                    pthread_cond_signal(&bufferFull); // signal to lift's that the buffer is full/ not empty
                    pthread_cond_wait(&bufferEmpty, &lock1);   // unlock mutex and wait until the buffer is empty/not full to acquire it  again   
                }
                pthread_mutex_unlock(&lock1); // unlocks utex as it is already acquired at top of while loop
            }    
        }
        done = TRUE; // sets done to true so lifts can exit their loop once finished
    }   
    fclose(f); 
    return ptr;
}

/*  NAME: lift
    IMPORTS: ptr(void*)
    EXPORTS: ptr
    PURPOSE: thread function to simulate operations of a lift. Removes a request from the queue and moves from the source floor
            to the destination floor( by altering data stored in *liftData). This data is written to a file for each operation
            that the lift performs( each request ). The function waits whilst there are no requests
            in the queue. */
void *lift(void *ptr)
{
    int *liftData; 
    liftData = (int*)malloc(sizeof(int) * 6); // pointer to store individual lift data, indexes defined in "LiftSimulator.h"
    Request* curRequest;
   
    int tid = getThreadId(pthread_self()); // gets the thread id of this thread
    liftData[LIFT_NUMBER] = tid;

    liftData[PREVIOUS_FLOOR] = 1; //start lift at Floor 1
 
    while( !done || !bufferIsEmpty(buf)) // loop whilst there are still requests to be read from file or buffer is not empty
    { 
       
        pthread_mutex_lock(&lock1); // acquire the mutex
        if( !bufferIsEmpty(buf) )
        {
            curRequest = removeRequest(buf); // gets first request in the q and removes it from buffer
            
            liftData[REQUEST_MOVEMENT] = abs(liftData[CURRENT_FLOOR] - curRequest->source) + 
                                         abs(curRequest->source - curRequest->dest); // calculates floor movement for this particular request
            // Move Lift
            liftData[CURRENT_FLOOR] = curRequest->dest; //floor that lift ends at after moving
            liftData[REQUESTS_SERVED]++; // updates number of requests served by this lift
            liftData[TOTAL_MOVEMENT] = liftData[TOTAL_MOVEMENT] + liftData[REQUEST_MOVEMENT]; // updates total movement 
         
            fileWrite(liftData, curRequest); // write the data for this request to the file
           
            liftData[PREVIOUS_FLOOR] = liftData[CURRENT_FLOOR]; // update previous floor
         
            pthread_cond_signal(&bufferFull); // allows other lifts to acquire mutex
            pthread_cond_signal(&bufferEmpty); // allows LiftR to add more requests to the buffer3
            pthread_mutex_unlock(&lock1); // unlocks mutex so other threads can acquire it
          
            sleep(*t); // sleep for *t seconds
        }
        else
        {
                while( bufferIsEmpty(buf) ) // loop while the buffer is empty
                {
                    pthread_cond_signal(&bufferEmpty); // signal to liftR that the buffer is empty
                    if(done)
                    {
                        break;
                    }
                    pthread_cond_wait(&bufferFull,&lock1); // wait for the buffer to be full/ not empty + unlock mutex   
                } 
                pthread_mutex_unlock(&lock1); // unlock mutex
        }
    }
    *totalMove = *totalMove + liftData[TOTAL_MOVEMENT]; // add this particular lift's total movement to the total movement of all lifts
    *totalReq = *totalReq +liftData[REQUESTS_SERVED]; // add this particular lift's # of requests served to the # served by all lifts

    free(liftData);   // free malloc'd space
    pthread_exit(NULL);
    return ptr;
}


/*  NAME: validateThreadCreation
    IMPORTS: s(int) - status returned by pthread_create when making a thread
    EXPORTS: s(int)
    PURPOSE: notifies user of an error during thread creation if the status != 0
            at this stage should do more error handling 
*/   
int validateThreadCreation(int s)
{
    if (s != 0)
        printf("Error: pthread_create\n");
    return s;    
}

 /*  NAME: getThreadId
     IMPORTS: thread(pthread_t)
     EXPORTS: int 
     PURPOSE: mainly for debugging, converts the thread id into the corresponding thread number
             that was assigned to it in main (ie 1, 2 or 3, representing each of the three lifts)
             returns 100(error) if the thread could not be found. */  
 int getThreadId(pthread_t thread)
 {
    int i,s; 
     for(i = 0; i < 4;i++)
     {
         s = pthread_equal(thread, tInfo[i].thread_id);
         if(s > 0)
         {
             return tInfo[i].thread_num;
         }
     }
    return 100;
 }