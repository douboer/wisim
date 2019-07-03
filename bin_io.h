/******************************************************************************************/
/**** PROGRAM: bin_io.h                                                                ****/
/**** Michael Mandell 2/21/03                                                          ****/
/******************************************************************************************/
/**** Functions for reading and writing binary data to a file stream.                  ****/
/******************************************************************************************/

#ifndef BIN_IO_H
#define BIN_IO_H

#include <stdio.h>

int read_fs_int(FILE *fs);
unsigned int read_fs_uint(FILE *fs);
double read_fs_double(FILE *fs);
unsigned char read_fs_uchar(FILE *fs);
char *read_fs_str_allocate(FILE *fs);
double dblstr_to_double(char *dblstr);

int write_fs_int(FILE *fs, int n);
int write_fs_uchar(FILE *fs, unsigned char n);
int write_fs_double(FILE *fs, double dval);
int write_fs_str(FILE *fs, char *str);
void double_to_dblstr(char *dblstr, double dval);

#endif
