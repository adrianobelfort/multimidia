#ifndef __DIFFERENTIAL_BASE_H__
#define __DIFFERENTIAL_BASE_H__

#include <stdint.h>

#define huge_t unsigned long long

/********************** ORGANIZAÇÃO ESTRUTURAL DOS DADOS **********************/

typedef struct
{
	int size;
	char* data;
} Sample;

typedef struct
{
	int size;
	Sample* samples;
} Chunk;

#define POSITIVE 0
#define NEGATIVE 1
#define BITS_PER_BYTE 8

/********************** MANIPULAÇÃO DE AMOSTRAS **********************/

void initializeSample(Sample* sample, int bitsPerSample);

void populateSample(Sample* sample, char* data);

void copySample(Sample *destination, Sample *source);

void destroySample(Sample* sample);

Sample* newSample(int bitsPerSample);

void killSample(Sample* sample);

void destroySampleArray(Sample* samples, int n);

/********************* MANIPULAÇÃO DE BLOCOS DE AMOSTRAS *********************/

void initializeChunk(Chunk* chunk, int capacity);

void populateChunk(Chunk* chunk, Sample* samples);

void copyChunk(Chunk *destination, Chunk* source);

void destroyChunk(Chunk* chunk);

Chunk* newChunk(int capacity);

void killChunk(Chunk* chunk);

void destroyChannels(Chunk* chunks, int channels);

/**************************** MANIPULAÇÃO DE CANAIS ****************************/

Chunk* isolateChannels(Sample* samples, int numberOfSamples, int channels);

Sample* combineChannels(Chunk* channelChunks, int n);

/********************** MISCELÂNEA **********************/

void assign(Sample* destination, Sample* source);

char* copyBits(char* bits, int n);

void putBits(char* destination, char* source, int n);

void invertEndianess(char* bitStream, huge_t size, int bitsPerSample);

/********************** CONVERSÕES E IMPRESSÃO COM BITS **********************/

int32_t niceNumber(char* sample, int n);

void nicePrint(char* sample, int n);

void printBits(char* stream, int n, int bitsPerSample);

Sample bitsToSample(char* bits, int bitsPerSample);

Chunk bitsToChunk(char* bits, int n, int bitsPerSample);

Sample* bitsToSampleArray(char* stream, int n, int bitsPerSample);

char* sampleToBits(Sample sample, int *size);

char* sampleArrayToBits(Sample* samples, int n);

char* chunkToBits(Chunk chunk, huge_t *bitStreamSize);

void printSample(Sample sample);

void printChunk(Chunk chunk);

char* numberToBits(int32_t number, int bits);

Sample numberToSample(int32_t number, int bitsPerSample);

/********************** ARITMÉTICA BOOLEANA **********************/

int sumBits(char* a, char* b, char* result, int n);

char* negate(char* number, int n);

int sumOne(char* number, int n);

Sample convertSampleToSigned(Sample sample);

Chunk convertChunkToSigned(Chunk chunk);

Sample signedDifference(Sample a, Sample b);

Sample unsignedDifference(Sample a, Sample b);

/* O mesmo que a operação a - b */
Sample difference(Sample a, Sample b);

Sample sum(Sample a, Sample b);

Chunk computeDifference(Chunk chunk);

Chunk computeSum(Chunk chunk);

Chunk* computeDifferenceWithChannels(Chunk* channelChunks, int channels);

Chunk* computeSumWithChannels(Chunk *channelChunks, int channels);

/************************** UTILITÁRIAS **************************/

int minimumSizeInBits(char* number, int n);

int minimumSampleSizeInBits(Sample sample);

int minimumRepresentationSizeInBits(Chunk chunk);

// NOVAS

int reduceSample(Sample* sample, int bitsPerSample);

void expandSample(Sample* sample, int bitsPerSample);

int reduceChunk(Chunk* chunk);

int expandChunk(Chunk* chunk, int bitsPerSample);

#endif
