OBJS=sf_str.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -pthread -Wall -Wpadded -c $< -o $@

%_t:%_t.c $(OBJS)
	gcc -pg -g $(OBJS) $< -o $@

test:sf_str_t

all:$(OBJS)
#	gcc -g -fPIC -shared -pthread $(OBJS) $(JPWOBJS) $(REDISOBJS) -o $(BUILD)/libmj.so
#	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS) $(REDISOBJS)
	gcc $(OBJS)

clean:
	rm -rf $(OBJS)
	rm -rf gmon.out
	rm -rf *_t
