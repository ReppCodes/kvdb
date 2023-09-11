CC=gcc
LIBS=
CFLAGS=-g
SOURCE=src/kvdb.c
SRC_DIR = ./src
OUTPUT=kvdb 

all: build
build:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(SOURCE) $(LIBS)

format: 
	$(foreach file, $(wildcard $(SRC_DIR)/*), clang-format --style=Webkit -i $(file);)

clean:
	rm $(OUTPUT)
