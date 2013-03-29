INCLUDE=../include
LIB=../build

TARGET=mjstr_t mjsock_t mjlist_t mjconnb_t mjcomm_t mjpq_t mjev_t mjmap_t mjconn_t mjtcpsrv_t mjreg_t mjpool_t mjlog_t mjthreadpool_t mjtcpsrvt_t mjsig_t mjsql_t mjopt_t mjconn2_t mjworker_t mjconnb2_t mjproto_http_t mjtcpsrvm_t mjio_t msg_t server1_t router_t

%_t:%_t.c
	gcc -pg -g -std=gnu99 -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
