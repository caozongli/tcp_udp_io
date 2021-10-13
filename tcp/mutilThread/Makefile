all:server client
  
server:server.o wrap.o
	gcc $^ -o $@ -lpthread

client:client.o wrap.o
	gcc $^ -o $@ -lpthread

server.o:server.c
	gcc -c $^ -o $@

client.o:client.c
	gcc -c $< -o $@

wrap.o:wrap.c
	gcc -c $^ -o wrap.o

.PTHONY:clean
clean:
	rm -f server
	rm -f client
	rm -f *.o
