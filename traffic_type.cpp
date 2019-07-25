/******************************************************************************************/
/**** FILE: traffic_type.cpp                                                           ****/
/**** Michael Mandell 2/6/04                                                           ****/
/******************************************************************************************/

#include <string.h>
#include "cconst.h"
#include "wisim.h"
#include "traffic_type.h"

/******************************************************************************************/
/**** FUNCTION: TrafficTypeClass:: get functions                                       ****/
/******************************************************************************************/
int    TrafficTypeClass::get_color()     { return(color);     }
char * TrafficTypeClass::get_strid()     { return(strid);     }
double TrafficTypeClass::get_mean_time() { return(mean_time); }
double TrafficTypeClass::get_min_time()  { return(min_time);  }
double TrafficTypeClass::get_max_time()  { return(max_time);  }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: TrafficTypeClass::TrafficTypeClass                                     ****/
/******************************************************************************************/
TrafficTypeClass::TrafficTypeClass(char *p_strid)
{
    if (p_strid) {
        strid = strdup(p_strid);
    } else {
        strid = (char *) NULL;
    }

    color = 0x808080;
    duration_dist = CConst::ExpoDist;
    mean_time = 1.0;
    min_time  = 0.5;
    max_time  = 1.5;
}
TrafficTypeClass::~TrafficTypeClass()
{
    if (strid) {
        free(strid);
    }
}
/******************************************************************************************/
/**** FUNCTION: TrafficTypeClass::name                                                 ****/
/******************************************************************************************/
char *TrafficTypeClass::name()
{
    if (strid) {
        return(strid);
    } else {
        return("UNNAMED");
    }
}
/******************************************************************************************/
/**** FUNCTION: TrafficTypeClass::check                                                ****/
/******************************************************************************************/
int TrafficTypeClass::check(NetworkClass *np)
{
    int num_error = 0;

    if (duration_dist == CConst::UnifDist) {
        if (max_time < min_time) {
            sprintf(np->msg, "ERROR: Traffic Type %s MAX_TIME = %12.7f < MIN_TIME = %12.10f\n",
                name(), max_time, min_time);
            PRMSG(stdout, np->msg);
            num_error++;
        } else if (max_time <= 0.0) {
            sprintf(np->msg, "ERROR: Traffic Type %s MAX_TIME = %12.7f <= 0.0\n",
                name(), max_time);
            PRMSG(stdout, np->msg);
            num_error++;
        }
    } else if (duration_dist == CConst::ExpoDist) {
        if (mean_time <= 0.0) {
            sprintf(np->msg, "ERROR: Traffic Type %s MEAN_TIME = %12.7f <= 0.0\n",
                name(), mean_time);
            PRMSG(stdout, np->msg);
            num_error++;
        }
    }

    return(num_error);
}
/******************************************************************************************/
