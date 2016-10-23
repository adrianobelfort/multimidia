#ifndef __DIFFERENTIAL_H__
#define __DIFFERENTIAL_H__

#include <stdint.h>

#define huge_t unsigned long long
#define large_t unsigned long

typedef struct
{
	large_t numberOfSamplesPerChannel;
	unsigned short channels;
	short originalBitsPerSample;
} StaticDifferentialHeader;

typedef struct
{
	StaticDifferentialHeader sheader;
	short *encodedBitsPerSample;
	//int32_t *initialSamples;
} DifferentialHeader;

char* differentialEncoding(char* stream, int n, int bitsPerSample);

char* differentialDecoding(char* stream, int n, int bitsPerSample);

char* differentialEncodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

char* differentialDecodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

#endif
