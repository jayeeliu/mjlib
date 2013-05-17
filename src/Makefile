BUILD=../build

IFLAGS= -I../deps

OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjmap.o mjhttpreq.o mjreg.o mjpool.o mjsig.o mjsql.o mjopt.o mjworker.o mjproto_http.o mjhttprsp.o mjio.o mjjson.o mjthread.o mjthreadpool.o mjlf.o mjproto_txt.o mjmainsrv.o mjconn.o mjev.o mjtcpsrv.o

JPWDIR=../deps/libjpw
JPWOBJS=$(JPWDIR)/json_parser.o $(JPWDIR)/jpw.o $(JPWDIR)/jpw_objects.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 $(IFLAGS) -pthread -Wall -c $< -o $@

all:$(OBJS)
	gcc -g -fPIC -shared -pthread $(OBJS) $(JPWOBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS)

clean:
	rm -rf $(OBJS)
