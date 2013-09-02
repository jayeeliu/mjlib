INCLUDE=../src
INCLUDE_DEP=../deps
LIB=../build

TARGET=mjredis_t

%_t:%_t.c
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -I$(INCLUDE_DEP) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
