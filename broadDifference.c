#include <stdio.h>
#include <stdlib.h>

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

#define BITS_PER_BYTE 8

void initializeSample(Sample* sample, int bitsPerSample)
{
	sample->size = bitsPerSample;
	sample->data = (char*) malloc(bitsPerSample * sizeof(char));
}

void populateSample(Sample* sample, char* data)
{
	int i;

	for (i = 0; i < sample->size; i++)
	{
		sample->data[i] = data[i];
	}
}

int32_t niceNumber(char* sample, int n)
{
	int32_t number = 0;
	int i, limit;

	limit = n-1;

	for (i = 0; i < n; i++)
	{
		number |= ((int32_t) sample[limit - i]) << i;
	}

	if (sample[0] == 1)
	{
		for (i = n; i < BITS_PER_BYTE * sizeof(int32_t); i++)
		{
			number |= ((int32_t) 0x1) << i;
		}
	}

	return number;
}

void nicePrint(char* sample, int n)
{
	int32_t number = niceNumber(sample, n);
	printf("%d", number);
}

void printBits(char* stream, int n, int bitsPerSample)
{
	int i, j, samples;

	samples = n / bitsPerSample;

	for (i = 0; i < samples; i++)
	{
		for (j = 0; j < bitsPerSample; j++)
		{
			printf("%d", stream[i * bitsPerSample + j]);
		}
		printf(" ");
	}
}

void copySample(Sample *destination, Sample *source)
{
	initializeSample(destination, source->size);
	populateSample(destination, source->data);
}

void destroySample(Sample* sample)
{
	free(sample->data);
}

Sample* newSample(int bitsPerSample)
{
	Sample* sample = (Sample*) malloc(sizeof(Sample));
	initializeSample(sample, bitsPerSample);
	return sample;
}

void killSample(Sample* sample)
{
	destroySample(sample);
	free(sample);
}

void initializeChunk(Chunk* chunk, int capacity)
{
	chunk->size = capacity;
	chunk->samples = (Sample*) malloc(capacity * sizeof(Sample));
}

void copyChunk(Chunk *destination, Chunk* source)
{
	int i;

	initializeChunk(destination, source->size);
	for (i = 0; i < source->size; i++)
	{
		copySample(&destination->samples[i], &source->samples[i]);
	}
}

void destroyChunk(Chunk* chunk)
{
	int i;

	for (i = 0; i < chunk->size; i++)
	{
		destroySample(&chunk->samples[i]);
	}

	free(chunk->samples);
}

Chunk* newChunk(int capacity)
{
	Chunk* chunk = (Chunk*) malloc(sizeof(Chunk));

	initializeChunk(chunk, capacity);

	return chunk;
}

void killChunk(Chunk* chunk)
{
	destroyChunk(chunk);
	free(chunk);
}

Sample bitsToSample(int bitsPerSample, char* bits)
{
	Sample sample;

	initializeSample(&sample, bitsPerSample);
	populateSample(&sample, bits);

	return sample;
}

Chunk bitsToChunk(int bitsPerSample, char* bits, int n)
{
	int i, numberOfSamples;
	Chunk chunk;

	numberOfSamples = n / bitsPerSample;

	//printf("Samples: %d, bits per sample %d\n", numberOfSamples, bitsPerSample);

	initializeChunk(&chunk, numberOfSamples);

	for (i = 0; i < numberOfSamples; i++)
	{
		// Endereçamento a amostras
		chunk.samples[i] = bitsToSample(bitsPerSample, bits + i*bitsPerSample);
	}

	return chunk;
}

char* sampleToBits(Sample sample)
{
	char* streamline;
	int i;

	streamline = (char*) malloc(sample.size * sizeof(char));
	for (i = 0; i < sample.size; i++)
	{
		streamline[i] = sample.data[i];
	}

	return streamline;
}

char* chunkToBits(Chunk chunk)
{
	long int slots = 0;
	int i, j, k = 0;
	char *stream, *sampleStream;

	for (i = 0; i < chunk.size; i++)
	{
		slots += chunk.samples[i].size;
	}

	stream = (char*) malloc(slots * sizeof(char));

	for (i = 0; i < chunk.size; i++)
	{
		for(j = 0; j < chunk.samples[i].size; j++)
		{
			stream[k++] = chunk.samples[i].data[j];
		}
	}

	return stream;
}

void printSample(Sample sample)
{
	int i;
	int32_t number;

	printf("Sample: ");
	for (i = 0; i < sample.size; i++)
	{
		printf("%x", sample.data[i]);
	}

	number = niceNumber(sample.data, sample.size);
	printf(" (%d)\n", number);
}

void printChunk(Chunk chunk)
{
	int i;

	printf("Chunk: (size %d)\n", chunk.size);
	for (i = 0; i < chunk.size; i++)
	{
		printf("[%d] ", i);
		printSample(chunk.samples[i]);
	}
}

/*Chunk bitsToSamples(int bitsPerSample, char* bits, int n)
{
	int i, numberOfSamples;
	Chunk chunk;
	Sample sample;

	numberOfSamples = n / bitsPerSample;
	initializeChunk(&chunk, numberOfSamples);

	for (i = 0; i < numberOfSamples; i++)
	{
		initializeSample(&sample, bitsPerSample);

		for (j = 0; j < bitsPerSample; j++)
		{
			sample.data[j] = bits[i * numberOfSamples + j];
		}

		chunk.samples[i] = sample;
	}

	return chunk;
}*/

int sumBits(char* a, char* b, char* result, int n)
{
	int i;
	char carry = 0, and, xor;

	//printf("\nA is %d, B is %d\n", niceNumber(a,n), niceNumber(b,n));

	for (i = n-1; i >= 0; i--)
	{
		//printf("For i = %d: a[i] is %d and b[i] is %d\n", i, a[i], b[i]);
		xor = a[i] ^ b[i];
		and = a[i] & b[i];
		//printf("xor yielded %d and and yielded %d - carry is %d\n", xor, and, carry);

		result[i] = xor ^ carry;
		//printf("Carry is %d\n", carry);
		carry = (carry & xor) | and;
	}

	return carry; // indica overflow
}

char* invert(char* number, int n)
{
	int i;
	char* invertedNumber;

	invertedNumber = (char*) malloc(n * sizeof(char));

	for (i = 0; i < n; i++)
	{
		invertedNumber[i] = ~number[i] & 0x01;
	}

	return invertedNumber;
}

int sumOne(char* number, int n)
{
	int i;
	char carry = 0, and, xor;
	char one = 0x1;

	for (i = n-1; i >= 0; i--)
	{
		xor = number[i] ^ one;
		and = number[i] & one;

		number[i] = xor ^ carry;
		carry = and | (carry & xor);

		one = 0;
	}

	return carry; // indica overflow
}

Sample convertSampleToSigned(Sample sample)
{
	int i;
	Sample convertedSample;

	initializeSample(&convertedSample, sample.size + 1);

	convertedSample.data[0] = (char) 0;
	for (i = 0; i < sample.size; i++)
	{
		convertedSample.data[i+1] = sample.data[i];
	}

	return convertedSample;
}

Chunk convertChunkToSigned(Chunk chunk)
{
	Chunk convertedChunk;
	int i;

	initializeChunk(&convertedChunk, chunk.size);

	for (i = 0; i < chunk.size; i++)
	{
		convertedChunk.samples[i] = convertSampleToSigned(chunk.samples[i]);
	}

	return convertedChunk;
}

Sample signedDifference(Sample a, Sample b)
{
	char *invertedB, *result;
	int size = b.size;
	Sample differenceSample;

	invertedB = invert(b.data, b.size);
	result = invertedB; // sim, reusamos o vetor B invertido
	sumOne(invertedB, size);

	sumBits(a.data, invertedB, result, size);

	initializeSample(&differenceSample, size);
	populateSample(&differenceSample, result);

	free(invertedB);
	return differenceSample;
}

Sample unsignedDifference(Sample a, Sample b)
{
	Sample signedA, signedB;

	signedA = convertSampleToSigned(a);
	signedB = convertSampleToSigned(b);

	return signedDifference(signedA, signedB);
}

// O mesmo que a operação a - b
Sample difference(Sample a, Sample b)
{
	int size;
	Sample differenceSample;

	if (a.size != b.size) printf("[Difference] Warning: a and b sizes do not match.\n");

	size = a.size;

	if (size == 8)
	{
		differenceSample = unsignedDifference(a, b);
	}
	else
	{
		differenceSample = signedDifference(a, b);
	}

	return differenceSample;
}

Sample sum(Sample a, Sample b)
{
	int size;
	Sample sumSample;

	if (a.size != b.size) printf("[Sum] Warning: a and b sizes do not match.\n");
	size = a.size;

	initializeSample(&sumSample, size);

	sumBits(a.data, b.data, sumSample.data, size);

	return sumSample;
}

Chunk computeDifference(Chunk chunk)
{
	int i, size;
	Chunk differenceChunk;

	size = chunk.size;
	initializeChunk(&differenceChunk, size);

	copySample(&differenceChunk.samples[0], &chunk.samples[0]);
	for (i = 1; i < size; i++)
	{
		differenceChunk.samples[i] = difference(chunk.samples[i], chunk.samples[i-1]);
	}

	return differenceChunk;
}

void assign(Sample* destination, Sample* source)
{
	free(destination->data);
	destination->data = source->data;
}

Chunk computeSum(Chunk chunk)
{
	int i, size;
	Chunk sumChunk;
	Sample sumResult;

	size = chunk.size;

	copyChunk(&sumChunk, &chunk);
	for (i = 1; i < size; i++)
	{
		sumResult = sum(sumChunk.samples[i], sumChunk.samples[i-1]);
		assign(&sumChunk.samples[i], &sumResult);
	}

	return sumChunk;
}

char* differentialEncoding(char* stream, int n, int bitsPerSample)
{
	Chunk streamChunk, differenceChunk;
	char* encodedStream;

	streamChunk = bitsToChunk(bitsPerSample, stream, n);

	printf("Before encoding\n");
	printChunk(streamChunk);

	differenceChunk = computeDifference(streamChunk);

	printf("\nAfter encoding\n");
	printChunk(differenceChunk);

	encodedStream = chunkToBits(differenceChunk);

	destroyChunk(&streamChunk);
	destroyChunk(&differenceChunk);

	return encodedStream;
}

char* differentialDecoding(char* stream, int n, int bitsPerSample)
{
	Chunk streamChunk, sumChunk;
	char* decodedStream;

	streamChunk = bitsToChunk(bitsPerSample, stream, n);

	printf("Before decoding\n");
	printChunk(streamChunk);
	sumChunk = computeSum(streamChunk);

	printf("\nAfter decoding\n");
	printChunk(sumChunk);

	decodedStream = chunkToBits(sumChunk);

	destroyChunk(&streamChunk);
	destroyChunk(&sumChunk);

	return decodedStream;
}

int main(int argc, char* argv[])
{
	char sample[] = {0,1,0,1, 0,0,1,1, 0,1,0,0, 0,1,0,1, 0,1,1,1}; //5,3,4,5,7
	char *differentialSamples, *rebuiltSamples;
	int i;

	printf("Original bits: ");
	printBits(sample, 5*4, 4);
	printf("\n\n");

	differentialSamples = differentialEncoding(sample, 5*4, 4);

	printf("\nEncoded bits: ");
	printBits(differentialSamples, 5*4, 4);
	printf("\n\n");

	rebuiltSamples = differentialDecoding(differentialSamples, 5*4, 4);

	printf("\nRebuilt bits: ");
	printBits(rebuiltSamples, 5*4, 4);
	printf("\n\n");

	free(differentialSamples);
	free(rebuiltSamples);
	return 0;
}
