/* Acronym Management Tool (amt): main.c */

#include "main.h"

/* added to enable compile on MacOSX */
#ifndef __clang__
# include <malloc.h>    /* free for use with strdup */
#endif

#include <locale.h>     /* number output formatting with commas */
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* getenv */
#include <string.h>     /* strlen strdup */

int main(int argc, char **argv) {

    //amtdb_struct amtdb;

    atexit(exit_cleanup);
    setlocale(LC_NUMERIC, "");
    /* Set the default locale values according to environment variables. */
    // setlocale (LC_ALL, "");

    #if DEBUG
    fprintf(stderr, "DEBUG: the programs was built in 'debug' mode\n");
    #endif

    const char *prog_name = argv[0];
    if ((NULL == prog_name) | (strlen(prog_name) <= 0)) {
        fprintf(stderr, "ERROR: unable to set program name\n");
    }

    if (!check4DBFile(prog_name, &amtdb)) {
        fprintf(stderr,"\n\tERROR: No suitable database file can be located - "
                       "program will exit\n");
        exit(EXIT_FAILURE);
    }

    if (!initialise_database(&amtdb)) {
        fprintf(stderr,"\n\tERROR: database initialisation failed - "
                       "program will exit\n");
        exit(EXIT_FAILURE);
    }

    int totalrec = get_rec_count(&amtdb);
    printf(" - Current record count is: %'d\n", totalrec);

    #if DEBUG
        fprintf(stderr, "DEBUG: getting last acronym.\n");
    #endif

    char *lastacro = get_last_acronym(&amtdb);
    printf(" - Newest acronym is: %s\n", lastacro);
    if (lastacro != NULL) {
        free(lastacro);
    }

    #if DEBUG
    fprintf(stderr, "DEBUG: checking command line args\n");
    #endif

    /** @note obtain any command line args from the user and action them */
    if (argc > 1) {

    #if DEBUG
    fprintf(stderr, "DEBUG: command lines args > 1\n");
    #endif

        /* @note HELP : print out help and exit */
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            show_help();
            return (EXIT_SUCCESS);
        }

        /* @note SEARCH : search for provided acronym */
        if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--search") == 0) {
            if (argc > 2 && strlen(argv[2]) > 0) {
                const int rec_match = do_acronym_search(argv[2],&amtdb);
                printf("\nDatabase search found '%'d' matching records\n",
                       rec_match);
                return (EXIT_SUCCESS);

            } else {
                fprintf(stderr, "\nERROR: for '-s' or '--search' option please provide "
                                "an acronym to search for.\n");
                exit(EXIT_FAILURE);
            }
        }

        /* @note NEW : add a new acronym via user prompts */
        if (strcmp(argv[1], "-n") == 0 || strcmp(argv[1], "--new") == 0) {
            int add_worked = new_acronym(&amtdb);
            if (add_worked) {
                printf("\nADD DONE");
                return (EXIT_SUCCESS);
            } else {
                fprintf(stderr, "ERROR: failed to complete adding new record.");
                exit(EXIT_FAILURE);
            }
        }

        /* @note DELETE : delete an acronym record */
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--delete") == 0) {
            if ( argc > 2 ) {
                long record_ID =  strtol(argv[2], NULL, 10);
                #if DEBUG
                fprintf(stderr, "DEBUG: parsed Record ID is: '%ld'\n",record_ID);
                #endif
                if ( record_ID > 0 && record_ID <= totalrec) {
                    const int del_worked = del_acro_rec((int)record_ID,&amtdb);
                    if (del_worked) {
                        printf("\nDELETE DONE");
                        return (EXIT_SUCCESS);
                    } else {
                        fprintf(stderr, "ERROR: failed to complete deleting the record.");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    fprintf(stderr, "ERROR: invalid record ID '%ld' - out of range.",record_ID);
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr,
                        "\nERROR: for -d option please provide "
                        "an acronym ID for removal.\nUse "
                        "search function to "
                        "locate correct record 'ID' first, as "
                        "the provided "
                        "argument '%s' is not valid.\n",
                        argv[2] ? argv[2] : "NOT SPECIFIED");
                exit(EXIT_FAILURE);
            }
        }

        /* @note UPDATE : update an acronym record */
        if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--update") == 0) {
            if ( argc > 2 ) {
                long record_ID = strtol(argv[2], NULL, 10);
                #if DEBUG
                fprintf(stderr, "DEBUG: parsed Record ID is: '%ld'\n",record_ID);
                #endif
                if (record_ID > 0 && record_ID <= totalrec) {
                    const int update_worked = update_acro_rec((int)record_ID,&amtdb);
                    if (update_worked) {
                        printf("\nUPDATE DONE");
                        return (EXIT_SUCCESS);
                    } else {
                        fprintf(stderr, "ERROR: failed to complete updating the record.");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    fprintf(stderr, "ERROR: invalid record ID '%ld' - out of range.",record_ID);
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr,
                        "\nERROR: for -u option please provide "
                        "an acronym ID for update.\nUse "
                        "search function to "
                        "locate correct record 'ID' first, as "
                        "the provided "
                        "argument '%s' is not valid.\n",
                        argv[2] ? argv[2] : "NOT SPECIFIED");
                exit(EXIT_FAILURE);
            }
        }

        /* @note VERSION : update an acronym record */
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            display_version(prog_name);
            return (EXIT_SUCCESS);
        }

        /* no matching command lines options - default action to search */
        if (strlen(argv[1]) > 0)  {
            const int rec_match = do_acronym_search(argv[1],&amtdb);
            printf("\nDatabase search found '%'d' matching records\n",
                   rec_match);
            return (EXIT_SUCCESS);

        } else {
            fprintf(stderr, "\nERROR: for '-s' or '--search' option please provide "
                            "an acronym to search for.\n");
            exit(EXIT_FAILURE);
        }

    } else {  /* NO COMMAND LINE ARGS PROVIDED */
            fprintf(stderr,"\nERROR: no command lines argument provided.\n");
            display_version(prog_name);
            show_help();
            exit(EXIT_FAILURE);
    }
    //exit main()
}

void display_version(const char *prog_name) {

    /* Check build flag used when program was compiled */
    #if DEBUG
        char Build_Type[] = "Debug";
    #else
        char Build_Type[] = "Release";
    #endif

    printf("\n'%s' version is: '%s'.\n", prog_name, amt_version);
    printf("Compiled on: '%s @ %s' with C source built as '%s'.\n",__DATE__,__TIME__,Build_Type);
    printf("Complied with SQLite version: %s\n", SQLITE_VERSION);
    puts("Copyright (c) 2021 Simon Rowe.\n");
    puts("For licenses and further information visit:");
    puts("- Application      : https://github.com/wiremoons/acroman/");
    puts("- SQLite database  :  https://www.sqlite.org/\n");

    if ( getenv("NO_COLOR") ) {
        puts("\n'NO_COLOR' environment exist as: https://no-color.org/");
    }

}

void show_help(void) {
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

void exit_cleanup() {
    if (amtdb.db == NULL) {
        printf("\nNo SQLite database shutdown required\n\nAll is well\n");
        exit(EXIT_SUCCESS);
    }

    int rc = sqlite3_close_v2(amtdb.db);
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

    exit(EXIT_SUCCESS);
}