all:main server



main:main.o threadpool.o
	gcc $^ -o $@ -lpthread

main.o:main.c
	gcc -c $^ -o $@

server:server.o threadpool.o wrap.o
	gcc $^ -o $@ -lpthread

server.o:server.c
	gcc -c $^ -o $@

wrap.o:wrap.c
	gcc -c $< -o $@

threadpool.o:threadpool.c
	gcc -c $< -o $@

.PTHONY:clean
clean:
	rm -f *.o
