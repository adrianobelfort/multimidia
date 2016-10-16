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
		printf("Input file is %s\n", arguments.input);
		printf("Output file is %s\n", arguments.output);
	}

	/* Lógica principal */

	return 0;
}
