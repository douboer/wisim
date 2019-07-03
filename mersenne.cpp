/************************** MERSENNE.CPP ******************** AgF 2001-10-18 *
*  Random Number generator 'Mersenne Twister'                                *
*                                                                            *
*  This random number generator is described in the article by               *
*  M. Matsumoto & T. Nishimura, in:                                          *
*  ACM Transactions on Modeling and Computer Simulation,                     *
*  vol. 8, no. 1, 1998, pp. 3-30.                                            *
*                                                                            *
*  Experts consider this an excellent random number generator.               *
*                                                                            *
* © 2003 A. Fog. GNU General Public License www.gnu.org/copyleft/gpl.html    *
*****************************************************************************/

#include "randomc.h"


void TRandomMersenne::RandomInit(long int seed) {
  // re-seed generator
  unsigned long s = (unsigned long)seed;
  for (mti = 0; mti < MERS_N; mti++) {
    s = s * 29943829 - 1;
    mt[mti] = s;}
  // detect computer architecture
  union {double f; unsigned long i[2];} convert;
  convert.f = 1.0;
  if (convert.i[1] == 0x3FF00000) Architecture = (TRandomMersenne::TArch) R_LITTLE_ENDIAN;
  else if (convert.i[0] == 0x3FF00000) Architecture = (TRandomMersenne::TArch) R_BIG_ENDIAN;
  else Architecture = R_NON_IEEE;}


unsigned long TRandomMersenne::BRandom() {
  // generate 32 random bits
  unsigned long y;

  if (mti >= MERS_N) {
    // generate MERS_N words at one time
    const unsigned long LOWER_MASK = (1LU << MERS_R) - 1; // lower MERS_R bits
    const unsigned long UPPER_MASK = -1L  << MERS_R;      // upper (32 - MERS_R) bits
    int kk, km;
    for (kk=0, km=MERS_M; kk < MERS_N-1; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[km] ^ (y >> 1) ^ (-(signed long)(y & 1) & MERS_A);
      if (++km >= MERS_N) km = 0;}

    y = (mt[MERS_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[MERS_N-1] = mt[MERS_M-1] ^ (y >> 1) ^ (-(signed long)(y & 1) & MERS_A);
    mti = 0;}

  y = mt[mti++];

  // Tempering (May be omitted):
  y ^=  y >> MERS_U;
  y ^= (y << MERS_S) & MERS_B;
  y ^= (y << MERS_T) & MERS_C;
  y ^=  y >> MERS_L;

  // printf("MFK: RANDOMB -> %d\n", y);
  return y;}

double TRandomMersenne::Random() {
  // output random float number in the interval 0 <= x < 1
  union {double f; unsigned long i[2];} convert;
  unsigned long r = BRandom(); // get 32 random bits
  // The fastest way to convert random bits to floating point is as follows:
  // Set the binary exponent of a floating point number to 1+bias and set
  // the mantissa to random bits. This will give a random number in the
  // interval [1,2). Then subtract 1.0 to get a random number in the interval
  // [0,1). This procedure requires that we know how floating point numbers
  // are stored. The storing method is tested in function RandomInit and saved
  // in the variable Architecture. (A PC running Windows or Linux uses
  // LITTLE_ENDIAN architecture).
  switch (Architecture) {
  case R_LITTLE_ENDIAN:
    convert.i[0] =  r << 20;
    convert.i[1] = (r >> 12) | 0x3FF00000;
    return convert.f - 1.0;
  case R_BIG_ENDIAN:
    convert.i[1] =  r << 20;
    convert.i[0] = (r >> 12) | 0x3FF00000;
    return convert.f - 1.0;
  case R_NON_IEEE: default:
  ;}
  // This somewhat slower method works for all architectures, including
  // non-IEEE floating point representation:
  return (double)r * (1./((double)(unsigned long)(-1L)+1.));}

/******************************************************************************************/
/***  FUNCTION: TRandomMersenne::Gaussian                                               ***/
/***  INPUTS:                                                                           ***/
/***  OUTPUTS: p1, p2                                                                   ***/
/***  Returns 2 indep. mean 0 var 1 gaussian r.v.s                                      ***/
void TRandomMersenne::Gaussian(double &n1, double &n2)
{
    double x, y, u, r;

    do
    {   x = 2*Random() - 1.0;
        y = 2*Random() - 1.0;

        u = x*x + y*y; }
    while ( u >= 1.0);

    r = sqrt(-2.0*log(u));
    u = sqrt(u);
    n1 = r*x/u;
    n2 = r*y/u;
}
/******************************************************************************************/

long TRandomMersenne::IRandom(long min, long max) {
  // output random integer in the interval min <= x <= max
  long r;
  r = long((max - min + 1) * Random()) + min; // multiply interval with random and truncate
  if (r > max) r = max;
  if (max < min) return 0x80000000;
  return r;}

