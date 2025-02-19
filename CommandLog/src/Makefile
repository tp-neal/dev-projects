CFLAGS = -Wall -Wextra
CC = gcc
TARGET = hscript

all: $(TARGET)

$(TARGET): hscript.o
	$(CC) $(CFLAGS) -o $(TARGET) hscript.o

hscript.o: hscript.c hscript.h
	$(CC) $(CFLAGS) -c hscript.c

clean:
	rm -f $(TARGET) *.o err_log
	rm -f -r dir

.PHONY: all clean