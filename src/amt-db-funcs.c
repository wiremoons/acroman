/**
 * @file amt-db-funcs.c
 * @brief Acronym Management Tool (amt). A program to managed SQLite database containing acronyms.
 * @details Program to managed SQLite database containing acronyms. This source code manages the SQLite database
 * functions for the application.
 * @See https://github.com/wiremoons/acroman
 *
 * @license MIT License
 *
 */

#include "amt-db-funcs.h"


/* added to enable compile on macOS */
#ifndef __clang__
#include <malloc.h> /* free for use with strdup and malloc */
#endif

#include <errno.h>             /* strerror */
#include <libgen.h>            /* basename and dirname */
#include <locale.h>            /* number output formatting with commas */
#include <stdio.h>             /* printf and asprintf */
#include <stdlib.h>            /* getenv */
#include <string.h>            /* strlen strdup */
#include <sys/stat.h>          /* stat */
#include <sys/types.h>         /* stat */
#include <time.h>              /* stat file modification time */
#include <unistd.h>            /* strdup access stat and FILE */
#include "linenoise.h"         /** @note Linenoise library: readline replacement */

/**
 * @brief Get the total records held in the database; store any prior total; write both to 'amtdb' struct.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 * @note Uses the following SQL:
 * @code select count(*) from ACRONYMS;
 */
bool set_record_count(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;

    /* capture any previous record count if it exists */
    if (amtdb->totalrec > 0) {
        amtdb->prevtotalrec = amtdb->totalrec;
    }

    int rc = sqlite3_prepare_v2(amtdb->db, "select count(*) from ACRONYMS", -1, &stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        perror("\nERROR: unable to access the SQLite database to "
               "perform a record count\n");
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        amtdb->totalrec = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return true;
}


/**
 * @brief Get the maximum record id number in use by the database, and write result to 'amtdb' struct.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 * @note Uses the follow SQL:
 * @code select MAX(rowid) from ACRONYMS;
 */
bool update_max_recid(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(amtdb->db, "select MAX(rowid) from ACRONYMS", -1, &stmt,
                                NULL);

    if (rc != SQLITE_OK) {
        perror("\nERROR: unable to access the SQLite database to "
               "obtain the maximum record ID\n");
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        amtdb->maxrecid = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return true;
}


/**
 * @brief Check for the existence of a valid SQLite database file. Record found file and path to the 'amtdb' struct.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 */
bool check_4_db_file(amtdb_struct *amtdb)
{
    /**
     * @note Checks for a valid database filename to open looking at:
     *   1 : environment variable 'ACRODB'
     *   2 : file 'acronyms.db' in same location as the application
     *   3 : TODO offer to create a new Database
     */

    /** @note get database file from environment variable ACRODB first */
    amtdb->dbfile = getenv("ACRODB");
    /** @note if the environment variable exists - check if its valid */
    if (amtdb->dbfile != NULL) {
        if (check_db_access(amtdb)){
            return true;
        }
    } else {
        fprintf(stderr,"ERROR: No database identified via environment variable 'ACRODB'.\n");
        printf("Checking for a suitable database in same directory as the executable...\n");
    }

    /**
     * @note nothing is set in environment variable ACRODB - so database
     * might be found in the application directory instead. Database named
     * by default as filename: 'acronyms.db'
     * tmp copy needed here as each call to dirname() below can change the
     * string being used in the call - so need one string copy for each
     * successful call we need to make. This is a 'feature' of dirname()
     */
    char *tmpDirname = strndup(amtdb->prog_name, strlen(amtdb->prog_name));
    size_t newDbfileSz = (sizeof(char) * (strlen(dirname(tmpDirname)) +
                                          strlen("/acronyms.db") + 1));
    char *newDbfile = malloc(newDbfileSz);

    if (newDbfile == NULL) {
        perror("\nERROR: unable to allocate memory with malloc() for 'newDbfile' and path\n");
        return false;
    }

    int x = snprintf(newDbfile, newDbfileSz, "%s%s", dirname((char *)amtdb->prog_name),
                     "/acronyms.db");

    if (x == -1) {
        perror("\nERROR: unable to allocate memory with snprintf() for 'newDbfile' and path\n");
        return false;
    }

    /**
     * @note 'newDbfile' is ready as it contains the path and the
     * default db filename. To use it, now copy it our global var
     * 'dbfile' ; then get rid of newDbfile as it is done with.
     */
    if ((amtdb->dbfile = strdup(newDbfile)) == NULL) {
        perror("\nERROR: unable to allocate memory with strdup() for 'newDbfile'\n");
        return false;
    }

    /* debug code
       printf("\nnewDbfile: '%s' and dbfile: '%s'\n", newDbfile, dbfile);
    */

    free(newDbfile);

    /* now recheck if the newDbfile is suitable for use? */
    if (check_db_access(amtdb)) {
        return true;
    }

    /* run out of options to find a suitable database - exit */
    /* TODO : offer to create a new dbfile here! */
    puts("TODO : offer to create a new database here\n");
    return false;
}


/**
 * @brief Check the valid SQLite database file. Record the database file stats into the 'amtdb' struct.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 */
bool check_db_access(amtdb_struct *amtdb)
{
    if (amtdb->dbfile == NULL || strlen(amtdb->dbfile) == 0) {
        fprintf(stderr,
                "ERROR: The database file '%s' is an empty string.\n",amtdb->dbfile);
        return false;
    }

    if (access(amtdb->dbfile, F_OK | R_OK) == -1) {
        fprintf(stderr,
                "ERROR: The database file '%s' is missing or is not accessible.\n",amtdb->dbfile);
        return false;
    }

    struct stat sb;
    int check;

    check = stat(amtdb->dbfile, &sb);

    if (check) {
        perror("ERROR: call to 'stat' for database file failed.\n");
        return false;
    }

    /** @note get the size of the database file */
    amtdb->dbsize = sb.st_size;

    /** @note get the database file modification date */
    char tempDate[100];
    size_t dateLength = strftime(tempDate, 100, "%a %d %b %Y @ %H:%M:%S", localtime(&sb.st_mtime));
    if (dateLength < 1 ) {
        fprintf(stderr,"ERROR: file modification date 'strftime' conversion failure\n");
        return false;
    }
    amtdb->dblastmod = strndup(tempDate, dateLength);
    if (amtdb->dblastmod == NULL) {
        perror("ERROR: Failed to copy database modification time into 'amdb->dblastmod' structure field.\n");
        return false;
    }

    return true;
}

/**
 * @brief Output the database stats as stored in the 'amtdb' struct populated by 'check_db_access()'.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 */
bool output_db_stats(amtdb_struct *amtdb)
{
    if ( strlen(amtdb->dbfile) <= 0 && strlen(amtdb->dblastmod) <= 0 && amtdb->dbsize < 0  && amtdb->totalrec < 0) {
        fprintf(stderr,
                "ERROR: The database status information for file '%s' is missing\n",
                amtdb->dbfile);
        return false;
    }

    printf("Database full path:   '%s'\n", amtdb->dbfile);
    printf("Database file size:   '%'lld' bytes\n", amtdb->dbsize);
    printf("Database modified:    '%s'\n\n", amtdb->dblastmod);
    printf("SQLite version:       '%s'\n", SQLITE_VERSION);
    printf("Total acronyms:       '%'d'\n", amtdb->totalrec);
    printf("Last acronym entered: '%s'\n", get_last_acronym(amtdb));

    return true;
}


/**
 * @brief Ensure the database is opened and working correctly. Get initial record counts and max record ID.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 */
bool initialise_database(amtdb_struct *amtdb) {

    int rc = sqlite3_initialize();
    if (rc != SQLITE_OK) {
        fprintf(stderr,"ERROR: 'sqlite3_initialize()' failed with: '%d'.",rc);
        return false;
    }

    rc = sqlite3_open_v2(amtdb->dbfile, &amtdb->db,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr,"ERROR: attempt to open database failed with: '%d'.",rc);
        return false;
    }

    if (!update_max_recid(amtdb)) {
        fprintf(stderr,
                "ERROR: Failed to obtain the database maximum record ID.\n");
        return false;
    }

    if (!set_record_count(amtdb)) {
        fprintf(stderr,
                "ERROR: Failed to set the database record count.\n");
        return false;
    }

    return true;
}


/**
 * @brief Get the last acronym entered into the database.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return char* : a pointer to a heap allocated string containing the last acronym entered.
 * @note Uses the following SQL:
 * @code SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1;
 */
char *get_last_acronym(amtdb_struct *amtdb)
{
    char *acronymName;
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(
        amtdb->db, "SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1;", -1,
        &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(-1);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        acronymName = strdup((const char *)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);

    if (acronymName == NULL) {
        fprintf(stderr, "ERROR: last acronym lookup return NULL\n");
    }

    return (acronymName);
}


/**
 * @brief Display the five latest acronym records in the database.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : true if records found.
 * @note Uses the following SQL:
 * @code select rowid,ifnull(Acronym,''), ifnull(Definition,''), ifnull(Source,''), ifnull(Description,'')
 * from ACRONYMS Order by rowid DESC LIMIT 5;
 */
bool latest_acronym(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;   	    /* pre-prepared SQL query statement */
    bool result = false;

    int rc = sqlite3_prepare_v2(amtdb->db,
                                "select rowid,ifnull(Acronym,''), "
                                "ifnull(Definition,''), "
                                "ifnull(Source,''), "
                                "ifnull(Description,'') "
                                "from ACRONYMS "
                                "Order by rowid DESC LIMIT 5;",
                                -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        result = false;
        exit(EXIT_FAILURE);
    }

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        result = false;
        exit(EXIT_FAILURE);
    }

    int searchRecCount = 0;
    printf("\nFive newest acronym records added are:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\nID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: '%s'.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("SOURCE:      '%s'\n",
               (const char *)sqlite3_column_text(stmt, 3));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        searchRecCount++;
    }

    sqlite3_finalize(stmt);

    return result = true;
}


/**
 * @brief Search for the provided acronym in the database and return the matching number of records found.
 * @param char *findme : Pointer to a string containing the acronym to be searched for.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return int : the number of matching acronyms found in the database.
 * @note Uses the following SQL:
 * @code select rowid,ifnull(Acronym,''), ifnull(Definition,''), ifnull(Source,''), ifnull(Description,'')
 * from ACRONYMS where Acronym like ? COLLATE NOCASE ORDER BY Source;
 */
int do_acronym_search(char *findme, amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;   	    /* pre-prepared SQL query statement */

    int rc = sqlite3_prepare_v2(amtdb->db,
                            "select rowid,ifnull(Acronym,''), "
                            "ifnull(Definition,''), "
                            "ifnull(Source,''), "
                            "ifnull(Description,'') "
                            "from ACRONYMS where Acronym like ? "
                            "COLLATE NOCASE ORDER BY Source;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_text(stmt, 1, (const char *)findme, -1, SQLITE_STATIC);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    int searchRecCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\nID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: '%s'.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("SOURCE:      '%s'\n",
               (const char *)sqlite3_column_text(stmt, 3));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        searchRecCount++;
    }

    sqlite3_finalize(stmt);

    return searchRecCount;
}

/**
 * @brief Ensure sane base setting for linenoise prior to is usse in the 'delete'; 'update'; and 'new' functions.
 * @param none
 * @return none
 */
void linenoise_initialise(void) {
    /** @note enable or disable multiline editing feature */
    linenoiseSetMultiLine(1);
    /** @note max size of the number if items to hold in the linenoise history that
     * must be greater than '0' to enable in linenoise library.
     */
    linenoiseHistorySetMaxLen(20);
}


/**
 * @brief Add a new acronym record using input provided by the user when they are prompted on screen for inputs.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 * @note Uses the following SQL:
 * @code insert into ACRONYMS(Acronym,Definition,Description,Source) values(?,?,?,?);
 */
bool new_acronym(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_record_count(amtdb);

    linenoise_initialise();

    printf("\nAdding a new record...\n");
    printf("\nNote: To abort the input of a new record - press 'Ctrl + c'\n\n");

    char *complete = NULL;
    char *nAcro = NULL;
    char *nAcroExpd = NULL;
    char *nAcroDesc = NULL;
    char *nAcroSrc = NULL;

    while (1) {
        nAcro = linenoise("Enter the acronym: ");
        linenoiseHistoryAdd(nAcro);
        nAcroExpd = linenoise("Enter the expanded acronym: ");
        linenoiseHistoryAdd(nAcroExpd);
        puts("Enter the acronym description:\n");
        nAcroDesc = linenoise("");
        linenoiseHistoryAdd(nAcroDesc);
        puts("\n");
        get_acronym_src_list(amtdb);
        nAcroSrc = linenoise("Enter the acronym source: ");
        linenoiseHistoryAdd(nAcroSrc);

        printf("\nConfirm entry for:\n\n");
        printf("ACRONYM:     '%s' is: %s.\n", nAcro, nAcroExpd);
        printf("DESCRIPTION: %s\n", nAcroDesc);
        printf("SOURCE:      %s\n\n", nAcroSrc);

        complete = linenoise("Enter record? [ y/n or q ] : ");
        if (strcasecmp((const char *)complete, "y") == 0) {
            break;
        }
        if (strcasecmp((const char *)complete, "q") == 0) {
            /* Clean up linenoise allocated memory */
            if (complete != NULL) {
                free(complete);
            }
            if (nAcro != NULL) {
                free(nAcro);
            }
            if (nAcroExpd != NULL) {
                free(nAcroExpd);
            }
            if (nAcroDesc != NULL) {
                free(nAcroDesc);
            }
            if (nAcroSrc != NULL) {
                free(nAcroSrc);
            }
            // TODO: linenoise for below ??
            // clear_history();
            return false;
        }
    }

    char *sqlInsNew = NULL;
    sqlInsNew = sqlite3_mprintf("insert into ACRONYMS"
                              "(Acronym, Definition, Description, Source) "
                              "values(%Q,%Q,%Q,%Q);",
                                nAcro, nAcroExpd, nAcroDesc, nAcroSrc);

    int rc = sqlite3_prepare_v2(amtdb->db, sqlInsNew, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        /* Clean up linenoiseallocated memory */
        if (complete != NULL) {
            free(complete);
        }
        if (nAcro != NULL) {
            free(nAcro);
        }
        if (nAcroExpd != NULL) {
            free(nAcroExpd);
        }
        if (nAcroDesc != NULL) {
            free(nAcroDesc);
        }
        if (nAcroSrc != NULL) {
            free(nAcroSrc);
        }
        // TODO: linenoise for below ??
        // clear_history();
        return false;
    }

    rc = sqlite3_exec(amtdb->db, sqlInsNew, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(amtdb->db));
        /* Clean up linenoiseallocated memory */
        if (complete != NULL) {
            free(complete);
        }
        if (nAcro != NULL) {
            free(nAcro);
        }
        if (nAcroExpd != NULL) {
            free(nAcroExpd);
        }
        if (nAcroDesc != NULL) {
            free(nAcroDesc);
        }
        if (nAcroSrc != NULL) {
            free(nAcroSrc);
        }
        // TODO: linenoise for below ??
        // clear_history();
        return false;
    }

    sqlite3_finalize(stmt);

    /* free up any allocated memory by sqlite3 */
    if (sqlInsNew != NULL) {
        sqlite3_free(sqlInsNew);
    }

    /* Clean up linenoiseallocated memory */
    if (complete != NULL) {
        free(complete);
    }
    if (nAcro != NULL) {
        free(nAcro);
    }
    if (nAcroExpd != NULL) {
        free(nAcroExpd);
    }
    if (nAcroDesc != NULL) {
        free(nAcroDesc);
    }
    if (nAcroSrc != NULL) {
        free(nAcroSrc);
    }
    // TODO: linenoise for below ??
    // clear_history();

    set_record_count(amtdb);
    printf("Inserted '%d' new record. Total database record count "
           "is now"
           " %'d (was %'d).\n",
           (amtdb->totalrec - amtdb->prevtotalrec), amtdb->totalrec, amtdb->prevtotalrec);

    return true;
}


/**
 * @brief Delete an users provided acronym record from the SQLite database.
 * @param int delRecId : the record ID provided by the user to be deleted from the database.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 * @note Uses the following SQL to delete the record:
 * @code delete from ACRONYMS where rowid = ?;
 */
bool delete_acronym_record(int delRecId, amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_record_count(amtdb);

    linenoise_initialise();

    printf("\nDeleting an acronym record...\n");
    printf("\nNote: To abort the delete of a record - press 'Ctrl "
           "+ c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", delRecId);

    int rc = sqlite3_prepare_v2(amtdb->db,
                            "select rowid,Acronym,Definition,Description,"
                            "Source from ACRONYMS where rowid like ?;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 1, delRecId);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    int deleteRecCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: %s.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 3));
        printf("SOURCE: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        deleteRecCount++;
    }

    sqlite3_finalize(stmt);

    if (deleteRecCount == 1) {
        char *continueDelete = NULL;
        puts("");
        continueDelete = linenoise("Delete above record? [ y/n ] : ");
        if (strcasecmp((const char *)continueDelete, "y") == 0) {

            /* free 'linenoise memory as no longer used */
            if (continueDelete != NULL) {
                free(continueDelete);
            }

            rc = sqlite3_prepare_v2(amtdb->db,
                                    "delete from ACRONYMS where "
                                    "rowid = ?;",
                                    -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
                return false;
            }

            rc = sqlite3_bind_int(stmt, 1, delRecId);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
                return false;
            }

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(amtdb->db));
                return false;
            }

           sqlite3_finalize(stmt);
        } else {
            /* free 'linenoise memory as no longer used */
            if (continueDelete != NULL) {
                free(continueDelete);
            }
            printf("\nRequest to delete record ID '%d' was abandoned "
                   "by the user\n\n",
                   delRecId);
        }
    } else if (deleteRecCount > 1) {
        printf(" » ERROR: record ID '%d' search returned '%d' records "
               "«\n\n",
               delRecId, deleteRecCount);
    } else {
        printf(" » WARNING: record ID '%d' found '%d' matching "
               "records «\n\n",
               delRecId, deleteRecCount);
    }

    set_record_count(amtdb);
    printf("Deleted '%d' record. Total database record count is now"
           " %'d (was %'d).\n",
           (amtdb->prevtotalrec - amtdb->totalrec), amtdb->totalrec, amtdb->prevtotalrec);

    return true;
}


/**
 * @brief Gets a list of all the 'source' entries from the SQLite database, and adds them to the linenoisehistory.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @note Uses the following SQL to delete the record:
 * @code select distinct(source) from acronyms;
 */
void get_acronym_src_list(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(amtdb->db,
                            "select distinct(source) "
                            "from acronyms order by source;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        exit(-1);
    }

    char *acroSrcName;

    printf("\nSelect a source (use ↑ or ↓ ):\n\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        acroSrcName = strdup((const char *)sqlite3_column_text(stmt, 0));
        printf("[ %s ] ", acroSrcName);
        linenoiseHistoryAdd(acroSrcName);

        /* free per loop to stop memory leaks - strdup malloc
         * above */
        if (acroSrcName != NULL) {
            free(acroSrcName);
        }
    }
    printf("\n");

    sqlite3_finalize(stmt);
}


/**
 * @brief Updates a acronym record stored in the SQLite database based on the user provided record ID .
 * @param int updateRecId : the record ID provided by the user to be updated in the database.
 * @param amtdb_struct *amtdb : Pointer to the structure to manage the apps SQLite database information.
 * @return bool : success status for functions execution.
 * @note Uses the following SQL to delete the record:
 * @code select rowid,ifnull(Acronym,''), ifnull(Definition,''), fnull(Description,''), ifnull(Source,'')
 * from ACRONYMS where rowid = ?;
 */
bool update_acronym_record(int updateRecId, amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_record_count(amtdb);

    linenoise_initialise();

    printf("\nUpdating an acronym record...\n");
    printf("\nNote: To abort the update of a record - press 'Ctrl + c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", updateRecId);

    int rc = sqlite3_prepare_v2(amtdb->db,
                           "select rowid, ifnull(Acronym,''),"
                           " ifnull(Definition,''), ifnull(Description,''),"
                           " ifnull(Source,'') from ACRONYMS where rowid is ?;",
                           -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        return false;
    }

    rc = sqlite3_bind_int(stmt, 1, updateRecId);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        return false;
    }

    int updateRecCount = 0;
    while ( sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: %s.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));

        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 3));
        printf("SOURCE: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        /* grab a copy of the returned record id fields into Readline
         * history - for user recall later to save re-typing entries */
        linenoiseHistoryAdd((const char *)sqlite3_column_text(stmt, 1));
        linenoiseHistoryAdd((const char *)sqlite3_column_text(stmt, 2));
        linenoiseHistoryAdd((const char *)sqlite3_column_text(stmt, 3));
        linenoiseHistoryAdd((const char *)sqlite3_column_text(stmt, 4));
        updateRecCount++;
    }

    sqlite3_finalize(stmt);

    /* if we found a record to update */
    if (updateRecCount == 1) {
        char *continueDelete = NULL;
        puts("");
        continueDelete = linenoise("Update above record? [ y/n ] : ");
        if (strcasecmp((const char *)continueDelete, "y") == 0) {

            /* free 'linenoise memory as no longer used */
            if (continueDelete != NULL) {
                free(continueDelete);
            }

            char *complete = NULL;
            char *uAcro = NULL;
            char *uAcroExpd = NULL;
            char *uAcroDesc = NULL;
            char *uAcroSrc = NULL;

            printf("\nUse ↑ or ↓ keys to select previous entries text "
                   "for re-editing or just type in new:\n\n");

            while (1) {
                uAcro = linenoise("Enter the acronym: ");
                linenoiseHistoryAdd(uAcro);
                uAcroExpd = linenoise("Enter the expanded acronym: ");
                linenoiseHistoryAdd(uAcroExpd);
                puts("Enter the acronym description:\n");
                uAcroDesc = linenoise("");
                linenoiseHistoryAdd(uAcroDesc);
                get_acronym_src_list(amtdb);
                puts("");
                uAcroSrc = linenoise("Enter the acronym source: ");
                linenoiseHistoryAdd(uAcroSrc);

                printf("\nConfirm entry for:\n\n");
                printf("ACRONYM:     '%s' is: %s.\n", uAcro, uAcroExpd);
                printf("DESCRIPTION: %s\n", uAcroDesc);
                printf("SOURCE:      %s\n\n", uAcroSrc);

                complete = linenoise("Enter record? [ y/n or q ] : ");
                if (strcasecmp((const char *)complete, "y") == 0) {
                    /* Clean up linenoiseallocated memory */
                    if (complete != NULL) {
                        free(complete);
                    }
                    break;
                }
                if (strcasecmp((const char *)complete, "q") == 0) {
                    /* Clean up linenoiseallocated memory */
                    if (complete != NULL) {
                        free(complete);
                    }
                    if (uAcro != NULL) {
                        free(uAcro);
                    }
                    if (uAcroExpd != NULL) {
                        free(uAcroExpd);
                    }
                    if (uAcroDesc != NULL) {
                        free(uAcroDesc);
                    }
                    if (uAcroSrc != NULL) {
                        free(uAcroSrc);
                    }
                    // TODO: linenoise for below ??
                    // clear_history();
                    return false;
                }
            }

            char *sqlUpdate = NULL;

            /* build SQLite 'UPDATE' query */
            sqlUpdate =
                sqlite3_mprintf("update ACRONYMS set Acronym=%Q, "
                                "Definition=%Q, Description=%Q, "
                                "Source=%Q where rowid is ?;",
                                uAcro, uAcroExpd, uAcroDesc, uAcroSrc);

            rc = sqlite3_prepare_v2(amtdb->db, sqlUpdate, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
                if (uAcro != NULL) {
                    free(uAcro);
                }
                if (uAcroExpd != NULL) {
                    free(uAcroExpd);
                }
                if (uAcroDesc != NULL) {
                    free(uAcroDesc);
                }
                if (uAcroSrc != NULL) {
                    free(uAcroSrc);
                }
                // TODO: linenoise for below ??
                // clear_history();
                return false;
            }

            /* bind in the record id to UPDATE */
            rc = sqlite3_bind_int(stmt, 1, updateRecId);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(amtdb->db));
                 if (uAcro != NULL) {
                    free(uAcro);
                }
                if (uAcroExpd != NULL) {
                    free(uAcroExpd);
                }
                if (uAcroDesc != NULL) {
                    free(uAcroDesc);
                }
                if (uAcroSrc != NULL) {
                    free(uAcroSrc);
                }
                // TODO: linenoise for below ??
                // clear_history();
                return false;
            }

            /* reset updateRecCount so can re-use here */
            updateRecCount = 0;

            /* perform the actual database update */
            while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                /* should not run here as 'sqlite3_step(stmt)'
                   should
                   immediately return with SQLITE_DONE for an
                   SQL UPDATE */
                updateRecCount++;
            }

            if (rc != SQLITE_DONE) {
                fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(amtdb->db));
                /* Clean up linenoiseallocated memory */
                if (uAcro != NULL) {
                    free(uAcro);
                }
                if (uAcroExpd != NULL) {
                    free(uAcroExpd);
                }
                if (uAcroDesc != NULL) {
                    free(uAcroDesc);
                }
                if (uAcroSrc != NULL) {
                    free(uAcroSrc);
                }
                // TODO: linenoise for below ??
                // clear_history();
                return false;
            }

            /* capture number of Sqlite database changes made in
               last transaction - should be one */
            int dbChanges = sqlite3_changes(amtdb->db);

            /* check for how many records were updated - should only
             * be one! */
            if (updateRecCount > 0 || dbChanges > 1) {
                fprintf(stderr,"\n\nWARNING: Database changes made were: "
                       "'%d' but actual record changes made by "
                       "sqlite3_step() "
                       "were: '%d'\n",
                       dbChanges, updateRecCount);
                fprintf(stderr,"Only record id: '%d' should of been "
                       "changed - "
                       "but '%d' changes were made "
                       "unexpectedly.\n\n",
                       updateRecId, updateRecCount);
            }

            sqlite3_finalize(stmt);

            /* free up any allocated memory by sqlite3 */
            if (sqlUpdate != NULL) {
                sqlite3_free(sqlUpdate);
            }

            if (uAcro != NULL) {
                free(uAcro);
            }
            if (uAcroExpd != NULL) {
                free(uAcroExpd);
            }
            if (uAcroDesc != NULL) {
                free(uAcroDesc);
            }
            if (uAcroSrc != NULL) {
                free(uAcroSrc);
            }
            // TODO: linenoise for below ??
            // clear_history();

            set_record_count(amtdb);
            printf("Updated '%d' record. Total database record count "
                   "is now"
                   " %'d (was %'d).\n",
                   amtdb->totalrec, amtdb->totalrec, amtdb->prevtotalrec);
        } else {

            /* free 'linenoise' memory as no longer used */
            if (continueDelete != NULL) {
                free(continueDelete);
            }

            printf("\nRequest to update record ID '%d' was abandoned "
                   "by the user\n\n",
                   updateRecId);

            return false;
        }

    }
    return true;
}
