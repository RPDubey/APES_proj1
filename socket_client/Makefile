#*******************************************************************************
   #Filename:Makefile
   #Brief: Makefile for socket client
   #Author:Ravi Dubey
   #Date:3/14/2018
 #*****************************************************************************/

client.elf:client.c client_test.o
	gcc -g -o client.elf client.c client_test.o -lrt -pthread

client_test.o:client_test.c
	gcc -c -o client_test.o client_test.c -lrt -pthread

clean:
	rm -f client.elf
