#ifndef INTINT_H
#define INTINT_H

#include <iostream>

std::ostream& operator<<(std::ostream& s, class IntIntClass& val);

class IntIntClass {
    public:
        IntIntClass(int i0 = 0, int i1 = 0);
        ~IntIntClass();
        int operator==(IntIntClass& val);
        int operator>(IntIntClass& val);
        friend std::ostream& operator<<(std::ostream& s, IntIntClass& val);
        int    getInt(int i);
        int    x();
        int    y();
    private:
        int ival0, ival1;
};

#endif
