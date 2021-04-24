/* Acronym Management Tool (amt): cli-args.h */

#ifndef CLI_ARGS_H_ /* Include guard */
#define CLI_ARGS_H_

extern int argc;
extern char **argv;
extern char *findme;
extern int newrec;
extern int help;
extern char appversion[];
extern int del_rec_id;
extern int update_rec_id;

/*
 *   FUNCTION DECLARATIONS FOR cli-args.c
 */

void get_cli_args(int argc, char **argv);

#endif // CLI_ARGS_H_
