OBJS=mjstr.o mjsock.o mjlog.o mjconnb.o mjcomm.o mjpq.o mjev.o mjmap.o mjtcpsrv.o mjconn.o mjhttpreq.o mjreg.o mjpool.o mjthreadpool.o mjtcpsrvt.o mjsig.o mjsql.o mjopt.o mjworker.o mjproto_http.o mjhttprsp.o mjtcpsrvm.o
TARGET=mjstr_t mjsock_t mjlist_t mjconnb_t mjcomm_t mjpq_t mjev_t mjmap_t mjconn_t mjtcpsrv_t mjreg_t mjpool_t mjlog_t mjthreadpool_t mjtcpsrvt_t mjsig_t mjsql_t mjopt_t mjconn2_t mjworker_t mjconnb2_t mjproto_http_t mjtcpsrvm_t

.c.o:
	gcc -pg -g -std=gnu99 -Wall -c $<

%_t:%_t.c $(OBJS)
	gcc -pg -g -std=gnu99 -L/usr/lib64/mysql/ -lmysqlclient -lpthread -Wall $(OBJS) $< -o $@

all:$(OBJS) $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET) gmon.out
