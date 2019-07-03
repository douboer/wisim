#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************/
/**** FUNCTION: isortz.cpp                                                             ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Numerical Recipes in C: Modification of sort.c to sort integer array.            ****/
/**** Modified to use zero-based vectors and translated to C++.                        ****/
/******************************************************************************************/

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define M 7
#define NSTACK 50

void isortz(int n, int *arr)
{
    int i,ir=n,j,k,l=1;
    int jstack=0,*istack;
    int a,temp;

    istack = (int *) malloc(NSTACK*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                a=arr[j-1];
                for (i=j-1;i>=1;i--) {
                    if (arr[i-1] <= a) break;
                    arr[i]=arr[i-1];
                }
                arr[i]=a;
            }
            if (jstack == 0) {
                free(istack);
                return;
            }
            ir=istack[jstack-1];
            l=istack[jstack-2];
            jstack -= 2;
        } else {
            k=(l+ir) >> 1;
            SWAP(arr[k-1],arr[l])
            if (arr[l] > arr[ir-1]) {
                SWAP(arr[l],arr[ir-1])
            }
            if (arr[l-1] > arr[ir-1]) {
                SWAP(arr[l-1],arr[ir-1])
            }
            if (arr[l] > arr[l-1]) {
                SWAP(arr[l],arr[l-1])
            }
            i=l+1;
            j=ir;
            a=arr[l-1];
            for (;;) {
                do i++; while (arr[i-1] < a);
                do j--; while (arr[j-1] > a);
                if (j < i) break;
                SWAP(arr[i-1],arr[j-1]);
            }
            arr[l-1]=arr[j-1];
            arr[j-1]=a;
            jstack += 2;
            if (jstack > NSTACK) {
                printf("NSTACK too small in isort.");
                exit(1);
            }
            if (ir-i+1 >= j-l) {
                istack[jstack-1]=ir;
                istack[jstack-2]=i;
                ir=j-1;
            } else {
                istack[jstack-1]=j-1;
                istack[jstack-2]=l;
                l=i;
            }
        }
    }
}
#undef M
#undef NSTACK
#undef SWAP
/******************************************************************************************/
