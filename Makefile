CC:=clang
SRC:=src
BUILD:=build
TEST:=test

COMPILE_FLAGS:=-std=c11 -Wall -Werror

.PHONY: build run test clean debug release

default: build run

debug: COMPILE_FLAGS +=-g -O0
debug: build run

release: COMPILE_FLAGS +=-O3
release: build run

build:
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/str.c -o $(BUILD)/str.o
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/tokens.c -o $(BUILD)/tokens.o
	$(CC) $(COMPILE_FLAGS) $(SRC)/main.c $(BUILD)/tokens.o $(BUILD)/dynarray.o $(BUILD)/str.o -o $(BUILD)/clexer

run:
	./$(BUILD)/clexer simpletest.c

test: COMPILE_FLAGS +=-O3
test:
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/dynarray.o $(TEST)/dyntest.c -o $(BUILD)/dyntest
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/ast.c -o $(BUILD)/ast.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/ast.o $(TEST)/asttest.c -o $(BUILD)/asttest
	./$(BUILD)/dyntest
	./$(BUILD)/asttest

clean:
	rm -r $(BUILD)/*

