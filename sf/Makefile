OBJS=sf_str.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -pthread -Wall -Wpadded -c $< -o $@

%_t:%_t.c $(OBJS)
	gcc -pg -g -std=gnu99 $(OBJS) $< -o $@

all:$(OBJS)

test:sf_str_t

clean:
	rm -rf $(OBJS)
	rm -rf gmon.out
	rm -rf *_t
