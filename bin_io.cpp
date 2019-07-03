/******************************************************************************************/
/**** PROGRAM: bin_io.cpp                                                              ****/
/**** Michael Mandell 2/21/03                                                          ****/
/******************************************************************************************/
/**** Functions for reading and writing binary data to a file stream.                  ****/
/******************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __linux__
#include <netinet/in.h>
#define FINITE(xxx) (finite(xxx))
#else
#include <winsock2.h>
#include <float.h>
#define FINITE(xxx) (_finite(xxx))
#endif

#include "bin_io.h"

/******************************************************************************************/
int read_fs_int(FILE *fs)
{
    int n;

    if (fread(&n, sizeof(int), 1, fs) != 1) {
        printf("Error in routine read_fs_int()\n");
        exit(1);
    }

    n = (int) ntohl((unsigned long) n);

    return(n);
}
/******************************************************************************************/
unsigned int read_fs_uint(FILE *fs)
{
    unsigned int n;

    if (fread(&n, sizeof(unsigned int), 1, fs) != 1) {
        printf("Error in routine read_fs_uint()\n");
        exit(1);
    }

    n = (unsigned int) ntohl((unsigned long) n);

    return(n);
}
/******************************************************************************************/
double read_fs_double(FILE *fs)
{
    double dval;
    char dblstr[8];

    if (fread(dblstr, sizeof(char), 8, fs) != 8) {
        printf("Error in routine read_fs_double()\n");
        exit(1);
    }

    dval = dblstr_to_double(dblstr);

    return(dval);
}
/******************************************************************************************/
unsigned char read_fs_uchar(FILE *fs)
{
    unsigned char n;

    if (fread(&n, sizeof(unsigned char), 1, fs) != 1) {
        printf("Error in routine read_fs_uchar()\n");
        exit(1);
    }

    return(n);
}
/******************************************************************************************/
char *read_fs_str_allocate(FILE *fs)
{
    unsigned int n;
    char *c;

    n = read_fs_uint(fs);

    c = (char *) malloc((n+1)*sizeof(char));

    if (fread(c, sizeof(char), n, fs) != n) {
        printf("Error in routine read_fs_str_allocate()\n");
        exit(1);
    }
    c[n] = (char) NULL;

    return(c);
}
/******************************************************************************************/
int write_fs_int(FILE *fs, int h)
{
    int b;
    int n;

    n = htonl((unsigned long) h);

    b = fwrite((void *) &n, sizeof(int), 1, fs);

    return(b);
}
/******************************************************************************************/
int write_fs_uchar(FILE *fs, unsigned char n)
{
    int b;

    b = fwrite((void *) &n, sizeof(char), 1, fs);

    return(b);
}
/******************************************************************************************/
int write_fs_double(FILE *fs, double dval)
{
    int b;
    char dblstr[8];

    double_to_dblstr(dblstr, dval);

    b = fwrite((void *) dblstr, sizeof(char), 8, fs);

    return(b);
}
/******************************************************************************************/
int write_fs_str(FILE *fs, char *str)
{
    int b;
    int n = strlen(str);

    write_fs_int(fs, n);

    b = fwrite((void *) str, sizeof(char), n, fs);

    return(b);
}
/******************************************************************************************/
// Format of dblstr: (Same as internal storage for linux little endian system on i386)
//
// 7      0 15     8 23    16 31    24 39    32 47    40    51 48
// |......| |......| |......| |......| |......| |......|     |..|
//                                                                S        Sign Bit
//                                                       3  0      10    4
//                                                       |--|      |-----| Exponent + 1022
// xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
//    0        1        2        3        4        5        6        7
//
// if (exponent == -1022) {
//    dval = 0.(52-bit value) x 2^exponent
// } else {
//    dval = 0.1(52-bit value) x 2^exponent
// }
//
// Conversion functions are written to be ARCHITECTURE INDEPENDENT.
/******************************************************************************************/
double dblstr_to_double(char *dblstr)
{
    int i, flag, exponent, abs_expo, is_neg;
    double m, mantissa, expo_mult, dval;

    exponent = ((dblstr[7]&(0x7F))<<4) | ((dblstr[6]&(0xF0))>>4);
    exponent -= 1022;

    if ( (dblstr[7]) & 0x80 ) { /* Number is negative */
        is_neg = 1;
    } else {                    /* Number is positive */
        is_neg = 0;
    }

    if (exponent == 1025) {
        flag = 0;
        for (i=0; (i<=5)&&(!flag); i++) {
            if (dblstr[i]&0xFF) {
                flag = 1;
            }
        }
        if (dblstr[6]&0x0F) {
            flag = 1;
        }
        if (flag) {
            dval = atof("nan");
        } else if (is_neg) {
            dval = atof("-inf");
        } else {
            dval = atof("inf");
        }
        return(dval);
    }

    mantissa = 0.0;

    for (i=0; i<=5; i++) {
        mantissa = (mantissa + (dblstr[i]&0xFF)) / (1<<8);
    }

    mantissa = (mantissa + (dblstr[6]&0x0F)) / (1<<4);

    if (exponent != -1022) {
        mantissa = (mantissa + 1.0) / 2;
    }

    expo_mult = 1.0;
    if (exponent < 0) {
        m = 0.5;
        abs_expo = -exponent;
    } else {
        m = 2.0;
        abs_expo = exponent;
    }

    for (i=10; i>=1; i--) {
        expo_mult = expo_mult*expo_mult;
        if (abs_expo & (1<<i)) {
            expo_mult *= m;
        }
    }
    dval = (mantissa*expo_mult)*expo_mult;

    if (abs_expo & 1) {
        dval *= m;
    }

    if (is_neg) {
        dval = -dval;
    }

    return(dval);
}
/******************************************************************************************/
void double_to_dblstr(char *dblstr, double x)
{
    int i, is_neg, exponent, abs_expo;
    double mantissa, expo_mult, m;

#if 0
    if (x == atof("inf")) {
        for (i=0; i<=5; i++) {
            dblstr[i] = (char) 0x00;
        }
        dblstr[6] = (char) 0xF0;
        dblstr[7] = (char) 0x7F;
    } else if (x == atof("-inf")) {
        for (i=0; i<=5; i++) {
            dblstr[i] = (char) 0x00;
        }
        dblstr[6] = (char) 0xF0;
        dblstr[7] = (char) 0xFF;
    } else if (isnan(x)) {
        for (i=0; i<=5; i++) {
            dblstr[i] = (char) 0x00;
        }
        dblstr[6] = (char) 0xF8;
        dblstr[7] = (char) 0x7F;
    }
#endif


    if (!FINITE(x)) {
        for (i=0; i<=5; i++) {
            dblstr[i] = (char) 0x00;
        }
        dblstr[6] = (char) 0xF8;
        dblstr[7] = (char) 0x7F;
    } else {
        if (x < 0.0) {
            is_neg = 1;
             x = -x;
        }

        if (x == 0.0) {
            mantissa = 0.0;
            exponent = -1022;
        } else {
            exponent = ((int) floor(log(x)/log(2.0))) + 1;
            if (exponent < -1022) {
                exponent = -1022;
            }

            expo_mult = 1.0;
            if (exponent < 0) {
                m = 2.0;
                abs_expo = -exponent;
            } else {
                m = 0.5;
                abs_expo = exponent;
            }

            for (i=10; i>=0; i--) {
                expo_mult = expo_mult*expo_mult;
                if (abs_expo & (1<<i)) {
                    expo_mult *= m;
                }
            }
            mantissa = x*expo_mult;

            while(mantissa >= 1.0) {
                mantissa *= 0.5;
                exponent++;
            }
            while( (mantissa < 0.5) && (exponent > -1022) ) {
                mantissa *= 2.0;
                exponent--;
            }
        }

        dblstr[7] = (is_neg<<7) | (((exponent + 1022)>>4)&0x7F);
        dblstr[6] = (((exponent + 1022)&0x0F)<<4);
        if (exponent != -1022) {
            mantissa = mantissa*2.0 - 1.0;
        }

        dblstr[6] |= ((int) floor(mantissa*(1<<4)))&0x0F;
        mantissa = mantissa*(1<<4) - floor(mantissa*(1<<4));

        for (i=5; i>=0; i--) {
            dblstr[i] = ((int) floor(mantissa*(1<<8)))&0xFF;
            mantissa = mantissa*(1<<8) - floor(mantissa*(1<<8));
        }
    }

    return;
}

#undef FINITE
/******************************************************************************************/
