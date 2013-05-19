CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude -I/usr/include/postgresql $(INC)
LFLAGS := -levent -lpq
CC     := gcc
BINARY := kismet2postgres
DEPS   := build/main.o build/debug.o build/config.o build/callbacks.o build/query_printf.o build/postgres.o

.PHONY: all clean

all: build $(DEPS) bin/$(BINARY)

build:
	-mkdir -p build bin

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INC) -c -o build/main.o src/main.c

build/debug.o: src/debug.c include/debug.h
	$(CC) $(CFLAGS) $(INC) -c -o build/debug.o src/debug.c

build/config.o: src/config.c include/config.h
	$(CC) $(CFLAGS) $(INC) -c -o build/config.o src/config.c

build/callbacks.o: src/callbacks.c include/callbacks.h
	$(CC) $(CFLAGS) $(INC) -c -o build/callbacks.o src/callbacks.c

build/query_printf.o: src/query_printf.c include/query_printf.h
	$(CC) $(CFLAGS) $(INC) -c -o build/query_printf.o src/query_printf.c

build/postgres.o: src/postgres.c include/postgres.h
	$(CC) $(CFLAGS) $(INC) -c -o build/postgres.o src/postgres.c

bin/$(BINARY): $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

clang:
	$(MAKE) CC=clang
