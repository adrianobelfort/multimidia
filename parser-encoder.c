#include "parser.h"
#include <argp.h>

/***************************** ARGP HANDLING *******************************/

const char* argp_program_version = "Zato Encoder v1.0";
const char* argp_program_bug_address = "Adriano Belfort <adriano.belfort@outlook.com>, Henrique Silveira <henrique.tijolo@gmail.com>";

/* Program documentation */

static char documentation[] = "\nZato - encoding wave after wave\
\vEncodes .wav files using differential, run-length and Huffman coding\n";

/* A description of the arguments we accept */
static char args_doc[] = "<input file> <output file>";

/* The options we understand */
static struct argp_option options[] = {
/* long name | character | value | option flag | description */

    {"huffman", 'h', 0, 0, "Compress audio data with Huffman coding"},
	{"carreira", 'c', 0, 0, "Compress audio data with run-length coding"},
    {"difference", 'd', 0, 0, "Compress audio data with difference coding"},
	{"verbose", 'v', 0, 0, "Shows verbose output"},
    {0}
};

/* Parse a single option */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    /*  Get the input argument from argp_parse, which we
        know is a pointer to our arguments structure */
    struct arguments *arguments = state->input;

    /*  We set the right flags and variables according to
        what we get as key  */
    switch (key)
    {
        case 'h':
            arguments->huffman = 1;
            break;
        case 'c':
            arguments->runlength = 1;
            break;
		case 'd':
			arguments->difference = 1;
			break;
		case 'v':
			arguments->verbose = 1;
			break;

		case ARGP_KEY_NO_ARGS:
			argp_usage(state);
			break;

		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
			{
				/* Too many arguments */
				argp_usage(state);
			}

			if (state->arg_num == 0)
			{
				arguments->input = arg;
			}

			if (state->arg_num == 1)
			{
				arguments->output = arg;
			}

			break;

		default:
			return ARGP_ERR_UNKNOWN;
    }
	return 0;
}

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

/* Definition of our argp parser */
static struct argp argp = {options, parse_opt, args_doc, documentation};

/*************************** END OF ARGP HANDLING **************************/

void parseArguments(int argc, char* argv[], struct arguments *arguments)
{
	initializeArguments(arguments);
	argp_parse(&argp, argc, argv, 0, 0, arguments);
}
