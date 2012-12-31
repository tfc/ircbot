EXECUTABLE=bot
OBJECTS=main.o irc.o
CFLAGS:=-g -Wall -Werror
TESTSERVER=localhost

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(TESTSERVER) 6667
