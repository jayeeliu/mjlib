INCLUDE=../src
DEPS=../deps
LIB=../build

#TARGET=proxy getopt
TARGET=getopt

%:%.c
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -I$(DEPS) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
