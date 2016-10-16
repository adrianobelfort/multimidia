#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "utils.h"
#include "List.h"
#include "differential.h"

int applyRunlength, applyDifference;

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

char *readData(FILE *input, wav_hdr *header) {

    // Aloca um vetor de chars, sendo que cada um representa um bit.
    // Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
    // um char tem um byte.
    char *dataBits = (char *) malloc(header->Subchunk2Size * BITS_PER_CHAR);

    // Vetor dos bytes lidos originalmente do arquivo
    char *originalData = (char *) malloc(header->Subchunk2Size);

    // Prepara o offset para leitura dos dados do arquivo,
    // pulando o header
    fseek(input, sizeof(wav_hdr), SEEK_SET);


    fread(originalData, header->Subchunk2Size, 1, input);

    // Mascara utilizada para isolar bits
    char mask = 0x1;
    unsigned long long int i;
    int currentBitPosition; // Atual posicao do bit dentro de um byte
    int dataBitsPosition = 0; // Posicao no vetor dataBits
    int shift; // Quantidade a ser deslocada na operacao bitshift

    // Para todos os bytes lidos do arquivo
    for(i = 0; i < header->Subchunk2Size; i++) {
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

// AQUI, FALTA IMPLEMENTAR NOSSO PROPRIO HEADER. A IDEIA EH USAR UM CHAR
// OU UM INT PARA DIZER QUAIS OS TIPOS DE COMPRESSÃO FEITOS. APÓS O HEADER
// ORIGINAL, ANTES DOS DADOS, COLOCAMOS ESSE NOSSO "HEADER"
int writeToOutput(FILE *output, wav_hdr *header, char *data, unsigned long long int size, unsigned int runlengthNumBits) {

    enc_hdr encodeHeader;
    encodeHeader.encodeType = 0;
    encodeHeader.runlengthNumBits = 0;

	if(applyDifference)
	{
		encodeHeader.encodeType |= 0x4;
		encodeHeader.channels = header->NumOfChan;
		encodeHeader.bitsPerSample = header->bitsPerSample;
	}

    if(applyRunlength) {
        encodeHeader.encodeType |= 0x1;
        encodeHeader.runlengthNumBits = runlengthNumBits;
        encodeHeader.totalLength = size/BITS_PER_CHAR;
    }

    // 1 e o numero de elementos a serem escritos. 1 header apenas
    if(fwrite(header, sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    // 1 e o numero de elementos a serem escritos. 1 encode header apenas
    if(fwrite(&encodeHeader, sizeof(enc_hdr), 1, output)!= 1) {
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

    free(byteData);

    return EXIT_SUCCESS;
}

int convertRunLengthToBits(char *runLengthBits, unsigned long int size, List *runs, unsigned int numBits) {
    int i = 0, currBit;
    unsigned long int currRun;
    Node *aux = runs->head;

    while(aux) {
        currRun = aux->data;
        currBit = 0;
        while(currRun > 0) {
            runLengthBits[i + numBits - 1 - currBit] = currRun % 2;
            currRun /= 2;
            currBit++;
        }
        while(currBit < numBits) {
            runLengthBits[i + numBits - 1 - currBit] = 0;
            currBit++;
        }
        i += numBits;
        aux = aux->next;
    }

    if(i!=size) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char *runlength(char *data, unsigned long long int size, unsigned long long  int *runlengthSize, unsigned int *numBits) {
    if(size == 0)
        return NULL;
    List *runs = create();
    unsigned long int maxRun = 1, currentRun = 1;
    char lastBit = data[0];

    if(lastBit == 1) {
        add(0, runs);
    }

    //printf("\n\n\nOriginal Size: %llu\n\n\n", size);
    unsigned long long int i;
    for(i = 1; i < size; i++) {
        if(lastBit != data[i]) {
            lastBit = data[i];
            if(currentRun > maxRun) {
                maxRun = currentRun;
            }
            add(currentRun, runs);
            currentRun = 1;
        } else {
            currentRun++;
        }
    }

    if(currentRun != 0) {
        add(currentRun, runs);
    }

    //printf("\n\n\nOriginal i: %llu\n\n\n", i);

    *numBits = findNumBits(maxRun);
    *runlengthSize = *numBits * runs->size;

    printf("\n\nnumbits: %d - size: %llu\nlist: %lu - max: %lu\n\n", *numBits, *runlengthSize, runs->size, maxRun);
    char *runlengthBits = (char *) malloc(*runlengthSize);
    if(convertRunLengthToBits(runlengthBits, *runlengthSize, runs, *numBits)) {
        free(runlengthBits);
        return NULL;
    }
//    traverse(runs);
    clearList(runs);
    return runlengthBits;
}


int main (int argc, char* argv[])
{
	/***************** LEITURA DOS ARGUMENTOS DA LINHA DE COMANDO *****************/

	struct arguments arguments;
	char verbose;

	parseArguments(argc, argv, &arguments);
	verbose = arguments.verbose;

	if (verbose)
	{
		printf("Uses Huffman? %s\n", arguments.huffman == 1 ? "yes" : "no");
		printf("Uses run-length? %s\n", arguments.runlength == 1 ? "yes" : "no");
		printf("Uses differential? %s\n", arguments.difference == 1 ? "yes" : "no");
		printf("Input file is %s\n", arguments.input);
		printf("Output file is %s\n", arguments.output);
		printf("\n");
	}

	if (!(arguments.huffman || arguments.runlength || arguments.difference) != 0)
	{
		printf("Nothing to do.\n");
		return 1;
	}

	/******************************************************/
	/******************** Lógica principal ****************/
	/******************************************************/

	applyRunlength = (int) arguments.runlength;
	applyDifference = (int) arguments.difference;

	FILE *input = NULL, *output = NULL; // descritores dos arquivos de entrada e saída a ser processado
    int fileSize; // tamanho total do arquivo

	if(openFiles(&input, &output, arguments.input, arguments.output)) {
        return EXIT_FAILURE;
    }

	wav_hdr *header = readHeader(input, &fileSize);

    char *dataBits = readData(input, header);
    unsigned long long int dataBitsSize = header->Subchunk2Size * BITS_PER_CHAR;
    unsigned long long int runlengthSize;
    unsigned int runlengthNumBits = 0;

	if (arguments.verbose)
	{
		printf("Data bits size: %llu\n", dataBitsSize);
	}

    // FAZER AQUI AS CHAMADAS PARA AS CODIFICACOES
    // PROTOTIPO:
    // char *codificacaoMetodoX(char *input, char* size, int bitsperSample)
    // A CADA CHAMADA PARA UM METODO DIFERENTE, ATUALIZAMOS INPUT,
    // SIZE E BITSPERSAMPLE. NUM PRIMEIRO MOMENTO,
    // INPUT É dataBits, E SIZE E BITSPERSEAMPLE DIZEM RESPEITO AOS
    // DADOS DO ARQUIVO DE ENTRADA. DEPOIS, APOS A PRIMEIRA
    // CODIFICACAO, ESSES DADOS SAO ATUALIZADOS.

	if (arguments.difference)
	{
		if (verbose) printf("Running differential encoding...\n");

		char *differentialBits = differentialEncodingWithChannels(dataBits, (int) dataBitsSize,
			header->NumOfChan, header->bitsPerSample);

		free(dataBits);

		dataBits = differentialBits;
	}

    if(arguments.runlength)
	{
		if (verbose) printf("Running run-length encoding...\n");

		printf("ORIGINAL DATA BITS SIZE: %llu\n", dataBitsSize);

        char *runlengthBits = runlength(dataBits, dataBitsSize, &runlengthSize, &runlengthNumBits);
        if(!runlengthBits)
            return EXIT_FAILURE;
        free(dataBits);

		printf("AFTER RUN LENGTH: %llu\n", runlengthSize);

        dataBits = runlengthBits;
        dataBitsSize = runlengthSize;
    }


    if(writeToOutput(output, header, dataBits, dataBitsSize, runlengthNumBits)) {
        return EXIT_FAILURE;
    }

    free(dataBits);
    free(header);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;

	return 0;
}
