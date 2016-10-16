#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main (int argc, char* argv[])
{
	struct arguments arguments;
	char verbose;

	parseArguments(argc, argv, &arguments);
	verbose = arguments.verbose;

	if (verbose)
	{
		printf("Uses Huffman? %s\n", arguments.huffman == 1 ? "yes" : "no");
		printf("Uses run-length? %s\n", arguments.runlength == 1 ? "yes" : "no");
		printf("Uses difference? %s\n", arguments.difference == 1 ? "yes" : "no");
		printf("Input file is %s\n", arguments.input);
		printf("Output file is %s\n", arguments.output);
		printf("\n");
	}

	if (!(arguments.huffman || arguments.runlength || arguments.difference) != 0)
	{
		printf("Nothing to do.\n");
		return 1;
	}

	/* Lógica principal */

	return 0;
}
