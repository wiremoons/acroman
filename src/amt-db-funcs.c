/* Acronym Management Tool (amt)
 *
 * amt-db-funcs.c : managed SQLite database functions
 */

#include "amt-db-funcs.h"

/* added to enable compile on MacOSX */
#ifndef __clang__
#include <malloc.h> /* free for use with strdup and malloc */
#endif

#include <errno.h>             /* strerror */
#include <libgen.h>            /* basename and dirname */
#include <locale.h>            /* number output formatting with commas */
#include <stdio.h>             /* printf and asprintf */
#include <readline/readline.h> /* readline support for text entry */
#include <readline/history.h>  /* readline history support */
#include <stdlib.h>            /* getenv */
#include <string.h>            /* strlen strdup */
#include <sys/stat.h>          /* stat */
#include <sys/types.h>         /* stat */
#include <time.h>              /* stat file modification time */
#include <unistd.h>            /* strdup access stat and FILE */



/***********************************************************************/
/* Run SQL query to obtain current number of acronyms in the database. */
/***********************************************************************/
bool set_rec_count(amtdb_struct *amtdb)
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

/***********************************************************************/
/* Run SQL query to obtain highest record ID number in the database. */
/***********************************************************************/
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


/****************************************************************/
/* Checks for a valid database filename to open looking at:     */
/* - 1 : environment variable 'ACRODB'                          */
/* - 2 : file 'acronyms.db' in same location as the application */
/* - 3 : TODO offer to create a new Database                    */
/****************************************************************/
bool check4DBFile(amtdb_struct *amtdb)
{

    /* get database file from environment variable ACRODB first */
    amtdb->dbfile = getenv("ACRODB");
    /* if the environment variable exists - check if its valid */
    if (amtdb->dbfile != NULL) {
        if (check_db_access(amtdb)){
            return true;
        }
    } else {
        fprintf(stderr,"ERROR: No database identified via environment variable 'ACRODB'.\n");
        printf("Checking for a suitable database in same directory as the executable...\n");
    }

    /* nothing is set in environment variable ACRODB - so database 			*/
    /* might be found in the application directory instead, named 				*/
    /* as the default filename: 'acronyms.db' 									*/
    /* tmp copy needed here as each call to dirname() below can change the	*/
    /* string being used in the call - so need one string copy for each 		*/
    /* successful call we need to make. This is a 'feature' of dirname() 		*/
    char *tmp_dirname = strndup(amtdb->prog_name, strlen(amtdb->prog_name));
    size_t new_dbfile_sz = (sizeof(char) * (strlen(dirname(tmp_dirname)) +
                                            strlen("/acronyms.db") + 1));
    char *new_dbfile = malloc(new_dbfile_sz);

    if (new_dbfile == NULL) {
        perror("\nERROR: unable to allocate memory with malloc() for 'new_dbfile' and path\n");
        return false;
    }

    int x = snprintf(new_dbfile, new_dbfile_sz, "%s%s", dirname((char *)amtdb->prog_name),
                     "/acronyms.db");

    if (x == -1) {
        perror("\nERROR: unable to allocate memory with snprintf() for 'new_dbfile' and path\n");
        return false;
    }

    /* 'new_dbfile' is ready as it contains the path and the 			*/
    /* default db filename. To use it, now copy it our global var 		*/
    /* 'dbfile' ; then get rid of new_dbfile as it is done with 		*/
    if ((amtdb->dbfile = strdup(new_dbfile)) == NULL) {
        perror("\nERROR: unable to allocate memory with strdup() for 'new_dbfile'\n");
        return false;
    }

    /* debug code
       printf("\nnew_dbfile: '%s' and dbfile: '%s'\n", new_dbfile, dbfile);
    */

    free(new_dbfile);

    /* now recheck if the new_dbfile is suitable for use? */
    if (check_db_access(amtdb)) {
        return true;
    }

    /* run out of options to find a suitable database - exit */
    /* TODO : offer to create a new dbfile here! */
    puts("TODO : offer to create a new database here\n");
    return false;
}

/**********************************************************************/
/* Check the filename and path given for the acronym database and see */
/* if it is accessible. If the file is located then print some stats  */
/* on path, size and last modified time.                              */
/**********************************************************************/
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

    /* get the size of the database file */
    amtdb->dbsize = sb.st_size;
    /* ctime() returns pointer to a 26 character string */
    amtdb->dblastmod = strndup(ctime(&sb.st_mtime),26);
    if (amtdb->dblastmod == NULL) {
        perror("ERROR: Failed to copy database modification time.\n");
        return false;
    }

    return true;
}

/**********************************************************************/
/* Output the summary stats for the SQLite acronyms database          */
/**********************************************************************/
bool output_db_stats(amtdb_struct *amtdb)
{
    if ( strlen(amtdb->dbfile) <= 0 && strlen(amtdb->dblastmod) <= 0 && amtdb->dbsize < 0  && amtdb->totalrec < 0) {
        fprintf(stderr,
                "ERROR: The database status information for file '%s' is missing\n",
                amtdb->dbfile);
        return false;
    }

    printf("Database Summary:\n");
    printf(" - Database location: %s\n", amtdb->dbfile);
    printf(" - Database size: %'lld bytes\n", amtdb->dbsize);
    printf(" - Database last modified: %s", amtdb->dblastmod);
    printf(" - Database total record count: %d\n", amtdb->totalrec);
    printf(" - Newest acronym is: %s\n", get_last_acronym(amtdb));

    return true;
}


/**********************************************************************/
/* Check the filename and path given for the acronym database and see */
/* if it is accessible. If the file is located then print some stats  */
/* on path, size and last modified time.                              */
/**********************************************************************/
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

    if (!set_rec_count(amtdb)) {
        fprintf(stderr,
                "ERROR: Failed to set the database record count.\n");
        return false;
    }

    return true;
}

/*************************************************************/
/* GET NAME OF LAST ACRONYM ENTERED                          */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯                          */
/* SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1; */
/*************************************************************/
char *get_last_acronym(amtdb_struct *amtdb)
{
    char *acronym_name;
    //const char *data = NULL;     	    /* data returned from SQL stmt run */
    sqlite3_stmt *stmt = NULL;   	    /* pre-prepared SQL query statement */

    int rc = sqlite3_prepare_v2(
        amtdb->db, "SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1;", -1,
        &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(-1);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        acronym_name = strdup((const char *)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);

    if (acronym_name == NULL) {
        fprintf(stderr, "ERROR: last acronym lookup return NULL\n");
    }

    return (acronym_name);
}

/********************************************************/
/* SEARCH FOR A NEW RECORD                              */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯                              */
/* "Select rowid, ifnull(Acronym,''), "                 */
/* "ifnull(Definition,''), "                            */
/* "ifnull(Source,''), "                                */
/* "ifnull(Description,'') "                            */
/* "from Acronyms where Acronym like ?1 COLLATE "       */
/* " NOCASE ORDER BY Source";                           */
/********************************************************/

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

    int search_rec_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\nID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: '%s'.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("SOURCE:      '%s'\n",
               (const char *)sqlite3_column_text(stmt, 3));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        search_rec_count++;
    }

    sqlite3_finalize(stmt);

    return search_rec_count;
}

/***************************************************************/
/* ADDING A NEW RECORD                                         */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯                                         */
/* insert into ACRONYMS(Acronym,Definition,Description,Source) */
/* values(?,?,?,?);                                            */
/***************************************************************/
bool new_acronym(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_rec_count(amtdb);

    printf("\nAdding a new record...\n");
    printf("\nNote: To abort the input of a new record - press 'Ctrl + c'\n\n");

    char *complete = NULL;
    char *n_acro = NULL;
    char *n_acro_expd = NULL;
    char *n_acro_desc = NULL;
    char *n_acro_src = NULL;

    while (1) {
        n_acro = readline("Enter the acronym: ");
        add_history(n_acro);
        n_acro_expd = readline("Enter the expanded acronym: ");
        add_history(n_acro_expd);
        n_acro_desc = readline("Enter the acronym description: \n\n");
        add_history(n_acro_desc);

        get_acro_src(amtdb);
        n_acro_src = readline("\nEnter the acronym source: ");
        add_history(n_acro_src);

        printf("\nConfirm entry for:\n\n");
        printf("ACRONYM:     '%s' is: %s.\n", n_acro, n_acro_expd);
        printf("DESCRIPTION: %s\n", n_acro_desc);
        printf("SOURCE:      %s\n\n", n_acro_src);

        complete = readline("Enter record? [ y/n or q ] : ");
        if (strcasecmp((const char *)complete, "y") == 0) {
            break;
        }
        if (strcasecmp((const char *)complete, "q") == 0) {
            /* Clean up readline allocated memory */
            if (complete != NULL) {
                free(complete);
            }
            if (n_acro != NULL) {
                free(n_acro);
            }
            if (n_acro_expd != NULL) {
                free(n_acro_expd);
            }
            if (n_acro_desc != NULL) {
                free(n_acro_desc);
            }
            if (n_acro_src != NULL) {
                free(n_acro_src);
            }
            clear_history();
            return false;
        }
    }

    char *sql_ins = NULL;
    sql_ins = sqlite3_mprintf("insert into ACRONYMS"
                              "(Acronym, Definition, Description, Source) "
                              "values(%Q,%Q,%Q,%Q);",
                              n_acro, n_acro_expd, n_acro_desc, n_acro_src);

    int rc = sqlite3_prepare_v2(amtdb->db, sql_ins, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        /* Clean up readline allocated memory */
        if (complete != NULL) {
            free(complete);
        }
        if (n_acro != NULL) {
            free(n_acro);
        }
        if (n_acro_expd != NULL) {
            free(n_acro_expd);
        }
        if (n_acro_desc != NULL) {
            free(n_acro_desc);
        }
        if (n_acro_src != NULL) {
            free(n_acro_src);
        }
        clear_history();
        return false;
    }

    rc = sqlite3_exec(amtdb->db, sql_ins, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(amtdb->db));
        /* Clean up readline allocated memory */
        if (complete != NULL) {
            free(complete);
        }
        if (n_acro != NULL) {
            free(n_acro);
        }
        if (n_acro_expd != NULL) {
            free(n_acro_expd);
        }
        if (n_acro_desc != NULL) {
            free(n_acro_desc);
        }
        if (n_acro_src != NULL) {
            free(n_acro_src);
        }
        clear_history();
        return false;
    }

    sqlite3_finalize(stmt);

    /* free up any allocated memory by sqlite3 */
    if (sql_ins != NULL) {
        sqlite3_free(sql_ins);
    }

    /* Clean up readline allocated memory */
    if (complete != NULL) {
        free(complete);
    }
    if (n_acro != NULL) {
        free(n_acro);
    }
    if (n_acro_expd != NULL) {
        free(n_acro_expd);
    }
    if (n_acro_desc != NULL) {
        free(n_acro_desc);
    }
    if (n_acro_src != NULL) {
        free(n_acro_src);
    }
    clear_history();

    set_rec_count(amtdb);
    printf("Inserted '%d' new record. Total database record count "
           "is now"
           " %'d (was %'d).\n",
           (amtdb->totalrec - amtdb->prevtotalrec), amtdb->totalrec, amtdb->prevtotalrec);

    return true;
}

/********************************************************************/
/* DELETE A RECORD BASE ROWID                                       */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯                                       */
/* select rowid,Acronym,Definition,Description,Source from ACRONYMS */
/* where rowid                                                      */
/* = ?;                                                             */
/*                                                                  */
/* delete from ACRONYMS where rowid = ?;                            */
/********************************************************************/
bool del_acro_rec(int del_rec_id, amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_rec_count(amtdb);

    printf("\nDeleting an acronym record...\n");
    printf("\nNote: To abort the delete of a record - press 'Ctrl "
           "+ c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", del_rec_id);

    int rc = sqlite3_prepare_v2(amtdb->db,
                            "select rowid,Acronym,Definition,Description,"
                            "Source from ACRONYMS where rowid like ?;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 1, del_rec_id);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        exit(EXIT_FAILURE);
    }

    int delete_rec_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: %s.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 3));
        printf("SOURCE: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        delete_rec_count++;
    }

    sqlite3_finalize(stmt);

    if (delete_rec_count == 1) {
        char *cont_del = NULL;
        cont_del = readline("\nDelete above record? [ y/n ] : ");
        if (strcasecmp((const char *)cont_del, "y") == 0) {

            rc = sqlite3_prepare_v2(amtdb->db,
                                    "delete from ACRONYMS where "
                                    "rowid = ?;",
                                    -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                return false;
            }

            rc = sqlite3_bind_int(stmt, 1, del_rec_id);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                return false;
            }

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(amtdb->db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                return false;
            }

            /* free readline memory allocated */
            if (cont_del != NULL) {
                free(cont_del);
            }
            sqlite3_finalize(stmt);
        } else {
            printf("\nRequest to delete record ID '%d' was abandoned "
                   "by the user\n\n",
                   del_rec_id);
        }
    } else if (delete_rec_count > 1) {
        printf(" » ERROR: record ID '%d' search returned '%d' records "
               "«\n\n",
               del_rec_id, delete_rec_count);
    } else {
        printf(" » WARNING: record ID '%d' found '%d' matching "
               "records «\n\n",
               del_rec_id, delete_rec_count);
    }

    set_rec_count(amtdb);
    printf("Deleted '%d' record. Total database record count is now"
           " %'d (was %'d).\n",
           (amtdb->prevtotalrec - amtdb->totalrec), amtdb->totalrec, amtdb->prevtotalrec);

    return true;
}

/******************************************/
/* GETTING LIST OF ACRONYM SOURCES        */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯        */
/* select distinct(source) from acronyms; */
/******************************************/
void get_acro_src(amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(amtdb->db,
                            "select distinct(source) "
                            "from acronyms order by source;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        exit(-1);
    }

    char *acro_src_name;

    printf("\nSelect a source (use ↑ or ↓ ):\n\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        acro_src_name = strdup((const char *)sqlite3_column_text(stmt, 0));
        printf("[ %s ] ", acro_src_name);
        add_history(acro_src_name);

        /* free per loop to stop memory leaks - strdup malloc
         * above */
        if (acro_src_name != NULL) {
            free(acro_src_name);
        }
    }
    printf("\n");

    sqlite3_finalize(stmt);
}

/********************************************************************/
/* UPDATE A RECORD BASED ON ROWID                                   */
/* select rowid,Acronym,Definition,Description,Source from ACRONYMS */
/* where rowid                                                      */
/* = ?;                                                             */
/*                                                                  */
/* update                           */
/********************************************************************/
bool update_acro_rec(int update_rec_id, amtdb_struct *amtdb)
{
    sqlite3_stmt *stmt = NULL;
    set_rec_count(amtdb);

    printf("\nUpdating an acronym record...\n");
    printf("\nNote: To abort the update of a record - press 'Ctrl "
           "+ c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", update_rec_id);

    /* ifnull() is used to replace any database NULL fields with
       an empty string as NULL was causing issues with readline
       when saving a NULL value to add_history() */
    int rc =
        sqlite3_prepare_v2(amtdb->db,
                           "select rowid, ifnull(Acronym,''),"
                           " ifnull(Definition,''), ifnull(Description,''),"
                           " ifnull(Source,'') from ACRONYMS where rowid is ?;",
                           -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
        return false;
    }

    rc = sqlite3_bind_int(stmt, 1, update_rec_id);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
        return false;
    }

    int update_rec_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: %s.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));

        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 3));
        printf("SOURCE: %s\n", (const char *)sqlite3_column_text(stmt, 4));
        /* grab a copy of the returned record id fields into Readline
         * history - for user recall later to save re-typing entries */
        add_history((const char *)sqlite3_column_text(stmt, 1));
        add_history((const char *)sqlite3_column_text(stmt, 2));
        add_history((const char *)sqlite3_column_text(stmt, 3));
        add_history((const char *)sqlite3_column_text(stmt, 4));
        update_rec_count++;
    }

    sqlite3_finalize(stmt);

    /* if we found a record to update */
    if (update_rec_count == 1) {
        char *cont_del = NULL;
        cont_del = readline("\nUpdate above record? [ y/n ] : ");
        if (strcasecmp((const char *)cont_del, "y") == 0) {

            char *complete = NULL;
            char *u_acro = NULL;
            char *u_acro_expd = NULL;
            char *u_acro_desc = NULL;
            char *u_acro_src = NULL;

            printf("\nUse ↑ or ↓ keys to select previous entries text "
                   "for re-editing or just type in new:\n\n");

            while (1) {
                u_acro = readline("Enter the acronym: ");
                add_history(u_acro);
                u_acro_expd = readline("Enter the expanded acronym: ");
                add_history(u_acro_expd);
                u_acro_desc = readline("Enter the acronym description: \n\n");
                add_history(u_acro_desc);
                get_acro_src(amtdb);
                u_acro_src = readline("\nEnter the acronym source: ");
                add_history(u_acro_src);

                printf("\nConfirm entry for:\n\n");
                printf("ACRONYM:     '%s' is: %s.\n", u_acro, u_acro_expd);
                printf("DESCRIPTION: %s\n", u_acro_desc);
                printf("SOURCE:      %s\n\n", u_acro_src);

                complete = readline("Enter record? [ y/n or q ] : ");
                if (strcasecmp((const char *)complete, "y") == 0) {
                    break;
                }
                if (strcasecmp((const char *)complete, "q") == 0) {
                    /* Clean up readline allocated memory */
                    if (complete != NULL) {
                        free(complete);
                    }
                    if (u_acro != NULL) {
                        free(u_acro);
                    }
                    if (u_acro_expd != NULL) {
                        free(u_acro_expd);
                    }
                    if (u_acro_desc != NULL) {
                        free(u_acro_desc);
                    }
                    if (u_acro_src != NULL) {
                        free(u_acro_src);
                    }
                    clear_history();
                    return false;
                }
            }

            char *sql_update = NULL;

            /* build SQLite 'UPDATE' query */
            sql_update =
                sqlite3_mprintf("update ACRONYMS set Acronym=%Q, "
                                "Definition=%Q, Description=%Q, "
                                "Source=%Q where rowid is ?;",
                                u_acro, u_acro_expd, u_acro_desc, u_acro_src);

            rc = sqlite3_prepare_v2(amtdb->db, sql_update, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(amtdb->db));
                /* Clean up readline allocated memory */
                if (complete != NULL) {
                    free(complete);
                }
                if (u_acro != NULL) {
                    free(u_acro);
                }
                if (u_acro_expd != NULL) {
                    free(u_acro_expd);
                }
                if (u_acro_desc != NULL) {
                    free(u_acro_desc);
                }
                if (u_acro_src != NULL) {
                    free(u_acro_src);
                }
                clear_history();
                return false;
            }

            /* bind in the record id to UPDATE */
            rc = sqlite3_bind_int(stmt, 1, update_rec_id);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(amtdb->db));
                return false;
            }

            /* reset update_rec_count so can re-use here */
            update_rec_count = 0;

            /* perform the actual database update */
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                /* should not run here as 'sqlite3_step(stmt)'
                   should
                   immediately return with SQLITE_DONE for an
                   SQL UPDATE */
                update_rec_count++;
            }

            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(amtdb->db));
                /* Clean up readline allocated memory */
                if (complete != NULL) {
                    free(complete);
                }
                if (u_acro != NULL) {
                    free(u_acro);
                }
                if (u_acro_expd != NULL) {
                    free(u_acro_expd);
                }
                if (u_acro_desc != NULL) {
                    free(u_acro_desc);
                }
                if (u_acro_src != NULL) {
                    free(u_acro_src);
                }
                clear_history();
                return false;
            }

            /* capture number of Sqlite database changes made in
               last transaction - should be one */
            int db_changes = sqlite3_changes(amtdb->db);

            /* check for how many records were updated - should only
             * be one! */
            if (update_rec_count > 0) {
                printf("\n\nWARNING: Database changes made was: "
                       "'%d' but record changes made by "
                       "sqlite3_step() "
                       "were: '%d'\n",
                       db_changes, update_rec_count);
                printf("Only record id: '%d' should of been "
                       "changed - "
                       "but '%d' changes were made "
                       "unexpectedly.\n\n",
                       update_rec_id, update_rec_count);
            } else {
                /* no update issues found - so set count to
                   actual
                   database change count */
                update_rec_count = db_changes;
            }

            sqlite3_finalize(stmt);

            /* free up any allocated memory by sqlite3 */
            if (sql_update != NULL) {
                sqlite3_free(sql_update);
            }

            /* Clean up readline allocated memory */
            if (complete != NULL) {
                free(complete);
            }
            if (u_acro != NULL) {
                free(u_acro);
            }
            if (u_acro_expd != NULL) {
                free(u_acro_expd);
            }
            if (u_acro_desc != NULL) {
                free(u_acro_desc);
            }
            if (u_acro_src != NULL) {
                free(u_acro_src);
            }
            clear_history();

            set_rec_count(amtdb);
            printf("Updated '%d' record. Total database record count "
                   "is now"
                   " %'d (was %'d).\n",
                   amtdb->totalrec, amtdb->totalrec, amtdb->prevtotalrec);
        }
    }
    return true;
}
