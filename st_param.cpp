/******************************************************************************************/
/**** FILE: st_param.cpp                                                               ****/
/**** Michael Mandell 8/04/06                                                          ****/
/******************************************************************************************/

#include <string.h>

#include "global_defines.h"
#include "st_param.h"

/******************************************************************************************/
/**** FUNCTION: STParamClass::STParamClass                                             ****/
/******************************************************************************************/
STParamClass::STParamClass(int melcoIdx, int sanyoIdx, const char *p_description)
{
    STIdx = IVECTOR(2);
    STIdx[0] = melcoIdx;
    STIdx[1] = sanyoIdx;
    description = strdup(p_description);
}
/******************************************************************************************/
/**** FUNCTION: STParamClass::~STParamClass                                            ****/
/******************************************************************************************/
STParamClass::~STParamClass()
{
    free(STIdx);
    free(description);
}
/******************************************************************************************/
/**** FUNCTION: STParamClass:: get Functions                                           ****/
/******************************************************************************************/
int   STParamClass::getSTIdx(int format) { return(STIdx[format]); }
char *STParamClass::getDescription()     { return(description);   }
/******************************************************************************************/
