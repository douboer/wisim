/******************************************************************************************/
/**** FILE: list.h                                                                     ****/
/**** Michael Mandell 7/30/04                                                          ****/
/******************************************************************************************/

#ifndef LIST_H
#define LIST_H

/******************************************************************************************/
/**** Template Class: ListClass                                                        ****/
/**** Based on template class example from "C++ Inside & Out", pg 528.                 ****/
/******************************************************************************************/
template<class T>
class ListClass
{
    public:
        ListClass(int n, int incr = 10);
        ~ListClass();
        T& operator[] (int index);
        int getSize();
        void append(T val);
        void reset();
        void resize(int n = 0);
        void sort();
        void reverse();
        void del_elem(T val, const int err = 1);
        void del_elem_idx(int index, const int err = 1);
        void toggle_elem(T val);
        T pop();  // push not needed because append does the same thing.

        int ins_elem(T val, const int err = 1);
        int get_index(T val, const int err = 1);
        int contains(T val);

        int get_index_sorted(T val, const int err = 1);
        int contains_sorted(T val);

        void printlist();

    private:
        T *a;
        int size;
        int allocation_size;
        int allocation_increment;
};
/******************************************************************************************/

template<class T, class U>
void sort2(ListClass<T> *lc_t, ListClass<U> *lc_u);

template<class T, class U, class V>
void sort3(ListClass<T> *lc_t, ListClass<U> *lc_u, ListClass<V> *lc_v);

template<class T>
int ins_pointer(ListClass<T> *lc_t, T& val, const int err = 1);

#endif
