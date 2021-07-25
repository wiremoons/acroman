/* Acronym Management Tool (amt)
 *
 * amt-db-funcs.h : managed SQLite database functions
 */

#ifndef AMT_DB_FUNCS_H_ /* Include guard */
#define AMT_DB_FUNCS_H_

#include "sqlite3.h" /* SQLite header */
#include "types.h"   /* Structure to manage SQLite database information */
#include <stdbool.h> /* use of true / false booleans for declaration below*/

int get_rec_count(amtdb_struct *amtdb);                         /* get current acronym record count */
bool check4DBFile(const char *prog_name, amtdb_struct *amtdb);  /* ensure database exists and is accessible */
bool check_db_access(amtdb_struct *amtdb);                      /* database file exists and can be accessed? */
bool initialise_database(amtdb_struct *amtdb);                  /* initialise SQLite and open database file */
char *get_last_acronym(amtdb_struct *amtdb);                    /* get last acronym added to database */
int do_acronym_search(char *findme, amtdb_struct *amtdb);       /* search database for 'findme' string */
int new_acronym(amtdb_struct *amtdb);                           /* add a new record entry to the database */
void get_acro_src(amtdb_struct *amtdb);                         /* get a list of acronym sources */
int del_acro_rec(int del_rec_id,amtdb_struct *amtdb);           /* delete a acronym record */
int update_acro_rec(int update_rec_id,amtdb_struct *amtdb);     /* update a record in the database */

#endif // AMT_DB_FUNCS_H_
