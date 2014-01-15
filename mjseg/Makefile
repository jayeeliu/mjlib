%:%.c

OBJS=mjstr.o mjlog.o mjseg.o mjlf.o mjconnb.o mjthreadpool.o mjthread.o mjsock.o mjmap.o mjsig.o mjlockless.o mjcomm.o mjlf_txt.o main.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -Wall -c $< -o $@

all:$(OBJS)
	gcc -pg -g -lstdc++ -lpthread $(OBJS) libmbsegc_64.a -o mjseg

clean:
	rm -rf $(OBJS) mjseg gmon.out
