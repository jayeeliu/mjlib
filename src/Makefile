BUILD=../build

IFLAGS= -I../deps

OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjmap.o mjhttpreq.o mjreg.o mjsig.o mjsql.o mjopt.o mjhttprsp.o mjio.o mjjson.o mjthread2.o mjthreadpool2.o mjproto_txt.o mjconn.o mjev.o mjtcpsrv.o mjmd5.o mjsort.o mjrbtree.o mjlf.o mjmainsrv.o mjproto_http.o mjredis.o mjlockless.o mjpool.o mjvec.o

JPWDIR=../deps/libjpw
JPWOBJS=$(JPWDIR)/libjpw.a

REDISDIR=../deps/hiredis
REDISOBJS=$(REDISDIR)/libhiredis.a

.c.o:
	gcc -pg -g -fPIC -std=gnu99 $(IFLAGS) -pthread -Wall -c $< -o $@

all:$(OBJS)
	gcc -g -fPIC -shared -pthread $(OBJS) $(JPWOBJS) $(REDISOBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS) $(REDISOBJS)

clean:
	rm -rf $(OBJS)
