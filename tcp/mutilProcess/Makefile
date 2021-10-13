all:server client
  
server:server.o wrap.o
    gcc $^ -o $@

client:client.o wrap.o
    gcc $^ -o $@

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
