#ifndef __DIFFERENTIAL_H__
#define __DIFFERENTIAL_H__

char* differentialEncoding(char* stream, int n, int bitsPerSample);

char* differentialDecoding(char* stream, int n, int bitsPerSample);

char* differentialEncodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

char* differentialDecodingWithChannels(char* stream, int n, int channels, int bitsPerSample);

#endif
