all: posix_mq heap_mq

posix_mq: posix_mq.c
	gcc -g posix_mq.c -lpthread -lrt -o posix_mq

heap_mq: heap_mq.c
	gcc -g heap_mq.c -lpthread -lrt -o heap_mq

clean:
	rm -rf  posix_mq heap_mq