#include <iostream>

std::ostream& operator<<(std::ostream& s, class IntIntIntClass& val);

class IntIntIntClass {
    public:
        IntIntIntClass(int i0 = 0, int i1 = 0, int i2 = 0);
        ~IntIntIntClass();
        int operator==(IntIntIntClass& val);
        int operator>(IntIntIntClass& val);
        friend std::ostream& operator<<(std::ostream& s, IntIntIntClass& val);
        int    getInt(int i);
        int    x();
        int    y();
        int    z();
    private:
        int ival0, ival1, ival2;
};

