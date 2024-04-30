CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include

all: client server test/test-insert test/test-delete test/test-search test/test-multi-req

client:
	gcc -o client src/client.c src/hash-table.c -Iinclude


server:
	gcc -o server src/server.c src/hash-table.c -Iinclude

test/test-insert:
	gcc -o test/test-insert test/test-hash-table-insert.c src/hash-table.c -Iinclude

test/test-delete:
	gcc -o test/test-delete test/test-hash-table-delete.c src/hash-table.c -Iinclude

test/test-search:
	gcc -o test/test-search test/test-hash-table-search.c src/hash-table.c -Iinclude

test/test-multi-req:
	gcc -o test/test-multi-req test/test-client-send-multi-req.c src/hash-table.c -Iinclude

clean:
	rm -f client server test/test-insert test/test-delete test/test-search test/test-multi-req