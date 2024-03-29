/**
 * @file amt-db-funcs.h
 * @brief Acronym Management Tool (amt). A program to managed SQLite database containing acronyms.
 *
 * @author     simon rowe <simon@wiremoons.com>
 * @license    open-source released under "MIT License"
 * @source     https://github.com/wiremoons/acroman
 *
 */

#ifndef AMT_AMT_DB_FUNCS_H /* Include guard */
#define AMT_AMT_DB_FUNCS_H

#include "types.h"      /** @note Programs own structure to manage SQLite database information */
#include "sqlite3.h"    /** @note SQLite database C amalgamation header */
#include <stdbool.h>    /** @note use of true / false booleans for declarations below*/

bool set_record_count(amtdb_struct *amtdb);                        /* get current acronym record count */
bool check_4_db_file(amtdb_struct *amtdb);                         /* ensure database exists and is accessible */
bool check_db_access(amtdb_struct *amtdb);                         /* database file exists and can be accessed? */
bool initialise_database(amtdb_struct *amtdb);                     /* initialise SQLite and open database file */
char *get_last_acronym(amtdb_struct *amtdb);                       /* get last acronym added to database */
int do_acronym_search(char *findme, amtdb_struct *amtdb);          /* search database for 'findme' string */
bool new_acronym(amtdb_struct *amtdb);                             /* add a new record entry to the database */
void get_acronym_src_list(amtdb_struct *amtdb);                    /* get a list of acronym sources */
bool delete_acronym_record(int delRecId, amtdb_struct *amtdb);     /* delete a acronym record */
bool update_acronym_record(int updateRecId, amtdb_struct *amtdb);  /* update a record in the database */
bool output_db_stats(amtdb_struct *amtdb);                         /* show database file, file size, modified date */
bool update_max_recid(amtdb_struct *amtdb);                        /* obtain max record ID number in the database */
bool latest_acronym(amtdb_struct *amtdb);                          /* show five latest records in the database */

#endif // AMT_AMT_DB_FUNCS_H
