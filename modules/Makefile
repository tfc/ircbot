CFLAGS:=-g -Wall -Werror `pkg-config --cflags glib-2.0` `curl-config --cflags`
LIBS:=`pkg-config --libs glib-2.0` `curl-config --libs`

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)

MODULE_SUFFIX:=.dylib

%.dylib: %.o
	$(CC) -dynamiclib -undefined suppress -flat_namespace $< -o $@ $(LIBS)

else

MODULE_SUFFIX:=.so
CFLAGS:=$(CFLAGS) -fPIC -D_GNU_SOURCE
LIBS:=$(LIBS) -ldl -rdynamic

%.so: %.o
	$(CC) -shared $< -o $@ $(LIBS)

endif

SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)
MODULES=$(SRCS:.c=$(MODULE_SUFFIX))

default: $(MODULES)

clean:
	rm -f *$(MODULE_SUFFIX) *.o


