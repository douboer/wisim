/******************************************************************************************/
/**** FILE: posn_scan.cpp                                                              ****/
/******************************************************************************************/

#include "intintint.h"
#include "list.h"
#include "posn_scan.h"

/******************************************************************************************/
/**** CLASS: PosnScanClass::PosnScanClass                                              ****/
/******************************************************************************************/
PosnScanClass::PosnScanClass()
{
    pt_list = new ListClass<IntIntIntClass>(0);
    x_list  = new ListClass<IntIntIntClass>(0);
}
/******************************************************************************************/
/**** CLASS: SparseMatrixClass::~SparseMatrixClass                                     ****/
/******************************************************************************************/
PosnScanClass::~PosnScanClass()
{
    if (pt_list) {
        delete pt_list;
    }

    if (x_list) {
        delete x_list;
    }
}
/******************************************************************************************/
/**** CLASS: PosnScanClass::add_point                                                  ****/
/******************************************************************************************/
void PosnScanClass::add_point(int x, int y, int scan)
{
    pt_list->append(IntIntIntClass(x, y, scan));
}
/******************************************************************************************/
/**** CLASS: PosnScanClass::comp_x_list                                                ****/
/******************************************************************************************/
void PosnScanClass::comp_x_list()
{
    int i, i0, i1;

    pt_list->sort();

    for(i = pt_list->getSize()-1; i>=1; i--) {
        if (    ((*pt_list)[i].x() == (*pt_list)[i-1].x())
             && ((*pt_list)[i].y() == (*pt_list)[i-1].y()) ) {
            pt_list->del_elem_idx(i);
        }
    }

    pt_list->sort();

    x_list->reset();

    i0 = 0;
    while(i0 <= pt_list->getSize()-1) {
        i1 = i0;
        while( (i1 <= pt_list->getSize()-1) && ((*pt_list)[i1].x() == (*pt_list)[i0].x()) ) {
            i1++;
        }
        x_list->append(IntIntIntClass((*pt_list)[i0].x(), i0, i1-1));
        i0 = i1;
    }
}
/******************************************************************************************/
/**** CLASS: PosnScanClass::get_closest_pt                                             ****/
/******************************************************************************************/
IntIntIntClass *PosnScanClass::get_closest_pt(int x, int y, int d0, int d1)
{
    int found_low, found_high, found_mid;
    int i_low, i_mid, i_high;
    int i, i0, i1;
    int i_x, i_x_low, i_x_high;
    int idx, idx_0, idx_1;
    int possible, done;
    int pt_x, pt_y, dsq;
    int closest_idx, closest_dsq;

    int num_x = x_list->getSize();

    if (    ((*x_list)[0].x() > x+d1)
         || ((*x_list)[num_x-1].x() < x-d1) ) {
        return( (IntIntIntClass *) NULL );
    }

    found_low  = 0;
    found_high = 0;
    found_mid  = 0;
    if ((*x_list)[0].x() >= x-d1) {
        found_low = 1;
        found_mid = 1;
        i_low     = 0;
        i_mid     = 0;
    }

    if ((*x_list)[num_x-1].x() <= x+d1) {
        found_high = 1;
        found_mid = 1;
        i_high     = num_x-1;
        i_mid      = num_x-1;
    }

    /**************************************************************************************/
    /**** Find point in range [x-d1,x+d1] (i_mid)                                      ****/
    /**************************************************************************************/
    if (!found_mid) {
        i0 = 0;
        i1 = num_x-1;
        done = 0;

        while (!done) {
            i = (i0 + i1)/2;
            if ((*x_list)[i].x() < x - d1) {
                i0 = i;
            } else if ((*x_list)[i].x() > x + d1) {
                i1 = i;
            } else {
                done  = 1;
                found_mid = 1;
                i_mid = i;
            }

            if (i1 <= i0 + 1) {
                done = 1;
            }
        }
        if (!found_mid) {
            return( (IntIntIntClass *) NULL );
        } else if ((*x_list)[i_mid].x() == x - d1) {
            found_low = 1;
            i_low = i_mid;
        } else if ((*x_list)[i_mid].x() == x + d1) {
            found_high = 1;
            i_high = i_mid;
        }
        
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Find i_low                                                                   ****/
    /**************************************************************************************/
    if (!found_low) {
        i0 = i_mid - ((*x_list)[i_mid].x() - (x - d1)) - 1;
        if (i0 < 0) { i0 = 0; }
        i1 = i_mid;
        done = 0;

        while (!done) {
            i = (i0 + i1)/2;
            if ((*x_list)[i].x() < x - d1) {
                i0 = i;
            } else if ((*x_list)[i].x() > x - d1) {
                i1 = i;
            } else {
                done  = 1;
                found_low = 1;
                i_low = i;
            }

            if (i1 <= i0 + 1) {
                done = 1;
                found_low = 1;
                i_low = i1;
            }
        }
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Find i_high                                                                  ****/
    /**************************************************************************************/
    if (!found_high) {
        i0 = i_mid;
        i1 = i_mid + ( (x + d1) - (*x_list)[i_mid].x() ) + 1;
        if (i1 > num_x-1) { i1 = num_x-1; }
        done = 0;

        while (!done) {
            i = (i0 + i1)/2;
            if ((*x_list)[i].x() < x + d1) {
                i0 = i;
            } else if ((*x_list)[i].x() > x + d1) {
                i1 = i;
            } else {
                done  = 1;
                found_high = 1;
                i_high = i;
            }

            if (i1 <= i0 + 1) {
                done = 1;
                found_high = 1;
                i_high = i0;
            }
        }
    }
    /**************************************************************************************/

    i_x_low  = i_low;
    i_x_high = i_high;
    closest_idx = -1;

    for (i_x=i_x_low; i_x<=i_x_high; i_x++) {
        idx_0 = (*x_list)[i_x].y();
        idx_1 = (*x_list)[i_x].z();
        if (    ((*pt_list)[idx_0].y() > y+d1)
               || ((*pt_list)[idx_1].y() < y-d1) ) {
            possible = 0;
        } else {
            possible = 1;
        }

        if (possible) {
            found_low  = 0;
            found_high = 0;
            found_mid  = 0;
            if ((*pt_list)[idx_0].y() >= y-d1) {
                found_low = 1;
                found_mid = 1;
                i_low     = idx_0;
                i_mid     = idx_0;
            }

            if ((*pt_list)[idx_1].y() <= y+d1) {
                found_high = 1;
                found_mid = 1;
                i_high     = idx_1;
                i_mid      = idx_1;
            }

            /******************************************************************************/
            /**** Find point in range [y-d1,y+d1] (i_mid)                              ****/
            /******************************************************************************/
            if (!found_mid) {
                i0 = idx_0;
                i1 = idx_1;
                done = 0;

                while (!done) {
                    i = (i0 + i1)/2;
                    if ((*pt_list)[i].y() < y - d1) {
                        i0 = i;
                    } else if ((*pt_list)[i].y() > y + d1) {
                        i1 = i;
                    } else {
                        done  = 1;
                        found_mid = 1;
                        i_mid = i;
                    }

                    if (i1 <= i0 + 1) {
                        done = 1;
                    }
                }
                if (!found_mid) {
                    possible = 0;
                } else if ((*pt_list)[i_mid].y() == y - d1) {
                    found_low = 1;
                    i_low = i_mid;
                } else if ((*pt_list)[i_mid].y() == y + d1) {
                    found_high = 1;
                    i_high = i_mid;
                }
            }
            /******************************************************************************/
        }

        if (possible) {
            /******************************************************************************/
            /**** Find i_low                                                           ****/
            /******************************************************************************/
            if (!found_low) {
                i0 = i_mid - ((*pt_list)[i_mid].x() - (x - d1)) - 1;
                if (i0 < idx_0) { i0 = idx_0; }
                i1 = i_mid;
                done = 0;

                while (!done) {
                    i = (i0 + i1)/2;
                    if ((*pt_list)[i].y() < y - d1) {
                        i0 = i;
                    } else if ((*pt_list)[i].y() > y - d1) {
                        i1 = i;
                    } else {
                        done  = 1;
                        found_low = 1;
                        i_low = i;
                    }
        
                    if (i1 <= i0 + 1) {
                        done = 1;
                        found_low = 1;
                        i_low = i1;
                    }
                }
            }
            /******************************************************************************/
        
            /******************************************************************************/
            /**** Find i_high                                                          ****/
            /******************************************************************************/
            if (!found_high) {
                i0 = i_mid;
                i1 = i_mid + ( (x + d1) - (*pt_list)[i_mid].x() ) + 1;
                if (i1 > idx_1) { i1 = idx_1; }
                done = 0;

                while (!done) {
                    i = (i0 + i1)/2;
                    if ((*pt_list)[i].y() < y + d1) {
                        i0 = i;
                    } else if ((*pt_list)[i].y() > y + d1) {
                        i1 = i;
                    } else {
                        done  = 1;
                        found_high = 1;
                        i_high = i;
                    }

                    if (i1 <= i0 + 1) {
                        done = 1;
                        found_high = 1;
                        i_high = i0;
                    }
                }
            }
            /******************************************************************************/

            for (idx=i_low; idx<=i_high; idx++) {
                pt_x = (*pt_list)[idx].x();
                pt_y = (*pt_list)[idx].y();
                dsq = (pt_x - x)*(pt_x - x) + (pt_y - y)*(pt_y - y);
                if ( (closest_idx == -1) || (dsq < closest_dsq) ) {
                    closest_idx = idx;
                    closest_dsq = dsq;
                }
            }
        }
    }

    if (closest_idx == -1) {
        return( (IntIntIntClass *) NULL );
    } else {
        return( &((*pt_list)[closest_idx]) );
    }
}
/******************************************************************************************/
