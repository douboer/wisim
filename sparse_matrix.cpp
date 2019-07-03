/******************************************************************************************/
/**** FILE: sparse_matrix.cpp                                                          ****/
/******************************************************************************************/

#include <stdio.h>

#include "doubleintint.h"
#include "intintint.h"
#include "list.h"
#include "sparse_matrix.h"

/******************************************************************************************/
/**** CLASS: SparseMatrixClass::SparseMatrixClass                                      ****/
/******************************************************************************************/
SparseMatrixClass::SparseMatrixClass(int ni, int nj)
{
    num_row = ni;
    num_col = nj;

    a = new ListClass<DoubleIntIntClass>(0);
    col_list = (ListClass<int> *) NULL;
}
/******************************************************************************************/
/**** CLASS: SparseMatrixClass::~SparseMatrixClass                                     ****/
/******************************************************************************************/
SparseMatrixClass::~SparseMatrixClass()
{
    if (col_list) {
        delete col_list;
    }

    delete a;
}
/******************************************************************************************/
/**** CLASS: SparseMatrixClass::create_col_list                                        ****/
/******************************************************************************************/
void SparseMatrixClass::create_col_list()
{
    int i, idx;

    if (col_list) {
        delete col_list;
    }

    ListClass<IntIntIntClass> *ii_list = new ListClass<IntIntIntClass>(a->getSize());

    for (i=0; i<=a->getSize()-1; i++) {
        ii_list->append(IntIntIntClass((*a)[i].getInt(1), (*a)[i].getInt(0), i));
    }
    ii_list->sort();

    col_list = new ListClass<int>(a->getSize());
    for (i=0; i<=ii_list->getSize()-1; i++) {
        col_list->append( (*ii_list)[i].getInt(2));
    }

    delete ii_list;
}
/******************************************************************************************/
/**** CLASS: SparseMatrixClass::dump                                                   ****/
/******************************************************************************************/
void SparseMatrixClass::dump()
{
    int i;
    DoubleIntIntClass mx_elem;

    printf("LIST SIZE: %d\n", a->getSize());
    for (i=0; i<=a->getSize()-1; i++) {
        mx_elem = (*a)[i];
        printf("%3d: %3d %3d %15.12f\n", i, mx_elem.getInt(0), mx_elem.getInt(1), mx_elem.getDouble());
    }

    printf("COL SIZE: %d\n", col_list->getSize());
    for (i=0; i<=col_list->getSize()-1; i++) {
        printf("%3d: %3d\n", i, (*col_list)[i]);
    }
}
/******************************************************************************************/
