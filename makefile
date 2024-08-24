exc: ./test/sim_tcp.c ./tcp/oldtcp.c ./a_queue/a_queue.c ./threadpool/threadpool.c
	gcc -o $@  ./test/sim_tcp.c ./tcp/oldtcp.c ./a_queue/a_queue.c ./threadpool/threadpool.c -lWs2_32 -pthread