#include "intint.h"

IntIntClass::IntIntClass(int i0, int i1) {
    ival0 = i0;
    ival1 = i1;
}

IntIntClass::~IntIntClass() {
}

int IntIntClass::getInt(int i) {
    if (i==0) {
        return(ival0);
    } else {
        return(ival1);
    }
}

int IntIntClass::x() {
    return(ival0);
}

int IntIntClass::y() {
    return(ival1);
}

int IntIntClass::operator==(IntIntClass& val) {
    if ((val.ival0 == ival0) && (val.ival1 == ival1)) {
        return(1);
    } else {
        return(0);
    }
}

int IntIntClass::operator>(IntIntClass& val) {
    if ( (ival0 > val.ival0) || ( (ival0 == val.ival0) && (ival1 > val.ival1) ) ) {
        return(1);
    } else {
        return(0);
    }
}

std::ostream& operator<<(std::ostream& s, IntIntClass& val) {
    s << "(" << val.ival0 << "," << val.ival1 << ")";
    return(s);
}
