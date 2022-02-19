[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/hyperium/hyper/master/LICENSE)![](https://github.com/wiremoons/acroman/workflows/amt-build/badge.svg)

# What is 'amt'?

A small command line application called '`amt`' (which is an acronym for
'*Acronym Management Tool*') which can be used to store, look up, change, or
delete acronyms that are kept in a SQLite database.

The program is small, fast, and is free software. It is used on a daily basis by
the author, running it on both Linux, macSOS, and Windows operating systems. 
It should also compile and run on the various BSD Unix too, although this has
not been tested.


## Status

From version 0.5.0 of '`amt`' it is functionally feature complete, based on its
ability to provide **CRUD**. This is a set of basic features which includes:

 - *CREATE* : new records can be added (ie created) in the database;
 - *READ* : existing records can be searched for (ie read) from the database;
 - *UPDATE*: existing records held in the database can be altered (ie changed);
 - *DELETE*: existing records held in the database can be removed (ie deleted).

This does not mean the program is fully completed (or bug free) - but that it
provides a solid basic set of functionality. The program is used on a daily basis 
so it will continue to be improved as is felt necessary.


## Usage Examples

Running `amt` without any parameters, but with a database already available, it 
will output the following information:

```
ERROR: no command lines argument provided.

'/Users/simon/GenIsys-macOS/assets/amt-arm64' version is: '0.10.2'.
Compiled on: 'Feb 19 2022 @ 19:20:47'.
Copyright (c) 2022 Simon Rowe.

C source built as 'Release' using compiler '13.0.0 (clang-1300.0.29.30)'.

For licenses and further information visit:
Application:          https://github.com/wiremoons/acroman
Linenoise:            https://github.com/antirez/linenoise
SQLite database:      https://www.sqlite.org/

Database full path:   '/Users/simon/icloud/amt-db/acronyms.db'
Database file size:   '2,134,016' bytes
Database modified:    'Fri 18 Feb 2022 @ 21:28:27'

SQLite version:       '3.37.1'
Total acronyms:       '17,951'
Last acronym entered: 'STL'


Application to manage acronyms stored in a SQLite database.
Usage: /Users/simon/GenIsys-macOS/assets/amt-arm64 [switches] [arguments]

[Switches]        [Arguments]      [Description]
-d, --delete       <rec_id>        delete an acronym record. Argument is mandatory.
-h, --help                         display help information.
-l, --latest                       display the five latest records added.
-n, --new                          add a new record.
-s, --search       <acronym>       find a acronym record. Argument is mandatory.
-u, --update       <rec_id>        update an existing record. Argument is mandatory.
-v, --version                      display program version information.

Arguments
 <acronym> : a string representing an acronym to be found. Use quotes if contains spaces.
 <rec_id>  : unique number assigned to each acronym. Can be found with a '-s, --search'.
Use '%' for wildcard searches.
```

Running `amt -h` or `amt -v` displays a cut down version of the above output, just showing 
those specific elements respectively. 


## Building the Application

A C language compiler will be needed to build the application. There are also a
few dependencies that are required — many of which are included where possible.
For a successful build of `amt` follow the more detailed steps below.

### Dependencies

In order to compile `amt` successfully, it requires a few development C code
libraries to support its functionality. These libraries are described below for
reference.

#### SQLite

The application uses SQLite to store the acronyms. The application 
**includes copies** of the `sqlite3.c` and `sqlite3.h` source code files 
the code distribution. These are compiled directly into the application when it 
is built. The two included files are obtained from 
[http://www.sqlite.org/amalgamation.html](The SQLite Amalgamation) source 
download option.

More information on SQLite can be found here: [http://www.sqlite.org/](http://www.sqlite.org/).

#### Linenoise

The application uses *linenoise* craeted by Salvatore Sanfilippo to manage the user input and input history when 
updating or adding new acronym records. The application **includes copies** of the 
`linenoise.c` and `linenoise.h` source code files within this applications own code 
distribution. These are compiled directly into the application when it
is built. The two included files are obtained from
[https://github.com/antirez/linenoise](https://github.com/antirez/linenoise)

The `amt` versions prior to '`0.9.0`' used the *GNU Readline* library which has been 
removed completely in favour of the self-contained and more flexible (for my usage at 
least)  *linenoise*, as described above.

### Install a C Compiler and Supporting Libraries

To install the required libraries and compiler tools on various systems, use the
following commands before attempting to compile `amt`:

- Ubuntu Linux: `sudo apt install build-essential cmake`
- Fedora (Workstation) Linux: `sudo dnf install cmake`

On Windows you can use a C compiler such as MinGW (or equivalent) to build the
application. In order to build the Windows version for testing and personal use,
this is often done on Fedora Workstation as a cross compile.

### Building 'amt'

Before trying to build `amt` make sure you have the required dependencies install
- see above for more information.

Use the provided `bash` shell script to run the build steps: `./build.sh`

The build binary will be added to a new `./bin` subdirectory - the above build script 
also provided the same information.

To compile yourself without using `cmake` or the recommended `build.sh` script - the 
following command can be used to compile `amt` with GCC compiler on a 64bit Linux 
system is shown below:
```shell
cc -g -Wall -m64 -std=gnu11 -o amt amt-db-funcs.c main.c sqlite3.c linenoise.c -lpthread -ldl
```

## Database Location

The SQLite database used to store the acronyms can be located in the same
directory as the programs executable. The default filename that is looked for
by the program is: '***acronyms.db***'

However, this can be overridden, by giving a preferred location, which can be
specified by an environment variable called ***ACRODB***. You should set this
to the path and preferred database file name of your SQLite acronyms database.
Examples of how to set this for different operating systems are shown below.

On Linux and similar operating systems when using bash shell, add this line to
your `.bashrc` configuration file, located in your home directory (ie
*~/.bashrc*), just amend the path and database file name to suit your own needs:

```
export ACRODB=$HOME/work/my-own.db
```

on Windows or Linux when using Microsoft Powershell:

```
$env:ACRODB += "c:\users\simon\work\my-own.db"
```

on Windows when using a cmd.exe console:

```
set ACRODB=c:\users\simon\work\my-own.db
```

or Windows to add persistently to your environment run the following in a
cmd.exe console:

```
setx ACRODB=c:\users\simon\work\my-own.db
```

## Database and Acronyms Table Setup

**NOTE:** More detailed information is to be added here - plus see point 1 in
todo list below.

SQLite Table used by the program is created with:

```
CREATE TABLE Acronyms ("Acronym","Definition","Description","Source");
```
As long as the same table name and column names are used, the program should
function with an empty database.

With the SQLite command line application `sqlite3` (or `sqlite3.exe` on
Windows) you can create a new database and add a new record using a terminal
window and running the following commands:

```
sqlite3 acronyms.db

CREATE TABLE Acronyms ("Acronym", "Definition", "Description", "Source");

INSERT INTO ACRONYMS(Acronym,Definition,Description,Source) values 
("AMT2","Acronym Management Tool",
"Command line application to manage a database of acronyms.","Misc");

.quit
```


## Todo ideas and Future Development Plans

Below are some ideas that I am considering adding to the program, in no
particular priority order.

1. Offer to create a new default database if one is not found on start up
2. Ability to populate the database from a remote source
3. Ability to update and/or check for a new version of the program
4. Output of records in different formats (json, csv, etc)
5. Ability to backup database
6. Ability to backup the table within database and keep older versions
7. Tune and add an index to the database
8. Merge contents of different databases that have been updated on separate computer to keep in sync


## Licenses

The following opensource licenses apply to the `amt` source code, and resulting built
application.

#### License for 'amt'

This program `amt` is licensed under the **MIT License** see
http://opensource.org/licenses/mit. A copy of the applications license is here 
[amt License](https://github.com/wiremoons/acroman/blob/master/LICENSE.txt).

#### License for 'SQLite'

The *SQLite* database code used in this application is licensed as **Public
Domain**, see http://www.sqlite.org/copyright.html for more details.

#### License for 'linenoise'

The *linenoise* code used in this application is licensed under the 
**BSD-2-Clause License**. See [Linenoise License](https://github.com/antirez/linenoise/blob/master/LICENSE) 
for more details. The *linenoise* code is copyright (c) of Salvatore Sanfilippo contactable 
at *antirez at gmail dot com*.

