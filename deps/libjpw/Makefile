BUILD	= ../build
OBJS	= json_parser.o jpw.o jpw_objects.o

.PHONY: clean

.c.o:
	gcc -pg -g -fPIC -c $*.c

default: $(OBJS)
	ar -r libjpw.a $(OBJS)
clean:
	rm -f $(OBJS)
	rm -f libjpw.a


