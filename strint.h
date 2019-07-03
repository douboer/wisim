#include <iostream>

std::ostream& operator<<(std::ostream& s, class StrIntClass& val);

class StrIntClass {
    public:
        StrIntClass(char *s = (char *) NULL, int i = 0);
        ~StrIntClass();
        int operator==(StrIntClass& val);
        int operator>(StrIntClass& val);
        friend std::ostream& operator<<(std::ostream& s, StrIntClass& val);
        int    getInt();
        char * getStr();
    private:
        char *str;
        int ival;
};

