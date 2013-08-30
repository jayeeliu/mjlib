INCLUDE=../src
LIB=../build

DPCFILES=dpc.c validator.c *store.c

TARGET=dpc

all:$(TARGET)

dpc: $(DPCFILES)
	gcc -pg -g -std=gnu99 -pthread -I. -I$(INCLUDE) -L$(LIB) -L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall $< -o $@

clean:
	-rm -rf $(TARGET)
