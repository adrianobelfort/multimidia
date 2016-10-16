#ifndef __DIFFERENTIAL_H__
#define __DIFFERENTIAL_H__

char* differentialEncoding(char* stream, int n, int bitsPerSample);
char* differentialDecoding(char* stream, int n, int bitsPerSample);

#endif
