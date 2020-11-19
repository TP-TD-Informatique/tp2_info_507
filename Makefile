###
# info507
# TP2
# Kevin Traini
###


# compiler
# CC := clang
CC := gcc

# compilation flags
# CFLAGS := --std=c99 -Wall -Wextra -pedantic
CFLAGS := --std=c99 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter


# files to compile
FILES = $(wildcard question*.c) tinygc.c

# corresponding executable files
EXE := $(FILES:.c=)


# default target
all: $(EXE)

gcc: CC = gcc
gcc: clean all

clang: CC = clang
clang: clean all

# compiling a single file
$(EXE):%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# remove everything except source files
clean:
	rm -f a.out *.o $(EXE)

.PHONY: all clean gcc clang
