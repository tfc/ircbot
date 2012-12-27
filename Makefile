EXECUTABLE=bot
OBJECTS=main.o
CFLAGS:=-g -Wall
TESTSERVER=127.0.0.1
#TESTSERVER=clanserver4u1.de.quakenet.org

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(TESTSERVER) 6667
