/*  AUTHOR: Elijah Combes
    PURPOSE: Contains functions to act on a Buffer allowing the array of Requests in
            the Buffer to act like a circular queue */
#include <stdlib.h>
#include <stdio.h>

#include "BufferOperations.h"
#include "LiftSimulator.h"

/* NAME: addRequest
   IMPORTS: buf - the buffer containing all requests and relevant data
            newRequest - pointer to the new request to be added
   EXPORTS: newRequest - pointer to where the new request must go
   PURPOSE: returns a pointer to the space in the buffer where the new request should be added. updates the buffer/queue data,
            increments numReq, updates head and or tail pointers   */       
void addRequest( Buffer *buf)
{
    if( bufferIsFull(buf)) // this should never be true if logic in LiftSimulators is correct
    {
        printf("Error buffer is full, cannot add request\n"); 
    }
    else
    {
        if(buf->tail == (buf->size - 1))
        {
            buf->tail = 0;
        }
        else
        {
            buf->tail = buf->tail + 1;
        }
        buf->numReq++; // increment number of requests currently in buffer   
    }
    if( buf->head == -1 ) // first element ever added to buffer
    {
        buf->head++;
    }
}

/*  NAME: removeRequest
    IMPORTS: buf(Buffer*) pointer to the buffer to operate on
    EXPORTS: r(Request) - the request that has been removed from the buffer
    PURPOSE: to remove the head element(Request) in the buffer and return it to the calling fucntion 
            also to update the head pointer to track the first element in the queue/buffer */
Request* removeRequest(Buffer *buf)
{
    Request *r = &(buf->requests[buf->head]);
    if((buf->head + 1) == buf->size)
    {
        buf->head = 0;
    }
    else
    {
        buf->head++;
    }
    buf->numReq--;
    return r;   
}

/*  NAME: bufferIsFull
    IMPORTS: buf(Buffer*)
    EXPORTS: isFull(int) - boolean defined in BufferOperations.h
    PURPOSE: check if the imported Buffer is full or not */
int bufferIsFull(Buffer *buf)
{
    int isFull = TRUE;
    if(buf->numReq < buf->size )
    {
        isFull = FALSE;
    }
    return isFull;
}

/*  NAME: bufferIsEmpty
    IMPORTS: buf(Buffer*)
    EXPORTS: isEmpty(int) - boolean defined in BufferOperations.h
    PURPOSE: check if the imported Buffer is Empty or not */
int bufferIsEmpty(Buffer *buf)
{
    int isEmpty = FALSE;
   // printf("IN ISEMPTY size: %d",buf->size);
    // printf("IN ISEMPTY numReq: %d",buf->numReq);
    if(buf->numReq == 0)
    {
        isEmpty = TRUE;
    }
    return isEmpty;
}