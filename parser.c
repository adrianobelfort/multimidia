#include "parser.h"
#include <string.h>

void initializeArguments(struct arguments* arguments)
{
    /* Default values for the arguments */
    arguments->huffman = 0;
    arguments->runlength = 0;
    arguments->difference = 0;
    arguments->verbose = 0;
    arguments->input = NULL;
    arguments->output = NULL;
}

int showErrorMessageDecode() {
    printf("\nIncorrect usage! Run this program as below:\n");
    printf("\n./decode [-v] <entrada.wav> <saida.bin>\n");
    printf("\nFor help, type ./decode --help. For verbose output, use the -v flag.\n");
    return 0;
}

int showErrorMessageEncode() {
    printf("\nIncorrect usage! Run this program as below:\n");
    printf("\n./encode [-d] [-c] [-h] [-v] <entrada.wav> <saida.bin>\n");
    printf("\nFor help, type ./encode --help. For verbose output, use the -v flag.\n");
    return 0;
}

int showHelpEncode() {
    printf("\nZato Encoder v1.0\n");
    printf("Encoding wave after wave\nEncodes .wav files using differential, run-length and Huffman coding\n");
    printf("Usage:\n");
    printf("\n./encode [-d] [-c] [-h] [-v] <entrada.wav> <saida.bin>\n");
    printf("-d: Compress audio data with difference coding\n");
    printf("-c: Compress audio data with run-length coding\n");
    printf("-h: Compress audio data with Huffman coding\n");
    printf("\nFor help, type ./encode --help. For verbose output, use the -v flag.\n");
    printf("Created by Adriano Belfort <adriano.belfort@outlook.com>, Henrique Silveira <henrique.tijolo@gmail.com>\n");
    return 0;
}

int showHelpDecode() {
    printf("\nZato Decoder v1.0\n");
    printf("Decoding wave after wave\nDecodes compressed .wav files using differential, run-length and Huffman coding\n");
    printf("Usage:\n");
    printf("\n./decode [-v] <entrada.wav> <saida.bin>\n");
    printf("\nFor help, type ./decode --help. For verbose output, use the -v flag.\n");
    printf("Created by Adriano Belfort <adriano.belfort@outlook.com>, Henrique Silveira <henrique.tijolo@gmail.com>\n");
    return 0;
}


int parseArguments(int argc, char* argv[], struct arguments *arguments, char mode)
{
    int i;
    initializeArguments(arguments);
    
    if(argc < 2) {
        return mode == 'e' ? showErrorMessageEncode() : showErrorMessageDecode();
    }
    
    if(argc == 2) {
        if(strcmp(argv[1], "--help")!= 0) {
            return mode == 'e' ? showErrorMessageEncode() : showErrorMessageDecode();
        } else {
            return mode == 'e' ? showHelpEncode() : showHelpDecode();
        }
    }
    
    if(mode == 'e') {
        for(i = 1; i < argc - 2; i++) {
            if(strcmp(argv[i], "-d") == 0) {
                arguments->difference = 1;
            } else if(strcmp(argv[i], "-c") == 0) {
                arguments->runlength = 1;
            } else if(strcmp(argv[i], "-h") == 0) {
                arguments->huffman = 1;
            } else if(strcmp(argv[i], "-v") == 0) {
                arguments->verbose = 1;
            }
            else {
                return showErrorMessageEncode();
            }
        }
    } else {
        if(argc > 4)
            return showErrorMessageDecode();
        
        if(argc == 4 && strcmp(argv[1], "-v") == 0)
            arguments->verbose = 1;
        else if(argc == 4 && strcmp(argv[1], "-v") != 0)
            return showErrorMessageDecode();
    }
    
    arguments->input = argv[argc-2];
    arguments->output = argv[argc-1];
    
    return 1;
}
