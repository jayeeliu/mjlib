OBJS=sf_str.o sf_module.o sf_object.o sf_rbtree.o sf_main.o sf_stub.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -pthread -Wall -Wpadded -c $< -o $@


all:$(OBJS)
	gcc -pg -g -std=gnu99 -lpthread $(OBJS) -o sf

clean:
	rm -rf $(OBJS)
	rm -rf sf
