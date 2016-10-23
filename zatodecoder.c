#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
#include "List.h"
#include "parser.h"
#include "runlength.h"
#include "utils.h"

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

/* Le o header geral de compressao */
enc_hdr *readEncodeHeader(FILE *input, struct arguments *arguments) {
    enc_hdr* header = (enc_hdr *) malloc(sizeof(enc_hdr));
    
    fseek(input, sizeof(wav_hdr), SEEK_SET);
    
    fread(header, sizeof(enc_hdr), 1, input);
    
    if(header->encodeType & HUFFMAN_MASK) {
        arguments->huffman = 1;
    }
    
    if(header->encodeType & RUNLENGTH_MASK) {
        arguments->runlength = 1;
    }
    
    if(header->encodeType & DIFFERENCE_MASK) {
        arguments->difference = 1;
    }
    
    if(DEBUG_FLAG) {
        printf("Encode Mode: %u\n", header->encodeType);
        printf("TotalLength: %llu\n", header->totalLength);
        printf("Original File Size: %u\n", header->originalFileSize);
    }
    
    return header;
}

/* Le o header de compressao por diferenca */
dif_hdr *readDifferenceHeader(FILE *input) {
    dif_hdr* header = (dif_hdr *) malloc(sizeof(dif_hdr));
    
    long int seekSize = sizeof(wav_hdr)+sizeof(enc_hdr);
    
    fseek(input, seekSize, SEEK_SET);
    
    fread(header, sizeof(dif_hdr), 1, input);
    
    return header;
}

/* Le o header de compressao por runlength */
run_hdr *readRunlengthHeader(FILE *input, struct arguments *arguments) {
    run_hdr* header = (run_hdr *) malloc(sizeof(run_hdr));
    
    long int seekSize = sizeof(wav_hdr)+sizeof(enc_hdr);
    
    if(arguments->difference) {
        seekSize += sizeof(dif_hdr);
    }
    
    fseek(input, seekSize, SEEK_SET);
    
    fread(header, sizeof(run_hdr), 1, input);
    
    if(DEBUG_FLAG) {
        printf("Runlength Numbits: %u\n", header->runlengthNumBits);
        printf("Runlength Padding: %u\n", header->runlengthPadding);
    }
    
    return header;
}

/* Le o header de compressao por Huffman */
huf_hdr *readHuffmanHeader(FILE *input, struct arguments *arguments) {
    huf_hdr* header = (huf_hdr *) malloc(sizeof(huf_hdr));
    
    long int seekSize = sizeof(wav_hdr)+sizeof(enc_hdr);
    
    if(arguments->difference) {
        seekSize += sizeof(dif_hdr);
    }
    
    if(arguments->runlength) {
        seekSize += sizeof(run_hdr);
    }
    
    fseek(input, seekSize, SEEK_SET);
    
    fread(header, sizeof(huf_hdr), 1, input);
    
    if(DEBUG_FLAG) {
        printf("Huffman Frequencies Count: %u\n", header->huffmanFrequenciesCount);
        printf("Huffman Max Value: %u\n", header->huffmanMaxValue);
    }
    
    return header;
}

char *readData(FILE *input, wav_hdr *header, enc_hdr *encodeHeader, struct arguments *arguments, huf_hdr *huffmanHeader, unsigned int **frequencyArray) {
    
    long int seekSize = sizeof(wav_hdr)+sizeof(enc_hdr);
    
    if(arguments->difference) {
        seekSize += sizeof(dif_hdr);
    }
    
    if(arguments->runlength) {
        seekSize += sizeof(run_hdr);
    }
    
    if(arguments->huffman) {
        seekSize += sizeof(huf_hdr);
    }
    
    /* Aloca um vetor de chars, sendo que cada um representa um bit.
     * Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
     * um char tem um byte.
     */
    char *dataBits = (char *) malloc(encodeHeader->totalLength * BITS_PER_CHAR);
    
    /* Vetor dos bytes lidos originalmente do arquivo */
    char *originalData = (char *) malloc(encodeHeader->totalLength);
    
    /* Prepara o offset para leitura dos dados do arquivo, pulando os headers */
    fseek(input, seekSize, SEEK_SET);
    
    /* Le o vetor de frequencias para o caso de codificacao Huffman*/
    if(arguments->huffman) {
        unsigned int index, value;
        unsigned int * fArray = (unsigned int *) malloc((huffmanHeader->huffmanMaxValue+1) * sizeof(unsigned int));
        unsigned int i;
        
        for(i = 0; i < huffmanHeader->huffmanMaxValue+1; i++) {
            fArray[i] = 0;
        }
        
        for(i = 0; i < huffmanHeader->huffmanFrequenciesCount; i++) {
            fread(&index, sizeof(unsigned int), 1, input);
            fread(&value, sizeof(unsigned int), 1, input);
            fArray[index] = value;
            if(DEBUG_FLAG) {
                printf("%u - %u\n", index, value);
            }
        }
        *frequencyArray = fArray;
    }
    
    /* Le os dados comprimidos */
    fread(originalData, encodeHeader->totalLength, 1, input);
    
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
    for(i = 0; i < encodeHeader->totalLength; i++) {
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

/* Escreve os dados descomprimidos no arquivo de saida */
int writeToOutput(FILE *output, wav_hdr *header, char *data, unsigned long long int size, unsigned int originalFileSize) {
    
    /* 1 e o numero de elementos a serem escritos. 1 header apenas */
    if(fwrite(header, sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    
    if(DEBUG_FLAG) {
        printf("\n\nSize - %llu\n\n", size);
    }
    
    unsigned int byteDataSize = originalFileSize-sizeof(wav_hdr);
    char *byteData = (char *) malloc(byteDataSize * sizeof(char));
    unsigned long long int i, j = 0;
    char currByte = 0;
    int currBit = 0;
    int shift;
    
    for(i = 0; i < byteDataSize; i++)
    {
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
    
    if(fwrite(byteData, originalFileSize-sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    
    free(byteData);
    
    return EXIT_SUCCESS;
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
    /* Header geral do arquivo comprimido */
    enc_hdr *encode_header;
    /* Header da codificacao runlength */
    run_hdr *runlengthHeader = NULL;
    /* Header da codificacao Huffman */
    huf_hdr *huffmanHeader = NULL;
    /* Header da codificacao por diferenca */
    dif_hdr *differenceHeader = NULL;
    /* Vetor que representa o stream de bits que representa o audio original/codificado */
    char *dataBits;
    /* Tamanho do vetor dataBits */
    unsigned long long int dataBitsSize;
    /* Ponteiro temporario para vetor de dados apos codificacao Runlength */
    char *runlengthDecoded;
    /* Numero de bits por amostra da codificacao Runlength */
    unsigned int runlengthNumBits;
    /* Tamanho do vetor de dados apos decodificacao runlength */
    unsigned long long int runlengthBitsSize;
    /* Padding nos dados apos codificacao runlength para que runlengthSize seja multiplo de 8 */
    unsigned int runlengthPadding;
    /* Ponteiro temporario para vetor de dados apos codificacao Huffman */
    char *huffmanDecoded;
    /* Tamanho do vetor de dados apos codificacao Huffman */
    unsigned long long int huffmanSize;
    /* Vetor de frequencias da codificacao Huffman */
    unsigned int *frequencyArray;
    
    char verbose;
    
    input = NULL;
    output = NULL;
    runlengthPadding = 0;
    runlengthNumBits = 0;
    
    /* Caso haja algum erro nos argumentos passados ou a opção --help
     * seja selecionada , termina o programa
     */
    if(!parseArguments(argc, argv, &arguments, 'd')) {
        return 0;
    }
    
    verbose = arguments.verbose;
    
    /* Abre os aqrquivos passados como argumento */
    if(openFiles(&input, &output, arguments.input, arguments.output)) {
        return EXIT_FAILURE;
    }
    
    /* Leitura do header WAV e calculo do tamanho do arquivo de entrada */
    header = readHeader(input, &fileSizeInput);
    
    
    encode_header = readEncodeHeader(input, &arguments);
    
    /* Caso nenhuma das tecnicas de codificacao tenha sido passada,
     * nao ha nada a ser feito.
     */
    if (!(arguments.huffman || arguments.runlength || arguments.difference) != 0)
    {
        printf("Nothing to do.\n");
        return 0;
    }
    
    if (verbose)
    {
        printf("Is input file Huffman-compressed? %s\n", arguments.huffman == 1 ? "yes" : "no");
        printf("Is input file run-length-compressed? %s\n", arguments.runlength == 1 ? "yes" : "no");
        printf("Is input file differential-compressed? %s\n", arguments.difference == 1 ? "yes" : "no");
        printf("Input file is %s\n", arguments.input);
        printf("Output file is %s\n", arguments.output);
        printf("\n");
    }
    
    /* Le os header de diferenca, runlength e huffman caso tais codificacoes
     * tenham sido feitas
     */
    if(arguments.difference) {
        differenceHeader = readDifferenceHeader(input);
    }
    
    if(arguments.runlength) {
        runlengthHeader = readRunlengthHeader(input, &arguments);
    }
    
    if(arguments.huffman) {
        huffmanHeader = readHuffmanHeader(input, &arguments);
    }
    
    /* Inicializa dataBitsSize e dataBits com os valores lidos a partir
     * do arquivo de entrada
     */
    dataBitsSize = encode_header->totalLength * BITS_PER_CHAR;
    
    dataBits = readData(input, header, encode_header, &arguments, huffmanHeader, &frequencyArray);
    
    if(DEBUG_FLAG) {
        printf("\n\nDataBitsSizeOrig - %llu\n\n", dataBitsSize);
    }
    
    /* Se tiver sido feita a codificacao Huffman, realiza a descodificacao Huffman */
    if(arguments.huffman) {
        huffmanDecoded = huffmanDecode(dataBits, dataBitsSize, huffmanHeader->huffmanFrequenciesCount, huffmanHeader->huffmanMaxValue, frequencyArray, &huffmanSize);
        free(dataBits);
        dataBits = huffmanDecoded;
        dataBitsSize = huffmanSize;
    }
    
    if(DEBUG_FLAG) {
        printf("\n\nDataBitsSizeHuffi - %llu\n\n", dataBitsSize);
    }
    
    /* Se tiver sido feita a codificacao Runlength, realiza a descodificacao Runlength */
    if(arguments.runlength) {
        runlengthDecoded = runlengthDecode(dataBits, dataBitsSize, runlengthHeader->runlengthNumBits, &runlengthBitsSize, runlengthHeader->runlengthPadding);
        free(dataBits);
        dataBits = runlengthDecoded;
        dataBitsSize = runlengthBitsSize;
    }
    
    if(DEBUG_FLAG) {
        printf("\n\nDataBitsSizeRun - %llu\n\n", dataBitsSize);
    }
    
    /* Se tiver sido feita a codificacao por Diferenca, realiza a descodificacao por Diferenca */
    if(arguments.difference) {
        /********* TO DO ********/
    }
    
    if(writeToOutput(output, header, dataBits, dataBitsSize, encode_header->originalFileSize)) {
        return EXIT_FAILURE;
    }
    
    if(arguments.difference) {
        free(differenceHeader);
    }
    
    if(arguments.runlength) {
        free(runlengthHeader);
    }
    
    if(arguments.huffman) {
        free(huffmanHeader);
        free(frequencyArray);
    }
    
    free(dataBits);
    free(header);
    free(encode_header);
    fclose(input);
    fclose(output);
    
    return EXIT_SUCCESS;
}
