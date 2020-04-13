CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
CFLAGS=-g -Wall -Werror
# LDFLAGS=
# LDLIBS=
SRCS=main.c
OBJS=main.o
TARGET=vlist

all: $(TARGET)

vlist: $(OBJS)
	$(CC) -c $(SRCS)
	$(CC) -o vlist $(OBJS)


clean:
	del .\*.o
	del $(TARGET).exe
