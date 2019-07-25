/******************************************************************************************/
/**** FILE: prop_model.cpp                                                             ****/
/******************************************************************************************/

#include <math.h>
#include <string.h>
#include <time.h>

#include "cconst.h"
#include "wisim.h"
#include "prop_model.h"

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
int              SegmentPropModelClass :: cutoff_slope      = -25;
int     SegmentWithThetaPropModelClass :: cutoff_slope      = -25;
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: Virtual Functions                                                      ****/
/******************************************************************************************/
const int PropModelClass::type()                                    { CORE_DUMP; return(-1); }
double    PropModelClass::prop_power_loss(NetworkClass *, SectorClass *, int, int, int, double) { CORE_DUMP; return(0.0); }
void      PropModelClass::get_prop_model_param_ptr(int, char *, int&, int *&, double *&) { CORE_DUMP; return; }
int       PropModelClass::comp_num_prop_param()                     { CORE_DUMP; return(-1); }
void      PropModelClass::print_params(FILE *, char *, int)         { CORE_DUMP; return; }
int       PropModelClass::is_clutter_model()                        { return(0); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PropModelClass::PropModelClass                                         ****/
/******************************************************************************************/
PropModelClass::PropModelClass(char *p_strid)
{
    if (p_strid) {
        strid = strdup(p_strid);
    } else {
        strid = (char *) NULL;
    }

    flag = CConst::PropFlagIndividual;
}
/******************************************************************************************/
/**** FUNCTION: PropModelClass::PropModelClass                                         ****/
/******************************************************************************************/
PropModelClass::~PropModelClass()
{
    if (strid) {
        free(strid);
    }
}
/******************************************************************************************/
/**** FUNCTION: PropModelClass:get_strid                                               ****/
/******************************************************************************************/
char *PropModelClass::get_strid()
{
    return(strid);
}
/******************************************************************************************/
/**** FUNCTION: PropModelClass:set_strid                                               ****/
/******************************************************************************************/
void PropModelClass::set_strid(char *str, int allocate)
{
    if (allocate) {
        strid = strdup(str);
    } else {
        strid = str;
    }
}
/******************************************************************************************/
/**** FUNCTION: PropModelClass:report                                                  ****/
/******************************************************************************************/
void PropModelClass::report(NetworkClass *np, FILE *fp)
{
    char *chptr;

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Propagation Model Report                                        ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "NAME: %s\n", strid);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    print_params(fp, np->msg, CConst::reportFileType);

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    PRMSG(fp, np->msg);
}
/******************************************************************************************/


/*
*******************************************************************************************
* ExpoPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::ExpoPropModelClass                                 ****/
/******************************************************************************************/
ExpoPropModelClass::ExpoPropModelClass(char *p_strid) : PropModelClass(p_strid)
{
    /* set default values */
    exponent = 3.0;
    coefficient = 0.3125e-3;
}
/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::type                                               ****/
/******************************************************************************************/
const int ExpoPropModelClass::type() { return CConst::PropExpo; }
/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::prop_power_loss                                    ****/
/******************************************************************************************/
double ExpoPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight, double)
{
    double dz = (useheight ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }

    double rel_height_sq = (sector->antenna_height / 25.0) * (sector->antenna_height / 25.0);
    double prop = coefficient * exp(-(exponent/2)*log(rsq))*rel_height_sq;

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::get_prop_model_param_ptr                           ****/
/******************************************************************************************/
void ExpoPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&, double *&dptr)
{
    switch(param_idx) {
        case 0: sprintf(str, "EXPONENT:");    type=CConst::NumericDouble; dptr = &exponent;    break;
        case 1: sprintf(str, "COEFFICIENT:"); type=CConst::NumericDouble; dptr = &coefficient; break;
    }
}
/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::comp_num_prop_param                                ****/
/******************************************************************************************/
int ExpoPropModelClass::comp_num_prop_param()
{
    return(2);
}
/******************************************************************************************/
/**** FUNCTION: ExpoPropModelClass::print_params                                       ****/
/******************************************************************************************/
void ExpoPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: EXPONENTIAL\n", indent);
    chptr += sprintf(chptr, "%sEXPONENT: %9.7f\n", indent, exponent);
    chptr += sprintf(chptr, "%sCOEFFICIENT: %12.7e\n", indent, coefficient);
    PRMSG(fp, msg);
}
/******************************************************************************************/

/*
*******************************************************************************************
* RSquaredPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::RSquaredPropModelClass                         ****/
/******************************************************************************************/
RSquaredPropModelClass::RSquaredPropModelClass(char *p_strid) : PropModelClass(p_strid)
{
    /* set default values */
    coefficient = 0.3125e-3;
}
/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::type                                           ****/
/******************************************************************************************/
const int RSquaredPropModelClass::type() { return CConst::PropRSquared; }
/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::prop_power_loss                                ****/
/******************************************************************************************/
double RSquaredPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int, double)
{
    double res_sq = np->resolution*np->resolution;

    double rsq = (delta_x*delta_x + delta_y*delta_y)*res_sq;
    if (rsq < res_sq) {
        rsq = res_sq;
    }

    double prop = coefficient / rsq;

    return(prop);
}
/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::get_prop_model_param_ptr                           ****/
/******************************************************************************************/
void RSquaredPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&, double *&dptr)
{
    switch(param_idx) {
        case 0: sprintf(str, "COEFFICIENT:"); type=CConst::NumericDouble; dptr = &coefficient; break;
    }
}
/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::comp_num_prop_param                                ****/
/******************************************************************************************/
int RSquaredPropModelClass::comp_num_prop_param()
{
    return(1);
}
/******************************************************************************************/
/**** FUNCTION: RSquaredPropModelClass::print_params                                       ****/
/******************************************************************************************/
void RSquaredPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: RSQUARED\n", indent);
    chptr += sprintf(chptr, "%sCOEFFICIENT: %12.7e\n", indent, coefficient);
    PRMSG(fp, msg);
}
/******************************************************************************************/
/*
*******************************************************************************************
* PwLinPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::PwLinPropModelClass                               ****/
/******************************************************************************************/
PwLinPropModelClass::PwLinPropModelClass(char *p_strid) : PropModelClass(p_strid)
{
}
/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::type                                              ****/
/******************************************************************************************/
const int PwLinPropModelClass::type() { return CConst::PropPwLin; }
/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::prop_power_loss                                   ****/
/******************************************************************************************/
double PwLinPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight, double)
{
    double prop;

    double dz = (useheight ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }

    double prop_y  = y0 + ys*sector->antenna_height;
    double prop_s2 = k0 + k1*sector->antenna_height;
    double d = 0.5*log(rsq)/log(10.0);
    if (d < prop_y) {
        prop = exp((py + s1*(d-prop_y))*log(10.0)/10.0);
    } else {
        prop = exp((py + prop_s2*(d-prop_y))*log(10.0)/10.0);
    }

    return(prop);
}
/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::get_prop_model_param_ptr                          ****/
/******************************************************************************************/
void PwLinPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&, double *&dptr)
{
    switch(param_idx) {
        case 0: sprintf(str, "Y0:"); type=CConst::NumericDouble; dptr = &y0; break;
        case 1: sprintf(str, "YS:"); type=CConst::NumericDouble; dptr = &ys; break;
        case 2: sprintf(str, "K0:"); type=CConst::NumericDouble; dptr = &k0; break;
        case 3: sprintf(str, "K1:"); type=CConst::NumericDouble; dptr = &k1; break;
        case 4: sprintf(str, "S1:"); type=CConst::NumericDouble; dptr = &s1; break;
        case 5: sprintf(str, "PY:"); type=CConst::NumericDouble; dptr = &py; break;
    }
}
/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::comp_num_prop_param                               ****/
/******************************************************************************************/
int PwLinPropModelClass::comp_num_prop_param()
{
    return(6);
}
/******************************************************************************************/
/**** FUNCTION: PwLinPropModelClass::print_params                                      ****/
/******************************************************************************************/
void PwLinPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: PW_LINEAR\n", indent);
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "%sY0: %9.7f\n", indent, y0);
    chptr += sprintf(chptr, "%sYS: %9.7f\n", indent, ys);
    chptr += sprintf(chptr, "%sK0: %9.7f\n", indent, k0);
    chptr += sprintf(chptr, "%sK1: %9.7f\n", indent, k1);
    chptr += sprintf(chptr, "%sS1: %9.7f\n", indent, s1);
    chptr += sprintf(chptr, "%sPY: %9.7f\n", indent, py);
    PRMSG(fp, msg);
}
/******************************************************************************************/

/*
*******************************************************************************************
* TerrainPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::TerrainPropModelClass                           ****/
/******************************************************************************************/
TerrainPropModelClass::TerrainPropModelClass(MapClutterClass *& p_map_clutter, char *p_strid) : PropModelClass(p_strid), map_clutter(p_map_clutter)
{
    /* set default values */
    num_clutter_type = 0;
    useheight = 0;
    val_y   = 2.0;
    val_py  = -87.0;
    val_s1  = -1.0;
    val_s2  = -3.0;

    vec_k = (double *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::type                                            ****/
/******************************************************************************************/
const int TerrainPropModelClass::type() { return CConst::PropTerrain; }
/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::prop_power_loss                                 ****/
/******************************************************************************************/
double TerrainPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight, double)
{
    int clutter_idx;
    double prop_db;

    double dz = (useheight ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }

    double d = 0.5*log(rsq)/log(10.0);
    if (d < val_y) {
        prop_db = val_py + val_s1*(d-val_y);
    } else {
        prop_db = val_py + val_s2*(d-val_y);
    }

    CellClass *cell     = sector->parent_cell;
    double *dist_vector = DVECTOR(num_clutter_type);
    if (num_clutter_type) {
        np->get_path_clutter(cell->posn_x, cell->posn_y, cell->posn_x+delta_x, cell->posn_y+delta_y, dist_vector);
    }

    int k_idx = 0;
    if (useheight) {
        double h = log(sector->antenna_height)/log(10.0);
        prop_db += vec_k[k_idx++]*h;
        prop_db += vec_k[k_idx++]*h*d;
    }

    for (clutter_idx=0; clutter_idx<=num_clutter_type-1; clutter_idx++) {
        prop_db += vec_k[k_idx++]*dist_vector[clutter_idx];
    }

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::get_prop_model_param_ptr                        ****/
/******************************************************************************************/
void TerrainPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    if (param_idx == 6) {
        vec_k = DVECTOR(num_clutter_type + 2*useheight);
    }

    switch(param_idx) {
        case 0: sprintf(str, "Y:");                 type=CConst::NumericDouble; dptr = &val_y;              break;
        case 1: sprintf(str, "PY:");                type=CConst::NumericDouble; dptr = &val_py;             break;
        case 2: sprintf(str, "S1:");                type=CConst::NumericDouble; dptr = &val_s1;             break;
        case 3: sprintf(str, "S2:");                type=CConst::NumericDouble; dptr = &val_s2;             break;
        case 4: sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;          break;
        case 5: sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;   break;
        default:
                sprintf(str, "C_%d:", param_idx-6); type=CConst::NumericDouble; dptr = &(vec_k[param_idx-6]); break;
    }
}
/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::comp_num_prop_param                             ****/
/******************************************************************************************/
int TerrainPropModelClass::comp_num_prop_param()
{
    int n;
    n = 6 + num_clutter_type + 2*useheight;
    return(n);
}
/******************************************************************************************/
/**** FUNCTION: TerrainPropModelClass::print_params                                    ****/
/******************************************************************************************/
void TerrainPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    int c_idx;
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: TERRAIN\n", indent);
    chptr += sprintf(chptr, "%sY: %9.7f\n",             indent, val_y);
    chptr += sprintf(chptr, "%sPY: %9.7f\n",            indent, val_py);
    chptr += sprintf(chptr, "%sS1: %9.7f\n",            indent, val_s1);
    chptr += sprintf(chptr, "%sS2: %9.7f\n",            indent, val_s2);
    chptr += sprintf(chptr, "%sUSE_HEIGHT: %d\n",       indent, useheight);
    chptr += sprintf(chptr, "%sNUM_CLUTTER_TYPE: %d\n", indent, num_clutter_type);
    PRMSG(fp, msg);
    for (c_idx=0; c_idx<=num_clutter_type+2*useheight-1; c_idx++) {
        sprintf(msg, "%sC_%d: %9.7f\n", indent, c_idx, vec_k[c_idx]);
        PRMSG(fp, msg);
    }
}
/******************************************************************************************/

/*
*******************************************************************************************
* SegmentPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::SegmentPropModelClass                           ****/
/******************************************************************************************/
SegmentPropModelClass::SegmentPropModelClass(MapClutterClass *& p_map_clutter, char *p_strid) : PropModelClass(p_strid), map_clutter(p_map_clutter)
{
    num_inflexion = 1;
    x = DVECTOR( num_inflexion );
    y = DVECTOR( num_inflexion );
    x[0] = 0.0;
    y[0] = -35.0;
    num_clutter_type = 0;
    useheight = 0;
    start_slope = -30.0;
    final_slope = -30.0;
    vec_k = (double *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::type                                            ****/
/******************************************************************************************/
const int SegmentPropModelClass::type() { return CConst::PropSegment; }
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::~SegmentPropModelClass                          ****/
/******************************************************************************************/
SegmentPropModelClass::~SegmentPropModelClass()
{
    free(x);
    free(y);
    if (num_clutter_type + 2*useheight) {
        free(vec_k);
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::prop_power_loss                                 ****/
/******************************************************************************************/
double SegmentPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight_for_dist, double)
{
    int done;
    double prop_db = 0.0;

    double dz = (useheight_for_dist ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }
    double d = 0.5*log(rsq)/log(10.0);
    
    //according to the distance compute the power of the propagation attenuation
    if ( d<=x[0] ) {
        prop_db = y[0] + start_slope*(d-x[0]);
    } else if ( d>x[num_inflexion-1] ) {
        prop_db = y[num_inflexion-1] + final_slope*(d-x[num_inflexion-1]);
    } else {
        done = 0;
        for ( int idx=0; (idx<num_inflexion-1)&&(!done); idx++ ) {
            if ( (d>x[idx]) && (d<=x[idx+1]) ) {
                prop_db = y[idx] + ((y[idx]-y[idx+1])/(x[idx]-x[idx+1]))*(d-x[idx]);
                done = 1;
            }
        }
    }

    CellClass *cell     = sector->parent_cell;
    double *dist_vector = DVECTOR(num_clutter_type);
    if (num_clutter_type) {
        np->get_path_clutter(cell->posn_x, cell->posn_y, cell->posn_x+delta_x, cell->posn_y+delta_y, dist_vector);
    }

    int k_idx = 0;
    if (useheight) {
        double h = log(sector->antenna_height)/log(10.0);
        prop_db += vec_k[k_idx++]*h;
        prop_db += vec_k[k_idx++]*h*d;
    }

    for ( int clutter_idx=0; clutter_idx<=num_clutter_type-1; clutter_idx++) {
        prop_db += vec_k[k_idx++]*dist_vector[clutter_idx];
    }

    double prop;
    if (prop_db < -2000.0) {
        prop = exp(-2000.0*log(10.0)/10.0);
    } else {
        prop = exp(prop_db*log(10.0)/10.0);
    }

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::get_prop_model_param_ptr                        ****/
/******************************************************************************************/
void SegmentPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i, j;

    if (param_idx == 1) {
        x = (double *) realloc((void *) x, num_inflexion*sizeof(double));
        y = (double *) realloc((void *) y, num_inflexion*sizeof(double));
    } else if (param_idx == 2*num_inflexion+5) {
        vec_k = DVECTOR(num_clutter_type + 2*useheight);
    }

    if (param_idx == 0) {
        sprintf(str, "N:");                 type=CConst::NumericInt;    iptr = &num_inflexion;
    } else if (param_idx <= 2*num_inflexion) {
        i = param_idx-1;
        j = i>>1;
        if ((i&1) == 0) {
            sprintf(str, "X[%d]:", j);          type=CConst::NumericDouble; dptr = &x[j];
        } else {
            sprintf(str, "Y[%d]:", j);          type=CConst::NumericDouble; dptr = &y[j];
        }
    } else if (param_idx == 2*num_inflexion+1) {
        sprintf(str, "S:");                 type=CConst::NumericDouble; dptr = &start_slope;
    } else if (param_idx == 2*num_inflexion+2) {
        sprintf(str, "F:");                 type=CConst::NumericDouble; dptr = &final_slope;
    } else if (param_idx == 2*num_inflexion+3) {
        sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 2*num_inflexion+4) {
        sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
    } else {
        i = param_idx-(2*num_inflexion+5);
        sprintf(str, "C_%d:", i);           type=CConst::NumericDouble; dptr = &(vec_k[i]);
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::comp_num_prop_param                             ****/
/******************************************************************************************/
int SegmentPropModelClass::comp_num_prop_param()
{
    int n;

    n = 2*num_inflexion + 5 + num_clutter_type + 2*useheight;

    return(n);
}
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::print_params                                    ****/
/******************************************************************************************/
void SegmentPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    int c_idx, idx;
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: SEGMENT\n", indent);
    PRMSG(fp, msg);

    if (fmt == CConst::geometryFileType) {
        chptr = msg;
        chptr += sprintf(chptr, "%sN: %d\n",               indent, num_inflexion);
        for( idx=0; idx<=num_inflexion-1; idx++ ) {
            chptr += sprintf(chptr, "%sX[%d]: %9.7f\n",            indent, idx,x[idx]);
            chptr += sprintf(chptr, "%sY[%d]: %9.7f\n",            indent, idx,y[idx]);
        }
        chptr += sprintf(chptr, "%sS: %9.7f\n",             indent, start_slope);
        chptr += sprintf(chptr, "%sF: %9.7f\n",             indent, final_slope);
        chptr += sprintf(chptr, "%sUSE_HEIGHT: %d\n",       indent, useheight);
        chptr += sprintf(chptr, "%sNUM_CLUTTER_TYPE: %d\n", indent, num_clutter_type);
        PRMSG(fp, msg);
        for (c_idx=0; c_idx<=num_clutter_type+2*useheight-1; c_idx++) {
            sprintf(msg, "%sC_%d: %9.7f\n", indent, c_idx, vec_k[c_idx]);
            PRMSG(fp, msg);
        }
    }

    if (fmt == CConst::reportFileType) {
        chptr = msg;
        chptr += sprintf(chptr, "%sNUMBER OF INFLEXION POINTS: %d\n",               indent, num_inflexion);
        PRMSG(fp, msg);
        for( idx=0; idx<=num_inflexion-1; idx++ ) {
            chptr = msg;
            chptr += sprintf(chptr, "%sPT_%d: LOG(DIST) = %9.7f DIST = %8.3f m EFF PATH GAIN = %8.3f dB\n", indent, idx, x[idx], exp(log(10.0)*x[idx]), y[idx]);
            PRMSG(fp, msg);
        }
        chptr = msg;
        chptr += sprintf(chptr, "\n");

        chptr += sprintf(chptr, "%sSTART SLOPE: %9.7f\n",   indent, start_slope);
        PRMSG(fp, msg);
        for( idx=0; idx<=num_inflexion-2; idx++ ) {
            chptr = msg;
            chptr += sprintf(chptr, "%sSLOPE (PT_%d --> PT_%d): %9.7f\n",   indent, idx, idx+1, (y[idx+1] - y[idx])/(x[idx+1] - x[idx]));
            PRMSG(fp, msg);
        }
        chptr = msg;
        chptr += sprintf(chptr, "%sFINAL SLOPE: %9.7f\n",             indent, final_slope);
        chptr += sprintf(chptr, "\n");

        if (useheight) {
            chptr += sprintf(chptr, "%sHEIGHT COEFFICIENT 1: %9.7f\n",    indent, vec_k[num_clutter_type+2*useheight-2]);
            chptr += sprintf(chptr, "%sHEIGHT COEFFICIENT 2: %9.7f\n",    indent, vec_k[num_clutter_type+2*useheight-1]);
        } else {
            chptr += sprintf(chptr, "%sNO HEIGHT COEFFICIENTS\n",       indent);
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        if (num_clutter_type) {
            sprintf(msg, "%sCLUTTER COEFFICIENTS:\n", indent);
            PRMSG(fp, msg);
            for (c_idx=0; c_idx<=num_clutter_type-1; c_idx++) {
                sprintf(msg, "%sC_%d: %9.7f\n", indent, c_idx, vec_k[c_idx]);
                PRMSG(fp, msg);
            }
        } else {
            sprintf(msg, "%sNO CLUTTER COEFFICIENTS\n", indent);
            PRMSG(fp, msg);
        }
    }
}
/******************************************************************************************/

/*
*******************************************************************************************
* SegmentWithThetaPropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::SegmentWithThetaPropModelClass         ****/
/******************************************************************************************/
SegmentWithThetaPropModelClass::SegmentWithThetaPropModelClass(MapClutterClass *& p_map_clutter, char *p_strid) : PropModelClass(p_strid), map_clutter(p_map_clutter)
{
    int i;

    num_inflexion = 1;
    x = DVECTOR( num_inflexion );
    y = DVECTOR( num_inflexion );
    x[0] = 0.0;
    y[0] = -35.0;
    num_clutter_type = 0;
    useheight = 0;
    start_slope  = -30.0;
    final_slope  = -30.0;
    max_level_db = 108.0;
    vec_k = (double *) NULL;

    n_series_y = IVECTOR( num_inflexion );
    c_series_y = (double **) malloc( num_inflexion * sizeof(double *));
    s_series_y = (double **) malloc( num_inflexion * sizeof(double *));
    for (i=0; i<=num_inflexion-1; i++) {
        n_series_y[i] = 0;
        c_series_y[i] = (double *) NULL;
        s_series_y[i] = (double *) NULL;
    }

    n_series_start_slope = 0;
    c_series_start_slope = (double *) NULL;
    s_series_start_slope = (double *) NULL;

    n_series_final_slope = 0;
    c_series_final_slope = (double *) NULL;
    s_series_final_slope = (double *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::~SegmentWithThetaPropModelClass        ****/
/******************************************************************************************/
SegmentWithThetaPropModelClass::~SegmentWithThetaPropModelClass()
{
    int i;

    free(x);
    free(y);
    if (num_clutter_type) {
        free(vec_k);
    }

    for (i=0; i<=num_inflexion-1; i++) {
        if (n_series_y[i]) {
            free(c_series_y[i]);
            free(s_series_y[i]);
        }
    }
    free(n_series_y);
    free(c_series_y);
    free(s_series_y);

    if (n_series_start_slope) {
        free(c_series_start_slope);
        free(s_series_start_slope);
    }

    if (n_series_final_slope) {
        free(c_series_final_slope);
        free(s_series_final_slope);
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::type                                   ****/
/******************************************************************************************/
const int SegmentWithThetaPropModelClass::type() { return CConst::PropSegmentWithTheta; }
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::set_num_inflexion                      ****/
/******************************************************************************************/
void SegmentWithThetaPropModelClass::set_num_inflexion(int n)
{
    int i;

    x = ( double *) realloc((void *) x, n*sizeof(double));
    y = ( double *) realloc((void *) y, n*sizeof(double));

    n_series_y = (int *) realloc((void *) n_series_y, n*sizeof(int));
    c_series_y = (double **) realloc( c_series_y, n*sizeof(double *));
    s_series_y = (double **) realloc( s_series_y, n*sizeof(double *));
    for (i=num_inflexion; i<=n-1; i++) {
        n_series_y[i] = 0;
        c_series_y[i] = (double *) NULL;
        s_series_y[i] = (double *) NULL;
    }

    num_inflexion = n;
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::add_cutoff_slope                       ****/
/******************************************************************************************/
void SegmentWithThetaPropModelClass::add_cutoff_slope(double d, double level, double s_max_level_db, double slope)
{
    int i;

    if (d < x[num_inflexion-1]) {
        CORE_DUMP;
    }

    set_num_inflexion(num_inflexion+2);

    n_series_y[num_inflexion-2] = n_series_y[num_inflexion-3];
    if (n_series_final_slope > n_series_y[num_inflexion-2]) {
        n_series_y[num_inflexion-2] = n_series_final_slope;
    }
    c_series_y[num_inflexion-2] = DVECTOR(n_series_y[num_inflexion-2]);
    s_series_y[num_inflexion-2] = DVECTOR(n_series_y[num_inflexion-2]);

    x[num_inflexion-2] = d;
    y[num_inflexion-2] = y[num_inflexion-3] + final_slope*(d-x[num_inflexion-3]);

    for (i=0; i<=n_series_y[num_inflexion-2]-1; i++) {
        c_series_y[num_inflexion-2][i] = 0.0;
        s_series_y[num_inflexion-2][i] = 0.0;
        if (i <= n_series_y[num_inflexion-2]-1) {
            c_series_y[num_inflexion-2][i] += c_series_y[num_inflexion-3][i];
            s_series_y[num_inflexion-2][i] += s_series_y[num_inflexion-3][i];
        }
        if (i <= n_series_final_slope-1) {
            c_series_y[num_inflexion-2][i] += c_series_final_slope[i]*(d-x[num_inflexion-3]);
            s_series_y[num_inflexion-2][i] += s_series_final_slope[i]*(d-x[num_inflexion-3]);
        }
    }

    n_series_y[num_inflexion-1] = 0;
    free(c_series_y[num_inflexion-1]);
    free(s_series_y[num_inflexion-1]);
    c_series_y[num_inflexion-1] = (double *) NULL;
    s_series_y[num_inflexion-1] = (double *) NULL;

    x[num_inflexion-1] = d + 0.1;
    y[num_inflexion-1] = level - 1.0;

    n_series_final_slope = 0;
    free(c_series_final_slope);
    free(s_series_final_slope);
    c_series_final_slope = (double *) NULL;
    s_series_final_slope = (double *) NULL;

    final_slope = slope;

    max_level_db = s_max_level_db;
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::prop_power_loss                        ****/
/******************************************************************************************/
double SegmentWithThetaPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight_for_dist, double angle_deg)
{
    int i, done, seg, max_series_n;
    double prop_db = 0.0;
    double *cc, *ss;
    double eff_slope, eff_ya, eff_yb;

    double dz = (useheight_for_dist ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }
    double d = 0.5*log(rsq)/log(10.0);

    /**************************************************************************************/
    /**** Determine segment number and max_series_n                                    ****/
    /**************************************************************************************/
    if ( d<=x[0] ) {
        seg = 0;
        max_series_n = n_series_y[0];
        if (n_series_start_slope > max_series_n) {
            max_series_n = n_series_start_slope;
        }
    } else if ( d>x[num_inflexion-1] ) {
        seg = num_inflexion;
        max_series_n = n_series_y[num_inflexion-1];
        if (n_series_final_slope > max_series_n) {
            max_series_n = n_series_final_slope;
        }
    } else {
        done = 0;
        for ( i=0; (i<num_inflexion-1)&&(!done); i++ ) {
            if ( (d>x[i]) && (d<=x[i+1]) ) {
                seg = i+1;
                done = 1;
            }
        }
        max_series_n = n_series_y[seg-1];
        if (n_series_y[seg] > max_series_n) {
            max_series_n = n_series_y[seg];
        }
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Compute cos/sin series values                                                ****/
    /**************************************************************************************/
    if (max_series_n) {
        cc = DVECTOR(max_series_n);
        ss = DVECTOR(max_series_n);
        cc[0] = delta_x / sqrt( (double) delta_x*delta_x + (double) delta_y*delta_y );
        ss[0] = delta_y / sqrt( (double) delta_x*delta_x + (double) delta_y*delta_y );
        if (angle_deg != 0.0) {
            double cos_angle = cos(angle_deg*PI/180.0);
            double sin_angle = sin(angle_deg*PI/180.0);
            double cos_0 = cc[0]*cos_angle - ss[0]*sin_angle;
            double sin_0 = cc[0]*sin_angle + ss[0]*cos_angle;
            cc[0] = cos_0;
            ss[0] = sin_0;
        }
        for (i=1; i<=max_series_n-1; i++) {
            cc[i] = cc[i-1]*cc[0] - ss[i-1]*ss[0];
            ss[i] = cc[i-1]*ss[0] + ss[i-1]*cc[0];
        }
    } else {
        cc = (double *) NULL;
        ss = (double *) NULL;
    }
    /**************************************************************************************/

    if ( seg == 0 ) {
        eff_ya = y[0];
        for (i=0; i<=n_series_y[0]-1; i++) {
            eff_ya += c_series_y[0][i]*cc[i] + s_series_y[0][i]*ss[i];
        }
        eff_slope = start_slope;
        for (i=0; i<=n_series_start_slope-1; i++) {
            eff_slope += c_series_start_slope[i]*cc[i] + s_series_start_slope[i]*ss[i];
        }
        prop_db = eff_ya + eff_slope*(d-x[0]);
    } else if ( seg == num_inflexion ) {
        eff_ya = y[num_inflexion-1];
        for (i=0; i<=n_series_y[num_inflexion-1]-1; i++) {
            eff_ya += c_series_y[num_inflexion-1][i]*cc[i] + s_series_y[num_inflexion-1][i]*ss[i];
        }
        eff_slope = final_slope;
        for (i=0; i<=n_series_final_slope-1; i++) {
            eff_slope += c_series_final_slope[i]*cc[i] + s_series_final_slope[i]*ss[i];
        }
        prop_db = eff_ya + eff_slope*(d-x[num_inflexion-1]);
    } else {
        eff_ya = y[seg-1];
        for (i=0; i<=n_series_y[seg-1]-1; i++) {
            eff_ya += c_series_y[seg-1][i]*cc[i] + s_series_y[seg-1][i]*ss[i];
        }

        eff_yb = y[seg];
        for (i=0; i<=n_series_y[seg]-1; i++) {
            eff_yb += c_series_y[seg][i]*cc[i] + s_series_y[seg][i]*ss[i];
        }

        prop_db = eff_ya + ((eff_ya-eff_yb)/(x[seg-1]-x[seg]))*(d-x[seg-1]);
    }

    CellClass *cell     = sector->parent_cell;
    double *dist_vector = DVECTOR(num_clutter_type);
    if (num_clutter_type) {
        np->get_path_clutter(cell->posn_x, cell->posn_y, cell->posn_x+delta_x, cell->posn_y+delta_y, dist_vector);
    }

    int k_idx = 0;
    if (useheight) {
        double h = log(sector->antenna_height)/log(10.0);
        prop_db += vec_k[k_idx++]*h;
        prop_db += vec_k[k_idx++]*h*d;
    }

    for ( int clutter_idx=0; clutter_idx<=num_clutter_type-1; clutter_idx++) {
        prop_db += vec_k[k_idx++]*dist_vector[clutter_idx];
    }

    double prop;
    if (prop_db < -2000.0) {
        prop = exp(-2000.0*log(10.0)/10.0);
    } else if (prop_db > max_level_db) {
        prop = exp(max_level_db*log(10.0)/10.0);
    } else {
        prop = exp(prop_db*log(10.0)/10.0);
    }

    if (num_clutter_type) {
        free(dist_vector);
    }
    if (max_series_n) {
        free(cc);
        free(ss);
    }

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::get_prop_model_param_ptr               ****/
/******************************************************************************************/
void SegmentWithThetaPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i, j, new_val, done, idx;
    static int old_val = 0;

    done = 0;
    idx = 0;
    if (!done) {
        if (param_idx == idx) {
            sprintf(str, "N:");                 type=CConst::NumericInt;    iptr = &num_inflexion;
            old_val = num_inflexion;
        }
        if (param_idx == idx+1) {
            new_val = num_inflexion;
            num_inflexion = old_val;
            set_num_inflexion(new_val);
        } else if (param_idx < idx+1) {
            done = 1;
        }
    }
    for (i=0; (i<=num_inflexion-1)&&(!done); i++) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "X[%d]:", i);          type=CConst::NumericDouble; dptr = &x[i];
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "Y[%d]:", i);          type=CConst::NumericDouble; dptr = &y[i];
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "N_SERIES_Y[%d]:", i);          type=CConst::NumericInt; iptr = &n_series_y[i];
        }
        if (param_idx == idx+1) {
            c_series_y[i] = (double *) realloc((void *) c_series_y[i], n_series_y[i]*sizeof(double));
            s_series_y[i] = (double *) realloc((void *) s_series_y[i], n_series_y[i]*sizeof(double));
        }
        if (param_idx < idx+1) {
            done = 1;
        } else {
            idx += 2*n_series_y[i];
            if ((param_idx <= idx) && (n_series_y[i])) {
                j = param_idx - idx + 2*n_series_y[i] - 1;
                if ((j&1) == 0) {
                    sprintf(str, "C_SERIES_Y[%d][%d]:", i, j>>1); type=CConst::NumericDouble; dptr = &c_series_y[i][j>>1];
                } else {
                    sprintf(str, "S_SERIES_Y[%d][%d]:", i, j>>1); type=CConst::NumericDouble; dptr = &s_series_y[i][j>>1];
                }
            }
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "S:");              type=CConst::NumericDouble; dptr = &start_slope;
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "N_SERIES_S:");     type=CConst::NumericInt;    iptr = &n_series_start_slope;
        }
        if (param_idx == idx+1) {
            c_series_start_slope = (double *) realloc((void *) c_series_start_slope, n_series_start_slope*sizeof(double));
            s_series_start_slope = (double *) realloc((void *) s_series_start_slope, n_series_start_slope*sizeof(double));
        }
        if (param_idx < idx+1) {
            done = 1;
        } else {
            idx += 2*n_series_start_slope;
            if ((param_idx <= idx) && (n_series_start_slope)) {
                j = param_idx - idx + 2*n_series_start_slope - 1;
                if ((j&1) == 0) {
                    sprintf(str, "C_SERIES_START_SLOPE[%d]:", j>>1); type=CConst::NumericDouble; dptr = &c_series_start_slope[j>>1];
                } else {
                    sprintf(str, "S_SERIES_START_SLOPE[%d]:", j>>1); type=CConst::NumericDouble; dptr = &s_series_start_slope[j>>1];
                }
            }
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "F:");              type=CConst::NumericDouble; dptr = &final_slope;
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "N_SERIES_F:");     type=CConst::NumericInt;    iptr = &n_series_final_slope;
        }
        if (param_idx == idx+1) {
            c_series_final_slope = (double *) realloc((void *) c_series_final_slope, n_series_final_slope*sizeof(double));
            s_series_final_slope = (double *) realloc((void *) s_series_final_slope, n_series_final_slope*sizeof(double));
        }
        if (param_idx < idx+1) {
            done = 1;
        } else {
            idx += 2*n_series_final_slope;
            if ((param_idx <= idx) && (n_series_final_slope)) {
                j = param_idx - idx + 2*n_series_final_slope - 1;
                if ((j&1) == 0) {
                    sprintf(str, "C_SERIES_FINAL_SLOPE[%d]:", j>>1); type=CConst::NumericDouble; dptr = &c_series_final_slope[j>>1];
                } else {
                    sprintf(str, "S_SERIES_FINAL_SLOPE[%d]:", j>>1); type=CConst::NumericDouble; dptr = &s_series_final_slope[j>>1];
                }
            }
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
        }
        if (param_idx == idx+1) {
            vec_k = (double *) realloc((void *) vec_k, (num_clutter_type + 2*useheight)*sizeof(double));
        }
        if (param_idx < idx+1) {
            done = 1;
        } else {
            idx += num_clutter_type + 2*useheight;
            if ((param_idx <= idx) && (num_clutter_type + 2*useheight)) {
                j = param_idx - idx + num_clutter_type + 2*useheight - 1;
                sprintf(str, "C_%d:", j);           type=CConst::NumericDouble; dptr = &(vec_k[j]);
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::comp_num_prop_param                    ****/
/******************************************************************************************/
int SegmentWithThetaPropModelClass::comp_num_prop_param()
{
    int i, n;

    n = 2*num_inflexion + 1;
    for (i=0; i<=num_inflexion-1; i++) {
        n += 2*n_series_y[i] + 1;
    }

    n += 2*n_series_start_slope + 2;
    n += 2*n_series_final_slope + 2;

    n += 2 + num_clutter_type + 2*useheight;

    return(n);
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::print_params                           ****/
/******************************************************************************************/
void SegmentWithThetaPropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    int idx, k, c_idx;
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: SEGMENT_WITH_THETA\n", indent);
    chptr += sprintf(chptr, "%sN: %d\n",               indent, num_inflexion);
    for( idx=0; idx<=num_inflexion-1; idx++ ) {
        chptr += sprintf(chptr, "%sX[%d]: %9.7f\n",            indent, idx,x[idx]);
        chptr += sprintf(chptr, "%sY[%d]: %9.7f\n",            indent, idx,y[idx]);
        chptr += sprintf(chptr, "%sN_SERIES_Y[%d]: %d\n",      indent, idx,n_series_y[idx]);
        for( k=0; k<=n_series_y[idx]-1; k++ ) {
            chptr += sprintf(chptr, "%sC_SERIES_Y[%d][%d]: %9.7f\n", indent, idx, k, c_series_y[idx][k]);
            chptr += sprintf(chptr, "%sS_SERIES_Y[%d][%d]: %9.7f\n", indent, idx, k, s_series_y[idx][k]);
        }
    }
    chptr += sprintf(chptr, "%sS: %9.7f\n",             indent, start_slope);
    chptr += sprintf(chptr, "%sN_SERIES_S: %d\n",       indent, n_series_start_slope);
    for( k=0; k<=n_series_start_slope-1; k++ ) {
        chptr += sprintf(chptr, "%sC_SERIES_START_SLOPE[%d]: %9.7f\n", indent, k, c_series_start_slope[k]);
        chptr += sprintf(chptr, "%sS_SERIES_START_SLOPE[%d]: %9.7f\n", indent, k, s_series_start_slope[k]);
    }
    chptr += sprintf(chptr, "%sF: %9.7f\n",             indent, final_slope);
    chptr += sprintf(chptr, "%sN_SERIES_F: %d\n",       indent, n_series_final_slope);
    for( k=0; k<=n_series_final_slope-1; k++ ) {
        chptr += sprintf(chptr, "%sC_SERIES_FINAL_SLOPE[%d]: %9.7f\n", indent, k, c_series_final_slope[k]);
        chptr += sprintf(chptr, "%sS_SERIES_FINAL_SLOPE[%d]: %9.7f\n", indent, k, s_series_final_slope[k]);
    }
    chptr += sprintf(chptr, "%sUSE_HEIGHT: %d\n",       indent, useheight);
    chptr += sprintf(chptr, "%sNUM_CLUTTER_TYPE: %d\n", indent, num_clutter_type);
    PRMSG(fp, msg);
    for (c_idx=0; c_idx<=num_clutter_type+2*useheight-1; c_idx++) {
        sprintf(msg, "%sC_%d: %9.7f\n", indent, c_idx, vec_k[c_idx]);
        PRMSG(fp, msg);
    }
}
/******************************************************************************************/

/*
*******************************************************************************************
* SegmentAnglePropModelClass definition
*******************************************************************************************
*/

/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::SegmentAnglePropModelClass                 ****/
/******************************************************************************************/
SegmentAnglePropModelClass::SegmentAnglePropModelClass(MapClutterClass *& p_map_clutter, char *p_strid) : PropModelClass(p_strid), map_clutter(p_map_clutter)
{
    int i;

    num_inflexion = 1;
    x = DVECTOR( num_inflexion );
    x[0] = 0.0;
    num_clutter_type = 0;
    useheight = 0;
    max_level_db = -5.0;
    vec_k = (double *) NULL;

    n_angle = 0;

    y = (double **) malloc( num_inflexion * sizeof(double *));
    use_n_y = IVECTOR(num_inflexion);
    for (i=0; i<=num_inflexion-1; i++) {
        use_n_y[0] = 0;
        y[i] = DVECTOR(1);
        y[i][0] = -35.0;
    }

    use_n_start_slope = 0;
    start_slope = DVECTOR(1);
    start_slope[0] = -30;

    use_n_final_slope = 0;
    final_slope = DVECTOR(1);
    final_slope[0] = -30;

    cc = (double *) NULL;
    ss = (double *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::~SegmentAnglePropModelClass                ****/
/******************************************************************************************/
SegmentAnglePropModelClass::~SegmentAnglePropModelClass()
{
    int i;

    free(x);
    if (num_clutter_type) {
        free(vec_k);
    }

    for (i=0; i<=num_inflexion-1; i++) {
        free(y[i]);
    }
    free(y);
    free(use_n_y);

    free(start_slope);

    free(final_slope);

    if (n_angle) {
        free(cc);
        free(ss);
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::type                                       ****/
/******************************************************************************************/
const int SegmentAnglePropModelClass::type() { return CConst::PropSegmentAngle; }
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::set_num_inflexion                          ****/
/******************************************************************************************/
void SegmentAnglePropModelClass::set_num_inflexion(int n)
{
    int i;

    x = ( double  *) realloc((void *) x, n*sizeof(double));
    y = ( double **) realloc((void *) y, n*sizeof(double *));

    use_n_y  = (int *) realloc((void *) use_n_y, n*sizeof(int));
    for (i=num_inflexion; i<=n-1; i++) {
        use_n_y[i] = 0;
        y[i] = DVECTOR(1);
        y[i][0] = 78.0;
    }

    num_inflexion = n;
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::add_cutoff_slope                           ****/
/******************************************************************************************/
void SegmentAnglePropModelClass::add_cutoff_slope(double d, double level, double s_max_level_db, double slope)
{
    int i;

    if (d < x[num_inflexion-1]) {
        CORE_DUMP;
    }

    set_num_inflexion(num_inflexion+2);

    if (use_n_y[num_inflexion-3] || use_n_final_slope) {
        use_n_y[num_inflexion-2] = 1;
        y[num_inflexion-2] = ( double  *) realloc((void *) y[num_inflexion-2], n_angle*sizeof(double));
    }
        
    x[num_inflexion-2] = d;
    if (use_n_y[num_inflexion-2]) {
        for (i=0; i<=n_angle-1; i++) {
            y[num_inflexion-2][i] = y[num_inflexion-3][i] + final_slope[i]*(d-x[num_inflexion-3]);
        }
    } else {
        y[num_inflexion-2][0] = y[num_inflexion-3][0] + final_slope[0]*(d-x[num_inflexion-3]);
    }

    use_n_y[num_inflexion-1] = 0;

    x[num_inflexion-1] = d + 0.1;
    y[num_inflexion-1][0] = level - 1.0;

    use_n_final_slope = 0;

    final_slope[0] = slope;

    max_level_db = s_max_level_db;
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::comp_angular_seg                           ****/
/******************************************************************************************/
void SegmentAnglePropModelClass::comp_angular_seg(double cos_a, double sin_a, int &a_seg, int &a_seg_p1, double &alpha)
{
    int found, prev_i, i;
    double dc, ds, dc1, ds1;

    /* xxxxxxxx Optimize this using efficient table lookup scheme */
    if (n_angle == 1) {
        a_seg    = 0;
        a_seg_p1 = 0;
        alpha    = 1.0;
    } else {
        found = 0;
        prev_i = n_angle-1;
        for (i=0; (i<=n_angle-1)&&(!found); i++) {

            if (fabs(cos_a) < fabs(sin_a)) {
                if (    ((sin_a > 0.0) && (ss[i] > 0.0))
                     || ((sin_a < 0.0) && (ss[i] < 0.0)) ) {
                    if (    ((cc[prev_i] <= cos_a) && (cos_a <= cc[i]))
                         || ((cc[prev_i] >= cos_a) && (cos_a >= cc[i])) ) {
                        found = 1;
                        a_seg    = prev_i;
                        a_seg_p1 = i;
                    }
                }
            } else {
                if (    ((cos_a > 0.0) && (cc[i] > 0.0))
                     || ((cos_a < 0.0) && (cc[i] < 0.0)) ) {
                    if (    ((ss[prev_i] <= sin_a) && (sin_a <= ss[i]))
                         || ((ss[prev_i] >= sin_a) && (sin_a >= ss[i])) ) {
                        found = 1;
                        a_seg    = prev_i;
                        a_seg_p1 = i;
                    }
                }
            }

            prev_i = i;
        }

        // printf("ANGULAR_SEG: COS = %12.10f SIN = %12.10f A_SEG = %d A_SEG_P1 = %d\n", cos_a, sin_a, a_seg, a_seg_p1);

        dc  = cos_a - cc[a_seg];
        ds  = sin_a - ss[a_seg];
        dc1 = cos_a - cc[a_seg_p1];
        ds1 = sin_a - ss[a_seg_p1];

        alpha = (dc1*dc1 + ds1*ds1)/(dc*dc + ds*ds + dc1*dc1 + ds1*ds1);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::prop_power_loss                            ****/
/******************************************************************************************/
double SegmentAnglePropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight_for_dist, double angle_deg)
{
    int i, done, r_seg, a_seg, a_seg_p1, use_n;
    double prop_db = 0.0;
    double eff_slope, eff_ya, eff_yb;
    double cos_a, sin_a, alpha;

    double dz = (useheight_for_dist ? -sector->antenna_height : 0.0);
    double rsq = (delta_x*delta_x + delta_y*delta_y)*np->resolution*np->resolution + dz*dz;
    if (rsq < np->resolution) {
        rsq = np->resolution;
    }
    double d = 0.5*log(rsq)/log(10.0);

    /**************************************************************************************/
    /**** Determine radial segment number and use_n                                    ****/
    /**************************************************************************************/
    if ( d<=x[0] ) {
        r_seg = 0;
        use_n = use_n_y[0] | use_n_start_slope;
    } else if ( d>x[num_inflexion-1] ) {
        r_seg = num_inflexion;
        use_n = use_n_y[num_inflexion-1] | use_n_final_slope;
    } else {
        done = 0;
        for ( i=0; (i<num_inflexion-1)&&(!done); i++ ) {
            if ( (d>x[i]) && (d<=x[i+1]) ) {
                r_seg = i+1;
                done = 1;
            }
        }
        use_n = use_n_y[r_seg-1] | use_n_y[r_seg];
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Compute angular segment number                                               ****/
    /**************************************************************************************/
    if (use_n) {
        if (delta_x || delta_y) {
            cos_a = delta_x / sqrt( (double) delta_x*delta_x + (double) delta_y*delta_y );
            sin_a = delta_y / sqrt( (double) delta_x*delta_x + (double) delta_y*delta_y );
            if (angle_deg != 0.0) {
                double cos_angle = cos(angle_deg*PI/180.0);
                double sin_angle = sin(angle_deg*PI/180.0);
                double cos_0 = cos_a*cos_angle - sin_a*sin_angle;
                double sin_0 = cos_a*sin_angle + sin_a*cos_angle;
                cos_a = cos_0;
                sin_a = sin_0;
            }
        } else {
            cos_a = 1.0;
            sin_a = 0.0;
        }

        comp_angular_seg(cos_a, sin_a, a_seg, a_seg_p1, alpha);
    }
    /**************************************************************************************/

    if ( r_seg == 0 ) {
        eff_ya    = (use_n_y[0]        ? alpha*y[0][a_seg]        + (1.0-alpha)*y[0][a_seg_p1]        : y[0][0]);
        eff_slope = (use_n_start_slope ? alpha*start_slope[a_seg] + (1.0-alpha)*start_slope[a_seg_p1] : start_slope[0]);
        prop_db = eff_ya + eff_slope*(d-x[0]);
    } else if ( r_seg == num_inflexion ) {
        eff_ya    = (use_n_y[num_inflexion-1] ? alpha*y[num_inflexion-1][a_seg] + (1.0-alpha)*y[num_inflexion-1][a_seg_p1] : y[num_inflexion-1][0]);
        eff_slope = (use_n_final_slope ? alpha*final_slope[a_seg] + (1.0-alpha)*final_slope[a_seg_p1] : final_slope[0]);
        prop_db = eff_ya + eff_slope*(d-x[num_inflexion-1]);
    } else {
        eff_ya    = (use_n_y[r_seg-1] ? alpha*y[r_seg-1][a_seg] + (1.0-alpha)*y[r_seg-1][a_seg_p1] : y[r_seg-1][0]);
        eff_yb    = (use_n_y[r_seg  ] ? alpha*y[r_seg  ][a_seg] + (1.0-alpha)*y[r_seg  ][a_seg_p1] : y[r_seg  ][0]);

        prop_db = eff_ya + ((eff_ya-eff_yb)/(x[r_seg-1]-x[r_seg]))*(d-x[r_seg-1]);
    }

    CellClass *cell     = sector->parent_cell;
    double *dist_vector = DVECTOR(num_clutter_type);
    if (num_clutter_type) {
        np->get_path_clutter(cell->posn_x, cell->posn_y, cell->posn_x+delta_x, cell->posn_y+delta_y, dist_vector);
    }

    int k_idx = 0;
    if (useheight) {
        double h = log(sector->antenna_height)/log(10.0);
        prop_db += vec_k[k_idx++]*h;
        prop_db += vec_k[k_idx++]*h*d;
    }

    for ( int clutter_idx=0; clutter_idx<=num_clutter_type-1; clutter_idx++) {
        prop_db += vec_k[k_idx++]*dist_vector[clutter_idx];
    }

    double prop;
    if (prop_db < -2000.0) {
        prop = exp(-2000.0*log(10.0)/10.0);
    } else if (prop_db > max_level_db) {
        prop = exp(max_level_db*log(10.0)/10.0);
    } else {
        prop = exp(prop_db*log(10.0)/10.0);
    }

    if (num_clutter_type) {
        free(dist_vector);
    }

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::get_prop_model_param_ptr                   ****/
/******************************************************************************************/
void SegmentAnglePropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i, j, new_val, done, idx;
    static int old_val = 0;

    done = 0;
    idx = 0;
    if (!done) {
        if (param_idx == idx) {
            sprintf(str, "N:");                 type=CConst::NumericInt;    iptr = &num_inflexion;
            old_val = num_inflexion;
        }
        if (param_idx == idx+1) {
            new_val = num_inflexion;
            num_inflexion = old_val;
            set_num_inflexion(new_val);
        } else if (param_idx < idx+1) {
            done = 1;
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "N_ANGLE:");            type=CConst::NumericInt;    iptr = &n_angle;
        }
        if (param_idx < idx+1) {
            done = 1;
        }
    }
    for (i=0; (i<=num_inflexion-1)&&(!done); i++) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "X[%d]:", i);          type=CConst::NumericDouble; dptr = &(x[i]);
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "USE_N_Y[%d]:", i);    type=CConst::NumericInt;    iptr = &(use_n_y[i]);
        }
        if (param_idx == idx+1) {
            y[i] = (double *) realloc((void *) y[i], (use_n_y[i]?n_angle:1)*sizeof(double));
        }
        for (j=0; j<=(use_n_y[i]?n_angle:1)-1; j++) {
            idx++;
            if (param_idx == idx) {
                sprintf(str, "Y[%d][%d]:", i, j);        type=CConst::NumericDouble; dptr = &(y[i][j]);
            }
        }
        if (param_idx < idx+1) {
            done = 1;
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "USE_N_START_SLOPE:");    type=CConst::NumericInt; iptr = &use_n_start_slope;
        }
        if (param_idx == idx+1) {
            start_slope = (double *) realloc((void *) start_slope, (use_n_start_slope?n_angle:1)*sizeof(double));
        }
        for (j=0; j<=(use_n_start_slope?n_angle:1)-1; j++) {
            idx++;
            if (param_idx == idx) {
                sprintf(str, "START_SLOPE[%d]:", j);        type=CConst::NumericDouble; dptr = &(start_slope[j]);
            }
        }
        if (param_idx < idx+1) {
            done = 1;
        }
    }

    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "USE_N_FINAL_SLOPE:");       type=CConst::NumericInt; iptr = &use_n_final_slope;
        }
        if (param_idx == idx+1) {
            final_slope = (double *) realloc((void *) final_slope, (use_n_final_slope?n_angle:1)*sizeof(double));
        }
        for (j=0; j<=(use_n_final_slope?n_angle:1)-1; j++) {
            idx++;
            if (param_idx == idx) {
                sprintf(str, "FINAL_SLOPE[%d]:", j);        type=CConst::NumericDouble; dptr = &(final_slope[j]);
            }
        }
        if (param_idx < idx+1) {
            done = 1;
        }
    }
    if (!done) {
        idx++;
        if (param_idx == idx) {
            sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
        }
        idx++;
        if (param_idx == idx) {
            sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
        }
        if (param_idx == idx+1) {
            vec_k = (double *) realloc((void *) vec_k, (num_clutter_type + 2*useheight)*sizeof(double));
        }
        if (param_idx < idx+1) {
            done = 1;
        } else {
            idx += num_clutter_type + 2*useheight;
            if ((param_idx <= idx) && (num_clutter_type + 2*useheight)) {
                j = param_idx - idx + num_clutter_type + 2*useheight - 1;
                sprintf(str, "C_%d:", j);           type=CConst::NumericDouble; dptr = &(vec_k[j]);
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::comp_num_prop_param                        ****/
/******************************************************************************************/
int SegmentAnglePropModelClass::comp_num_prop_param()
{
    int i, n;

    n = 2;

    for (i=0; i<=num_inflexion-1; i++) {
        n += 2;
        if (use_n_y[i]) {
            n += n_angle;
        } else {
            n += 1;
        }
    }
    n++;
    if (use_n_start_slope) {
        n += n_angle;
    } else {
        n += 1;
    }
    n++;
    if (use_n_final_slope) {
        n += n_angle;
    } else {
        n += 1;
    }

    n += 2 + num_clutter_type + 2*useheight;

    return(n);
}
/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::print_params                               ****/
/******************************************************************************************/
void SegmentAnglePropModelClass::print_params(FILE *fp, char *msg, int fmt)
{
    int idx, k, c_idx;
    char *chptr, indent[10];

    switch(fmt) {
        case CConst::geometryFileType:
            strcpy(indent, "    ");
            break;
        case CConst::reportFileType:
            strcpy(indent, "");
            break;
        default:
            CORE_DUMP;
            break;
    }

    chptr = msg;
    chptr += sprintf(chptr, "%sTYPE: SEGMENT_ANGLE\n", indent);
    chptr += sprintf(chptr, "%sN: %d\n",               indent, num_inflexion);
    chptr += sprintf(chptr, "%sN_ANGLE: %d\n",         indent, n_angle);
    for( idx=0; idx<=num_inflexion-1; idx++ ) {
        chptr += sprintf(chptr, "%sX[%d]: %9.7f\n",            indent, idx,x[idx]);
        chptr += sprintf(chptr, "%sUSE_N_Y[%d]: %d\n",         indent, idx,use_n_y[idx]);
        for( k=0; k<=(use_n_y[idx] ? n_angle : 1)-1; k++ ) {
            chptr += sprintf(chptr, "%sY[%d][%d]: %9.7f\n", indent, idx, k, y[idx][k]);
        }
    }
    chptr += sprintf(chptr, "%sUSE_N_START_SLOPE: %d\n",      indent, use_n_start_slope);
    for( k=0; k<=(use_n_start_slope ? n_angle : 1)-1; k++ ) {
        chptr += sprintf(chptr, "%sSTART_SLOPE[%d]: %9.7f\n", indent, k, start_slope[k]);
    }
    chptr += sprintf(chptr, "%sUSE_N_FINAL_SLOPE: %d\n",      indent, use_n_final_slope);
    for( k=0; k<=(use_n_final_slope ? n_angle : 1)-1; k++ ) {
        chptr += sprintf(chptr, "%sFINAL_SLOPE[%d]: %9.7f\n", indent, k, final_slope[k]);
    }
    chptr += sprintf(chptr, "%sUSE_HEIGHT: %d\n",       indent, useheight);
    chptr += sprintf(chptr, "%sNUM_CLUTTER_TYPE: %d\n", indent, num_clutter_type);
    PRMSG(fp, msg);
    for (c_idx=0; c_idx<=num_clutter_type+2*useheight-1; c_idx++) {
        sprintf(msg, "%sC_%d: %9.7f\n", indent, c_idx, vec_k[c_idx]);
        PRMSG(fp, msg);
    }
}
/******************************************************************************************/
