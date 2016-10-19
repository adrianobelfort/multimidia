#include "runlength.h"

#define BITS_PER_CHAR 8

/* Funcao utilizada para obter-se o numero de bits necessario para
 * representr um certo numero "number"
 */
unsigned int findNumBits(unsigned long long int number) {
    
    unsigned int bit = sizeof(unsigned long long) * BITS_PER_CHAR - 1;
    unsigned int mask = 0x1;
    
    while(!((number & (mask << bit)) >> bit)) {
        bit--;
    }
    return bit+1;
}

/* Funcao utilizada para converter "carreiras" armazenadas em uma lista, "runs",
 * em um stream de bits.
 */
int convertRunLengthToBitsEncode(char *runLengthBits, unsigned long int size, List *runs, unsigned int numBits) {
    
    int i = 0, currBit;
    unsigned long int currRun;
    
    /* Ponteiro auxiliar que aponta para a cabeca da lista */
    Node *aux = runs->head;
    
    /* Percorremos toda a lista */
    while(aux) {
        /* O valor da primeira carreira e lido */
        currRun = aux->data;
        currBit = 0;
        
        /* O valor e entao codificado em binario, sendo que
         * cada posicao do vetor armazenara um digito da
         representacao binaria da carreira
         */
        while(currRun > 0) {
            runLengthBits[i + numBits - 1 - currBit] = currRun % 2;
            currRun /= 2;
            currBit++;
        }
        
        /* Os bits restantes necessarios para se atingir o numero
         * de bits por carreira sendo utilizado sao preenchidos
         * com zero.
         */
        while(currBit < numBits) {
            runLengthBits[i + numBits - 1 - currBit] = 0;
            currBit++;
        }
        
        i += numBits;
        aux = aux->next;
    }
    
    return EXIT_SUCCESS;
}

/* Funcao que recebe um stream de bits e codifica-o em outro stream de bits
 * apos aplicacao da codificacao runlength.
 */
char *runlengthEncode(char *data, unsigned long long int size, unsigned int *runlengthPadding, unsigned long long  int *runlengthSize, unsigned int *numBits) {
    
    if(size == 0)
        return NULL;
    
    /* Uma lista e utilizada para armazenar os valores das carreiras. */
    List *runs = create();
    unsigned long int maxRun = 1, currentRun = 1;
    char lastBit = data[0];
    
    /* Caso o primeiro bit do stream de entrada seja 1, como aqui assumimos
     que a contagem de carreiras se inicia em um bit 0, precisamos indicar
     que a primeira careira de bit 0 tem tamanho zero. */
    if(lastBit == 1) {
        add(0, runs);
    }
    
    /* O stream de entrada e lido e as carreiras sao computadas,
     * sendo adicionadas na lista. A maior carreira encontrada ate o
     * momento e armazenada para que a codificacao posteriormente
     * seja feita com o menor numero de bits necessarios
     */
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
    
    /* Adiciona a ultima carreira na lista caso ela nao tenha sido
     * adicionada.
     */
    if(currentRun != 0) {
        add(currentRun, runs);
    }
    
    
    /* O minimo numero de bits necessario para representacao das carreiras
     * e encontrado.
     */
    *numBits = findNumBits(maxRun);
    
    /* O tamanho do stream de saida e calculado, sendo ajustado
     * para que seja um multiplo de 8 (BITS_PER_CHAR)
     */
    *runlengthSize = *numBits * runs->size;
    if(*runlengthSize % BITS_PER_CHAR != 0) {
        *runlengthPadding = BITS_PER_CHAR - (*runlengthSize % BITS_PER_CHAR);
        *runlengthSize += *runlengthPadding;
    }
    
    /* Um vetor para o stream de saida e alocado e as carreiras
     * sao convertidas para bits
     */
    char *runlengthBits = (char *) malloc(*runlengthSize);
    if(convertRunLengthToBitsEncode(runlengthBits, *runlengthSize, runs, *numBits)) {
        free(runlengthBits);
        return NULL;
    }
    
    clearList(runs);
    return runlengthBits;
}

/* Funcao utilizada para converter valores de carreira em bits na decodificacao */
char *convertRunLengthToBitsDecode(unsigned long long int totalBitsLength, unsigned long long int *samples, unsigned long int numberSamples) {
    
    /* Vetor que armazenara o stream de bits e criado */
    char *runLengthBits = (char *) malloc(totalBitsLength);
    
    unsigned long int i;
    unsigned long long int j, offset = 0;
    unsigned int current = 0;
    
    /* Quando lemos uma carreira, inserimos um numero de bits '0' ou '1'*/
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

/* Funcao que dedocidifca um stream de bits codificado com Runlength */
char *runlengthDecode(char *data, unsigned long long int size, unsigned int numBits, unsigned long long int *totalBitsLength, unsigned int runlengthPadding) {
    
    /* O tamanho do vetor de bits e ajustado de acordo com o padding
     * que foi utilizado no encoding
     */
    size -= runlengthPadding;
    size = size - (size % numBits);
    unsigned long long int i;
    unsigned long long int numberSamples = size/numBits;
    unsigned int currBit = 0, shift;
    unsigned long long int j = 0, currValue = 0;
    *totalBitsLength = 0;
    
    /* Um vetor para armazenar as amostras de Runlength e criado e inicializado*/
    unsigned long long int *runlengthSamples = (unsigned long long int *) malloc(numberSamples*sizeof(unsigned long long int));
    
    for(i = 0; i < numberSamples; i++) {
        runlengthSamples[i] = 0;
    }
    
    /* O stream de entrada e percorrido, sendo que cada carreira e lida
     * a partir da interpretacao de blocos de bits
     */
    for(i = 0; i < size; i++) {
        if(currBit == numBits)
        {
            *totalBitsLength += currValue;
            runlengthSamples[j++] = currValue;
            currBit = 0;
            currValue = 0;
        }
        
        shift = numBits - 1 - currBit++;
        currValue |= data[i] << shift;
    }
    
    /* O stream de saida e criado ao converter as carreiras para sua representacao
     * binaria
     */
    char *dataBits = convertRunLengthToBitsDecode(*totalBitsLength * sizeof(char), runlengthSamples, numberSamples);
    
    return dataBits;
}


