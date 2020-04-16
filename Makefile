CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
CFLAGS=-g -Wall -Werror #-D DEBUG
# LDFLAGS=
# LDLIBS=
SRCS=main.c vlist.c
OBJS=main.o vlist.o
TARGET=vlist
OSFLAG=

ifneq ($(OS),Windows_NT)
	CC=gcc
endif

all: $(TARGET)

vlist: $(OBJS)
	$(CC) $(CFLAGS) -c $(SRCS)
	$(CC) -o a $(OBJS)


clean:
	del .\*.o
	del .\*.exe
