/******************************************************************************************/
/**** FILE: posn_scan.h                                                                ****/
/******************************************************************************************/

#ifndef POSN_SCAN_H
#define POSN_SCAN_H

class IntIntIntClass;

template<class T> class ListClass;

/******************************************************************************************/
/**** CLASS: PosnScanClass                                                             ****/
/******************************************************************************************/
class PosnScanClass
{
public:
    PosnScanClass();
    ~PosnScanClass();
    void add_point(int x, int y, int scan);
    void comp_x_list();
    IntIntIntClass *get_closest_pt(int x, int y, int d1, int d2);

    ListClass<IntIntIntClass> *pt_list;
    ListClass<IntIntIntClass> *x_list;
};
/******************************************************************************************/

#endif
