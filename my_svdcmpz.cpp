/******************************************************************************************/
/**** Modified to use zero-based vectors                                               ****/
/******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void jacobiz(double **a, int n, double *d, double **v, int *nrot);

void my_svdcmpz(double **a, int m, int n, double *w, double **v)
{
    int n1, n2, k, nrot, i;
    double **mx_ata, **mx_a;

#if 0
    printf("IN FUNCTION: my_svdcmpz()");
    printf("M = %d\n", m);
    printf("N = %d\n", n);
    for (k=0; k<=m-1; k++) {
        for (n1=0; n1<=n-1; n1++) {
            printf("A[%d][%d] = %12.10f\n", k, n1, a[k][n1]);
        }
    }
#endif

    mx_ata = (double **) malloc(n*sizeof(double *));
    for (i=0; i<=n-1; i++) {
        mx_ata[i] = (double *) malloc(n*sizeof(double));
    }
    mx_a   = (double **) malloc(m*sizeof(double *));
    for (i=0; i<=m-1; i++) {
        mx_a[i] = (double *) malloc(n*sizeof(double));
    }

    for (n1=0; n1<=n-1; n1++) {
        for (n2=0; n2<=n-1; n2++) {
            if (n1 > n2) {
                mx_ata[n1][n2] = mx_ata[n2][n1];
            } else {
                mx_ata[n1][n2] = 0.0;
                for (k=0; k<=m-1; k++) {
                    mx_ata[n1][n2] += a[k][n1]*a[k][n2];
                }
            }
        }
    }

#if 0
    printf("BEFORE CALLING jacobiz():\n");
    for (n1=0; n1<=n-1; n1++) {
        for (n2=0; n2<=n-1; n2++) {
            printf("MX_ATA[%d][%d] = %12.10f\n", n1, n2, mx_ata[n1][n2]);
        }
    }
#endif

    jacobiz(mx_ata, n, w, v, &nrot);

#if 0
    printf("AFTER CALLING jacobiz():\n");
    for (n1=0; n1<=n-1; n1++) {
        printf("W[%d] = %20.15e\n", n1, w[n1]);
    }
    for (n1=0; n1<=n-1; n1++) {
        for (n2=0; n2<=n-1; n2++) {
            printf("V[%d][%d] = %12.10f\n", n1, n2, v[n1][n2]);
        }
    }
#endif

    for (n1=0; n1<=n-1; n1++) {
        if (fabs(w[n1]) < 1.0e-9) {
            w[n1] = 0.0;
        } else {
            w[n1] = sqrt(w[n1]);
        }
    }

    for (n1=0; n1<=m-1; n1++) {
        for (n2=0; n2<=n-1; n2++) {
            mx_a[n1][n2] = a[n1][n2];
        }
    }

    for (n1=0; n1<=m-1; n1++) {
        for (n2=0; n2<=n-1; n2++) {
            a[n1][n2] = 0.0;
            if (!(fabs(w[n2]) < 1.0e-12)) {
                for (k=0; k<=n-1; k++) {
                    a[n1][n2] += mx_a[n1][k]*v[k][n2];
                }
                a[n1][n2] /= w[n2];
            }
        }
    }

#if 0
    printf("IN FUNCTION: my_svdcmpz()");
    printf("AFTER DECOMPOSITION\n");
    for (k=0; k<=m-1; k++) {
        for (n1=0; n1<=n-1; n1++) {
            printf("A[%d][%d] = %12.10f\n", k, n1, a[k][n1]);
        }
    }
#endif

    for (i=0; i<=n-1; i++) {
        free(mx_ata[i]);
    }
    free(mx_ata);

    for (i=0; i<=m-1; i++) {
        free(mx_a[i]);
    }
    free(mx_a);

    return;
}
/******************************************************************************************/
