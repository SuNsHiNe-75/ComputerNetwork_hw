all:	server client

server: server.o mytcp.o
	gcc server.o mytcp.o -o server -lm

client: client.o mytcp.o
	gcc client.o mytcp.o -o client -lm

server.o: server.c
	gcc -c server.c

client.o: client.c
	gcc -c client.c

mytcp.o: mytcp.c mytcp.h
	gcc -c mytcp.c mytcp.h

clean:
	rm *.o
	rm server
	rm client
