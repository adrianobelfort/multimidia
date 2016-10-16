CC=gcc
CFLAGS=-Wall -g
DEPS = parser.h
OBJ = parser-encoder.o parser-decoder.o

all: $(OBJ) encode decode

encode: parser-encoder.o zatoencoder.c differential.o List.o utils.h
	$(CC) zatoencoder.c parser-encoder.o differential.o List.o -o encode $(CFLAGS)

decode: parser-decoder.o zatodecoder.c differential.o List.o utils.h
	$(CC) zatodecoder.c parser-decoder.o differential.o List.o -o decode $(CFLAGS)

$(OBJ): %.o: %.c $(DEPS)
	$(CC) $< -c $(CFLAGS)

differential: differential.c differential-base.h differential.h
	$(CC) differential.c -c $(CFLAGS)

runlength: List.c List.h
	$(CC) List.c -c $(CFLAGS)

clean:
	rm *.o encode decode
