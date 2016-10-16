#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "utils.h"
#include "List.h"
#include "differential.h"

#define RUNLENGTH_MASK 0x1
#define HUFFMAN_MASK 0x2
#define DIFFERENCE_MASK 0x4

// Le o header do arquivo .wav e encontra o tamanho do arquivo
wav_hdr *readHeader(FILE *input, int *fileSize) {
    wav_hdr* header = (wav_hdr *) malloc(sizeof(wav_hdr));

    fseek(input, 0, SEEK_END);

    *fileSize = (int) ftell(input);

    fseek(input, 0, SEEK_SET);

    fread(header, sizeof(wav_hdr), 1, input);

    printf("File size: %d bytes\n", *fileSize);
    printf("ChunkID: %s\n", header->RIFF);
    printf("ChunkSize: %u\n", header->ChunkSize);
    printf("AudioFormat: %d\n", header->AudioFormat);
    printf("NumOfChan: %d\n", header->NumOfChan);
    printf("SamplesPerSec: %u\n", header->SamplesPerSec);
    printf("bytesPerSec: %u\n", header->bytesPerSec);
    printf("bitsPerSample: %hu\n", header->bitsPerSample);
    printf("SubChunk2Size (data): %u\n", header->Subchunk2Size);

    return header;
}

enc_hdr *readEncodeHeader(FILE *input) {
    enc_hdr* header = (enc_hdr *) malloc(sizeof(enc_hdr));

    fseek(input, sizeof(wav_hdr), SEEK_SET);

    fread(header, sizeof(enc_hdr), 1, input);

    printf("Encode Mode: %d\n", header->encodeType);
    printf("RunlengthNumBits: %u\n", header->runlengthNumBits);
    printf("TotalLength: %llu\n", header->totalLength);

    return header;
}

char *readData(FILE *input, wav_hdr *header, enc_hdr *encodeHeader) {

    // Aloca um vetor de chars, sendo que cada um representa um bit.
    // Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
    // um char tem um byte.
    char *dataBits = (char *) malloc(encodeHeader->totalLength * BITS_PER_CHAR);

    // Vetor dos bytes lidos originalmente do arquivo
    char *originalData = (char *) malloc(encodeHeader->totalLength);

    // Prepara o offset para leitura dos dados do arquivo,
    // pulando o header
    fseek(input, sizeof(wav_hdr)+sizeof(enc_hdr), SEEK_SET);


    fread(originalData, encodeHeader->totalLength, 1, input);

    // Mascara utilizada para isolar bits
    char mask = 0x1;
    unsigned long long int i;
    int currentBitPosition; // Atual posicao do bit dentro de um byte
    int dataBitsPosition = 0; // Posicao no vetor dataBits
    int shift; // Quantidade a ser deslocada na operacao bitshift

    // Para todos os bytes lidos do arquivo
    for(i = 0; i < encodeHeader->totalLength; i++) {
        // Seleciona o byte atual
        char currValue = originalData[i];
        // Inicializa a posicao de bit no byte atual
        currentBitPosition = 0;
        // Para todos os bits no byte
        while(currentBitPosition < BITS_PER_CHAR) {
            // Calcula o shift que deve ser feito
            shift = BITS_PER_CHAR - 1 - currentBitPosition++;
            // Isola o bit do byte e atribui a posicao do vetor dataBits
            dataBits[dataBitsPosition++] = (currValue & (mask << shift)) >> shift;
        }
    }

    free(originalData);
    return dataBits;
}

int openFiles(FILE **input, FILE **output, char *inputName, char *outputName) {

    *input = fopen(inputName, "r");
    if(*input == NULL) {
        printf("Could not open wave file %s\n", inputName);
        return EXIT_FAILURE;
    }

    *output = fopen(outputName, "w");
    if(*output == NULL) {
        printf("Could not open wave file %s\n", outputName);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

unsigned int findNumBits(unsigned long long int number) {
    unsigned int bit = sizeof(unsigned long long) * BITS_PER_CHAR - 1;
    unsigned int mask = 0x1;

    while(!((number & (mask << bit)) >> bit)) {
        bit--;
    }
    return bit+1;
}

char *convertRunLengthToBits(unsigned long long int totalBitsLength, unsigned long long int *samples, unsigned long int numberSamples) {

    char *runLengthBits = (char *) malloc(totalBitsLength);

    unsigned long int i;
    unsigned long long int j, offset = 0;
    unsigned int current = 0;

//    for(i = 0; i < numberSamples; i++) {
//        printf("\nSample %lu - %llu\n", i, samples[i]);
//    }

    for(i = 0; i < numberSamples; i++) {
        for(j = 0; j < samples[i]; j++) {
            runLengthBits[offset + j] = current;
        }

        offset += j;

        if(current == 1)
            current = 0;
        else
            current = 1;
    }

    return runLengthBits;
}



char *runlength(char *data, unsigned long long int size, unsigned int numBits, unsigned long long int *totalBitsLength) {

    unsigned long long int i;
    unsigned long long int numberSamples = size/numBits;
    printf("\n\n\nnumSamples - %llu\nSize - %llu\nnumbits - %u\n\n\n", numberSamples, size, numBits);
    unsigned int currBit = 0, j = 0, shift;
    unsigned long long int currValue = 0;
    *totalBitsLength = 0;

    //printf("\n\n\nsize: %llu\n\n\nnumbits: %u\n\n\n", size, numBits);

    unsigned long long int *runlengthSamples = (unsigned long long int *) malloc(numberSamples*sizeof(unsigned long long int));

    //printf("\n\ncurrValue: %llu\n\n", currValue);
    for(i = 0; i < size; i++) {
        if(currBit == numBits)
        {
            //printf("\n\ncurrValue: %llu\n\n", currValue);
            *totalBitsLength += currValue;
            runlengthSamples[j++] = currValue;
            currBit = 0;
            currValue = 0;
        }

        shift = numBits - 1 - currBit++;
        //printf("\n\ncurrBit: %u\n\n", currBit);
        currValue |= data[i] << shift;
    }

    if(currValue != 0) {
        runlengthSamples[j++] = currValue;
        *totalBitsLength += currValue;
    }

    //printf("\n\n\n%llu\n\n\n", *totalBitsLength * sizeof(char));
    char *dataBits = convertRunLengthToBits(*totalBitsLength * sizeof(char), runlengthSamples, numberSamples);

    return dataBits;
}

// AQUI, FALTA IMPLEMENTAR NOSSO PROPRIO HEADER. A IDEIA EH USAR UM CHAR
// OU UM INT PARA DIZER QUAIS OS TIPOS DE COMPRESSÃO FEITOS. APÓS O HEADER
// ORIGINAL, ANTES DOS DADOS, COLOCAMOS ESSE NOSSO "HEADER"
int writeToOutput(FILE *output, wav_hdr *header, char *data, unsigned long long int size) {

    // 1 e o numero de elementos a serem escritos. 1 header apenas
    if(fwrite(header, sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    char *byteData = (char *) malloc(size/BITS_PER_CHAR);
    unsigned long long int i, j = 0;
    char currByte = 0;
    int currBit = 0;
    int shift;
    for(i = 0; i < size; i++) {
        if(currBit == BITS_PER_CHAR) {
            currBit = 0;
            byteData[j++] = currByte;
            currByte = 0;
        }

        shift = BITS_PER_CHAR - 1 - currBit++;
        // Isola o bit do byte e atribui a posicao do vetor dataBits
        currByte |= data[i] << shift;
    }

    if(currByte != 0)
        byteData[j++] = currByte;

    if(fwrite(byteData, size/BITS_PER_CHAR, 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    //printf("\n\n\nJ FINAL - %llu\n\n\nSIZE - %llu\n\n\n", j, size);
    free(byteData);

    return EXIT_SUCCESS;
}

int main (int argc, char* argv[])
{
	struct arguments arguments;
	char verbose;

	parseArguments(argc, argv, &arguments);
	verbose = arguments.verbose;

	if (verbose)
	{
		printf("Input file is %s\n", arguments.input);
		printf("Output file is %s\n", arguments.output);
	}

	/* Lógica principal */

	FILE *input = NULL, *output = NULL; // descritores dos arquivos de entrada e saída a ser processado
    int fileSize; // tamanho total do arquivo

    if(openFiles(&input, &output, arguments.input, arguments.output)) {
        return EXIT_FAILURE;
    }

    wav_hdr *header = readHeader(input, &fileSize);
    enc_hdr *encode_header = readEncodeHeader(input);

    char *dataBits = readData(input, header, encode_header);
    unsigned long long int dataBitsSize;
    unsigned long long int runlengthBitsSize;

    // FAZER AQUI AS CHAMADAS PARA AS CODIFICACOES
    // PROTOTIPO:
    // char *codificacaoMetodoX(char *input, char* size, int bitsperSample)
    // A CADA CHAMADA PARA UM METODO DIFERENTE, ATUALIZAMOS INPUT,
    // SIZE E BITSPERSAMPLE. NUM PRIMEIRO MOMENTO,
    // INPUT É dataBits, E SIZE E BITSPERSEAMPLE DIZEM RESPEITO AOS
    // DADOS DO ARQUIVO DE ENTRADA. DEPOIS, APOS A PRIMEIRA
    // CODIFICACAO, ESSES DADOS SAO ATUALIZADOS.

    if(encode_header->encodeType & RUNLENGTH_MASK)
	{
		if (verbose) printf("Running run-length decoding...\n");

        dataBitsSize = encode_header->totalLength * BITS_PER_CHAR;
		printf("ORIGINAL DATA BITS SIZE: %llu\n", dataBitsSize);
        char *runlengthDecoded = runlength(dataBits, dataBitsSize, encode_header->runlengthNumBits, &runlengthBitsSize);
        free(dataBits);
		printf("AFTER RUN LENGTH: %llu\n", runlengthBitsSize);
        dataBits = runlengthDecoded;
        dataBitsSize = runlengthBitsSize;
    }

	if (encode_header->encodeType & DIFFERENCE_MASK)
	{
		if (verbose) printf("Running differential decoding...\n");

		printf("Data bits size: %llu, samples %llu, %llu per channel\n", dataBitsSize,
			dataBitsSize / (unsigned long long int) encode_header->bitsPerSample,
			(dataBitsSize / (unsigned long long int) encode_header->bitsPerSample) / (unsigned long long int) encode_header->channels);

		char *differenceDecoded = differentialDecodingWithChannels(dataBits, dataBitsSize,
			encode_header->channels, encode_header->bitsPerSample);

		free(dataBits);
		dataBits = differenceDecoded;
		/* dataBitsSize não muda */
	}

    if(writeToOutput(output, header, dataBits, dataBitsSize)) {
        return EXIT_FAILURE;
    }

    free(dataBits);
    free(header);
    free(encode_header);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;
}
