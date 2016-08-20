/* Acronym Management Tool (amt): amt.c */

#include "amt.h"

/**-------- FUNCTION:  recCount
** 
** Function: run SQL query to obtain current number of acronyms in the database
** 
*/
int recCount(void)
{
    int totalrec = 0;
    /* prepare a SQL statement to obtain the current record count of the table */
    rc = sqlite3_prepare_v2(db,"select count(*) from ACRONYMS",-1, &stmt, NULL);
    if ( rc != SQLITE_OK) exit(-1);

    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        totalrec = sqlite3_column_int(stmt,0);
        if (debug) { printf("DEBUG: total records found: %d\n",totalrec); }
    }

    sqlite3_finalize(stmt);
    return(totalrec);
}

/**-------- FUNCTION: checkDB
** 
** Function: check for a valid database file to open
** 
*/
void checkDB(void)
{

    /* check if acronyms database file was supplied on the command line */

    /* if ( ! dbfile = "") */
    /* { */

    /* } */

    /* obtain the acronyms database file from the environment */
    dbfile = getenv("ACRODB");
    if (dbfile)
    {
        printf(" - Database location: %s\n", dbfile);
        /* check database file is valid and accessible */
        if (access(dbfile, F_OK | R_OK) == -1)
        {
            fprintf(stderr,"\n\nERROR: The database file '%s'"
                    " is missing or is not accessible\n\n", dbfile);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("\tWARNING: No database specified using 'ACRODB' environment variable\n");
        exit(EXIT_FAILURE);
    }
    // if neither of the above - check current directory we are running
    // in - or then offer to create a new db? otherwise exit prog here

}

