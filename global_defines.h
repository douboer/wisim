/******************************************************************************************/
/**** FILE: global_defines.h                                                            ****/
/**** Michael Mandell 2006.06.16                                                       ****/
/******************************************************************************************/

#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#define MAX_LINE_SIZE            50000 /* Max num chars in input line                     */
#define CHDELIM                " \t\n" /* Delimiting characters, used for string parsing  */
#define PI             3.1415926535897932384626433832795029

#define IVECTOR(nn) (int    *) ( (nn) ? malloc((nn)    *sizeof(int)   ) : NULL )
#define DVECTOR(nn) (double *) ( (nn) ? malloc((nn)    *sizeof(double)) : NULL )
#define CVECTOR(nn) (char   *) ( (nn) ? malloc(((nn)+1)*sizeof(char)  ) : NULL )

#define CORE_DUMP      printf("%d", *((int *) NULL))
/******************************************************************************************/

#define BITWIDTH(www, nnn) {                  \
    int tmp;                                  \
    tmp = (nnn);                              \
    www = 0;                                  \
    while (tmp) {                             \
        www++;                                \
        tmp>>=1;                              \
    }                                         \
}

#define MOD(m, n)  ( (m) >= 0 ? (m)%(n) : ((n)-1) - ((-(m)-1)%(n)) )

#define DIV(m, n) (                                      \
    ((m) >= 0)&&((n) >= 0) ? (m)/(n) :                   \
    ((m) <  0)&&((n) <  0) ? (-(m))/(-(n)) :             \
    ((m) <  0)&&((n) >= 0) ? -((-(m)-1)/(n) + 1) :       \
                             -(((m)-1)/(-(n)) + 1) )

#ifdef __linux__
#define LONGLONG_TYPE long long
#define FPATH_SEPARATOR '/'
#else
#define LONGLONG_TYPE __int64
#define FPATH_SEPARATOR '\\'
#endif

#endif
