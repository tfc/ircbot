CFLAGS:=-g -Wall -Werror `pkg-config --cflags glib-2.0`
LIBS:=`pkg-config --libs glib-2.0`

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)
MODULE_SUFFIX:=.dylib

%.dylib: %.o
	gcc -dynamiclib -undefined suppress -flat_namespace $< -o $@

else
MODULE_SUFFIX:=.so
FLAGS:=$(CFLAGS) -fPIC -D_GNU_SOURCE
LIBS:=$(LIBS) -ldl

%.so: %.o
	gcc -shared $< -o $@

endif

SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)
LIBS=$(SRCS:.c=$(MODULE_SUFFIX))

default: $(LIBS)

clean:
	rm -f *$(MODULE_SUFFIX) *.o


