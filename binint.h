/******************************************************************************************/
/**** FILE: binint.h                                                                   ****/
/**** Michael Mandell 11/04/03                                                         ****/
/******************************************************************************************/

#ifndef BININT_H
#define BININT_H

#ifdef __linux__
#include <stdint.h>
#   define SEG_T   uint32_t     /* segment size is 32-bits */
#   define D_SEG_T uint64_t
#else
#include <basetsd.h>
#   define SEG_T   UINT32
#   define D_SEG_T UINT64
#endif

#define BYTES_PER_SEG     4                    /* Number of BYTES per segment is 4        */
#define SEG_MASK          0xFFFFFFFF           /* MASK = 2^BITS_PER_SEG-1                 */
#define LOG_BITS_PER_SEG  5                    /* Number of BITS  per segment is 32 = 2^5 */
#define LOG_BYTES_PER_SEG (LOG_BITS_PER_SEG-3) /* Number of BITS  per segment is 32 = 2^5 */
#define BITS_PER_SEG      (8*BYTES_PER_SEG)    /* Number of BITS  per segment is 32       */

/******************************************************************************************/
/**** CLASS: BinIntClass (Arbitrary length binary integer)                             ****/
/**** Binary integer stored in *val                                                    ****/
/****     val[0]          : Least significant BYTE                                     ****/
/****     val[byte_len-1] : Most  significant BYTE                                     ****/
/**** Thus, numeric value is: N = SUM{ val[i]*256^i }                                  ****/
/******************************************************************************************/
class BinIntClass
{
    public:
        BinIntClass(int n);
        ~BinIntClass();
        void setval(unsigned char *src);
        void writebytestr(unsigned char *src);
        SEG_T *getval();
        void exp_mod(BinIntClass &m, BinIntClass &p, BinIntClass &n, int n_msb, BinIntClass **f);
        void add(const BinIntClass &a, const BinIntClass &b, int p = 0);
        void sub(const BinIntClass &a, const BinIntClass &b);
        void incrementby(SEG_T incr);
        void left_bit_shift(int rb);
        void left_seg_shift(SEG_T rs);
        void tc(const BinIntClass &a);
        void mod_b1(const BinIntClass &x, const BinIntClass &n);
        void mod(BinIntClass &x, BinIntClass &n);
        void mod(BinIntClass &x, BinIntClass &n, int n_msb, BinIntClass **f);
        void mult_mod(BinIntClass &a, BinIntClass &b,  BinIntClass &n, int n_msb, BinIntClass **f);
        void mult_mod(BinIntClass &a, SEG_T b, BinIntClass &n, int n_msb, BinIntClass **f);
        void comp_f(int &msb, BinIntClass **f);
        void printbin();
        void printhex();

        static int  ge(const BinIntClass &a, const BinIntClass &b);
    private:
        SEG_T *val;
        unsigned int num_seg;
};
/******************************************************************************************/

#endif
