/******************************************************************************************/
/**** FILE: prop_model.cpp                                                             ****/
/******************************************************************************************/

#include <math.h>
#include <string.h>
#include <time.h>

#include "cconst.h"
#include "WiSim.h"
#include "rd_prop_model.h"

/*
*******************************************************************************************
* RdPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: RdPropModelClass::RdPropModelClass                                     ****/
/******************************************************************************************/
RdPropModelClass::RdPropModelClass(char *p_strid) : PropModelClass(p_strid)
{
    /* set default values */
    exponent = 3.0;
    coefficient = 0.3125e-3;
}

/******************************************************************************************/
/**** FUNCTION: RdPropModelClass::prop_power_loss                                      ****/
/******************************************************************************************/
double RdPropModelClass::prop_power_loss(NetworkClass *, SectorClass *sector, double delta_x, double delta_y)
{
    double rsq = delta_x*delta_x + delta_y*delta_y;
    if (rsq == 0.0) {
        rsq = 1.0;
    }

    double rel_height_sq = (sector->antenna_height / 25.0) * (sector->antenna_height / 25.0);
    double prop = coefficient * exp(-(exponent/2)*log(rsq))*rel_height_sq;

    return(prop);
}
/******************************************************************************************/
/**** FUNCTION: RdPropModelClass::type                                                 ****/
/******************************************************************************************/
const int RdPropModelClass::type() { return CConst::PropRd; }
/******************************************************************************************/
