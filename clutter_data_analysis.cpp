/*******************************************************************************************
**** PROGRAM: clutter_data_analysis.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/


#include <iostream>
#include <iomanip>
#include <string.h>

#include "antenna.h"
#include "bin_io.h"
#include "cconst.h"
#include "time.h"
#include "wisim.h"
#include "clutter_data_analysis.h"
#include "doubleintint.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "polygon.h"
#include "road_test_data.h"
#include "sparse_matrix.h"
#include "utm_conversion.h"


#if HAS_GUI
#include <qapplication.h>
#include "progress_slot.h"
extern int use_gui;
#endif


#define CLUTTER_PROP_MODEL_FORMAT "1.0"
static char *header = "WISIM_CLUTTER_PROP_MODEL_FILE";

/******************************************************************************************/
/**** Virtual Functions                                                                ****/
/******************************************************************************************/
const int GenericClutterPropModelClass::type()                                   { CORE_DUMP; return(-1); }
double    GenericClutterPropModelClass::prop_power_loss(NetworkClass *, SectorClass *, int, int, int, double) { CORE_DUMP; return(0.0); }
void      GenericClutterPropModelClass::get_prop_model_param_ptr(int, char *, int&, int *&, double *&) { CORE_DUMP; return; }
int       GenericClutterPropModelClass::comp_num_prop_param()                    { CORE_DUMP; return(-1); }
void      GenericClutterPropModelClass::print_params(FILE *, char *, int)        { CORE_DUMP; return; }
int       GenericClutterPropModelClass::is_clutter_model()                       { return(1); }
void      GenericClutterPropModelClass::split_clutter(NetworkClass *)            { CORE_DUMP; return; }
void      GenericClutterPropModelClass::clt_regulation( NetworkClass*, ListClass<int> *, int) { CORE_DUMP; return; }
void      GenericClutterPropModelClass::clt_fill(NetworkClass*, ListClass<int>*) { CORE_DUMP; return; }
int       GenericClutterPropModelClass::get_color(int, double, double)           { CORE_DUMP; return(-1); }
void      GenericClutterPropModelClass::get_min_max_color(double &, double &)    { CORE_DUMP; return; }
void      GenericClutterPropModelClass::report_clutter(NetworkClass *, FILE *, PolygonClass *)   { CORE_DUMP; return; }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::GenericClutterPropModelClass()           ****/
/******************************************************************************************/
GenericClutterPropModelClass::GenericClutterPropModelClass(char *p_strid ) : PropModelClass(p_strid) 
{
    //defalt value
    num_clutter_type = 0;
    useheight        = 0;
    mvec_x           = (double *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::~GenericClutterPropModelClass()          ****/
/******************************************************************************************/
GenericClutterPropModelClass::~GenericClutterPropModelClass()
{
    free ( mvec_x );
}

/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::create_map_bdy                           ****/
/******************************************************************************************/
PolygonClass *GenericClutterPropModelClass::create_map_bdy()
{
    ListClass<IntIntClass> *ii_list = new ListClass<IntIntClass>(4);

    ii_list->append(IntIntClass(offset_x-1,                              offset_y-1));
    ii_list->append(IntIntClass(offset_x + clutter_sim_res_ratio*npts_x, offset_y-1));
    ii_list->append(IntIntClass(offset_x + clutter_sim_res_ratio*npts_x, offset_y + clutter_sim_res_ratio*npts_y));
    ii_list->append(IntIntClass(offset_x-1,                              offset_y + clutter_sim_res_ratio*npts_y));

    PolygonClass *map_bdy = new PolygonClass(ii_list);
    delete ii_list;

    return(map_bdy);
}
/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::refine_clutter                           ****/
/******************************************************************************************/
void GenericClutterPropModelClass::refine_clutter(NetworkClass *np, int n, ListClass<int> *int_list)
{
    int i;


#if HAS_GUI
    ProgressSlot* prog_bar;
    int curr_prog = 0;
    if (use_gui) {
        prog_bar = new ProgressSlot(0, "Progress Bar", qApp->translate("ProgressSlot", "Running Clutter Simulation - Refine Clutter ") + "...");
    }
#endif


#if OUTPUT_TIME_INFOR
    time_t td;
#endif


    if ( (clutter_sim_res_ratio&((1<<n)-1)) == 1 ) {
        sprintf(np->msg, "ERROR: clutter_sim_res_ratio = %d, value must be multiple of %d.\n", clutter_sim_res_ratio, 1<<n);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

   if (    (type() != CConst::PropClutterWtExpoSlope)
        && (type() != CConst::PropClutterExpoLinear)
        && (type() != CConst::PropClutterGlobal)) {
        sprintf(np->msg, "ERROR: Specified propagation model has is not of type %s\n", "CLUTTERWTEXPOSLOPE, CLUTTEREXPOLINEAR or CLUTTERGLOBAL");
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    for (i=0; i<=n-1; i++) {
        clutter_sim_res_ratio /= 2;
        npts_x *= 2;
        npts_y *= 2;
        num_clutter_type = npts_x * npts_y;

#if HAS_GUI
        curr_prog = (int) 100.0*(2*(i+1)-1)/(2*n);
        if (use_gui) {
            prog_bar->set_prog_percent(curr_prog);
        }
#endif

        split_clutter(np);

#if HAS_GUI
        curr_prog = (int) 100.0*2*(i+1)/(2*n);
        if (use_gui) {
            prog_bar->set_prog_percent(curr_prog);
        }
#endif

        if (!np->error_state) {
            clt_regulation(np, int_list, 1);
        }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n---- Time ---- %s", ctime(&td));

        sprintf(np->msg, "---- Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif

    }

    if (!np->error_state) {
        clt_fill(np, int_list);
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif

#if HAS_GUI
    if (use_gui) {
        delete prog_bar;
    }
#endif

    return;

}

/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::save                                     ****/
/**** Save clutter propagation model to the specified file, clutter data is stored in  ****/
/**** a binary format.                                                                 ****/
/******************************************************************************************/
void GenericClutterPropModelClass::save(NetworkClass *np, char *filename)
{
#if (DEMO == 0)
    int map_startx, map_starty, num_var, var_idx, ntype, *iptr;
    double resolution, *dptr;
    char *chptr;
    FILE *fp;

    if ( !(fp = fopen(filename, "wb")) ) {
        sprintf(np->msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    sprintf(np->msg, "Writing map clutter file: %s\n", filename);
    PRMSG(stdout, np->msg);

    /**************************************************************************************/
    /**** HEADER                                                                       ****/
    /**************************************************************************************/
    fwrite((void *) header, sizeof(char), strlen(header), fp);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** VERSION                                                                      ****/
    /**************************************************************************************/
    write_fs_str(fp, CLUTTER_PROP_MODEL_FORMAT);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** TYPE                                                                         ****/
    /**************************************************************************************/
    switch(type()) {
        case CConst::PropClutterWtExpoSlope:
            sprintf(np->msg, "CLUTTERWTEXPOSLOPE");
            num_var = num_clutter_type + useheight + 1;
            break;
        case CConst::PropClutterExpoLinear:
            sprintf(np->msg, "CLUTTEREXPOLINEAR");
            num_var = num_clutter_type + useheight + 1;
            break;
        case CConst::PropClutterGlobal:
            sprintf(np->msg, "CLUTTERGLOBAL");
            num_var = num_clutter_type + useheight + 1;
            break;
        default: CORE_DUMP; break;
    }
    write_fs_str(fp, np->msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** COORDINATE SYSTEM                                                            ****/
    /**************************************************************************************/
    chptr = np->msg;
    if (np->coordinate_system == CConst::CoordGeneric) {
        chptr += sprintf(chptr, "GENERIC");
    } else if (np->coordinate_system == CConst::CoordUTM) {
        chptr += sprintf(chptr, "UTM:%.0f:%.9f:%d:%c", np->utm_equatorial_radius, np->utm_eccentricity_sq,
            np->utm_zone, (np->utm_north ? 'N' : 'S'));
    } else if (np->coordinate_system == CConst::CoordLONLAT) {
        chptr += sprintf(chptr, "LON_LAT");
    } else {
        CORE_DUMP;
    }
    write_fs_str(fp, np->msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** RESOLUTION                                                                   ****/
    /**************************************************************************************/
    resolution = np->resolution * clutter_sim_res_ratio;
    write_fs_double(fp, resolution);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** XSTART, YSTART, XSIZE, YSIZE                                                 ****/
    /**************************************************************************************/
    map_startx = (np->system_startx + offset_x) / clutter_sim_res_ratio;
    map_starty = (np->system_starty + offset_y) / clutter_sim_res_ratio;

    write_fs_int(fp, map_startx);
    write_fs_int(fp, map_starty);

    write_fs_int(fp, npts_x);
    write_fs_int(fp, npts_y);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** NAME                                                                         ****/
    /**************************************************************************************/
    write_fs_str(fp, get_strid());
    /**************************************************************************************/

    /**************************************************************************************/
    /**** NUM CLUTTER TYPE                                                             ****/
    /**************************************************************************************/
    write_fs_int(fp, num_clutter_type);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** USEHEIGHT                                                                    ****/
    /**************************************************************************************/
    write_fs_uchar(fp, (unsigned char) useheight);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** MVEC_X (clutter coefficients)                                                ****/
    /**************************************************************************************/
    for (var_idx=0; var_idx<=num_var-1; var_idx++) {
        write_fs_double(fp, mvec_x[var_idx]);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** TYPE SPECIFIC PARAMETERS                                                     ****/
    /**************************************************************************************/
    switch(type()) {
        case CConst::PropClutterWtExpoSlope:
            get_prop_model_param_ptr(7, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "R0:")!=0) {
                CORE_DUMP;
            }
            write_fs_double(fp, *dptr);
            break;
        case CConst::PropClutterExpoLinear:
            get_prop_model_param_ptr(7, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "R0:")!=0) {
                CORE_DUMP;
            }
            write_fs_double(fp, *dptr);
            get_prop_model_param_ptr(8, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "EXPONENT:")!=0) {
                CORE_DUMP;
            }
            write_fs_double(fp, *dptr);
            break;

        // global model parameters
        case CConst::PropClutterGlobal:
            SegmentPropModelClass* gpm;
            int num_inflexion;
            int i, j, idx;

            char* str;
            str = (char*) malloc ( 100*sizeof(char) );

            get_prop_model_param_ptr(7, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "R0:") != 0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            get_prop_model_param_ptr(8, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "N:") != 0) CORE_DUMP;
            write_fs_int(fp, *iptr);

            // write inflexion coordination info to output binary file
            num_inflexion = *iptr;
            for ( idx=9; idx<2*num_inflexion+9; idx++ )
            {
                i = idx-9;
                j = i>>1;
                if ((i&1) == 0) sprintf(str, "X[%d]:", j); // i is even number
                else            sprintf(str, "Y[%d]:", j); // i is odd number

                get_prop_model_param_ptr(idx, np->msg, ntype, iptr, dptr);
                if (strcmp(np->msg, str) != 0) CORE_DUMP;
                write_fs_double(fp, *dptr);
            }

            get_prop_model_param_ptr(2*num_inflexion+9, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "S:") != 0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            get_prop_model_param_ptr(2*num_inflexion+10, np->msg, ntype, iptr, dptr);
            if (strcmp(np->msg, "F:") != 0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            free (str);

            break;

        default: CORE_DUMP; break;
    }
    /**************************************************************************************/

    fclose(fp);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::read_clutter_model                                       ****/
/**** Read clutter propagation model from the specified file.                          ****/
/******************************************************************************************/
int NetworkClass::read_clutter_model(char *filename, int force_read)
{
#if (DEMO == 0)
    int pm_idx;
    char *format_str;
    FILE *fp;

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file 2 %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return(-1);
    }

    /**************************************************************************************/
    /**** HEADER                                                                       ****/
    /**************************************************************************************/
    char *read_header = CVECTOR(strlen(header));
    if (fread(read_header, sizeof(char), strlen(header), fp) != strlen(header)) {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\", file header does not match\n", filename);
        PRMSG(stdout, msg); error_state = 1; fclose(fp);
        return(-1);
    }
    read_header[strlen(header)] = (char) NULL;
    if (strcmp(read_header, header) != 0) {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\", file header does not match\n", filename);
        PRMSG(stdout, msg); error_state = 1; fclose(fp);
        return(-1);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** VERSION                                                                      ****/
    /**************************************************************************************/
    format_str = read_fs_str_allocate(fp);

    if (strcmp(format_str,"1.0")==0) {
        pm_idx = read_clutter_model_1_0(fp, filename, force_read);
    } else {
        sprintf(msg, "ERROR: clutter propagation model file \"%s\" has invalid format \"%s\"\n", filename, format_str);
        PRMSG(stdout, msg);
        error_state = 1;
        return(-1);
    }
    /**************************************************************************************/

    free(read_header);
    free(format_str);

    fclose(fp);

    return(pm_idx);
#else
    return(-1);
#endif
}
/******************************************************************************************/
/**** FUNCTION: read_clutter_model_1_0                                                 ****/
/**** Read map clutter file with format 1.0                                            ****/
/**** Clutter data is stored in a binary format.                                       ****/
/******************************************************************************************/
int NetworkClass::read_clutter_model_1_0(FILE *fp, char *filename, int force_read)
{
#if (DEMO == 0)
    int map_startx, map_starty;
    int num_var, var_idx, has_geometry, utm_zone_mismatch;
    int ntype, *iptr;
    double clutter_res, lon_deg, lat_deg, utm_x, utm_y, *dptr;
    char *type_str, *coord_sys_str, *str1, *chptr;
    GenericClutterPropModelClass *pm;

    sprintf(msg, "Reading clutter propagation model file: %s\n", filename);
    PRMSG(stdout, msg);

    sprintf(msg, "%s: clutter propagation model file \"%s\" COORDINATE_SYSTEM does not match\n", (force_read ? "WARNING" : "ERROR"), filename);

    if (!system_bdy) {
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    /**************************************************************************************/
    /**** TYPE                                                                         ****/
    /**************************************************************************************/
    type_str = read_fs_str_allocate(fp);

    if (strcmp(type_str, "CLUTTERWTEXPOSLOPE")==0) {
        pm = (GenericClutterPropModelClass *) new ClutterWtExpoSlopePropModelClass((char *) NULL);
    } else if (strcmp(type_str, "CLUTTEREXPOLINEAR")==0) {
        pm = (GenericClutterPropModelClass *) new ClutterExpoLinearPropModelClass((char *) NULL);
    } else if (strcmp(type_str, "CLUTTERGLOBAL")==0) {
        pm = (GenericClutterPropModelClass *) new ClutterGlobalPropModelClass((char *) NULL);
    } else {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\"\n"
                     "Unrecognized propagation model type: \"%s\"\n", filename, type_str);
        PRMSG(stdout, msg);
        error_state = 1;
        return(-1);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** COORDINATE SYSTEM                                                            ****/
    /**************************************************************************************/
    coord_sys_str = read_fs_str_allocate(fp);
    str1 = coord_sys_str;
    if (strcmp(str1, "GENERIC")==0) {
        if (has_geometry) {
            if (coordinate_system != CConst::CoordGeneric) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); } else { warning_state = 1; }
            }
        } else {
            coordinate_system = CConst::CoordGeneric;
        }
    } else if (strncmp(str1, "UTM:", 4)==0) {
        if (has_geometry) {
            if (coordinate_system != CConst::CoordUTM) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); }
            }
        } else {
            coordinate_system = CConst::CoordUTM;
        }
        str1 = strtok(str1+4, ":");
        if (has_geometry) {
            if (utm_equatorial_radius != atof(str1)) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); }
            }
        } else {
            utm_equatorial_radius = atof(str1);
        }
        str1 = strtok(NULL, ":");
        if (has_geometry) {
            if (utm_eccentricity_sq != atof(str1)) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); }
            }
        } else {
            utm_eccentricity_sq = atof(str1);
        }
        str1 = strtok(NULL, ":");
        utm_zone = atoi(str1);
        if (has_geometry) {
            if (utm_zone != utm_zone) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); }
                utm_zone_mismatch = 1;
            }
        } else {
            utm_zone = utm_zone;
        }
        str1 = strtok(NULL, ":");
        if (has_geometry) {
            if (strcmp(str1, (utm_north ? "N" : "S")) != 0) {
                PRMSG(stdout, msg);
                if (!force_read) { error_state = 1; return(-1); }
                utm_zone_mismatch = 1;
                if (strcmp(str1, "N") == 0) {
                    utm_north = 1;
                } else if (strcmp(str1, "S") == 0) {
                    utm_north = 0;
                } else {
                    utm_north = utm_north;
                }
            }
        } else {
            utm_north = (strcmp(str1,"N")==0 ? 1 : 0);
        }
    } else {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\" unrecognized coordinate system \"%s\"\n",
                filename, coord_sys_str);
        PRMSG(stdout, msg); error_state = 1;
        return(-1);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** RESOLUTION                                                                   ****/
    /**************************************************************************************/
    // CG MODI 11/14/2006  read_fs_double(fp) < 0 ?
    clutter_res = fabs(read_fs_double(fp));

    if (clutter_res <= 0.0) {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\"\n"
                         "resolution = %15.10f must be > 0.0\n", filename, clutter_res);
        PRMSG(stdout, msg);
        error_state = 1;
        return(-1);
    }

    if (!has_geometry) {
        resolution = clutter_res;
        res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
    }
    if (!check_grid_val(clutter_res, resolution, 0, &pm->clutter_sim_res_ratio)) {
        sprintf(msg, "ERROR: invalid clutter propagation model file \"%s\"\n"
                     "ERROR: clutter propagation model file resolution is not an integer multiple of simulation resolution\n"
                     "Clutter resolution = %15.10f simulation resolution = %15.10f\n",
                filename, clutter_res, resolution);
        PRMSG(stdout, msg); error_state = 1;
        return(-1);
    }

    chptr = msg;
    chptr += sprintf(chptr, "Clutter resolution %15.10f\n", clutter_res);
    chptr += sprintf(chptr, "Simulation resolution %15.10f\n", resolution);
    chptr += sprintf(chptr, "Ratio: %d\n", pm->clutter_sim_res_ratio);
    PRMSG(stdout, msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** XSTART, YSTART, XSIZE, YSIZE                                                 ****/
    /**** Create system boundary if it does not exist.                                 ****/
    /**************************************************************************************/
    map_startx = read_fs_int(fp);
    map_starty = read_fs_int(fp);

// xxxxxxxxxx    map_startx = (np->system_startx + offset_x) / clutter_sim_res_ratio;
// xxxxxxxxxx    map_starty = (np->system_starty + offset_y) / clutter_sim_res_ratio;

    if (utm_zone_mismatch) {
        UTMtoLL( map_startx*pm->clutter_sim_res_ratio*resolution, map_starty*pm->clutter_sim_res_ratio*resolution,
                 lon_deg,  lat_deg, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
        LLtoUTM( lon_deg, lat_deg, utm_x,  utm_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
        check_grid_val(utm_x, resolution, 0, &map_startx);
        check_grid_val(utm_y, resolution, 0, &map_starty);
    }

    pm->npts_x = read_fs_int(fp);
    pm->npts_y = read_fs_int(fp);

    if (!has_geometry) {
        system_bdy = new PolygonClass();
        system_bdy->num_segment = 1;
        system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
        system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
        system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
        system_bdy->num_bdy_pt[0] = 4;
        system_bdy->bdy_pt_x[0] = IVECTOR(4);
        system_bdy->bdy_pt_y[0] = IVECTOR(4);
        system_bdy->bdy_pt_x[0][0] = 0;
        system_bdy->bdy_pt_y[0][0] = 0;
        system_bdy->bdy_pt_x[0][1] = pm->npts_x-1;
        system_bdy->bdy_pt_y[0][1] = 0;
        system_bdy->bdy_pt_x[0][2] = pm->npts_x-1;
        system_bdy->bdy_pt_y[0][2] = pm->npts_y-1;
        system_bdy->bdy_pt_x[0][3] = 0;
        system_bdy->bdy_pt_y[0][3] = pm->npts_y-1;
        system_startx = map_startx;
        system_starty = map_starty;
        npts_x        = pm->npts_x;
        npts_y        = pm->npts_y;
        num_antenna_type = 1;
        antenna_type_list = (AntennaClass **) malloc(num_antenna_type*sizeof(AntennaClass *));
        antenna_type_list[0] = new AntennaClass(CConst::antennaOmni, "OMNI");
    }

    pm->offset_x = map_startx - system_startx;
    pm->offset_y = map_starty - system_starty;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** NAME                                                                         ****/
    /**************************************************************************************/
    pm->set_strid(read_fs_str_allocate(fp), 0);
    sprintf(msg, "Name = \"%s\"\n", pm->get_strid());
    PRMSG(stdout, msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** NUM CLUTTER TYPE                                                             ****/
    /**************************************************************************************/
    pm->num_clutter_type = read_fs_int(fp);
    sprintf(msg, "Num Clutter Types = %d\n", pm->num_clutter_type);
    PRMSG(stdout, msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** USEHEIGHT                                                                    ****/
    /**************************************************************************************/
    pm->useheight= (int) read_fs_uchar(fp);
    sprintf(msg, "Use Height = %d\n", pm->useheight);
    PRMSG(stdout, msg);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** MVEC_X (clutter coefficients)                                                ****/
    /**************************************************************************************/
    switch(pm->type()) {
        case CConst::PropClutterWtExpoSlope:
            num_var = pm->num_clutter_type + pm->useheight + 1;
            break;
        case CConst::PropClutterExpoLinear:
            num_var = pm->num_clutter_type + pm->useheight + 1;
            break;
        case CConst::PropClutterGlobal:
            num_var = pm->num_clutter_type + pm->useheight + 1;
            break;
        default: CORE_DUMP; break;
    }
    pm->mvec_x = DVECTOR(num_var);

    for (var_idx=0; var_idx<=num_var-1; var_idx++) {
        pm->mvec_x[var_idx] = read_fs_double(fp);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** TYPE SPECIFIC PARAMETERS                                                     ****/
    /**************************************************************************************/
    switch(pm->type()) {
        case CConst::PropClutterWtExpoSlope:
            pm->get_prop_model_param_ptr(7, msg, ntype, iptr, dptr);
            if (strcmp(msg, "R0:")!=0) {
                CORE_DUMP;
            }
            *dptr = read_fs_double(fp);
            break;
        case CConst::PropClutterExpoLinear:
            pm->get_prop_model_param_ptr(7, msg, ntype, iptr, dptr);
            if (strcmp(msg, "R0:")!=0) {
                CORE_DUMP;
            }
            *dptr = read_fs_double(fp);
            pm->get_prop_model_param_ptr(8, msg, ntype, iptr, dptr);
            if (strcmp(msg, "EXPONENT:")!=0) {
                CORE_DUMP;
            }
            *dptr = read_fs_double(fp);
            break;
        case CConst::PropClutterGlobal:
            int num_inflexion;
            int i, j, idx;

            char* str;
            // allocate space before use sprintf
            str = (char*) malloc ( 100*sizeof(char));

            pm->get_prop_model_param_ptr(7, msg, ntype, iptr, dptr);
            if (strcmp(msg, "R0:") != 0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            pm->get_prop_model_param_ptr(8, msg, ntype, iptr, dptr);
            if (strcmp(msg, "N:") != 0) CORE_DUMP;
            write_fs_int(fp, *iptr);

            // write inflexion coordination info to output binary file
            num_inflexion = *iptr;
            for ( idx=9; idx<2*num_inflexion+9; idx++ )
            {
                i = idx-9;
                j = i>>1;
                if ((i&1) == 0) sprintf(str, "X[%d]:", j); // i is even number
                else            sprintf(str, "Y[%d]:", j); // i is odd number

                pm->get_prop_model_param_ptr(idx, msg, ntype, iptr, dptr);
                if (strcmp(msg, str) != 0) CORE_DUMP;
                write_fs_double(fp, *dptr);
            }

            pm->get_prop_model_param_ptr(2*num_inflexion+9, msg, ntype, iptr, dptr);
            if (strcmp(msg, "S:")!=0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            pm->get_prop_model_param_ptr(2*num_inflexion+10, msg, ntype, iptr, dptr);
            if (strcmp(msg, "F:") != 0) CORE_DUMP;
            write_fs_double(fp, *dptr);

            free (str);

            break;
        default: CORE_DUMP; break;
    }
    /**************************************************************************************/

    num_prop_model++;
    prop_model_list = (PropModelClass **) realloc( (void *) prop_model_list, num_prop_model*sizeof(PropModelClass *));
    prop_model_list[num_prop_model-1] = pm;

    free(coord_sys_str);

    return(num_prop_model-1);
#else
    return(-1);
#endif

}
/******************************************************************************************/
/*
 */
ClutterPropModelClass::ClutterPropModelClass(char *p_strid ) : GenericClutterPropModelClass(p_strid) 
{
}

/*
 */
ClutterPropModelClass::~ClutterPropModelClass()
{
    free ( mvec_x );
}

const int ClutterPropModelClass::type() { return CConst::PropClutterSimp; }

/* 
   Compute the Path Loss from point(cs_x, cs_y) to point(pt_x, pt_y).
 */
double ClutterPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int dx, int dy, int /* useheight */, double angle_deg)
{
    CellClass *cell = sector->parent_cell;
    MapClutterClass* map_clutter = np->map_clutter;
    /*
        As for ald algorithm this vector save the distance of passing clutters, 
        and for new algorithm this vector save the divide value of cumulative sum distance of passing clutters
        (D(i)/D(i-1), i is the passed clutter index, D is the cumulative sum distance of passed clutters).
     */
    divDistStruct* divD_vector = (divDistStruct*) malloc ( num_clutter_type * sizeof ( divDistStruct ) );

    int    clutter_idx    = 0;
    int    index          = 0;
    double prop_dB        = 0.0;

    // Values with unit system resolution. 
    int offset_rx = map_clutter->offset_x;
    int offset_ry = map_clutter->offset_y;
    int cs_rx = cell->posn_x;
    int cs_ry = cell->posn_y;
    int pt_rx = 1;
    int pt_ry = 1;
    int c_rdx = 1;
    int c_rdy = 1;

    // Values with unit m, i.e. true distance.
    double offset_x = offset_rx * (np->resolution);
    double offset_y = offset_ry * (np->resolution);
    double cs_x = (double) cs_rx * np->resolution;
    double cs_y = (double) cs_ry * np->resolution;
    double dz   = -sector->antenna_height;
    double pt_x = np->resolution;
    double pt_y = np->resolution;
    double c_dx = np->resolution;
    double c_dy = np->resolution;
    double logh    = 0.0;
    double sq_dist = 0.0; 
    double antenna_height = -dz;

    if ( fabs( angle_deg ) < DERROR  ) {
        c_rdx = dx;
        c_rdy = dy;
        c_dx  = (double) dx * np->resolution;
        c_dy  = (double) dy * np->resolution;
    } else {
        c_rdx = (int)(dx*cos(angle_deg)-dy*sin(angle_deg));
        c_rdy = (int)(dx*sin(angle_deg)+dy*cos(angle_deg));
        c_dx = (double) ( dx*cos(angle_deg)-dy*sin(angle_deg) ) * np->resolution;
        c_dy = (double) ( dx*sin(angle_deg)+dy*cos(angle_deg) ) * np->resolution;
    }
    sq_dist = (double) (c_dx*c_dx + c_dy*c_dy + dz*dz);
    if ( sq_dist < DERROR ) sq_dist = 1.0;
    antenna_height = -dz;
    logh = log(antenna_height)/log(10.0);
        
    pt_rx = cs_rx + c_rdx;
    pt_ry = cs_ry + c_rdy;
    pt_x = cs_x + c_dx;
    pt_y = cs_y + c_dy;
    
    for (clutter_idx=0; clutter_idx<num_clutter_type; clutter_idx++) {
        divD_vector[clutter_idx].divD  = 1.0;
        divD_vector[clutter_idx].c_idx = 0;
    }
    
    int    clutter_type = 0;         //Keep the value of passed clutter type.
    int    pass_clt = 0;             //Keep the number of Passed clutter type.
    int    pass_idx = 0;             //Keep the passed clutter index.
    double divDist  = 1.0;           //Keep the value of D(i)/D(i-1).
    int    n(0), si(0), sj(0), pi(0), pj(0), q(0);
    int    map_i      = 0;
    int    map_j      = 0;
    int    map_i_1    = 0;
    int    map_j_1    = 0;
    double prev_dist  = np->resolution;
    double curr_dist  = np->resolution;
    double total_dist = np->resolution;
    double dmin       = np->resolution;
    
    n = map_clutter->map_sim_res_ratio;
    
    si = ((pt_x-cs_x>=0) ? 1 : -1);
    sj = ((pt_y-cs_y>=0) ? 1 : -1);
    pi = ((pt_x-cs_x>=0) ? 1 : 0);
    pj = ((pt_y-cs_y>=0) ? 1 : 0);
    
    total_dist = sqrt( (pt_x-cs_x)*(pt_x-cs_x) + (pt_y-cs_y)*(pt_y-cs_y) );
    
    map_i = (cs_rx-offset_rx)/n;
    map_j = (cs_ry-offset_ry)/n;
    map_i_1 = DIV(pt_rx-offset_rx, n);
    map_j_1 = DIV(pt_ry-offset_ry, n);
    
    bool first_scan = true;
    bool on_bdy     = false;
    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {
        
        clutter_type = map_clutter->get_clutter_type(map_i, map_j);
        q = (n*((map_j+pj)*(pt_rx-cs_rx)-(map_i+pi)*(pt_ry-cs_ry)) + (pt_ry-offset_ry)*(cs_rx-offset_rx) - (pt_rx-offset_rx)*(cs_ry-offset_ry))*si*sj;
        
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            curr_dist = total_dist*(n*(map_i+pi) - cs_x + offset_x)/(pt_x-cs_x);
            map_i += si;
        } else {
            curr_dist = total_dist*(n*(map_j+pj) - cs_y + offset_y)/(pt_y-cs_y);
            map_j += sj;
        }
        
#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
                map_i, map_j, curr_dist, prev_dist, curr_dist - prev_dist, clutter_type, map_clutter->description[clutter_type]);
#endif
        //Fill Sum-Distance to the correct position of matrix mmx_a. 
        if ( first_scan && fabs (curr_dist) > DERROR ) {
            first_scan = false;
            prev_dist  = dmin;
            divDist = curr_dist/prev_dist;
            divD_vector[pass_idx].divD = divDist;
            divD_vector[pass_idx].c_idx = clutter_type;
        } else if ( first_scan && fabs (curr_dist) < DERROR ) {
            first_scan = false;
            on_bdy = true;
        } else { 
            if ( on_bdy ) prev_dist = dmin;
            divDist = curr_dist/prev_dist;
            
            divD_vector[pass_idx].divD = divDist;
            divD_vector[pass_idx].c_idx = clutter_type;
            on_bdy = false;
        }
        prev_dist = curr_dist;
        pass_idx++;
    }
    curr_dist = total_dist;
    
    clutter_type = map_clutter->get_clutter_type(map_i, map_j);
    
    if ( first_scan && fabs (curr_dist) > DERROR ) {
        prev_dist  = dmin;
        divDist = curr_dist/prev_dist;
    } else if ( first_scan && fabs (curr_dist) < DERROR )
        divDist = 1.0;
    else divDist = curr_dist/prev_dist;
    
    if ( first_scan && fabs (curr_dist) > DERROR ) {
        prev_dist  = dmin;
        divDist = curr_dist/prev_dist;
        divD_vector[pass_idx].divD = divDist;
        divD_vector[pass_idx].c_idx = clutter_type;
    } else if ( first_scan && fabs (curr_dist) < DERROR ) {
        on_bdy = true;
        divD_vector[pass_idx].divD = 1.0;
        divD_vector[pass_idx].c_idx = clutter_type;
    } else { 
        if ( on_bdy ) prev_dist = dmin;
        divDist = curr_dist/prev_dist;
        divD_vector[pass_idx].divD = divDist;
        divD_vector[pass_idx].c_idx = clutter_type;
        on_bdy = false;
    }
    //Pass_clt save the count of passed clutters;
    pass_clt = pass_idx;
    
#if CLUTTER_DEBUG
    printf("map_i = %d map_j = %d curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
            map_i, map_j, curr_dist, prev_dist, curr_dist - prev_dist, clutter_type, map_clutter->description[clutter_type]);
#endif
    
    if (useheight) {
        prop_dB += mvec_x[index ++]*logh;
    }

    //prop_dB += mvec_x[index ++] * b;
    prop_dB += b;
    prop_dB += mvec_x[divD_vector[0].c_idx+useheight]*( log(divD_vector[0].divD)/log(10.0) + k );
    for ( pass_idx=1; pass_idx<=pass_clt; pass_idx++) {
        prop_dB += mvec_x[divD_vector[pass_idx].c_idx+useheight]*log(divD_vector[pass_idx].divD)/log(10.0);
    }
    
    double prop = exp(prop_dB*log(10.0)/10.0);
    
#if CLUTTER_DEBUG
    std::cout << "Power loss dB = " << prop_dB << std::endl;
#endif
    
    free ( divD_vector );
    
    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::compute_error                                   ****/
/******************************************************************************************/
double ClutterPropModelClass::compute_error(int num_road_test_pt, double **clutter_dis, double *power, int plot, char *flname)
{
    int       pt_idx      = 0; 
    int       clutter_idx = 0;
    double    diff        = 0.0;
    double    error       = 0.0;
    FILE      *fp         = NULL;     

    if (plot) {
        if ( !(fp = fopen(flname, "w")) ) {
            fprintf(stderr, "\nERROR: Unable to write to file %s\n", flname);
            exit(1);
        }
    }

    for (pt_idx=0; pt_idx<num_road_test_pt; pt_idx++) {
        diff = 0.0;

        if( useheight != 0 ) {
            diff += mvec_x[0]*clutter_dis[pt_idx][0];
        }

        //diff += mvec_x[useheight] * b;
        for (clutter_idx=0; clutter_idx<num_clutter_type; clutter_idx++) {
            if ( fabs(clutter_dis[pt_idx][clutter_idx+useheight] ) > DERROR )
#if 0
            if ( clutter_idx == a_posn[pt_idx] )  
                diff += mvec_x[clutter_idx+useheight+1]*( clutter_dis[pt_idx][clutter_idx+useheight+1] + k );   // Matrix clutter_dis has added the value of k. 
            else 
#endif
            //diff += mvec_x[clutter_idx+useheight+1]*clutter_dis[pt_idx][clutter_idx+useheight+1]; 
            diff += mvec_x[clutter_idx+useheight]*clutter_dis[pt_idx][clutter_idx+useheight]; 
        }

        diff -= power[pt_idx];
        error += diff*diff;
        if (plot)
            fprintf(fp, "%15.10f\n", diff);
    }


#if CLUTTER_DEBUG 
    if (plot) {
        fprintf(fp, "\n");
        fclose(fp);
    }
#endif

    return(error);
}


/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::get_prop_model_param_ptr                        ****/
/******************************************************************************************/
void ClutterPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 4) {
        mvec_x = DVECTOR(num_clutter_type + useheight);
    }

    if (param_idx == 0 ) {
        sprintf(str, "USE_HEIGHT:");   type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 1 ) {
        sprintf(str, "SLOPE:");        type=CConst::NumericDouble;    dptr = &k;
    } else if (param_idx == 2 ) {
        sprintf(str, "INTERCEPT:");    type=CConst::NumericDouble;    dptr = &b;
    } else if (param_idx == 3 ) {
        sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
    } else {
        i = param_idx-4;
        sprintf(str, "C_%d:", i);      type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::comp_num_prop_param                             ****/
/******************************************************************************************/
int  ClutterPropModelClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + num_clutter_type + 4;

    // DBG
    //std::cout << " ClutterPropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}


/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::print_params                                    ****/
/******************************************************************************************/
void ClutterPropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTER\n");
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",  useheight);
    chptr += sprintf(chptr, "    SLOPE: %f\n",       k );
    chptr += sprintf(chptr, "    INTERCEPT: %f\n",   b );
    chptr += sprintf(chptr, "    NUM_CLUTTER_TYPE: %d\n", num_clutter_type);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<num_clutter_type; c_idx++) {
        sprintf(msg, "    C_%d: %9.7f\n", c_idx+useheight, mvec_x[useheight+c_idx]);
        PRMSG(fp, msg);
    }
}

/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::clt_regulation()                                  ****/
/******************************************************************************************/
void ClutterPropModelClass::clt_regulation( NetworkClass* np )
{
    int  map_i   = 0;
    int  map_j   = 0;
    int  clt_idx = 0;
    int  div_num = 0;
    bool nozero  = true;
    bool allzero = true;
    struct scanStruct {
        double m;
        bool scanned;
    };
    scanStruct* scan_vec = (scanStruct*) malloc ( num_clutter_type * sizeof ( scanStruct ) ); 

    MapClutterClass* map_clutter = np->map_clutter;

    int num_clt_col = map_clutter->get_clutter_type(0, 0) - 
        map_clutter->get_clutter_type(0, 1);
    int num_clt_row = num_clutter_type/num_clt_col; 


    // Scanning all the clutters to make sure there is more than one clutter with none zero coeficient.
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) 
    {
        if ( doubleEqual( mvec_x[clt_idx+useheight], 0.0 ) ) {
            scan_vec[clt_idx].scanned = false;
            nozero = false;
        } 
        else { 
            // 2005.3.31 MODI CG
#if !CLUTTER_DEBUG
            if ( mvec_x[clt_idx+useheight] > 1.0 )
                mvec_x[clt_idx+useheight] = 1.0;
            if ( mvec_x[clt_idx+useheight] < -300 )
                mvec_x[clt_idx+useheight] = -300 ;
#endif
            scan_vec[clt_idx].scanned = true;
            allzero = false;
        }
        scan_vec[clt_idx].m = mvec_x[clt_idx+useheight];
    }

    int loop_i = 0;
    while ( !nozero && !allzero ) 
    {
        nozero = true;
        clt_idx = 0;

        //std::cout << "#####################################################" << std::endl;
        for ( map_j=num_clt_row-1; map_j>=0; map_j-- ) {
            for ( map_i=0; map_i<num_clt_col; map_i++ ) 
            {
                //std::cout << "clt_idx " << clt_idx << "  scan_vec[clt_idx].m  " << scan_vec[clt_idx].m << "  "  << scan_vec[clt_idx].scanned << std::endl;
                if ( doubleEqual( scan_vec[clt_idx].m, 0.0 ) ) {
                    nozero = false;

                    if ( map_i == 0 ) {
                        if ( map_j == 0 ) {
                            div_num = 0; 
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        } else if ( map_j == num_clt_row-1 ) {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        } else {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                    } else if ( map_i == num_clt_col-1 ) {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 )
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                        else
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                    } else {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                        else 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].m < 0.0 && scan_vec[clt_idx-1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-1].m;
                            }
                            if ( scan_vec[clt_idx+1].m < 0.0 && scan_vec[clt_idx+1].scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+1].m;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].m < 0.0 && scan_vec[clt_idx-num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx-num_clt_col].m ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].m < 0.0 && scan_vec[clt_idx+num_clt_col].scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].m += scan_vec[clt_idx+num_clt_col].m ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].m /= div_num;
                        }
                    }
                }
                clt_idx ++;
            }
        }

        for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
            if ( !doubleEqual( scan_vec[clt_idx].m, 0.0 ) )
                scan_vec[clt_idx].scanned = true;
        }

        loop_i ++;

        int num = ((num_clt_row>num_clt_col) ? num_clt_row : num_clt_col ); 
        if ( loop_i > 2*num - 2 )
            break;
    }
    
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        mvec_x[clt_idx+useheight] = scan_vec[clt_idx].m;
    }

    free ( scan_vec ); 
}


/******************************************************************************************/
/**** FUNCTION: ClutterPropModelClass::clt_statistic()                                 ****/
/**** colculate numbers of clutter with positive coeficient, and show with special color.*/
/******************************************************************************************/
void ClutterPropModelClass::clt_statistic( int& uu, int& zz, int& dd )
{
    int clt_idx;

    uu = 0;                               //if CLUTTER coeficient is positive, uu++
    zz = 0;                               //if CLUTTER coeficient is zero, zz++
    dd = 0;                               //if CLUTTER coeficient is negative, uu++
    
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        if ( mvec_x[clt_idx+useheight] > 0.0 ) {
            uu ++;
        } else if ( doubleEqual( mvec_x[clt_idx+useheight], 0 ) ) {
            zz ++;
        } else if ( mvec_x[clt_idx+useheight] < 0.0 ) {
            dd ++;
        }
    }
}

/*
    CONSTRUCT FUNCTION : ClutterPropModelFullClass( char* )                                
 */
ClutterPropModelFullClass::ClutterPropModelFullClass(char *p_strid ) : GenericClutterPropModelClass(p_strid) 
{
}

/*
    DISCONSTRUCT FUNCTION : ~ClutterPropModelFullClass( )                                  
 */
ClutterPropModelFullClass::~ClutterPropModelFullClass()
{    free ( mvec_x );
}

const int ClutterPropModelFullClass::type() { return CConst::PropClutterFull; }

/* 
 * Path Loss from point(cs_x, cs_y) to point(pt_x, pt_y).    */
double ClutterPropModelFullClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{

    CellClass *cell = sector->parent_cell;
    MapClutterClass* map_clutter = np->map_clutter;

    /*
        As for ald algorithm this vector save the distance of passing clutters, 
        and for new algorithm this vector save the divide value of cumulative sum distance of passing clutters
        (D(i)/D(i-1), i is the passed clutter index, D is the cumulative sum distance of passed clutters).
     */
    divDistStruct* divD_vector = (divDistStruct*) malloc ( num_clutter_type * sizeof ( divDistStruct ) );
    distStruct*    d_vector    = (distStruct*) malloc ( num_clutter_type * sizeof ( distStruct ) );

    int    clutter_idx    = 0;
    int    index          = 0;
    double prop_dB        = 0.0;

    // Values with unit system resolution. 
    int offset_x = map_clutter->offset_x;
    int offset_y = map_clutter->offset_y;
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    double total_dist;

    // Values with unit m, i.e. true distance.
    double logh;

    if ( fabs( angle_deg ) < DERROR  ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (x1 - x0) * np->resolution;
    double dy = (y1 - y0) * np->resolution;
    double dz = -sector->antenna_height;

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );
    if ( total_dist < DERROR ) total_dist = 1.0;

    logh = log(-dz)/log(10.0);
        
    for (clutter_idx=0; clutter_idx<num_clutter_type; clutter_idx++) {
        divD_vector[clutter_idx].divD  = 1.0;
        divD_vector[clutter_idx].c_idx = 0;
        d_vector[clutter_idx].d  = 1.0;
        d_vector[clutter_idx].c_idx = 0;
    }

    int    clutter_type = 0;         //Keep the value of passed clutter type.
    int    pass_clt = 0;             //Keep the number of Passed clutter type.
    int    pass_idx = 0;             //Keep the passed clutter index.
    double divDist  = 1.0;           //Keep the value of D(i)/D(i-1).
    int    n(0), si(0), sj(0), pi(0), pj(0), q(0);
    int    map_i      = 0;
    int    map_j      = 0;
    int    map_i_1    = 0;
    int    map_j_1    = 0;
    double prev_dist  = np->resolution;
    double curr_dist  = np->resolution;
    double dmin       = np->resolution;
    
    n = map_clutter->map_sim_res_ratio;
    
    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);
    
    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    bool first_scan = true;
    bool on_bdy     = false;
    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {
        
        clutter_type = map_clutter->get_clutter_type(map_i, map_j);
        // q = (n*((map_j+pj)*(pt_rx-cs_rx)-(map_i+pi)*(pt_ry-cs_ry)) + (pt_ry-offset_ry)*(cs_rx-offset_rx) - (pt_rx-offset_rx)*(cs_ry-offset_ry))*si*sj;

       // Offset by 1/2 resolution
       q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;
        
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            curr_dist = total_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            curr_dist = total_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }
#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
                map_i, map_j, curr_dist, prev_dist, curr_dist - prev_dist, clutter_type, map_clutter->description[clutter_type]);
#endif
        
        //Fill Sum-Distance to the correct position of matrix mmx_a. 
        if ( first_scan && fabs (curr_dist) > DERROR ) {
            first_scan = false;
            prev_dist  = dmin;
            divDist = curr_dist/prev_dist;
            d_vector[pass_idx].d = curr_dist;
            d_vector[pass_idx].c_idx = clutter_type;
            divD_vector[pass_idx].divD = divDist;
            divD_vector[pass_idx].c_idx = clutter_type;
        } else if ( first_scan && fabs (curr_dist) < DERROR ) {
            on_bdy = true;
            first_scan = false;
        } else { 
            if ( on_bdy ) prev_dist = dmin;
            divDist = curr_dist/prev_dist;
            
            d_vector[pass_idx].d = curr_dist;
            d_vector[pass_idx].c_idx = clutter_type;
            divD_vector[pass_idx].divD = divDist;
            divD_vector[pass_idx].c_idx = clutter_type;
            on_bdy = false;
        }
        prev_dist = curr_dist;

        //std::cout << "d_vector[pass_idx].c_idx " << d_vector[pass_idx].c_idx << " d_vector[pass_idx].d " << d_vector[pass_idx].d << std::endl;

        pass_idx++;
    }
    curr_dist = total_dist;
    
    clutter_type = map_clutter->get_clutter_type(map_i, map_j);
    
    if ( first_scan && fabs (curr_dist) > DERROR ) {
        prev_dist  = dmin;
        divDist = curr_dist/prev_dist;
    } else if ( first_scan && fabs (curr_dist) < DERROR )
        divDist = 1.0;
    else divDist = curr_dist/prev_dist;
    
    if ( first_scan && fabs (curr_dist) > DERROR ) {
        prev_dist  = dmin;
        divDist = curr_dist/prev_dist;
        d_vector[pass_idx].d = curr_dist;
        d_vector[pass_idx].c_idx = clutter_type;
        divD_vector[pass_idx].divD = divDist;
        divD_vector[pass_idx].c_idx = clutter_type;
    } else if ( first_scan && fabs (curr_dist) < DERROR ) {
        on_bdy = true;
        divD_vector[pass_idx].divD = 1.0;
        divD_vector[pass_idx].c_idx = clutter_type;
    } else { 
        if ( on_bdy ) prev_dist = dmin;
        divDist = curr_dist/prev_dist;
        d_vector[pass_idx].d = curr_dist;
        d_vector[pass_idx].c_idx = clutter_type;
        divD_vector[pass_idx].divD = divDist;
        divD_vector[pass_idx].c_idx = clutter_type;
        on_bdy = false;
    }
#if CLUTTER_DEBUG
    printf("map_i = %d map_j = %d curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
            map_i, map_j, curr_dist, prev_dist, curr_dist - prev_dist, clutter_type, map_clutter->description[clutter_type]);
#endif
    //Pass_clt keep the count of passed clutters;
    pass_clt = pass_idx;
    
    if (useheight) {
        prop_dB += mvec_x[index ++]*logh;
    }

    /* MIDI 2005.5.7 CG
     * Wuhan '60m--506 CLT.--31# CS' problem.
     ******************************************************************/
    double rdB = 0.;    // recieve power in dB;
    double gdB = 0.;    // antenna gain in dB;
    double tdB = 0.;    // transmit power in dB;
    double pdB = 0.;    // temp variable power in dB;
    double divD = divD_vector[0].divD;
    double pdB0 = mvec_x[2*divD_vector[0].c_idx+useheight]*log(divD)/log(10.0);
    prop_dB += mvec_x[2*divD_vector[0].c_idx+useheight+1];
    pdB0 += prop_dB;
    for ( pass_idx=0; pass_idx<=pass_clt; pass_idx++) {
        divD = divD_vector[pass_idx].divD;
#if CLUTTER_DEBUG 
        if ( pass_idx == 0 && divD <= 10.0 && pdB0 > -45)
        {
            std::cout << " -- REGU -- \n" << std::endl;
            divD = 10.0; 
            pdB = mvec_x[2*divD_vector[pass_idx].c_idx+useheight]*log(divD)/log(10.0); 
        }
#endif
        double dz = -sector->antenna_height;
        AntennaClass *antenna = np->antenna_type_list[sector->antenna_type];

        gdB = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
        tdB = 10.0*log(sector->tx_pwr)/log(10.0);
        rdB = tdB + gdB + prop_dB;

#if 0
// xxxxxxxxxx MM *** Don't think this is a good idea.
        if ( d_vector[pass_idx].d > 300 && rdB > -68. )    // distance > 500 && recieve power is 50 dBuV
        {
            pdB = -250*log(divD)/log(10.0); 
        } else {
            pdB = mvec_x[2*divD_vector[pass_idx].c_idx+useheight]*log(divD)/log(10.0); 
        }
#else
        pdB = mvec_x[2*divD_vector[pass_idx].c_idx+useheight]*log(divD)/log(10.0); 
#endif
        prop_dB += pdB; 


#if 0
        // DBG
        std::cout << "pass_idx " << pass_idx
                  << " pdB0 "    << pdB0
                  << " divD "    << divD
                  << " Prop_dB " << prop_dB
                  << " rdB     " << rdB + 113
                  << std::endl;
#endif
    }

#if 0
    // DBG
    std::cout << "prop_dB " << prop_dB
              << std::endl  << std::endl;
#endif
    
    double prop = exp(prop_dB*log(10.0)/10.0);
    
    free ( divD_vector );
    free ( d_vector );
    
    return(prop);
}

double ClutterPropModelFullClass::compute_error(int num_road_test_pt, double **clutter_dis, double *power, int *a_posn, int plot, char *flname)
{
    int       pt_idx      = 0; 
    int       clutter_idx = 0;
    double    diff        = 0.0;
    double    error       = 0.0;
    FILE      *fp         = NULL;     

    if (plot) {
        if ( !(fp = fopen(flname, "w")) ) {
            fprintf(stderr, "\nERROR: Unable to write to file %s\n", flname);
            exit(1);
        }
    }

    for (pt_idx=0; pt_idx<num_road_test_pt; pt_idx++) {
        diff = 0.0;

        if( useheight != 0 ) {
            diff += mvec_x[0]*clutter_dis[pt_idx][0];
        }

        diff += mvec_x[2*a_posn[pt_idx]+useheight+1];
        for (clutter_idx=0; clutter_idx<num_clutter_type; clutter_idx++) {
            if ( fabs(clutter_dis[pt_idx][2*clutter_idx+useheight] ) > DERROR ) {
                diff += mvec_x[2*clutter_idx+useheight]*clutter_dis[pt_idx][2*clutter_idx+useheight]; 
            }
        }

        diff -= power[pt_idx];
        error += diff*diff;
        if (plot)  fprintf(fp, "%15.10f\n", diff);
    }

    if (plot) {
        fprintf(fp, "\n");
        fclose(fp);
    }

    return(error);
}

void ClutterPropModelFullClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 2) {
        mvec_x = DVECTOR(2*num_clutter_type + useheight);
    }

    if (param_idx == 0 ) {
        sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 1 ) {
        sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
    } else {
        i = param_idx-2;
        sprintf(str, "C_%d:", i);           type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterPropModelFullClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + 2*num_clutter_type + 2; 

    // DBG
    //std::cout << " ClutterPropModelFullClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterPropModelFullClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTERFULL\n");
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    NUM_CLUTTER_TYPE: %d\n", num_clutter_type);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<2*num_clutter_type; c_idx++) {
        if ( (c_idx)%2 == 0 ) {
            sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        } else {
            sprintf(msg, "    C_%d: %9.7f  #CONSTANT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        }
    }
}

/*
    CONSTRUCT FUNCTION : ClutterSymFullPropModelClass( char* )                                
 */
ClutterSymFullPropModelClass::ClutterSymFullPropModelClass(char *p_strid ) : GenericClutterPropModelClass(p_strid) 
{
}

/*
    DISCONSTRUCT FUNCTION : ~ClutterSymFullPropModelClass( )                                  
 */
ClutterSymFullPropModelClass::~ClutterSymFullPropModelClass()
{    free ( mvec_x );
}

const int ClutterSymFullPropModelClass::type() { return CConst::PropClutterSymFull; }

/* 
 * Path Loss from point(cs_x, cs_y) to point(pt_x, pt_y).    */
double ClutterSymFullPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{
    int clutter_type;
    CellClass *cell = sector->parent_cell;
    MapClutterClass* map_clutter = np->map_clutter;

    double prop_db        = 0.0;

    // Values with unit system resolution.
    int offset_x = map_clutter->offset_x;
    int offset_y = map_clutter->offset_y;
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    double total_dist;

    if ( angle_deg == 0.0 ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (double) (x1 - x0) * np->resolution;
    double dy = (double) (y1 - y0) * np->resolution;
    double dz = -sector->antenna_height;
    if (fabs(dz) < np->resolution) { dz = -np->resolution; }

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );

    int    n, si, sj, pi, pj, q;
    int    map_i, map_j;
    int    map_i_1, map_j_1;

    n = map_clutter->map_sim_res_ratio;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    clutter_type = map_clutter->get_clutter_type(map_i, map_j);
    prop_db += mvec_x[2*clutter_type + 1+ useheight];

    int first_segment = 1;
    double sum      = 0.0;
    double prev_sum;

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

        prev_sum = sum;
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            sum = total_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            sum = total_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }

#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
                map_i, map_j, sum, prev_sum, sum-prev_sum, clutter_type, map_clutter->description[clutter_type]);
#endif

        if (first_segment) {
            prop_db += mvec_x[2*clutter_type + useheight] * log( (sum*total_dist)/(total_dist-sum) ) / log(10.0);
            first_segment = 0;
        } else {
            prop_db += mvec_x[2*clutter_type + useheight] * log( (sum*(total_dist-prev_sum))/(prev_sum*(total_dist-sum)) ) / log(10.0);
        }

        clutter_type = map_clutter->get_clutter_type(map_i, map_j);
    }

    prop_db += mvec_x[2*clutter_type + 1+ useheight];
    if (first_segment) {
        prop_db += 2*mvec_x[2*clutter_type + useheight] * log( total_dist ) / log(10.0);
    } else {
        prop_db += mvec_x[2*clutter_type + useheight] * log( ((total_dist-sum)*total_dist)/sum ) / log(10.0);
    }

    prop_db /= 2.0;
    if (useheight) {
        prop_db += mvec_x[0] * log(-dz)/log(10.0);
    }

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}

double ClutterSymFullPropModelClass::compute_error(int num_road_test_pt, double **clutter_dis, double *power, int *a_posn, int plot, char *flname)
{
#if 0
    int       pt_idx      = 0; 
    int       clutter_idx = 0;
    double    diff        = 0.0;
    double    error       = 0.0;
    FILE      *fp         = NULL;     

    if (plot) {
        if ( !(fp = fopen(flname, "w")) ) {
            fprintf(stderr, "\nERROR: Unable to write to file %s\n", flname);
            exit(1);
        }
    }

    for (pt_idx=0; pt_idx<num_road_test_pt; pt_idx++) {
        diff = 0.0;

        if( useheight != 0 ) {
            diff += mvec_x[0]*clutter_dis[pt_idx][0];
        }

        diff += mvec_x[2*a_posn[pt_idx]+useheight+1];
        for (clutter_idx=0; clutter_idx<num_clutter_type; clutter_idx++) {
            if ( fabs(clutter_dis[pt_idx][2*clutter_idx+useheight] ) > DERROR ) {
                diff += mvec_x[2*clutter_idx+useheight]*clutter_dis[pt_idx][2*clutter_idx+useheight]; 
            }
        }

        diff -= power[pt_idx];
        error += diff*diff;
        if (plot)  fprintf(fp, "%15.10f\n", diff);
    }

    if (plot) {
        fprintf(fp, "\n");
        fclose(fp);
    }

    return(error);
#else
    CORE_DUMP;
    return(0.0);
#endif
}

void ClutterSymFullPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 2) {
        mvec_x = DVECTOR(2*num_clutter_type + useheight);
    }

    if (param_idx == 0 ) {
        sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 1 ) {
        sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
    } else {
        i = param_idx-2;
        sprintf(str, "C_%d:", i);           type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterSymFullPropModelClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + 2*num_clutter_type + 2; 

    // DBG
    //std::cout << " ClutterSymFullPropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterSymFullPropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTERSYMFULL\n");
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    NUM_CLUTTER_TYPE: %d\n", num_clutter_type);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<2*num_clutter_type; c_idx++) {
        if ( (c_idx)%2 == 0 ) {
            sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        } else {
            sprintf(msg, "    C_%d: %9.7f  #CONSTANT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        }
    }
}

/*
    CONSTRUCT FUNCTION : ClutterWtExpoPropModelClass( char* )                                
 */
ClutterWtExpoPropModelClass::ClutterWtExpoPropModelClass(char *p_strid) : GenericClutterPropModelClass(p_strid) 
{
}

/*
    DISCONSTRUCT FUNCTION : ~ClutterWtExpoPropModelClass( )                                  
 */
ClutterWtExpoPropModelClass::~ClutterWtExpoPropModelClass()
{
}

const int ClutterWtExpoPropModelClass::type() { return CConst::PropClutterWtExpo; }

/******************************************************************************************/
/**** CLASS: ClutterWtExpoSlopePropModelClass : Constructor, Destructor, type()        ****/
/******************************************************************************************/
ClutterWtExpoSlopePropModelClass::ClutterWtExpoSlopePropModelClass(char *p_strid) : GenericClutterPropModelClass(p_strid) 
{
    r0 =  15.0;
}
ClutterWtExpoSlopePropModelClass::~ClutterWtExpoSlopePropModelClass()
{
}
const int ClutterWtExpoSlopePropModelClass::type() { return CConst::PropClutterWtExpoSlope; }
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: ClutterExpoLinearPropModelClass : Constructor, Destructor, type()         ****/
/******************************************************************************************/
ClutterExpoLinearPropModelClass::ClutterExpoLinearPropModelClass(char *p_strid) : GenericClutterPropModelClass(p_strid) 
{
    exponent    =  -2.0;
    r0          =  1.0;
}
ClutterExpoLinearPropModelClass::~ClutterExpoLinearPropModelClass()
{
}

const int ClutterExpoLinearPropModelClass::type() { return CConst::PropClutterExpoLinear; }
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: ClutterGlobalPropModelClass : Constructor, Destructor, type()         ****/
/******************************************************************************************/
ClutterGlobalPropModelClass::ClutterGlobalPropModelClass(char *p_strid) : GenericClutterPropModelClass(p_strid) 
{
    // This minimun distance is limited by global segment model
    r0 = 15;
    globPm = (SegmentPropModelClass *) NULL;
}
ClutterGlobalPropModelClass::~ClutterGlobalPropModelClass()
{
    if (globPm) {
        delete globPm;
    }
}

const int ClutterGlobalPropModelClass::type() { return CConst::PropClutterGlobal; }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::define_clutter                           ****/
/******************************************************************************************/
void GenericClutterPropModelClass::define_clutter(NetworkClass *np, int p_clutter_sim_res_ratio)
{
    clutter_sim_res_ratio = p_clutter_sim_res_ratio;

    offset_x = -MOD(np->system_startx, clutter_sim_res_ratio);
    offset_y = -MOD(np->system_starty, clutter_sim_res_ratio);

    npts_x = ( np->npts_x - offset_x + clutter_sim_res_ratio-1 - MOD(np->npts_x-offset_x-1, clutter_sim_res_ratio) ) / clutter_sim_res_ratio;
    npts_y = ( np->npts_y - offset_y + clutter_sim_res_ratio-1 - MOD(np->npts_y-offset_y-1, clutter_sim_res_ratio) ) / clutter_sim_res_ratio;

    num_clutter_type = npts_x*npts_y;
}
/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::create_svd_matrices                      ****/
/******************************************************************************************/
void GenericClutterPropModelClass::create_svd_matrices(NetworkClass *np, ListClass<int> *scan_index_list,
    PolygonClass *map_bdy, int num_insys_rtp, int num_var, double **&mmx_a, double *&vec_b)
{
    int x0, y0, x1, y1;
    int n, si, sj, pi, pj, q;
    int map_i, map_j;
    int map_i_1, map_j_1;
    int cell_idx, sector_idx, scan_idx, clutter_idx, insys_rtp_idx;
    int rtp_idx;
    int clutter_type;
    double dx, dy, dz, total_dist, xy_dist;
    double r0, exponent;
    double m_slope, m_intercept;
    CellClass       *cell;
    SectorClass     *sector;
    RoadTestPtClass *road_test_pt;

    // show progress bar in clutter simulation - CG MOD 7/19/06
#if HAS_GUI
    ProgressSlot* prog_bar;
    int curr_prog = 0;
    if (use_gui) {
        prog_bar = new ProgressSlot(0, "Progress Bar", qApp->translate("ProgressSlot", "Running Clutter Simulation - SVD Matrices ") + "...");
    }
#endif

    int m_type = type();

    if ( m_type == CConst::PropClutterSimp ) {
        m_slope     = ((ClutterPropModelClass *) this)->k;
        m_intercept = ((ClutterPropModelClass *) this)->b;
    } else {
        m_slope     = 0.0;
        m_intercept = 0.0;
    }

    if ( m_type == CConst::PropClutterWtExpoSlope ) {
        r0 = ((ClutterWtExpoSlopePropModelClass *) this)->r0;
    }
    else if ( m_type == CConst::PropClutterExpoLinear ) {
        r0 = ((ClutterExpoLinearPropModelClass *) this)->r0;
        exponent = ((ClutterExpoLinearPropModelClass *) this)->exponent;
    }
    else if ( m_type == CConst::PropClutterGlobal ) {
        r0 = ((ClutterGlobalPropModelClass *) this)->r0;
    }
    else
        CORE_DUMP;

    mmx_a = (double**) malloc( num_insys_rtp * sizeof (double*) );
    vec_b = DVECTOR(num_insys_rtp);
    for ( rtp_idx=0; rtp_idx<num_insys_rtp; rtp_idx++ ) {
        mmx_a[rtp_idx] = DVECTOR(num_var);
        for ( clutter_idx=0; clutter_idx<=num_var-1; clutter_idx++ ) {
            mmx_a[rtp_idx][clutter_idx] = 0.0;
        }
    }

    /* 
       Function of this loop:
       Fill vec_b
       Get distance of passed clutter types, and fill to matrix mmx_a. 
     */
    insys_rtp_idx = 0;
    int num_rtd = np->road_test_data_list->getSize();
    for (rtp_idx=0; rtp_idx<=num_rtd-1; rtp_idx++) {

        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );         
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
#if 0
// xxxxxxxxxxxxxxxxxxxxxxx
printf("RTP: %d CELL_SECTOR: %d_%d CELL_POSN: (%9.7f %9.7f) RTP_POSN: (%9.7f, %9.7f) PWR_DB = %9.7f\n",
    insys_rtp_idx, cell_idx, sector_idx, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y),
    idx_to_x(road_test_pt->posn_x), idx_to_y(road_test_pt->posn_y), road_test_pt->pwr_db);
#endif
                x0 = cell->posn_x;
                y0 = cell->posn_y;
                x1 = road_test_pt->posn_x;
                y1 = road_test_pt->posn_y;

                dx = (double) (x1 - x0)*np->resolution;
                dy = (double) (y1 - y0)*np->resolution;
                dz = -(sector->antenna_height);
                if (fabs(dz) < np->resolution) dz = -np->resolution;

                if ( sector->antenna_height <= 0.0 ) {
                    sprintf(np->msg, "ERROR: Cell %d Sector %d has antenna_height = %12.10f less than 0.0\n",
                        cell_idx, sector_idx, sector->antenna_height);
                    PRMSG(stdout, np->msg); 
                    np->error_state = 1;
                    return;
                }

                total_dist = sqrt( dx*dx + dy*dy + dz*dz );
                xy_dist    = sqrt( dx*dx + dy*dy );

                AntennaClass *antenna = np->antenna_type_list[sector->antenna_type];
                double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);

                //Fill matrix mmx_a
                if ( m_type == CConst::PropClutterSimp ) {
                    //Loops for new type of clutter propagation model algorithm.
                    //Compute the summer distance of passed clutters, and fill them to mvec_Dist.
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db - m_intercept;
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                        /*To solve the value ai in equal Pl = ( Ci*log10(d) + ai ), 
                          this is the propagation model of the clutter that the CS lies in.*/
                        //Take value m_intercept from vec_b;
                        //mmx_a[insys_rtp_idx][1] = m_intercept;              
                        //matrix_test << std::setw(10) << logh << std::setw(10) << logh+logd << std::setw(10) << "1.0" ; 
                    } else {
                        //Take value m_intercept from vec_b;
                        //mmx_a[insys_rtp_idx][0] = m_intercept;
                        //matrix_test << std::setw(10) << "1.0" ; 
                    }
                }
                else if ( m_type == CConst::PropClutterFull ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight )  {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }
                else if ( m_type == CConst::PropClutterSymFull ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }
                else if ( m_type == CConst::PropClutterWtExpo ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }
                else if ( m_type == CConst::PropClutterWtExpoSlope ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }
                else if ( m_type == CConst::PropClutterExpoLinear ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db - 10.0*exponent*log(total_dist<r0?r0:total_dist)/log(10.0);
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }
                else if ( m_type == CConst::PropClutterGlobal ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db -
                        10.0 * log(((ClutterGlobalPropModelClass *) this)->globPm->
                        //prop_power_loss(np, sector, (int)dx, (int)dy))/log(10.0);
                        prop_power_loss(np, sector, (x1-x0), (y1-y0)))/log(10.0);
                    if( useheight ) {
                        mmx_a[insys_rtp_idx][0] = log(-dz)/log(10.0);
                    }
                }

                n = clutter_sim_res_ratio;

                si = ((x1-x0>=0) ? 1 : -1);
                sj = ((y1-y0>=0) ? 1 : -1);
                pi = ((x1-x0>=0) ? 1 : 0);
                pj = ((y1-y0>=0) ? 1 : 0);

                map_i   = DIV(x0-offset_x, n);
                map_j   = DIV(y0-offset_y, n);
                map_i_1 = DIV(x1-offset_x, n);
                map_j_1 = DIV(y1-offset_y, n);

                clutter_type = map_j*npts_x + map_i;
                if ( m_type == CConst::PropClutterFull ) {
                    mmx_a[insys_rtp_idx][2*clutter_type+1+useheight] = 1.0; 
                } else if ( m_type == CConst::PropClutterSymFull ) {
                    mmx_a[insys_rtp_idx][2*clutter_type+1+useheight] = 0.5; 
                }

                int first_segment = 1;
                double sum = 0.0;
                double prev_sum;

                while ( (map_i != map_i_1) || (map_j != map_j_1) ) {
                    // Offset by 1/2 resolution
                    q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

                    prev_sum = sum;
                    if ( (q > 0) || ( (q == 0) && (pi) ) ) {
                        if ( (m_type == CConst::PropClutterWtExpo) || (m_type == CConst::PropClutterWtExpoSlope) ) {
                            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        } else if ( (m_type == CConst::PropClutterExpoLinear) ) {
                            sum = xy_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        } else if ( (m_type == CConst::PropClutterGlobal) ) {
                            sum = xy_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        } else {
                            sum = total_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        }
                        map_i += si;
                    } else {
                        if ( (m_type == CConst::PropClutterWtExpo) || (m_type == CConst::PropClutterWtExpoSlope) ) {
                            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        } else if ( (m_type == CConst::PropClutterExpoLinear) ) {
                            sum = xy_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        // CG EDIT, No need to change
                        } else if ( (m_type == CConst::PropClutterGlobal) ) {
                            sum = xy_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        } else {
                            sum = total_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        }
                        map_j += sj;
                    }

#if CLUTTER_DEBUG 
                    printf("map_i = %d (%d) map_j = %d (%d) curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d\n",
                            map_i, x1/n, map_j, y1/n, sum, prev_sum, sum- prev_sum, clutter_type);
#endif

                    if ( m_type == CConst::PropClutterSimp ) {
                        if (first_segment) {
                            mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( sum )/log(10.0) + m_intercept;
                            first_segment = 0;
                        } else {
                            mmx_a[insys_rtp_idx][clutter_type] = log( sum / prev_sum )/log(10.0);
                        }
                    } else if ( m_type == CConst::PropClutterFull ) {
                        if (first_segment) {
                            mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( sum ) / log(10.0);
                            first_segment = 0;
                        } else {
                            mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( sum / prev_sum ) / log(10.0);
                        }
                    } else if ( m_type == CConst::PropClutterSymFull ) {
                        if (first_segment) {
                            mmx_a[insys_rtp_idx][2*clutter_type + useheight] = 0.5*log( (sum*total_dist)/(total_dist-sum) ) / log(10.0);
                            first_segment = 0;
                        } else {
                            mmx_a[insys_rtp_idx][2*clutter_type + useheight] = 0.5*log( (sum*(total_dist-prev_sum))/(prev_sum*(total_dist-sum)) ) / log(10.0);
                        }
                    } else if ( m_type == CConst::PropClutterWtExpo ) {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = (sum-prev_sum)*log(total_dist)/log(10.0);
                        mmx_a[insys_rtp_idx][2*clutter_type + 1 + useheight] = (sum-prev_sum);
                    } else if ( m_type == CConst::PropClutterWtExpoSlope ) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = (sum-prev_sum)*log(total_dist/r0)/log(10.0);
                    } else if ( m_type == CConst::PropClutterExpoLinear ) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = sum-prev_sum;
                    // No need to change
                    } else if ( m_type == CConst::PropClutterGlobal) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = sum-prev_sum;
                    }

                    clutter_type = map_j*npts_x + map_i;
                }

                if ( m_type == CConst::PropClutterSimp ) {
                    if (first_segment) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = log( total_dist )/log(10.0) + m_intercept;
                        first_segment = 0;
                    } else {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = log( total_dist / sum )/log(10.0);
                    }
                } else if ( m_type == CConst::PropClutterFull ) {
                    if (first_segment) {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( total_dist ) / log(10.0);
                        first_segment = 0;
                    } else {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( total_dist / sum ) / log(10.0);
                    }
                } else if ( m_type == CConst::PropClutterSymFull ) {
                    mmx_a[insys_rtp_idx][2*clutter_type + 1+ useheight] += 0.5;
                    if (first_segment) {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( total_dist ) / log(10.0);
                    } else {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = 0.5*log( ((total_dist-sum)*total_dist)/sum ) / log(10.0);
                    }
                } else if ( m_type == CConst::PropClutterWtExpo) {
                        mmx_a[insys_rtp_idx][2*clutter_type + useheight] = (1.0 - sum)*log(total_dist)/log(10.0);
                        mmx_a[insys_rtp_idx][2*clutter_type + 1 + useheight] = (1.0 - sum);
                } else if ( m_type == CConst::PropClutterWtExpoSlope) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = (1.0 - sum)*log(total_dist/r0)/log(10.0);
                        mmx_a[insys_rtp_idx][num_var-1] = 1.0;
                } else if ( m_type == CConst::PropClutterExpoLinear) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = xy_dist - sum;
                        mmx_a[insys_rtp_idx][num_var-1] = 1.0;

#if 0
                        std::cout << "clutter_type + useheight " << clutter_type + useheight << std::endl;
                        std::cout << "num_var-1 " << num_var-1 << std::endl;
#endif
                } else if ( m_type == CConst::PropClutterGlobal) {
                        mmx_a[insys_rtp_idx][clutter_type + useheight] = xy_dist - sum;

                        // constant of PM
                        mmx_a[insys_rtp_idx][num_var-1] = 1.0;
                }

                insys_rtp_idx ++;
            }
        }

#if HAS_GUI
        curr_prog = (int) 100.0*rtp_idx/num_rtd;
    if (use_gui) {
        prog_bar->set_prog_percent(curr_prog);
    }
#endif
    }

#if 0
    printf("SVD MATRICES\n");
    for ( rtp_idx=0; rtp_idx<num_insys_rtp; rtp_idx++ ) {
        for ( clutter_idx=0; clutter_idx<=num_var-1; clutter_idx++ ) {
            printf("A[%d][%d] = %15.10e\n", rtp_idx, clutter_idx, mmx_a[rtp_idx][clutter_idx]);
        }
        printf("B[%d] = %15.10e\n", rtp_idx, vec_b[rtp_idx]);
        printf("\n");
    }
    exit(1);
#endif


#if HAS_GUI
    if (use_gui) {
        delete prog_bar;
    }
#endif
}


/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::create_svd_matrices                      ****/
/******************************************************************************************/
void GenericClutterPropModelClass::create_svd_matrices(NetworkClass *np, ListClass<int> *scan_index_list,
    PolygonClass *map_bdy, int num_insys_rtp, int num_var, SparseMatrixClass *&mmx_a, double *&vec_b)
{
    int x0, y0, x1, y1;
    int n, si, sj, pi, pj, q;
    int map_i, map_j;
    int map_i_1, map_j_1;
    int cell_idx, sector_idx, scan_idx, insys_rtp_idx;
    int rtp_idx;
    int clutter_type;
    double dx, dy, dz, total_dist, xy_dist;
    double r0, exponent;
    double m_slope, m_intercept;
    CellClass       *cell;
    SectorClass     *sector;
    RoadTestPtClass *road_test_pt;

#if OUTPUT_TIME_INFOR
    time_t td;
#endif

    int m_type = type();

    if ( m_type == CConst::PropClutterSimp ) {
        m_slope     = ((ClutterPropModelClass *) this)->k;
        m_intercept = ((ClutterPropModelClass *) this)->b;
    } else {
        m_slope     = 0.0;
        m_intercept = 0.0;
    }

    if ( m_type == CConst::PropClutterWtExpoSlope ) {
        r0 = ((ClutterWtExpoSlopePropModelClass *) this)->r0;
    } else if ( m_type == CConst::PropClutterExpoLinear ) {
        r0 = ((ClutterExpoLinearPropModelClass *) this)->r0;
        exponent = ((ClutterExpoLinearPropModelClass *) this)->exponent;
    }
    else if ( m_type == CConst::PropClutterGlobal ) {
        // global segment PM and its parameters to be created and computed in gen_clutter.cpp
        r0 = ((ClutterGlobalPropModelClass *) this)->r0;
    }
    else
        CORE_DUMP;

    mmx_a = new SparseMatrixClass(num_insys_rtp, num_var);
    vec_b = DVECTOR(num_insys_rtp);

    /* 
       Function of this loop:
       Get distance of passed clutter types, and fill to matrix mmx_a. 
     */
    insys_rtp_idx = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );         
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                x0 = cell->posn_x;
                y0 = cell->posn_y;
                x1 = road_test_pt->posn_x;
                y1 = road_test_pt->posn_y;

                dx = (double) (x1 - x0)*np->resolution;
                dy = (double) (y1 - y0)*np->resolution;
                dz = -sector->antenna_height;
                if (fabs(dz) < np->resolution) dz = -np->resolution;

                if ( sector->antenna_height <= 0.0 ) {
                    sprintf(np->msg, "ERROR: Cell %d Sector %d has antenna_height = %12.10f less than 0.0\n",
                        cell_idx, sector_idx, sector->antenna_height);
                    PRMSG(stdout, np->msg); 
                    np->error_state = 1;
                    return;
                }

                total_dist = sqrt( dx*dx + dy*dy + dz*dz );
                xy_dist    = sqrt( dx*dx + dy*dy );

                AntennaClass *antenna = np->antenna_type_list[sector->antenna_type];
                double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);

                //Fill matrix mmx_a
                if ( m_type == CConst::PropClutterSimp ) {
                    //Loops for new type of clutter propagation model algorithm.
                    //Compute the summer distance of passed clutters, and fill them to mvec_Dist.
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db - m_intercept;
                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                        /*To solve the value ai in equal Pl = ( Ci*log10(d) + ai ), 
                          this is the propagation model of the clutter that the CS lies in.*/
                        //Take value m_intercept from vec_b;
                        //mmx_a[insys_rtp_idx][1] = m_intercept;              
                        //matrix_test << std::setw(10) << logh << std::setw(10) << logh+logd << std::setw(10) << "1.0" ; 
                    } else {
                        //Take value m_intercept from vec_b;
                        //mmx_a[insys_rtp_idx][0] = m_intercept;
                        //matrix_test << std::setw(10) << "1.0" ; 
                    }
                }
                else if ( m_type == CConst::PropClutterFull ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight )  {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                else if ( m_type == CConst::PropClutterSymFull ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                else if ( m_type == CConst::PropClutterWtExpo ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                else if ( m_type == CConst::PropClutterWtExpoSlope ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db;
                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                else if ( m_type == CConst::PropClutterExpoLinear ) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db
                        - 10.0*exponent*log(total_dist<r0?r0:total_dist)/log(10.0);
                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                // CG EDIT
                else if ( m_type == CConst::PropClutterGlobal) {
                    vec_b[insys_rtp_idx] = road_test_pt->pwr_db - gain_db - tx_pwr_db -
                        10.0 * log(((ClutterGlobalPropModelClass *) this)->globPm->
                        //prop_power_loss(np, sector, (int)dx, (int)dy))/log(10.0);
                        prop_power_loss(np, sector, (x1-x0), (y1-y0)))/log(10.0);

                    if( useheight ) {
                        mmx_a->a->append(DoubleIntIntClass(log(-dz)/log(10.0), insys_rtp_idx, 0));
                    }
                }
                else
                    CORE_DUMP;

                n = clutter_sim_res_ratio;

                si = ((x1-x0>=0) ? 1 : -1);
                sj = ((y1-y0>=0) ? 1 : -1);
                pi = ((x1-x0>=0) ? 1 : 0);
                pj = ((y1-y0>=0) ? 1 : 0);

                map_i   = DIV(x0-offset_x, n);
                map_j   = DIV(y0-offset_y, n);
                map_i_1 = DIV(x1-offset_x, n);
                map_j_1 = DIV(y1-offset_y, n);

                clutter_type = map_j*npts_x + map_i;
                if ( m_type == CConst::PropClutterFull ) {
                    mmx_a->a->append(DoubleIntIntClass(1.0, insys_rtp_idx, 2*clutter_type+1+useheight));
                } else if ( m_type == CConst::PropClutterSymFull ) {
                    mmx_a->a->append(DoubleIntIntClass(0.5, insys_rtp_idx, 2*clutter_type+1+useheight));
                }

                int first_segment = 1;
                double sum = 0.0;
                double prev_sum;

                while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

                    // Offset by 1/2 resolution
                    q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

                    prev_sum = sum;
                    if ( (q > 0) || ( (q == 0) && (pi) ) ) {
                        if ( (m_type == CConst::PropClutterWtExpo) || (m_type == CConst::PropClutterWtExpoSlope) ) {
                            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        }
                        else if ( (m_type == CConst::PropClutterExpoLinear) ) {
                            sum = xy_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        }
                        else if ( (m_type == CConst::PropClutterGlobal) ) {
                            sum = xy_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        }
                        else {
                            sum = total_dist*(n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
                        }
                        map_i += si;
                    } else {
                        if ( (m_type == CConst::PropClutterWtExpo) || (m_type == CConst::PropClutterWtExpoSlope) ) {
                            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        }
                        else if ( (m_type == CConst::PropClutterExpoLinear) ) {
                            sum = xy_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        }
                        else if ( (m_type == CConst::PropClutterGlobal) ) {
                            sum = xy_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        }
                        else {
                            sum = total_dist*(n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
                        }
                        map_j += sj;
                    }

#if CLUTTER_DEBUG 
                    printf("map_i = %d (%d) map_j = %d (%d) curr_dist = %15.8f prev_dist = %15.8f delta_dist = %15.8f clutter_type = %d\n",
                            map_i, x1/n, map_j, y1/n, sum, prev_sum, sum - prev_sum, clutter_type);
#endif

                    if ( m_type == CConst::PropClutterSimp ) {
                        if (first_segment) {
                            mmx_a->a->append( DoubleIntIntClass(log( sum )/log(10.0) + m_intercept, insys_rtp_idx, 2*clutter_type + useheight));
                            first_segment = 0;
                        } else {
                            mmx_a->a->append(DoubleIntIntClass( log( sum / prev_sum )/log(10.0), insys_rtp_idx, clutter_type ));
                        }
                    } else if ( m_type == CConst::PropClutterFull ) {
                        if (first_segment) {
                            mmx_a->a->append(DoubleIntIntClass( log( sum ) / log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                            first_segment = 0;
                        } else {
                            mmx_a->a->append(DoubleIntIntClass( log( sum / prev_sum ) / log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                        }
                    } else if ( m_type == CConst::PropClutterSymFull ) {
                        if (first_segment) {
                            mmx_a->a->append(DoubleIntIntClass( 0.5*log( (sum*total_dist)/(total_dist-sum) )/log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                            first_segment = 0;
                        } else {
                            mmx_a->a->append(DoubleIntIntClass( 0.5*log( (sum*(total_dist-prev_sum))/(prev_sum*(total_dist-sum)) ) / log(10.0),
                                insys_rtp_idx, 2*clutter_type + useheight ));
                        }
                    } else if ( m_type == CConst::PropClutterWtExpo ) {
                        mmx_a->a->append(DoubleIntIntClass( (sum-prev_sum)*log(total_dist)/log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                        mmx_a->a->append(DoubleIntIntClass( (sum-prev_sum), insys_rtp_idx, 2*clutter_type + 1 + useheight ));
                    } else if ( m_type == CConst::PropClutterWtExpoSlope ) {
                        mmx_a->a->append(DoubleIntIntClass( (sum-prev_sum)*log(total_dist/r0)/log(10.0), insys_rtp_idx, clutter_type + useheight ));
                    }
                    else if ( m_type == CConst::PropClutterExpoLinear ) {
                        mmx_a->a->append(DoubleIntIntClass( sum-prev_sum, insys_rtp_idx, clutter_type + useheight ));
                    }
                    else if ( m_type == CConst::PropClutterGlobal) {
                        mmx_a->a->append(DoubleIntIntClass( sum-prev_sum, insys_rtp_idx, clutter_type + useheight ));
                    }

                    clutter_type = map_j*npts_x + map_i;
                }

                if ( m_type == CConst::PropClutterSimp ) {
                    if (first_segment) {
                        mmx_a->a->append(DoubleIntIntClass( log( total_dist )/log(10.0) + m_intercept, insys_rtp_idx, clutter_type + useheight ));
                        first_segment = 0;
                    } else {
                        mmx_a->a->append(DoubleIntIntClass( log( total_dist / sum )/log(10.0), insys_rtp_idx, clutter_type + useheight ));
                    }
                } else if ( m_type == CConst::PropClutterFull ) {
                    if (first_segment) {
                        mmx_a->a->append(DoubleIntIntClass( log( total_dist ) / log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                        first_segment = 0;
                    } else {
                        mmx_a->a->append(DoubleIntIntClass( log( total_dist / sum ) / log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                    }
                } else if ( m_type == CConst::PropClutterSymFull ) {
                    CORE_DUMP;
                    // xxxxxxx mmx_a[insys_rtp_idx][2*clutter_type + 1+ useheight] += 0.5;
                    // xxxxxx  if (first_segment) {
                    // xxxxxx      mmx_a[insys_rtp_idx][2*clutter_type + useheight] = log( total_dist ) / log(10.0);
                    // xxxxxx  } else {
                    // xxxxxx      mmx_a[insys_rtp_idx][2*clutter_type + useheight] =
                    //                 0.5*log( ((total_dist-sum)*total_dist)/sum ) / log(10.0);
                    // xxxxxx  }
                } else if ( m_type == CConst::PropClutterWtExpo) {
                        mmx_a->a->append(DoubleIntIntClass( (1.0 - sum)*log(total_dist)/log(10.0), insys_rtp_idx, 2*clutter_type + useheight ));
                        mmx_a->a->append(DoubleIntIntClass( (1.0 - sum), insys_rtp_idx, 2*clutter_type + 1 + useheight ));
                } else if ( m_type == CConst::PropClutterWtExpoSlope) {
                        mmx_a->a->append(DoubleIntIntClass( (1.0 - sum)*log(total_dist/r0)/log(10.0), insys_rtp_idx, clutter_type + useheight ));
                        mmx_a->a->append(DoubleIntIntClass( 1.0, insys_rtp_idx, num_var-1 ));
                }
                else if ( m_type == CConst::PropClutterExpoLinear) {
                        mmx_a->a->append(DoubleIntIntClass( xy_dist - sum, insys_rtp_idx, clutter_type + useheight ));
                        mmx_a->a->append(DoubleIntIntClass( 1.0, insys_rtp_idx, num_var-1 ));
                }
                else if ( m_type == CConst::PropClutterGlobal) {
                        mmx_a->a->append(DoubleIntIntClass( xy_dist - sum, insys_rtp_idx, clutter_type + useheight ));
                        mmx_a->a->append(DoubleIntIntClass( 1.0, insys_rtp_idx, num_var-1 ));
                }

                insys_rtp_idx ++;
            }
        }
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----create_svd_matrices loop 1 end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----create_svd_matrices loop 1 end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif 

    int diism = DoubleIntIntClass::sortby;
    DoubleIntIntClass::sortby = CConst::NumericInt;
    mmx_a->a->sort();
    DoubleIntIntClass::sortby = diism;

    mmx_a->create_col_list();
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: GenericClutterPropModelClass::solve_clutter_coe                        ****/
/******************************************************************************************/
double GenericClutterPropModelClass::solve_clutter_coe(int num_eqn, int num_var, double **mx_a, double *vec_b)
{
    double   error;

    mvec_x = DVECTOR(num_var);

    error  = svd_solve(mx_a, vec_b, num_eqn, num_var, mvec_x);


    for (int i=0; i<num_var; i++)
        printf("mvec_x[%d] = %f \n", i, mvec_x[i]);

#if 0
// xxxxxxxx MM
    double errsq = 0.0;
    for (int rtp_idx=0; rtp_idx<=num_eqn-1; rtp_idx++) {
        double val = 0.0;
        for(int i=0; i<=num_var-1; i++) {
            val += mx_a[rtp_idx][i]*mvec_x[i];
        }
        errsq += (val - vec_b[rtp_idx])*(val - vec_b[rtp_idx]);
        // std::cout << "ERROR " << rtp_idx << " = " << (val - vec_b[rtp_idx])*(val - vec_b[rtp_idx]) << std::endl;
    }
    std::cout << "MANUAL RMS ERROR               " << sqrt(errsq/num_eqn) << std::endl;
#endif

    std::cout << "\n===================================================\n";
    std::cout << "RMS ERROR ( BY SVD )           " << sqrt(error/num_eqn) << std::endl;
    //std::cout << "===================================================\n";

    return(error);
}

/* 
 * Path Loss from point(cs_x, cs_y) to point(pt_x, pt_y).    */
double ClutterWtExpoPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{
    int clutter_type;
    CellClass *cell = sector->parent_cell;
    MapClutterClass* map_clutter = np->map_clutter;

    double prop_db        = 0.0;

    // Values with unit system resolution.
    int offset_x = map_clutter->offset_x;
    int offset_y = map_clutter->offset_y;
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    int prev_clutter_type;
    double total_dist;

    if ( angle_deg == 0.0 ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (double) (x1 - x0)*np->resolution;
    double dy = (double) (y1 - y0)*np->resolution;
    double dz = -sector->antenna_height;
    if (fabs(dz) < np->resolution) dz = -np->resolution;

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );

    int    n, si, sj, pi, pj, q;
    int    map_i, map_j;
    int    map_i_1, map_j_1;

    n = map_clutter->map_sim_res_ratio;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    clutter_type = map_clutter->get_clutter_type(map_i, map_j);
    if (clutter_type == -1) {
        clutter_type = 0;
    }

    double sum      = 0.0;
    double prev_sum;
    double total_coeff = 0.0;
    double total_const = 0.0;

#if CLUTTER_DEBUG
        printf("offset_x = %d offset_y = %d  map_i = %d map_j = %d map_i_1 = %d map_j_1 = %d \n",
                offset_x, offset_y, map_i, map_j, map_i_1, map_j_1);
#endif

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

        prev_sum = sum;
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }

#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d map_i_1 = %d map_j_1 = %d curr_frac = %15.8f prev_frac = %15.8f delta_frac = %15.8f clutter_type = %d (%s)\n",
                map_i, map_j, sum, prev_sum, sum - prev_sum, clutter_type, map_clutter->description[clutter_type]);
#endif

        total_const += mvec_x[2*clutter_type + 1 + useheight] * (sum - prev_sum);
        total_coeff += mvec_x[2*clutter_type + useheight] * (sum - prev_sum);

        prev_clutter_type = clutter_type;
        clutter_type = map_clutter->get_clutter_type(map_i, map_j);
        if (clutter_type == -1) {
            clutter_type = prev_clutter_type;
        }
    }

    total_const += mvec_x[2*clutter_type + 1 + useheight] * (1.0 - sum);
    total_coeff += mvec_x[2*clutter_type + useheight] * (1.0 - sum);

    prop_db = total_const + total_coeff*log(total_dist)/log(10.0);

    if (useheight) {
        prop_db += mvec_x[0] * log(-dz)/log(10.0);
    }

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}

/******************************************************************************************/
/**** FUNCTION: ClutterWtExpoSlopePropModelClass::prop_power_loss()                    ****/
/******************************************************************************************/
double ClutterWtExpoSlopePropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{
    int clutter_type;
    CellClass *cell = sector->parent_cell;

    double prop_db        = 0.0;

    // Values with unit system resolution.
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    int prev_clutter_type;
    double total_dist;

    if ( angle_deg == 0.0 ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (double) (x1 - x0)*np->resolution;
    double dy = (double) (y1 - y0)*np->resolution;
    double dz = -sector->antenna_height;
    if (fabs(dz) < np->resolution) dz = -np->resolution;

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );

    int    n, si, sj, pi, pj, q;
    int    map_i, map_j;
    int    map_i_1, map_j_1;

    n = clutter_sim_res_ratio;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
        clutter_type = map_j*npts_x + map_i;
    } else {
        clutter_type = 0;
    }

    double sum      = 0.0;
    double prev_sum;
    double total_coeff = 0.0;

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

        prev_sum = sum;
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }

#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d sum = %6.4f prev_sum = %6.4f delta_sum = %6.4f clutter_type = %d\n",
                map_i, map_j, sum, prev_sum, sum - prev_sum, clutter_type);
#endif
        total_coeff += mvec_x[clutter_type + useheight] * (sum - prev_sum);

        prev_clutter_type = clutter_type;
        if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
            clutter_type = map_j*npts_x + map_i;
        } else {
            clutter_type = prev_clutter_type;
        }
    }

    // last clutter
    total_coeff += mvec_x[clutter_type + useheight] * (1.0 - sum);

    // mvec_x[num_clutter_type + useheight] -- constant value
    // total_coeff*log(total_dist/r0)/log(10.0);  ??
    prop_db = mvec_x[num_clutter_type + useheight] + total_coeff*log(total_dist/r0)/log(10.0);

    if (useheight) {
        prop_db += mvec_x[0] * log(-dz)/log(10.0);
    }

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}


/******************************************************************************************/
/**** FUNCTION: ClutterExpoLinearPropModelClass::prop_power_loss()                    ****/
/******************************************************************************************/
double ClutterExpoLinearPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{
    int clutter_type;
    CellClass *cell = sector->parent_cell;

    double prop_db = 0.0;

    // Values with unit system resolution.
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    int prev_clutter_type;
    double total_dist, xy_dist;

    if ( angle_deg == 0.0 ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (double) (x1 - x0)*np->resolution;
    double dy = (double) (y1 - y0)*np->resolution;
    double dz = -sector->antenna_height;
    if (fabs(dz) < np->resolution) dz = -np->resolution;

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );
    xy_dist    = (double) sqrt( dx*dx + dy*dy );

    int    n, si, sj, pi, pj, q;
    int    map_i, map_j;
    int    map_i_1, map_j_1;

    n = clutter_sim_res_ratio;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
        clutter_type = map_j*npts_x + map_i;
    } else {
        clutter_type = 0;
    }

    double sum      = 0.0;
    double prev_sum;
    double total_coeff = 0.0;

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

        prev_sum = sum;
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }

#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d curr_frac = %15.8f prev_frac = %15.8f delta_frac = %15.8f clutter_type = %d\n",
                map_i, map_j, sum, prev_sum, sum - prev_sum, clutter_type);
#endif

        total_coeff += mvec_x[clutter_type + useheight] * (sum - prev_sum);

        prev_clutter_type = clutter_type;
        if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
            clutter_type = map_j*npts_x + map_i;
        } else {
            clutter_type = prev_clutter_type;
        }
    }

    total_coeff += mvec_x[clutter_type + useheight] * (1.0 - sum);

    prop_db = 10.0*exponent*log(total_dist<r0?r0:total_dist)/log(10.0) + mvec_x[num_clutter_type + useheight] + total_coeff*xy_dist;

    if (useheight) {
        prop_db += mvec_x[0] * log(-dz)/log(10.0);
    }

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}


/******************************************************************************************/
/**** FUNCTION: ClutterGlobalPropModelClass::prop_power_loss()                    ****/
/******************************************************************************************/
double ClutterGlobalPropModelClass::prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int /* useheight */, double angle_deg)
{
    int clutter_type;
    CellClass *cell = sector->parent_cell;

    double prop_db = 0.0;

    // Values with unit system resolution.
    int x0 = cell->posn_x;
    int y0 = cell->posn_y;
    int x1, y1;
    int prev_clutter_type;
    double total_dist, xy_dist;

    if ( angle_deg == 0.0 ) {
        x1 = x0 + delta_x;
        y1 = y0 + delta_y;
    } else {
        x1 = x0 + (int) floor(delta_x*cos(angle_deg)-delta_y*sin(angle_deg) + 0.5);
        y1 = y0 + (int) floor(delta_x*sin(angle_deg)+delta_y*cos(angle_deg) + 0.5);
    }
    double dx = (double) (x1 - x0)*np->resolution;
    double dy = (double) (y1 - y0)*np->resolution;
    double dz = -sector->antenna_height;
    if (fabs(dz) < np->resolution) dz = -np->resolution;

    total_dist = (double) sqrt( dx*dx + dy*dy + dz*dz );
    xy_dist    = (double) sqrt( dx*dx + dy*dy );

    int    n, si, sj, pi, pj, q;
    int    map_i, map_j;
    int    map_i_1, map_j_1;

    n = clutter_sim_res_ratio;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);
    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    map_i   = DIV(x0-offset_x, n);
    map_j   = DIV(y0-offset_y, n);
    map_i_1 = DIV(x1-offset_x, n);
    map_j_1 = DIV(y1-offset_y, n);

    if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
        clutter_type = map_j*npts_x + map_i;
    } else {
        clutter_type = 0;
    }

    double sum      = 0.0;
    double prev_sum;
    double total_coeff = 0.0;

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-offset_y)*(x0-offset_x) - (x1-offset_x)*(y0-offset_y)) - (x1-x0) + (y1-y0) )*si*sj;

        prev_sum = sum;
        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            sum = (n*(map_i+pi) - x0 + offset_x - 0.5)/(x1-x0);
            map_i += si;
        } else {
            sum = (n*(map_j+pj) - y0 + offset_y - 0.5)/(y1-y0);
            map_j += sj;
        }

#if CLUTTER_DEBUG
        printf("map_i = %d map_j = %d curr_frac = %15.8f prev_frac = %15.8f delta_frac = %15.8f clutter_type = %d\n",
                map_i, map_j, sum, prev_sum, sum - prev_sum, clutter_type);
#endif

        total_coeff += mvec_x[clutter_type + useheight] * (sum - prev_sum);

        prev_clutter_type = clutter_type;
        if ( (map_i >= 0) && (map_i <= npts_x-1) && (map_j >= 0) && (map_j <= npts_y-1) ) {
            clutter_type = map_j*npts_x + map_i;
        } else {
            clutter_type = prev_clutter_type;
        }
    }

    total_coeff += mvec_x[clutter_type + useheight] * (1.0 - sum);

    // CG MOD - need to include the path loss of segment global PM
    prop_db = mvec_x[num_clutter_type + useheight] + total_coeff*xy_dist
        + 10.0 * log(globPm->prop_power_loss(np, sector, (x1-x0), (y1-y0)))/log(10.0) ;

    /*
        printf("mvec_x[num_clutter_type + useheight] %f total_coeff*xy_dist %f \n",
                mvec_x[num_clutter_type + useheight], total_coeff*xy_dist);
        printf("prop_db1 %f \n", prop_db);
     */

    if (useheight) prop_db += mvec_x[0] * log(-dz)/log(10.0);

    double prop = exp(prop_db*log(10.0)/10.0);

    return(prop);
}


/*******************************************************************
 * FUNCTION: calculate segment propagation model
 *           no adjusting antenna angles
 *           no clutter
 *           no warning
 * RETURN: True if suceed to create global propagation model
 *         else return False
 * NOTE: move from comp_prop_model, modified for ClutterGlobalPropModel
 *******************************************************************/
bool ClutterGlobalPropModelClass::comp_global_model(NetworkClass *np, int num_scan_index, int *scan_index_list, double min_logd_threshold )
{
    //simple variable definition
    int    cell_idx;          //cell loop variable
    int    sector_idx;        //sector loop variable
    int    scan_idx;
    int    rtd_idx;           //road test point loop variable for each sector
    int    pt_idx;            //road test point loop variable for total
    int    num_road_test_pt;  //the number of the test point of all the cell.
    int    scan_list_idx;
    double height;            //the height of the base station of each sector
    double min_height;        //the average height of CS antenna to compute propagation model
    double error = 0.0;       //error of the propagation model.
    double dx;
    double dy;
    double dz;
    double logd, min_logd;

    //struct variable definition
    CellClass       *cell   = NULL; //pointer of cell struct
    SectorClass     *sector = NULL; //pointer of sector struct
    RoadTestPtClass *rtp;           //pointer of the test point

    //matrix and array of the singular value decomposition
    double *vec_b;  //vector B , Mx1
    double *vec_logd;//vector logd

    /* not create PM
     * must allocate struct for propataion model before calling this function
     **************************************************************/
    globPm->useheight = useheight;
    globPm->vec_k = DVECTOR(num_clutter_type+2*useheight);
    for (int i=0; i<num_clutter_type+2*useheight; i++) globPm->vec_k[i] = 0.0;

    //get the number of the road test points
    num_road_test_pt = 0;
    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        scan_idx = (rtp->sector_idx << np->bit_cell) | rtp->cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            num_road_test_pt++;
        }
    }

    // allocate logd vector for svd solve
    vec_logd = DVECTOR(num_road_test_pt);
    vec_b = DVECTOR(num_road_test_pt);  //for optimize_clutter_coeffs will
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        vec_b[pt_idx]   = 0.0;  //the vector is initial to zero too.
        vec_logd[pt_idx] = 0.0;  //the vector is initial to zero too.
    }

    /*
    *************************************************************************
    * Prepare the vector logd and matrix for the SVD
    *************************************************************************
    */
    pt_idx = 0;
    min_height = -1.0;
    min_logd = 0.0;
    double avg_max_logd = 0.0;
    double* cell_max_logd = (double*)malloc(np->num_cell*sizeof(double));
    for(int i=0; i<np->num_cell; i++)
        cell_max_logd[i] = 0.0;
    
    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*np->road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];

            height = sector->antenna_height;

            if ( (min_height == -1.0) || (height < min_height) ) {
                min_height = height;
            }

            dx = (rtp->posn_x - cell->posn_x) * np->resolution;
            dy = (rtp->posn_y - cell->posn_y) * np->resolution;
            dz = - sector->antenna_height;
            logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);

            if (logd >= min_logd_threshold) {
                vec_logd[pt_idx] = logd;

                // TYC 
                if(logd > cell_max_logd[cell_idx])
                    cell_max_logd[cell_idx] = logd;
                    
                if ( (pt_idx == 0) || (logd < min_logd) ) {
                    min_logd = logd;
                }

                pt_idx = pt_idx + 1;
            }
        }
    }
    
    double sum_max_logd = 0.0;
    for(int i=0; i<np->num_cell; i++)
        sum_max_logd += cell_max_logd[i];
    avg_max_logd = sum_max_logd / num_scan_index;

    /* prepared for propagation model computation
     ********************************************************************************/
    pt_idx = 0;
    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            dx = (rtp->posn_x - cell->posn_x) * np->resolution;
            dy = (rtp->posn_y - cell->posn_y) * np->resolution;
            dz = -sector->antenna_height;
            logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
            if (logd >= min_logd_threshold) {
                AntennaClass *antenna = np->antenna_type_list[sector->antenna_type];
                double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]   = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx = pt_idx + 1;
            }
        }
    }

    /* compute propagation model
     *******************************************************************/
    if (!globPm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,avg_max_logd,error)) {
        sprintf(np->msg, "WARNING: Problems creating prop_model for sector group.\n");
        PRMSG(stdout, np->msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<np->bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> np->bit_cell;
            sprintf(np->msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, np->msg);
        }
        sprintf(np->msg, "\n");
        PRMSG(stdout, np->msg);

        /*
        if (err) {
            printf("error \n");
            error_state = 1;
        } else {
            warning_state = 1;
        }
        return;
        */
    }

    free(vec_b);
    free(vec_logd);

    if (!globPm->adjust_near_field(min_logd, min_logd_threshold, np->frequency, np->msg, 1)) {
        PRMSG(stdout, np->msg);
        sprintf(np->msg, "WARNING: Problems creating prop_model for sector group.\n");
        PRMSG(stdout, np->msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<np->bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> np->bit_cell;
            sprintf(np->msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, np->msg);
        }
        sprintf(np->msg, "\n");
        PRMSG(stdout,np->msg);

        /*
        if (err) {
            printf("error \n");
            error_state = 1;
        } else {
            warning_state = 1;
        }
        */
    }

    return (true);
}


double GenericClutterPropModelClass::compute_error(int num_eqn, int num_var, double **mx_a,double *vec_b)
{
    int eqn_idx, var_idx;
    double sum, tot_sqerr;

    tot_sqerr = 0.0;
    for (eqn_idx=0; eqn_idx<=num_eqn-1; eqn_idx++) {
        sum = 0.0;

        for (var_idx=0; var_idx<=num_var-1; var_idx++) {
            sum += mx_a[eqn_idx][var_idx]*mvec_x[var_idx];
        }

        tot_sqerr += (sum-vec_b[eqn_idx])*(sum-vec_b[eqn_idx]);
    }

    return( sqrt(tot_sqerr/num_eqn) );
}

double GenericClutterPropModelClass::compute_error(SparseMatrixClass *mx_a,double *vec_b)
{
    int eqn_idx, var_idx, idx;
    double sum, tot_sqerr;
    DoubleIntIntClass mx_elem;

    tot_sqerr = 0.0;
    eqn_idx = -1;

    for (idx=0; idx<=mx_a->a->getSize()-1; idx++) {
        mx_elem = (*mx_a->a)[idx];
        if (mx_elem.getInt(0) != eqn_idx) {
            if (eqn_idx != -1) {
                tot_sqerr += (sum-vec_b[eqn_idx])*(sum-vec_b[eqn_idx]);
            }
            eqn_idx = mx_elem.getInt(0);
            sum = 0.0;
        }
        var_idx = mx_elem.getInt(1);
        sum += mx_elem.getDouble()*mvec_x[var_idx];
    }

    if (eqn_idx != -1) {
        tot_sqerr += (sum-vec_b[eqn_idx])*(sum-vec_b[eqn_idx]);
    }

    return( sqrt(tot_sqerr/mx_a->num_row) );
}

void ClutterWtExpoPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 2) {
        mvec_x = DVECTOR(2*num_clutter_type + useheight);
    }

    if (param_idx == 0 ) {
        sprintf(str, "USE_HEIGHT:");        type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 1 ) {
        sprintf(str, "NUM_CLUTTER_TYPE:");  type=CConst::NumericInt;    iptr = &num_clutter_type;
    } else {
        i = param_idx-2;
        sprintf(str, "C_%d:", i);           type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterWtExpoPropModelClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + 2*num_clutter_type + 2; 

    // DBG
    //std::cout << " ClutterWtExpoPropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterWtExpoPropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTERWTEXPO\n");
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    NUM_CLUTTER_TYPE: %d\n", num_clutter_type);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<2*num_clutter_type; c_idx++) {
        if ( (c_idx)%2 == 0 ) {
            sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        } else {
            sprintf(msg, "    C_%d: %9.7f  #CONSTANT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx/2 );
            PRMSG(fp, msg);
        }
    }
}

void ClutterWtExpoPropModelClass::split_clutter(NetworkClass *np)
{
    int i, j, old_i, old_j;
    int c_idx, old_c_idx;
    double *old_mvec_x = mvec_x;

    num_clutter_type *= 4;
    mvec_x = DVECTOR(2*num_clutter_type + useheight);

    if (useheight) {
        mvec_x[0] = old_mvec_x[0];
    }

    for (c_idx=0; c_idx<=num_clutter_type-1; c_idx++) {
        i = c_idx % np->map_clutter->npts_x;
        j = c_idx / np->map_clutter->npts_x;

        old_i = i/2;
        old_j = j/2;

        old_c_idx = old_j*(np->map_clutter->npts_x/2) + old_i;

        mvec_x[useheight + 2*c_idx    ] = old_mvec_x[useheight + 2*old_c_idx    ];
        mvec_x[useheight + 2*c_idx + 1] = old_mvec_x[useheight + 2*old_c_idx + 1];
    }

    free(old_mvec_x);
}

void ClutterWtExpoSlopePropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 6) {
        num_clutter_type = npts_x*npts_y;
        mvec_x = DVECTOR(num_clutter_type + useheight + 1);
    }

    if (param_idx == 0 ) {
        sprintf(str, "OFFSET_X:");              type=CConst::NumericInt;    iptr = &offset_x;
    } else if (param_idx == 1 ) {
        sprintf(str, "OFFSET_Y:");              type=CConst::NumericInt;    iptr = &offset_y;
    } else if (param_idx == 2 ) {
        sprintf(str, "NPTS_X:");                type=CConst::NumericInt;    iptr = &npts_x;
    } else if (param_idx == 3 ) {
        sprintf(str, "NPTS_Y:");                type=CConst::NumericInt;    iptr = &npts_y;
    } else if (param_idx == 4 ) {
        sprintf(str, "CLUTTER_SIM_RES_RATIO:"); type=CConst::NumericInt;    iptr = &clutter_sim_res_ratio;
    } else if (param_idx == 5 ) {
        sprintf(str, "USE_HEIGHT:");            type=CConst::NumericInt;    iptr = &useheight;
    } else if (param_idx == 6 ) {
        sprintf(str, "P0:");                type=CConst::NumericDouble; dptr = &(mvec_x[num_clutter_type + useheight]);
    } else if (param_idx == 7 ) {
        sprintf(str, "R0:");                type=CConst::NumericDouble; dptr = &r0;
    } else {
        i = param_idx-8;
        sprintf(str, "C_%d:", i);           type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterWtExpoSlopePropModelClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + num_clutter_type + 8; 

    // DBG
    //std::cout << " ClutterWtExpoSlopePropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterWtExpoSlopePropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTERWTEXPOSLOPE\n");
    chptr += sprintf(chptr, "    OFFSET_X: %d\n", offset_x);
    chptr += sprintf(chptr, "    OFFSET_Y: %d\n", offset_y);
    chptr += sprintf(chptr, "    NPTS_X: %d\n", npts_x);
    chptr += sprintf(chptr, "    NPTS_Y: %d\n", npts_y);
    chptr += sprintf(chptr, "    CLUTTER_SIM_RES_RATIO: %d\n", clutter_sim_res_ratio);
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    P0: %15.10f\n", mvec_x[num_clutter_type + useheight]);
    chptr += sprintf(chptr, "    R0: %15.10f\n", r0);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<num_clutter_type; c_idx++) {
        sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx );
        PRMSG(fp, msg);
    }
}

void ClutterWtExpoSlopePropModelClass::split_clutter(NetworkClass *)
{
    int i, j, old_i, old_j;
    int c_idx, old_c_idx;
    double *old_mvec_x = mvec_x;
    int old_num_clutter_type = num_clutter_type / 4;

    mvec_x = DVECTOR(num_clutter_type + useheight + 1);

    if (useheight) {
        mvec_x[0] = old_mvec_x[0];
    }

    for (c_idx=0; c_idx<=num_clutter_type-1; c_idx++) {
        i = c_idx % npts_x;
        j = c_idx / npts_x;

        old_i = i/2;
        old_j = j/2;

        old_c_idx = old_j*(npts_x/2) + old_i;

        mvec_x[useheight + c_idx    ] = old_mvec_x[useheight + old_c_idx    ];
    }
    mvec_x[num_clutter_type + useheight] = old_mvec_x[old_num_clutter_type + useheight];

    free(old_mvec_x);
}

/******************************************************************************************/
/**** FUNCTION: ClutterWtExpoSlopePropModelClass::report_clutter()                     ****/
/******************************************************************************************/
void ClutterWtExpoSlopePropModelClass::report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p)
{
    int p_minx, p_maxx, p_miny, p_maxy, x_idx, y_idx, num_pt, posn_x, posn_y, clutter_idx;
    double slope, sum_slope;
    char *chptr;

    if (p) {
        p->comp_bdy_min_max(p_minx, p_maxx, p_miny, p_maxy);

        p_minx = DIV(p_minx - offset_x, clutter_sim_res_ratio);
        p_maxx = DIV(p_maxx - offset_x, clutter_sim_res_ratio) + 1;
        p_miny = DIV(p_miny - offset_y, clutter_sim_res_ratio);
        p_maxy = DIV(p_maxy - offset_y, clutter_sim_res_ratio) + 1;

        if (p_minx < 0) { p_minx = 0; } else if (p_minx > npts_x-1) { p_minx = npts_x-1; }
        if (p_maxx < 0) { p_maxx = 0; } else if (p_maxx > npts_x-1) { p_maxx = npts_x-1; }
        if (p_miny < 0) { p_miny = 0; } else if (p_miny > npts_y-1) { p_miny = npts_y-1; }
        if (p_maxy < 0) { p_maxy = 0; } else if (p_maxy > npts_y-1) { p_maxy = npts_y-1; }
    } else {
        p_minx = 0;
        p_maxx = npts_x-1;
        p_miny = 0;
        p_maxy = npts_y-1;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Clutter Propagation Model Report                                ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "NAME: %s\n", get_strid());
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "  IDX CLT_X CLT_Y           SLOPE\n");
    chptr += sprintf(chptr, "---------------------------------\n");
    PRMSG(fp, np->msg);
    num_pt = 0;
    sum_slope = 0.0;
    for (y_idx = p_miny; y_idx<=p_maxy; y_idx++) {
        posn_y = (int) floor((offset_y-0.5) + (y_idx + 0.5)*clutter_sim_res_ratio);
        for (x_idx = p_minx; x_idx<=p_maxx; x_idx++) {
            posn_x = (int) floor((offset_x-0.5) + (x_idx + 0.5)*clutter_sim_res_ratio);
            if ((!p) || (p->in_bdy_area(posn_x, posn_y))) {
                num_pt++;
                clutter_idx = x_idx + y_idx*npts_x;
                slope = mvec_x[useheight + clutter_idx];
                sum_slope += slope;
                chptr = np->msg;
                chptr += sprintf(chptr, "%5d %5d %5d %15.10f\n", num_pt, x_idx, y_idx, slope);
                PRMSG(fp, np->msg);
            }
        }
    }
    chptr = np->msg;
    chptr += sprintf(chptr, "---------------------------------\n");
    chptr += sprintf(chptr, "AVERAGE SLOPE:    %15.10f\n", sum_slope / num_pt);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    PRMSG(fp, np->msg);
}
/******************************************************************************************/

void ClutterExpoLinearPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i;

    if (param_idx == 6) {
        num_clutter_type = npts_x*npts_y;
        mvec_x = DVECTOR(num_clutter_type + useheight + 1);
    }

    if (param_idx == 0 ) {
        sprintf(str, "OFFSET_X:");              type=CConst::NumericInt; iptr = &offset_x;
    } else if (param_idx == 1 ) {
        sprintf(str, "OFFSET_Y:");              type=CConst::NumericInt; iptr = &offset_y;
    } else if (param_idx == 2 ) {
        sprintf(str, "NPTS_X:");                type=CConst::NumericInt; iptr = &npts_x;
    } else if (param_idx == 3 ) {
        sprintf(str, "NPTS_Y:");                type=CConst::NumericInt; iptr = &npts_y;
    } else if (param_idx == 4 ) {
        sprintf(str, "CLUTTER_SIM_RES_RATIO:"); type=CConst::NumericInt; iptr = &clutter_sim_res_ratio;
    } else if (param_idx == 5 ) {
        sprintf(str, "USE_HEIGHT:");            type=CConst::NumericInt; iptr = &useheight;
    } else if (param_idx == 6 ) {
        sprintf(str, "P0:");                 type=CConst::NumericDouble; dptr = &(mvec_x[num_clutter_type + useheight]);
    } else if (param_idx == 7 ) {
        sprintf(str, "R0:");                 type=CConst::NumericDouble; dptr = &r0;
    } else if (param_idx == 8 ) {
        sprintf(str, "EXPONENT:");           type=CConst::NumericDouble; dptr = &exponent;
    } else {
        i = param_idx-9;
        sprintf(str, "C_%d:", i);            type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterExpoLinearPropModelClass::comp_num_prop_param()
{
    int num_param;
    num_param = useheight + num_clutter_type + 9; 

    // DBG
    //std::cout << " ClutterExpoLinearPropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterExpoLinearPropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int  c_idx;
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTEREXPOLINEAR\n");
    chptr += sprintf(chptr, "    OFFSET_X: %d\n", offset_x);
    chptr += sprintf(chptr, "    OFFSET_Y: %d\n", offset_y);
    chptr += sprintf(chptr, "    NPTS_X: %d\n", npts_x);
    chptr += sprintf(chptr, "    NPTS_Y: %d\n", npts_y);
    chptr += sprintf(chptr, "    CLUTTER_SIM_RES_RATIO: %d\n", clutter_sim_res_ratio);
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    P0: %15.10f\n", mvec_x[num_clutter_type + useheight]);
    chptr += sprintf(chptr, "    R0: %15.10f\n", r0);
    chptr += sprintf(chptr, "    EXPONENT: %15.10f\n", exponent);
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }

    for (c_idx=0; c_idx<num_clutter_type; c_idx++) {
        sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", c_idx+useheight, mvec_x[useheight+c_idx], c_idx );
        PRMSG(fp, msg);
    }
}

void ClutterExpoLinearPropModelClass::split_clutter(NetworkClass *)
{
    int i, j, old_i, old_j;
    int c_idx, old_c_idx;
    double *old_mvec_x = mvec_x;
    int old_num_clutter_type = num_clutter_type / 4;

    mvec_x = DVECTOR(num_clutter_type + useheight + 1);

    if (useheight) {
        mvec_x[0] = old_mvec_x[0];
    }

    for (c_idx=0; c_idx<=num_clutter_type-1; c_idx++) {
        i = c_idx % npts_x;
        j = c_idx / npts_x;

        old_i = i/2;
        old_j = j/2;

        old_c_idx = old_j*(npts_x/2) + old_i;

        mvec_x[useheight + c_idx] = old_mvec_x[useheight + old_c_idx];

        // CG DBG
        /*
        printf("oldi %d oldj %d i %d j %d mvec_x[useheight + c_idx] %6.4f \n",
                old_i, old_j, i, j, mvec_x[useheight + c_idx]);
        */
    }
    mvec_x[num_clutter_type + useheight] = old_mvec_x[old_num_clutter_type + useheight];

    free(old_mvec_x);
}

/******************************************************************************************/
/**** FUNCTION: ClutterExpoLinearPropModelClass::report_clutter()                     ****/
/******************************************************************************************/
void ClutterExpoLinearPropModelClass::report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p)
{
    int p_minx, p_maxx, p_miny, p_maxy, x_idx, y_idx, num_pt, posn_x, posn_y, clutter_idx;
    double slope, sum_slope;
    char *chptr;

    if (p) {
        p->comp_bdy_min_max(p_minx, p_maxx, p_miny, p_maxy);

        p_minx = DIV(p_minx - offset_x, clutter_sim_res_ratio);
        p_maxx = DIV(p_maxx - offset_x, clutter_sim_res_ratio) + 1;
        p_miny = DIV(p_miny - offset_y, clutter_sim_res_ratio);
        p_maxy = DIV(p_maxy - offset_y, clutter_sim_res_ratio) + 1;

        if (p_minx < 0) { p_minx = 0; } else if (p_minx > npts_x-1) { p_minx = npts_x-1; }
        if (p_maxx < 0) { p_maxx = 0; } else if (p_maxx > npts_x-1) { p_maxx = npts_x-1; }
        if (p_miny < 0) { p_miny = 0; } else if (p_miny > npts_y-1) { p_miny = npts_y-1; }
        if (p_maxy < 0) { p_maxy = 0; } else if (p_maxy > npts_y-1) { p_maxy = npts_y-1; }
    } else {
        p_minx = 0;
        p_maxx = npts_x-1;
        p_miny = 0;
        p_maxy = npts_y-1;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Clutter Propagation Model Report                                ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "NAME: %s\n", get_strid());
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "  IDX CLT_X CLT_Y     COEFFICIENT\n");
    chptr += sprintf(chptr, "---------------------------------\n");
    PRMSG(fp, np->msg);
    num_pt = 0;
    sum_slope = 0.0;
    for (y_idx = p_miny; y_idx<=p_maxy; y_idx++) {
        posn_y = (int) floor((offset_y-0.5) + (y_idx + 0.5)*clutter_sim_res_ratio);
        for (x_idx = p_minx; x_idx<=p_maxx; x_idx++) {
            posn_x = (int) floor((offset_x-0.5) + (x_idx + 0.5)*clutter_sim_res_ratio);
            if ((!p) || (p->in_bdy_area(posn_x, posn_y))) {
                num_pt++;
                clutter_idx = x_idx + y_idx*npts_x;
                slope = mvec_x[useheight + clutter_idx];
                sum_slope += slope;
                chptr = np->msg;
                chptr += sprintf(chptr, "%5d %5d %5d %15.10f\n", num_pt, x_idx, y_idx, slope);
                PRMSG(fp, np->msg);
            }
        }
    }
    chptr = np->msg;
    chptr += sprintf(chptr, "---------------------------------\n");
    chptr += sprintf(chptr, "AVERAGE SLOPE:    %15.10f\n", sum_slope / num_pt);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    PRMSG(fp, np->msg);
}
/******************************************************************************************/


/*******************************************************************************************
* FUNCTION: ClutterGlobalPropModelClass::report_clutter()                     
*
*           Need to report Global PM parameters
*******************************************************************************************/
void ClutterGlobalPropModelClass::report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p)
{
    int p_minx, p_maxx, p_miny, p_maxy, x_idx, y_idx, num_pt, posn_x, posn_y, clutter_idx;
    int idx;  // loop variant
    double slope, sum_slope;
    char *chptr;

    int num_inflexion = (globPm ? globPm->num_inflexion : 0);

    if (p) {
        p->comp_bdy_min_max(p_minx, p_maxx, p_miny, p_maxy);

        p_minx = DIV(p_minx - offset_x, clutter_sim_res_ratio);
        p_maxx = DIV(p_maxx - offset_x, clutter_sim_res_ratio) + 1;
        p_miny = DIV(p_miny - offset_y, clutter_sim_res_ratio);
        p_maxy = DIV(p_maxy - offset_y, clutter_sim_res_ratio) + 1;

        if (p_minx < 0) { p_minx = 0; } else if (p_minx > npts_x-1) { p_minx = npts_x-1; }
        if (p_maxx < 0) { p_maxx = 0; } else if (p_maxx > npts_x-1) { p_maxx = npts_x-1; }
        if (p_miny < 0) { p_miny = 0; } else if (p_miny > npts_y-1) { p_miny = npts_y-1; }
        if (p_maxy < 0) { p_maxy = 0; } else if (p_maxy > npts_y-1) { p_maxy = npts_y-1; }
    } else {
        p_minx = 0;
        p_maxx = npts_x-1;
        p_miny = 0;
        p_maxy = npts_y-1;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Clutter Propagation Model Report                                ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "NAME: %s\n", get_strid());
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    /*
     *  Segment Global model parameters
     *  to be test later!
     **************************************************************************/
    chptr += sprintf(chptr, "    S: %9.7f\n", globPm->start_slope);

    chptr += sprintf(chptr, "N: %d\n",            num_inflexion);
    for( idx=0; idx<=num_inflexion-1; idx++ ) {
        chptr += sprintf(chptr, "X[%d]: %9.7f\n", idx,globPm->x[idx]);
        chptr += sprintf(chptr, "Y[%d]: %9.7f\n", idx,globPm->y[idx]);
    }

    chptr += sprintf(chptr, "S: %9.7f\n",         globPm->start_slope);
    chptr += sprintf(chptr, "F: %9.7f\n",         globPm->final_slope);
    chptr += sprintf(chptr, "USE_HEIGHT: %d\n",   globPm->useheight);

    PRMSG(fp, np->msg);
    /**************************************************************************/

    chptr = np->msg;
    chptr += sprintf(chptr, "  IDX CLT_X CLT_Y     COEFFICIENT\n");
    chptr += sprintf(chptr, "---------------------------------\n");
    PRMSG(fp, np->msg);
    num_pt = 0;
    sum_slope = 0.0;
    for (y_idx = p_miny; y_idx<=p_maxy; y_idx++) {
        posn_y = (int) floor((offset_y-0.5) + (y_idx + 0.5)*clutter_sim_res_ratio);
        for (x_idx = p_minx; x_idx<=p_maxx; x_idx++) {
            posn_x = (int) floor((offset_x-0.5) + (x_idx + 0.5)*clutter_sim_res_ratio);
            if ((!p) || (p->in_bdy_area(posn_x, posn_y))) {
                num_pt++;
                clutter_idx = x_idx + y_idx*npts_x;
                slope = mvec_x[useheight + clutter_idx];
                sum_slope += slope;
                chptr = np->msg;
                chptr += sprintf(chptr, "%5d %5d %5d %15.10f\n", num_pt, x_idx, y_idx, slope);
                PRMSG(fp, np->msg);
            }
        }
    }
    chptr = np->msg;
    chptr += sprintf(chptr, "---------------------------------\n");
    chptr += sprintf(chptr, "AVERAGE SLOPE:    %15.10f\n", sum_slope / num_pt);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    PRMSG(fp, np->msg);
}
/******************************************************************************************/


void ClutterGlobalPropModelClass::get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr)
{
    int i, j;
    int num_inflexion = (globPm ? globPm->num_inflexion : 0);
    MapClutterClass* map_clutter = NULL;

    // allocate global segment model and PM's coeffecients
    if (param_idx == 8) {
        globPm = new SegmentPropModelClass(map_clutter);
    }
    if (param_idx == 9) {
        globPm->x = (double *) realloc((void *) globPm->x, num_inflexion*sizeof(double));
        globPm->y = (double *) realloc((void *) globPm->y, num_inflexion*sizeof(double));
    }
    // allocate clutter coeffecient vector
    else if (param_idx == 6) {
        num_clutter_type = npts_x*npts_y;
        mvec_x = DVECTOR(num_clutter_type + useheight + 1);
    }

    // bound address of parameter to iptr or dptr,
    // so PM parameters can be obtained in other place by iptr or dptr
    if (param_idx == 0 ) {
        sprintf(str, "OFFSET_X:");              type=CConst::NumericInt; iptr = &offset_x;
    } else if (param_idx == 1 ) {
        sprintf(str, "OFFSET_Y:");              type=CConst::NumericInt; iptr = &offset_y;
    } else if (param_idx == 2 ) {
        sprintf(str, "NPTS_X:");                type=CConst::NumericInt; iptr = &npts_x;
    } else if (param_idx == 3 ) {
        sprintf(str, "NPTS_Y:");                type=CConst::NumericInt; iptr = &npts_y;
    } else if (param_idx == 4 ) {
        sprintf(str, "CLUTTER_SIM_RES_RATIO:"); type=CConst::NumericInt; iptr = &clutter_sim_res_ratio;
    } else if (param_idx == 5 ) {
        sprintf(str, "USE_HEIGHT:");            type=CConst::NumericInt; iptr = &useheight;
    } else if (param_idx == 6 ) {
        // constant value
        sprintf(str, "P0:");                    type=CConst::NumericDouble; dptr = &(mvec_x[num_clutter_type + useheight]);
    } else if (param_idx == 7 ) {
        // min distance of model
        sprintf(str, "R0:");                    type=CConst::NumericDouble; dptr = &r0;
    } else if (param_idx == 8 ) {
        // inflexion number of global model
        sprintf(str, "N:");                     type=CConst::NumericInt;    iptr = &(globPm->num_inflexion);
    } else if (param_idx < 2*num_inflexion + 9) {
        // inflexion coordination of global model
        i = param_idx-9;
        j = i>>1;
        if ((i&1) == 0) {
            sprintf(str, "X[%d]:", j);          type=CConst::NumericDouble; dptr = &(globPm->x[j]);
        } else {
            sprintf(str, "Y[%d]:", j);          type=CConst::NumericDouble; dptr = &(globPm->y[j]);
        }
    } else if (param_idx == 2*num_inflexion+9) {
        // start slope of global model
        sprintf(str, "S:");                     type=CConst::NumericDouble; dptr = &(globPm->start_slope);
    } else if (param_idx == 2*num_inflexion+10) {
        // final segment slope of global model
        sprintf(str, "F:");                     type=CConst::NumericDouble; dptr = &(globPm->final_slope);
    } else {
        i = param_idx-(2*num_inflexion+11);
        sprintf(str, "C_%d:", i);               type=CConst::NumericDouble; dptr = &(mvec_x[i]);
    }
}

int ClutterGlobalPropModelClass::comp_num_prop_param()
{
    int num_param;
    int num_inflexion = (globPm ? globPm->num_inflexion : 0);

    // include parameters of segment PM
    num_param = useheight + num_clutter_type + 2*num_inflexion + 11;

    // CG DBG
    //std::cout << " ClutterGlobalPropModelClass::comp_num_prop_param() "
    //          << num_param << std::endl;

    return( num_param );
}

void ClutterGlobalPropModelClass::print_params(FILE *fp, char *msg, int /* fmt */)
{ 
    int idx = 0;
    char *chptr = NULL;
    int num_inflexion = globPm->num_inflexion;

    chptr = msg;
    chptr += sprintf(chptr, "    TYPE: CLUTTERGLOBAL\n");
    chptr += sprintf(chptr, "    OFFSET_X: %d\n", offset_x);
    chptr += sprintf(chptr, "    OFFSET_Y: %d\n", offset_y);
    chptr += sprintf(chptr, "    NPTS_X: %d\n", npts_x);
    chptr += sprintf(chptr, "    NPTS_Y: %d\n", npts_y);
    chptr += sprintf(chptr, "    CLUTTER_SIM_RES_RATIO: %d\n", clutter_sim_res_ratio);
    chptr += sprintf(chptr, "    USE_HEIGHT: %d\n",       useheight);
    chptr += sprintf(chptr, "    P0: %15.10f\n", mvec_x[num_clutter_type + useheight]);
    chptr += sprintf(chptr, "    R0: %15.10f\n", r0);
    chptr += sprintf(chptr, "    N: %d\n", num_inflexion);
    // print inflexion point
    for( idx=0; idx<=num_inflexion-1; idx++ ) {
        chptr += sprintf(chptr, "    X[%d]: %9.7f\n", idx, globPm->x[idx]);
        chptr += sprintf(chptr, "    Y[%d]: %9.7f\n", idx, globPm->y[idx]);
    }
    // print start and final segment slope
    chptr += sprintf(chptr, "    S: %9.7f\n", globPm->start_slope);
    chptr += sprintf(chptr, "    F: %9.7f\n", globPm->final_slope);

    // print clutter coefficients
    PRMSG(fp, msg);
    if ( useheight != 0 ) {
        sprintf(msg, "    C_0: %9.7f  #COEFFICIENT OF ANTENNA HEIGHT\n", mvec_x[0]);
        PRMSG(fp, msg);
    }
    for ( idx=0; idx<num_clutter_type; idx++ ) {
        sprintf(msg, "    C_%d: %9.7f  #COEFFICIENT OF CLUTTER %d\n", idx+useheight, mvec_x[useheight+idx], idx );
        PRMSG(fp, msg);
    }
}


void ClutterGlobalPropModelClass::split_clutter(NetworkClass *)
{
    int i, j, old_i, old_j;
    int c_idx, old_c_idx;
    double *old_mvec_x = mvec_x;
    int old_num_clutter_type = num_clutter_type / 4;

    mvec_x = DVECTOR(num_clutter_type + useheight + 1);

    if (useheight) {
        mvec_x[0] = old_mvec_x[0];
    }

    for (c_idx=0; c_idx<=num_clutter_type-1; c_idx++) {
        i = c_idx % npts_x;
        j = c_idx / npts_x;

        old_i = i/2;
        old_j = j/2;

        old_c_idx = old_j*(npts_x/2) + old_i;

        mvec_x[useheight + c_idx] = old_mvec_x[useheight + old_c_idx];

        /*
        printf("oldi %d oldj %d i %d j %d mvec_x[useheight + c_idx] %6.4f \n",
                old_i, old_j, i, j, mvec_x[useheight + c_idx]);
        */

    }
    mvec_x[num_clutter_type + useheight] = old_mvec_x[old_num_clutter_type + useheight];

    free(old_mvec_x);
}
/******************************************************************************************/

/******************************************************************************************/
/*  FUNCTION: get_clt_idx()
 *            This function calculates the index of the clutter that the given CS lay on.
 *  INPUT:    NetworkClass, CS class
 *  RETURN:   index of clutter
 **********************************************************************/
int get_clt_idx( NetworkClass* np, CellClass *cell )
{
    MapClutterClass* map_clutter = np->map_clutter;
    int  clt_res = map_clutter->map_sim_res_ratio;
    int  cs_rx = cell->posn_x;
    int  cs_ry = cell->posn_y;

    int adjust_sx (0);         // Offset of start point of clutter map.
    int adjust_sy (0);         // Offset of start point of clutter map.
    
    adjust_sx = MOD(np->system_startx, clt_res);
    adjust_sy = MOD(np->system_starty, clt_res);

#if 0
    //??
    int adjust_nx = adjust_sx + clt_res-1 - MOD(np->npts_x+adjust_sx-1, clt_res);
    int adjust_ny = adjust_sy + clt_res-1 - MOD(np->npts_y+adjust_sy-1, clt_res);
    np->adjust_coord_system(np->system_startx-adjust_sx, np->system_starty-adjust_sy,
            np->npts_x+adjust_nx,        np->npts_y+adjust_ny);
    int num_clutter = (np->npts_x / clt_res) * (np->npts_y / clt_res);

    std::cout << std::endl;
    std::cout << " np->system_startx    " << np->system_startx << std::endl;
    std::cout << " np->system_starty    " << np->system_starty << std::endl;
    std::cout << " adjust_sx            " << adjust_sx         << std::endl;
    std::cout << " adjust_sy            " << adjust_sy         << std::endl;
    std::cout << " adjust_nx            " << adjust_nx         << std::endl;
    std::cout << " adjust_ny            " << adjust_ny         << std::endl;
    std::cout << " np->npts_x / clt_res " << np->npts_x / clt_res << std::endl;
    std::cout << " np->npts_y / clt_res " << np->npts_x / clt_res << std::endl;
#endif

    int num_clt_col = map_clutter->get_clutter_type(0, 0) -
        map_clutter->get_clutter_type(0, 1);
    int num_clt_row = map_clutter->num_clutter_type/num_clt_col;

    int cs_col_idx = (cs_rx+adjust_sx)/clt_res ;
    int cs_row_idx = num_clt_row - (cs_ry+adjust_sy)/clt_res - 1;

    int clt_idx = num_clt_col * cs_row_idx + cs_col_idx;
#if 0
    std::cout << "cs_rx       " << cs_rx << std::endl;
    std::cout << "cs_ry       " << cs_ry << std::endl;
    std::cout << "num_clt_col " << num_clt_col << std::endl;
    std::cout << "num_clt_row " << num_clt_row << std::endl;
    std::cout << "cs_col_idx  " << cs_col_idx  << std::endl;
    std::cout << "cs_row_idx  " << cs_row_idx  << std::endl;
    std::cout << "clt_idx     " << clt_idx     << std::endl;
    std::cout << std::endl;
#endif

    return ( clt_idx );
}


#if 0
// xxxxxxxxxx DELETE
/*
 *   FUNCTION: check_clt_bdy()   
 *             Check whether given CS is laying on the boundary of a clutter. 
 *             In order to solve the ploblem of the 'boundary-problem' producted in function prop_power_loss(), 
 *             which may lead to irregular results in power loss computing.
 *   RETURN:   true   If given CS lays on the bdy of a clutter.
 *             false  If given CS do not lay on the bdy of a clutter.             */
bool check_clt_bdy( NetworkClass* np, CellClass *cell )
{
    MapClutterClass* map_clutter = np->map_clutter;

    int  clt_res = map_clutter->map_sim_res_ratio;
    int  cs_rx = cell->posn_x; 
    int  cs_ry = cell->posn_y; 

    int  adjust_sx = MOD(np->system_startx, clt_res);      // Offset of start point of clutter map.
    int  adjust_sy = MOD(np->system_starty, clt_res);      // Offset of start point of clutter map.
    
    bool on_xbdy = ( (MOD(cs_rx+adjust_sx, clt_res) == 0) ? true : false );
    bool on_ybdy = ( (MOD(cs_ry+adjust_sy, clt_res) == 0) ? true : false );
    bool on_virtual_xbdy = ( (MOD(cs_rx+adjust_sx, clt_res) == clt_res-1) ? true : false );
    bool on_virtual_ybdy = ( (MOD(cs_ry+adjust_sy, clt_res) == clt_res-1) ? true : false );

    if ( on_xbdy && on_ybdy ) {
        //std::cout << "ON CLT BDY " ;
        cell->posn_x += 2 * ((int)(np->resolution));
        cell->posn_y += 2 * ((int)(np->resolution));
        return true;
    }
    else if ( on_xbdy && !on_ybdy )
    {
        //std::cout << "ON CLT BDY " ;
        cell->posn_x += 2 * ((int)(np->resolution));
        return true;
    }
    else if ( !on_xbdy && on_ybdy )
    {
        //std::cout << "ON CLT BDY " ;
        cell->posn_y += 2 * ((int)(np->resolution));
        return true;
    }

    if ( on_virtual_xbdy && on_virtual_ybdy ) {
        //std::cout << "ON V CLT BDY " ;
        cell->posn_x -= ((int)(np->resolution));
        cell->posn_y -= ((int)(np->resolution));
        return true;
    }
    else if ( on_virtual_xbdy && !on_virtual_ybdy )
    {
        //std::cout << "ON V CLT BDY " ;
        cell->posn_x -= ((int)(np->resolution));
        return true;
    }
    else if ( !on_virtual_xbdy && on_virtual_ybdy )
    {
        //std::cout << "ON V CLT BDY " ;
        cell->posn_y -= ((int)(np->resolution));
        return true;
    }

    return false;
    // return ( ( on_xbdy || on_ybdy ) ? true : false );  
}
#endif


/*
 *   FUNCTION: ClutterPropModelFullClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 *   XXXXXX    need to add estimating constant function later           */
void ClutterPropModelFullClass::clt_regulation( NetworkClass* np )
{
    int  map_i   = 0;
    int  map_j   = 0;
    int  clt_idx = 0;
    int  div_num = 0;          // Decide how many none zero clutter variables nearby the corresponding clutter.
                               // We divide sum of nearby clutter variables by div_num to get the evaluation value of 
                               // clutter variables to be regulated. 

    bool coe_nozero = true;    // Means all clutter coeficients are none-zero. 
                               // if the value of coe_nozero is true, we needn't regulate the coeficient values.
                               // In some cases we need regulate the positive value of coeficients, in order 
                               // to make the coeficient more reasonable. 
                               // It lead to considerable deviation of RMS.

    bool coe_allzero = true;   // Means all clutter coeficients are zero. 
                               // If the value of coe_allzero is true, we cannot regulate the coeficients of value, 
                               // so we donnt run the below loop.

    struct scanStruct {
        double coef;
        bool   coe_scanned;    // Scanned if the corresponding value of coefficient is less than zero, the value is true and vise versa. 
    };


#if 0
    // MODI for testing
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ )
    {
        mvec_x[2*clt_idx + useheight] = 0.0;
        mvec_x[2*clt_idx + useheight + 1] = 0.0;
    }
    mvec_x[10] = -30;
    mvec_x[11] = 60;
    mvec_x[20] = -40;
    mvec_x[21] = 40;
#endif


    /* Using original simulate output to fit a line.
     * We can use the relationship express by the line to estimate the values of constants
     * which cannot be resolved.
    ************************************************************************************************/
    double k = 0.0;
    double b = 0.0;
    bool flag = clt_fit_line( k, b );
    //std:: cout << "flag = " << flag << std::endl;

    scanStruct* scan_vec = (scanStruct*) malloc ( num_clutter_type * sizeof ( scanStruct ) ); 
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        scan_vec[clt_idx].coef = 0.0;
        scan_vec[clt_idx].coe_scanned = true;  // Default values of all coefficient are true, means that the value of coefficients are less than zero.
    }

    MapClutterClass* map_clutter = np->map_clutter;

    int num_clt_col = map_clutter->get_clutter_type(0, 0) - 
        map_clutter->get_clutter_type(0, 1);
    int num_clt_row = num_clutter_type/num_clt_col; 



    /* Scanning all the clutters to make sure there is more than one clutter with none zero coeficient or constant.
    ************************************************************************************************/
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) 
    {
        /* Judging whether or not there are more than one none zero coeficients.
        ********************************************************************************************/
        if ( doubleEqual( mvec_x[2*clt_idx + useheight], 0.0 ) ) {
            scan_vec[clt_idx].coe_scanned = false;
            coe_nozero = false;
        } 
        /* The following regulation of the coefecients will make the clutters' coefecients more reasonable,
         * but it also lead to a considerable deviation of Root-Mean-Square,
        ********************************************************************************************/
        // 2005.3.31 MODI CG
#if 0
        else if ( mvec_x[2*clt_idx + useheight] > 0.0 ) {
#if CLUTTER_DEBUG
            mvec_x[2*clt_idx + useheight] = 0.0;
            coe_nozero = false;
#else
            coe_allzero = false;
#endif
        }
#endif
        /* Judging whether or not all of the coeficients are zero.
        ********************************************************************************************/
        else { 
            // The following regulation of the coefecients will make the clutters' coefecients more reasonable,
            // but it also lead to a considerable deviation of Root-Mean-Square,
            // so we should get to a balance point.

            // 2005.3.31 MODI CG
            if ( mvec_x[2*clt_idx + useheight] > 100 ) {
                mvec_x[2*clt_idx + useheight] = 100;
                if ( flag )
                    mvec_x[2*clt_idx + useheight + 1] = 100 * k + b; 
            }
            else if ( mvec_x[2*clt_idx + useheight] < -300 )
            {
                mvec_x[2*clt_idx + useheight] = -300 ;
                if ( flag )
                    mvec_x[2*clt_idx + useheight + 1] = -300 * k + b; 
            }

            scan_vec[clt_idx].coe_scanned = false;
            coe_allzero = false;
        }
        scan_vec[clt_idx].coef = mvec_x[2*clt_idx + useheight];

    }


    /* Loop for clutter variables regulating.
     ***********************************************************************************************/
    int loop_i = 0;
    while ( !coe_nozero && !coe_allzero )
    {
        coe_nozero  = true;
        coe_allzero = true;
        clt_idx = 0;

        //std::cout << "=====================================================" << std::endl;
        for ( map_j=num_clt_row-1; map_j>=0; map_j-- ) {
            for ( map_i=0; map_i<num_clt_col; map_i++ ) 
            {
                //std::cout << "clt_idx " << clt_idx << "  scan_vec[clt_idx].coef  " << scan_vec[clt_idx].coef 
                //          << "  "  << scan_vec[clt_idx].coe_scanned << std::endl;
                //std::cout << "clt_idx " << clt_idx << "  scan_vec[clt_idx].cons  " << scan_vec[clt_idx].cons 
                //          << "  "  << scan_vec[clt_idx].con_scanned << std::endl;
                
                /* Loop for clutter coefficents regulating.
                 ***********************************************************************************/
                if ( doubleEqual( scan_vec[clt_idx].coef, 0.0 ) ) {
                    coe_nozero = false;

                    if ( map_i == 0 ) {
                        if ( map_j == 0 ) { 
                            div_num = 0; 
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        } else if ( map_j == num_clt_row-1 ) {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        } else {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    } else if ( map_i == num_clt_col-1 ) {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 )
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    } else {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    }
                } 
                else  coe_allzero = false;

                clt_idx ++;
            }
        }

        for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
            if ( !doubleEqual( scan_vec[clt_idx].coef, 0.0 ) )
                scan_vec[clt_idx].coe_scanned = true;
            else 
                scan_vec[clt_idx].coe_scanned = false;
        }

        loop_i ++;

        int num = ((num_clt_row>num_clt_col) ? num_clt_row : num_clt_col ); 
        if ( loop_i > 2*num - 2 )
            break;
    }
    

    /* Save the regulated coefficients. 
    ************************************************************************************************/
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        mvec_x[2*clt_idx + useheight] = scan_vec[clt_idx].coef;

        // DBG
        //std::cout << clt_idx << "  " << mvec_x[2*clt_idx + useheight] 
        //                     << "  " << mvec_x[2*clt_idx + useheight + 1] 
        //                     << std::endl;
    }


    /* Loop for clutter constants regulating with express 'cons = k * coef + b'.
    ************************************************************************************************/
    if ( flag ) 
        for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
            if ( doubleEqual( mvec_x[2*clt_idx + useheight + 1], 0.0 ) 
                    && mvec_x[2*clt_idx + useheight + 1] != k * mvec_x[2*clt_idx + useheight] )
                mvec_x[2*clt_idx + useheight + 1] = k * mvec_x[2*clt_idx + useheight] + b; 

            // DBG
            //std::cout << clt_idx << "  " << mvec_x[2*clt_idx + useheight]
            //                     << "  " << mvec_x[2*clt_idx + useheight + 1] 
            //                     << std::endl;
        }
    else 
        std::cout << "\nFAIL TO GET A FITNESS LINE BY CLUTTER COEFFS AND CONSTS\n";


    /*
     * This loop regulate abnormal coefficient and constant value of a clutter that CS lay on.
     * A abnormal parameters' clutter with CS lay on will lead to very abnormal result of recieve
     *    power simulation, i.e. coverage analysis. 
     * So these clutters are our special attention point, and we should regulate their parameters.
     * For most of such kind of clutter distribute in the BDY of simulation system and with much less 
     *    RTP pass them, so the regulation of parameters may not lead to considerable deviation of RMS.
     ***********************************************************************************************/
    int cell_idx = 0;
    int num_cell = np->num_cell;
    CellClass* cell = (CellClass*) NULL;
    for ( cell_idx=0; cell_idx<num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        clt_idx = get_clt_idx( np, cell );

        // DBG
        if ( mvec_x[2*clt_idx + useheight] > 0.0 ) {
            mvec_x[2*clt_idx + useheight] = 0.0;
            mvec_x[2*clt_idx + useheight + 1] = b - 130;
        }
    }

    free ( scan_vec ); 
}

/*
 *   FUNCTION: ClutterSymFullPropModelClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 *   XXXXXX    need to add estimating constant function later           */
void ClutterSymFullPropModelClass::clt_regulation( NetworkClass* np )
{
    int  map_i   = 0;
    int  map_j   = 0;
    int  clt_idx = 0;
    int  div_num = 0;          // Decide how many none zero clutter variables nearby the corresponding clutter.
                               // We divide sum of nearby clutter variables by div_num to get the evaluation value of 
                               // clutter variables to be regulated. 

    bool coe_nozero = true;    // Means all clutter coeficients are none-zero. 
                               // if the value of coe_nozero is true, we needn't regulate the coeficient values.
                               // In some cases we need regulate the positive value of coeficients, in order 
                               // to make the coeficient more reasonable. 
                               // It lead to considerable deviation of RMS.

    bool coe_allzero = true;   // Means all clutter coeficients are zero. 
                               // If the value of coe_allzero is true, we cannot regulate the coeficients of value, 
                               // so we donnt run the below loop.

    struct scanStruct {
        double coef;
        bool   coe_scanned;    // Scanned if the corresponding value of coefficient is less than zero, the value is true and vise versa. 
    };


#if 0
    // MODI for testing
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ )
    {
        mvec_x[2*clt_idx + useheight] = 0.0;
        mvec_x[2*clt_idx + useheight + 1] = 0.0;
    }
    mvec_x[10] = -30;
    mvec_x[11] = 60;
    mvec_x[20] = -40;
    mvec_x[21] = 40;
#endif


    /* Using original simulate output to fit a line.
     * We can use the relationship express by the line to estimate the values of constants
     * which cannot be resolved.
    ************************************************************************************************/
    double k = 0.0;
    double b = 0.0;
    bool flag = clt_fit_line( k, b );
    //std:: cout << "flag = " << flag << std::endl;

    scanStruct* scan_vec = (scanStruct*) malloc ( num_clutter_type * sizeof ( scanStruct ) ); 
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        scan_vec[clt_idx].coef = 0.0;
        scan_vec[clt_idx].coe_scanned = true;  // Default values of all coefficient are true, means that the value of coefficients are less than zero.
    }

    MapClutterClass* map_clutter = np->map_clutter;

    int num_clt_col = map_clutter->get_clutter_type(0, 0) - 
        map_clutter->get_clutter_type(0, 1);
    int num_clt_row = num_clutter_type/num_clt_col; 



    /* Scanning all the clutters to make sure there is more than one clutter with none zero coeficient or constant.
    ************************************************************************************************/
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) 
    {
        /* Judging whether or not there are more than one none zero coeficients.
        ********************************************************************************************/
        if ( doubleEqual( mvec_x[2*clt_idx + useheight], 0.0 ) ) {
            scan_vec[clt_idx].coe_scanned = false;
            coe_nozero = false;
        } 
        /* The following regulation of the coefecients will make the clutters' coefecients more reasonable,
         * but it also lead to a considerable deviation of Root-Mean-Square,
        ********************************************************************************************/
        // 2005.3.31 MODI CG
#if 0
        else if ( mvec_x[2*clt_idx + useheight] > 0.0 ) {
#if CLUTTER_DEBUG
            mvec_x[2*clt_idx + useheight] = 0.0;
            coe_nozero = false;
#else
            coe_allzero = false;
#endif
        }
#endif
        /* Judging whether or not all of the coeficients are zero.
        ********************************************************************************************/
        else { 
            // The following regulation of the coefecients will make the clutters' coefecients more reasonable,
            // but it also lead to a considerable deviation of Root-Mean-Square,
            // so we should get to a balance point.

            // 2005.3.31 MODI CG
            if ( mvec_x[2*clt_idx + useheight] > 100 ) {
                mvec_x[2*clt_idx + useheight] = 100;
                if ( flag )
                    mvec_x[2*clt_idx + useheight + 1] = 100 * k + b; 
            }
            else if ( mvec_x[2*clt_idx + useheight] < -300 )
            {
                mvec_x[2*clt_idx + useheight] = -300 ;
                if ( flag )
                    mvec_x[2*clt_idx + useheight + 1] = -300 * k + b; 
            }

            scan_vec[clt_idx].coe_scanned = false;
            coe_allzero = false;
        }
        scan_vec[clt_idx].coef = mvec_x[2*clt_idx + useheight];

    }


    /* Loop for clutter variables regulating.
     ***********************************************************************************************/
    int loop_i = 0;
    while ( !coe_nozero && !coe_allzero )
    {
        coe_nozero  = true;
        coe_allzero = true;
        clt_idx = 0;

        //std::cout << "=====================================================" << std::endl;
        for ( map_j=num_clt_row-1; map_j>=0; map_j-- ) {
            for ( map_i=0; map_i<num_clt_col; map_i++ ) 
            {
                //std::cout << "clt_idx " << clt_idx << "  scan_vec[clt_idx].coef  " << scan_vec[clt_idx].coef 
                //          << "  "  << scan_vec[clt_idx].coe_scanned << std::endl;
                //std::cout << "clt_idx " << clt_idx << "  scan_vec[clt_idx].cons  " << scan_vec[clt_idx].cons 
                //          << "  "  << scan_vec[clt_idx].con_scanned << std::endl;
                
                /* Loop for clutter coefficents regulating.
                 ***********************************************************************************/
                if ( doubleEqual( scan_vec[clt_idx].coef, 0.0 ) ) {
                    coe_nozero = false;

                    if ( map_i == 0 ) {
                        if ( map_j == 0 ) { 
                            div_num = 0; 
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        } else if ( map_j == num_clt_row-1 ) {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        } else {
                            div_num = 0;
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    } else if ( map_i == num_clt_col-1 ) {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 )
                        {
                            div_num = 0; 
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    } else {
                        if ( map_j == 0 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else if ( map_j == num_clt_row-1 ) 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                        else 
                        {
                            div_num = 0;
                            if ( scan_vec[clt_idx-1].coef < 0.0 && scan_vec[clt_idx-1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-1].coef;
                            }
                            if ( scan_vec[clt_idx+1].coef < 0.0 && scan_vec[clt_idx+1].coe_scanned == true ) { 
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+1].coef;
                            }
                            if ( scan_vec[clt_idx-num_clt_col].coef < 0.0 && scan_vec[clt_idx-num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx-num_clt_col].coef ;
                            }
                            if ( scan_vec[clt_idx+num_clt_col].coef < 0.0 && scan_vec[clt_idx+num_clt_col].coe_scanned == true ) {
                                div_num ++;
                                scan_vec[clt_idx].coef += scan_vec[clt_idx+num_clt_col].coef ;
                            }
                            if ( div_num == 0 ) div_num = 1;
                            scan_vec[clt_idx].coef /= div_num;
                        }
                    }
                } 
                else  coe_allzero = false;

                clt_idx ++;
            }
        }

        for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
            if ( !doubleEqual( scan_vec[clt_idx].coef, 0.0 ) )
                scan_vec[clt_idx].coe_scanned = true;
            else 
                scan_vec[clt_idx].coe_scanned = false;
        }

        loop_i ++;

        int num = ((num_clt_row>num_clt_col) ? num_clt_row : num_clt_col ); 
        if ( loop_i > 2*num - 2 )
            break;
    }
    

    /* Save the regulated coefficients. 
    ************************************************************************************************/
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        mvec_x[2*clt_idx + useheight] = scan_vec[clt_idx].coef;

        // DBG
        //std::cout << clt_idx << "  " << mvec_x[2*clt_idx + useheight] 
        //                     << "  " << mvec_x[2*clt_idx + useheight + 1] 
        //                     << std::endl;
    }


    /* Loop for clutter constants regulating with express 'cons = k * coef + b'.
    ************************************************************************************************/
    if ( flag ) 
        for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
            if ( doubleEqual( mvec_x[2*clt_idx + useheight + 1], 0.0 ) 
                    && mvec_x[2*clt_idx + useheight + 1] != k * mvec_x[2*clt_idx + useheight] )
                mvec_x[2*clt_idx + useheight + 1] = k * mvec_x[2*clt_idx + useheight] + b; 

            // DBG
            //std::cout << clt_idx << "  " << mvec_x[2*clt_idx + useheight]
            //                     << "  " << mvec_x[2*clt_idx + useheight + 1] 
            //                     << std::endl;
        }
    else 
        std::cout << "\nFAIL TO GET A FITNESS LINE BY CLUTTER COEFFS AND CONSTS\n";


    /*
     * This loop regulate abnormal coefficient and constant value of a clutter that CS lay on.
     * A abnormal parameters' clutter with CS lay on will lead to very abnormal result of recieve
     *    power simulation, i.e. coverage analysis. 
     * So these clutters are our special attention point, and we should regulate their parameters.
     * For most of such kind of clutter distribute in the BDY of simulation system and with much less 
     *    RTP pass them, so the regulation of parameters may not lead to considerable deviation of RMS.
     ***********************************************************************************************/
    int cell_idx = 0;
    int num_cell = np->num_cell;
    CellClass* cell = (CellClass*) NULL;
    for ( cell_idx=0; cell_idx<num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        clt_idx = get_clt_idx( np, cell );

        // DBG
        if ( mvec_x[2*clt_idx + useheight] > 0.0 ) {
            mvec_x[2*clt_idx + useheight] = 0.0;
            mvec_x[2*clt_idx + useheight + 1] = b - 130;
        }
    }

    free ( scan_vec ); 
}

/*
 *   FUNCTION: ClutterWtExpoPropModelClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 *   XXXXXX    need to add estimating constant function later           */
void ClutterWtExpoPropModelClass::clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag )
{
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int clutter_idx, var_idx, eqn_idx;
    double **mmx_a, *vec_b, error, prev_error, c_coeff, c_const;
    double e_dot_d, d_dot_d, c_dot_d, e_val;
    double e_dot_c, c_dot_c;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;

    PolygonClass *map_bdy = np->map_clutter->create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = 2*num_clutter_type+useheight;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    error = compute_error(num_insys_rtp, num_var, mmx_a, vec_b);

    sprintf(np->msg, "ERROR = %12.10f dB\n", error);
    PRMSG(stdout, np->msg);

    double min_threshold = -100.0;
    double max_threshold =  0.0;
    int iteration = 0;

    do {
        iteration++;
        prev_error = error;
        for ( clutter_idx=0; clutter_idx<=num_clutter_type-1; clutter_idx++ ) {
            c_coeff = mvec_x[2*clutter_idx + useheight];
            c_const = mvec_x[2*clutter_idx + useheight + 1];

            printf("CLUTTER IDX = %d,  COEFF = %12.10f, CONST = %12.10f\n", clutter_idx, c_coeff, c_const);
            e_dot_d = 0.0;
            c_dot_d = 0.0;
            d_dot_d = 0.0;
            e_dot_c = 0.0;
            c_dot_c = 0.0;

            for (eqn_idx=0; eqn_idx<=num_insys_rtp-1; eqn_idx++) {
                e_val = vec_b[eqn_idx];
                for (var_idx=0; var_idx<=num_var-1; var_idx++) {
                    if ( (var_idx < 2*clutter_idx + useheight) || (var_idx > 2*clutter_idx + useheight + 1) ) {
                        e_val -= mmx_a[eqn_idx][var_idx]*mvec_x[var_idx];
                    }
                }

                e_dot_d += e_val*mmx_a[eqn_idx][2*clutter_idx + useheight + 1];
                c_dot_d += mmx_a[eqn_idx][2*clutter_idx + useheight    ]*mmx_a[eqn_idx][2*clutter_idx + useheight + 1];
                d_dot_d += mmx_a[eqn_idx][2*clutter_idx + useheight + 1]*mmx_a[eqn_idx][2*clutter_idx + useheight + 1];

                e_dot_c += e_val*mmx_a[eqn_idx][2*clutter_idx + useheight];
                c_dot_c += mmx_a[eqn_idx][2*clutter_idx + useheight]*mmx_a[eqn_idx][2*clutter_idx + useheight];
            }

            if (c_dot_c != 0.0) {
                c_coeff = (e_dot_d*c_dot_d - e_dot_c*d_dot_d) / (c_dot_d*c_dot_d - c_dot_c*d_dot_d);
    
                mvec_x[2*clutter_idx + useheight] = ((c_coeff < min_threshold) ? min_threshold :
                                                     (c_coeff > max_threshold) ? max_threshold : c_coeff);
                mvec_x[2*clutter_idx + useheight + 1] = (e_dot_d - c_dot_d*mvec_x[2*clutter_idx + useheight]) / d_dot_d;
                c_coeff = mvec_x[2*clutter_idx + useheight];
                c_const = mvec_x[2*clutter_idx + useheight + 1];

                printf("---- COEFF SET TO = %12.10f, CONST SET TO = %12.10f\n", c_coeff, c_const);
                // error = compute_error(num_insys_rtp, num_var, mmx_a, vec_b);
                // printf("---- ERROR = %12.10f dB\n", error);
            } else {
                mvec_x[2*clutter_idx + useheight    ] = 0.0;
                mvec_x[2*clutter_idx + useheight + 1] = 0.0;
            }
        }
        error = compute_error(num_insys_rtp, num_var, mmx_a, vec_b);
        sprintf(np->msg, "---- INTERATION %d: ERROR = %12.10f dB\n", iteration, error);
        PRMSG(stdout, np->msg);
    } while ( iterate_flag && ((prev_error - error) > 0.01) );

    return;
}

/*
 *   FUNCTION: ClutterWtExpoSlopePropModelClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 *   XXXXXX    need to add estimating constant function later           */
void ClutterWtExpoSlopePropModelClass::clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag )
{
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int var_idx, eqn_idx, t_var_idx;
    int col_idx, idx, incr, t_idx, cont, new_var;
    double *vec_b, error, prev_error, c_coeff;
    SparseMatrixClass *mmx_a;
    double e_val;
    double e_dot_c, c_dot_c;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    PolygonClass *map_bdy = create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    delete map_bdy;

// mmx_a->dump();
// fflush(stdout);
// CORE_DUMP;

    error = compute_error(mmx_a, vec_b);

    sprintf(np->msg, "ERROR = %12.10f dB\n", error);
    PRMSG(stdout, np->msg);

    double min_threshold = -100.0;
    double max_threshold =  0.0;
    int iteration = 0;

    do {
        iteration++;
        prev_error = error;

        new_var = 1;
        for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
            idx = (*mmx_a->col_list)[col_idx];
            mx_elem = (*mmx_a->a)[idx];

            if (new_var) {
                var_idx = mx_elem.getInt(1);

                c_coeff = mvec_x[var_idx];

#if 0
                if ( useheight && (var_idx == 0) ) {
                    printf("HEIGHT COEFF = %12.10f\n", c_coeff);
                } else if (var_idx < num_var-1) {
                    printf("CLUTTER IDX = %d,  COEFF = %12.10f\n", var_idx-useheight, c_coeff);
                } else {
                    printf("P0 = %12.10f\n", c_coeff);
                }
#endif
                e_dot_c = 0.0;
                c_dot_c = 0.0;
                new_var = 0;
            }

            eqn_idx = mx_elem.getInt(0);
            e_val = vec_b[eqn_idx];
            for (incr=-1; incr<=1; incr+=2) {
                t_idx = idx;
                cont = 1;
                while(cont) {
                    t_idx += incr;
                    if ( (t_idx < 0) || (t_idx > mmx_a->a->getSize()-1) ) {
                        cont = 0;
                    } else {
                        t_mx_elem = (*mmx_a->a)[t_idx];
                        if (t_mx_elem.getInt(0) != eqn_idx) {
                            cont = 0;
                        }
                    }
                    if (cont) {
                        t_var_idx = t_mx_elem.getInt(1);
                        e_val -= t_mx_elem.getDouble()*mvec_x[t_var_idx];
                    }
                }
            }

            e_dot_c += e_val*mx_elem.getDouble();
            c_dot_c += mx_elem.getDouble()*mx_elem.getDouble();

            if ( (col_idx==mmx_a->a->getSize()-1) || ((*mmx_a->a)[(*mmx_a->col_list)[col_idx+1]].getInt(1) != var_idx) ) {

                if (c_dot_c != 0.0) {
                    c_coeff = e_dot_c / c_dot_c;
                } else {
                    c_coeff = 0.0;
                }
    
                if ( (useheight && (var_idx == 0)) || (var_idx == num_var-1) ) {
                    mvec_x[var_idx] = c_coeff;
                } else {
                    mvec_x[var_idx] = ((c_coeff < min_threshold) ? min_threshold :
                                       (c_coeff > max_threshold) ? max_threshold : c_coeff);
                }
                c_coeff = mvec_x[var_idx];

                // printf("---- E.C = %12.10f C.C = %12.10f COEFF SET TO = %12.10f\n", e_dot_c, c_dot_c, c_coeff);

                new_var = 1;
            }
        }

        error = compute_error(mmx_a, vec_b);
        sprintf(np->msg, "---- ITERATION %d: ERROR = %12.10f dB\n", iteration, error);
        PRMSG(stdout, np->msg);
    } while ( iterate_flag && ((prev_error - error) > 0.01) );

    delete mmx_a;
    free(vec_b);

    return;
}

/******************************************************************************************/
/**** FUNCTION: ClutterWtExpoSlopePropModelClass::clt_fill                             ****/
/**** Full in unused clutter values.                                                   ****/
/******************************************************************************************/
void ClutterWtExpoSlopePropModelClass::clt_fill( NetworkClass* np, ListClass<int> *scan_index_list)
{
    int i, j, n;
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int clutter_idx, var_idx, prev_var_idx;
    int col_idx, idx;
    double *vec_b, sum;
    SparseMatrixClass *mmx_a;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    ListClass<int> *unused_clutter_list = new ListClass<int>(0);
    ListClass<int> *assigned_list = new ListClass<int>(0);
    PolygonClass *map_bdy = create_map_bdy();

#if OUTPUT_TIME_INFOR
    time_t td;
#endif

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill 1st loop begin Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill 1st loop begin Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill 1st loop end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill 1st loop end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill create_svd_matrices end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill create_svd_matrices end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif

    delete map_bdy;

    prev_var_idx = -1;
    for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
        idx = (*mmx_a->col_list)[col_idx];
        mx_elem = (*mmx_a->a)[idx];

        var_idx = mx_elem.getInt(1);
        if (var_idx != prev_var_idx) {
            if ( useheight && (var_idx == 0) ) {
                // Do nothing
            } else {
                for (i=prev_var_idx+1; i<=var_idx-1; i++) {
                    unused_clutter_list->append(i-useheight);
                }
            }
        }
        prev_var_idx = var_idx;
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill 2nd loop end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill 2nd loop end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif 
   
    for (i=prev_var_idx+1; i<=num_var-1; i++) {
        unused_clutter_list->append(i-useheight);
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill 3rd loop end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill 3rd loop end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif 

    delete mmx_a;
    free(vec_b);

#if 1
    while(unused_clutter_list->getSize()) {

        assigned_list->reset();
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            i = clutter_idx % npts_x;
            j = clutter_idx / npts_x;

            n = 0;
            sum = 0.0;
            if (i > 0) {
                if (!unused_clutter_list->contains(clutter_idx-1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-1];
                }
            }
            if (i < npts_x-1) {
                if (!unused_clutter_list->contains(clutter_idx+1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+1];
                }
            }
            if (j > 0) {
                if (!unused_clutter_list->contains(clutter_idx-npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-npts_x];
                }
            }
            if (j < npts_y-1) {
                if (!unused_clutter_list->contains(clutter_idx+npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+npts_x];
                }
            }

            if (n) {
                // printf("ASSIGN VALUE FOR CLUTTER TYPE %d\n", clutter_idx);
                mvec_x[useheight + clutter_idx] = sum / n;
                assigned_list->append(clutter_idx);
            }
        }

        for (idx=0; idx<=assigned_list->getSize()-1; idx++) {
            clutter_idx = (*assigned_list)[idx];
            unused_clutter_list->del_elem(clutter_idx);
        }
        // printf("=============================\n");
    }

#if OUTPUT_TIME_INFOR
        time(&td);
        printf("\n----clt_fill 4th loop end Time ---- %s", ctime(&td));

        sprintf(np->msg, "----clt_fill 4th loop end Time ---- %s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif 
    
#else
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            mvec_x[useheight + clutter_idx] = 1.0;
        }
#endif

    delete assigned_list;
    delete unused_clutter_list;

    return;
}

/*   Signed clutters with special color by coefficient levels.
 */ 
void ClutterWtExpoSlopePropModelClass::get_min_max_color(double &min_neg, double &max_pos)
{
    int clt_idx, c;
    double attn_res;

    min_neg = 0.0;
    max_pos = 0.0;

    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        attn_res =  mvec_x[clt_idx+useheight];
        if ( attn_res < min_neg ) {
            min_neg = attn_res;
        } else if ( attn_res > max_pos ) {
            max_pos = attn_res;
        }
    }

    if (min_neg == 0.0) {
        min_neg = -1.0;
    }

    if (max_pos == 0.0) {
        max_pos = 1.0;
    }
}

/*   Signed clutters with special color by coefficient levels.
 */ 
int ClutterWtExpoSlopePropModelClass::get_color(int clutter_type, double min_neg, double max_pos)
{
    int c;
    double attn_res;

    attn_res =  mvec_x[clutter_type+useheight];
    if (attn_res <= 0.0) {
        c = (int) floor( ( (attn_res - min_neg) / (- min_neg) ) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|c;
    } else {
        c = (int) floor( ( (max_pos - attn_res) / max_pos) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|255;
    }

    return(c);
}


/*
 *   FUNCTION: ClutterExpoLinearPropModelClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 *   XXXXXX    need to add estimating constant function later           */
void ClutterExpoLinearPropModelClass::clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag )
{
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int var_idx, eqn_idx, t_var_idx;
    int col_idx, idx, incr, t_idx, cont, new_var;
    double *vec_b, error, prev_error, c_coeff;
    SparseMatrixClass *mmx_a;
    double e_val;
    double e_dot_c, c_dot_c;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    PolygonClass *map_bdy = create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    delete map_bdy;

// mmx_a->dump();
// fflush(stdout);
// CORE_DUMP;

    error = compute_error(mmx_a, vec_b);

    sprintf(np->msg, "ERROR = %12.10f dB\n", error);
    PRMSG(stdout, np->msg);

    double min_threshold = -0.1;
    double max_threshold =  0.0;
    int iteration = 0;

    do {
        iteration++;
        prev_error = error;

        new_var = 1;
        for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
            idx = (*mmx_a->col_list)[col_idx];
            mx_elem = (*mmx_a->a)[idx];

            if (new_var) {
                var_idx = mx_elem.getInt(1);

                c_coeff = mvec_x[var_idx];

#if 0
                if ( useheight && (var_idx == 0) ) {
                    printf("HEIGHT COEFF = %12.10f\n", c_coeff);
                } else if (var_idx < num_var-1) {
                    printf("CLUTTER IDX = %d,  COEFF = %12.10f\n", var_idx-useheight, c_coeff);
                } else {
                    printf("P0 = %12.10f\n", c_coeff);
                }
#endif
                e_dot_c = 0.0;
                c_dot_c = 0.0;
                new_var = 0;
            }

            eqn_idx = mx_elem.getInt(0);
            e_val = vec_b[eqn_idx];
            for (incr=-1; incr<=1; incr+=2) {
                t_idx = idx;
                cont = 1;
                while(cont) {
                    t_idx += incr;
                    if ( (t_idx < 0) || (t_idx > mmx_a->a->getSize()-1) ) {
                        cont = 0;
                    } else {
                        t_mx_elem = (*mmx_a->a)[t_idx];
                        if (t_mx_elem.getInt(0) != eqn_idx) {
                            cont = 0;
                        }
                    }
                    if (cont) {
                        t_var_idx = t_mx_elem.getInt(1);
                        e_val -= t_mx_elem.getDouble()*mvec_x[t_var_idx];
                    }
                }
            }

            e_dot_c += e_val*mx_elem.getDouble();
            c_dot_c += mx_elem.getDouble()*mx_elem.getDouble();

            if ( (col_idx==mmx_a->a->getSize()-1) || ((*mmx_a->a)[(*mmx_a->col_list)[col_idx+1]].getInt(1) != var_idx) ) {

                if (c_dot_c != 0.0) {
                    c_coeff = e_dot_c / c_dot_c;
                } else {
                    c_coeff = 0.0;
                }
    
                if ( (useheight && (var_idx == 0)) || (var_idx == num_var-1) ) {
                    mvec_x[var_idx] = c_coeff;
                } else {
                    mvec_x[var_idx] = ((c_coeff < min_threshold) ? min_threshold :
                                       (c_coeff > max_threshold) ? max_threshold : c_coeff);
                }
                c_coeff = mvec_x[var_idx];

                // printf("---- E.C = %12.10f C.C = %12.10f COEFF SET TO = %12.10f\n", e_dot_c, c_dot_c, c_coeff);

                new_var = 1;
            }
        }

        error = compute_error(mmx_a, vec_b);
        sprintf(np->msg, "---- ITERATION %d: ERROR = %12.10f dB\n", iteration, error);
        PRMSG(stdout, np->msg);
    } while ( iterate_flag && ((prev_error - error) > 0.01) );

    delete mmx_a;
    free(vec_b);

    return;
}

/******************************************************************************************/
/**** FUNCTION: ClutterExpoLinearPropModelClass::clt_fill                             ****/
/**** Full in unused clutter values.                                                   ****/
/******************************************************************************************/
void ClutterExpoLinearPropModelClass::clt_fill( NetworkClass* np, ListClass<int> *scan_index_list)
{
    int i, j, n;
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int clutter_idx, var_idx, prev_var_idx;
    int col_idx, idx;
    double *vec_b, sum;
    SparseMatrixClass *mmx_a;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    ListClass<int> *unused_clutter_list = new ListClass<int>(0);
    ListClass<int> *assigned_list = new ListClass<int>(0);
    PolygonClass *map_bdy = create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    delete map_bdy;

    prev_var_idx = -1;
    for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
        idx = (*mmx_a->col_list)[col_idx];
        mx_elem = (*mmx_a->a)[idx];

        var_idx = mx_elem.getInt(1);
        if (var_idx != prev_var_idx) {
            if ( useheight && (var_idx == 0) ) {
                // Do nothing
            } else {
                for (i=prev_var_idx+1; i<=var_idx-1; i++) {
                    unused_clutter_list->append(i-useheight);
                }
            }
        }
        prev_var_idx = var_idx;
    }
    for (i=prev_var_idx+1; i<=num_var-1; i++) {
        unused_clutter_list->append(i-useheight);
    }

    delete mmx_a;
    free(vec_b);

#if 1
    while(unused_clutter_list->getSize()) {

        assigned_list->reset();
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            i = clutter_idx % npts_x;
            j = clutter_idx / npts_x;

            n = 0;
            sum = 0.0;
            if (i > 0) {
                if (!unused_clutter_list->contains(clutter_idx-1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-1];
                }
            }
            if (i < npts_x-1) {
                if (!unused_clutter_list->contains(clutter_idx+1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+1];
                }
            }
            if (j > 0) {
                if (!unused_clutter_list->contains(clutter_idx-npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-npts_x];
                }
            }
            if (j < npts_y-1) {
                if (!unused_clutter_list->contains(clutter_idx+npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+npts_x];
                }
            }

            if (n) {
                // printf("ASSIGN VALUE FOR CLUTTER TYPE %d\n", clutter_idx);
                mvec_x[useheight + clutter_idx] = sum / n;
                assigned_list->append(clutter_idx);
            }
        }

        for (idx=0; idx<=assigned_list->getSize()-1; idx++) {
            clutter_idx = (*assigned_list)[idx];
            unused_clutter_list->del_elem(clutter_idx);
        }
        // printf("=============================\n");
    }
#else
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            mvec_x[useheight + clutter_idx] = 1.0;
        }
#endif

    delete assigned_list;
    delete unused_clutter_list;

    return;
}

/*   Signed clutters with special color by coefficient levels.
 */ 
void ClutterExpoLinearPropModelClass::get_min_max_color(double &min_neg, double &max_pos)
{
    int clt_idx, c;
    double attn_res;

    min_neg = 0.0;
    max_pos = 0.0;

    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        attn_res =  mvec_x[clt_idx+useheight];
        if ( attn_res < min_neg ) {
            min_neg = attn_res;
        } else if ( attn_res > max_pos ) {
            max_pos = attn_res;
        }
    }

    if (min_neg == 0.0) {
        min_neg = -1.0;
    }

    if (max_pos == 0.0) {
        max_pos = 1.0;
    }
}

/*   Signed clutters with special color by coefficient levels.
 */ 
int ClutterExpoLinearPropModelClass::get_color(int clutter_type, double min_neg, double max_pos)
{
    int c;
    double attn_res;

    attn_res =  mvec_x[clutter_type+useheight];
    if (attn_res <= 0.0) {
        c = (int) floor( ( (attn_res - min_neg) / (- min_neg) ) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|c;
    } else {
        c = (int) floor( ( (max_pos - attn_res) / max_pos) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|255;
    }

    return(c);
}

/*
 *   FUNCTION: ClutterGlobalPropModelClass::clt_regulation()   
 *             Estimate the values of clutters' variable of coefcient and constant. 
 *   Note:     after calling this function the RMS of propagation model should be a significantly deviation.
 ***************************************************************************************************************/
void ClutterGlobalPropModelClass::clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag )
{
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int var_idx, eqn_idx, t_var_idx;
    int col_idx, idx, incr, t_idx, cont, new_var;
    double *vec_b, error, prev_error, c_coeff;
    SparseMatrixClass *mmx_a;
    double e_val;
    double e_dot_c, c_dot_c;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    PolygonClass *map_bdy = create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    delete map_bdy;

// mmx_a->dump();
// fflush(stdout);
// CORE_DUMP;

    error = compute_error(mmx_a, vec_b);

    sprintf(np->msg, "ERROR = %12.10f dB\n", error);
    PRMSG(stdout, np->msg);

    double min_threshold = -10.0;
    double max_threshold =  0.005;
    int iteration = 0;

    do {
        iteration++;
        prev_error = error;

        new_var = 1;
        for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
            idx = (*mmx_a->col_list)[col_idx];
            mx_elem = (*mmx_a->a)[idx];

            if (new_var) {
                var_idx = mx_elem.getInt(1);

                c_coeff = mvec_x[var_idx];

#if 0
                if ( useheight && (var_idx == 0) ) {
                    printf("HEIGHT COEFF = %12.10f\n", c_coeff);
                } else if (var_idx < num_var-1) {
                    printf("CLUTTER IDX = %d,  COEFF = %12.10f\n", var_idx-useheight, c_coeff);
                } else {
                    printf("P0 = %12.10f\n", c_coeff);
                }
#endif
                e_dot_c = 0.0;
                c_dot_c = 0.0;
                new_var = 0;
            }

            eqn_idx = mx_elem.getInt(0);
            e_val = vec_b[eqn_idx];
            for (incr=-1; incr<=1; incr+=2) {
                t_idx = idx;
                cont = 1;
                while(cont) {
                    t_idx += incr;
                    if ( (t_idx < 0) || (t_idx > mmx_a->a->getSize()-1) ) {
                        cont = 0;
                    } else {
                        t_mx_elem = (*mmx_a->a)[t_idx];
                        if (t_mx_elem.getInt(0) != eqn_idx) {
                            cont = 0;
                        }
                    }
                    if (cont) {
                        t_var_idx = t_mx_elem.getInt(1);
                        e_val -= t_mx_elem.getDouble()*mvec_x[t_var_idx];
                    }
                }
            }

            e_dot_c += e_val*mx_elem.getDouble();
            c_dot_c += mx_elem.getDouble()*mx_elem.getDouble();

            if ( (col_idx==mmx_a->a->getSize()-1) || ((*mmx_a->a)[(*mmx_a->col_list)[col_idx+1]].getInt(1) != var_idx) ) {

                if (c_dot_c != 0.0) {
                    c_coeff = e_dot_c / c_dot_c;
                } else {
                    c_coeff = 0.0;
                }
    
#if 1
                if ( (useheight && (var_idx == 0)) || (var_idx == num_var-1) ) {
                    mvec_x[var_idx] = c_coeff;
                } else {
                    
                    mvec_x[var_idx] = ((c_coeff < min_threshold) ? min_threshold :
                                       (c_coeff > max_threshold) ? max_threshold : c_coeff);
                     
                }
#else
                mvec_x[var_idx] = c_coeff;
#endif
                c_coeff = mvec_x[var_idx];

                // printf("---- E.C = %12.10f C.C = %12.10f COEFF SET TO = %12.10f\n", e_dot_c, c_dot_c, c_coeff);

                new_var = 1;
            }
        }

        error = compute_error(mmx_a, vec_b);
        sprintf(np->msg, "---- ITERATION %d: ERROR = %12.10f dB\n", iteration, error);
        PRMSG(stdout, np->msg);

        // CG DBG
        //printf("---- ITERATION %d: ERROR = %12.10f dB\n", iteration, error);

    } while ( iterate_flag && ((prev_error - error) > 0.01) );

    delete mmx_a;
    free(vec_b);

    return;
}

/******************************************************************************************/
/**** FUNCTION: ClutterGlobalPropModelClass::clt_fill                             ****/
/**** Full in unused clutter values.                                                   ****/
/******************************************************************************************/
void ClutterGlobalPropModelClass::clt_fill( NetworkClass* np, ListClass<int> *scan_index_list)
{
    int i, j, n;
    int cell_idx, sector_idx, scan_idx;
    int rtp_idx, num_insys_rtp, num_var;
    int clutter_idx, var_idx, prev_var_idx;
    int col_idx, idx;
    double *vec_b, sum;
    SparseMatrixClass *mmx_a;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *road_test_pt;
    DoubleIntIntClass mx_elem, t_mx_elem;

    ListClass<int> *unused_clutter_list = new ListClass<int>(0);
    ListClass<int> *assigned_list = new ListClass<int>(0);
    PolygonClass *map_bdy = create_map_bdy();

    num_insys_rtp = 0;
    for (rtp_idx=0; rtp_idx<=np->road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*(np->road_test_data_list))[rtp_idx] );
        cell_idx = road_test_pt->cell_idx;
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << np->bit_cell) | cell_idx;

        if (scan_index_list->contains(scan_idx)) {
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++;
            }
        }
    }

    num_var = num_clutter_type+useheight+1;

    create_svd_matrices(np, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

    delete map_bdy;

    prev_var_idx = -1;
    for ( col_idx=0; col_idx<=mmx_a->a->getSize()-1; col_idx++ ) {
        idx = (*mmx_a->col_list)[col_idx];
        mx_elem = (*mmx_a->a)[idx];

        var_idx = mx_elem.getInt(1);
        if (var_idx != prev_var_idx) {
            if ( useheight && (var_idx == 0) ) {
                // Do nothing
            } else {
                for (i=prev_var_idx+1; i<=var_idx-1; i++) {
                    unused_clutter_list->append(i-useheight);
                }
            }
        }
        prev_var_idx = var_idx;
    }
    for (i=prev_var_idx+1; i<=num_var-1; i++) {
        unused_clutter_list->append(i-useheight);
    }

    delete mmx_a;
    free(vec_b);

#if 1
    while(unused_clutter_list->getSize()) {

        assigned_list->reset();
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            i = clutter_idx % npts_x;
            j = clutter_idx / npts_x;

            n = 0;
            sum = 0.0;
            if (i > 0) {
                if (!unused_clutter_list->contains(clutter_idx-1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-1];
                }
            }
            if (i < npts_x-1) {
                if (!unused_clutter_list->contains(clutter_idx+1)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+1];
                }
            }
            if (j > 0) {
                if (!unused_clutter_list->contains(clutter_idx-npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx-npts_x];
                }
            }
            if (j < npts_y-1) {
                if (!unused_clutter_list->contains(clutter_idx+npts_x)) {
                    n++;
                    sum += mvec_x[useheight + clutter_idx+npts_x];
                }
            }

            if (n) {
                // printf("ASSIGN VALUE FOR CLUTTER TYPE %d\n", clutter_idx);
                
                mvec_x[useheight + clutter_idx] = ((sum >0.0) ? 0.0 : sum/n);
                
                assigned_list->append(clutter_idx);
            }
        }

        for (idx=0; idx<=assigned_list->getSize()-1; idx++) {
            clutter_idx = (*assigned_list)[idx];
            unused_clutter_list->del_elem(clutter_idx);
        }
        // printf("=============================\n");
    }
#else
        for (idx=0; idx<=unused_clutter_list->getSize()-1; idx++) {
            clutter_idx = (*unused_clutter_list)[idx];
            mvec_x[useheight + clutter_idx] = 1.0;
        }
#endif

    delete assigned_list;
    delete unused_clutter_list;

    return;
}

/*   Signed clutters with special color by coefficient levels.
 */ 
void ClutterGlobalPropModelClass::get_min_max_color(double &min_neg, double &max_pos)
{
    int clt_idx, c;
    double attn_res;

    min_neg = 0.0;
    max_pos = 0.0;

    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        attn_res =  mvec_x[clt_idx+useheight];
        if ( attn_res < min_neg ) {
            min_neg = attn_res;
        } else if ( attn_res > max_pos ) {
            max_pos = attn_res;
        }
    }

    if (min_neg == 0.0) {
        min_neg = -1.0;
    }

    if (max_pos == 0.0) {
        max_pos = 1.0;
    }
}

/*   Signed clutters with special color by coefficient levels.
 */ 
int ClutterGlobalPropModelClass::get_color(int clutter_type, double min_neg, double max_pos)
{
    int c;
    double attn_res;

    attn_res =  mvec_x[clutter_type+useheight];
    if (attn_res <= 0.0) {
        c = (int) floor( ( (attn_res - min_neg) / (- min_neg) ) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|c;
    } else {
        c = (int) floor( ( (max_pos - attn_res) / max_pos) * 255 + 0.5 );
        c = (c<<16)|(c<<8)|255;
    }

    return(c);
}

/*
 *  FUNCTION: ClutterPropModelFullClass::clt_fit_line()
 *            applicate in the condition of clutter algorithm without b & k              */
bool ClutterPropModelFullClass::clt_fit_line( double& k, double& b )
{
    b = 0.0;
    k = 0.0;
    int    clt_idx = 0;
    double M(0.0), N(0.0), U(0.0), V(0.0);
    double P(0.0);

    /*
     * Fit a line of 'Y = k*X + b' with clutter coefecients(X-asix) and constant data(Y-asix).     */
    int m(0), i(0);
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        if ( fabs(mvec_x[2*clt_idx+useheight]) > DERROR && fabs(mvec_x[2*clt_idx+useheight+1]) > DERROR ) {
            i ++;
            M += mvec_x[2*clt_idx+useheight];
            N += mvec_x[2*clt_idx+useheight+1];
            U += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight];
            V += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight+1];
        }
    }
    m = i;

    P = M*M - U*m;
    if ( !P || !U ) {
        return (false);
    }
    else {
        //Solve slope k and intercept b;
        b = (V*M - N*U) / (M*M - U*m);
        k = V/U - b*M/U;
    }

    // DBG
    //std::cout << "k = " << k << std::endl;
    //std::cout << "b = " << b << std::endl;

    return (true);
}

/*
 *  FUNCTION: ClutterSymFullPropModelClass::clt_fit_line()
 *            applicate in the condition of clutter algorithm without b & k              */
bool ClutterSymFullPropModelClass::clt_fit_line( double& k, double& b )
{
    b = 0.0;
    k = 0.0;
    int    clt_idx = 0;
    double M(0.0), N(0.0), U(0.0), V(0.0);
    double P(0.0);

    /*
     * Fit a line of 'Y = k*X + b' with clutter coefecients(X-asix) and constant data(Y-asix).     */
    int m(0), i(0);
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        if ( fabs(mvec_x[2*clt_idx+useheight]) > DERROR && fabs(mvec_x[2*clt_idx+useheight+1]) > DERROR ) {
            i ++;
            M += mvec_x[2*clt_idx+useheight];
            N += mvec_x[2*clt_idx+useheight+1];
            U += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight];
            V += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight+1];
        }
    }
    m = i;

    P = M*M - U*m;
    if ( !P || !U ) {
        return (false);
    }
    else {
        //Solve slope k and intercept b;
        b = (V*M - N*U) / (M*M - U*m);
        k = V/U - b*M/U;
    }

    // DBG
    //std::cout << "k = " << k << std::endl;
    //std::cout << "b = " << b << std::endl;

    return (true);
}

/*
 *  FUNCTION: ClutterWtExpoPropModelClass::clt_fit_line()
 *            applicate in the condition of clutter algorithm without b & k              */
bool ClutterWtExpoPropModelClass::clt_fit_line( double& k, double& b )
{
    b = 0.0;
    k = 0.0;
    int    clt_idx = 0;
    double M(0.0), N(0.0), U(0.0), V(0.0);
    double P(0.0);

    /*
     * Fit a line of 'Y = k*X + b' with clutter coefecients(X-asix) and constant data(Y-asix).     */
    int m(0), i(0);
    for ( clt_idx=0; clt_idx<num_clutter_type; clt_idx++ ) {
        if ( fabs(mvec_x[2*clt_idx+useheight]) > DERROR && fabs(mvec_x[2*clt_idx+useheight+1]) > DERROR ) {
            i ++;
            M += mvec_x[2*clt_idx+useheight];
            N += mvec_x[2*clt_idx+useheight+1];
            U += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight];
            V += mvec_x[2*clt_idx+useheight] * mvec_x[2*clt_idx+useheight+1];
        }
    }
    m = i;

    P = M*M - U*m;
    if ( !P || !U ) {
        return (false);
    }
    else {
        //Solve slope k and intercept b;
        b = (V*M - N*U) / (M*M - U*m);
        k = V/U - b*M/U;
    }

    // DBG
    //std::cout << "k = " << k << std::endl;
    //std::cout << "b = " << b << std::endl;

    return (true);
}
