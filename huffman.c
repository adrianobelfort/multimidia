#include "huffman.h"
#include <stdlib.h>
#include <string.h>
#include "List.h"
#include "utils.h"

/* Funcao utilizada pelo qsort para ordenar os TreeNodes da fila em ordem crescente.
 * O qsort necessita de uma funcao de comparacao definida como int (*compareFrequencies)(const void*,const void*)
 * (fonte: http://www.cplusplus.com/reference/cstdlib/qsort/). Como ao acessar o vetor a ser ordenado
 * o qsort utiliza enderecos dos elementos, e nossa fila ja e um vetor de ponteiros, e necessario
 * realizar o casting de a e b como sendo ponteiros de ponteiros para TreeNode.
 */
int compareFrequencies(const void *a, const void *b) {
    /* Recebe dois ponteiros, realizando casting para TreeNode, e compara frequencias */
    const TreeNode **left = (const TreeNode **) a, **right = (const TreeNode **) b;
    if((*left)->frequency == (*right)->frequency) return 0;
    else return ((*left)->frequency < (*right)->frequency) ? 1 : -1;
}

/* Funcao utilizada para construir uma string que simboliza o caminho sendo
 * percorrido pela arvore de Huffman. Dado um vetor de caracteres representando
 * o caminho atual (currentPath) e um caractere a ser adicionado, o caractere
 * e concatenado ao caminho atual apos um vetor de maior tamanho ser alocado
 * para comportar o caractere extra
 */
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

/* Funcao que cria a arvore de Huffman a partir do vetor de frequencias de valores. */
TreeNode *createTree(unsigned int *frequencies , unsigned int maxValue) {
    
    unsigned int i;
    
    /* position indica a posicao atual do vetor e, tambem, o tamanho da fila */
    int position = 0;
    
    /* Um vetor de (TreeNode *)'s e criado para armazenar cada nó inicialmente.
     * Cada no aqui e um no folha, representando um valor e uma frequencia.
     * Como a indexacao de frequencias e feita por meio do maior valor
     * encontrado no metodo Huffman (maxValue), esse valor tambem e recebido como
     * parametro e o vetor de (TreeNode *)'s e alocado com esse tamanho.
     * O vetor funcionara como uma fila, ordenando os TreeNode * por frequencia.
     */
    TreeNode **queue = (TreeNode **) malloc(maxValue * sizeof(TreeNode *));
    
    /* Itera sobre o vetor de frequencias e aloca um TreeNode * para cada uma,
     * inserindo-o na fila.
     */
    for(i = 0; i < maxValue; i++)
    {
        /* So cria TreeNode * para frequencias nao nulas. */
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
    
    if(DEBUG_FLAG) {
        printf("\n");
        for(i = 0; i < position; i++) {
            printf("(%lu, %d)\n", queue[i]->value, queue[i]->frequency);
        }
    }
    
    /* Enquanto a arvore nao estiver formada, havendo apenas uma raiz */
    while(position > 1)
    {
        /* Um novo no e alocado para ser o no pai das duas menores frequencias
         * filhas disponiveis no momento.
         */
        TreeNode *newNode = (TreeNode *) malloc(sizeof(TreeNode));
        
        newNode->value = -1;
        
        /* Utilização do qsort para ordenar a fila de frequencias:
         * A funcao compareFrequencies e passada como parametro para qsort
         * tendo sido definida de acordo com os requisitos para o qsort.
         * Ordenacao e feita em ordem crescente.
         */
        qsort(queue, position, sizeof(TreeNode *), compareFrequencies);
        
        /* Assim, os dois ultimos elementos da fila sao os que tem menor frequencia.
         * Eles sao utilizados para formar um novo no, cuja frequencia e a soma
         * das frequencias dos ultimos elementos da fila.
         */
        newNode->left = queue[position - 1];
        position--;
        newNode->right = queue[position - 1];
        position--;
        newNode->frequency = newNode->left->frequency + newNode->right->frequency;
        
        /* O penultimo elemento da fila e entao substituido pelo novo no, que tem
         * como filho os dois ultimos elementos da etapa anterior. O numero de elementos
         * indicado pela variavel position e entao incrementado.
         */
        queue[position++] = newNode;
    }
    
    /* Quando restar apenas um elemento na fila, a arvore estara completa.
     * A raiz da arvore e retornada.
     */
    return queue[0];
}

/* Funcao recursiva para construcao da tabela de correspondencias
 * (valor, encoding) a partir da arvore de Huffman.
 */
void buildTableFromTree(TreeNode *tree, char **table, char *path) {
    
    /* Caso o no atual, designado por tree, seja um no folha, ou seja,
     * seus filhos da direita e da esquerda forem nulos, a recursão termina
     * e a tabela na linha correspondente ao no folha atual e preenchida com
     * o caminho obtido (que e o encoding do valor).
     */
    if(tree->left == NULL && tree->right == NULL) table[tree->value] = path;
    else
    {
        /* Caso contrario, se houver filho da esquerda, chama recursivamente a funcao para
         * ele, concatenando ao caminho atual o caradctere '0' via funcao buildPath()
         */
        if(tree->left != NULL) buildTableFromTree(tree->left, table, buildPath(path, '0'));
        /* O mesmo acontece para o filho da direita. */
        if(tree-> right != NULL) buildTableFromTree(tree->right, table, buildPath(path, '1'));
        free(path);
    }
}

/* Libera os recursos alocados para a arvore, de maneira recursiva. */
void clearTree(TreeNode *node) {
    if(node != NULL) {
        clearTree(node->left);
        clearTree(node->right);
        free(node);
    }
}

/* Funcao auxiliar na construcao da tabela de traducao de (valores, encoding).
 * Atraves dessa funcao e obtida a tabela. Primeiramente, e alocada memoria para
 * comportar a tabela, sendo um vetor de (char *), ja que cada linha sera um encoding
 * indexado por valor. E criado o caminho (path) inicial, vazio, e entao
 * a arvore e criada via createTree() e a tabela e construida via buildTableFromTree().
 */
char **translationTable(unsigned int *frequencies, unsigned int maxValue) {
    char **translationTable = (char **) malloc(maxValue*sizeof(char *));
    char *path = (char *) malloc(sizeof(char));
    path[0] = '\0';
    TreeNode *tree = createTree(frequencies, maxValue);
    buildTableFromTree(tree, translationTable, path);
    clearTree(tree);
    
    return translationTable;
}

/* Funcao que retorna um stream de bits correspondente a codificacao de Huffman,
 * tendo ocmo entrada um outro stream de bits designado por data.
 */
char *huffmanEncode(char *data, unsigned long long int size, unsigned int bitsPerSample, unsigned long long  int *huffmanSize, unsigned int **frequencyArray, unsigned char *huffmanMaxValue) {
    
    if(size == 0)
        return NULL;
    
    *huffmanSize = 0;
    List *values = create();
    
    unsigned long long int i;
    unsigned char currValue = 0, maxValue = 0;
    int currBit = 0;
    int shift;
    
    /* Agrupamos os bits do stream original, em blocos de tamanho bitsPerSample e armazenamos tais
     * valores em uma lista chamada values. O valor maxValue e atualizado sempre que um novo maximo
     * e encontrado.
     */
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
        /* Isola o bit do da amostra e atribui a posicao do vetor dataBits */
        currValue |= data[i] << shift;
    }
    
    /* Armazena o ultimo bloco de bits interpretado, caso ele ainda nao tenha
     * sido armazenado.
     */
    if(currValue != 0) {
        
        add(currValue, values);
        
        if(currValue > maxValue) {
            maxValue = currValue;
        }
    }
    
    /* O maximo valor encontrado na codificacao Huffman e armazenado em huffmanMaxValue
     * para que possa ser acessado pela funcao que chamou esta funcao atual.
     */
    *huffmanMaxValue = maxValue;
    
    /* Um vetor de frequencias, fArray, e criado para armazenar as frequencias dos
     * valores da codificacao Huffman.
     */
    unsigned int *fArray = (unsigned int *) malloc((maxValue+1) * sizeof(unsigned int));
    for(i = 0; i <= maxValue; i++) {
        fArray[i] = 0;
    }
    
    /* A lista de valores e percorrida para realizar a contagem de frequencias. */
    Node *aux = values->head;
    
    while(aux) {
        fArray[aux->data]++;
        aux = aux->next;
    }
    
    /* O vetor de frequencias pode ser retornado para a funcao que chamou esta
     * funcao atual via frequencyArray
     */
    *frequencyArray = fArray;
    
    /* A tabela de traducao (valor, encoding) e criada. */
    char **trTable = translationTable(fArray, maxValue+1);
    
    /* Conta o numero de bits necessarios para representar a informacao codificada, saida
     * do algoritmo de Huffman
     */
    aux = values->head;
    while(aux) {
        *huffmanSize += strlen(trTable[aux->data]);
        aux = aux->next;
    }
    
    /* Cria o vetor que armazenara os dados codificados em Huffman. */
    char *huffmanData = (char *) malloc(*huffmanSize * sizeof(char));
    
    /* Escreve os dados no vetor huffmanData. */
    aux = values->head;
    unsigned long long int j = 0;
    
    while(aux) {
        for(i = 0; i < strlen(trTable[aux->data]); i++)
            huffmanData[j++] = trTable[aux->data][i] - '0';
        aux = aux->next;
    }
    
    return huffmanData;
}

void preOrder(TreeNode *tree) {
    if(tree==NULL)
        return;
    preOrder(tree->left);
    printf("\n(%lu, %d)\n", tree->value, tree->frequency);
    preOrder(tree->right);
}

/* Decodifica um stream de bits que foi codificado utilizando o codigo de Huffman */
char *huffmanDecode(char *data, unsigned long long int size, unsigned int huffmanFrequenciesCount, unsigned int huffmanMaxValue, unsigned int * frequencyArray, unsigned long long int *huffmanSize) {
    
    /* A arvore de Huffman e criada a partir das frequencias */
    TreeNode *tree = createTree(frequencyArray, huffmanMaxValue+1), *aux;
    
    if(DEBUG_FLAG) {
        preOrder(tree);
    }
    
    List *values = create();
    
    *huffmanSize = 0;
    unsigned long long int i = 0, j = 0;
    unsigned long long test = 0;
    unsigned long value;
    unsigned int numBits;
    aux = tree;
    
    /* A arvore e percorrida de acordo com os bits sendo lidos. Quando chega-se a um
     * no folha, utiliza-se o valor armazenado nele e armazena-se tal valor em uma lista.
     */
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
            test += BITS_PER_CHAR;
            aux = tree;
        }
    }
    
    if(DEBUG_FLAG) {
        printf("\n\ni - %llu\ndatabits Size - %llu\n\n", i, size);
        printf("\n\nHuffmanSize - %llu\n\n", *huffmanSize);
    }
    
    /* O vetor para stream de bits decodificados e criado */
    char * huffmanDecoded = (char *) malloc(*huffmanSize * sizeof(char));
    
    Node *it = values->head;

    /* A lista criada com os valores e percorrida e os valors vao sendo escritos
     * no stream por meio de sua representacao binaria
     */
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
