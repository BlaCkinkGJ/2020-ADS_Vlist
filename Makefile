CC=g++
CFLAGS=-pg -Wall -Werror -Wextra #-D DEBUG
# LDFLAGS=
# LDLIBS=
OBJS=main.o vlist.o
TARGET=vlist
OSFLAG=
EXE=a.out

all: $(TARGET)

vlist: $(OBJS)
	gcc $(CFLAGS) -c vlist.c
	g++ $(CFLAGS) -c main.cpp
	g++ -o $(EXE) $(OBJS)


clean:
	rm *.o
	rm $(EXE)
