CC:=clang
SRC:=src
BUILD:=build
TEST:=utils-test

COMPILE_FLAGS:=-std=c11 -Wall -Werror

SRC_FILES:=$(wildcard $(SRC)/*.c $(SRC)/utils/*.c)
OBJ_FILES:=$(patsubst $(SRC)/%.c, $(BUILD)/obj/%.o, $(SRC_FILES))
TEST_CASES:=$(wildcard test-cases/*.c)

.PHONY: build test cases clean debug release

default: build

debug: COMPILE_FLAGS +=-g -O0
debug: build cases

release: COMPILE_FLAGS +=-O3
release: build cases

build: $(BUILD)/clexer

$(BUILD)/clexer: $(OBJ_FILES)
	@$(CC) $(COMPILE_FLAGS) -o $(BUILD)/minic $^

$(BUILD)/obj/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(COMPILE_FLAGS) -c -o $@ $<

test: COMPILE_FLAGS +=-O3
test:
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/dynarray.o $(TEST)/dyntest.c -o $(BUILD)/dyntest
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/ast.c -o $(BUILD)/ast.o
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/arena.c -o $(BUILD)/arena.o
	$(CC) $(COMPILE_FLAGS) -c $(SRC)/utils/llvm.c -o $(BUILD)/llvm.o
	$(CC) $(COMPILE_FLAGS) $(BUILD)/ast.o $(BUILD)/dynarray.o $(BUILD)/arena.o $(BUILD)/llvm.o $(TEST)/asttest.c -o $(BUILD)/asttest
	./$(BUILD)/dyntest
	./$(BUILD)/asttest

cases: $(TEST_CASES)
	./$(BUILD)/minic $<

clean:
	rm -rf $(BUILD)/obj/*
	rm -r $(BUILD)/minic

