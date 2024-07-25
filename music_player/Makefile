DEBUG=-DARM
#DEBUG=
Target=main

Object=main.o select.o device.o player.o link.o socket.o

CC=arm-linux-gcc
#CC=gcc

CFLAGS=-L /home/huang/json_arm_install/lib -I /home/huang/json_arm_install/include $(DEBUG)
#CFLAGS=

$(Target):$(Object)
	$(CC) -o $(Target) $(Object) -Wall -ljson $(DEBUG) $(CFLAGS) -static
clean:
	rm *.o main
