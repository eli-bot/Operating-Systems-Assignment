/* Header file for FileReader.c containing struct and function defs */

int readInputLine(FILE* f,Request *);
void fileWrite();
void requestWrite( Request *curRequest, int *requestNo );
void writeTotals(int* totalMove, int* totalReq);
int getNumLines(char *fileName);

#define indent "    "
#define MAXSIZE 20