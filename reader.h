#ifndef __READER_H__
#define __READER_H__

#include "utils.h"

#define huge_t unsigned long long
#define BITS_PER_BYTE 8

FILE* openFile(char* filename, char *mode);

void closeFile(FILE* file);

long fileSize(FILE *file);

wav_hdr readHeader(FILE* file);

void printHeader(wav_hdr header);

enc_hdr readEncodeHeader(FILE* file);

void printEncodeHeader(enc_hdr header);

char* bitStreamToCharStream(char* bitStream, huge_t size, huge_t* charStreamSize, short *outstandingBits);

char* charStreamToBitStream(char* charStream, huge_t size, huge_t *bitStreamSize, short outstandingBits);

char* readData(FILE* file, wav_hdr* header);

char* extractFile(FILE* file, wav_hdr* header, huge_t *dataBitsSize);

char* extractEncodedFile(FILE* file, wav_hdr* header, enc_hdr *encodeHeader, huge_t *dataBitsSize);

size_t writeData(char* data, huge_t size, FILE* file);

size_t writeHeader(wav_hdr header, FILE* file);

size_t writeEncodeHeader(enc_hdr header, FILE* file);

size_t writeEncodedFile(char* data, huge_t size, wav_hdr header, enc_hdr encodeHeader, FILE* file);

size_t writeFile(char* data, huge_t size, wav_hdr header, FILE* file);

#endif
