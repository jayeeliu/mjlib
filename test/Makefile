INCLUDE=../src
LIB=../build

TARGET=mjstr_t

%_t:%_t.c
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql/ -lpthread -lmysqlclient -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
