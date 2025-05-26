CC:=clang
SRC:=src
BUILD:=build
TEST:=test

default: compile run

compile:
	$(CC) -c $(SRC)/libs/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) -c $(SRC)/tokens.c -o $(BUILD)/tokens.o
	$(CC) $(SRC)/main.c $(BUILD)/tokens.o $(BUILD)/dynarray.o -o $(BUILD)/main

run:
	./$(BUILD)/main test.c

tests:
	$(CC) -c $(SRC)/libs/dynarray.c -o $(BUILD)/dynarray.o
	$(CC) $(BUILD)/dynarray.o $(TEST)/dyntest.c -o $(BUILD)/dyntest
	./$(BUILD)/dyntest

clean:
	rm $(BUILD)/*

