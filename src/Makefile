CC=clang
WARNINGFLAGS=-Wall
CFLAGS=-I. $(WARNINGFLAGS) -g -O0
LDFLAGS=
OBJECTS=main.o db.o btree.o storage.o util.o

EXECUTABLE=ldb

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
