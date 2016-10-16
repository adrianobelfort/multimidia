#include <stdio.h>
#include <stdlib.h>
#include "differential-base.h"
#include "differential.h"

/********************** PROGRAMA PRINCIPAL **********************/

void testA()
{
	char sample[] = {0,1,0,1, 0,0,1,1, 0,1,0,0, 0,1,0,1, 0,1,1,1};  /* 5,3,4,5,7 */
	char *differentialSamples, *rebuiltSamples;

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
}

void testB()
{
	int n, bits, i, sample, representation;
	Chunk stream, encodedStream, decodedStream;
	Sample streamSample;

	printf("Enter the number of samples you want to use: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	printf("\n");

	initializeChunk(&stream, n);

	for (i = 0; i < n; i++)
	{
		printf("Sample %d: ", i);
		scanf("%d", &sample);

		streamSample = numberToSample(sample, bits+1);
		stream.samples[i] = streamSample;
	}
	printf("\n");

	printf("Original data:\n");
	printChunk(stream);
	printf("\n");

	printf("Encoding the samples...\n");

	encodedStream = computeDifference(stream);

	printf("Encoded data:\n");
	printChunk(encodedStream);
	printf("\n");

	representation = minimumRepresentationSizeInBits(encodedStream);
	printf("The encoded data can be represented using %d bits.\n\n", representation);

	printf("Decoding the samples...\n");

	decodedStream = computeSum(encodedStream);

	printf("Decoded data:\n");
	printChunk(decodedStream);

	destroyChunk(&stream);
	destroyChunk(&encodedStream);
	destroyChunk(&decodedStream);
}

void testC()
{
	int n, bits, i, sample, representation;
	Chunk stream, encodedStream, decodedStream, doubleEncoded, doubleDecoded;
	Sample streamSample;

	printf("Enter the number of samples you want to use: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	printf("\n");

	initializeChunk(&stream, n);

	for (i = 0; i < n; i++)
	{
		printf("Sample %d: ", i);
		scanf("%d", &sample);

		streamSample = numberToSample(sample, bits+1);
		stream.samples[i] = streamSample;
	}
	printf("\n");

	printf("Original data:\n");
	printChunk(stream);
	printf("\n");

	printf("Encoding the samples...\n");

	encodedStream = computeDifference(stream);

	printf("Encoded data:\n");
	printChunk(encodedStream);
	printf("\n");

	representation = minimumRepresentationSizeInBits(encodedStream);
	printf("The encoded data can be represented using %d bits.\n\n", representation);

	doubleEncoded = computeDifference(encodedStream);

	printf("Double encoded data:\n");
	printChunk(doubleEncoded);
	printf("\n");

	representation = minimumRepresentationSizeInBits(doubleEncoded);
	printf("The double encoded data can be represented using %d bits.\n\n", representation);

	printf("Decoding the samples...\n");

	decodedStream = computeSum(doubleEncoded);

	printf("1-fold decoded data:\n");
	printChunk(decodedStream);

	doubleDecoded = computeSum(encodedStream);

	printf("2-fold decoded data:\n");
	printChunk(doubleDecoded);

	destroyChunk(&stream);
	destroyChunk(&encodedStream);
	destroyChunk(&doubleEncoded);
	destroyChunk(&decodedStream);
	destroyChunk(&doubleDecoded);
}

void testD()
{
	int n, bits, i, j, sample, channels;
	Chunk *streams, *encodedStreams, *decodedStreams;
	Sample streamSample;

	printf("Enter the number of channels you want to use: ");
	scanf("%d", &channels);
	printf("Enter the number of samples you want to use on each channel: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	printf("\n");

	streams = (Chunk*) malloc(channels * sizeof(Chunk));
	for(i = 0; i < channels; i++) initializeChunk(&streams[i], n);

	for (j = 0; j < channels; j++)
	{
		printf("[Channel %d]\n", j+1);
		for (i = 0; i < n; i++)
		{
			printf("Sample %d: ", i+1);
			scanf("%d", &sample);

			streamSample = numberToSample(sample, bits+1);
			streams[j].samples[i] = streamSample;
		}
	}

	printf("Original data:\n\n");

	for (i = 0; i < channels; i++)
	{
		printf("Channel %d\n", i+1);
		printChunk(streams[i]);
	}
	printf("\n");

	encodedStreams = computeDifferenceWithChannels(streams, channels);

	printf("Encoded data:\n\n");

	for (i = 0; i < channels; i++)
	{
		printf("Channel %d\n", i+1);
		printChunk(encodedStreams[i]);
	}
	printf("\n");

	decodedStreams = computeSumWithChannels(encodedStreams, channels);

	printf("Decoded data:\n\n");

	for (i = 0; i < channels; i++)
	{
		printf("Channel %d\n", i+1);
		printChunk(decodedStreams[i]);
	}
	printf("\n");

	destroyChannels(encodedStreams, channels);
	destroyChannels(decodedStreams, channels);
	destroyChannels(streams, channels);
}

void testE()
{
	char *differentialStream, *rebuiltStream, *originalStream, *numberBits;
	int n, bits, i, j, k, sample, channels;
	int streamSize;

	printf("Enter the number of channels you want to use: ");
	scanf("%d", &channels);
	printf("Enter the number of samples you want to use on each channel: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	bits += 1;
	printf("\n");

	streamSize = channels * n * bits;
	originalStream = (char*) malloc(streamSize * sizeof(char));

	for (j = 0; j < channels; j++)
	{
		printf("[Channel %d]\n", j+1);
		for (i = 0; i < n; i++)
		{
			printf("Sample %d: ", i+1);
			scanf("%d", &sample);

			numberBits = numberToBits(sample, bits);

			for (k = 0; k < bits; k++)
			{
				originalStream[j*bits + i*bits*channels + k] = numberBits[k];
			}

			free(numberBits);
		}
	}

	printf("Original data:\n");
	printBits(originalStream, streamSize, bits);
	printf("\n");

	differentialStream = differentialEncodingWithChannels(originalStream, streamSize, channels, bits);

	printf("Encoded data:\n");
	printBits(differentialStream, streamSize, bits);
	printf("\n");

	rebuiltStream = differentialDecodingWithChannels(differentialStream, streamSize, channels, bits);

	printf("Decoded data:\n");
	printBits(rebuiltStream, streamSize, bits);
	printf("\n");

	free(originalStream);
	free(differentialStream);
	free(rebuiltStream);
}

int main(int argc, char* argv[])
{
	testE();

	return 0;
}
