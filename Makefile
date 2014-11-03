#
# Programming Assignment #1 Makefile
#
# -lsocket option:  Removed. You will need to add this ahead
#    of -lnsl on Solaris machines.
#

CC     = gcc
CFLAGS = -g 

all: SendApp 

SendApp: sender.o srmp.o wraparound.o 
	$(CC) -o $@ sender.o srmp.o wraparound.o 

sender.o receiver.o srmp.o wraparound.o: srmp.h

spotless:  
	rm *.o SendApp
