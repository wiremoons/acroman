/* Acronym Management Tool (amt)
 *
 * types.h : Structure to manage SQLite database information
 */

#ifndef AMT_TYPES_H
#define AMT_TYPES_H

typedef struct AmtDB_Struct {
    char *dbfile;
    sqlite3 *db;
    long long dbsize;
    char *dblastmod;
    int totalrec;
    int prevtotalrec;
    int maxrecid;
} amtdb_struct;


#endif //AMT_TYPES_H
