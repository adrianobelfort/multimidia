#ifndef __DIFFERENTIAL_H__
#define __DIFFERENTIAL_H__

#include <stdint.h>
#include "utils.h"

char* differentialEncoding(char* stream, int n, int bitsPerSample);

char* differentialDecoding(char* stream, int n, int bitsPerSample);

char* differentialEncodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

char* differentialDecodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

char* compressibleDifferentialEncodingWithChannels(char* stream, huge_t size, int bitsPerSample,
	unsigned short channels, large_t *numberOfSamples, short* newBitsPerChannel, huge_t* compressedStreamSize);

char* decompressibleDifferentialDecodingWithChannels(char* stream, huge_t size, short* bitsPerSample,
	unsigned short channels, large_t numberOfSamples, int oldBitsPerSample, huge_t* decompressedStreamSize);

#endif
