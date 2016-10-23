CC=gcc
CFLAGS=-Wall -g -ansi
#DEPS = parser.h
#OBJ = parser-encoder.o parser-decoder.o

all: encode decode #test

encode: parser.o zatoencoder.c differential.o List.o utils.h runlength.o huffman.o
	$(CC) zatoencoder.c parser.o differential.o List.o runlength.o huffman.o -o encode $(CFLAGS)

decode: parser.o zatodecoder.c differential.o List.o utils.h runlength.o huffman.o
	$(CC) zatodecoder.c parser.o differential.o List.o runlength.o huffman.o -o decode $(CFLAGS)

# $(OBJ): %.o: %.c $(DEPS)
# 	$(CC) $< -c $(CFLAGS)

parser: parser.c parser.h
	$(CC) parser.c -c $(CFLAGS)

List: List.c List.h
	$(CC) List.c -c $(CFLAGS)

differential: differential.c differential-base.h differential.h
	$(CC) differential.c -c $(CFLAGS)

huffman: huffman.c List.h utils.h
	$(CC) huffman.c -c $(CFLAGS)

runlength: runlength.c List.h utils.h
	$(CC) runlength.c -c $(CFLAGS)

# test: dtest.c differential.o reader.o
# 	$(CC) dtest.c reader.o differential.o -o test $(CFLAGS)

# reader: reader.c reader.h
# 	$(CC) reader.c -c $(CFLAGS)

clean:
	rm *.o encode decode #test
