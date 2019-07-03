/******************************************************************************************/
/**** FILE: global_fn.h                                                                ****/
/**** Michael Mandell 1/15/02                                                          ****/
/******************************************************************************************/

#ifndef GLOBAL_FN_H
#define GLOBAL_FN_H

template<class T> class ListClass;

int gcd(int a, int b);
void extended_euclid(int a, int b, int& gcd, int& p1, int& p2);
int fgetline(FILE *, char *);
char *remove_quotes(char *str);
void uniquify_str(char *&str, ListClass<char *> *strlist);
void set_current_dir_from_file(char *filename);
int stringcmp(char *s1, char *s2);
void lowercase(char *s1);
void get_bits(char *str, int n, int num_bits);

#endif
