CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.
INC = include/
SRC = src/
BIN = bin/

parser: $(BIN)libvcparser.so
	
$(BIN)libvcparser.so: LinkedListAPI.o VCParser.o VCHelpers.o
	$(CC) -shared -o $(BIN)libvcparser.so LinkedListAPI.o VCParser.o VCHelpers.o

LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)LinkedListAPI.c

VCParser.o: $(SRC)VCParser.c $(INC)VCParser.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)VCParser.c

VCHelpers.o: $(SRC)VCHelpers.c $(INC)VCHelpers.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)VCHelpers.c

clean:
	rm -rf *.o $(BIN)*.so
