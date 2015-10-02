#
# qthreads Makefile
#

EXES = test1 philosopher server
LIBS = libqthread.a 
CFLAGS = -g -I.
QTH = qthread.o

all: $(LIBS) $(EXES)

clean:
	rm -f *.o $(LIBS) $(EXES)

libqthread.a: $(QTH) do-switch.o 
	ar r libqthread.a $?

test1: test1.o libqthread.a
	gcc -g test1.o -o test1 -L. -lqthread 

philosopher: philosopher.o libqthread.a
	gcc -g philosopher.o -o philosopher -L. -lqthread -lm

server: server.o libqthread.a
	gcc -g server.o -o server -L. -lqthread
