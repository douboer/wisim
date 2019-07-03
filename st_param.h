/******************************************************************************************/
/**** FILE: st_param.h                                                                 ****/
/******************************************************************************************/

#ifndef ST_PARAM_H
#define ST_PARAM_H

#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************/
/**** CLASS: STParamClass                                                              ****/
/******************************************************************************************/
class STParamClass
{
public:
    STParamClass(int melcoIdx, int sanyoIdx, const char *p_description);
    ~STParamClass();
    int getSTIdx(int format);
    char *getDescription();

private:
    char *description;
    int *STIdx;
};
/******************************************************************************************/

#endif
