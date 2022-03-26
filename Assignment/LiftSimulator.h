// LiftSimulator.c header file containing struct and fucntion declarations and definitions   

/* each represents an index of an array that this data is stored in
   all are variables that must be kept track of by each lift */ 
#define CURRENT_FLOOR 0
#define PREVIOUS_FLOOR 1
#define TOTAL_MOVEMENT 2
#define REQUESTS_SERVED 3
#define REQUEST_MOVEMENT 4
#define LIFT_NUMBER 5

/* data */
#define NUM_TASKS 4 // 4 threads ( lift_R, lift1, lift2, lift3)

void *request(void *ptr);
int validateThreadCreation(int s);



