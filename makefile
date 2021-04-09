# change application name here (executable output name)
TARGET=compiler
 
# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall

LIBS=-lm
 
PTHREAD=-pthread
 
CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe -fno-stack-protector -w
 
# linker
LD=gcc
LDFLAGS=$(PTHREAD) -export-dynamic
 
OBJS=    compiler.o
 
all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)
    
compiler.o: compiler.c
	$(CC) -c $(CCFLAGS) compiler.c -o compiler.o
    
clean:
	rm -f *.o $(TARGET)
