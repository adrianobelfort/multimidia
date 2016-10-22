#include <stdio.h>
#include <stdlib.h>
#include "differential-base.h"
#include "differential.h"
#include "reader.h"
#include <string.h>

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

void testF()
{
	char sample[] = {0,1,0,1,1,1,0,1,0,0,0,1,0,1,0,1}; //01011101 00010101
	char *result, *rebuiltSample;
	huge_t originalSize, convertedSize, rebuiltSize, i;
	short outstandingBits;

	originalSize = 16;

	result = bitStreamToCharStream(sample, originalSize, &convertedSize, &outstandingBits);

	for (i = 0; i < convertedSize; i++)
	{
		printf("Byte %llu: %d\n", i, result[i]);
	}

	rebuiltSample = charStreamToBitStream(result, convertedSize, &rebuiltSize, outstandingBits);

	for (i = 0; i < rebuiltSize; i++)
	{
		if (i % BITS_PER_BYTE == 0) printf(" ");
		printf("%d", rebuiltSample[i]);
	}
	printf("\n");

	free(result);
	free(rebuiltSample);
}

void testG() /* Encoding com o arquivo */
{
	char inputFilename[50], outputFilename[50];
	char *originalDataBits, *encodedBits, *encodedBytes;
	wav_hdr header;
	enc_hdr encodeHeader;
	FILE *wavFile, *encodedWavFile;
	huge_t originalDataBitsSize, encodedBytesSize;
	short outstandingBits;

	printf("Encoding\n");

	printf("Enter the name of the input file: ");
	scanf("%s", inputFilename);
	printf("Enter the name of the output file: ");
	scanf("%s", outputFilename);

	wavFile = openFile(inputFilename, "rb");
	if (wavFile == NULL) return;

	encodedWavFile = openFile(outputFilename, "wb");
	if (encodedWavFile == NULL)
	{
		closeFile(wavFile);
		return;
	}

	originalDataBits = extractFile(wavFile, &header, &originalDataBitsSize);

	/* Codificação - pode encapsular isso */
	encodeHeader.encodeType = 0x4;
	encodeHeader.channels = header.NumOfChan;
	encodeHeader.differenceLength = (huge_t) header.Subchunk2Size * BITS_PER_BYTE;

	//encodedBits = differentialEncoding(originalDataBits, originalDataBitsSize, header.bitsPerSample);
	encodedBits = differentialEncodingWithChannels(originalDataBits, originalDataBitsSize, header.NumOfChan, header.bitsPerSample);
	encodedBytes = bitStreamToCharStream(encodedBits, originalDataBitsSize, &encodedBytesSize, &outstandingBits);

	//writeData(encodedBytes, encodedBytesSize, encodedWavFile);
	writeEncodedFile(encodedBytes, encodedBytesSize, header, encodeHeader, encodedWavFile);

	closeFile(wavFile);
	closeFile(encodedWavFile);
	free(originalDataBits);
	free(encodedBits);
	free(encodedBytes);
}

void testH() /* Decoding com o arquivo */
{
	char inputFilename[50], outputFilename[50];
	char *originalDataBits, *decodedBits, *decodedBytes;
	wav_hdr header;
	enc_hdr encodeHeader;
	FILE *encodedFile, *decodedWavFile;
	huge_t originalDataBitsSize, decodedBytesSize;
	short outstandingBits;

	printf("Decoding\n");

	printf("Enter the name of the input file: ");
	scanf("%s", inputFilename);
	printf("Enter the name of the output file: ");
	scanf("%s", outputFilename);

	encodedFile = openFile(inputFilename, "r");
	if (encodedFile == NULL) return;

	decodedWavFile = openFile(outputFilename, "w");
	if (decodedWavFile == NULL)
	{
		closeFile(decodedWavFile);
		return;
	}

	originalDataBits = extractEncodedFile(encodedFile, &header, &encodeHeader, &originalDataBitsSize);

	printf("Encode header:\n");
	printf("Channels: %d\n", encodeHeader.channels);
	printf("Difference length: %llu\n", encodeHeader.differenceLength);

	//decodedBits = differentialDecoding(originalDataBits, originalDataBitsSize, header.bitsPerSample);
	decodedBits = differentialDecodingWithChannels(originalDataBits, originalDataBitsSize, header.NumOfChan, header.bitsPerSample);
	decodedBytes = bitStreamToCharStream(decodedBits, originalDataBitsSize, &decodedBytesSize, &outstandingBits);

	writeFile(decodedBytes, decodedBytesSize, header, decodedWavFile);

	closeFile(encodedFile);
	closeFile(decodedWavFile);
	free(originalDataBits);
	free(decodedBits);
	free(decodedBytes);
}

void testI()
{
	char inputFilename[50], outputFilename[50], rebuiltFilename[50];
	char *originalDataBits, *encodedBytes;
	wav_hdr header;
	enc_hdr encodeHeader;
	FILE *wavFile, *encodedWavFile;
	huge_t originalDataBitsSize, encodedBytesSize;
	short outstandingBits;

	printf("Encoding\n");

	printf("Enter the name of the input file: ");
	scanf("%s", inputFilename);
	printf("Enter the name of the output file: ");
	scanf("%s", outputFilename);
	printf("Enter the name of the rebuilt file: ");
	scanf("%s", rebuiltFilename);

	wavFile = openFile(inputFilename, "r");
	if (wavFile == NULL) return;

	encodedWavFile = openFile(outputFilename, "w");
	if (encodedWavFile == NULL)
	{
		closeFile(wavFile);
		return;
	}

	//memset(&header, 0, sizeof(wav_hdr));
	originalDataBits = extractFile(wavFile, &header, &originalDataBitsSize);

	printHeader(header);
	printf("File size: %ld\n", fileSize(wavFile));

	/* Codificação - pode encapsular isso */
	encodeHeader.encodeType = 0x4;
	encodeHeader.channels = header.NumOfChan;
	encodeHeader.differenceLength = (huge_t) header.Subchunk2Size * BITS_PER_BYTE;

	encodedBytes = bitStreamToCharStream(originalDataBits, originalDataBitsSize, &encodedBytesSize, &outstandingBits);

	/*printf("Original: ");
	for(i = 0; i < 100; i++)
	{
		printf("%x", encodedBytes[i]);
	}
	printf("\n");*/

	//writeData(encodedBytes, encodedBytesSize, encodedWavFile);
	writeEncodedFile(encodedBytes, encodedBytesSize, header, encodeHeader, encodedWavFile);

	closeFile(wavFile);
	closeFile(encodedWavFile);
	free(originalDataBits);
	free(encodedBytes);

	encodedWavFile = openFile(outputFilename, "rb");
	if (wavFile == NULL) return;

	wavFile = openFile(rebuiltFilename, "wb");
	if (encodedWavFile == NULL)
	{
		closeFile(encodedWavFile);
		return;
	}

	originalDataBits = extractEncodedFile(encodedWavFile, &header, &encodeHeader, &originalDataBitsSize);
	encodedBytes = bitStreamToCharStream(originalDataBits, originalDataBitsSize, &encodedBytesSize, &outstandingBits);

	/*printf("Read: ");
	for(i = 0; i < 100; i++)
	{
		printf("%x", encodedBytes[i]);
	}
	printf("\n");*/

	writeFile(encodedBytes, encodedBytesSize, header, wavFile);

	closeFile(wavFile);
	closeFile(encodedWavFile);
	free(originalDataBits);
	free(encodedBytes);
}

void testJ()
{
	char samples[] = {10, 14, 13, 17, 11};
	char *bits, *rebuilt;
	huge_t bitsSize, rebuiltSize, i;
	short outstandingBits;

	bits = charStreamToBitStream(samples, 5, &bitsSize, 0);

	for (i = 0; i < bitsSize; i++)
	{
		if (i%8 == 0) printf(" ");
		printf("%d", bits[i]);
	}
	printf("\n");

	rebuilt = bitStreamToCharStream(bits, bitsSize, &rebuiltSize, &outstandingBits);
	for (i = 0; i < rebuiltSize; i++)
	{
		if (i%8 == 0) printf(" ");
		printf("%d ", rebuilt[i]);
	}
	printf("\n");
}

void testK()
{
	char sample[] = {1,1,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,1,0,0,1,0};
	int size, i;

	size = 24;

	for (i = 0; i < size; i++)
	{
		if (i%8 == 0) printf(" ");
		printf("%d", sample[i]);
	}
	printf("\n");

	invertEndianess(sample, size, 8);

	for (i = 0; i < size; i++)
	{
		if (i%8 == 0) printf(" ");
		printf("%d", sample[i]);
	}
	printf("\n");

	invertEndianess(sample, size, 8);

	for (i = 0; i < size; i++)
	{
		if (i%8 == 0) printf(" ");
		printf("%d", sample[i]);
	}
	printf("\n");
}

void testL()
{
	char sample[] = {0,0,0,0,0,1,0,1, 0,0,0,0,1,0,1,0, 0,0,0,0,1,0,1,1, 0,0,0,0,0,1,1,1, 1,1,1,1,1,0,1,0};
	int size = 40;
	Chunk chunk;

	chunk = bitsToChunk(sample, size, 8);

	printf("Original chunk:\n");
	printChunk(chunk);

	reduceChunk(&chunk);

	printf("Reduced chunk:\n");
	printChunk(chunk);

	expandChunk(&chunk, 8);

	printf("Expanded chunk:\n");
	printChunk(chunk);

	destroyChunk(&chunk);
}

void testM()
{
	char sample[] = {0,0,0,0,0,1,0,1, 0,0,0,0,1,0,1,0, 0,0,0,0,1,0,1,1, 0,0,0,0,0,1,1,1, 0,0,0,0,1,0,0,0, 0,0,0,0,0,1,1,1};
	huge_t size = 48;
	char *compressedStream, *decompressedStream;
	huge_t compressedStreamSize, decompressedStreamSize;
	huge_t i;

	/* Para o header */
	large_t numberOfSamples;
	short *numberOfBitsPerChannel;

	numberOfBitsPerChannel = (short*) malloc(2 * sizeof(short));

	compressedStream = compressibleDifferentialEncodingWithChannels(sample, size, 8, 2, &numberOfSamples,
		numberOfBitsPerChannel, &compressedStreamSize);

	for (i = 0; i < 2; i++)
	{
		printf("Channel %llu: %hd bits\n", i+1, numberOfBitsPerChannel[i]);
	}

	printf("Compressed stream: %llu bits\n", compressedStreamSize);
	/*for (i = 0; i < compressedStreamSize; i++)
	{
		printf("%d", compressedStream[i]);
	}
	printf("\n");*/

	decompressedStream = decompressibleDifferentialDecodingWithChannels(compressedStream, compressedStreamSize,
		numberOfBitsPerChannel, 2, numberOfSamples, 8, &decompressedStreamSize);

	printf("Decompressed stream: %llu bits\n", decompressedStreamSize);
	for (i = 0; i < decompressedStreamSize; i++)
	{
		printf("%d", decompressedStream[i]);
	}
	printf("\n");
}

void testN()
{
	char *differentialStream, *rebuiltStream, *originalStream, *numberBits;
	huge_t differentialStreamSize, originalStreamSize, rebuiltStreamSize;
	int n, bits, i, j, k, sample, channels;

	/* Para o header */
	large_t numberOfSamples;
	short *numberOfBitsPerChannel;

	printf("Enter the number of channels you want to use: ");
	scanf("%d", &channels);
	printf("Enter the number of samples you want to use on each channel: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	bits += 1;
	printf("\n");

	numberOfBitsPerChannel = (short*) malloc(channels * sizeof(short));

	originalStreamSize = channels * n * bits;
	originalStream = (char*) malloc(originalStreamSize * sizeof(char));

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
	printBits(originalStream, originalStreamSize, bits);
	printf("\n");

	differentialStream = compressibleDifferentialEncodingWithChannels(originalStream, originalStreamSize, bits, channels, &numberOfSamples,
		numberOfBitsPerChannel, &differentialStreamSize);

	printf("Encoded data:\n");
	printBits(differentialStream, differentialStreamSize, bits);
	printf("\n");

	rebuiltStream = decompressibleDifferentialDecodingWithChannels(differentialStream, differentialStreamSize,
		numberOfBitsPerChannel, channels, numberOfSamples, bits, &rebuiltStreamSize);

	printf("Decoded data:\n");
	printBits(rebuiltStream, rebuiltStreamSize, bits);
	printf("\n");

	free(originalStream);
	free(differentialStream);
	free(rebuiltStream);
}

void testO()
{
	char *differentialStream, *rebuiltStream, *originalStream, *numberBits, *byteStream;
	huge_t differentialStreamSize, originalStreamSize, rebuiltStreamSize, byteStreamSize;
	int n, bits, i, j, k, sample, channels;
	short outstandingBits;
	StaticDifferentialHeader diffsheader;
	DifferentialHeader diffheader;
	long filesize;

	/* Para o header */
	large_t numberOfSamples;
	short *numberOfBitsPerChannel;

	char encodedfilename[50];
	FILE *encodedFile;

	printf("Enter the number of channels you want to use: ");
	scanf("%d", &channels);
	printf("Enter the number of samples you want to use on each channel: ");
	scanf("%d", &n);
	printf("Enter the maximum number of bits to represent each (unsigned) value: ");
	scanf("%d", &bits);
	printf("Enter the name of the file you wish to write the encoded data: ");
	scanf("%s", encodedfilename);

	bits += 1;
	printf("\n");

	numberOfBitsPerChannel = (short*) malloc(channels * sizeof(short));

	originalStreamSize = channels * n * bits;
	originalStream = (char*) malloc(originalStreamSize * sizeof(char));

	printf("ORIGINAL STREAM SIZE: %llu\n", originalStreamSize);

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
	printBits(originalStream, originalStreamSize, bits);
	printf("\n");

	differentialStream = compressibleDifferentialEncodingWithChannels(originalStream, originalStreamSize, bits, channels, &numberOfSamples,
		numberOfBitsPerChannel, &differentialStreamSize);

	printf("COMPRESSED DATA SIZE: %llu\n", differentialStreamSize);

	printf("Encoded data:\n");
	printBits(differentialStream, differentialStreamSize, bits);
	printf("\n");

	byteStream = bitStreamToCharStream(differentialStream, differentialStreamSize, &byteStreamSize, &outstandingBits);

	encodedFile = openFile(encodedfilename, "wb");

	diffsheader = buildStaticDifferentialHeader(n, channels, bits);
	diffheader = buildDifferentialHeader(diffsheader, numberOfBitsPerChannel, NULL);
	printDifferentialHeader(diffheader);
	writeDifferentialHeader(diffheader, encodedFile);
	writeData(byteStream, byteStreamSize, encodedFile);

	destroyDifferentialHeader(diffheader);
	free(byteStream);
	free(differentialStream);
	closeFile(encodedFile);

	printf("File written.\n");

	encodedFile = openFile(encodedfilename, "rb");
	filesize = fileSize(encodedFile);

	diffheader = readDifferentialHeader(encodedFile);
	printDifferentialHeader(diffheader);

	byteStreamSize = filesize - ftell(encodedFile);
	byteStream = (char*) calloc(byteStreamSize, sizeof(char));
	fread(byteStream, byteStreamSize, sizeof(char), encodedFile);

	differentialStream = charStreamToBitStream(byteStream, byteStreamSize, &differentialStreamSize, outstandingBits);

	closeFile(encodedFile);

	rebuiltStream = decompressibleDifferentialDecodingWithChannels(differentialStream, differentialStreamSize,
		numberOfBitsPerChannel, channels, numberOfSamples, bits, &rebuiltStreamSize);

	printf("REBUILT STREAM SIZE: %llu\n", rebuiltStreamSize);

	printf("Decoded data:\n");
	printBits(rebuiltStream, rebuiltStreamSize, bits);
	printf("\n");

	free(originalStream);
	free(differentialStream);
	free(rebuiltStream);
}

int main(int argc, char* argv[])
{
	testO();
	return 0;
}
