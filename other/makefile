CC 			= gcc
CFLAGS 		= -Wall
LDFLAGS 	= 
OBJFILES 	= main.o parser.o semantics.o scanner.o errores.o
TARGET		= compiler

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
