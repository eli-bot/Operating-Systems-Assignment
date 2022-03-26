/*  AUTHOR: Elijah Combes
    NAME: LiftSimulatorB.c
    PURPOSE: program to simulate the operations of three lifts/elevators running concurrently
            to service requests from different floors from a building. Each lift is run by a seperate process. Writes the details of each operation
            and each request to a file( "sim_out" ) and terminates when all requests from the input file
            and in the buffer of requests have been serviced.
*/    
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
  #include <sys/mman.h>
       #include <sys/stat.h>  
       #include <fcntl.h>
#include <semaphore.h>       

#include "BufferOperations.h"
#include "LiftSimulator.h" 
#include "FileReader.h"
// struct to contain all data that needs to be in shared memory
typedef struct SharedM{
    Buffer buf; // buffer
    sem_t sem; // sempahore
    int done; // 0(FALSE) if there are still requests to read in, 1(TRUE) if all requests have been read
    int totalReq; // total number of requests served
    int totalMove; // total number of movement completed
} SharedM;

int getProcessNumber(pid_t* pids); // function declarations, here as A and B use the same header file
void *lift(void *ptr, pid_t *pids);

int *t; 

// MAIN:
//  used to create shared memory and the 4 processes used in this program to represent the 3 lifts and the request creator
int main(int argc, char *argv[])
{
    int s,i,j;
    pid_t *pids;
    Request *requests;
    SharedM *sharedMem = (SharedM*)malloc(sizeof(SharedM));
    pid_t wpid;
    t = (int*)malloc(sizeof(int));

    if( argc != 3) // incorrect number of args
    {
        printf("Please provide the buffer size(m) and time(t)\n");
        printf("run using ./LiftSimulator m t\n");
    }
    else
    {
        // holds each of the process ids: main process(index 0),request(index 1 etc.) lift: 1,2,3 (ie 5 processes total)
        pids = (pid_t*)malloc(sizeof(pid_t) * NUM_TASKS + 1);
    
        if(atoi(argv[1]) < 1) // buffer size must be >= 1
        {
            printf("Error: please ensure the buffer size is >= 1\n");
        }
        else if(atoi(argv[2]) < 0) // time must be >= 0
        {
            printf("Error: time must be >= 0\n");
        }
        else
        {   
            requests = (Request*)malloc(sizeof(Request) * atoi( argv[1] ) ); //create space to store buffer of requests
            // get shared memory for SharedM
            int fd =  shm_open("/sharedMem", O_RDWR | O_CREAT, 0660);
            if(fd < 0)
                printf("Error: shm_open - /x\n");
            if( ftruncate(fd, sizeof(SharedM*)) == -1)
                printf("Error: ftruncate\n");   
            if((sharedMem = (SharedM*)mmap(0,sizeof(SharedM*),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
                printf("Error: mmap\n");
            // create shared mem for Request buffer(requests) 
            fd =  shm_open("/requests", O_RDWR | O_CREAT, 0660);
            if(fd < 0)
                printf("Error: shm_open - /requests\n");
            if( ftruncate(fd, sizeof(Request*)) == -1)
                printf("Error: ftruncate\n");
            if((requests = (Request*)mmap(0,sizeof(Request*),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
                printf("Error: mmap\n"); 
            // create shared memory for process id's
            fd =  shm_open("/pids", O_RDWR | O_CREAT, 0660);
            if(fd < 0)
                printf("Error: shm_open - /pids\n");
            if( ftruncate(fd, sizeof(pid_t*)) == -1)
                printf("Error: ftruncate\n");
            if((pids = (pid_t*)mmap(0,sizeof(pid_t*),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
                printf("Error: mmap\n");     

            pids[0] = getpid(); // set pid of main    
    
            sharedMem->buf.size = atoi(argv[1]); // assign buffer size from command line
            *t = atoi(argv[2]); // time(seconds) from command line
            sharedMem->buf.requests = requests; // add request buffer to the main shared memory SharedM
            sharedMem->buf.numReq = 0;
            
            int semVal = sem_init(&(sharedMem->sem),1,1); // initialise semaphore with value 1
            if( semVal == -1 )
            {
                printf("Pid: %d, Error: sem_init\n", getpid());
            }
            
            //create a process for request() 
            pids[1] = fork();
            if( pids[1] < 0 )
            {
                perror("Fork unsuccessful\n");
            }   
            if( pids[1] == 0) // child process
            {
                pids[1] = getpid();
                request(sharedMem);
                _exit(3);
        
            }
                // create a process for each lift and run the lift function for each one
                for(i = 2; i < NUM_TASKS + 1; i++)
                {
                    // create lift processes
                    pids[i] = fork();
                    if( pids[i] < 0 )
                    {
                        perror("Fork unsuccessful\n");
                    }   
                    if( pids[i] == 0)  // child process
                    {
                        pids[i] = getpid();
                        lift(sharedMem,pids);
                        _exit(3);
                    }    
                }       
        }
        int status = 0;
        for(int k = 0; k < 4; k++)
        {  
            wait(NULL);
        }   
        writeTotals(&sharedMem->totalMove, &sharedMem->totalReq);
        sem_destroy(&(sharedMem->sem));
        shm_unlink("/requests");
        shm_unlink("/sharedMem");
        free(t);
        printf("Simulation Complete\n");
        printf("Please refer to the file 'sim_out' for results\n");
    }    
    return 0;    
} 

/*  NAME: request
    IMPORTS: ptr(void*) - a pointer to a shared memory
    EXPORTS: void 
    PURPOSE: function to open the input file "sim_input" to read in data about a request and save this data in a queue(sharedMem->buf.requests)
             stored in a Buffer. The request data is then written to a file "sim_out".If the buffer is full then it waits for a request to be removed from the queue and 
             then proceeds to read in another request and add it to the queue. Process repeats until the end of the 
             input file is reached and there are no more requests to add. */  
void *request(void *ptr)
{
    char* fileName = "sim_input";
    FILE *f;
    int i = 1;
    int requestNo = 0;
    int numLines;
   
    SharedM *sharedMem = ptr;
    Buffer *buf = &(sharedMem->buf);   
    Request *req = buf->requests; // convert the void pointer from thread creation to the requests buffer
 
    numLines = getNumLines(fileName); //fgets in this method causes process to terminate
    f = fopen(fileName,"r"); // open the input file
 
    while( numLines > 0 )// loop whilst the input file still has lines to be read
    {   
        sem_wait(&(sharedMem->sem)); // wait for semaphore 
        if(!bufferIsFull(buf))    
        {
            addRequest(buf); // updates head/tail pointers in buffer ready for a new request
            printf("\n");
            readInputLine(f, &(*(req + buf->tail)) ); // read a request from the file and add to buffer
            requestNo++;
            requestWrite( &req[buf->tail], &requestNo );  // write the new request to the output file
            numLines--; // when numLines reaches 0 the last line has been read
        
            sem_post(&(sharedMem->sem)); // increment semaphore
        }
        else
        {
            while(bufferIsFull(buf)) // 
            {      
                sem_post(&(sharedMem->sem)); // increment semaphore
                sem_wait(&(sharedMem->sem)); // wait for semaphore/ added to ready q 
            }
            sem_post(&(sharedMem->sem));
        }    
    }
    fclose(f);
    sem_wait(&(sharedMem->sem));
    sharedMem->done = TRUE; // all requests have been added to the buffer
    sem_post(&(sharedMem->sem));
    return ptr;
}

/*  NAME: lift
    IMPORTS: ptr(void*) - pointer to shared memory, pids - process id's of processes created in this program
    EXPORTS: void
    PURPOSE: function to simulate operations of a lift. Removes a request from the queue and moves from the source floor
            to the destination floor( by altering data stored in *liftData). This data is written to a file for each operation
            that the lift performs( each request ). The function waits whilst there are no requests
            in the queue. */
void *lift(void *ptr, pid_t *pids)
{
    int *liftData;
    SharedM *sharedMem = (SharedM*)ptr;
    liftData = (int*)malloc(sizeof(int) * 6); 
    Request* curRequest;
   
    sem_wait(&(sharedMem->sem));
    liftData[LIFT_NUMBER] = getProcessNumber(pids); // gets lift number/process number
    sem_post(&(sharedMem->sem));
    liftData[PREVIOUS_FLOOR] = 1; //start lift at Floor 1

    while( !(sharedMem->done) || !bufferIsEmpty(&(sharedMem->buf)) ) // loop whilst there are still requests to be read(ie not done), if done then check if there are requests left in the buffer
    {
        sem_wait(&(sharedMem->sem));
        if( !bufferIsEmpty(&(sharedMem->buf) ))
        {
            curRequest = removeRequest(&(sharedMem->buf)); // gets first request in the q and removes it from buffer
            liftData[REQUEST_MOVEMENT] = abs(liftData[CURRENT_FLOOR] - curRequest->source) + 
                                         abs(curRequest->source - curRequest->dest); // calculates floor movement for this particular request
            //move lift  
            liftData[CURRENT_FLOOR] = curRequest->dest; //floor that lift ends at after moving
            liftData[REQUESTS_SERVED]++; // updates number of requests served by this lift
            liftData[TOTAL_MOVEMENT] = liftData[TOTAL_MOVEMENT] + liftData[REQUEST_MOVEMENT]; // updates total movement 
           
            fileWrite(liftData, curRequest); // write the data for this request to the file
            liftData[PREVIOUS_FLOOR] = liftData[CURRENT_FLOOR]; // update prev floor
            sem_post(&(sharedMem->sem));
            sleep(*t); // sleep for t seconds
        }
        else
        {
                while( bufferIsEmpty(&(sharedMem->buf)) ) 
                {
                    if(sharedMem->done) // if done then exit from this while loop
                    {
                        sem_post(&(sharedMem->sem)); // increment semaphore before exiting
                        break;
                    }
                    sem_post(&(sharedMem->sem));
                    sem_wait(&(sharedMem->sem));  
                }   
                sem_post(&(sharedMem->sem)); 
        }
    }
    sem_wait(&(sharedMem->sem));
    sharedMem->totalMove = sharedMem->totalMove + liftData[TOTAL_MOVEMENT];
    sharedMem->totalReq = sharedMem->totalReq + liftData[REQUESTS_SERVED];
    sem_post(&(sharedMem->sem));
    free(liftData);
    return ptr;
}

/*  NAME: getProcessNumber
     IMPORTS: pids - process id's for all created threads in this program
     EXPORTS: int 
     PURPOSE: returns the process number of the process that calls it,
             made mainly to number each lift process */
int getProcessNumber(pid_t* pids)
{
    pid_t pid = getpid();
    int processNumber = -1;
    for(int i = 0; i < NUM_TASKS + 1; i++)
    {
        if(pids[i] == pid)
        {
            processNumber = i - 1; // minus one as Lift-1 is at index 2 etc
        }
    }
    return processNumber;
}

// for LiftSimulatorA.c
int validateThreadCreation(int s)
{
    if (s != 0)
        printf("Error: pthread_create\n");   
    return s;    
}

 