/******************************************************************************************/
/**** FILE: charstr.cpp                                                                ****/
/**** Michael Mandell 08/03/06                                                         ****/
/******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "charstr.h"
#include "global_fn.h"

/******************************************************************************************/
/**** FUNCTION: CharStrClass::CharStrClass                                           ****/
/******************************************************************************************/
CharStrClass::CharStrClass() { str = (char *) NULL; }
CharStrClass::~CharStrClass() { }
char *CharStrClass::getStr() { return(str); }
void CharStrClass::setStr(char *p_str) { if (str) { free(str); } str = (p_str ? strdup(p_str) : (char *) NULL); }
int CharStrClass::operator==(CharStrClass val) { return((strcmp(str, val.getStr())==0) ? 1 : 0); }
int CharStrClass::operator>(CharStrClass val) { return(stringcmp(str, val.getStr())==1 ? 1 : 0); }
std::ostream& operator<<(std::ostream& s, CharStrClass charStr) { s << charStr.getStr(); return(s); }
/******************************************************************************************/
