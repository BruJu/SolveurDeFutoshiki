CC = gcc

LIBS = -lm 
CCFLAGS = -Wall -W -Wextra -std=gnu99 -g3

OBJ = affichage.o definitions.o lecture.o resolution.o heuristique.o resolutionDeductive.o gestHeuristique.o


all: solver clean

solver: principal.c $(OBJ)
	$(CC) $(CCFLAGS) -o solver principal.c $(OBJ)

affichage.o: affichage.c
	$(CC) $(CCFLAGS) -c affichage.c

definitions.o: definitions.c
	$(CC) $(CCFLAGS) -c definitions.c
	
resolution.o: resolution.c
	$(CC) $(CCFLAGS) -c resolution.c
	
heuristique.o: heuristique.c
	$(CC) $(CCFLAGS) -c heuristique.c
	
gestHeuristique.o: gestHeuristique.c
	$(CC) $(CCFLAGS) -c gestHeuristique.c
	
lecture.o: lecture.c
	$(CC) $(CCFLAGS) -c lecture.c

resolutionDeductive.o: resolutionDeductive.c
	$(CC) $(CCFLAGS) -c resolutionDeductive.c

.PHONY : clean

clean:
	rm *.o
