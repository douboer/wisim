/******************************************************************************************/
/**** FILE: charstr.h                                                                  ****/
/**** Michael Mandell 08/03/06                                                         ****/
/******************************************************************************************/

#ifndef CHARSTR_H
#define CHARSTR_H

#include <iostream>

std::ostream& operator<<(std::ostream& s, class CharStrClass r);

/******************************************************************************************/
/**** CLASS: CharStrClass                                                              ****/
/******************************************************************************************/
class CharStrClass
{
    public:
        CharStrClass();
        ~CharStrClass();
        char *getStr();
        void setStr(char *p_str);
        int operator==(CharStrClass val);
        int operator>(CharStrClass val);
        int operator<<(CharStrClass val);
        friend std::ostream& operator<<(std::ostream& s, class CharStrClass r);

    private:
        char *str;
};
/******************************************************************************************/

#endif
