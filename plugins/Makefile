SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)
LIBS=$(SRCS:.c=.dylib)

all: $(LIBS)

%.dylib: %.o
	gcc -dynamiclib -undefined suppress -flat_namespace $< -o $@


