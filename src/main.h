/**
 * @file main.h
 * @brief Acronym Management Tool (amt). A program to managed SQLite database containing acronyms.
 *
 * @author     simon rowe <simon@wiremoons.com>
 * @license    open-source released under "MIT License"
 * @source     https://github.com/wiremoons/acroman
 *
 * @date originally created: 05 Jan 2016
 * @date updated significantly: 01 May 2021
 *
 * The program is licensed under the "*MIT License*" see
 * http://opensource.org/licenses/MIT for more details.
 *
 * @details The program is used to managed SQLite database containing acronyms. This source code manages the programs
 * execution. The application uses the SQLite amalgamation source code files. Ensure the latest version is included
 * in the same directory as this programs source code. The program can access the SQLite database and look up a
 * requested acronym that maybe held in a table called 'ACRONYMS'. Also supports the creation of new acronym records,
 * alterations of existing, and deletion of records no longer required.
 *
 * @note The program can e compiled with CMake or directly with
 * @code cc -Wall -std=gnu11 -g -o amt ./src/amt-db-funcs.c ./src/main.c ./src/sqlite3.c ./src/linenoise.c -lpthread -ldl
 *
 */

#ifndef AMT_MAIN_H /* Include guard */
#define AMT_MAIN_H

#include "amt-db-funcs.h" /* manages the database access for the application */
#include "types.h"        /* Structure to manage SQLite database information */
#include "sqlite3.h"      /* SQLite header */

const char amtVersion[] = "0.9.2";  /** @note set the version of the app here */
amtdb_struct amtdb;                 /** @note Declared globally for 'atexit()'. See 'types.h' */

void exit_cleanup(void);            /** @note Run by 'atexit()' on normal program exit */
void show_help(void);               /** @note display help and usage information to screen */
void display_version(void);         /** @note display program version details */
bool bootstrap_db(void);            /** @note ensure database is available and accessible */

#endif // AMT_MAIN_H
