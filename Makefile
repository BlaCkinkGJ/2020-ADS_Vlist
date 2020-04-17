CC=g++
CFLAGS=-pg -Wall -Wextra #-D DEBUG
# LDFLAGS=
# LDLIBS=
OBJS=main.o vlist.o
TARGET=vlist deque list vector forward
OSFLAG=

ifneq ($(OS),Windows_NT)
	CC=gcc
	EXE=a.out
	CFLAGS+=-pedantic -pg
endif
>>>>>>> master

all: $(TARGET)

vlist: $(OBJS)
	$(CC) $(CFLAGS) -D VLIST -c main.cpp
	$(CC) -o vlist.out $(OBJS)

deque: $(OBJS)
	$(CC) $(CFLAGS) -D DEQUE -c main.cpp
	$(CC) -o deque.out $(OBJS)

list: $(OBJS)
	$(CC) $(CFLAGS) -D LIST -c main.cpp
	$(CC) -o list.out $(OBJS)

vector: $(OBJS)
	$(CC) $(CFLAGS) -D VECTOR -c main.cpp
	$(CC) -o vector.out $(OBJS)

forward: $(OBJS)
	$(CC) $(CFLAGS) -D FORWARD_LIST -c main.cpp
	$(CC) -o forward.out $(OBJS)

vlist.o: vlist.c
	gcc $(CFLAGS) -c vlist.c


clean:
	rm *.o
	rm $(TARGET)
