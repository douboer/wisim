#include "doubleintint.h"
#include "cconst.h"

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
int                DoubleIntIntClass :: sortby                = CConst::NumericDouble;
/******************************************************************************************/

DoubleIntIntClass::DoubleIntIntClass(double d, int i0, int i1) {
    dval  = d;
    ival0 = i0;
    ival1 = i1;
}

DoubleIntIntClass::~DoubleIntIntClass() {
}

double DoubleIntIntClass::getDouble() {
    return(dval);
}

int DoubleIntIntClass::getInt(int i) {
    if (i==0) {
        return(ival0);
    } else {
        return(ival1);
    }
}

int DoubleIntIntClass::operator==(DoubleIntIntClass& val) {
    if ((val.dval == dval) && (val.ival0 == ival0) && (val.ival1 == ival1)) {
        return(1);
    } else {
        return(0);
    }
}

int DoubleIntIntClass::operator>(DoubleIntIntClass& val) {
    int retval;
    switch(sortby) {
        case CConst::NumericDouble:
            if (dval > val.dval) {
                retval = 1;
            } else {
                retval = 0;
            }
            break;
        case CConst::NumericInt:
            if (ival0 > val.ival0) {
                retval = 1;
            } else if (ival0 < val.ival0) {
                retval = 0;
            } else {
                if (ival1 > val.ival1) {
                    retval = 1;
                } else {
                    retval = 0;
                }
            }
            break;
        default:
            retval = 0;
            break;
    }
    return(retval);
}

std::ostream& operator<<(std::ostream& s, DoubleIntIntClass& val) {
    s << "(" << val.dval << "," << val.ival0 << "," << val.ival1 << ")";
    return(s);
}
