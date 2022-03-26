OBJ = LiftSimulatorA.o FileReader.o BufferOperations.o
OBJ2 = LiftSimulatorB.o FileReader.o BufferOperations.o
EXEC = lift_sim_A
EXEC2 = lift_sim_B


CFLAGS = -g  -pthread -lpthread ## -Wall -Werror 

lift_sim_A: $(OBJ)
	gcc $(OBJ) -pthread -o $(EXEC) -lm 

lift_sim_B: $(OBJ2)
	gcc $(OBJ2) -pthread -o $(EXEC2) -lm -lrt	

LiftSimulatorA.o: FileReader.h LiftSimulatorA.c LiftSimulator.h  BufferOperations.h
	gcc -c LiftSimulatorA.c $(CFLAGS) 

LiftSimulatorB.o: FileReader.h LiftSimulatorB.c LiftSimulator.h BufferOperations.h
	gcc -c LiftSimulatorB.c $(CFLAGS)

FileReader.o: FileReader.c FileReader.h LiftSimulator.h BufferOperations.h
	gcc -c FileReader.c $(CFLAGS)

BufferOperations.o: BufferOperations.c BufferOperations.h LiftSimulator.h
	gcc -c BufferOperations.c $(CFLAGS)

all: LiftSimulatorA.o LiftSimulatorB.o lift_sim_A lift_sim_B

clean:
	rm -f $(EXEC) $(OBJ) $(EXEC2) $(OBJ2) sim_out
