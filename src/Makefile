BUILD=../build

IFLAGS= -I../deps

#OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjmap.o mjhttpreq.o mjreg.o mjpool.o mjsig.o mjsql.o mjopt.o mjworker.o mjproto_http.o mjhttprsp.o mjio.o mjjson.o mjthread.o mjthreadpool.o mjlf.o mjproto_txt.o mjmainsrv.o mjconn.o mjev.o mjtcpsrv.o mjmd5.o mjsort.o mjrbtree.o
OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjmap.o mjhttpreq.o mjreg.o mjpool.o mjsig.o mjsql.o mjopt.o mjworker.o mjhttprsp.o mjio.o mjjson.o mjthread.o mjthreadpool.o mjproto_txt.o mjconn.o mjev.o mjtcpsrv.o mjmd5.o mjsort.o mjrbtree.o mjlf.o mjmainsrv.o mjproto_http.o 

JPWDIR=../deps/libjpw
JPWOBJS=$(JPWDIR)/json_parser.o $(JPWDIR)/jpw.o $(JPWDIR)/jpw_objects.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 $(IFLAGS) -pthread -Wall -c $< -o $@

all:$(OBJS)
	gcc -g -fPIC -shared -pthread $(OBJS) $(JPWOBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS)

test:
	gcc -pg -g -std=gnu99 mjsig.c mjcomm.c mjpq.c mjev.c mjstr2.c mjtcpsrv.c mjlog.c mjsock.c mjconn.c mjmap.c mjtcpsrv_t.c -o mjtcpsrv_t

test_str:
	gcc -std=gnu99 -g mjlog.c mjstr2.c mjstr2_t.c -o mjstr2_t

clean:
	rm -rf $(OBJS)
	rm -rf mjtcpsrv_t gmon.out
