[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/hyperium/hyper/master/LICENSE)![](https://github.com/wiremoons/acroman/workflows/amt-build/badge.svg)

# What is 'amt'?

A small command line application called '`amt`' (which is an acronym for
'*Acronym Management Tool*') which can be used to store, look up, change, or
delete acronyms that are kept in a SQLite database.

The program is small, fast, and is free software. It is used on a daily basis by
the author, running it on both Linux and Windows operating systems. It should
also compile and run on BSD Unix too, and Mac OS X as well, although this has
not been tested.


## Status

From version 0.5.0 of '`amt`' is functionally feature complete, based on its
ability to provide **CRUD**. This is a set of basic features which includes:

 - *CREATE* : new records can be added (ie created) in the database;
 - *READ* : existing records can be searched for (ie read) from the database;
 - *UPDATE*: existing records held in the database can be altered (ie changed);
 - *DELETE*: existing records held in the database can be removed (ie deleted).

This does not mean the program is fully completed (or bug free) - but that it
provides a basic set of functionality. The program is used on a daily basis by
the author, and will continue to be improved as is felt to be necessary.


## Usage Examples

Running `amt` without any parameters, but with a database available will
output the following information:

```
                Acronym Management Tool
                ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
Summary:
 - 'amt' version is: 0.5.0 complied with SQLite version: 3.18.0
 - Database location: /home/simon/Sybil/db-repo/acronyms.db
 - Database size: 1,929,216 bytes
 - Database last modified: Sun Apr 23 08:47:43 2017

 - Current record count is: 17,314
 - Newest acronym is: BPS

Completed SQLite database shutdown

All is well
```

Running `amt -h` displays the help screen which will output the following
information:

```
                Acronym Management Tool
                ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
Summary:
 - 'amt' version is: 0.5.0 complied with SQLite version: 3.18.0

Help Summary:
The following command line switches can be used:

  -d ?  Delete : remove an acronym where ? == ID of record to delete
  -h    Help   : show this help information
  -n    New    : add a new acronym record to the database
  -s ?  Search : find an acronym where ? == acronym to locate
  -u ?  Update : update an acronym where ? == ID of record to update

No SQLite database shutdown required

All is well
```

## Building the Application

A C language compiler will be needed to build the application. There are also a
few dependencies that will need to be met for a successful build of `amt`.
These steps are explained below.

### Dependencies

In order to compile `amt` successfully, it requires a few development C code
libraries to support its functionality. These libraries are described below for
reference.

#### SQLite

The application uses SQLite, however it **includes copies** of the `sqlite3.c` and
`sqlite3.h` source code files within this applications own code distribution.
These are are compiled directly into the application when it is built. The two
includes files are obtained from [http://www.sqlite.org/amalgamation.html](The
SQLite Amalgamation) source download option.

More information on SQLite can be found here: http://www.sqlite.org/

#### Readline

The application uses GNU Readline, and requires these development libraries to
be installed on the system that `amt` is being built on. On most Linux systems
the Readline library can be installed from the distributions software
repositories. The libraries will need to be available before `amt` will compile
successfully. The GNU Readline library is used to provide some of the
applications functionality during text entry.

More information on GNU Readline Library can be found here:
http://cnswww.cns.cwru.edu/php/chet/readline/readline.html

### Install a C Compiler and Supporting Libraries

To install the required libraries and compiler tools on various systems, use the
following commands before attempting to compile `amt`:

- Ubuntu Linux: `sudo apt install build-essential cmake readline-common libreadline-dev`
- Fedora (Workstation) Linux: `sudo dnf install readline-devel cmake`

On Windows you can use a C compiler such as MinGW (or equivalent) to build the
application. In order to build the Windows version for testing and personal use,
this is done on Fedora Workstation as a cross compile.

### Building 'amt'

Before trying to build `amt` make sure you have the required dependencies install
- see above for more information.

Use the provided `bash` shell script to run the build steps: `./build.sh`

The build binary will be added to a new `./bin` sub directory - the above build script also provided the same informaiton.

To compile yourself without using `cmake` or the recommended `build.sh` script - the following command can be used to compile `amt` with GCC compiler on a 64bit Linux system is shown below:
```shell
gcc -g -Wall -m64 -std=gnu11 -o amt amt-db-funcs.c main.c sqlite3.c -lpthread -ldl -lreadline
```

## Database Location

The SQLite database used to store the acronyms can be located in the same
directory as the programs executable. The default filename that is looked for
by the program is: '***acronyms.db***'

However this can be overridden, by giving a preferred location, which can be
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
4. Output of records in different fomats (json, csv, etc)
5. Ability to backup database
6. Ability to backup table within database and keep older versions
7. Tune and add an index to the database
8. Merge contents of different databases that have been updated on separate computer to keep in sync


## Known Issues

Below are issues known with v0.5.0:

**Issue ONE:** On record update the following is printed to the
screen: **symbol lookup error: amt: undefined symbol: rl_clear_history**

**Cause for ONE:** This is caused by a older GNU Readline library not
having the newer function call *rl_clear_history()* that is used by
the application. The `amt` application was probably compiled on a
system with the newer library, but is now being used an a system that
has an older version Readline shared library installed. The use of
this function newer call *rl_clear_history()* replaced the older
*clear_history()* because it additionally frees private data Readline
saves in the history list.

**Fix for ONE:** Currently one fix is to upgrade the Readline library
  on the system that `amt` is being used on. Use GNU Readline version
  6.3 or newer, as this function call was added in version 6.3. A work
  around from a development perspective to check for this should be
  added in the future.


## Licenses

The following licenses apply to the `amt` source code, and resulting built
application, as described below.

#### License for 'amt'

This program `amt` is licensed under the **MIT License** see
http://opensource.org/licenses/mit for more details.

#### License for 'SQLite'

The SQLite database code used in this application is licensed as **Public
Domain**, see http://www.sqlite.org/copyright.html for more details.

#### License for 'GNU Readline'

The GNU Readline Library used in this application is licensed under the **GNU
GPL v3**, see https://www.gnu.org/licenses/ for more details. 

### License Limitations

**PLEASE NOTE**: Any *distributed compiled binary* versions of `amt` will
include GNU Readline functionality, and these specific binary versions will
therefore be 'infected' by the GPL license. In these circumstances, if the
binary is onward distributed (ie shared with another party), it should
therefore be licensed as 'GNU GPL v3'. For most users of other GPL licensed
software (such as GNU Linux distributions), this will not be an issue or
concern.

The `amt` source code remains completely free under a the **MIT License**
however. The impact of the Readline GNU GPL license creates a *developer
limitation of freedom*, which can be circumvented by perhaps using an
alternative to the GNU Readline library, such as:

- [http://thrysoee.dk/editline/](EditLine) 
- [https://github.com/antirez/linenoise](Linenoise)

These two example alternative libraries are **not** GNU GPL licensed libraries.
These options provide an alternative *choice*, should you want to invoke it.
