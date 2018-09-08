/* Acronym Management Tool (amt): cli-args.c */

#include "cli-args.h"

/* added to enable compile on MacOSX */
#ifndef __clang__
# include <malloc.h>    /* free for use with strdup */
#endif
#include <ctype.h>		/* isdigit */
#include <stdio.h>		/* printf */
#include <stdlib.h>		/* getenv */
#include <string.h>		/* strlen strdup */
#include <unistd.h>		/* strdup access */
#include <getopt.h>		/* getopt long */

/*
 * Used to parse command line options provided by the user.
 * Uses the POSIX compliant getopts()
 */
void get_cli_args(int argc, char **argv)
{
	opterr = 0;
	int c = 0;

	while ((c = getopt(argc, argv, "d:hns:u:")) != -1) {
		switch (c) {
		case 'd':
			if (!optarg || !isdigit(*optarg)) {
				fprintf(stderr,
					"\nERROR: for -d option please provide "
					"an acronym ID for removal.\nUse "
					"search function to "
					"locate correct record 'ID' first, as "
					"the provided "
					"argument '%s' is not valid.\n",
					optarg ? optarg : "");
				exit(EXIT_FAILURE);
			}
			del_rec_id = atoi(optarg);
			break;
		case 'h':
			help = 1;
			break;
		case 'n':
			newrec = 1;
			break;
		case 's':
			findme = strdup(optarg);
			if (strlen(findme) <= 0 || findme == NULL) {
				fprintf(stderr,
					"\nERROR: for -s option please provide "
					"an acronym to search for\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'u':
			if (!optarg || !isdigit(*optarg)) {
				fprintf(stderr,
					"\nERROR: for -u option please provide "
					"an acronym ID for update.\nUse "
					"search function to "
					"locate correct record 'ID' first, as "
					"the provided "
					"argument '%s' is not valid.\n",
					optarg ? optarg : "");
				exit(EXIT_FAILURE);
			}
			update_rec_id = atoi(optarg);
			break;
		case ':':
			fprintf(stderr,
				"\nERROR: '%s' option '-%c' requires an argument\n",
				argv[0], optopt);
			exit(EXIT_FAILURE);
			break;
		case '?':
		default:
			fprintf(stderr, "\nERROR: '%s' option '-%c' is invalid "
				"or missing input data\n", argv[0], optopt);
			exit(EXIT_FAILURE);
			break;
		}
	}
}
