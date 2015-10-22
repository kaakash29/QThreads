#
# qthreads Makefile
#

EXES = test1 philosopher server sleep_test mutex_test exit_join_test condition_test io_test
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

sleep_test: sleep_test.o libqthread.a
	gcc -g sleep_test.o -o sleep_test -L. -lqthread

mutex_test: mutex_test.o libqthread.a
	gcc -g mutex_test.o -o mutex_test -L. -lqthread

exit_join_test: exit_join_test.o libqthread.a
	gcc -g exit_join_test.o -o exit_join_test -L. -lqthread

condition_test: condition_test.o libqthread.a
	gcc -g condition_test.o -o condition_test -L. -lqthread

io_test: io_test.o libqthread.a
	gcc -g io_test.o -o io_test -L. -lqthread

test:
	./test1 | grep -I "PASS"
	./sleep_test -v | ./greenest | grep -I "PASS"
	./mutex_test -v | ./greenest | grep -I "PASS"
	./exit_join_test -v | ./greenest | grep -I "PASS"
	./condition_test -v | ./greenest | grep -I "PASS"
	./io_test -v | ./greenest | grep -I "PASS"	
