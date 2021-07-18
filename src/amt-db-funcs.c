/* Acronym Management Tool (amt): amt.c */

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
int get_rec_count(void)
{
    int totalrec = 0;
    rc = sqlite3_prepare_v2(db, "select count(*) from ACRONYMS", -1, &stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        perror("\nERROR: unable to access the SQLite database to "
               "perform a record count\n");
        exit(EXIT_FAILURE);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalrec = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return (totalrec);
}

/****************************************************************/
/* Checks for a valid database filename to open looking at:     */
/* - 1 : environment variable 'ACRODB'                          */
/* - 2 : file 'acronyms.db' in same location as the application */
/* - 3 : TODO offer to create a new Database                    */
/****************************************************************/
void check4DB(const char *prog_name)
{
    bool run_ok;

    /* get database file from environment variable ACRODB first */
    dbfile = getenv("ACRODB");
    /* if the environment variable exists - check if its valid */
    if (dbfile != NULL) {
        run_ok = check_db_access();
		/* if db found ok just return */
        if (run_ok) {
            return;
        }
    }

    /* nothing is set in environment variable ACRODB - so database 			*/
    /* might be found in the application directory instead, named 				*/
    /* as the default filename: 'acronyms.db' 									*/
    /* tmp copy needed here as each call to dirname() below can change the	*/
    /* string being used in the call - so need one string copy for each 		*/
    /* successful call we need to make. This is a 'feature' of dirname() 		*/
    char *tmp_dirname = strdup(prog_name);

    size_t new_dbfile_sz = (sizeof(char) * (strlen(dirname(tmp_dirname)) +
                                            strlen("/acronyms.db") + 1));

    char *new_dbfile = malloc(new_dbfile_sz);

    if (new_dbfile == NULL) {
        perror("\nERROR: unable to allocate memory with "
               "malloc() for 'new_dbfile' and path\n");
        exit(EXIT_FAILURE);
    }

    int x = snprintf(new_dbfile, new_dbfile_sz, "%s%s", dirname((char *)prog_name),
                     "/acronyms.db");

    if (x == -1) {
        perror("\nERROR: unable to allocate memory with "
               "snprintf() for 'new_dbfile' and path\n");
        exit(EXIT_FAILURE);
    }

    /* 'new_dbfile' is ready as it contains the path and the 			*/
    /* default db filename. To use it, now copy it our global var 		*/
    /* 'dbfile' ; then get rid of new_dbfile as it is done with 		*/
    if ((dbfile = strdup(new_dbfile)) == NULL) {
        perror("\nERROR: unable to allocate memory with "
               "strdup() for 'new_dbfile' to 'dbfile' copy\n");
        exit(EXIT_FAILURE);
    }

    /* debug code
       printf("\nnew_dbfile: '%s' and dbfile: '%s'\n", new_dbfile, dbfile);
    */

    if (new_dbfile != NULL) {
        free(new_dbfile);
    }

    /* now recheck if the new_dbfile is suitable for use? */
    run_ok = check_db_access();
    if (run_ok) {
        return;
    }

    /* run out of options to find a suitable database - exit */
    /* TODO : offer to create a new dbfile here! */
    printf("\n\tWARNING: No suitable database file can be located - "
           "program will exit\n");
    exit(EXIT_FAILURE);
}

/**********************************************************************/
/* Check the filename and path given for the acronym database and see */
/* if it is accessible. If the file is located then print some stats  */
/* on path, size and last modified time.                              */
/**********************************************************************/
bool check_db_access(void)
{
    if (dbfile == NULL || strlen(dbfile) == 0) {
        fprintf(stderr,
                "ERROR: The database file '%s'"
                " is an empty string\n",
                dbfile);
        return (false);
    }

    if (access(dbfile, F_OK | R_OK) == -1) {
        fprintf(stderr,
                "ERROR: The database file '%s'"
                " is missing or is not accessible\n",
                dbfile);
        return (false);
    }

    printf(" - Database location: %s\n", dbfile);

    struct stat sb;
    int check;

    check = stat(dbfile, &sb);

    if (check) {
        perror("ERROR: call to 'stat' for database file "
               "failed\n");
        return (false);
    }

    printf(" - Database size: %'lld bytes\n", sb.st_size);
    printf(" - Database last modified: %s\n", ctime(&sb.st_mtime));

    return (true);
}

/*************************************************************/
/* GET NAME OF LAST ACRONYM ENTERED                          */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯                          */
/* SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1; */
/*************************************************************/
char *get_last_acronym(void)
{
    char *acronym_name;

    rc = sqlite3_prepare_v2(
        db, "SELECT Acronym FROM acronyms Order by rowid DESC LIMIT 1;", -1,
        &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
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
/* select rowid,Acronym,Definition,                     */
/* Description,Source from ACRONYMS                     */
/* where Acronym like ? COLLATE NOCASE ORDER BY Source; */
/********************************************************/
int do_acronym_search(char *findme)
{
    printf("\nSearching for: '%s' in database...\n\n", findme);

    rc = sqlite3_prepare_v2(db,
                            "select rowid,Acronym,Definition,Description,"
                            "Source from ACRONYMS where Acronym like ? "
                            "COLLATE NOCASE ORDER BY Source;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_text(stmt, 1, (const char *)findme, -1, SQLITE_STATIC);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    int search_rec_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID:          %s\n", (const char *)sqlite3_column_text(stmt, 0));
        printf("ACRONYM:     '%s' is: %s.\n",
               (const char *)sqlite3_column_text(stmt, 1),
               (const char *)sqlite3_column_text(stmt, 2));
        printf("DESCRIPTION: %s\n", (const char *)sqlite3_column_text(stmt, 3));
        printf("SOURCE:      %s\n\n",
               (const char *)sqlite3_column_text(stmt, 4));
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
int new_acronym(void)
{
    int old_rec_cnt = get_rec_count();

    printf("\nAdding a new record...\n");
    printf("\nNote: To abort the input of a new record - press "
           "'Ctrl + "
           "c'\n\n");

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

        get_acro_src();
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
            exit(EXIT_FAILURE);
        }
    }

    char *sql_ins = NULL;
    sql_ins = sqlite3_mprintf("insert into ACRONYMS"
                              "(Acronym, Definition, Description, Source) "
                              "values(%Q,%Q,%Q,%Q);",
                              n_acro, n_acro_expd, n_acro_desc, n_acro_src);

    rc = sqlite3_prepare_v2(db, sql_ins, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
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
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_exec(db, sql_ins, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(db));
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
        exit(EXIT_FAILURE);
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

    int new_rec_cnt = get_rec_count();
    printf("Inserted '%d' new record. Total database record count "
           "is now"
           " %'d (was %'d).\n",
           (new_rec_cnt - old_rec_cnt), new_rec_cnt, old_rec_cnt);

    return 0;
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
int del_acro_rec(int del_rec_id)
{
    int old_rec_cnt = get_rec_count();
    printf("\nDeleting an acronym record...\n");
    printf("\nNote: To abort the delete of a record - press 'Ctrl "
           "+ c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", del_rec_id);

    rc = sqlite3_prepare_v2(db,
                            "select rowid,Acronym,Definition,Description,"
                            "Source from ACRONYMS where rowid like ?;",
                            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 1, del_rec_id);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
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

            rc = sqlite3_prepare_v2(db,
                                    "delete from ACRONYMS where "
                                    "rowid = ?;",
                                    -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                exit(EXIT_FAILURE);
            }

            rc = sqlite3_bind_int(stmt, 1, del_rec_id);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                exit(EXIT_FAILURE);
            }

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(db));
                if (cont_del != NULL) {
                    free(cont_del);
                }
                exit(EXIT_FAILURE);
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

    int new_rec_cnt = get_rec_count();
    printf("Deleted '%d' record. Total database record count is now"
           " %'d (was %'d).\n",
           (old_rec_cnt - new_rec_cnt), new_rec_cnt, old_rec_cnt);

    return delete_rec_count;
}

/******************************************/
/* GETTING LIST OF ACRONYM SOURCES        */
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯        */
/* select distinct(source) from acronyms; */
/******************************************/
void get_acro_src(void)
{
    rc = sqlite3_prepare_v2(db,
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
int update_acro_rec(int update_rec_id)
{
    int old_rec_cnt = get_rec_count();
    printf("\nUpdating an acronym record...\n");
    printf("\nNote: To abort the update of a record - press 'Ctrl "
           "+ c'\n\n");

    printf("\nSearching for record ID: '%d' in database...\n\n", update_rec_id);

    /* ifnull() is used to replace any database NULL fields with
       an empty string as NULL was causing issues with readline
       when saving a NULL value to add_history() */
    rc =
        sqlite3_prepare_v2(db,
                           "select rowid, ifnull(Acronym,''),"
                           " ifnull(Definition,''), ifnull(Description,''),"
                           " ifnull(Source,'') from ACRONYMS where rowid is ?;",
                           -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 1, update_rec_id);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
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
                get_acro_src();
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
                    exit(EXIT_FAILURE);
                }
            }

            char *sql_update = NULL;

            /* build SQLite 'UPDATE' query */
            sql_update =
                sqlite3_mprintf("update ACRONYMS set Acronym=%Q, "
                                "Definition=%Q, Description=%Q, "
                                "Source=%Q where rowid is ?;",
                                u_acro, u_acro_expd, u_acro_desc, u_acro_src);

            rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
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
                exit(EXIT_FAILURE);
            }

            /* bind in the record id to UPDATE */
            rc = sqlite3_bind_int(stmt, 1, update_rec_id);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
                exit(EXIT_FAILURE);
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
                fprintf(stderr, "SQL exec error: %s\n", sqlite3_errmsg(db));
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
                exit(EXIT_FAILURE);
            }

            /* capture number of Sqlite database changes made in
               last transaction - should be one */
            int db_changes = sqlite3_changes(db);

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

            int new_rec_cnt = get_rec_count();
            printf("Updated '%d' record. Total database record count "
                   "is now"
                   " %'d (was %'d).\n",
                   update_rec_count, new_rec_cnt, old_rec_cnt);
        }
    }
    return update_rec_count;
}
