/* Acronym Management Tool (amt): amt.h */

#ifndef AMT_DB_FUNCS_H_ /* Include guard */
#define AMT_DB_FUNCS_H_

#include "sqlite3.h" /* SQLite header */
#include <stdbool.h> /* use of true / false booleans for declaration below*/

extern char *dbfile;
extern sqlite3 *db;
extern int rc;
extern sqlite3_stmt *stmt;
extern const char *data;
extern char *findme;

/*
 *   FUNCTION DECLARATIONS
 */

int get_rec_count(void);        		/* get current acronym record count */
void check4DB(char *prog_name); 		/* ensure database exists and is accessible */
char *get_last_acronym(void);   		/* get last acronym added to database */
int do_acronym_search(char *findme); 	/* search database for 'findme' string */
int new_acronym(void);            		/* add a new record entry to the database */
void get_acro_src(void);          		/* get a list of acronym sources */
int del_acro_rec(int del_rec_id); 		/* delete a acronym record */
bool check_db_access(void); 			/* database file exists and can be accessed? */
int update_acro_rec(int update_rec_id);/* update a record in the database */

#endif // AMT_DB_FUNCS_H_
