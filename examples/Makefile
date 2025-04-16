CC = clang

all: arithmetic noschema

arithmetic: arithmetic.c ../CLargs.c
	$(CC) -lm arithmetic.c ../CLargs.c -o arithmetic
noschema: noschema.c ../CLargs.c
	$(CC) noschema.c ../CLargs.c -o noschema