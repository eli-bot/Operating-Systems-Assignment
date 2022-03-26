---------- Lift Simulator ----------
Program that simulates the actions of 3 Lifts/elevators who all access a common buffer to service requests
each lift is portrayed by a seperate thread for lift_sim_A OR a seperate process for lift_sim_B.
This program is to demonstrate how processes/threads can run concurrently and access shared data
by conforming to mutual exclusion principles.
Input to the program must be specified in a file called 'sim_input'
Results of the program can be found in a file called 'sim_out' after the program has been run.

To compile:
    'make all'
To Run:
    Thread implementation:
        './lift_sim_A m t'  - where m is the size of the buffer which must be >= 1  
                            and t is the amount of seconds a lift takes to complete a request
    Process implementation:
        './lift_sim_B m t' -  where m is the size of the buffer which must be >= 1  
                            and t is the amount of seconds a lift takes to complete a request                     