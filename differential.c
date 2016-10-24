#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "differential-base.h"
#include "differential.h"

/********************** MANIPULAÇÃO DE AMOSTRAS **********************/

/*
 *	Inicializa uma amostra de tamanho bitsPerSample
 */
void initializeSample(Sample* sample, int bitsPerSample)
{
	sample->size = bitsPerSample;
	sample->data = (char*) malloc(bitsPerSample * sizeof(char));
}

/*
 *	Popula uma amostra a partir de um vetor de caracteres
 */
void populateSample(Sample* sample, char* data)
{
	int i;

	for (i = 0; i < sample->size; i++)
	{
		sample->data[i] = data[i];
	}
}

/*
 *	Faz uma cópia completa de uma amostra a outra
 */
void copySample(Sample *destination, Sample *source)
{
	initializeSample(destination, source->size);
	populateSample(destination, source->data);
}

/*
 *	Destrói uma amostra
 */
void destroySample(Sample* sample)
{
	free(sample->data);
}

/********************* MANIPULAÇÃO DE BLOCOS DE AMOSTRAS *********************/

/*
 *	Inicializa um bloco de amostras de tamanho capacity
 */
void initializeChunk(Chunk* chunk, int capacity)
{
	chunk->size = capacity;
	chunk->samples = (Sample*) malloc(capacity * sizeof(Sample));
}

/*
 *	Popula o bloco de amostra com as amostras provenientes de um vetor de amostras
 */
void populateChunk(Chunk* chunk, Sample* samples)
{
	int i;

	for (i = 0; i < chunk->size; i++)
	{
		chunk->samples[i] = samples[i];
	}
}

/*
 *	Faz uma cópia completa de um bloco a outro
 */
void copyChunk(Chunk *destination, Chunk* source)
{
	int i;

	initializeChunk(destination, source->size);
	for (i = 0; i < source->size; i++)
	{
		copySample(&destination->samples[i], &source->samples[i]);
	}
}

/*
 *	Destrói um bloco (considerando que tem partes dinâmicas)
 */
void destroyChunk(Chunk* chunk)
{
	int i;

	for (i = 0; i < chunk->size; i++)
	{
		destroySample(&chunk->samples[i]);
	}

	free(chunk->samples);
}

/********************** MISCELÂNEA **********************/

/*
 *	Atribui à amostra destino o conteúdo da amostra de origem,
 *	liberando os recursos até então usados pela primeira
 */
void assign(Sample* destination, Sample* source)
{
	free(destination->data);
	destination->data = source->data;
}

/*
 *	Cria uma cópia de um vetor de bits
 */
char* copyBits(char* bits, int n)
{
	char* copy;
	int i;

	copy = (char*) malloc(n * sizeof(char));

	for (i = 0; i < n; i++)
	{
		copy[i] = bits[i];
	}

	return copy;
}

/********************** CONVERSÕES E IMPRESSÃO COM BITS **********************/

/*
 *	Converte uma amostra em número para representação na tela
 */
int32_t niceNumber(char* sample, int n)
{
	int32_t number = 0;
	int i, limit, maxOffset;

	maxOffset = sizeof(number) * BITS_PER_BYTE;

	limit = n-1;

	for (i = 0; i < n && i < maxOffset; i++)
	{
		number |= ((int32_t) sample[limit - i]) << i;
	}

	if (sample[0] == 1)
	{
		for (i = n; i < BITS_PER_BYTE * sizeof(number); i++)
		{
			number |= ((int32_t) 0x1) << i;
		}
	}

	return number;
}

/*
 *	Mostra uma amostra no formato de um número inteiro
 */
void nicePrint(char* sample, int n)
{
	int32_t number = niceNumber(sample, n);
	printf("%d", number);
}

/*
 *	Imprime todos os bits de um fluxo de bits de amostra
 *	considerando o número de bits com o qual a amostra é representada
 */
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

/*
 *	Converte um vetor de bits em uma amostra
 */
Sample bitsToSample(char* bits, int bitsPerSample)
{
	Sample sample;

	initializeSample(&sample, bitsPerSample);
	populateSample(&sample, bits);

	return sample;
}

/*
 *	Converte um vetor de bits em um bloco de amostras
 */
Chunk bitsToChunk(char* bits, int n, int bitsPerSample)
{
	int i, numberOfSamples;
	Chunk chunk;

	numberOfSamples = n / bitsPerSample;

	initializeChunk(&chunk, numberOfSamples);

	for (i = 0; i < numberOfSamples; i++)
	{
		/* Endereçamento a amostras */
		chunk.samples[i] = bitsToSample(bits + i*bitsPerSample, bitsPerSample);
	}

	return chunk;
}

/*
 *	Converte uma amostra em um vetor de bits
 */
char* sampleToBits(Sample sample, int *size)
{
	char* streamline;
	int i;

	*size = sample.size;
	streamline = (char*) malloc(sample.size * sizeof(char));
	for (i = 0; i < sample.size; i++)
	{
		streamline[i] = sample.data[i];
	}

	return streamline;
}

/*
 *	Converte um bloco em um vetor de bits
 */
char* chunkToBits(Chunk chunk, huge_t *bitStreamSize)
{
	huge_t slots = 0;
	int i, j, k = 0;
	char *stream;

	for (i = 0; i < chunk.size; i++)
	{
		slots += chunk.samples[i].size;
	}

	*bitStreamSize = slots;
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

/*
 *	Imprime uma amostra formatada
 */
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

/*
 *	Imprime um bloco formatado
 */
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

/*
 *	Converte um número inteiro para um vetor de bits
 */
char* numberToBits(int32_t number, int bits)
{
	int i, size, limit, maxOffset;
	char* numberBits;

	maxOffset = sizeof(number) * BITS_PER_BYTE;
	size = bits;
	limit = size - 1;

	numberBits = (char*) calloc(bits, sizeof(char));

	for (i = 0; i < bits && i < maxOffset; i++)
	{
		numberBits[limit-i] = (char)((number >> i) & 0x1);
	}

	while (i < bits)
	{
		numberBits[limit-i] = numberBits[bits - maxOffset];
		i++;
	}

	return numberBits;
}

/*
 *	Converte um número inteiro em uma amostra
 */
Sample numberToSample(int32_t number, int bitsPerSample)
{
	Sample sample;

	sample.size = bitsPerSample;
	sample.data = numberToBits(number, bitsPerSample);

	return sample;
}

/********************** ARITMÉTICA BOOLEANA **********************/

/*
 *	Realiza a soma binária de dois números
 */
int sumBits(char* a, char* b, char* result, int n)
{
	int i;
	char carry = 0, and, xor;

	/* Implementação de um somador completo */
	for (i = n-1; i >= 0; i--)
	{
		xor = a[i] ^ b[i];
		and = a[i] & b[i];

		result[i] = xor ^ carry;
		carry = (carry & xor) | and;
	}

	return carry; /* Indica overflow */
}

/*
 *	Nega uma cadeia de bits. Útil para a operação de subtração.
 */
char* negate(char* number, int n)
{
	int i;
	char* negatedNumber;

	negatedNumber = (char*) malloc(n * sizeof(char));

	for (i = 0; i < n; i++)
	{
		negatedNumber[i] = ~number[i] & 0x01;
	}

	return negatedNumber;
}

/*
 *	Soma 1 ao número binário especificado.
 */
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

	return carry; /* Indica overflow */
}

/*Sample convertSampleToSigned(Sample sample)
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
}*/

/*
 *	Realiza a operação de diferença a - b.
 *	Ela é feita se somando a à negação de b + 1
 */
Sample signedDifference(Sample a, Sample b)
{
	char *negatedB, *result;
	int size = b.size;
	Sample differenceSample;

	negatedB = negate(b.data, b.size);
	result = negatedB; 							/* Reusamos o vetor B invertido */
	sumOne(negatedB, size);

	sumBits(a.data, negatedB, result, size);

	initializeSample(&differenceSample, size);
	populateSample(&differenceSample, result);

	free(negatedB);
	return differenceSample;
}

/*Sample unsignedDifference(Sample a, Sample b)
{
	Sample signedA, signedB;

	signedA = convertSampleToSigned(a);
	signedB = convertSampleToSigned(b);

	return signedDifference(signedA, signedB);
}*/

/*
 *	Executa a operação a - b com duas amostras.
 */
Sample difference(Sample a, Sample b)
{
	Sample differenceSample;

	if (a.size != b.size) printf("[Difference] Warning: a and b sizes do not match.\n");

	differenceSample = signedDifference(a, b);

	return differenceSample;
}

/*
 *	Soma duas amostras
 */
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

/*
 *	Calcula as diferenças entre as amostras de um bloco,
 *	salvando-as em um novo bloco.
 */
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

/*
 *	Calcula a soma entre as amostras de um bloco,
 *	guardando o resultado em um novo bloco.
 */
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

/************************** UTILITÁRIAS **************************/

/*
 *	Determina o número mínimo de bits para representar um dado
 *	número binário.
 */
int minimumSizeInBits(char* number, int n)
{
	int i, size, bits;
	char* absoluteNumber, sign;

	size = n-1;
	bits = 0;

	sign = number[0];

	if (sign == POSITIVE)
	{
		absoluteNumber = copyBits(number+1, n-1);
	}
	else if (sign == NEGATIVE)
	{
		absoluteNumber = negate(number+1, n-1);
	}

	for (i = 0; i < size && absoluteNumber[i] != 1; i++);

	bits = n-i;

	free(absoluteNumber);

	return bits;
}

/*
 *	Determina o número mínimo de bits para representar uma
 *	amostra.
 */
int minimumSampleSizeInBits(Sample sample)
{
	return minimumSizeInBits(sample.data, sample.size);
}

/*
 *	Determina o número mínimo de bits para representar um
 *	bloco completo de amostras.
 */
int minimumRepresentationSizeInBits(Chunk chunk)
{
	int i, minimumRepresentation, representation;

	minimumRepresentation = minimumSampleSizeInBits(chunk.samples[0]);

	/* 	Encontra o valor máximo de bits necessários para
		representar qualquer amostra */
	for (i = 1; i < chunk.size; i++)
	{
		representation = minimumSampleSizeInBits(chunk.samples[i]);

		if (representation > minimumRepresentation)
		{
			minimumRepresentation = representation;
		}
	}

	return minimumRepresentation;
}

/********************** CODIFICAÇÃO **********************/

/*
 *	Realiza a codificação por diferenças com um fluxo de bits
 *	de entrada e a taxa de bits por amostra. É a forma mais
 *	simples da codificação por diferenças.
 *
 *	O fluxo de bits é convertido para um bloco, do qual é
 *	calculado um novo bloco com as diferenças. Esse bloco
 *	é serializado e retornado.
 */
char* differentialEncoding(char* stream, int n, int bitsPerSample)
{
	Chunk streamChunk, differenceChunk;
	char* encodedStream;
	huge_t bitStreamSize;

	streamChunk = bitsToChunk(stream, n, bitsPerSample);

	differenceChunk = computeDifference(streamChunk);

	encodedStream = chunkToBits(differenceChunk, &bitStreamSize);

	destroyChunk(&streamChunk);
	destroyChunk(&differenceChunk);

	return encodedStream;
}

/********************** DECODIFICAÇÃO **********************/

/*
 *	Realiza a decodificação por diferenças com um fluxo de entrada
 *	e uma taxa de bits por amostra. É a forma mais simples de
 *	decodificação.
 *
 *	O fluxo de bits é convertido para um bloco, do qual é calculada
 *	a soma dos elementos, numa operação contrária à codificação.
 *	Esse bloco é então serializado e retornado.
 */
char* differentialDecoding(char* stream, int n, int bitsPerSample)
{
	Chunk streamChunk, sumChunk;
	char* decodedStream;
	huge_t bitStreamSize;

	streamChunk = bitsToChunk(stream, n, bitsPerSample);

	sumChunk = computeSum(streamChunk);

	decodedStream = chunkToBits(sumChunk, &bitStreamSize);

	destroyChunk(&streamChunk);
	destroyChunk(&sumChunk);

	return decodedStream;
}

/*
 *	Dada uma sequência de amostras lidas do arquivo, isola os canais
 *	e os coloca em blocos independentes para processamento
 *	individual.
 */
Chunk* isolateChannels(Sample* samples, int numberOfSamples, int channels)
{
	Chunk* chunks;
	int i;

	chunks = (Chunk*) malloc(channels * sizeof(Chunk));
	for (i = 0; i < channels; i++)
	{
		initializeChunk(&chunks[i], numberOfSamples / channels);
	}

	for (i = 0; i < numberOfSamples; i++)
	{
		copySample(&chunks[i % channels].samples[i / channels], &samples[i]);
	}

	return chunks;
}

/*
 *	Recria o vetor de amostras para gravação em arquivo a partir de
 *	um conjunto de blocos.
 */
Sample* combineChannels(Chunk* channelChunks, int channels)
{
	Sample* samples;
	int i, numberOfSamples = 0;

	for (i = 0; i < channels; i++)
	{
		numberOfSamples += channelChunks[i].size;
	}

	samples = (Sample*) malloc(numberOfSamples * sizeof(Sample));

	for (i = 0; i < numberOfSamples; i++)
	{
		copySample(&samples[i], &channelChunks[i % channels].samples[i / channels]);
	}

	return samples;
}

/*
 *	Converte um fluxo de bits para um vetor de amostras considerando a
 *	taxa de bits por amostra.
 */
Sample* bitsToSampleArray(char* stream, int n, int bitsPerSample)
{
	int i, numberOfSamples;
	Sample* samples;

	numberOfSamples = n / bitsPerSample;

	samples = (Sample*) malloc(numberOfSamples * sizeof(Sample));

	for (i = 0; i < numberOfSamples; i++)
	{
		samples[i] = bitsToSample(stream + i*bitsPerSample, bitsPerSample);
	}

	return samples;
}

/*
 *	Copia n "bits" do vetor source para o vetor destination.
 */
void putBits(char* destination, char* source, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		destination[i] = source[i];
	}
}

/*
 *	Converte um vetor de amostras para um fluxo de bits.
 */
char* sampleArrayToBits(Sample* samples, int n)
{
	char *bits;
	int i, size = 0;

	for (i = 0; i < n; i++)
	{
		size += samples[i].size;
	}

	bits = (char*) malloc(size * sizeof(char));

	for (i = 0; i < n; i++)
	{
		putBits(bits + i*samples[i].size, samples[i].data, samples[i].size);
	}

	return bits;
}

/*
 *	Aplica a codificação por diferenças considerando mais que um canal.
 *	Dessa forma, a localidade dos dados pode ser mais explorada e as
 *	diferenças tendem a diminuir.
 */
Chunk* computeDifferenceWithChannels(Chunk* channelChunks, int channels)
{
	Chunk *differenceChunks;
	int i;

	differenceChunks = (Chunk*) malloc(channels * sizeof(Chunk));

	for (i = 0; i < channels; i++)
	{
		differenceChunks[i] = computeDifference(channelChunks[i]);
	}

	return differenceChunks;
}

/*
 *	Destrói um vetor de amostras.
 */
void destroySampleArray(Sample* samples, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		destroySample(&samples[i]);
	}

	free(samples);
}

/*
 *	Destrói um conjunto de blocos de amostras.
 */
void destroyChannels(Chunk* chunks, int channels)
{
	int i;

	for (i = 0; i < channels; i++)
	{
		destroyChunk(&chunks[i]);
	}

	free(chunks);
}

/*
 *	Execução completa da codificação por diferenças considerando vários canais
 *	e um fluxo de bits de entrada.
 */
char* differentialEncodingWithChannels(char* stream, int n, int channels, int bitsPerSample)
{
	Chunk *channelChunks, *encodedChannelChunks;
	Sample *samples, *encodedSamples;
	char* encodedStream;
	int numberOfSamples;

	numberOfSamples = n / bitsPerSample;

	samples = bitsToSampleArray(stream, n, bitsPerSample);
	channelChunks = isolateChannels(samples, numberOfSamples, channels);

	encodedChannelChunks = computeDifferenceWithChannels(channelChunks, channels);

	encodedSamples = combineChannels(encodedChannelChunks, channels);
	encodedStream = sampleArrayToBits(encodedSamples, numberOfSamples);

	destroyChannels(channelChunks, channels);
	destroyChannels(encodedChannelChunks, channels);
	destroySampleArray(encodedSamples, numberOfSamples);
	destroySampleArray(samples, numberOfSamples);

	return encodedStream;
}

/*
 *	Como parte da decodificação, calcula a soma dos blocos que
 *	representam os canais do áudio codificados por diferenças.
 */
Chunk* computeSumWithChannels(Chunk *channelChunks, int channels)
{
	Chunk *sumChunks;
	int i;

	sumChunks = (Chunk*) malloc(channels * sizeof(Chunk));

	for (i = 0; i < channels; i++)
	{
		sumChunks[i] = computeSum(channelChunks[i]);
	}

	return sumChunks;
}

/*
 *	Decodifica um fluxo de bits considerando vários canais e a codificação por
 *	diferenças.
 */
char* differentialDecodingWithChannels(char* stream, int n, int channels, int bitsPerSample)
{
	Chunk *channelChunks, *decodedChannelChunks;
	Sample *samples, *decodedSamples;
	char* decodedStream;
	int numberOfSamples;

	numberOfSamples = n / bitsPerSample;

	samples = bitsToSampleArray(stream, n, bitsPerSample);
	channelChunks = isolateChannels(samples, numberOfSamples, channels);

	decodedChannelChunks = computeSumWithChannels(channelChunks, channels);

	decodedSamples = combineChannels(decodedChannelChunks, channels);
	decodedStream = sampleArrayToBits(decodedSamples, numberOfSamples);

	destroyChannels(channelChunks, channels);
	destroyChannels(decodedChannelChunks, channels);
	destroySampleArray(decodedSamples, numberOfSamples);
	destroySampleArray(samples, numberOfSamples);

	return decodedStream;
}

/*
 *	Inverte o endianess in-place de um fluxo de bits.
 */
void invertEndianess(char* bitStream, huge_t size, int bitsPerSample)
{
	huge_t samples, i, j;
	char aux;

	samples = size / (huge_t) bitsPerSample;

	for (i = 0; i < samples; i++)
	{
		for (j = 0; j < bitsPerSample/2; j++)
		{
			aux = bitStream[i*bitsPerSample + j];
			bitStream[i*bitsPerSample + j] = bitStream[i*bitsPerSample + (bitsPerSample - 1 - j)];
			bitStream[i*bitsPerSample + (bitsPerSample - 1 - j)] = aux;
		}
	}
}

/*
 *	Trunca a amostra para a nova taxa bitsPerSample
 */
short reduceSample(Sample* sample, short bitsPerSample)
{
	short i;

	if(sample->size == bitsPerSample) return bitsPerSample;

	for (i = 0; i < bitsPerSample; i++)
	{
		sample->data[i] = sample->data[i + sample->size - bitsPerSample];
	}

	sample->size = bitsPerSample;
	return bitsPerSample;
}

/*
 *	Expande o valor da amostra para um número binário de
 *	bitsPerSample bits.
 */
short expandSample(Sample* sample, short bitsPerSample)
{
	char* sampleData;
	short i;

	if (sample->size == bitsPerSample) return bitsPerSample;

	sampleData = (char*) malloc(bitsPerSample * sizeof(char));

	for (i = 0; i < sample->size; i++)
	{
		sampleData[i + bitsPerSample - sample->size] = sample->data[i];
	}

	i = bitsPerSample - sample->size - 1;

	while(i >= 0)
	{
		sampleData[i] = sample->data[0];
		i--;
	}

	free(sample->data);

	sample->data = sampleData;
	sample->size = bitsPerSample;

	return bitsPerSample;
}

/*
 *	Acha o número mínimo para representar todas as amostras
 *	de um bloco e as trunca para esse valor de representação binária.
 */
short reduceChunk(Chunk* chunk)
{
	int i;
	short newSize;

	newSize = minimumRepresentationSizeInBits(*chunk);

	for (i = 0; i < chunk->size; i++)
	{
		reduceSample(&chunk->samples[i], newSize);
	}

	return newSize;
}

/*
 *	Expande todas as amostras do bloco para a nova taxa
 *	bitsPerSample de bits de representação.
 */
short expandChunk(Chunk* chunk, short bitsPerSample)
{
	int i;
	short oldSize;

	oldSize = chunk->samples[0].size;

	for (i = 0; i < chunk->size; i++)
	{
		expandSample(&chunk->samples[i], bitsPerSample);
	}

	return oldSize;
}

/*
 *	Isola o vetor de amostras em blocos correspondentes a cada canal
 *	e calcula o bloco de diferenças das amostras.
 */
Chunk* chunkedDifferentialEncodingWithChannels(Sample* samples, large_t numberOfSamples, unsigned short channels)
{
	Chunk *encodedChannelChunks, *channelChunks;

	channelChunks = isolateChannels(samples, numberOfSamples, channels);
	encodedChannelChunks = computeDifferenceWithChannels(channelChunks, channels);

	destroyChannels(channelChunks, channels);
	return encodedChannelChunks;
}

/*
 *	Converte um vetor de blocos em um fluxo de bits.
 */
char* chunkArrayToBits(Chunk *chunks, large_t numberOfChunks, huge_t *streamSize)
{
	char* stream;
	huge_t i, j, k, l, size;

	for (i = 0, size = 0; i < numberOfChunks; i++)
	{
		size += chunks[i].samples[0].size * chunks[i].size;
	}

	*streamSize = size;

	stream = (char*) malloc(size * sizeof(char));

	for (i = 0, l = 0; i < numberOfChunks; i++)
	{
		for (j = 0; j < chunks[i].size; j++)
		{
			for (k = 0; k < chunks[i].samples[j].size; k++, l++)
			{
				stream[l] = chunks[i].samples[j].data[k];
			}
		}
	}

	return stream;
}

/*
 *	Converte um fluxo de bits para um vetor de blocos considerando vários canais.
 */
Chunk* bitsToChunkArray(char* stream, huge_t size, unsigned short channels, large_t numberOfSamples, short *bitsPerSample)
{
	Chunk* chunks;
	unsigned short i;
	large_t j, numberOfSamplesPerChannel;
	short k;
	huge_t l;

	chunks = (Chunk*) malloc(channels * sizeof(Chunk));

	numberOfSamplesPerChannel = numberOfSamples / channels;

	for (i = 0, l = 0; i < channels; i++)
	{
		initializeChunk(&chunks[i], numberOfSamplesPerChannel);

		for (j = 0; j < numberOfSamplesPerChannel; j++)
		{
			initializeSample(&chunks[i].samples[j], bitsPerSample[i]);

			for (k = 0; k < bitsPerSample[i]; k++, l++)
			{
				chunks[i].samples[j].data[k] = stream[l];
			}
		}
	}

	return chunks;
}

/*
 *	Aplica a codificação por diferenças utilizando vários canais e truncando
 *	os blocos resultantes para a menor representação possível, permitindo uma
 *	compressão dos dados.
 */
char* compressibleDifferentialEncodingWithChannels(char* stream, huge_t size, int bitsPerSample,
	unsigned short channels, large_t *numberOfSamples, short* newBitsPerChannel, huge_t* compressedStreamSize)
{
	Sample *extractedSamples, *initialSamples;
	Chunk *differenceChunks, auxiliarChunk;
	char *encodedStream, *outputStream, *initialSampleStream;
	unsigned short i;
	int j;

	*numberOfSamples = size / bitsPerSample;

	initialSamples = (Sample*) malloc(channels * sizeof(Sample));

	/*	Converte o fluxo de bits em um vetor de amostras, que então é
		convertido em blocos e sobre ele são calculadas as diferenças
		entre as amostras */
	extractedSamples = bitsToSampleArray(stream, size, bitsPerSample);
	differenceChunks = chunkedDifferentialEncodingWithChannels(extractedSamples, *numberOfSamples, channels);

	for (i = 0; i < channels; i++)
	{
		/* 	Isolamos a primeira amostra, pois ela pode ter o maior número
		 	de bits de representação. Apenas o bloco sem a primeira amostra
			é truncado. */
		copySample(&initialSamples[i], &differenceChunks[i].samples[0]);
		initializeChunk(&auxiliarChunk, differenceChunks[i].size - 1);
		for (j = 0; j < auxiliarChunk.size; j++)
		{
			copySample(&auxiliarChunk.samples[j], &differenceChunks[i].samples[j+1]);
		}
		destroyChunk(&differenceChunks[i]);

		differenceChunks[i] = auxiliarChunk;

		newBitsPerChannel[i] = reduceChunk(&differenceChunks[i]);
	}

	initialSampleStream = sampleArrayToBits(initialSamples, channels);
	encodedStream = chunkArrayToBits(differenceChunks, channels, compressedStreamSize);

	outputStream = (char*) malloc((channels * bitsPerSample + *compressedStreamSize) * sizeof(char));
	putBits(outputStream, initialSampleStream, channels * bitsPerSample);
	putBits(outputStream + channels * bitsPerSample, encodedStream, *compressedStreamSize);

	*compressedStreamSize += channels * bitsPerSample;

	destroySampleArray(initialSamples, channels);
	destroySampleArray(extractedSamples, size / bitsPerSample);
	destroyChannels(differenceChunks, channels);
	free(initialSampleStream);
	free(encodedStream);
	return outputStream;
}

/*
 *	Determina o tamanho em bits de um bloco.
 */
huge_t sizeOfChunk(Chunk chunk)
{
	int i;
	huge_t size;

	for (i = 0, size = 0; i < chunk.size; i++)
	{
		size += chunk.samples[i].size;
	}

	return size;
}

/*
 *	Decodifica um vetor de blocos codificados por diferenças considerando vários canais.
 */
Sample* chunkedDifferentialDecodingWithChannels(Chunk* encodedChunks, unsigned short channels,
	large_t *numberOfSamples)
{
	Chunk *decodedChunks;
	Sample *combinedDecodedSamples;
	large_t i;

	decodedChunks = computeSumWithChannels(encodedChunks, channels);
	combinedDecodedSamples = combineChannels(decodedChunks, channels);

	for (i = 0, *numberOfSamples = 0; i < channels; i++)
	{
		*numberOfSamples += decodedChunks[i].size;
	}

	destroyChannels(decodedChunks, channels);
	return combinedDecodedSamples;
}

/*
 *	Decodifica um fluxo de bits codificados por diferenças e truncados para a menor representação possível.
 */
char* decompressibleDifferentialDecodingWithChannels(char* stream, huge_t size, short* bitsPerSample,
	unsigned short channels, large_t numberOfSamples, int oldBitsPerSample, huge_t* decompressedStreamSize)
{
	Sample *decodedSamples, *initialSamples;
	Chunk *differenceChunks;
	char *extractedStream, *initialSampleStream;
	unsigned short i;
	large_t j, numberOfSamplesPerChannel;

	/* 	As amostras iniciais são recuperadas e isoladas da parte do fluxo
		correspondente às amostras truncadas. */
	initialSampleStream = (char*) malloc(oldBitsPerSample * channels * sizeof(char));
	putBits(initialSampleStream, stream, oldBitsPerSample * channels);

	/* 	Deslocamos o ponteiro de forma que ele aponte para o início do
		fluo de amostras truncadas */
	stream = stream + oldBitsPerSample * channels;
	size = size - oldBitsPerSample * channels;

	numberOfSamplesPerChannel = numberOfSamples/channels;

	/* 	As primeiras amostras são convertidas em amostras e os blocos subsequentes são convertidos
		também na estrutura apropriada */
	initialSamples = bitsToSampleArray(initialSampleStream, oldBitsPerSample * channels, oldBitsPerSample);
	differenceChunks = bitsToChunkArray(stream, size, channels, numberOfSamples - channels, bitsPerSample);

	/* 	Expandimos cada bloco, atribuindo ao começo dele a primeira amostra que havia sido isolada */
	for (i = 0, *decompressedStreamSize = 0; i < channels; i++)
	{
		expandChunk(&differenceChunks[i], oldBitsPerSample);

		differenceChunks[i].samples = realloc(differenceChunks[i].samples, numberOfSamplesPerChannel * sizeof(Sample));
		for (j = numberOfSamplesPerChannel - 1; j > 0; j--)
		{
			copySample(&differenceChunks[i].samples[j], &differenceChunks[i].samples[j-1]);
		}
		copySample(&differenceChunks[i].samples[0], &initialSamples[i]);
		differenceChunks[i].size += 1;

		*decompressedStreamSize += sizeOfChunk(differenceChunks[i]);
	}

	/* 	Os blocos são decodificados e serializados */
	decodedSamples = chunkedDifferentialDecodingWithChannels(differenceChunks, channels, &numberOfSamples);
	extractedStream = sampleArrayToBits(decodedSamples, numberOfSamples);

	free(initialSampleStream);
	destroySampleArray(decodedSamples, numberOfSamples);
	destroyChannels(differenceChunks, channels);
	destroySampleArray(initialSamples, channels);
	return extractedStream;
}
