/******************************************************************************************/
/**** FILE: list.cpp                                                                   ****/
/**** Michael Mandell 7/30/04                                                          ****/
/******************************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"

#define CORE_DUMP      printf("%d", *((int *) NULL))

/******************************************************************************************/
/**** ListClass::ListClass                                                             ****/
/******************************************************************************************/
template<class T>
ListClass<T>::ListClass(int n, int incr)
{
    a = (T *) malloc(n * sizeof(T));
    allocation_size = n;
    if (incr <= 0) {
        printf("ERROR in routine ListClass::ListClass(): allocation_increment = %d, must be > 0\n", incr);
        CORE_DUMP;
        exit(1);
    }
    allocation_increment = incr;

    size = 0;
};
/******************************************************************************************/

/******************************************************************************************/
/**** ListClass::~ListClass                                                            ****/
/******************************************************************************************/
template<class T>
ListClass<T>::~ListClass()
{
    free(a);
};
/******************************************************************************************/

/******************************************************************************************/
/**** ListClass::operator[]                                                            ****/
/******************************************************************************************/
template<class T>
T& ListClass<T>::operator[] (int index)
{
    return a[index];
};
/******************************************************************************************/

/******************************************************************************************/
/**** ListClass::getSize                                                               ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::getSize()
{
    return(size);
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::ins_elem                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Inserts val into {list[0], list[1], ..., list[n-1]}                              ****/
/**** If val is already in list and err=0 do nothing, else give ERROR and exit         ****/
/**** The value returned is the position of val in the list.                           ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::ins_elem(T val, const int err)
{
    int i, found;
    int retval = -1;

    found = 0;
    for (i=size-1; (i>=0)&&(!found); i--) {
        if (a[i] == val) {
            found = 1;
            retval = i;
        }
    }

    if (found) {
        if (err) {
            printf("ERROR in routine ListClass::ins_elem()\n");
            CORE_DUMP;
            exit(1);
        }
    } else {
        retval = size;
        if ((size == allocation_size)) {
            allocation_size += allocation_increment;
            a = (T *) realloc((void *) a, allocation_size * sizeof(T));
        }
        a[size++] = val;
    }
    return(retval);
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::append                                                      ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Adds val to the end of {list[0], list[1], ..., list[n-1]}                        ****/
/**** There is no check whether or not val is already in the list.                     ****/
/******************************************************************************************/
template<class T>
void ListClass<T>::append(T val)
{
    if ((size == allocation_size)) {
        allocation_size += allocation_increment;
        a = (T *) realloc((void *) a, allocation_size * sizeof(T));
    }

    a[size++] = val;

    return;
};
/******************************************************************************************/
/**** FUNCTION: ListClass::del_elem                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Removes val from {list[0], list[1], ..., list[n-1]}                              ****/
/**** If val is not in list and err=0 do nothing, else give ERROR and exit             ****/
/******************************************************************************************/
template<class T>
void ListClass<T>::del_elem(T val, const int err)
{
    int i;

    for (i=size-1; i>=0; i--) {
        if (a[i] == val) {
            a[i] = a[size-1];
            size--;
            return;
        }
    }

    if (err) {
        printf("ERROR in routine del_elem()\n");
        CORE_DUMP;
        exit(1);
    }
    return;
};

template<class T>
void ListClass<T>::del_elem_idx(int index, const int err)
{
    if ( (index < 0) || (index > size-1) ) {
        if (err) {
            printf("ERROR in routine del_elem_idx(): index out of range, index = %d outside [0,%d]\n", index, size-1);
            CORE_DUMP;
            exit(1);
        } else {
            return;
        }
    }

    a[index] = a[size-1];
    size--;

    return;
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::toggle_elem                                                 ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** If val is not in the list, then add it; and if val is in the list, then remove   ****/
/**** it.                                                                              ****/
/******************************************************************************************/
template<class T>
void ListClass<T>::toggle_elem(T val)
{
    int i;

    for (i=size-1; i>=0; i--) {
        if (a[i] == val) {
            a[i] = a[size-1];
            size--;
            return;
        }
    }

    if ((size == allocation_size)) {
        allocation_size += allocation_increment;
        a = (T *) realloc((void *) a, allocation_size * sizeof(T));
    }

    a[size++] = val;

    return;
};
/******************************************************************************************/

/******************************************************************************************/
/**** ListClass::pop                                                                   ****/
/******************************************************************************************/
template<class T>
T ListClass<T>::pop()
{
    T val;

    if (size) {
        val = a[size-1];
        size--;
    } else {
        printf("ERROR in routine pop(), can't pop list of size zero.\n");
        CORE_DUMP;
        exit(1);
    }

    return val;
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::reset                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Removes all values from the list, sets allocation_size to zero.                  ****/
/******************************************************************************************/
template<class T>
void ListClass<T>::reset()
{
    size = 0;

    if (allocation_size) {
        allocation_size = 0;
        a = (T *) realloc((void *) a, allocation_size * sizeof(T));
    }

    return;
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::resize                                                      ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Resize allocation_size to the specified value, but can't be less than the size   ****/
/**** of the list                                                                      ****/
/******************************************************************************************/
template<class T>
void ListClass<T>::resize(int n)
{
    int new_alloc_size;

    if (n <= size) {
        new_alloc_size = size;
    } else {
        new_alloc_size = n;
    }

    if (allocation_size != new_alloc_size) {
        allocation_size = new_alloc_size;
        a = (T *) realloc((void *) a, allocation_size * sizeof(T));
    }

    return;
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::get_index                                                   ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Given list {list[0], list[1], ..., list[n-1]} and value val, return index i such ****/
/**** that list[i] = val.  If val is not in list and err=0 return -1, else give ERROR  ****/
/**** and exit                                                                         ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::get_index(T val, const int err)
{
    int i, found;
    int retval = -1;

    found = 0;
    for (i=size-1; (i>=0)&&(!found); i--) {
        if (a[i] == val) {
            found = 1;
            retval = i;
        }
    }

    if (!found) {
        if (err) {
            printf("ERROR in routine get_index()\n");
            CORE_DUMP;
            exit(1);
        }
    }

    return(retval);
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::contains                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Given list {list[0], list[1], ..., list[n-1]} and value val, return 1 if val is  ****/
/**** contained in the list and 0 otherwise.                                           ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::contains(T val)
{
    int i, found;

    found = 0;
    for (i=size-1; (i>=0)&&(!found); i--) {
        if (a[i] == val) {
            found = 1;
        }
    }

    return(found);
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::get_idx_sorted                                              ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Given the SORTED list {list[0], list[1], ..., list[n-1]} and value val, return   ****/
/**** index i such that list[i] = val.  If val is not in list and err=0 return -1,     ****/
/**** else give ERROR and exit.                                                        ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::get_index_sorted(T val, const int err)
{
    int i, i_min, i_max, done, found, retval;

    found  = 0;
    done   = 0;
    retval = -1;

    i_min = 0;
    if (size == 0) {
        done  = 1;
    } else if (a[i_min] == val) {
        done  = 1;
        found = 1;
        retval = i_min;
    } else if (a[i_min] > val) {
        done  = 1;
    }

    if (!done) {
        i_max = size-1;
        if (a[i_max] == val) {
            done  = 1;
            found = 1;
            retval = i_max;
        } else if (val > a[i_max]) {
            done  = 1;
        }
    }

    if (!done) {
        if (size <= 2) {
            done  = 1;
        }
    }

    while (!done) {
        i = (i_min + i_max)/2;
        if (a[i] == val) {
            done  = 1;
            found = 1;
            retval = i;
        } else if (val > a[i]) {
            i_min = i;
        } else {
            i_max = i;
        }

        if (i_max <= i_min + 1) {
            done = 1;
        }
    }

    if (!found) {
        if (err) {
            printf("ERROR in routine get_index_sorted()\n");
            CORE_DUMP;
            exit(1);
        }
    }

    return(retval);
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::contains_sorted                                             ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Given the SORTED list {list[0] < list[1] < ... < list[n-1]} and value val,       ****/
/**** return 1 if val is contained in the list and 0 otherwise.                        ****/
/******************************************************************************************/
template<class T>
int ListClass<T>::contains_sorted(T val)
{
#if 1
    if (get_index_sorted(val, 0) == -1) {
        return(0);
    } else {
        return(1);
    }
#else
    int i, i_min, i_max, done, found;

    found = 0;
    done  = 0;

    i_min = 0;
    if (a[i_min] == val) {
        done  = 1;
        found = 1;
    } else if (a[i_min] > val) {
        done  = 1;
    }

    if (!done) {
        i_max = size-1;
        if (a[i_max] == val) {
            done  = 1;
            found = 1;
        } else if (val > a[i_max]) {
            done  = 1;
        }
    }

    if (!done) {
        if (size <= 2) {
            done  = 1;
        }
    }

    while (!done) {
        i = (i_min + i_max)/2;
        if (a[i] == val) {
            done  = 1;
            found = 1;
        } else if (val > a[i]) {
            i_min = i;
        } else {
            i_max = i;
        }

        if (i_max <= i_min + 1) {
            done = 1;
        }
    }

    return(found);
#endif
};
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::printlist                                                   ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Prints values of {list[0], list[1], ..., list[n-1]}                              ****/
template<class T>
void ListClass<T>::printlist()
{
    int i;

    for (i=0; i<=size-1; i++) {
        std::cout << a[i] << ( (i%100 == 99) ? "\n   " : " " );
    }
    std::cout << std::endl;
    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::reverse                                                     ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Reverse the order of all values in the list.                                     ****/
template<class T>
void ListClass<T>::reverse()
{
    int i;
    T tmp;

    for (i=0; i<=(size/2)-1; i++) {
        tmp = a[i];
        a[i] = a[size-1-i];
        a[size-1-i] = tmp;
    }

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::sort                                                        ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Numerical Recipes in C: Modification of sort.c to sort integer array.            ****/
/**** Modified to use zero-based vectors and translated to C++.                        ****/
/******************************************************************************************/
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define M 7
#define NSTACK 50

template<class T>
void ListClass<T>::sort()
{
    int i,ir=size,j,k,l=1;
    int jstack=0,*istack;
    T vara,temp;

    istack = (int *) malloc(NSTACK*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                vara=a[j-1];
                for (i=j-1;i>=1;i--) {
                    if (!(a[i-1] > vara)) break;
                    a[i]=a[i-1];
                }
                a[i]=vara;
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
            SWAP(a[k-1],a[l])
            if (a[l] > a[ir-1]) {
                SWAP(a[l],a[ir-1])
            }
            if (a[l-1] > a[ir-1]) {
                SWAP(a[l-1],a[ir-1])
            }
            if (a[l] > a[l-1]) {
                SWAP(a[l],a[l-1])
            }
            i=l+1;
            j=ir;
            vara=a[l-1];
            for (;;) {
                do i++; while (vara > a[i-1]);
                do j--; while (a[j-1] > vara);
                if (j < i) break;
                SWAP(a[i-1],a[j-1]);
            }
            a[l-1]=a[j-1];
            a[j-1]=vara;
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

/******************************************************************************************/
/**** FUNCTION: ListClass::sort2                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Numerical Recipes in C: Modification of sort2.c to sort integer array, while     ****/
/**** making corresponding rearrangements in another integer array.  Overloaded to     ****/
/**** treat double, int                                                                ****/
/**** Modified to use zero-based vectors and translated to C++.                        ****/
/******************************************************************************************/

#define TSWAP(a,b) tempa=(a);(a)=(b);(b)=tempa;
#define USWAP(a,b) tempb=(a);(a)=(b);(b)=tempb;
#define M 7
#define NSTACK 50

template<class T, class U>
void sort2(ListClass<T> *lc_t, ListClass<U> *lc_u)
{
    int i,j,k,l=1;
    int *istack,jstack=0;
    T vara,tempa;
    U varb,tempb;

    int ir = lc_t->getSize();

    if (lc_u->getSize() != lc_t->getSize()) {
        printf("ERROR in routine ListClass::sort2(), lists are of unequal length\n");
        CORE_DUMP;
        exit(1);
    }

    istack = (int *) malloc((NSTACK+1)*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                vara=(*lc_t)[j-1];
                varb=(*lc_u)[j-1];
                for (i=j-1;i>=1;i--) {
                    if (!((*lc_t)[i-1] > vara)) break;
                    (*lc_t)[i]=(*lc_t)[i-1];
                    (*lc_u)[i]=(*lc_u)[i-1];
                }
                (*lc_t)[i]=vara;
                (*lc_u)[i]=varb;
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
            TSWAP((*lc_t)[k-1],(*lc_t)[l])
            USWAP((*lc_u)[k-1],(*lc_u)[l])
            if ((*lc_t)[l] > (*lc_t)[ir-1]) {
                TSWAP((*lc_t)[l],(*lc_t)[ir-1])
                USWAP((*lc_u)[l],(*lc_u)[ir-1])
            }
            if ((*lc_t)[l-1] > (*lc_t)[ir-1]) {
                TSWAP((*lc_t)[l-1],(*lc_t)[ir-1])
                USWAP((*lc_u)[l-1],(*lc_u)[ir-1])
            }
            if ((*lc_t)[l] > (*lc_t)[l-1]) {
                TSWAP((*lc_t)[l],(*lc_t)[l-1])
                USWAP((*lc_u)[l],(*lc_u)[l-1])
            }
            i=l+1;
            j=ir;
            vara=(*lc_t)[l-1];
            varb=(*lc_u)[l-1];
            for (;;) {
                do i++; while (vara > (*lc_t)[i-1]);
                do j--; while ((*lc_t)[j-1] > vara);
                if (j < i) break;
                TSWAP((*lc_t)[i-1],(*lc_t)[j-1])
                USWAP((*lc_u)[i-1],(*lc_u)[j-1])
            }
            (*lc_t)[l-1]=(*lc_t)[j-1];
            (*lc_t)[j-1]=vara;
            (*lc_u)[l-1]=(*lc_u)[j-1];
            (*lc_u)[j-1]=varb;
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
#undef TSWAP
#undef USWAP
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::sort3                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Numerical Recipes in C: Modification of sort2.c to sort integer array, while     ****/
/**** making corresponding rearrangements in another integer array.                    ****/
/**** Modified to use zero-based vectors and translated to C++.                        ****/
/******************************************************************************************/

#define TSWAP(a,b) tempa=(a);(a)=(b);(b)=tempa;
#define USWAP(a,b) tempb=(a);(a)=(b);(b)=tempb;
#define VSWAP(a,b) tempc=(a);(a)=(b);(b)=tempc;
#define M 7
#define NSTACK 50

template<class T, class U, class V>
void sort3(ListClass<T> *lc_t, ListClass<U> *lc_u, ListClass<V> *lc_v)
{
    int i,j,k,l=1;
    int *istack,jstack=0;
    T vara,tempa;
    U varb,tempb;
    V varc,tempc;

    int ir = lc_t->getSize();

    if ( (lc_u->getSize() != ir) || (lc_v->getSize() != ir) ) {
        printf("ERROR in routine ListClass::sort3(), lists are of unequal length\n");
        CORE_DUMP;
        exit(1);
    }

    istack = (int *) malloc((NSTACK+1)*sizeof(int));
    for (;;) {
        if (ir-l < M) {
            for (j=l+1;j<=ir;j++) {
                vara=(*lc_t)[j-1];
                varb=(*lc_u)[j-1];
                varc=(*lc_v)[j-1];
                for (i=j-1;i>=1;i--) {
                    if (!((*lc_t)[i-1] > vara)) break;
                    (*lc_t)[i]=(*lc_t)[i-1];
                    (*lc_u)[i]=(*lc_u)[i-1];
                    (*lc_v)[i]=(*lc_v)[i-1];
                }
                (*lc_t)[i]=vara;
                (*lc_u)[i]=varb;
                (*lc_v)[i]=varc;
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
            TSWAP((*lc_t)[k-1],(*lc_t)[l])
            USWAP((*lc_u)[k-1],(*lc_u)[l])
            VSWAP((*lc_v)[k-1],(*lc_v)[l])
            if ((*lc_t)[l] > (*lc_t)[ir-1]) {
                TSWAP((*lc_t)[l],(*lc_t)[ir-1])
                USWAP((*lc_u)[l],(*lc_u)[ir-1])
                VSWAP((*lc_v)[l],(*lc_v)[ir-1])
            }
            if ((*lc_t)[l-1] > (*lc_t)[ir-1]) {
                TSWAP((*lc_t)[l-1],(*lc_t)[ir-1])
                USWAP((*lc_u)[l-1],(*lc_u)[ir-1])
                VSWAP((*lc_v)[l-1],(*lc_v)[ir-1])
            }
            if ((*lc_t)[l] > (*lc_t)[l-1]) {
                TSWAP((*lc_t)[l],(*lc_t)[l-1])
                USWAP((*lc_u)[l],(*lc_u)[l-1])
                VSWAP((*lc_v)[l],(*lc_v)[l-1])
            }
            i=l+1;
            j=ir;
            vara=(*lc_t)[l-1];
            varb=(*lc_u)[l-1];
            varc=(*lc_v)[l-1];
            for (;;) {
                do i++; while (vara > (*lc_t)[i-1]);
                do j--; while ((*lc_t)[j-1] > vara);
                if (j < i) break;
                TSWAP((*lc_t)[i-1],(*lc_t)[j-1])
                USWAP((*lc_u)[i-1],(*lc_u)[j-1])
                VSWAP((*lc_v)[i-1],(*lc_v)[j-1])
            }
            (*lc_t)[l-1]=(*lc_t)[j-1];
            (*lc_t)[j-1]=vara;
            (*lc_u)[l-1]=(*lc_u)[j-1];
            (*lc_u)[j-1]=varb;
            (*lc_v)[l-1]=(*lc_v)[j-1];
            (*lc_v)[j-1]=varc;
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
#undef TSWAP
#undef USWAP
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ListClass::ins_pointer                                                 ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Same as ins_elem except values are dereferenced.                                 ****/
/******************************************************************************************/
template<class T>
int ins_pointer(ListClass<T> *lc_t, T& val, const int err)
{
    int i, found;
    int insIdx = -1;

    found = 0;
    for (i=lc_t->getSize()-1; (i>=0)&&(!found); i--) {
        if ( *((*lc_t)[i]) == *val) {
            found = 1;
            insIdx = i;
        }
    }

    if (found) {
        if (err) {
            printf("ERROR in routine ListClass::ins_pointer()\n");
            CORE_DUMP;
            exit(1);
        } else {
            delete val;
            val = (*lc_t)[insIdx];
        }
    } else {
        lc_t->append(val);
        insIdx = lc_t->getSize()-1;
    }

    return(insIdx);
};
/******************************************************************************************/

