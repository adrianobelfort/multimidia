#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>

/* Struct used to exchange information between parse_opt and main */
struct arguments
{
    char huffman,         	/* -h, --huffman */
    runlength,         /* -c, --carreira */
    difference,		/* -d, --diferenca */
    verbose,			/* --verbose */
    *input,			/* input file */
    *output;			/* output file */
};

int parseArguments(int argc, char* argv[], struct arguments *arguments, char mode);

#endif
