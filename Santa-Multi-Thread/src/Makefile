CC       = c++
FLAGS    = -no-pie

HEADER_FILE = thread.h thread-main.h
SRC_FILE = thread.cpp thread-main.cpp
OBJ_FILE = thread.o thread-main.o
EXE_FILE = prog5

all: ${EXE_FILE}

${EXE_FILE}: ${OBJ_FILE}
	${CC} ${FLAGS} -o ${EXE_FILE}  ${OBJ_FILE} -lpthread

thread.o: thread.cpp thread.h
	${CC} ${FLAGS} -c thread.cpp

thread-main.o: thread-main.cpp thread-main.h thread.h
	${CC} ${FLAGS} -c thread-main.cpp

clean:
	rm -f ${OBJ_FILE} ${EXE_FILE}