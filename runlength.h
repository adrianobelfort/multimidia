#ifndef runlength_h
#define runlength_h

#include <stdio.h>
#include <stdlib.h>
#include "List.h"

unsigned int findNumBits(unsigned long long int number);
int convertRunLengthToBitsEncode(char *runLengthBits, unsigned long int size, List *runs, unsigned int numBits);
char *runlengthEncode(char *data, unsigned long long int size, unsigned int *runlengthPadding, unsigned long long  int *runlengthSize, unsigned int *numBits);
char *runlengthDecode(char *data, unsigned long long int size, unsigned int numBits, unsigned long long int *totalBitsLength, unsigned int runlengthPadding);
char *convertRunLengthToBitsDecode(unsigned long long int totalBitsLength, unsigned long long int *samples, unsigned long int numberSamples);

#endif /* runlength_h */
