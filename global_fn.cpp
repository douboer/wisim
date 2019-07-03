#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#else
#include <direct.h>
#endif

#include "global_defines.h"
#include "list.h"

/******************************************************************************************/
/**** Read a line into string s, return length.  From "C Programming Language" Pg. 29  ****/
/**** Modified to be able to read both DOS and UNIX files.                             ****/
int fgetline(FILE *file, char *s)
{
    int c, i;

    for (i=0; (c=fgetc(file)) != EOF && c != '\n'; i++) {
        s[i] = c;
    }
    if ( (i >= 1) && (s[i-1] == '\r') ) {
        i--;
    }
    if (c == '\n') {
        s[i] = c;
        i++;
    }
    s[i] = '\0';
    return(i);
}
/******************************************************************************************/
/**** Remove double quotes from that surround a string.  Note that spaces are removed  ****/
/**** from the beginning of the string before the first quote and both spaces and <cr> ****/
/**** are removed from the end of the string after the final quote.  If a matching     ****/
/**** pair of quotes is not found the string is left unchanged.                        ****/
/******************************************************************************************/
char *remove_quotes(char *str)
{
#if 1
    int n;
    char *start, *end;
    char *ret_str = str;

    if (str) {
        start = str;
        while( (*start) == ' ' ) { start++; }
        n = strlen(str);
        end = str + n - 1;
        while( (end > start) && (((*end) == ' ') || ((*end) == '\n')) ) { end--; }
        if ( (end > start) && ((*start) == '\"') && ((*end) == '\"') ) {
            ret_str = start+1;
            (*end) = (char) NULL;
        }
    }
#else
    char *chptr, *ret_str;

    ret_str = (char *) NULL;
    if (str) {
        chptr = str;
        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
        if (*chptr) {
            chptr++;
            ret_str = chptr;
        }
        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
        if (*chptr) {
            *chptr = (char) NULL;
        } else {
            ret_str = (char *) NULL;
        }
    }
#endif

    return(ret_str);
}
/******************************************************************************************/
/**** FUNCTION: uniquify_str                                                           ****/
/**** INPUTS: str, n, strlist                                                          ****/
/**** OUTPUTS: possibly modified str                                                   ****/
/**** If str is in strlist, append either "(2)", "(3)" ... to make str unique          ****/
void uniquify_str(char *&str, ListClass<char *> *strlist)
{
    int i, found, num = 1;
    char *tmpstr = CVECTOR(strlen(str) + 2 + 40);

    strcpy(tmpstr, str);

    do {
        found = 0;
        for (i=0; (i<=strlist->getSize()-1)&&(!found); i++) {
            if (strcmp(tmpstr, (*strlist)[i])==0) {
                found = 1;
            }
        }
        if (found) {
            num++;
            sprintf(tmpstr, "%s(%d)", str, num);
        }
    } while (found);

    if (num != 1) {
        free(str);
        str = strdup(tmpstr);
    }

    free(tmpstr);
}
/******************************************************************************************/
/**** FUNCTION: gcd                                                                    ****/
/**** INPUTS: a, b                                                                     ****/
/**** OUTPUTS: gcd(a,b)                                                                ****/
/**** Returns the greatest common divisor of two integers.                             ****/
int gcd(int a, int b)
{   int c;

    a = abs(a); b = abs(b);
    while (a) {
        c = b;
        b = a;
        a = c % a;
    }

    return(b);
}
/******************************************************************************************/
void extended_euclid(int a, int b, int& gcd, int& p1, int& p2)
{
    int next_gcd, next_p1, next_p2;
    int  new_gcd,  new_p1,  new_p2;

    if ( (a<0) || (b<0) ) {
        printf("ERROR in routine extended_euclid():");
        printf(" values must be positive %d, %d\n", a,b);
        CORE_DUMP;
        exit(1);
    }

         gcd = a;      p1 = 1;      p2 = 0;
    next_gcd = b; next_p1 = 0; next_p2 = 1;

    while(next_gcd) {
        new_gcd = gcd % next_gcd;
        new_p1  = p1 - next_p1*(gcd/next_gcd);
        new_p2  = p2 - next_p2*(gcd/next_gcd);
        gcd = next_gcd;
        p1  = next_p1;
        p2  = next_p2;
        next_gcd = new_gcd;
        next_p1  = new_p1;
        next_p2  = new_p2;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: set_current_dir_from_file                                              ****/
/**** INPUTS: filename                                                                 ****/
/**** OUTPUTS:                                                                         ****/
/**** Set current working directory to that of the specified filename.                 ****/
void set_current_dir_from_file(char *filename)
{
    char *path = strdup(filename);
    int n = strlen(filename);
    int found = 0;
    int i;

    for (i=n-1; (i>=0)&&(!found); i--) {
        if ( (path[i] == '\\') || (path[i] == '/') ) {
            found = 1;
            path[i+1] = (char) NULL;
        }
    }

    if (found) {
        if (chdir(path) != 0) {
#if CDEBUG
            CORE_DUMP;
#endif
        }
    }

    free(path);
}
/******************************************************************************************/
int stringcmp(char *s1, char *s2)
{
    int n1 = strlen(s1);
    int n2 = strlen(s2);

    int done = 0;
    int d1, d2;
    int nd1=0, nd2=0;
    int v1, v2, num_eq;
    int retval = 0;
    char c1, c2;

    if ( (n1 == 0) && (n2 == 0) ) {
        return(0);
    } else if (n1 == 0) {
        return(-1);
    } else if (n2 == 0) {
        return(1);
    }

    int i1 = 0;
    int i2 = 0;

    while(!done) {
        c1 = s1[i1];
        d1 = ((c1 >= '0') && (c1 <= '9')) ? 1 : 0;
        if (d1) {
            nd1 = 1;
            while ((i1+nd1<n1) && (s1[i1+nd1] >= '0') && (s1[i1+nd1] <= '9')) {
                nd1++;
            }
        }

        c2 = s2[i2];
        d2 = ((c2 >= '0') && (c2 <= '9')) ? 1 : 0;
        if (d2) {
            nd2 = 1;
            while ((i2+nd2<n2) && (s2[i2+nd2] >= '0') && (s2[i2+nd2] <= '9')) {
                nd2++;
            }
        }

        if ((!d1) && (!d2)) {
            if (c1 < c2) {
                retval = -1;
                done = 1;
            } else if (c1 > c2) {
                retval = 1;
                done = 1;
            } else if ((i1 == n1-1) && (i2 < n2-1)) {
                retval = -1;
                done = 1;
            } else if ((i2 == n2-1) && (i1 < n1-1)) {
                retval = 1;
                done = 1;
            } else if ((i1 == n1-1) && (i2 == n2-1)) {
                retval = 0;
                done = 1;
            } else {
                i1++;
                i2++;
            }
        } else if ((!d1) && (d2)) {
            retval = 1;
            done = 1;
        } else if ((d1) && (!d2)) {
            retval = -1;
            done = 1;
        } else if ((d1) && (d2)) {
            while ((nd1 > nd2)&&(!done)) {
                if (s1[i1] > '0') {
                    retval = 1;
                    done = 1;
                } else {
                    i1++;
                    nd1--;
                }
            }
            while ((nd2 > nd1)&&(!done)) {
                if (s2[i2] > '0') {
                    retval = -1;
                    done = 1;
                } else {
                    i2++;
                    nd2--;
                }
            }
            num_eq = 0;
            while( (!done) && (!num_eq) ) {
                if (nd1==0) {
                    if ((i1 == n1) && (i2 < n2)) {
                        retval = -1;
                        done = 1;
                    } else if ((i2 == n2) && (i1 < n1)) {
                        retval = 1;
                        done = 1;
                    } else if ((i1 == n1) && (i2 == n2)) {
                        retval = 0;
                        done = 1;
                    } else {
                        num_eq = 1;
                    }
                } else {
                    v1 = s1[i1] - '0';
                    v2 = s2[i2] - '0';
                    if (v1 < v2) {
                        retval = -1;
                        done = 1;
                    } else if (v1 > v2) {
                        retval = 1;
                        done = 1;
                    } else {
                        i1++;
                        i2++;
                        nd1--;
                        nd2--;
                    }
                }
            }
        }
    }

    // printf("S1 = \"%s\"   S2 = \"%s\"   RETVAL = %d\n", s1.latin1(), s2.latin1(), retval);

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: lowercase                                                              ****/
/**** Convert string to lowercase.                                                     ****/
/******************************************************************************************/
void lowercase(char *s1)
{
    char *chptr = s1;

    while(*chptr) {
        if ( ((*chptr) >= 'A') && ((*chptr) <= 'Z') ) {
            *chptr += 'a' - 'A';
        }
        chptr++;
    }
}
/******************************************************************************************/
void get_bits(char *str, int n, int num_bits)
{
    int i, bit;

    for (i=num_bits-1; i>=0; i--)
    {   bit = (n >> i)&1;
        str[(num_bits-1) - i] = (bit ? '1' : '0');
    }

    str[num_bits] = '\0';
}
/******************************************************************************************/

