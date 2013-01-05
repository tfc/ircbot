EXECUTABLE=bot
OBJECTS=main.o irc.o module_support.o
CFLAGS:=-g -Wall -Werror `pkg-config --cflags glib-2.0`
LIBS:=`pkg-config --libs glib-2.0`
TESTSERVER=localhost

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)
else
	CFLAGS:=$(CFLAGS) -D_GNU_SOURCE
	LIBS:=$(LIBS) -ldl
endif

all: $(EXECUTABLE)
	$(MAKE) -C modules

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
	make -C modules clean

run: all
	./$(EXECUTABLE) $(TESTSERVER) 6667
