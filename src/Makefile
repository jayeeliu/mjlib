BUILD=../build
INCLUDE=../include

OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjev.o mjmap.o mjtcpsrv.o mjconn.o mjhttpreq.o mjreg.o mjpool.o mjthreadpool.o mjtcpsrvt.o mjsig.o mjsql.o mjopt.o mjworker.o mjproto_http.o mjhttprsp.o mjtcpsrvm.o mjio.o

.c.o:
	gcc -pg -g -fPIC -std=gnu99 -I$(INCLUDE) -Wall -c $< -o $@

all:$(OBJS)
	gcc -fPIC -shared $(OBJS) -o $(BUILD)/libmj.so
	ar -r $(BUILD)/libmj.a $(OBJS)

clean:
	rm -rf $(OBJS)
	rm -rf $(BUILD)/libmj.a $(BUILD)/libmj.so
