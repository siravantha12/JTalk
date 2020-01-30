EXECUTABLES = jtalk jtalk_server 

CFLAGS = -I/home/plank/cs360/include
CSDIR = /home/plank/cs360
LIB = $(CSDIR)/objs/libfdr.a 

CC = gcc

all: $(EXECUTABLES)

clean:
	rm -f core *.o $(EXECUTABLES) a.out

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c


jtalk: jtalk.o
	$(CC) $(CFLAGS) -o jtalk jtalk.o -lpthread socketfun.o $(LIB)

jtalk_server: jtalk_server.o
	$(CC) $(CFLAGS) -o jtalk_server jtalk_server.o -lpthread socketfun.o $(LIB)
