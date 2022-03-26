/* AUTHOR: Elijah Combes
   NAME: FileReader.c
   PURPOSE: perform file IO for the lift simulator( LiftSimulatorA/B.c )
            reading input file ("sim_input") containing Request data ( source and destination floors )
            writing imported data to the output file "sim_out" 
            - potentially could have been split up into 2 .c files to separate reading and writing functions */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "BufferOperations.h"
#include "LiftSimulator.h"
#include "FileReader.h"
   
/*  NAME: readInputLine
    IMPORTS: f (FILE*) - pointer to open file, ptr(Request*) - pointer to store the request data
    EXPORTS: done (int) - indicates when the file has no more lines to read
    PURPOSE: read one line from the input file and store the data in a Request struct to be returned */    
int readInputLine(FILE* f,Request *ptr)
{
    char *line;
    char *token;
    char *fileName = "sim_input";
    int done = FALSE;
   
    if(f == NULL)
    {
        perror("Error opening file '%s' "); 
    }
    else
    {
        // initialises request structure source and dest values
        if(!(fgets(line,MAXSIZE,f) == NULL))
        {
            //set first element tp ptr->source
            token = strtok(line, " ");
            ptr->source = atoi(token); // set the requests source
            token = strtok(NULL," ");
            ptr->dest = atoi(token);
        }
        else
        {
            printf("End of file reached\n)");
            done = TRUE;
        } 
    }
    return done;
}

/* NAME: getNumLines
   IMPORTS: fileName - name of the file to read
   EXPORTS: numLines - number of lines in the file
   PURPOSE: counts the number of lines in a file and returns # as an integer */
int getNumLines(char *fileName)
{
    FILE* f1;
    int numLines = 0;
    char *line1 = (char*)malloc(sizeof(char) * MAXSIZE);
    f1 = fopen(fileName,"r");
    if( f1 == NULL )
    {
        perror("Error opening file.");
    }
    else
    {
        // counts number of lines in the file
        while( !(fgets(line1,MAXSIZE,f1) == NULL))  // if next line is not null
        {
            numLines++; // increment number of lines
        }
    }
    fclose(f1);
    free(line1);
    return numLines;
}

/*  NAME: requestWrite
    IMPORTS: curRequest(Request*) - current request to writeto file, requestNo(int*) - request number
    EXPORTS: none
    PURPOSE: write data about the current request to the file "sim_out" */
void requestWrite(Request *curRequest, int *requestNo)
{
    // print to sim_out
    FILE *filePointer;
    char *outputFile = "sim_out";
    int end;

    filePointer = fopen(outputFile,"a");
    if( filePointer == NULL )
    {
        perror("Error opening file: '%s'");
    }
    else
    {
        fprintf(filePointer, "--------------------------------------------\n");
        fprintf(filePointer, "New Lift Request From Floor %d to Floor %d\n",curRequest->source, curRequest->dest);
        fprintf(filePointer, "Request No: %d\n", *requestNo);
        fprintf(filePointer, "--------------------------------------------\n\n");
        end = fclose(filePointer);
        if( !( end == 0 ) )
        {
            printf("Error writing to file\n");
        }
    }
}

/*  NAME: fileWrite
    IMPORTS: lift(int*) - lift number, curRequest(Request*)
    EXPORTS: none
    PURPOSE: write data about the current lift operation to the output file "sim_out"   */
void fileWrite(int *lift, Request *curRequest)
{
    FILE* f;
    char* outputFile = "sim_out";
    int end;
    f = fopen(outputFile,"a");
    if( f == NULL )
    {
        perror("Error opening file: '%s' ");
    }
    else
    {     
        fprintf(f,"Lift-%d Operation\n", lift[LIFT_NUMBER]);
        fprintf(f,"Previous position: Floor %d\n",lift[PREVIOUS_FLOOR]);
        fprintf(f,"Request: Floor %d to Floor %d\n",curRequest->source, curRequest->dest );
        fprintf(f,"Detail operations: \n");
        fprintf(f, "    Go from Floor %d to Floor %d\n",lift[PREVIOUS_FLOOR], curRequest->source);
        fprintf(f, "    Go from Floor %d to Floor %d\n",curRequest->source, curRequest->dest );
        fprintf(f, "    #movement for this request: %d\n",lift[REQUEST_MOVEMENT] /* reqMovement*/);
        fprintf(f, "    #request: %d\n",lift[REQUESTS_SERVED] /*totalReq*/);
        fprintf(f, "    Total #movement: %d\n",lift[TOTAL_MOVEMENT]/* totalMove */);
        fprintf(f, "Current position: Floor %d\n\n", lift[CURRENT_FLOOR] /* curFloor */);
    
        end = fclose(f); 
        if( !(end == 0) )
        {
            printf("Error when writing to file\n");
        }
    }
}    

/*  NAME: writeTotals
    IMPORTS: totalMove(int*), totalReq(int*)
    EXPORTS: none
    PURPOSE: write the total movements and total requests for all lifts 1,2 and 3 to the outputFile */
void writeTotals(int* totalMove, int* totalReq)
{
    FILE *f;
    char* outputFile = "sim_out";
    f = fopen(outputFile,"a");
    if( f == NULL )
    {
        perror("Error opening file '%s'\n");
    }
    else
    {
        fprintf(f, "Total number of requests: %d\n", *totalReq);
        fprintf(f, "Total number of movements: %d\n", *totalMove);
    }  
}