INCLUDE=../src
LIB=../build

TARGET=mjlf_txt_t

%_t:%_t.c
#	gcc -O3 -std=gnu99 -pthread -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql/ -lpthread -lmysqlclient -lmj -Wall $< -o $@
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql/ -lpthread -lmysqlclient -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
