/******************************************************************************************/
/**** FILE: binint.cpp                                                                 ****/
/**** Michael Mandell 11/04/03                                                         ****/
/******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "binint.h"

/******************************************************************************************/
/**** FUNCTION: BinIntClass::BinIntClass                                               ****/
/******************************************************************************************/
BinIntClass::BinIntClass(int n)
{
    // printf("Creating BinIntClass\n");
    num_seg = n;
    val = (SEG_T *) malloc(num_seg*sizeof(SEG_T));
    memset((void *) val, 0, num_seg*BYTES_PER_SEG);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::~BinIntClass                                              ****/
/******************************************************************************************/
BinIntClass::~BinIntClass()
{
    // printf("Deleting BinIntClass\n");
    free(val);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::getval                                                    ****/
/******************************************************************************************/
SEG_T *BinIntClass::getval() { return(val); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::BinIntClass                                               ****/
/******************************************************************************************/
void BinIntClass::setval(unsigned char *src)
{
    int i, j, k;

    k=0;
    for (i=0; i<=(int) num_seg-1; i++) {
        val[i] = 0;
        for (j=0; j<=BYTES_PER_SEG-1; j++) {
            val[i] |= ((SEG_T) src[k]) << (8*j);
            k++;
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::writebytestr                                              ****/
/**** Write number to a string byte-by-byte, low order byte first and high order byte  ****/
/**** last.                                                                            ****/
/******************************************************************************************/
void BinIntClass::writebytestr(unsigned char *src)
{
    int i, j, k;
    SEG_T seg_val;
    k = 0;
    for (i=0; i<=(int) num_seg-1; i++) {
        seg_val = val[i];
        for (j=0; j<=BYTES_PER_SEG-1; j++) {
            src[k] = (seg_val>>(8*j))&0xFF;
            k++;
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::exp_mod                                                   ****/
/**** Computes a^p mod n                                                               ****/
/******************************************************************************************/
void BinIntClass::exp_mod(BinIntClass &a, BinIntClass &p, BinIntClass &n, int n_msb, BinIntClass **f)
{
    BinIntClass t1(num_seg);
    BinIntClass prod(num_seg);
    prod.val[0] = 1;

    int i, j, seg_val, bit_val, found;
    int ms_seg = -1;
    found = 0;
    for (i = num_seg-1; (i>=0)&&(!found); i--) {
        if (p.val[i]) {
            ms_seg = i;
            found = 1;
        }
    }
    for (i = ms_seg; i>=0; i--) {
        seg_val = p.val[i];
        for (j = BITS_PER_SEG-1; j>=0; j--) {
            bit_val = (seg_val >> j) & (0x01);
            t1.mult_mod(prod, prod, n, n_msb, f);
            if (bit_val) {
                prod.mult_mod(t1, a, n, n_msb, f);
            } else {
                memcpy(prod.val, t1.val, num_seg*BYTES_PER_SEG);
            }
        }
    }
    memcpy(val, prod.val, num_seg*BYTES_PER_SEG);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::add                                                       ****/
/**** Computes a + b*2^(BITS_PER_SEG*p)                                                ****/
/******************************************************************************************/
void BinIntClass::add(const BinIntClass &a, const BinIntClass &b, int p)
{
    if (a.num_seg != num_seg) {
        exit(1);
    }

    if (p && (a.val != val) ) {
        memcpy((void *) val, (void *) a.val, p*BYTES_PER_SEG);
    }

    int i;
    int carry = 0;
    D_SEG_T sum;
    for (i = 0; i<=(int) num_seg-p-1; i++) {
        if (i <= (int) b.num_seg-1) {
            sum = (D_SEG_T) a.val[p+i] + b.val[i] + carry;
        } else {
            sum = (D_SEG_T) a.val[p+i] + carry;
        }
        val[p+i] = (SEG_T) ( sum & SEG_MASK );
        carry    = (int)   ( sum >> BITS_PER_SEG );
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::sub                                                       ****/
/**** Computes a - b                                                                   ****/
/******************************************************************************************/
void BinIntClass::sub(const BinIntClass &a, const BinIntClass &b)
{
    if ( (a.num_seg != num_seg) || (b.num_seg != num_seg) ) {
        exit(1);
    }
    BinIntClass tmp(num_seg);

    tmp.tc(b);
    add(a, tmp);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::incrementby                                               ****/
/**** Increments by incr where incr in [0,2^BITS_PER_SEG-1]                            ****/
/******************************************************************************************/
void BinIntClass::incrementby(SEG_T incr)
{
    int i;
    SEG_T carry = incr;
    D_SEG_T sum;
    for (i = 0; i<=(int) num_seg-1; i++) {
        sum = (D_SEG_T) val[i] + carry;
        val[i] = (SEG_T) ( sum & SEG_MASK );
        carry  = (SEG_T) ( sum >> BITS_PER_SEG );
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::left_bit_shift                                            ****/
/**** Bit shift to effectively multiply by 2, rb is shifted in on the right            ****/
/******************************************************************************************/
void BinIntClass::left_bit_shift(int rb)
{
    int i, lb;
    for (i = 0; i<=(int) num_seg-1; i++) {
        lb  = (val[i] >> (BITS_PER_SEG-1)) & 0x01;
        val[i] = ( (val[i]<<1) | (rb & 0x01) ) & SEG_MASK;
        rb = lb;
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::left_seg_shift                                            ****/
/**** Segment shift to effectively multiply by 2^BITS_PER_SEG                          ****/
/******************************************************************************************/
void BinIntClass::left_seg_shift(SEG_T rs)
{
    memmove((void *) (val + 1), (const void *) val, BYTES_PER_SEG*(num_seg-1));
    val[0] = rs;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::tc                                                        ****/
/**** Computes twos-complement of a = 2^(num_bits-1) - a                               ****/
/******************************************************************************************/
void BinIntClass::tc(const BinIntClass &a)
{
    if ( (a.num_seg != num_seg) ) {
        exit(1);
    }

    int i;
    for (i = 0; i<=(int) num_seg-1; i++) {
        val[i] = ~a.val[i];
    }

    incrementby((SEG_T) 1);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::ge                                                        ****/
/**** Determines whether of not a >= b                                                 ****/
/******************************************************************************************/
int BinIntClass::ge(const BinIntClass &a, const BinIntClass &b)
{
    int i, min_num_seg;

    if ( (a.num_seg > b.num_seg) ) {
        for (i = (int) a.num_seg-1; i>=(int) b.num_seg; i--) {
            if (a.val[i]) {
                return(1);
            }
        }
        min_num_seg = b.num_seg;
    } else {
        for (i = (int) b.num_seg-1; i>=(int) a.num_seg; i--) {
            if (b.val[i]) {
                return(0);
            }
        }
        min_num_seg = a.num_seg;
    }

    for (i = min_num_seg-1; i>=0; i--) {
        if (a.val[i] > b.val[i]) {
            return(1);
        } else if (a.val[i] < b.val[i]) {
            return(0);
        }
    }

    return(1);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::mod_b1                                                    ****/
/**** Computes x mod n when it is already known that x in [0,2*n-1]                    ****/
/******************************************************************************************/
void BinIntClass::mod_b1(const BinIntClass &x, const BinIntClass &n)
{
    if (ge(x, n)) {
        sub(x, n);
    } else if (val != x.val) {
        memcpy((void *) val, (const void *) x.val, num_seg*BYTES_PER_SEG);
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::mod                                                       ****/
/**** Fast computation of x mod n.                                                     ****/
/**** n_msb = position of MSB of n                                                     ****/
/**** f[k]  = 2 ^ (n_msb + k + 1) mod n, for k = 0,1,...,7                             ****/
/******************************************************************************************/
void BinIntClass::mod(BinIntClass &x, BinIntClass &n, int n_msb, BinIntClass **f)
{
    int r = -1;
    int r_byte_num, r_bit_num, u, p, k;
    int x_num_seg;

    if (x.val != val) {
        memcpy((void *) val, (const void *) x.val, num_seg*BYTES_PER_SEG);
    }

    /**************************************************************************************/
    /**** Find x_msb, set r = msb of x                                                 ****/
    /**************************************************************************************/
    int i, j, found;

    found = 0;
    for (i = x.num_seg-1; (i>=0)&&(!found); i--) {
        if (x.val[i]) {
            for (j=BITS_PER_SEG-1; (j>=0)&&(!found); j--) {
                if ((x.val[i] >> j) & 0x01) {
                    r = (i<<LOG_BITS_PER_SEG) | j;
                    found = 1;
                }
            }
        }
    }

    if (!found) {
        return;
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Perform modulo n reduction                                                   ****/
    /**************************************************************************************/
    while (r > n_msb) {
        r_byte_num = r >> LOG_BITS_PER_SEG;
        r_bit_num  = r & ((1<<LOG_BITS_PER_SEG)-1);
        if ( (val[r_byte_num] >> r_bit_num) & 0x01 ) {
            u = r - n_msb - 1;
            p = u >> LOG_BITS_PER_SEG;
            k = u & ((1<<LOG_BITS_PER_SEG)-1);
            val[r_byte_num] = val[r_byte_num] & (~( ((SEG_T) 0x01) << r_bit_num));
            add(*this, *f[k], p);
        } else {
            r--;
        }
    }
    /**************************************************************************************/


    if (ge(*this, n)) {
        x_num_seg = num_seg;
        num_seg   = n.num_seg;
        sub(*this, n);
        num_seg   = x_num_seg;
    }

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::mod                                                       ****/
/**** Computes x mod n for general case                                                ****/
/******************************************************************************************/
void BinIntClass::mod(BinIntClass &x, BinIntClass &n)
{
#if 0
    int i, j, byte_val, bit_val;
    for (i = byte_len-1; i>=0; i--) {
        byte_val = x.val[i];
        for (j = 7; j>=0; j--) {
            bit_val = (byte_val >> j) & (0x01);
            tmp.left_bit_shift(bit_val);
            mod_b1(tmp, n);
            memcpy((void *) tmp.val, (const void *) val, num_seg*BYTES_PER_SEG);
        }
    }
#elif 1
    int i, j, found;
    int msb_seg_posn_n = -1;
    int msb_seg_posn_x = -1;
    int byte_val, bit_val;

    found = 0;
    for (i = n.num_seg-1; (i>=0)&&(!found); i--) {
        if (n.val[i]) {
            msb_seg_posn_n = i;
            found = 1;
        }
    }

    found = 0;
    for (i = x.num_seg-1; (i>=0)&&(!found); i--) {
        if (x.val[i]) {
            msb_seg_posn_x = i;
            found = 1;
        }
    }

    if ((msb_seg_posn_x < msb_seg_posn_n) || (!found)) {
        memcpy((void *) val, (const void *) x.val, num_seg*BYTES_PER_SEG);
        return;
    }


    memcpy((void *) val, (void *) (x.val + msb_seg_posn_x - msb_seg_posn_n + 1), msb_seg_posn_n*BYTES_PER_SEG);
    memset((void *) (val + msb_seg_posn_n), 0, (num_seg - msb_seg_posn_n)*BYTES_PER_SEG);

    for (i = msb_seg_posn_x-msb_seg_posn_n; i>=0; i--) {
        byte_val = x.val[i];
        for (j = BITS_PER_SEG-1; j>=0; j--) {
            bit_val = (byte_val >> j) & (0x01);
            left_bit_shift(bit_val);
            mod_b1(*this, n);
        }
    }

#else
    memcpy((void *) val, (const void *) x.val, num_seg*BYTES_PER_SEG);
    while (ge(*this, n)) {
        tmp.sub(*this, n);
        memcpy((void *) val, (const void *) tmp.val, num_seg*BYTES_PER_SEG);
    }
#endif

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::mult_mod                                                  ****/
/**** Computes a*b mod n where b in [0,2^BITS_PER_SEG-1]                               ****/
/******************************************************************************************/
void BinIntClass::mult_mod(BinIntClass &a, SEG_T b, BinIntClass &n, int n_msb, BinIntClass **f)
{
    BinIntClass tmp(num_seg);

    int i;
    SEG_T carry = 0;
    D_SEG_T sum;
    for (i = 0; i<=(int) num_seg-1; i++) {
        sum = (D_SEG_T) a.val[i] * b + carry;
        tmp.val[i] = (SEG_T) ( sum & SEG_MASK );
        carry      = (SEG_T) ( sum >> BITS_PER_SEG );
    }

    mod(tmp, n, n_msb, f);
}

/******************************************************************************************/

#if 1
/******************************************************************************************/
/**** FUNCTION: BinIntClass::mult_mod                                                  ****/
/**** Computes a*b mod n                                                               ****/
/******************************************************************************************/
void BinIntClass::mult_mod(BinIntClass &a, BinIntClass &b, BinIntClass &n, int n_msb, BinIntClass **f)
{
    BinIntClass t2(num_seg);
    BinIntClass prod(2*num_seg);

    memset(prod.val, 0, 2*num_seg*BYTES_PER_SEG);

    int i, j;
    SEG_T carry;
    D_SEG_T sum;
    for (i = 0; i<=(int) num_seg-1; i++) {
        carry = 0;
        for (j = 0; j<=(int) num_seg-1; j++) {
            sum = (D_SEG_T) a.val[i] * b.val[j] + carry + prod.val[i+j];
            prod.val[i+j] = (SEG_T) ( sum & SEG_MASK );
            carry         = (SEG_T) ( sum >> BITS_PER_SEG );
        }
        prod.val[i+num_seg] += carry;
    }
    prod.mod(prod, n, n_msb, f);
    memcpy((void *) val, (void *) prod.val, num_seg*BYTES_PER_SEG);
}
/******************************************************************************************/
#else
/******************************************************************************************/
/**** FUNCTION: BinIntClass::mult_mod                                                  ****/
/**** Computes a*b mod n                                                               ****/
/******************************************************************************************/
void BinIntClass::mult_mod(BinIntClass &a, BinIntClass &b, BinIntClass &n, int n_msb, BinIntClass **f)
{
    BinIntClass t2(num_seg);

    memset(val, 0, num_seg*BYTES_PER_SEG);

    int i;
    for (i = (int) num_seg-1; i>=0; i--) {
        memmove((void *) (val + 1), (const void *) val, (num_seg-1)*BYTES_PER_SEG);
        val[0] = 0;
        if (b.val[i]) {
            t2.mult_mod(a, b.val[i], n, n_msb, f);
            add(*this, t2);
        }
        mod(*this, n, n_msb, f);
    }
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: BinIntClass::printbin                                                  ****/
/**** Prints binary representation of number, MSB first and LSB last                   ****/
/******************************************************************************************/
void BinIntClass::printbin()
{
    int i, j, bit_val;
    SEG_T seg_val;
    for (i = num_seg-1; i>=0; i--) {
        seg_val = val[i];
        for (j = BITS_PER_SEG-1; j>=0; j--) {
            bit_val = (seg_val >> j) & (0x01);
            printf("%d", bit_val);
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::printhex                                                  ****/
/**** Prints HEX representation of number, low order byte first and high order byte    ****/
/**** last.                                                                            ****/
/******************************************************************************************/
void BinIntClass::printhex()
{
    int i, j, k;
    SEG_T seg_val;
    k = 0;
    for (i=0; i<=(int) num_seg-1; i++) {
        seg_val = val[i];
        for (j=0; j<=BYTES_PER_SEG-1; j++) {
            printf("%.2X%c", (seg_val>>(8*j))&0xFF, (k % 10 == 9 ? '\n' : ' '));
            k++;
        }
    }
    printf("\n");
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: BinIntClass::comp_f                                                    ****/
/**** Prints binary representation of number, MSB first and LSB last                   ****/
/******************************************************************************************/
void BinIntClass::comp_f(int &msb, BinIntClass **f)
{
    int i, j, found;
    int q, q_seg_num, q_bit_num;
    /**************************************************************************************/
    /**** Find msb                                                                     ****/
    /**************************************************************************************/
    found = 0;
    for (i = num_seg-1; (i>=0)&&(!found); i--) {
        if (val[i]) {
            for (j=BITS_PER_SEG-1; (j>=0)&&(!found); j--) {
                if ((val[i] >> j) & 0x01) {
                    msb = (i<<LOG_BITS_PER_SEG) | j;
                    found = 1;
                }
            }
        }
    }

    if (!found) {
        printf("ERROR: n cannot be set to zero\n");
        exit(1);
    }
    /**************************************************************************************/

    q = msb + 1;
    q_seg_num  = q >> LOG_BITS_PER_SEG;
    q_bit_num  = q & ((1<<LOG_BITS_PER_SEG)-1);
    f[0]->val[q_seg_num] = 1<<q_bit_num;
    f[0]->mod_b1(*f[0], *this);
    for (i=1; i<=BITS_PER_SEG-1; i++) {
        memcpy((void *) f[i]->val, (const void *) f[i-1]->val, num_seg*BYTES_PER_SEG);
        f[i]->left_bit_shift(0);
        f[i]->mod_b1(*f[i], *this);
    }
}
/******************************************************************************************/
