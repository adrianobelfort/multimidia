#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define huge_t unsigned long long
#define BITS_PER_BYTE 8

FILE* openFile(char* filename, char *mode)
{
	FILE* filePointer;

	filePointer = fopen(filename, mode);

	if (filePointer == NULL)
	{
		printf("Could not open .wav file %s\n", filename);
	}

	return filePointer;
}

void closeFile(FILE* filePointer)
{
	fclose(filePointer);
}

long fileSize(FILE *filePointer)
{
	long size;

	fseek(filePointer, 0, SEEK_END);
	size = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);

	return size;
}

void printHeader(wav_hdr header)
{
	printf("ChunkID: %s\n", header.RIFF);
	printf("ChunkSize: %u\n", header.ChunkSize);
	printf("AudioFormat: %d\n", header.AudioFormat);
	printf("NumOfChan: %d\n", header.NumOfChan);
	printf("SamplesPerSec: %u\n", header.SamplesPerSec);
	printf("bytesPerSec: %u\n", header.bytesPerSec);
	printf("bitsPerSample: %hu\n", header.bitsPerSample);
	printf("SubChunk2Size (data): %u\n", header.Subchunk2Size);
}

wav_hdr readHeader(FILE* filePointer)
{
	wav_hdr header;
	long readbytes;

	printf("Deve ler %ld\n", sizeof(header));
	readbytes = fread(&header, sizeof(header), 1, filePointer);
	printf("Bytes read: %ld\n", readbytes);

	printHeader(header);

	return header;
}

enc_hdr readEncodeHeader(FILE* filePointer)
{
	enc_hdr header;

	//fseek(filePointer, sizeof(wav_hdr), SEEK_SET);
	fread(&header, sizeof(header), 1, filePointer);

	return header;
}

void printEncodeHeader(enc_hdr header)
{
	printf("Encode Mode: %d\n\n", header.encodeType);
	printf("Run-length fields:\n");

	printf("RunlengthNumBits: %u\n", header.runlengthNumBits);
	printf("TotalLength: %llu\n\n", header.totalLength);

	printf("Difference fields:\n");
	printf("Difference length: %llu\n\n", header.differenceLength);
}

/*char* bitStreamToCharStream(char* bitStream, huge_t size, huge_t* charStreamSize)
{
	huge_t i;
	int shift;
	char *charStream;

	*charStreamSize = size / BITS_PER_BYTE;

	charStream = (char*) calloc(*charStreamSize, sizeof(char));

	for (i = 0; i < size; i++)
	{
		shift = (int) (i % BITS_PER_BYTE);
		charStream[i / BITS_PER_BYTE] |= bitStream[i] << shift;
	}

	return charStream;
}*/

char* bitStreamToCharStream(char* bitStream, huge_t size, huge_t* charStreamSize, short *outstandingBits)
{
	huge_t i;
	int shift;
	char *charStream;
	short leftoverBits;

	*charStreamSize = size / BITS_PER_BYTE;
	leftoverBits = size % BITS_PER_BYTE;

	if (leftoverBits != 0) *charStreamSize += 1;
	*outstandingBits = leftoverBits;

	charStream = (char*) calloc(*charStreamSize, sizeof(char));

	for (i = 0; i < size; i++)
	{
		shift = (int) (BITS_PER_BYTE - 1 - i % BITS_PER_BYTE);
		charStream[i / BITS_PER_BYTE] |= bitStream[i] << shift;
	}

	return charStream;
}

/*char* charStreamToBitStream(char* charStream, huge_t size, huge_t *bitStreamSize)
{
	huge_t i;
	int shift;
	char* bitStream;

	*bitStreamSize = size * BITS_PER_BYTE;

	bitStream = (char*) malloc(*bitStreamSize * sizeof(char));

	for (i = 0; i < *bitStreamSize; i++)
	{
		shift = (int) (i % BITS_PER_BYTE);
		bitStream[i] = (charStream[i / BITS_PER_BYTE] >> shift) & 0x1;
	}

	return bitStream;
}*/

char* charStreamToBitStream(char* charStream, huge_t size, huge_t *bitStreamSize, short outstandingBits)
{
	huge_t i;
	int shift;
	char* bitStream;

	*bitStreamSize = size * BITS_PER_BYTE - (BITS_PER_BYTE - outstandingBits);

	bitStream = (char*) malloc(*bitStreamSize * sizeof(char));

	for (i = 0; i < *bitStreamSize; i++)
	{
		shift = (int) (BITS_PER_BYTE - 1 - i % BITS_PER_BYTE);
		bitStream[i] = (charStream[i / BITS_PER_BYTE] >> shift) & 0x1;
	}

	return bitStream;
}

char* readData(FILE* filePointer, wav_hdr* header)
{
	char *dataBytes;

	printf("SIZE %u\n", header->Subchunk2Size);
	dataBytes = (char*) calloc(header->Subchunk2Size, sizeof(char));

	//fseek(filePointer, sizeof(wav_hdr), SEEK_SET);

	fread(dataBytes, header->Subchunk2Size, 1, filePointer);

	return dataBytes;
}

char* extractFile(FILE* filePointer, wav_hdr* header, huge_t *dataBitsSize)
{
	char *dataBits, *dataBytes;

	*header = readHeader(filePointer);
	dataBytes = readData(filePointer, header);

	dataBits = charStreamToBitStream(dataBytes, header->Subchunk2Size, dataBitsSize, 0);

	free(dataBytes);
	return dataBits;
}

char* extractEncodedFile(FILE* filePointer, wav_hdr* header, enc_hdr *encodeHeader, huge_t *dataBitsSize)
{
	char *dataBits, *dataBytes;

	*header = readHeader(filePointer);
	*encodeHeader = readEncodeHeader(filePointer);
	dataBytes = readData(filePointer, header);

	dataBits = charStreamToBitStream(dataBytes, header->Subchunk2Size, dataBitsSize, 0);

	free(dataBytes);
	return dataBits;
}

size_t writeData(char* data, huge_t size, FILE* filePointer)
{
	return fwrite(data, size, 1, filePointer);
}

size_t writeHeader(wav_hdr header, FILE* filePointer)
{
	//fseek(filePointer, 0, SEEK_SET);
	return fwrite(&header, sizeof(header), 1, filePointer);
}

size_t writeEncodeHeader(enc_hdr header, FILE* filePointer)
{
	//fseek(filePointer, sizeof(wav_hdr) , SEEK_SET);
	return fwrite(&header, sizeof(header), 1, filePointer);
}

size_t writeEncodedFile(char* data, huge_t size, wav_hdr header, enc_hdr encodeHeader, FILE* filePointer)
{
	size_t bytesWritten; // converte pra bytes

	bytesWritten = 0;

	bytesWritten += writeHeader(header, filePointer);
	bytesWritten += writeEncodeHeader(encodeHeader, filePointer);
	bytesWritten += writeData(data, size, filePointer);

	return bytesWritten;
}

size_t writeFile(char* data, huge_t size, wav_hdr header, FILE* filePointer)
{
	size_t bytesWritten;

	bytesWritten = 0;

	bytesWritten += writeHeader(header, filePointer);
	bytesWritten += writeData(data, size, filePointer);

	return bytesWritten;
}
