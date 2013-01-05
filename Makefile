EXECUTABLE=bot
OBJECTS=main.o irc.o plugin.o
CFLAGS:=-g -Wall -Werror `pkg-config --cflags glib-2.0`
LIBS:=`pkg-config --libs glib-2.0`
TESTSERVER=localhost

all: $(EXECUTABLE)
	$(MAKE) -C plugins

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(TESTSERVER) 6667
