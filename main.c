/* Acronym Management Tool (amt): main.c */

#include "main.h"

/* added to enable compile on MacOSX */
#ifndef __clang__
# include <malloc.h>    /* free for use with strdup */
#endif
#include <locale.h>		/* number output formatting with commas */
#include <stdio.h>		/* printf */
#include <stdlib.h>		/* getenv */
#include <string.h>		/* strlen strdup */
#include <unistd.h>		/* strdup access */

int main(int argc, char **argv)
{
	atexit(exit_cleanup);
	setlocale(LC_NUMERIC, "");
	/* Set the default locale values according to environment variables. */
	// setlocale (LC_ALL, "");

	char *prog_name = strdup(argv[0]);
	if (prog_name == NULL) {
		fprintf(stderr, "ERROR: unable to set program name\n");
	}

	get_cli_args(argc, argv);

	print_start_screen(prog_name);

	if (help) {
		show_help();
		return EXIT_SUCCESS;
	}

	check4DB(prog_name);

	/* done with this now - used by: */
	if (prog_name != NULL) {
		free(prog_name);
	}

	sqlite3_initialize();
	rc = sqlite3_open_v2(dbfile, &db,
			     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (rc != SQLITE_OK) {
		exit(EXIT_FAILURE);
	}

	int totalrec = get_rec_count();
	printf(" - Current record count is: %'d\n", totalrec);

	char *lastacro = get_last_acronym();
	printf(" - Newest acronym is: %s\n", lastacro);
	if (lastacro != NULL) {
		free(lastacro);
	}

	/* perform a database acronym search */
	if (findme != NULL) {
		int rec_match = 0;
		rec_match = do_acronym_search(findme);
		printf("\nDatabase search found '%'d' matching records\n",
		       rec_match);
	}

	/* add a new acronym record */
	if (newrec) {
		int add_worked = new_acronym();
		if (add_worked) {
			printf("\nADD DONE");
		}
	}

	/* delete an acronym record */
	if (del_rec_id >= 0) {
		int del_worked = del_acro_rec(del_rec_id);
		if (del_worked) {
			printf("\nDELETE DONE");
		}
	}

	/* update an acronym record */
	if (update_rec_id >= 0) {
		int update_worked = update_acro_rec(update_rec_id);
		if (update_worked) {
			printf("\nUPDATE DONE");
		}
	}

	return (EXIT_SUCCESS);
}

void exit_cleanup(void)
{
	if (db == NULL) {
		printf
		    ("\nNo SQLite database shutdown required\n\nAll is well\n");
		exit(EXIT_SUCCESS);
	}

	rc = sqlite3_close_v2(db);
	if (rc != SQLITE_OK) {
		fprintf(stderr,
			"\nWARNING: error '%s' when trying to close the database\n",
			sqlite3_errstr(rc));
		exit(EXIT_FAILURE);
	}

	sqlite3_shutdown();
	printf("\n"
	       "Completed SQLite database shutdown\n"
	       "Run  with '-h' for help with available command line flags\n\n"
	       "All is well\n");

	/* free any global varables below */
	if (findme != NULL) {
		free(findme);
	}
	exit(EXIT_SUCCESS);
}

void print_start_screen(char *prog_name)
{
	printf("\n"
	       "\t\tAcronym Management Tool\n"
	       "\t\t¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n"
	       "Summary:\n"
	       " - '%s' version is: %s complied with SQLite version: %s\n",
	       prog_name, appversion, SQLITE_VERSION);
}

void show_help(void)
{
	printf("\n"
	       "Help Summary:\n"
	       "The following command line switches can be used:\n"
	       "\n"
	       "  -d ?  Delete : remove an acronym where ? == ID of record to delete\n"
	       "  -h    Help   : show this help information\n"
	       "  -n    New    : add a new acronym record to the database\n"
	       "  -s ?  Search : find an acronym where ? == acronym to locate\n"
	       "  -u ?  Update : update an acronym where ? == ID of record to update\n");
}
