
PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)
LIB_SUFFIX:=.dylib

%.dylib: %.o
	gcc -dynamiclib -undefined suppress -flat_namespace $< -o $@

else
LIB_SUFFIX:=.so
	CFLAGS:=-fPIC

%.so: %.o
	gcc -shared $< -o $@

endif

SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)
LIBS=$(SRCS:.c=$(LIB_SUFFIX))

default: $(LIBS)

clean:
	rm -f *$(LIB_SUFFIX) *.o


