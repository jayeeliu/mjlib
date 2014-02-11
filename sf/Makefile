BUILD=../build

IFLAGS= -I../deps

OBJS=mjstr.o mjsock.o mjlog.o mjconb.o mjcomm.o mjpq.o mjmap.o mjhttpreq.o mjreg.o mjsig.o mjsql.o mjopt.o mjhttprsp.o mjio.o mjjson.o mjthread.o mjthreadpool.o mjlf_txt.o mjconn.o mjev.o mjsrv.o mjmd5.o mjsort.o mjrbtree.o mjlf.o mjproto_http.o mjredis.o mjlockless.o mjpool.o mjvec.o mjlf_pb.o 

JPWDIR=../deps/libjpw
JPWOBJS=$(JPWDIR)/libjpw.a

REDISDIR=../deps/hiredis
REDISOBJS=$(REDISDIR)/libhiredis.a

.c.o:
	gcc -pg -g -fPIC -std=gnu99 $(IFLAGS) -pthread -Wall -Wpadded -c $< -o $@
#	gcc -O3 -fPIC -std=gnu99 $(IFLAGS) -pthread -Wall -c $< -o $@

all:$(OBJS)
	gcc -g -fPIC -shared -pthread $(OBJS) $(JPWOBJS) $(REDISOBJS) -o $(BUILD)/libmj.so
#	gcc -O3 -fPIC -shared -pthread $(OBJS) $(JPWOBJS) $(REDISOBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS) $(REDISOBJS)

clean:
	rm -rf $(OBJS)
