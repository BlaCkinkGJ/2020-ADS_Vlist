CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
CFLAGS=-pg -Wall -Werror -Wextra #-D DEBUG
# LDFLAGS=
# LDLIBS=
SRCS=main.c vlist.c
OBJS=main.o vlist.o
TARGET=vlist
OSFLAG=
EXE=a.exe

ifneq ($(OS),Windows_NT)
	CC=gcc
	EXE=a.out
	CFLAGS+=-pedantic 
endif

all: $(TARGET)

vlist: $(OBJS)
	$(CC) $(CFLAGS) -c $(SRCS)
	$(CC) -o $(EXE) $(OBJS)


clean:
	del .\*.o
	del .\*.exe
