#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "List.h"
#include "parser.h"
#include "runlength.h"
#include "utils.h"
#include "differential.h"

/* Le o header do arquivo .wav e encontra o tamanho do arquivo */
wav_hdr *readHeader(FILE *input, unsigned int *fileSize) {
    wav_hdr* header = (wav_hdr *) malloc(sizeof(wav_hdr));

    fseek(input, 0, SEEK_END);

    *fileSize = (int) ftell(input);

    fseek(input, 0, SEEK_SET);

    fread(header, sizeof(wav_hdr), 1, input);

    if(DEBUG_FLAG) {
        printf("File size: %u bytes\n", *fileSize);
        printf("ChunkID: %s\n", header->RIFF);
        printf("ChunkSize: %u\n", header->ChunkSize);
        printf("AudioFormat: %d\n", header->AudioFormat);
        printf("NumOfChan: %d\n", header->NumOfChan);
        printf("SamplesPerSec: %u\n", header->SamplesPerSec);
        printf("bytesPerSec: %u\n", header->bytesPerSec);
        printf("bitsPerSample: %hu\n", header->bitsPerSample);
        printf("SubChunk2Size (data): %u\n", header->Subchunk2Size);
    }

    return header;
}

/* Constrói a parte estática do cabeçalho de diferenças */
StaticDifferentialHeader buildStaticDifferentialHeader(large_t numberOfSamplesPerChannel,
	unsigned short channels, short originalBitsPerSample)
{
	StaticDifferentialHeader header;
	memset(&header, 0, sizeof(header));

	header.numberOfSamplesPerChannel = numberOfSamplesPerChannel;
	header.channels = channels;
	header.originalBitsPerSample = originalBitsPerSample;

	return header;
}

/* Constrói o cabeçalho completo de diferenças */
DifferentialHeader buildDifferentialHeader(StaticDifferentialHeader preHeader, short *encodedBitsPerSample)
{
	DifferentialHeader header;

	header.sheader = preHeader;
	header.encodedBitsPerSample = encodedBitsPerSample;

	return header;
}

/* Destrói a parte dinâmica do cabeçalho de diferenças */
void destroyDifferentialHeader(DifferentialHeader header)
{
	free(header.encodedBitsPerSample);
}

/* Le os dados referentes ao audio do arquivo de entrada */
char *readData(FILE *input, wav_hdr *header) {

    /* Aloca um vetor de chars, sendo que cada um representa um bit.
     * Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
     * um char tem um byte.
     */
    char *dataBits = (char *) malloc(header->Subchunk2Size * BITS_PER_CHAR);

    /* Vetor dos bytes lidos originalmente do arquivo */
    char *originalData = (char *) malloc(header->Subchunk2Size);

    /* Prepara o offset para leitura dos dados do arquivo,
     * pulando o header
     */
    fseek(input, sizeof(wav_hdr), SEEK_SET);

    /* Le os dados */
    fread(originalData, header->Subchunk2Size, 1, input);

    /* Mascara utilizada para isolar bits */
    char mask = 0x1;

    unsigned long long int i;

    /* Atual posicao do bit dentro de um byte */
    int currentBitPosition;

    /* Posicao no vetor dataBits */
    int dataBitsPosition = 0;

    /* Quantidade a ser deslocada na operacao bitshift */
    int shift;

    /* Para todos os bytes lidos do arquivo */
    for(i = 0; i < header->Subchunk2Size; i++) {
        /* Seleciona o byte atual */
        char currValue = originalData[i];
        /* Inicializa a posicao de bit no byte atual */
        currentBitPosition = 0;
        /* Para todos os bits no byte */
        while(currentBitPosition < BITS_PER_CHAR) {
            /* Calcula o shift que deve ser feito */
            shift = BITS_PER_CHAR - 1 - currentBitPosition++;
            /* Isola o bit do byte e atribui a posicao do vetor dataBits */
            dataBits[dataBitsPosition++] = (currValue & (mask << shift)) >> shift;
        }
    }

    free(originalData);

    return dataBits;
}

/* Funcao que auxilia na inicializacao dos descritores de arquivos */
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

/* Escreve o header geral de compressao no arquivo de saida */
int writeEncodingHeader(struct arguments *arguments, unsigned long long int size, unsigned int fileSize, FILE *output) {

    enc_hdr encodeHeader;
	memset(&encodeHeader, 0, sizeof(encodeHeader));

    encodeHeader.encodeType = 0;

    if(arguments->runlength) {
        encodeHeader.encodeType |= RUNLENGTH_MASK;
    }

    if(arguments->huffman) {
        encodeHeader.encodeType |= HUFFMAN_MASK;
    }

    if(arguments->difference) {
        encodeHeader.encodeType |= DIFFERENCE_MASK;
    }

    encodeHeader.totalLength = size/BITS_PER_CHAR;

    encodeHeader.originalFileSize = fileSize;

    if(DEBUG_FLAG) {
        printf("Encode Mode: %u\n", encodeHeader.encodeType);
        printf("TotalLength: %llu\n", encodeHeader.totalLength);
        printf("Original File Size: %u\n", encodeHeader.originalFileSize);
    }

    /* 1 e o numero de elementos a serem escritos. 1 encode header apenas */
    if(fwrite(&encodeHeader, sizeof(enc_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Escreve o cabeçalho da codificação por diferenças no arquivo de saída */
int writeDifferentialHeader(DifferentialHeader header, FILE* file)
{
	if (fwrite(&header.sheader, sizeof(header.sheader), 1, file) != 1)
		return EXIT_FAILURE;
	if (fwrite(header.encodedBitsPerSample, sizeof(short), header.sheader.channels, file) != header.sheader.channels)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

/* Escreve o header de runlength no arquivo de saida */
int writeRunlengthHeader(unsigned int runlengthNumBits, unsigned int runlengthPadding, FILE *output) {
    run_hdr runlengthHeader;
    runlengthHeader.runlengthNumBits = runlengthNumBits;
    runlengthHeader.runlengthPadding = runlengthPadding;

    if(DEBUG_FLAG) {
        printf("Runlength Numbits: %u\n", runlengthHeader.runlengthNumBits);
        printf("Runlength Padding: %u\n", runlengthHeader.runlengthPadding);
    }

    /* 1 e o numero de elementos a serem escritos. 1 encode header apenas */
    if(fwrite(&runlengthHeader, sizeof(run_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Escreve o header Huffman e o vetor de frequencias no arquivo de saida */
int writeHuffmanHeaderAndData(unsigned char huffmanMaxValue, unsigned int *frequencyArray, FILE *output) {
    huf_hdr huffmanHeader;
    huffmanHeader.huffmanMaxValue = (unsigned int) huffmanMaxValue;;
    huffmanHeader.huffmanFrequenciesCount = 0;

    unsigned int i;
    for(i = 0; i <= huffmanMaxValue; i++) {
        if(frequencyArray[i] != 0)
            huffmanHeader.huffmanFrequenciesCount++;
    }

    if(DEBUG_FLAG) {
        printf("Huffman Frequencies Count: %u\n", huffmanHeader.huffmanFrequenciesCount);
        printf("Huffman Max Value: %u\n", huffmanHeader.huffmanMaxValue);
    }

    /* 1 e o numero de elementos a serem escritos. 1 encode header apenas */
    if(fwrite(&huffmanHeader, sizeof(huf_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    for(i = 0; i <= huffmanMaxValue; i++) {
        if(frequencyArray[i] != 0) {
            if(DEBUG_FLAG) {
                printf("%u - %u\n", i, frequencyArray[i]);
            }
            fwrite(&i, sizeof(i), 1, output);
            fwrite(&frequencyArray[i], sizeof(frequencyArray[i]), 1, output);
        }
    }

    return EXIT_SUCCESS;
}

/* Escreve os dados comprimidos no arquivo de saida */
int writeByteData(FILE *output, char *data, unsigned long long int size) {
    
    unsigned char *byteData = (unsigned char *) malloc(size/BITS_PER_CHAR * sizeof(unsigned char));
    
    unsigned long long int i, j = 0;
    unsigned char currByte = 0;
    int currBit = 0;
    int shift;
    
    for(i = 0; i < size/BITS_PER_CHAR; i++) {
        byteData[i] = 0;
    }
    
    for(i = 0; i < size; i++) {
        if(currBit == BITS_PER_CHAR) {
            currBit = 0;
            byteData[j++] = currByte;
            currByte = 0;
        }
        
        shift = BITS_PER_CHAR - 1 - currBit++;
        
        /* Isola o bit do byte e atribui a posicao do vetor dataBits */
        currByte |= data[i] << shift;
    }
    
    if(j != size/BITS_PER_CHAR) {
        byteData[j] = currByte;
    }
    
    if(fwrite(byteData, size/BITS_PER_CHAR, 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    
    
    free(byteData);
    
    return EXIT_SUCCESS;
}

/* Funcao de escrita de dados para o arquivo de saida*/
int writeToOutput(struct arguments *arguments, FILE *output, wav_hdr *header,
	char *data, unsigned long long int size,
	short* differenceEncodedBitsPerSample,
	unsigned int runlengthNumBits, unsigned int runlengthPadding,
	unsigned int *frequencyArray, unsigned char huffmanMaxValue,
	unsigned int fileSize)
{
	StaticDifferentialHeader staticDifferentialHeader;
	DifferentialHeader differentialHeader;
	large_t numberOfSamplesPerChannel;

    /* 1 e o numero de elementos a serem escritos. 1 WAV header apenas */
    if(fwrite(header, sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    if(writeEncodingHeader(arguments, size, fileSize, output)!= EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(arguments->difference)
	{
		numberOfSamplesPerChannel = header->Subchunk2Size * BITS_PER_BYTE / (header->bitsPerSample * header->NumOfChan);
        staticDifferentialHeader = buildStaticDifferentialHeader(numberOfSamplesPerChannel, header->NumOfChan, header->bitsPerSample);
		differentialHeader = buildDifferentialHeader(staticDifferentialHeader, differenceEncodedBitsPerSample);

		writeDifferentialHeader(differentialHeader, output);
		destroyDifferentialHeader(differentialHeader);
    }

    if(arguments->runlength) {
        if(writeRunlengthHeader(runlengthNumBits, runlengthPadding, output)!= EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    if(arguments->huffman) {
        if(writeHuffmanHeaderAndData(huffmanMaxValue, frequencyArray, output)!= EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    if(writeByteData(output, data, size)!= EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void showCompressionStatistics(FILE* output, unsigned int fileSizeInput) {

    unsigned int fileSizeOutput;

    fseek(output, 0, SEEK_END);

    fileSizeOutput = (int) ftell(output);

    float taxaCompressao = 1.0 - fileSizeOutput/(float)fileSizeInput;
    taxaCompressao = taxaCompressao * 100;

    printf("\nDados da compressão:\n");
    printf("\nTamanho do arquivo original: %u\n", fileSizeInput);
    printf("\nTamanho do arquivo comprimido: %u\n", fileSizeOutput);
    printf("\nTaxa de compressão (espaço reduzido): %.2f%%\n", taxaCompressao);

}

int main(int argc, char **argv) {

    /* Descritores dos arquivos de entrada e saída a serem processados */
    FILE *input, *output;
    /* Struct que representa os argumentos da linha de comando */
    struct arguments arguments;
    /* tamanho total do arquivo de entrada */
    unsigned int fileSizeInput;
    /* Header do WAV */
    wav_hdr *header;
    /* Vetor que representa o stream de bits que representa o audio original/codificado */
    char *dataBits;
    /* Tamanho do vetor dataBits */
    unsigned long long int dataBitsSize;
    /* Numero de bits por amostra dos dados originais/codificados */
    unsigned int numBits;
    /* Ponteiro temporario para vetor de dados apos codificacao Runlength */
    char *runlengthBits;
    /* Numero de bits por amostra da codificacao Runlength */
    unsigned int runlengthNumBits;
    /* Tamanho do vetor de dados apos codificacao runlength */
    unsigned long long int runlengthSize;
    /* Padding nos dados apos codificacao runlength para que runlengthSize seja multiplo de 8 */
    unsigned int runlengthPadding;
    /* Ponteiro temporario para vetor de dados apos codificacao Huffman */
    char *huffmanBits;
    /* Tamanho do vetor de dados apos codificacao Huffman */
    unsigned long long int huffmanSize;
    /* Maior valor codificado na codificacao Huffman */
    unsigned char huffmanMaxValue = 0;
    /* Vetor de frequencias da codificacao Huffman */
    unsigned int *frequencyArray = NULL;
	/* Vetor de número de bits de representação de amostras em canais */
	short *differenceBitsPerChannel = NULL;
	/* Vetor de dados após a codificação por diferenças */
	char* differenceBits;
	/* Tamanho do vetor de dados após a codificação por diferenças */
	huge_t differenceSize;
	/* Número de amostras na codificação por diferenças */
	large_t numberOfSamples;

    char verbose;

    input = NULL;
    output = NULL;
    runlengthPadding = 0;
    runlengthNumBits = 0;

    /* Caso haja algum erro nos argumentos passados ou a opção --help
     * seja selecionada , termina o programa
     */
    if(!parseArguments(argc, argv, &arguments, 'e')) {
        return 0;
    }

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

    /* Caso nenhuma das tecnicas de codificacao tenha sido passada,
     * nao ha nada a ser feito.
     */
    if (!(arguments.huffman || arguments.runlength || arguments.difference) != 0)
    {
        printf("Nothing to do.\n");
        return 0;
    }

    /* Abre os aqrquivos passados como argumento */
    if(openFiles(&input, &output, arguments.input, arguments.output)) {
        return EXIT_FAILURE;
    }

    /* Leitura do header WAV e calculo do tamanho do arquivo de entrada */
    header = readHeader(input, &fileSizeInput);

    /* Os bits dos dados do arquivo de entrada sao lidos */
    dataBits = readData(input, header);
    /* O tamanho do vetor de bits e calculado */
    dataBitsSize = header->Subchunk2Size * BITS_PER_CHAR;
    /* O numero de bits por amostra e atualizado */
    numBits = header->bitsPerSample;

    if(arguments.difference)
	{
        /********* TO DO ********/
		differenceBitsPerChannel = (short*) malloc(header->NumOfChan * sizeof(short));

		differenceBits = compressibleDifferentialEncodingWithChannels(dataBits, dataBitsSize, numBits, header->NumOfChan,
		&numberOfSamples, differenceBitsPerChannel, &differenceSize);

		free(dataBits);
		dataBits = differenceBits;
		dataBitsSize = differenceSize;
    }

    /* Caso deva ser feita a codificacao Runlength */
    if(arguments.runlength) {
        /* A codificacao e feita e as variaveis atualizadas */
        runlengthBits = runlengthEncode(dataBits, dataBitsSize, &runlengthPadding, &runlengthSize, &runlengthNumBits);
        if(!runlengthBits)
            return EXIT_FAILURE;
        free(dataBits);
        dataBits = runlengthBits;
        numBits = runlengthNumBits;
        dataBitsSize = runlengthSize;
        dataBitsSize = dataBitsSize - (dataBitsSize % BITS_PER_CHAR);
    }

    /* Caso deva ser feita a codificacao Runlength */
    if(arguments.huffman) {
        /* A codificacao e feita e as variaveis atualizadas */
        huffmanBits = huffmanEncode(dataBits, dataBitsSize, BITS_PER_CHAR, &huffmanSize, &frequencyArray, &huffmanMaxValue);
        if(!huffmanBits)
            return EXIT_FAILURE;
        free(dataBits);
        dataBits = huffmanBits;
        dataBitsSize = huffmanSize;
    }

    if(DEBUG_FLAG) {
        printf("\n\nSize - %llu\n\n", dataBitsSize);
    }

    /* Os dados resultantes sao gravados no arquivo de saida */
    if(writeToOutput(&arguments, output, header, dataBits, dataBitsSize, differenceBitsPerChannel, runlengthNumBits, runlengthPadding, frequencyArray, huffmanMaxValue, fileSizeInput)) {
        return EXIT_FAILURE;
    }

    showCompressionStatistics(output, fileSizeInput);

    free(dataBits);
    free(header);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;
}
