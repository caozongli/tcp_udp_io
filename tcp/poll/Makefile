server:server.o wrap.o
	gcc $^ -o $@

server.o:server.c
	gcc -c $< -o server.o

wrap.o:wrap.c
	gcc -c wrap.c -o wrap.o

.PTHONY:clean
clean:
	rm -f *.o
	rm server
