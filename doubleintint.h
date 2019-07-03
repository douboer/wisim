#include <iostream>

std::ostream& operator<<(std::ostream& s, class DoubleIntIntClass& val);

class DoubleIntIntClass {
    public:
        DoubleIntIntClass(double d = 0.0, int i0 = 0, int i1 = 0);
        ~DoubleIntIntClass();
        int operator==(DoubleIntIntClass& val);
        int operator>(DoubleIntIntClass& val);
        friend std::ostream& operator<<(std::ostream& s, DoubleIntIntClass& val);
        double getDouble();
        int    getInt(int i);
        static int sortby;
    private:
        double dval;
        int ival0, ival1;
};

