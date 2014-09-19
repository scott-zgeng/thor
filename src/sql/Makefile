CC=gcc
CXX=g++


CFLAGS=
LIBS=


CFLAGS+=-g -O2 -DSQLITE_OS_UNIX=1 -I. -I./sqlite -I./ext/rtree -D_HAVE_SQLITE_CONFIG_H -DBUILD_sqlite -DNDEBUG
CFLAGS+=-I/usr/local/include -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_TEMP_STORE=1
CFLAGS+=-fPIC -DPIC -pthread


OBJS=$(patsubst sqlite/%.c, obj/%.o, $(wildcard sqlite/*.c))

BIN=bin/thor

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJS):obj/%.o:sqlite/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf obj/*.o $(BIN)


