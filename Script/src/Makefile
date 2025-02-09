CFLAGS = -Wall -Wextra
CC = gcc
TARGET = hscript

all: $(TARGET)

$(TARGET): hscript.o
	$(CC) $(CFLAGS) -o $(TARGET) hscript.o

hscript.o: hscript.c hscript.h
	$(CC) $(CFLAGS) -c hscript.c

clean:
	rm -f hscript outfile hscript.o
	rm -f -r log

.PHONY: all clean