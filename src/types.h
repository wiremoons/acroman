/* Acronym Management Tool (amt)
 *
 * types.h : Structure to manage SQLite database information
 */

#ifndef AMT_TYPES_H
#define AMT_TYPES_H

typedef struct AmtDB_Struct {
    char *dbfile;
    sqlite3 *db;
} amtdb_struct;


#endif //AMT_TYPES_H
