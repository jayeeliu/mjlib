OBJS=sf_string.o sf_module.o sf_rbtree.o sf_main.o sf_stub.o sf_worker.o sf_timer.o sf_epoll.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -pthread -Wall -c $< -o $@


all:$(OBJS)
	gcc -pg -g -std=gnu99 -lpthread $(OBJS) -o sf

clean:
	rm -rf $(OBJS)
	rm -rf sf
