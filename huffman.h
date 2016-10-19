#ifndef huffman_h
#define huffman_h

#include <stdio.h>

typedef struct tn {
    unsigned long int value;
    unsigned int frequency;
    struct tn *left;
    struct tn* right;
} TreeNode;

int compareFrequencies(const void *a, const void *b);
char *buildPath(char *currentPath, char toBeAdded);
TreeNode *createTree(unsigned int *frequencies , unsigned int maxValue);
void buildTableFromTree(TreeNode *tree, char **table, char *path);
void clearTree(TreeNode *node);
char **translationTable(unsigned int *frequencies, unsigned int maxValue);
char *huffmanEncode(char *data, unsigned long long int size, unsigned int bitsPerSample, unsigned long long  int *huffmanSize, unsigned int **frequencyArray, unsigned char *huffmanMaxValue);
char *huffmanDecode(char *data, unsigned long long int size, unsigned int huffmanFrequenciesCount, unsigned int huffmanMaxValue, unsigned int * frequencyArray, unsigned long long int *huffmanSize);

#endif /* huffman_h */
