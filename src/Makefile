BUILD=../build

IFLAGS= -I../deps

OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjev.o mjmap.o mjtcpsrv.o mjconn.o mjhttpreq.o mjreg.o mjpool.o mjthreadpool.o mjtcpsrvt.o mjsig.o mjsql.o mjopt.o mjworker.o mjproto_http.o mjhttprsp.o mjtcpsrvm.o mjio.o mjjson.o mjtcpsrvtp.o mjthread.o mjthreadpool2.o mjlf.o

JPWDIR=../deps/libjpw
JPWOBJS=$(JPWDIR)/json_parser.o $(JPWDIR)/jpw.o $(JPWDIR)/jpw_objects.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 $(IFLAGS) -Wall -c $< -o $@

all:$(OBJS)
	gcc -fPIC -shared $(OBJS) $(JPWOBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS) $(JPWOBJS)

clean:
	rm -rf $(OBJS)
