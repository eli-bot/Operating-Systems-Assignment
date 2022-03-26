typedef struct Request{
    //request data
    int source; // source floor
    int dest; 
          //destination floor
} Request;

typedef struct Buffer{
    Request *requests; //pointer to array of requests
    int numReq; // number of requests currently in the buffer
    int size; // size of the buffer determined by command line input
    int head; // head of request queue
    int tail; // tail of
} Buffer;

int bufferIsFull();
struct Request* removeRequest();
void addRequest( Buffer *buf);
int bufferIsEmpty();

#define FALSE 0
#define TRUE !FALSE