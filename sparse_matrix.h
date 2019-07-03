/******************************************************************************************/
/**** FILE: sparse_matrix.h                                                            ****/
/******************************************************************************************/

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

class DoubleIntIntClass;

template<class T> class ListClass;

/******************************************************************************************/
/**** CLASS: SparseMatrixClass                                                         ****/
/******************************************************************************************/
class SparseMatrixClass
{
public:
    SparseMatrixClass(int ni, int nj);
    ~SparseMatrixClass();
    void create_col_list();
    void dump();

    ListClass<DoubleIntIntClass> *a;
    ListClass<int> *col_list;

    int num_row, num_col;
};
/******************************************************************************************/

#endif
