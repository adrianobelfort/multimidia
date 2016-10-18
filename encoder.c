#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "List.h"

#define BITS_PER_CHAR 8

int applyRunlength, applyHuffman;

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

char *buildPath(char *currentPath, char toBeAdded) {
    char *newPath = (char *) malloc((strlen(currentPath)+2)*sizeof(char));
    int i;
    for(i = 0; currentPath[i]!='\0'; i++) {
        newPath[i] = currentPath[i];
    }
    newPath[i] = toBeAdded;
    newPath[i+1] = '\0';
    
    return newPath;
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


void buildTableFromTree(TreeNode *tree, char **table, char *path)
{
    if(tree->left == NULL && tree->right == NULL) table[tree->value] = path;
    else
    {
        if(tree->left != NULL) buildTableFromTree(tree->left, table, buildPath(path, '0'));
        if(tree-> right != NULL) buildTableFromTree(tree->right, table, buildPath(path, '1'));
        free(path);
    }
}

void clearTree(TreeNode *node) {
    if(node != NULL) {
        clearTree(node->left);
        clearTree(node->right);
        free(node);
    }
}

char **translationTable(unsigned int *frequencies, unsigned int maxValue) {
    char **translationTable = (char **) malloc(maxValue*sizeof(char *));
    char *path = (char *) malloc(sizeof(char));
    path[0] = '\0';
    TreeNode *tree = createTree(frequencies, maxValue);
    buildTableFromTree(tree, translationTable, path);
    clearTree(tree);
    
    return translationTable;
}

char *huffman(char *data, unsigned long long int size, unsigned int bitsPerSample, unsigned long long  int *huffmanSize, unsigned int **frequencyArray, unsigned char *huffmanMaxValue) {
    
    if(size == 0)
        return NULL;
    
    *huffmanSize = 0;
    List *values = create();
    
    unsigned long long int i;
    unsigned char currValue = 0, maxValue = 0;
    int currBit = 0;
    int shift;
    for(i = 0; i < size; i++) {
        if(currBit == bitsPerSample) {
            currBit = 0;
            add((unsigned int) currValue, values);
            if((unsigned int) currValue > (unsigned int) maxValue) {
                maxValue = currValue;
            }
            currValue = 0;
        }
        
        shift = bitsPerSample - 1 - currBit++;
        // Isola o bit do da amostra e atribui a posicao do vetor dataBits
        currValue |= data[i] << shift;
    }
    if(currValue != 0) {
        add(currValue, values);
        if(currValue > maxValue) {
            maxValue = currValue;
        }
    }
    
    *huffmanMaxValue = maxValue;
    
    unsigned int *fArray = (unsigned int *) malloc((maxValue+1) * sizeof(unsigned int));
    for(i = 0; i <= maxValue; i++) {
        fArray[i] = 0;
    }
    Node *aux = values->head;


    while(aux) {
        fArray[aux->data]++;
        aux = aux->next;
    }
    
    *frequencyArray = fArray;
    char **trTable = translationTable(fArray, maxValue+1);
    
//    printf("\n");
//    for(i = 0; i <= maxValue; i++) {
//        if(fArray[i] != 0) {
//            printf("Dado: %llu - Cod: %s\n", i, trTable[i]);
//
//        }
//    }
//    printf("\n");
    
    // Conta o numero de bits necessarios para representar a informacao.
    aux = values->head;
    while(aux) {
        *huffmanSize += strlen(trTable[aux->data]);
        aux = aux->next;
    }
    
    // Cria o vetor com os dados codificados em Huffman.
    char *huffmanData = (char *) malloc(*huffmanSize * sizeof(char));
    
    // Escreve os dados no vetor huffmanData.
    aux = values->head;
    unsigned long long int j = 0;
    while(aux) {
        //printf("%lu, ", aux->data);
        for(i = 0; i < strlen(trTable[aux->data]); i++)
            huffmanData[j++] = trTable[aux->data][i] - '0';
        aux = aux->next;
    }
    printf("\n");

    return huffmanData;
}

// AQUI, FALTA IMPLEMENTAR NOSSO PROPRIO HEADER. A IDEIA EH USAR UM CHAR
// OU UM INT PARA DIZER QUAIS OS TIPOS DE COMPRESSÃO FEITOS. APÓS O HEADER
// ORIGINAL, ANTES DOS DADOS, COLOCAMOS ESSE NOSSO "HEADER"
int writeToOutput(FILE *output, wav_hdr *header, char *data, unsigned long long int size, unsigned int runlengthNumBits, unsigned int *frequencyArray, unsigned char huffmanMaxValue) {
    
    enc_hdr encodeHeader;
    encodeHeader.encodeType = 0;
    encodeHeader.runlengthNumBits = 0;
    encodeHeader.huffmanFrequenciesCount = 0;
    encodeHeader.huffmanMaxValue = 0;
    
    if(applyRunlength) {
        encodeHeader.encodeType |= 0x1;
        encodeHeader.runlengthNumBits = runlengthNumBits;
        encodeHeader.totalLength = size/BITS_PER_CHAR;
        printf("\n\ntotalLength: %llu\n\n", encodeHeader.totalLength);
    }
    
    if(applyHuffman) {
        encodeHeader.encodeType |= 0b10;
        encodeHeader.huffmanMaxValue = (unsigned int) huffmanMaxValue;
//        printf("\nMAXVALUE: %lu\n\n", encodeHeader.huffmanMaxValue);
        encodeHeader.totalLength = size/BITS_PER_CHAR;
        unsigned long int i;
        for(i = 0; i <= huffmanMaxValue; i++) {
            if(frequencyArray[i] != 0)
                encodeHeader.huffmanFrequenciesCount++;
        }
//        printf("\n\nFrequency Count: %u\n\n", encodeHeader.huffmanFrequenciesCount);

    }
    // 1 e o numero de elementos a serem escritos. 1 header apenas
    if(fwrite(header, sizeof(wav_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }

    // 1 e o numero de elementos a serem escritos. 1 encode header apenas
    if(fwrite(&encodeHeader, sizeof(enc_hdr), 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    
//    printf("\nEncHeader: %lu\n", sizeof(enc_hdr));
    
    if(applyHuffman) {
        unsigned int i;


        for(i = 0; i <= huffmanMaxValue; i++) {
            if(frequencyArray[i] != 0) {
                fwrite(&i, sizeof(i), 1, output);
                fwrite(&frequencyArray[i], sizeof(frequencyArray[i]), 1, output);
            }
        }

    }
    
    char *byteData = (char *) malloc(size/BITS_PER_CHAR * sizeof(char));
    unsigned long long int i, j = 0;
    char currByte = 0;
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
        // Isola o bit do byte e atribui a posicao do vetor dataBits
        currByte |= data[i] << shift;
    }
    
//    printf("\n\nDADOS TAMANHO: %llu\n\n", size/BITS_PER_CHAR);
    
    if(fwrite(byteData, size/BITS_PER_CHAR, 1, output)!= 1) {
        return EXIT_FAILURE;
    }
    
    free(byteData);
    
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    FILE *input = NULL, *output = NULL; // descritores dos arquivos de entrada e saída a ser processado
    int fileSize; // tamanho total do arquivo
    
    /*******/
    applyRunlength = 0;// VARIAVEL AUXILIAR PARA DETERMINAR
    // QUANDO RUNLENGTH DEVE SER USADO. MUDAR DE ACORDO COM A
    // LOGICA DO DRIKA.
    /*******/
    applyHuffman = 1;

    
    if(argc <= 1) {
        printf("Usage: ./wave_reader file.wav\n");
        return EXIT_FAILURE;
    }
    
    if(openFiles(&input, &output, argv[1], "output.wav")) {
        return EXIT_FAILURE;
    }
    
    wav_hdr *header = readHeader(input, &fileSize);
    
    char *dataBits = readData(input, header);
    unsigned long long int dataBitsSize = header->Subchunk2Size * BITS_PER_CHAR;
    unsigned int numBits = header->bitsPerSample;
    unsigned long long int runlengthSize;
    unsigned long long int huffmanSize;
    unsigned char huffmanMaxValue;
    
    unsigned int *frequencyArray;
    unsigned int runlengthNumBits = 0;

    // FAZER AQUI AS CHAMADAS PARA AS CODIFICACOES
    // PROTOTIPO:
    // char *codificacaoMetodoX(char *input, char* size, int bitsperSample)
    // A CADA CHAMADA PARA UM METODO DIFERENTE, ATUALIZAMOS INPUT,
    // SIZE E BITSPERSAMPLE. NUM PRIMEIRO MOMENTO,
    // INPUT É dataBits, E SIZE E BITSPERSEAMPLE DIZEM RESPEITO AOS
    // DADOS DO ARQUIVO DE ENTRADA. DEPOIS, APOS A PRIMEIRA
    // CODIFICACAO, ESSES DADOS SAO ATUALIZADOS.
    
    if(applyRunlength) {
        char *runlengthBits = runlength(dataBits, dataBitsSize, &runlengthSize, &runlengthNumBits);
        if(!runlengthBits)
            return EXIT_FAILURE;
        free(dataBits);
        dataBits = runlengthBits;
        numBits = runlengthNumBits;
        dataBitsSize = runlengthSize;
        dataBitsSize = dataBitsSize - (dataBitsSize % BITS_PER_CHAR);
    }
    
    if(applyHuffman) {
        char *huffmanBits = huffman(dataBits, dataBitsSize, BITS_PER_CHAR, &huffmanSize, &frequencyArray, &huffmanMaxValue);
        if(!huffmanBits)
            return EXIT_FAILURE;
        free(dataBits);
        dataBits = huffmanBits;
        dataBitsSize = huffmanSize;
    }
    
    if(writeToOutput(output, header, dataBits, dataBitsSize, runlengthNumBits, frequencyArray, huffmanMaxValue)) {
        return EXIT_FAILURE;
    }
    
    free(dataBits);
    free(header);
    fclose(input);
    fclose(output);
    
    return EXIT_SUCCESS;
}
