#include "intintint.h"

IntIntIntClass::IntIntIntClass(int i0, int i1, int i2) {
    ival0 = i0;
    ival1 = i1;
    ival2 = i2;
}

IntIntIntClass::~IntIntIntClass() {
}

int IntIntIntClass::getInt(int i) {
    if (i==0) {
        return(ival0);
    } else if (i==1) {
        return(ival1);
    } else {
        return(ival2);
    }
}

int IntIntIntClass::x() {
    return(ival0);
}

int IntIntIntClass::y() {
    return(ival1);
}

int IntIntIntClass::z() {
    return(ival2);
}

int IntIntIntClass::operator==(IntIntIntClass& val) {
    if ((val.ival0 == ival0) && (val.ival1 == ival1) && (val.ival2 == ival2)) {
        return(1);
    } else {
        return(0);
    }
}

int IntIntIntClass::operator>(IntIntIntClass& val) {
    if ( (ival0 > val.ival0) || ( (ival0 == val.ival0) && (ival1 > val.ival1) )
                             || ( (ival0 == val.ival0) && (ival1 == val.ival1) && (ival2 > val.ival2) ) ) {
        return(1);
    } else {
        return(0);
    }
}

std::ostream& operator<<(std::ostream& s, IntIntIntClass& val) {
    s << "(" << val.ival0 << "," << val.ival1 << "," << val.ival2 << ")";
    return(s);
}
