#ifndef __DIFFERENTIAL_BASE_H__
#define __DIFFERENTIAL_BASE_H__

#include <stdint.h>

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

/********************* MANIPULAÇÃO DE BLOCOS DE AMOSTRAS *********************/

void initializeChunk(Chunk* chunk, int capacity);

void populateChunk(Chunk* chunk, Sample* samples);

void copyChunk(Chunk *destination, Chunk* source);

void destroyChunk(Chunk* chunk);

Chunk* newChunk(int capacity);

void killChunk(Chunk* chunk);

/********************** MISCELÂNEA **********************/

void assign(Sample* destination, Sample* source);

char* copyBits(char* bits, int n);

/********************** CONVERSÕES E IMPRESSÃO COM BITS **********************/

int32_t niceNumber(char* sample, int n);

void nicePrint(char* sample, int n);

void printBits(char* stream, int n, int bitsPerSample);

Sample bitsToSample(char* bits, int bitsPerSample);

Chunk bitsToChunk(char* bits, int bitsPerSample, int n);

char* sampleToBits(Sample sample);

char* chunkToBits(Chunk chunk);

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

/************************** UTILITÁRIAS **************************/

int minimumSizeInBits(char* number, int n);

int minimumSampleSizeInBits(Sample sample);

int minimumRepresentationSizeInBits(Chunk chunk);

/* COISAS NOVAS */

Chunk* isolateChannels(Sample* samples, int numberOfSamples, int channels);

Sample* combineChannels(Chunk* channelChunks, int n);

Sample* bitsToSampleArray(char* stream, int n, int bitsPerSample);

void putBits(char* destination, char* source, int n);

char* sampleArrayToBits(Sample* samples, int n);

Chunk* computeDifferenceWithChannels(Chunk* channelChunks, int channels);

Chunk* computeSumWithChannels(Chunk *channelChunks, int channels);

char* differentialEncodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

char* differentialDecodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

void destroyChannels(Chunk* chunks, int channels);

#endif
