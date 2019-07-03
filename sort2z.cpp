#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************/
/**** FUNCTION: sort2z.cpp                                                             ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Numerical Recipes in C: Modification of sort2.c to sort integer array, while     ****/
/**** making corresponding rearrangements in another integer array.  Overloaded to     ****/
/**** treat double, int                                                                ****/
/**** Modified to use zero-based vectors and translated to C++.                        ****/
/******************************************************************************************/

#define ISWAP(a,b) tempi=(a);(a)=(b);(b)=tempi;
#define DSWAP(a,b) tempd=(a);(a)=(b);(b)=tempd;
#define M 7
#define NSTACK 50

void sort2z(int n, int *arr, int *brr)
{
    int i,ir=n,j,k,l=1;
    int *istack,jstack=0;
    int a,b,tempi;

    istack = (int *) malloc((NSTACK+1)*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                a=arr[j-1];
                b=brr[j-1];
                for (i=j-1;i>=1;i--) {
                    if (arr[i-1] <= a) break;
                    arr[i]=arr[i-1];
                    brr[i]=brr[i-1];
                }
                arr[i]=a;
                brr[i]=b;
            }
            if (!jstack) {
                free(istack);
                return;
            }
            ir=istack[jstack-1];
            l=istack[jstack-2];
            jstack -= 2;
        } else {
            k=(l+ir) >> 1;
            ISWAP(arr[k-1],arr[l])
            ISWAP(brr[k-1],brr[l])
            if (arr[l] > arr[ir-1]) {
                ISWAP(arr[l],arr[ir-1])
                ISWAP(brr[l],brr[ir-1])
            }
            if (arr[l-1] > arr[ir-1]) {
                ISWAP(arr[l-1],arr[ir-1])
                ISWAP(brr[l-1],brr[ir-1])
            }
            if (arr[l] > arr[l-1]) {
                ISWAP(arr[l],arr[l-1])
                ISWAP(brr[l],brr[l-1])
            }
            i=l+1;
            j=ir;
            a=arr[l-1];
            b=brr[l-1];
            for (;;) {
                do i++; while (arr[i-1] < a);
                do j--; while (arr[j-1] > a);
                if (j < i) break;
                ISWAP(arr[i-1],arr[j-1])
                ISWAP(brr[i-1],brr[j-1])
            }
            arr[l-1]=arr[j-1];
            arr[j-1]=a;
            brr[l-1]=brr[j-1];
            brr[j-1]=b;
            jstack += 2;
            if (jstack > NSTACK) {
                printf("NSTACK too small in sort2.");
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

void sort2z(int n, double *arr, int *brr)
{
    int i,ir=n,j,k,l=1;
    int *istack,jstack=0;
    double a, tempd;
    int b, tempi;

    istack = (int *) malloc((NSTACK+1)*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                a=arr[j-1];
                b=brr[j-1];
                for (i=j-1;i>=1;i--) {
                    if (arr[i-1] <= a) break;
                    arr[i]=arr[i-1];
                    brr[i]=brr[i-1];
                }
                arr[i]=a;
                brr[i]=b;
            }
            if (!jstack) {
                free(istack);
                return;
            }
            ir=istack[jstack-1];
            l=istack[jstack-2];
            jstack -= 2;
        } else {
            k=(l+ir) >> 1;
            DSWAP(arr[k-1],arr[l])
            ISWAP(brr[k-1],brr[l])
            if (arr[l] > arr[ir-1]) {
                DSWAP(arr[l],arr[ir-1])
                ISWAP(brr[l],brr[ir-1])
            }
            if (arr[l-1] > arr[ir-1]) {
                DSWAP(arr[l-1],arr[ir-1])
                ISWAP(brr[l-1],brr[ir-1])
            }
            if (arr[l] > arr[l-1]) {
                DSWAP(arr[l],arr[l-1])
                ISWAP(brr[l],brr[l-1])
            }
            i=l+1;
            j=ir;
            a=arr[l-1];
            b=brr[l-1];
            for (;;) {
                do i++; while (arr[i-1] < a);
                do j--; while (arr[j-1] > a);
                if (j < i) break;
                DSWAP(arr[i-1],arr[j-1])
                ISWAP(brr[i-1],brr[j-1])
            }
            arr[l-1]=arr[j-1];
            arr[j-1]=a;
            brr[l-1]=brr[j-1];
            brr[j-1]=b;
            jstack += 2;
            if (jstack > NSTACK) {
                printf("NSTACK too small in sort2.");
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
#undef ISWAP
#undef DSWAP
/******************************************************************************************/
