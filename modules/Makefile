
PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)
MODULE_SUFFIX:=.dylib

%.dylib: %.o
	gcc -dynamiclib -undefined suppress -flat_namespace $< -o $@

else
MODULE_SUFFIX:=.so
	CFLAGS:=-fPIC

%.so: %.o
	gcc -shared $< -o $@

endif

SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)
LIBS=$(SRCS:.c=$(MODULE_SUFFIX))

default: $(LIBS)

clean:
	rm -f *$(MODULE_SUFFIX) *.o


