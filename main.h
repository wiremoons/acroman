/*
 * Acronym Management Tool (amt): main.h
 *
 * amt is program to manage acronyms held in an SQLite database
 *
 * author:     simon rowe <simon@wiremoons.com>
 * license:    open-source released under "MIT License"
 * source:     https://github.com/wiremoons/amt
 *
 * Program to access a SQLite database and look up a requested acronym that
 * maybe held in a table called 'ACRONYMS'.
 *
 * Also supports the creation of new acronym records, alterations of existing,
 * and deletion of records no longer required.
 *
 * created: 20 Jan 2016 - initial outline code written
 *
 * The application uses the SQLite amalgamation source code files, so ensure
 * they are included in the same directory as this programs source code.
 * To build the program, use the provided Makefile or compile with:
 *
 * gcc -Wall -std=gnu11 -m64 -g -o amt amt-db-funcs.c cli-args.c main.c
 * sqlite3.c lpthread -ldl -lreadline
 *
 */

#ifndef MAIN_H_ /* Include guard */
#define MAIN_H_

#include "amt-db-funcs.h" /* manages the database access for the application */
#include "cli-args.h"     /* manages the command line args from user */
#include "sqlite3.h"      /* SQLite header */

#include <stdlib.h> /* to allow NULL to be used for globals var declarations */

/*
 *   APPLICATION GLOBAL VARIABLES
 */

char *dbfile = "";		/* path and name of acronyms database filename */
sqlite3 *db = NULL;		/* handle to the database */
int rc = 0;			/* returned result codes from calling SQLite functions */
const char *data = NULL;	/* data returned from SQL stmt run */
sqlite3_stmt *stmt = NULL;	/* preprepared SQL query statement */
char appversion[] = "0.5.1";	/* set the version of the app here */
int help = 0;			/* control help outputs request 0 == off | 1 == on */
char *findme = NULL;		/* string request on command line for acronym search */
int del_rec_id = -1;		/* database record id (rowid) used to delete records */
int newrec = 0;			/* request to add a new record 0 == off | 1 == on */
int update_rec_id = -1;		/* database record id (rowid) used to update records */

/* FUNCTION DECLARATIONS FOR main.c */

void exit_cleanup(void);
void show_help(void);
void print_start_screen(char *prog_name);

#endif // MAIN_H_
