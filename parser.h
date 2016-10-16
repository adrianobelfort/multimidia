#ifndef __PARSER_H__
#define __PARSER_H__

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

void parseArguments(int argc, char* argv[], struct arguments *arguments);

#endif
