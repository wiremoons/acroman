/* Acronym Management Tool (amt): main.c */

#include "main.h"

/* added to enable compile on MacOSX */
#ifndef __clang__
# include <malloc.h>    /* free for use with strdup */
#endif

#include <locale.h>     /* number output formatting with commas */
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* getenv */
#include <string.h>     /* strlen strndup */

int main(int argc, char **argv) {

    atexit(exit_cleanup);
    setlocale(LC_NUMERIC, "");
    /* Set the default locale values according to environment variables. */
    // setlocale (LC_ALL, "");

    /* ensure known state */
    amtdb.db_OK = false;

    #if DEBUG
    fprintf(stderr, "DEBUG: the programs was built in 'debug' mode\n");
    #endif


    if ((amtdb.prog_name = strndup(argv[0], strlen(argv[0]))) == NULL) {
        perror("\nERROR: unable to set program name ");
    }

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
                if (!bootstrap_db()) { return (EXIT_FAILURE); }
                const int rec_match = do_acronym_search(argv[2],&amtdb);
                printf("\nSearch for: '%s' found '%d' records\n\n", argv[2], rec_match);
                return (EXIT_SUCCESS);

            } else {
                fprintf(stderr, "\nERROR: for '-s' or '--search' option please provide "
                                "an acronym to search for.\n");
                exit(EXIT_FAILURE);
            }
        }

        /* @note NEW : add a new acronym via user prompts */
        if (strcmp(argv[1], "-n") == 0 || strcmp(argv[1], "--new") == 0) {
             if (!bootstrap_db()) { return (EXIT_FAILURE); }
             if (new_acronym(&amtdb)) {
                printf("\nADD DONE\n");
                return (EXIT_SUCCESS);
            } else {
                fprintf(stderr, "ERROR: failed to complete adding new record.\n");
                exit(EXIT_FAILURE);
            }
        }

        /* @note DELETE : delete an acronym record */
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--delete") == 0) {
            if ( argc > 2 ) {
                if (!bootstrap_db()) { return (EXIT_FAILURE); }
                long record_ID =  strtol(argv[2], NULL, 10);
                #if DEBUG
                fprintf(stderr, "DEBUG: parsed Record ID is: '%ld'\n",record_ID);
                #endif
                if ( record_ID > 0 && record_ID <= amtdb.maxrecid) {
                    if (del_acro_rec((int)record_ID, &amtdb)) {
                        printf("\nDELETE DONE\n");
                        return (EXIT_SUCCESS);
                    } else {
                        fprintf(stderr, "ERROR: failed to complete deleting the record.\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    fprintf(stderr, "ERROR: invalid record ID '%ld' - out of range.\n",record_ID);
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
                if (!bootstrap_db()) { return (EXIT_FAILURE); }
                long record_ID = strtol(argv[2], NULL, 10);
                #if DEBUG
                fprintf(stderr, "DEBUG: parsed Record ID is: '%ld'\n",record_ID);
                #endif
                if (record_ID > 0 && record_ID <= amtdb.maxrecid) {
                    if (update_acro_rec((int)record_ID, &amtdb)) {
                        printf("\nUPDATE DONE\n");
                        return (EXIT_SUCCESS);
                    } else {
                        fprintf(stderr, "ERROR: failed to complete updating the record.\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    fprintf(stderr, "ERROR: invalid record ID '%ld' - out of range.\n",record_ID);
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
            display_version();
            return (EXIT_SUCCESS);
        }

        /* no matching command lines options - default action to search */
        if (strlen(argv[1]) > 0)  {
            if (!bootstrap_db()) { return (EXIT_FAILURE); }
            const int rec_match = do_acronym_search(argv[1],&amtdb);
            printf("\nSearch for: '%s' found '%d' records\n\n", argv[1], rec_match);
            return (EXIT_SUCCESS);
        } else {
            fprintf(stderr, "\nERROR: for '-s' or '--search' option please provide "
                            "an acronym to search for.\n");
            exit(EXIT_FAILURE);
        }

    } else {  /* NO COMMAND LINE ARGS PROVIDED */
            fprintf(stderr,"\nERROR: no command lines argument provided.\n");
            display_version();
            if (bootstrap_db()) { output_db_stats(&amtdb); }
            show_help();
            exit(EXIT_FAILURE);
    }
    //exit main()
}


bool bootstrap_db(void) {

    /* check for a valid database file */
    if (!check4DBFile(&amtdb)) {
        fprintf(stderr,"\nERROR: No suitable database file can be located. Program will exit.\n");
        return false;
    }

    /* connect and obtain initial database information */
    if (!initialise_database(&amtdb)) {
        fprintf(stderr,"\n\tERROR: database initialisation failed. Program will exit\n");
        return false;
    }

    /* set flag to show DB connection is good now */
    amtdb.db_OK = true;
    return true;

}


void display_version(void) {

    /* Check build flag used when program was compiled */
    #if DEBUG
        char Build_Type[] = "Debug";
    #else
        char Build_Type[] = "Release";
    #endif

    printf("\n'%s' version is: '%s'.\n", amtdb.prog_name, amt_version);
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
           "  -u ?  Update : update an acronym where ? == ID of record to update\n\n");
}

void exit_cleanup(void) {
    if (amtdb.db == NULL) {
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
    amtdb.db_OK = false;
    exit(EXIT_SUCCESS);
}