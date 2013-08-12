INCLUDE=../src
LIB=../build

#TARGET=proxy getopt
TARGET=getopt

%:%.c
	gcc -pg -g -std=gnu99 -pthread -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

all:$(TARGET)

clean:
	rm -rf $(TARGET) gmon.out
