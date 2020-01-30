EXECUTABLES = jtalk cat_server simpcat 

CFLAGS = -I/home/plank/cs360/include
CSDIR = /home/plank/cs360
//SOCKLIB = $(CSDIR)/objs/socketfun.o 
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

cat_server: cat_server.o simpcat
	$(CC) $(CFLAGS) -o cat_server cat_server.o socketfun.o

simpcat: simpcat.o
	$(CC) $(CFLAGS) -o simpcat simpcat.o 

