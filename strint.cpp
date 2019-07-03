#include "strint.h"
#include "string.h"

StrIntClass::StrIntClass(char *s, int i) {
    str = s;
    ival = i;
}

StrIntClass::~StrIntClass() {
}

int StrIntClass::getInt() {
    return(ival);
}

char * StrIntClass::getStr() {
    return(str);
}

int StrIntClass::operator==(StrIntClass& val) {
    if ((val.ival == ival) && (strcmp(val.str, str) == 0)) {
        return(1);
    } else {
        return(0);
    }
}

int StrIntClass::operator>(StrIntClass& val) {
    if ( (ival > val.ival) ) {
        return(1);
    } else {
        return(0);
    }
}

std::ostream& operator<<(std::ostream& s, StrIntClass& val) {
    s << "(" << val.ival << "," << val.str << ")";
    return(s);
}
