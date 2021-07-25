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
 * major update : July 2021 - refactored and improved source code and outputs
 *
 * The application uses the SQLite amalgamation source code files, so ensure
 * they are included in the same directory as this programs source code.
 * To build the program, use the provided Makefile or compile with:
 *
 * gcc -Wall -std=gnu11 -m64 -g -o amt amt-db-funcs.c main.c
 * sqlite3.c lpthread -ldl -lreadline
 *
 */

#ifndef MAIN_H_ /* Include guard */
#define MAIN_H_

#include "amt-db-funcs.h" /* manages the database access for the application */
#include "types.h"        /* Structure to manage SQLite database information */
#include "sqlite3.h"      /* SQLite header */

const char amt_version[] = "0.7.1";    /* set the version of the app here */
amtdb_struct amtdb;                    /* Declared globally for 'atexit()'. See 'types.h' */

void exit_cleanup(void);                       /* Run by 'atexit()' on normal program exit */
void show_help(void);                          /* display help and usage information to screen */
void display_version(const char *prog_name);   /* display program version details */

#endif // MAIN_H_
