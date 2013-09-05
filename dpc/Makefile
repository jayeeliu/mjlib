INCLUDE=../src
INCLUDE_DEP=../deps
LIB=../build

DPCFILES=dpc.c dpc_util.c *store.c

TARGET=dpc

all:$(TARGET)

dpc: $(DPCFILES)
	gcc -pg -g -std=gnu99 -pthread -I. -I$(INCLUDE) -I$(INCLUDE_DEP) -L$(LIB) \
		-L/usr/lib64/mysql -lmysqlclient -lpthread -lmj -Wall \
		$(DPCFILES) -o $@

clean:
	-rm -rf $(TARGET)
