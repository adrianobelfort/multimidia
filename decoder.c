#include <stdio.h>
#include <stdlib.h>
#include "List.h"

#define BITS_PER_CHAR 8
#define RUNLENGTH_MASK 0x1
#define HUFFMAN_MASK 0x2
#define DIFFERENCE_MASK 0x4

// *** References ***
// http://soundfile.sapp.org/doc/WaveFormat/
// http://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file

// Struc que representa o header dos arquivos .wav
typedef struct  WAV_HEADER{
    char                RIFF[4];        // RIFF Header      Magic header
    unsigned int        ChunkSize;      // RIFF Chunk Size
    char                WAVE[4];        // WAVE Header
    char                fmt[4];         // FMT header
    unsigned int        Subchunk1Size;  // Size of the fmt chunk
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio
    unsigned int        SamplesPerSec;  // Sampling Frequency in Hz
    unsigned int        bytesPerSec;    // bytes per second
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    unsigned short      bitsPerSample;  // Number of bits per sample
    char                Subchunk2ID[4]; // "data"  string
    unsigned int        Subchunk2Size;  // Sampled data length
    
} wav_hdr;

typedef struct encodeHeader {
    unsigned int encodeType;// 00000DHR (D - Diferença; H - Huffman; R - Runlength)
    unsigned int runlengthNumBits;
    unsigned int huffmanFrequenciesCount;
    unsigned int huffmanMaxValue;
    unsigned long long totalLength;
} enc_hdr;

typedef struct tn {
    unsigned long int value;
    unsigned int frequency;
    struct tn *left;
    struct tn* right;
} TreeNode;

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
    
    printf("Encode Mode: %u\n", header->encodeType);
    printf("RunlengthNumBits: %u\n", header->runlengthNumBits);
    printf("HuffmanFrequenciesCount: %u\n", header->huffmanFrequenciesCount);
    printf("HuffmanMaxValue: %u\n", header->huffmanMaxValue);
    printf("TotalLength: %llu\n", header->totalLength);
    
    return header;
}

char *readData(FILE *input, wav_hdr *header, enc_hdr *encodeHeader, unsigned int **frequencyArray) {
    
    // Aloca um vetor de chars, sendo que cada um representa um bit.
    // Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
    // um char tem um byte.
    char *dataBits = (char *) malloc(encodeHeader->totalLength * BITS_PER_CHAR);
    
    // Vetor dos bytes lidos originalmente do arquivo
    char *originalData = (char *) malloc(encodeHeader->totalLength);
    
    // Prepara o offset para leitura dos dados do arquivo,
    // pulando o header
    fseek(input, sizeof(wav_hdr)+sizeof(enc_hdr), SEEK_SET);
    
    if(encodeHeader->encodeType & HUFFMAN_MASK) {
        unsigned int index, value;
        unsigned int * fArray = (unsigned int *) malloc((encodeHeader->huffmanMaxValue+1) * sizeof(unsigned int));
        int i;
        for(i = 0; i < encodeHeader->huffmanFrequenciesCount; i++) {
            fread(&index, sizeof(unsigned int), 1, input);
            fread(&value, sizeof(unsigned int), 1, input);
            fArray[index] = value;
        }
        *frequencyArray = fArray;
    }
    
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

// Funcao utilizada pelo qsort para ordenar os TreeNodes da fila em ordem crescente.
// O qsort necessita de uma funcao de comparacao definida como int (*compareFrequencies)(const void*,const void*)
// (fonte: http://www.cplusplus.com/reference/cstdlib/qsort/). Como ao acessar o vetor a ser ordenado
// o qsort utiliza enderecos dos elementos, e nossa fila ja e um vetor de ponteiros, e necessario
// realizar o casting de a e b como sendo ponteiros de ponteiros para TreeNode.
int compareFrequencies(const void *a, const void *b)
{
    const TreeNode **left = (const TreeNode **) a, **right = (const TreeNode **) b;
    if((*left)->frequency == (*right)->frequency) return 0;
    else return ((*left)->frequency < (*right)->frequency) ? 1 : -1;
}

TreeNode *createTree(unsigned int *frequencies , unsigned int maxValue)
{
    unsigned int i;
    int position = 0;
    TreeNode **queue = (TreeNode **) malloc(maxValue * sizeof(TreeNode *));
    
    /* create trees for each character, add to the queue */
    for(i = 0; i < maxValue; i++)
    {
        if(frequencies[i] > 0)
        {
            TreeNode *toBeAdded = (TreeNode *) malloc(sizeof(TreeNode));
            toBeAdded->value = i;
            toBeAdded->frequency = frequencies[i];
            toBeAdded->left = NULL;
            toBeAdded->right = NULL;
            
            queue[position++] = toBeAdded;
        }
    }
    
    while(position > 1)
    {
        TreeNode *newNode = (TreeNode *) malloc(sizeof(TreeNode));
        
        // Utilização do qsort para ordenar a fila de frequencias
        // A funcao compareFrequencies e passada como parametro para qsort
        // tendo sido definida de acordo com os requisitos para o qsort
        // Ordenacao e feita em ordem crescente
        qsort(queue, position, sizeof(TreeNode *), compareFrequencies);
        
        // Assim, os dois ultimos elementos da fila sao os que tem menor frequencia.
        // Eles sao utilizados para formar um novo no, cuja frequencia e a soma
        // das frequencias dos ultimos elementos da fila
        newNode->left = queue[position - 1];
        position--;
        newNode->right = queue[position - 1];
        position--;
        newNode->frequency = newNode->left->frequency + newNode->right->frequency;
        
        // O penultimo elemento da fila e entao substituido pelo novo no, que tem
        // como filho os dois ultimos elementos da etapa anterior. O numero de elementos
        // indicado pela variavel position e entao incrementado
        queue[position++] = newNode;
    }
    
    // Quando restar apenas um elemento na fila, a arvore estara completa.
    // A raiz da arvore e retornada.
    return queue[0];
}

void clearTree(TreeNode *node) {
    if(node != NULL) {
        clearTree(node->left);
        clearTree(node->right);
        free(node);
    }
}

char *huffman(char *data, unsigned long long int size, unsigned int huffmanFrequenciesCount, unsigned int huffmanMaxValue, unsigned int * frequencyArray, unsigned long long int *huffmanSize) {
    
    TreeNode *tree = createTree(frequencyArray, huffmanMaxValue+1), *aux;
    List *values = create();
    
    *huffmanSize = 0;
    unsigned long long int i = 0, j = 0;
    unsigned long value;
    unsigned int numBits;
    printf("\n\nSIZE %llu\n\n\n", size);
    aux = tree;
    while(i < size) {
        if(aux->left || aux->right) {
            if(data[i])
                aux = aux->right;
            else
                aux = aux->left;
            
            if(!aux)
                return NULL;
            i++;
        }
        else {
            add(aux->value, values);
            (*huffmanSize) += BITS_PER_CHAR;
            aux = tree;
        }
    }
    
    char * huffmanDecoded = (char *) malloc(*huffmanSize * sizeof(char));
    
    Node *it = values->head;
    printf("\n\nCHOUZETSU\n\n");
    
    while(it) {
        value = it->data;
        numBits = BITS_PER_CHAR;
        if(value != 0){
            while(value && numBits) {
                huffmanDecoded[j + --numBits] = value % 2;
                value = value/2;
            }
        }
        long long int k;
        for(k = (long long int) numBits - 1; k >= 0; k--) {
            huffmanDecoded[j + k] = 0;
        }
        j += BITS_PER_CHAR;
        it = it->next;
    }
    
    clearList(values);
    clearTree(tree);
    return huffmanDecoded;
}

char *runlength(char *data, unsigned long long int size, unsigned int numBits, unsigned long long int *totalBitsLength) {
    
    size = size - (size % numBits);
    unsigned long long int i;
    unsigned long long int numberSamples = size/numBits;
    printf("\n\n\nnumSamples - %llu\nSize - %llu\nnumbits - %u\n\n\n", numberSamples, size, numBits);
    unsigned int currBit = 0, shift;
    unsigned long long int j = 0, currValue = 0;
    *totalBitsLength = 0;
    
    //printf("\n\n\nsize: %llu\n\n\nnumbits: %u\n\n\n", size, numBits);
    
    unsigned long long int *runlengthSamples = (unsigned long long int *) malloc(numberSamples*sizeof(unsigned long long int));
    
    for(i = 0; i < numberSamples; i++) {
        runlengthSamples[i] = 0;
    }
    
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
    
    if(fwrite(byteData, size/BITS_PER_CHAR, 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    //printf("\n\n\nJ FINAL - %llu\n\n\nSIZE - %llu\n\n\n", j, size);
    free(byteData);
    
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    FILE *input = NULL, *output = NULL; // descritores dos arquivos de entrada e saída a ser processado
    int fileSize; // tamanho total do arquivo
    
    if(argc <= 1) {
        printf("Usage: ./wave_reader file.wav\n");
        return EXIT_FAILURE;
    }
    
    if(openFiles(&input, &output, argv[1], "decompressed.wav")) {
        return EXIT_FAILURE;
    }
    
    wav_hdr *header = readHeader(input, &fileSize);
    enc_hdr *encode_header = readEncodeHeader(input);
    
    unsigned long long int dataBitsSize = encode_header->totalLength * BITS_PER_CHAR;
    unsigned long long int runlengthBitsSize;
    unsigned long long int huffmanSize;
    
    unsigned int *frequencyArray;
    
    char *dataBits = readData(input, header, encode_header, &frequencyArray);
    
    // FAZER AQUI AS CHAMADAS PARA AS CODIFICACOES
    // PROTOTIPO:
    // char *codificacaoMetodoX(char *input, char* size, int bitsperSample)
    // A CADA CHAMADA PARA UM METODO DIFERENTE, ATUALIZAMOS INPUT,
    // SIZE E BITSPERSAMPLE. NUM PRIMEIRO MOMENTO,
    // INPUT É dataBits, E SIZE E BITSPERSEAMPLE DIZEM RESPEITO AOS
    // DADOS DO ARQUIVO DE ENTRADA. DEPOIS, APOS A PRIMEIRA
    // CODIFICACAO, ESSES DADOS SAO ATUALIZADOS.
    
    if(encode_header->encodeType & HUFFMAN_MASK) {
        char *huffmanDecoded = huffman(dataBits, dataBitsSize, encode_header->huffmanFrequenciesCount, encode_header->huffmanMaxValue, frequencyArray, &huffmanSize);
        free(dataBits);
        dataBits = huffmanDecoded;
        dataBitsSize = huffmanSize;
    }
    
    if(encode_header->encodeType & RUNLENGTH_MASK) {
        char *runlengthDecoded = runlength(dataBits, dataBitsSize, encode_header->runlengthNumBits, &runlengthBitsSize);
        free(dataBits);
        dataBits = runlengthDecoded;
        dataBitsSize = runlengthBitsSize;
    }
    
    if(writeToOutput(output, header, dataBits, dataBitsSize)) {
        return EXIT_FAILURE;
    }
    
    free(frequencyArray);
    free(dataBits);
    free(header);
    free(encode_header);
    fclose(input);
    fclose(output);
    
    return EXIT_SUCCESS;
}
