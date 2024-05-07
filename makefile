CC := gcc
CFLAGS := -w -g
SRC_FILES := main.c
OBJ_FILES := $(SRC_FILES:.c=)
.DEFAULT_GOAL := all

.PHONY: all compile run clean

all: compile

compile: $(OBJ_FILES)

$(OBJ_FILES): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -lncurses -lpthread

run_%: %
	./$<

run: run_main

clean_%:
	rm -f $*

clean:
	rm -f $(OBJ_FILES)