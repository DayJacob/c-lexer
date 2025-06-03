CC:=clang
SRC:=src
BUILD:=build
TEST:=test

COMPILE_FLAGS:=-std=c11 -Wall -Werror

SRC_FILES:=$(wildcard $(SRC)/*.c $(SRC)/utils/*.c)
OBJ_FILES:=$(patsubst $(SRC)/%.c, $(BUILD)/obj/%.o, $(SRC_FILES))

.PHONY: build run test clean debug release

default: build run

debug: COMPILE_FLAGS +=-g -O0
debug: build run

release: COMPILE_FLAGS +=-O3
release: build run

build: $(BUILD)/clexer

$(BUILD)/clexer: $(OBJ_FILES)
	@$(CC) $(COMPILE_FLAGS) -o $(BUILD)/clexer $^

$(BUILD)/obj/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(COMPILE_FLAGS) -c -o $@ $<

run:
	./$(BUILD)/clexer simpletest.c

test: COMPILE_FLAGS +=-O3
test:
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/dynarray.o $(TEST)/dyntest.c -o $(BUILD)/dyntest
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/ast.c -o $(BUILD)/ast.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/ast.o $(BUILD)/dynarray.o $(TEST)/asttest.c -o $(BUILD)/asttest
	./$(BUILD)/dyntest
	./$(BUILD)/asttest

clean:
	rm -rf $(BUILD)/obj/*
	rm -r $(BUILD)/clexer
	rm -r $(BUILD)/clexer.dSYM

