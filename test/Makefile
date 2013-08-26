INCLUDE=../src
LIB=../build

#TARGET=mjstr_t mjsock_t mjlist_t mjconnb_t mjcomm_t mjpq_t mjmap_t mjtcpsrv_t mjreg_t mjpool_t mjlog_t mjsig_t mjsql_t mjopt_t mjworker_t mjconnb2_t mjproto_http_t mjio_t msg_t server1_t router_t mjthread_t mjthread2_t mjthreadpool_t mjlf_t mjconn_t mjmainsrv_t mjev_t mjmd5_t mjsort_t mjproto_http2_t
TARGET=mjtcpsrv_t

%_t:%_t.c
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
