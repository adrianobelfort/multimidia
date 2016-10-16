CC=gcc
CFLAGS=-Wall -ansi -g
DEPS = parser.h
OBJ = parser-encoder.o parser-decoder.o

all: $(OBJ) encode decode

encode: parser-encoder.o zatoencoder.c differential.o
	$(CC) zatoencoder.c parser-encoder.o differential.o -o encode $(CFLAGS)

decode: parser-decoder.o zatodecoder.c differential.o
	$(CC) zatodecoder.c parser-decoder.o differential.o -o decode $(CFLAGS)

$(OBJ): %.o: %.c $(DEPS)
	$(CC) $< -c $(CFLAGS)

differential: differential.c differential-base.h differential.h
	$(CC) differential.c -c $(CFLAGS)

clean:
	rm *.o encode decode
