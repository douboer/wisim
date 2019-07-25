/******************************************************************************************/
/**** PROGRAM: wisim.cpp                                                             ****/
/**** Michael Mandell 1/14/02                                                          ****/
/******************************************************************************************/
/**** Simulate cellular network using event-based Monte-Carlo simulation               ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iostream>

#ifdef __linux__
#include <unistd.h>
#else
#include <direct.h>
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "antenna.h"
#include "bin_io.h"
#include "cconst.h"
#include "wisim.h"
#include "coverage.h"
#include "clutter_data_analysis.h"
#include "doubleintint.h"
#include "hot_color.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "map_height.h"
#include "map_layer.h"
#include "match_data.h"
#include "mesh.h"
#include "phs.h"
#include "polygon.h"
#include "pref.h"
#include "prop_model.h"
#include "randomc.h"
#include "road_test_data.h"
#include "spline.h"
#include "statistics.h"
#include "strint.h"
#include "traffic_type.h"
#include "utm_conversion.h"

#include "clutter_data_analysis.h"

#if HAS_GUI
#include <qapplication.h>
#include <qbitmap.h>

#include "gcall.h"
#include "progress_slot.h"
#include "main_window.h"
#include "map_background.h"
#include "visibility_window.h"
extern int use_gui;
extern MainWindowClass *main_window;
#endif

/******************************************************************************************/
/**** Global Strings                                                                   ****/
/******************************************************************************************/
char gstr_cell_idx[] = "Cell Index";
/******************************************************************************************/

/******************************************************************************************/
/**** Default parameter values                                                         ****/
/******************************************************************************************/

double default_coverage_threshold_db              = -25.0;

/******************************************************************************************/

FILE *input_stream;

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
int                SectorClass       :: num_traffic           = 0;
int *              SectorClass       :: traffic_type_idx_list = (int *) 0;
int                CellClass         :: num_bm                = 0;
int                CellClass         :: text_color            = 0x000000;
int                NetworkClass      :: system_bdy_color      = 0x00FF00;
int                RoadTestPtClass   :: sort_type             = CConst::byPwrSort;
int                RoadTestPtClass   :: num_level             = 0;
double *           RoadTestPtClass   :: level_list            = (double *) NULL;
int *              RoadTestPtClass   :: color_list            = (int    *) NULL;
int                AntennaClass      :: color                 = 0x000000;
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: Virtual Functions                                                      ****/
/******************************************************************************************/
char *             CellClass         :: view_name(int, int)         { CORE_DUMP; return( (char *) NULL); }
int                SectorClass       :: get_ival(int)               { CORE_DUMP; return(-1);             }
double             SectorClass       :: st_comp_arrival_rate(int)   { CORE_DUMP; return(0.0);            }
const int          NetworkClass      :: technology()                { CORE_DUMP; return(-1);             }
void               NetworkClass      :: print_geometry(char *)      { CORE_DUMP; return;                 }
int                NetworkClass      :: check_parameters()          { CORE_DUMP; return(-1);             }
int                NetworkClass      :: Sum_st(int)                 { CORE_DUMP; return(-1);             }
void               NetworkClass      :: uid_to_sector(char *, int &, int &) { CORE_DUMP; return;         }
void               NetworkClass      :: uid_to_sector(int   , int &, int &) { CORE_DUMP; return;         }
int                NetworkClass      :: sector_to_uid(char *, int  , int  ) { CORE_DUMP; return(-1);     }
char *             NetworkClass      :: pa_to_str(int)              { CORE_DUMP; return( (char *) NULL); }

#if HAS_ORACLE
void      NetworkClass::read_geometry_db(char *)                  { CORE_DUMP; return;                 }
void      NetworkClass::print_geometry_db()                       { CORE_DUMP; return;                 }
#endif
/******************************************************************************************/

#if 0
Moved to global_fn.cpp
/******************************************************************************************/
/**** Read a line into string s, return length.  From "C Programming Language" Pg. 29  ****/
/**** Modified to be able to read both DOS and UNIX files.                             ****/
int fgetline(FILE *file, char *s)
{
    int c, i;

    for (i=0; (c=fgetc(file)) != EOF && c != '\n'; i++) {
        s[i] = c;
    }
    if ( (i >= 1) && (s[i-1] == '\r') ) {
        i--;
    }
    if (c == '\n') {
        s[i] = c;
        i++;
    }
    s[i] = '\0';
    return(i);
}
/******************************************************************************************/
#endif

/******************************************************************************************/
int get_file_size(char *filename)
{
    int size;

    std::ifstream file (filename, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open()) {
        size = (int) file.tellg();
        file.close();
    } else {
        size = -1;
    }
    return(size);
}
/******************************************************************************************/
/**** FUNCTION: get_path_clutter                                                       ****/
/******************************************************************************************/
void NetworkClass::get_path_clutter(int x0, int y0, int x1, int y1, double *dist)
{
    int i, n, si, sj, pi, pj, q;
    int map_i, map_j, map_i_1, map_j_1, clutter_type;
    double prev_dist, curr_dist, total_dist, unknown_dist;
    MapClutterClass *mc;

    mc = map_clutter;
    n = mc->map_sim_res_ratio;
    int dx = mc->offset_x;
    int dy = mc->offset_y;

    si = ((x1-x0>=0) ? 1 : -1);
    sj = ((y1-y0>=0) ? 1 : -1);

    pi = ((x1-x0>=0) ? 1 : 0);
    pj = ((y1-y0>=0) ? 1 : 0);

    total_dist = sqrt( (double) (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
//    printf("Get Path Clutter: (%d, %d) --> (%d, %d)\n", x0, y0, x1, y1);
//    printf("total_dist = %15.8f\n", total_dist);

    map_i = DIV(x0-dx, n);
    map_j = DIV(y0-dy, n);
    prev_dist  = 0.0;
    for (i=0; i<=mc->num_clutter_type-1; i++) {
        dist[i] = 0.0;
    }
    unknown_dist = 0.0;

    map_i_1 = DIV(x1-dx, n);
    map_j_1 = DIV(y1-dy, n);

    while ( (map_i != map_i_1) || (map_j != map_j_1) ) {
        clutter_type = mc->get_clutter_type(map_i, map_j);

//        q = (n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-dy)*(x0-dx) - (x1-dx)*(y0-dy))*si*sj;

        // Offset by 1/2 resolution
        q = ( 2*(n*((map_j+pj)*(x1-x0)-(map_i+pi)*(y1-y0)) + (y1-dy)*(x0-dx) - (x1-dx)*(y0-dy)) - (x1-x0) + (y1-y0) )*si*sj;

        if ( (q > 0) || ( (q == 0) && (pi) ) ) {
            curr_dist = total_dist*(n*(map_i+pi) - x0 + dx - 0.5)/(x1-x0);
            map_i += si;
        } else {
            curr_dist = total_dist*(n*(map_j+pj) - y0 + dy - 0.5)/(y1-y0);
            map_j += sj;
        }

        if (clutter_type == -1) {
            unknown_dist += curr_dist - prev_dist;
        } else {
            dist[clutter_type] += curr_dist - prev_dist;
        }

//        printf("map_i = %d (%d) map_j = %d (%d) curr_dist = %15.8f delta_dist = %15.8f clutter_type = %d (%s)\n",
//            map_i, x1/n, map_j, y1/n, curr_dist, curr_dist - prev_dist, clutter_type,
//            (clutter_type == -1 ? "UNKNOWN" : mc->description[clutter_type]));

        prev_dist = curr_dist;
    }

    clutter_type = mc->get_clutter_type(map_i, map_j);
    if (clutter_type == -1) {
        unknown_dist += total_dist - prev_dist;
    } else {
        dist[clutter_type] += total_dist - prev_dist;
    }

    for (i=0; i<=mc->num_clutter_type-1; i++) {
        dist[i] *= resolution;
    }
    unknown_dist *= resolution;

    return;
}
/******************************************************************************************/
/**** FUNCTION: check_grid_val                                                         ****/
/**** Checks that this given value is an integer multiple of the resolution.           ****/
/******************************************************************************************/
int check_grid_val(double val, double res, int offset, int *gdir_val_ptr)
{
    double d;
    int di, retval;
    double tolerance = 0.001;

    d  = val/res;
    di = (int) floor(d + 0.5);
    if (fabs(d - di) > tolerance) {
        retval = 0;
    } else {
        retval = 1;
    }

    *gdir_val_ptr = di - offset;

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::idx_to_x & idx_to_y                                      ****/
/******************************************************************************************/
double NetworkClass::idx_to_x(int x) {
    return( (system_startx+x)*resolution );
};
double NetworkClass::idx_to_y(int y) {
    return( (system_starty+y)*resolution );
};
/******************************************************************************************/

#if HAS_MONTE_CARLO
/******************************************************************************************/
/**** FUNCTION: NetworkClass::reset_system                                             ****/
/******************************************************************************************/
void NetworkClass::reset_system()
{
    int cell_idx, sector_idx, call_idx, event_idx, traffic_type_idx;
    CellClass *cell;
    SectorClass *sector;

    reset_call_statistics(0);

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            sector->active = 1;
            if (sector->call_list) {
                sector->call_list->reset();
            }
        }
    }

    if (master_call_list) {
        for(call_idx=0; call_idx<=master_call_list->getSize()-1; call_idx++) {
            free((*master_call_list)[call_idx]);
        }
        master_call_list->reset();
    }

    if (num_call_type) {
        for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
            num_call_type[traffic_type_idx] = 0;
        }
    }

    for (event_idx=0; event_idx<=num_pending_event-1; event_idx++) {
        delete pending_event_list[event_idx];
    }
    num_pending_event = 0;

    if (stat->plot_num_comm) {
        fclose(stat->fp_num_comm);
        stat->plot_num_comm = 0;
    }

    if (stat->plot_event) {
        fclose(stat->fp_event);
        stat->plot_event = 0;
    }

    if (stat->plot_throughput) {
        fclose(stat->fp_throughput);
        stat->plot_throughput = 0;
    }

    if (stat->plot_delay) {
        fclose(stat->fp_delay);
        stat->plot_delay = 0;
    }

    if (stat->measure_crr) {
        stat->measure_crr = 0;
        free(stat->p_crr);
    }

    abs_time = 0.0;
    // ran3(&seed, 1);
    // seed = ((int) time((time_t *) NULL)) & ((1<<25)-1);
    // rg->RandomInit(seed);

    return;
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: NetworkClass::~NetworkClass                                            ****/
/******************************************************************************************/
NetworkClass::~NetworkClass()
{
#if (CDEBUG)
    printf("Clearing memory ... \n");
#endif

    delete hot_color;

    free(RoadTestPtClass::level_list);
    free(RoadTestPtClass::color_list);

    delete rg;
    delete preferences;
    delete road_test_data_list;
    free(stat);
    free(prompt);
    free(line_buf);
    free(msg);

    delete map_layer_list;
    delete sector_group_list;
    delete sector_group_name_list;

    delete report_cell_name_opt_list;

#if HAS_MONTE_CARLO
    if (matchData) {
        delete matchData;
    }
#endif

    fclose(fl);

#if (CDEBUG)
    printf("Done clearing memory ... \n");
#endif
    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::close_geometry()                                         ****/
/******************************************************************************************/
void NetworkClass::close_geometry()
{
    int cell_idx, subnet_idx, ant_idx, sector_grp_idx;
    int map_layer_idx, pm_idx, tt_idx;
    int cvg_idx;

    if (mode == CConst::simulateMode) {
        switch_mode(CConst::editGeomMode);
    }

    road_test_data_list->reset();

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        delete cell_list[cell_idx];
    }
    if (num_cell) {
        free(cell_list);
    }

    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        delete prop_model_list[pm_idx];
    }
    if (num_prop_model) {
        free(prop_model_list);
    }

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            delete subnet_list[tt_idx][subnet_idx];
        }
        if (num_subnet[tt_idx]) {
            free(subnet_list[tt_idx]);
        }
        delete traffic_type_list[tt_idx];
    }
    if (num_traffic_type) {
        free(subnet_list);
        free(num_subnet);
        free(traffic_type_list);
    }

    if (SectorClass::num_traffic) {
        free(SectorClass::traffic_type_idx_list);
    }


    for (ant_idx=0; ant_idx<=num_antenna_type-1; ant_idx++) {
        delete antenna_type_list[ant_idx];
    }
    if (num_antenna_type) {
        free(antenna_type_list);
    }

    for (sector_grp_idx=0; sector_grp_idx<=sector_group_list->getSize()-1; sector_grp_idx++) {
        delete (ListClass<int> *) ((*sector_group_list)[sector_grp_idx]);
        free((*sector_group_name_list)[sector_grp_idx]);
    }
    sector_group_list->reset();
    sector_group_name_list->reset();


    for (map_layer_idx=0; map_layer_idx<=map_layer_list->getSize()-1; map_layer_idx++) {
        delete (*map_layer_list)[map_layer_idx];
    }
    map_layer_list->reset();

    if (map_clutter) {
        delete map_clutter;
    }

#if HAS_GUI
    if (map_background) {
        delete map_background;
    }
#endif

    if (map_height) {
        free(map_height->data);
        free(map_height);
    }

    for (cvg_idx=0; cvg_idx<=num_coverage_analysis-1; cvg_idx++) {
        delete coverage_list[cvg_idx];
    }
    if (coverage_list) {
        free(coverage_list);
    }

    delete system_bdy;
    clear_memory(cch_rssi_table);

    if (stat->plot_num_comm) {
        fclose(stat->fp_num_comm);
    }

    if (stat->plot_event) {
        fclose(stat->fp_event);
    }

    if (stat->plot_throughput) {
        fclose(stat->fp_throughput);
    }

    if (stat->plot_delay) {
        fclose(stat->fp_delay);
    }

    if (stat->measure_crr) {
        free(stat->p_crr);
    }

#if HAS_MONTE_CARLO
    if (matchData) {
        delete matchData;
    }
#endif

    set_default_parameters();

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::define_geometry()                                        ****/
/******************************************************************************************/
void NetworkClass::define_geometry(char *coordinate_system_str, char *system_bdy_str)
{
    int i, bdy_pt_idx, maxx, maxy;

    /**************************************************************************************/
    /**** Process Coordinate System                                                    ****/
    /**************************************************************************************/
    if (strcmp(coordinate_system_str, "GENERIC")==0) {
        coordinate_system = CConst::CoordGeneric;
    } else if (strncmp(coordinate_system_str, "UTM:", 4)==0) {
        coordinate_system = CConst::CoordUTM;
        coordinate_system_str = strtok(coordinate_system_str+4, ":");
        utm_equatorial_radius = atof(coordinate_system_str);
        coordinate_system_str = strtok(NULL, ":");
        utm_eccentricity_sq = atof(coordinate_system_str);
        coordinate_system_str = strtok(NULL, ":");
        utm_zone = atoi(coordinate_system_str);
        coordinate_system_str = strtok(NULL, ":");
        if (strcmp(coordinate_system_str, "N")==0) {
            utm_north = 1;
        } else if (strcmp(coordinate_system_str, "S")==0) {
            utm_north = 0;
        } else {
            coordinate_system = CConst::CoordUndefined;
            sprintf(msg, "ERROR: Invalid UTM Coordinate System specification\n");
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
    } else if (strcmp(coordinate_system_str, "LON_LAT")==0) {
        coordinate_system = CConst::CoordLONLAT;
    } else {
        sprintf(msg, "ERROR: Invalid Coordinate System specification, unrecognized type \"%s\"\n", coordinate_system_str);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Process System Boundary                                                      ****/
    /**************************************************************************************/
    int n = 0;
    int allocation_size = 10;
    double *x = DVECTOR(allocation_size);
    double *y = DVECTOR(allocation_size);
    proc_xy_list(n, x, y, system_bdy_str, allocation_size, error_state);

    if (error_state) {
        sprintf(msg, "ERROR: Invalid System Boundary specification\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    system_bdy = new PolygonClass();
    system_bdy->num_segment = 1;
    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
    system_bdy->num_bdy_pt[0] = n;

    if (n < 3) {
        sprintf(msg, "ERROR: Invalid system boundary specification, "
                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", system_bdy->num_bdy_pt[0]);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);

    for (bdy_pt_idx=0; bdy_pt_idx<=n-1; bdy_pt_idx++) {
         if (!check_grid_val(x[bdy_pt_idx], resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
             sprintf(msg, "ERROR: Invalid system boundary specification\n"
                              "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                              "resolution=%9.7f\n", bdy_pt_idx, x[bdy_pt_idx], resolution);
             PRMSG(stdout, msg);
             error_state = 1;
             return;
         }
         if (!check_grid_val(y[bdy_pt_idx], resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
             sprintf(msg, "ERROR: Invalid system boundary specification\n"
                              "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                              "resolution=%9.7f\n", bdy_pt_idx, y[bdy_pt_idx], resolution);
             PRMSG(stdout, msg);
             error_state = 1;
             return;
         }
    }

    system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
    npts_x = maxx - system_startx + 1;
    npts_y = maxy - system_starty + 1;

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
        system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
    }

    free(x);
    free(y);
    /**************************************************************************************/

    double area = system_bdy->comp_bdy_area();
    int tmp;
    if (area < 0.0) {
        for (i=0; i<=(n/2)-1; i++) {
            tmp = system_bdy->bdy_pt_x[0][i];
            system_bdy->bdy_pt_x[0][i] = system_bdy->bdy_pt_x[0][n-1-i];
            system_bdy->bdy_pt_x[0][n-1-i] = tmp;

            tmp = system_bdy->bdy_pt_y[0][i];
            system_bdy->bdy_pt_y[0][i] = system_bdy->bdy_pt_y[0][n-1-i];
            system_bdy->bdy_pt_y[0][n-1-i] = tmp;
        }
    }

    for (bdy_pt_idx=0; bdy_pt_idx<=n-1; bdy_pt_idx++) {
        for (i=bdy_pt_idx+2; i<=n-1-(bdy_pt_idx==0 ? 1 : 0); i++) {
            if (line_segments_cross(system_bdy->bdy_pt_x[0][bdy_pt_idx  ], system_bdy->bdy_pt_y[0][bdy_pt_idx  ],
                                    system_bdy->bdy_pt_x[0][bdy_pt_idx+1], system_bdy->bdy_pt_y[0][bdy_pt_idx+1],
                                    system_bdy->bdy_pt_x[0][i           ], system_bdy->bdy_pt_y[0][i           ],
                                    system_bdy->bdy_pt_x[0][(i+1)%n     ], system_bdy->bdy_pt_y[0][(i+1)%n     ]) ) {
                 sprintf(msg, "ERROR: Invalid system boundary specification, polygon segments cannot cross\n");
                 PRMSG(stdout, msg);
                 error_state = 1;
                 return;
            }
        }
    }

num_antenna_type = 1;
antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));
antenna_type_list[0] = new AntennaClass(CConst::antennaOmni, "OMNI");
}
/******************************************************************************************/
/**** FUNCTION: CellClass::~CellClass                                                  ****/
/******************************************************************************************/
CellClass::~CellClass()
{
    int sector_idx;
    SectorClass *sector;

    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        sector = sector_list[sector_idx];

        delete sector;
    }
    free(sector_list);

    return;
}
/******************************************************************************************/
/**** FUNCTION: SectorClass::~SectorClass                                              ****/
/******************************************************************************************/
SectorClass::~SectorClass()
{
    delete call_list;

#if 0
xxxxxxxxxx
    for(rt_idx=0; rt_idx<=num_road_test_pt-1; rt_idx++) {
        free(road_test_pt_list[rt_idx]);
    }
    if (num_road_test_pt) {
        free(road_test_pt_list);
    }
#endif

    if (num_traffic) {
        free(meas_ctr_list);
    }

    if (comment)  { free(comment);  }

#if 0
    if (num_unused_freq) {
        free(unused_freq);
    }

    if (csid_hex) { free(csid_hex); }
#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: clear_memory                                                           ****/
/**** flag: 0 Clear coverage analysis results only                                     ****/
/****       1 Clear entire structure                                                   ****/
/******************************************************************************************/
void clear_memory(CoverageClass *cvg, int flag)
{
    int p_idx;

    for (p_idx=0; p_idx<=cvg->scan_list->getSize()-1; p_idx++) {
        delete cvg->polygon_list[p_idx];
    }

    if (cvg->scan_list->getSize()) {
        free(cvg->polygon_list);
    }
    delete cvg->scan_list;

    if (flag) {
        delete cvg;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: LineClass::LineClass                                                   ****/
/******************************************************************************************/
LineClass::LineClass()
{
    num_pt = 0;
    pt_x    = (int *) NULL;
    pt_y    = (int *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: LineClass::~LineClass                                                  ****/
/******************************************************************************************/
LineClass::~LineClass()
{
    if (num_pt) {
        free(pt_x);
        free(pt_y);
    }
}
/******************************************************************************************/
/**** FUNCTION: LineClass::translate                                                   ****/
/******************************************************************************************/
void LineClass::translate(int x, int y)
{
    int i;

    for (i=0; i<=num_pt-1; i++) {
        pt_x[i] += x;
        pt_y[i] += y;
    }

    return;
}
/******************************************************************************************/

#if 0
/******************************************************************************************/
/**** FUNCTION: PolygonClass::PolygonClass                                             ****/
/******************************************************************************************/
PolygonClass::PolygonClass()
{
    num_segment = 0;
    num_bdy_pt  = (int  *) NULL;
    bdy_pt_x    = (int **) NULL;
    bdy_pt_y    = (int **) NULL;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::~PolygonClass                                            ****/
/******************************************************************************************/
PolygonClass::~PolygonClass()
{
    int segment_idx;

    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        free(bdy_pt_x[segment_idx]);
        free(bdy_pt_y[segment_idx]);
    }
    if (num_segment) {
        free(bdy_pt_x);
        free(bdy_pt_y);
        free(num_bdy_pt);
    }
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: SubnetClass::SubnetClass                                               ****/
/******************************************************************************************/
SubnetClass::SubnetClass()
{
    strid = (char *) NULL;
    arrival_rate = 0.0;
    minx = 0;
    maxx = 0;
    miny = 0;
    maxy = 0;
    color = 0;
    p = (PolygonClass *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: SubnetClass::~SubnetClass                                              ****/
/******************************************************************************************/
SubnetClass::~SubnetClass()
{
    if (strid) {
        free(strid);
    }

    if (p) {
        delete p;
    }
}
/******************************************************************************************/
/**** FUNCTION: clear_memory                                                           ****/
/******************************************************************************************/
void clear_memory(CchRssiTableClass *tbl)
{
    if (tbl) {
        free(tbl->sector_rx);
        free(tbl->sector_tx);
        free(tbl->rssi);
        free(tbl);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: CellClass::CellClass                                                   ****/
/******************************************************************************************/
CellClass::CellClass()
{
    posn_x = 0;
    posn_y = 0;;
    num_sector = 0;;
    sector_list = (SectorClass **) NULL;
    strid = (char *) NULL;
    color = 0;
    bm_idx = 2;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::NetworkClass                                             ****/
/******************************************************************************************/
NetworkClass::NetworkClass()
{
    int i;

    seed = ((int) time((time_t *) NULL)) & ((1<<25)-1);
    rg = new TRandomMersenne(seed);
    preferences = new PrefClass();
    map_layer_list = new ListClass<MapLayerClass *>(0);
    sector_group_list = new ListClass<void *>(0);
    sector_group_name_list = new ListClass<char *>(0);
    stat = (StatClass *) malloc(sizeof(StatClass));
    opt_msg = (const char **) NULL;

    road_test_data_list = new ListClass<RoadTestPtClass>(0);

    set_default_parameters();

    error_state = 0;
    warning_state = 0;
    prompt   = CVECTOR(15);
    line_buf = CVECTOR(MAX_LINE_SIZE);
    msg      = CVECTOR(MAX_LINE_SIZE);

#if 0
Delete -- use hot color class xxxxxxxxx
    num_default_color = 12;
    default_color_list              = IVECTOR(num_default_color);
    default_color_list[ 0]          = 0x406080;
    default_color_list[ 1]          = 0x00FF00;
    default_color_list[ 2]          = 0x0000FF;
    default_color_list[ 3]          = 0xFF0000;
    default_color_list[ 4]          = 0xFFFF00;
    default_color_list[ 5]          = 0xFF55FF;
    default_color_list[ 6]          = 0xFF0080;
    default_color_list[ 7]          = 0x005500;
    default_color_list[ 8]          = 0xFF8000;
    default_color_list[ 9]          = 0x00FFFF;
    default_color_list[10]          = 0x8000FF;
    default_color_list[11]          = 0x408060;
#endif

    hot_color = new HotColorClass();

    RoadTestPtClass::num_level = 7;
    RoadTestPtClass::level_list = DVECTOR(RoadTestPtClass::num_level);
    RoadTestPtClass::color_list = IVECTOR(RoadTestPtClass::num_level+1);
    double pwr_offset = get_pwr_unit_offset(CConst::PWRdBuV_113);
    for (i=0; i<=RoadTestPtClass::num_level-1; i++) {
        RoadTestPtClass::level_list[i] = 10.0*(i+1) + pwr_offset;
        // r = (int) ( 255.0*sqrt(i*1.0/(RoadTestPtClass::num_level-1)) );
        // b = (int) ( 255.0*sqrt((RoadTestPtClass::num_level-1-i)*1.0/(RoadTestPtClass::num_level-1)) );
        // RoadTestPtClass::color_list[i] = (r << 16) | (b << 8) | b;

        RoadTestPtClass::color_list[i] = hot_color->get_color(i, RoadTestPtClass::num_level+1);
    }
    i = RoadTestPtClass::num_level;
    RoadTestPtClass::color_list[i] = hot_color->get_color(i, RoadTestPtClass::num_level+1);

    add_rtd_pt_threshold = exp(-110.0*log(10.0)/10.0); // Threshold = -110 dBm

    CellClass::num_bm = 6;

    scan_cell_list = (ListClass<int> *) NULL;

    report_cell_name_opt_list = new ListClass<StrIntClass>(0);
    report_cell_name_opt_list->append( StrIntClass(gstr_cell_idx, CConst::CellIdxRef) );
    report_cell_name_opt_list->resize();

    strcpy(prompt, "WiSim> ");
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::set_default_parameters                                   ****/
/******************************************************************************************/
void NetworkClass::set_default_parameters()
{
    coordinate_system = CConst::CoordUndefined;
    system_bdy = (PolygonClass *) NULL;
    num_cell = 0;
    cell_list = (CellClass **) NULL;
    num_antenna_type = 0;
    num_prop_model = 0;
    prop_model_list = (PropModelClass **) NULL;
    master_call_list = (ListClass<void *> *) NULL;
    abs_time = 0.0;
    pending_event_list = (EventClass **) NULL;
    alloc_size_pending_event_list = 0;
    num_pending_event = 0;
    map_clutter = (MapClutterClass *) NULL;
    map_height = (MapHeightClass *) NULL;
    num_traffic_type = 0;
    traffic_type_list = (TrafficTypeClass **) NULL;
    num_subnet = (int *) NULL;
    subnet_list = (SubnetClass ***) NULL;
    total_arrival_rate = 0.0;
    num_call_type = (int *) NULL;;
    resolution = 1.0;
    res_digits = 0;
    system_startx = 0;
    system_starty = 0;
    npts_x = 100;
    npts_y = 100;
    mode = CConst::noGeomMode;

    SectorClass::num_traffic = 0;
    SectorClass::traffic_type_idx_list = (int *) NULL;

    num_coverage_analysis              = 0;
    coverage_list                      = (CoverageClass **) NULL;

    stat_count = (StatCountClass *) NULL;

#if HAS_MONTE_CARLO
    matchData = (MatchDataClass *) NULL;
#endif

    // stat = (StatClass *) malloc(sizeof(StatClass));
    stat->plot_num_comm = 0;
    stat->plot_event = 0;
    stat->plot_throughput = 0;
    stat->plot_delay = 0;
    stat->measure_crr = 0;
    stat->duration           = 0;

#if HAS_GUI
    prog_bar = (ProgressSlot *) NULL;
    map_background = (MapBackgroundClass *) NULL;
#endif
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::delete_all_unused_prop_model                             ****/
/******************************************************************************************/
void NetworkClass::delete_all_unused_prop_model()
{
    int cell_idx, sector_idx, pm_idx, new_pm_idx, unused_idx;
    ListClass<int> *pm_list = new ListClass<int>(num_prop_model);
    CellClass *cell;
    SectorClass *sector;

    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        pm_list->append(pm_idx);
    }

    for(cell_idx=0; (cell_idx<=num_cell-1)&&(pm_list->getSize()); cell_idx++) {
        cell = cell_list[cell_idx];
        for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if (sector->prop_model != -1) {
                pm_list->del_elem(sector->prop_model, 0);
            }
        }
    }

    pm_list->sort();

    printf("NUM_UNUSED = %d\n", pm_list->getSize());
    pm_list->printlist();

    ListClass<int> *pm_map = new ListClass<int>(num_prop_model);
    unused_idx = 0;
    new_pm_idx = 0;
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        if ((unused_idx <= pm_list->getSize()-1) && ((*pm_list)[unused_idx] == pm_idx)) {
            pm_map->append(-1);
            unused_idx++;
            delete prop_model_list[pm_idx];
            prop_model_list[pm_idx] = (PropModelClass *) NULL;
        } else {
            pm_map->append(new_pm_idx);
            new_pm_idx++;
        }
    }

    printf("NUM_PROP_MODEL = %d\n", num_prop_model);
    pm_map->printlist();

    new_pm_idx = 0;
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        if (prop_model_list[pm_idx]) {
            if (pm_idx != new_pm_idx) {
                prop_model_list[new_pm_idx] = prop_model_list[pm_idx];
                prop_model_list[pm_idx]     = (PropModelClass *) NULL;
            }
            new_pm_idx++;
        }
    }

    num_prop_model -= pm_list->getSize();
    prop_model_list = (PropModelClass **) realloc( (void *) prop_model_list, num_prop_model*sizeof(PropModelClass *));

    for(cell_idx=0; (cell_idx<=num_cell-1)&&(pm_list->getSize()); cell_idx++) {
        cell = cell_list[cell_idx];
        for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if (sector->prop_model != -1) {
                sector->prop_model = (*pm_map)[sector->prop_model];
            }
        }
    }

    free(pm_map);
    delete pm_list;

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::process_mode_change                                      ****/
/******************************************************************************************/
void NetworkClass::process_mode_change()
{
    int cell_idx, sector_idx, n, event_idx, traffic_type_idx;
    CellClass *cell;
    SectorClass *sector;

    if ( (mode == CConst::noGeomMode) || (mode == CConst::editGeomMode) ) {
        bit_cell = -1;
#if HAS_MONTE_CARLO
        reset_system();
        reset_call_statistics(3);
        for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                delete sector->call_list;
                sector->call_list = (ListClass<void *> *) NULL;;
            }
        }
        delete master_call_list;
        master_call_list = (ListClass<void *> *) NULL;;

        if (num_call_type) {
            free(num_call_type);
            num_call_type = (int *) NULL;;
        }

        reset_base_stations(0);
        for (event_idx=0; event_idx<=num_pending_event-1; event_idx++) {
            delete pending_event_list[event_idx];
        }
        free(pending_event_list);
        pending_event_list = (EventClass **) NULL;
        alloc_size_pending_event_list = 0;
        num_pending_event = 0;
#endif

    } else {
        BITWIDTH(bit_cell, num_cell-1);

        total_num_sectors = 0;
#if HAS_MONTE_CARLO
        n = 0;
#endif
        for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            total_num_sectors += cell->num_sector;
#if HAS_MONTE_CARLO
            for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                sector->call_list = new ListClass<void *>(0);
            }
#endif
        }
#if HAS_MONTE_CARLO
        master_call_list = new ListClass<void *>(0);
#endif

#if HAS_MONTE_CARLO
        StatCountClass::num_traffic_type  = num_traffic_type;
        StatCountClass::traffic_type_list = traffic_type_list;
        reset_call_statistics(1);
        reset_base_stations(1);

        num_call_type = IVECTOR(num_traffic_type);
        for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
            num_call_type[traffic_type_idx] = 0;
        }

        num_pending_event = 0;
        alloc_size_pending_event_list = 1000;
        pending_event_list = (EventClass **) malloc(alloc_size_pending_event_list*sizeof(EventClass *));
#endif

    }
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::delete_cell                                              ****/
/******************************************************************************************/
void NetworkClass::delete_cell(ListClass<int> *int_list)
{
    int i, j, cell_idx, rtd_idx, cvg_idx;
    RoadTestPtClass *rtp = (RoadTestPtClass *) NULL;
    CoverageClass *cvg = (CoverageClass *) NULL;

    ListClass<int> *new_cell_idx_list = new ListClass<int>(num_cell);

    if (!error_state) {
        j = 0;
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            if (int_list->contains(cell_idx)) {
                new_cell_idx_list->append(-1);
            } else {
                new_cell_idx_list->append(j);
                j++;
            }
        }
    }

#if HAS_GUI
    VisibilityList *vlist        = (VisibilityList *) NULL;
    VisibilityWindow *vw;
    if ( (use_gui) && (!error_state) ) {
        vw    = main_window->visibility_window;
        vlist = main_window->visibility_window->visibility_list;
        for (i=0; i<=int_list->getSize()-1; i++) {
            cell_idx = (*int_list)[i];
            vw->delete_cell(cell_idx);
        }
        j = 0;
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            if ((*new_cell_idx_list)[cell_idx] != -1) {
                vw->change_cell_idx(cell_idx, (*new_cell_idx_list)[cell_idx]);
            }
        }
        vw->vec_vis_cell = (char *) realloc((void *) vw->vec_vis_cell, (num_cell-int_list->getSize())*sizeof(char));
        vw->resize();
    }
#endif

    if (!error_state) {
        for (i=0; i<=int_list->getSize()-1; i++) {
            cell_idx = (*int_list)[i];
            delete cell_list[cell_idx];
            cell_list[cell_idx] = (CellClass *) NULL;
        }
        j = 0;
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            j = (*new_cell_idx_list)[cell_idx];
            if ( (j != -1) && (j != cell_idx) ) {
                cell_list[j] = cell_list[cell_idx];
            }
        }
        j = num_cell - int_list->getSize();
        while(j<=num_cell-1) {
            cell_list[j] = (CellClass *) NULL;
            j++;
        }
        num_cell-=int_list->getSize();
        cell_list = (CellClass **) realloc( (void *) cell_list, num_cell*sizeof(CellClass *));

        rtd_idx = 0;
        while(rtd_idx<=road_test_data_list->getSize()-1) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            j = (*new_cell_idx_list)[cell_idx];
            if (j == -1) {
                road_test_data_list->del_elem_idx(rtd_idx, 1);
            } else {
                rtp->cell_idx = j;
                rtd_idx++;
            }
        }
        RoadTestPtClass::sort_type = CConst::byPwrSort;
        road_test_data_list->sort();

        for (cvg_idx=0; cvg_idx<=num_coverage_analysis-1; cvg_idx++) {
            cvg = coverage_list[cvg_idx];
            if (cvg->clipped_region) {
                i = 0;
                while(i<=cvg->cell_list->getSize()-1) {
                    cell_idx = (*cvg->cell_list)[i];
                    j = (*new_cell_idx_list)[cell_idx];
                    if (j == -1) {
                        cvg->cell_list->del_elem_idx(i, 1);
                    } else {
                        (*cvg->cell_list)[i] = j;
                        i++;
                    }
                }
            }
        }
    }

    delete new_cell_idx_list;

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::delete_prop_model                                        ****/
/******************************************************************************************/
void NetworkClass::delete_prop_model(ListClass<int> *int_list)
{
    int i, j, cell_idx, sector_idx, pm_idx;
    CellClass *cell;
    SectorClass *sector;

    ListClass<int> *new_pm_idx_list = new ListClass<int>(num_prop_model);

    if (!error_state) {
        j = 0;
        for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
            if (int_list->contains(pm_idx)) {
                new_pm_idx_list->append(-1);
            } else {
                new_pm_idx_list->append(j);
                j++;
            }
        }
    }

#if HAS_GUI
    VisibilityList *vlist        = (VisibilityList *) NULL;
    VisibilityWindow *vw;
    VisibilityItem   *vi;
    VisibilityCheckItem *vci     = (VisibilityCheckItem *) NULL;
    VisibilityCheckItem *del_vci = (VisibilityCheckItem *) NULL;
    int flag = 0;

    if ( (use_gui) && (!error_state) ) {
        vlist = main_window->visibility_window->visibility_list;
        vi  = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::clutterPropModelRTTI, 0);
        if (vi) {
            vci = (VisibilityCheckItem *) vi->firstChild();
            del_vci = (VisibilityCheckItem *) NULL;
            while(vci) {
                i =  vci->getIndex();
                if (int_list->contains(i)) {
                    del_vci = vci;
                } else if (i != (*new_pm_idx_list)[i]) {
                    vci->setIndex((*new_pm_idx_list)[i]);
                }
                vci = (VisibilityCheckItem *) vci->nextSibling();

                if (del_vci) {
                    delete del_vci;
                    del_vci = (VisibilityCheckItem *) NULL;
                }
            }

            if (!vi->firstChild()) {
                delete vi;
                flag = 1;
            }
        }
        if (flag) {
            main_window->editor->setVisibility(GConst::clutterPropModelRTTI);
        }
    }
#endif

    if (!error_state) {
        for (i=0; i<=int_list->getSize()-1; i++) {
            pm_idx = (*int_list)[i];
            delete prop_model_list[pm_idx];
            prop_model_list[pm_idx] = (PropModelClass *) NULL;
        }
        j = 0;
        for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
            j = (*new_pm_idx_list)[pm_idx];
            if ( (j != -1) && (j != pm_idx) ) {
                prop_model_list[j] = prop_model_list[pm_idx];
            }
        }
        j = num_prop_model - int_list->getSize();
        while(j<=num_prop_model-1) {
            prop_model_list[j] = (PropModelClass *) NULL;
            j++;
        }
        num_prop_model-=int_list->getSize();
        prop_model_list = (PropModelClass **) realloc( (void *) prop_model_list, num_prop_model*sizeof(PropModelClass *));

        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                pm_idx = sector->prop_model;
                if (pm_idx == -1) {
                    j = -1;
                } else {
                    j = (*new_pm_idx_list)[pm_idx];
                }
                sector->prop_model = j;
            }
        }
    }

    delete new_pm_idx_list;

    return;
}
/******************************************************************************************/
/**** FUNCTION: set_total_arrival_rate                                                 ****/
/******************************************************************************************/
void NetworkClass::set_total_arrival_rate(int traffic_type_idx, double total_arrival_rate)
{
    int subnet_idx;
    SubnetClass *subnet;
    TrafficTypeClass *traffic_type = traffic_type_list[traffic_type_idx];

    double prev_total_rate = 0.0;
    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
        subnet = subnet_list[traffic_type_idx][subnet_idx];
        prev_total_rate += subnet->arrival_rate;
    }

    if (prev_total_rate == 0.0) {
        sprintf( msg, "ERROR: traffic type \"%s\" has no subnets with non-zero traffic\n", traffic_type->name());
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    double scale = total_arrival_rate / prev_total_rate;

    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
        subnet = subnet_list[traffic_type_idx][subnet_idx];
        subnet->arrival_rate *= scale;
    }

}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: EventClass::EventCLass                                                 ****/
/******************************************************************************************/
EventClass::EventClass()
{
}
/******************************************************************************************/
/**** FUNCTION: EventClass::~EventCLass                                                ****/
/******************************************************************************************/
EventClass::~EventClass()
{
}
/******************************************************************************************/
/**** FUNCTION: sqerr                                                                  ****/
/******************************************************************************************/
double sqerr(double x, double y)
{
    double err_eps = 0.01;
    double error;

    if (y > err_eps) {
        error = (x - y) / y;
        error *= error;
    } else if (y == 0) {
        error = x;
    } else {
        error = (x - y)*(x - y) / (err_eps * y);
    }

    return(error);
}
/******************************************************************************************/
/**** FUNCTION: EventClass::copy                                                       ****/
/******************************************************************************************/
void EventClass::copy(EventClass *e, double abs_time)
{
    event_type        = e->event_type;
    traffic_type_idx  = e->traffic_type_idx;

    time              = e->time - abs_time;
    posn_x            = e->posn_x;
    posn_y            = e->posn_y;

    cs_idx            = e->cs_idx;
    master_idx        = e->master_idx;
}
/******************************************************************************************/
/**** FUNCTION: comp_prop                                                              ****/
/**** This routine computes power propagation through the link between the CS located  ****/
/**** at (cell_idx, sector_idx) and a PS located at position (x,y).                    ****/
/******************************************************************************************/
double SectorClass::comp_prop(NetworkClass *np, int x, int y, int max_dist)
{
    int comp;
    double power = 0.0;

    if (prop_model != -1) {
        int delta_x  = (x - parent_cell->posn_x);
        int delta_y  = (y - parent_cell->posn_y);

        if (max_dist) {
            comp = 0;
            if (delta_x >= max_dist) {
                power = -1.0;
            } else if (delta_y >= max_dist) {
                power = -1.0;
            } else if (((double) delta_x)*delta_x + ((double) delta_y)*delta_y >= ((double) max_dist)*max_dist) {
                power = -1.0;
            } else {
                comp = 1;
            }
        } else {
            comp = 1;
        }

        if (comp) {
            PropModelClass *pm = np->prop_model_list[prop_model];

            double prop = pm->prop_power_loss(np, this, delta_x, delta_y);

            AntennaClass *antenna = np->antenna_type_list[antenna_type];
            double antenna_gain_db = antenna->gainDB(delta_x*np->resolution, delta_y*np->resolution, -antenna_height, antenna_angle_rad);

            if (antenna_gain_db == 0.0) {
                power = prop;
            } else {
                double antenna_gain    = exp(antenna_gain_db * log(10.0) / 10.0);
                power = prop*antenna_gain;
            }

        }
    }

    return(power);
}
/******************************************************************************************/
/**** FUNCTION: comp_sir_cs                                                            ****/
/**** This routine computes the Signal-to-Interference ratio (SIR) at the CS for the   ****/
/**** CS located at (cell_idx, sector_idx) and a PS located at position (x, y) on the  ****/
/**** specified channel.                                                               ****/
/******************************************************************************************/
double SectorClass::comp_sir_cs(NetworkClass *np, int x, int y, int channel, double *ptr_interference)
{
    int i_cell_idx, i_sector_idx, call_idx, found;
    double signal, interference, sir;
    CellClass *i_cell;
    SectorClass *i_sector;
    CallClass *call;

    signal = comp_prop(np, x, y);

    interference = 0.0;

    for (i_cell_idx=0; i_cell_idx<=np->num_cell-1; i_cell_idx++) {
        i_cell   = np->cell_list[i_cell_idx];
        for (i_sector_idx=0; i_sector_idx<=i_cell->num_sector-1; i_sector_idx++) {
            i_sector = i_cell->sector_list[i_sector_idx];
            if (i_sector != this) {
                found = 0;
                for (call_idx=0; (call_idx<=i_sector->call_list->getSize()-1)&&(!found); call_idx++) {
                    call = (CallClass *) (*(i_sector->call_list))[call_idx];
                    if (call->channel == channel) {
                        found = 1;
                        interference += comp_prop(np, call->posn_x, call->posn_y);
                    }
                }
            }
        }
    }

    if (interference > 0.0) {
        sir = signal / interference;
    } else {
        sir = MAX_SIR;
    }

    *ptr_interference = interference;

    return(sir);
}
/******************************************************************************************/
/**** FUNCTION: display_setting                                                        ****/
/**** Display parameter settings                                                       ****/
/******************************************************************************************/
void NetworkClass::display_settings(FILE *fp)
{
    char *chptr;

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "GENERIC PARAMETER SETTINGS:\n");
    chptr += sprintf(chptr, "RANDOM NUM GENERATOR SEED        = %d\n",                   seed);
    chptr += sprintf(chptr, "RESOLUTION                       = %-15.10f\n",        resolution);
    chptr += sprintf(chptr, "FREQUENCY                        = %-15.1f (Hz)\n",       frequency);
    PRMSG(fp, msg);

    chptr = msg;
    if (coordinate_system == CConst::CoordGeneric) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: GENERIC\n");
    } else if (coordinate_system == CConst::CoordUTM) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: UTM:%.0f:%.9f:%d:%c\n", utm_equatorial_radius, utm_eccentricity_sq,
            utm_zone, (utm_north ? 'N' : 'S'));
    } else if (coordinate_system == CConst::CoordLONLAT) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: LON_LAT\n");
    } else if (coordinate_system == CConst::CoordUndefined) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: UNDEFINED\n");
    } else {
        CORE_DUMP;
    }
    PRMSG(fp, msg);


    return;
}
/******************************************************************************************/

#if HAS_MONTE_CARLO
/******************************************************************************************/
/**** FUNCTION: reset_call_statistics                                                  ****/
/**** Reset counters used to measure system statistics.                                ****/
/**** allocate: 0 reset only                          reset                            ****/
/****           1 allocate, then reset                create                           ****/
/****           2 free, reallocate, then reset                                         ****/
/****           3 free only                           delete                           ****/
/******************************************************************************************/
void NetworkClass::reset_call_statistics(int allocate)
{
    int cell_idx, sector_idx;
    CellClass *cell;
    SectorClass *sector;

    stat->duration = 0.0;
    if ( (allocate == 0) && (stat_count) ) {
        stat_count->reset();
    } else if (allocate == 1) {
        stat_count = create_call_stat_count();
    } else if (allocate == 3) {
        delete stat_count; stat_count = (StatCountClass *) NULL;
    }

    for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];

            if ( (allocate == 0) && (sector->stat_count) ) {
                sector->stat_count->reset();
            } else if (allocate == 1) {
                sector->stat_count = create_call_stat_count();
            } else if (allocate == 3) {
                delete sector->stat_count; sector->stat_count = (StatCountClass *) NULL;
            }
        }
    }

    return;
}
/******************************************************************************************/
#endif

#if 1
/******************************************************************************************/
/**** FUNCTION: PolygonClass::comp_bdy_area                                            ****/
/**** Compute area from list of boundary points                                        ****/
/******************************************************************************************/
double PolygonClass::comp_bdy_area(const int n, const int *x, const int *y)
{
    int i, x1, y1, x2, y2;
    double area;

    area = 0.0;

    for (i=1; i<=n-2; i++) {
        x1 = x[i] - x[0];
        y1 = y[i] - y[0];
        x2 = x[i+1] - x[0];
        y2 = y[i+1] - y[0];

        area += ( (double) x1)*y2 - ( (double) x2)*y1;
    }

    area /= 2.0;

    return(area);
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: PolygonClass::comp_bdy_area                                            ****/
/**** Compute area from list of boundary points                                        ****/
/******************************************************************************************/
double PolygonClass::comp_bdy_area(ListClass<IntIntClass> *ii_list)
{
    int i, x1, y1, x2, y2, n;
    double area;

    area = 0.0;

    n = ii_list->getSize();
    for (i=1; i<=n-2; i++) {
        x1 = (*ii_list)[i].x() - (*ii_list)[0].x();
        y1 = (*ii_list)[i].y() - (*ii_list)[0].y();
        x2 = (*ii_list)[i+1].x() - (*ii_list)[0].x();
        y2 = (*ii_list)[i+1].y() - (*ii_list)[0].y();

        area += ( (double) x1)*y2 - ( (double) x2)*y1;
    }

    area /= 2.0;

    return(area);
}
/******************************************************************************************/

#if 1
/******************************************************************************************/
/**** FUNCTION: in_bdy_area                                                            ****/
/**** Determine whether or not a given point lies within the bounded area              ****/
/******************************************************************************************/
int PolygonClass::in_bdy_area(const int a, const int b, const int n, const int *x, const int *y, int *edge)
{
    int i, num_left, num_right, same_y, index;
    int x1, y1, x2, y2, eps;

    index = -1;
    do {
        index++;
        if (index == n) { return(0); }
        x2 = x[index];
        y2 = y[index];
    } while (y2 == b);

    if (edge) { *edge = 0; }
    same_y   = 0;
    num_left = 0;
    num_right = 0;
    for (i=0; i<=n-1; i++) {
        if (index == n-1) {
            index = 0;
        } else {
            index++;
        }
        x1 = x2;
        y1 = y2;
        x2 = x[index];
        y2 = y[index];

        if ( (x2 == a) && (y2 == b) ) {
            if (edge) { *edge = 1; }
            return(0);
        }

        if (!same_y) {
            if (    ((y1 < b) && (b < y2))
                 || ((y1 > b) && (b > y2)) ) {
                if ( (x1 > a) && (x2 > a) ) {
                    num_right++;
                } else if ( (x1 < a) && (x2 < a) ) {
                    num_left++;
                } else {
                    eps = ((x2-x1)*(b-y1)-(a-x1)*(y2-y1));
                    if (eps == 0) {
                        if (edge) { *edge = 1; }
                        return(0);
                    }
                    if ( ((y1<y2) && (eps > 0)) || ((y1>y2) && (eps < 0)) ) {
                        num_right++;
                    } else {
                        num_left++;
                    }
                }
            } else if (y2 == b) {
                same_y = (y1 > b) ? 1 : -1;
            }
        } else {
            if (y2 == b) {
                if (  ((x1 <= a) && (a <= x2))
                    ||((x2 <= a) && (a <= x1)) ) {
                    if (edge) { *edge = 1; }
                    return(0);
                }
            } else {
                if (  ((y2 < b) && (same_y == 1))
                    ||((y2 > b) && (same_y == -1)) ) {
                    if (x1 < a) {
                        num_left++;
                    } else {
                        num_right++;
                    }
                }
                same_y = 0;
            }
        }
    }

    if ((num_left + num_right) & 1) {
        printf("ERROR in routine in_bdy_area()\n");
        CORE_DUMP;
    }

    return(num_left&1);
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: spline_init_lut                                                        ****/
/**** INPUTS: flname, xcol, ycol, tabsize                                              ****/
/**** OUTPUTS: gain[0..tabsize-1]                                                      ****/
/**** Reads a two-column tabular data file and produces a lookup table of tabsize      ****/
/**** points equally spaced in x for columns xcol and ycol.  Third order spline        ****/
/**** interpolation is used.                                                           ****/
/**** Return value: 1 if successfull, 0 if not successful                              ****/
/******************************************************************************************/
int spline_init_lut(char *flname, int tabsize, double *gain)
{
    int numrows, i;
    double *f_gain, *f_phs;
    double xval, x_start, x_stop, u;
    char *chptr, *errmsg;
    SplineClass *spline;

    errmsg = CVECTOR(MAX_LINE_SIZE);
    f_gain = DVECTOR(MAX_PTS);
    f_phs  = DVECTOR(MAX_PTS);

    spline = (SplineClass *) malloc(sizeof(SplineClass));

    if (!read_two_col(f_phs, f_gain, &numrows, flname)) {
        return(0);
    }

    if (f_phs[0] + 360.0 > f_phs[numrows-1]) {
        numrows++;
        f_phs[numrows-1]  = f_phs[0] + 360.0;
        f_gain[numrows-1] = f_gain[0];
    } else if ( (f_phs[0] + 360.0 != f_phs[numrows-1]) || (f_gain[0] != f_gain[numrows-1]) ) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine spline_init_lut()\n");
        chptr += sprintf(chptr, "Data in file %s does not seem to be periodic\n", flname);
        PRMSG(stdout, errmsg);
        return(0);
    }

    spline->a = DVECTOR(numrows+2);
    spline->b = DVECTOR(numrows+2);
    spline->c = DVECTOR(numrows+2);
    spline->d = DVECTOR(numrows+2);
    spline->x = DVECTOR(numrows+2);

    /*****************************************************/
    /**** Convert phase from degrees to radians       ****/
    /*****************************************************/
    for (i=0; i<=numrows-1; i++) {
        f_phs[i] *= PI/180.0;
    }
    /*****************************************************/

    x_start = -PI;
    x_stop  =  PI;

    /*****************************************************/
    /***** Make spline                               *****/
    /*****************************************************/
    if (!spline->makesplinecoeffs(1,numrows,f_phs-1,f_gain-1)) {
        return(0);
    }

    for (i=0; i<=tabsize-1; i++) {
        u = (double) i/(tabsize-1);
        xval = x_start*(1.0-u) + x_stop*u;
        while(xval >= f_phs[0]+2*PI) { xval -= 2*PI; }
        while(xval < f_phs[0])       { xval += 2*PI; }
        gain[i] = spline->splineval(xval);
    }
    /*****************************************************/

    free(spline->a);
    free(spline->b);
    free(spline->c);
    free(spline->d);
    free(spline->x);
    free(spline);
    free(f_gain);
    free(f_phs);
    free(errmsg);

    return(1);
}
/******************************************************************************************/
/**** FUNCTION: spline_init_lut                                                        ****/
/**** INPUTS: flname, xcol, ycol, tabsize                                              ****/
/**** OUTPUTS: gain[0..tabsize-1]                                                      ****/
/**** Reads a two-column tabular data file and produces a lookup table of tabsize      ****/
/**** points equally spaced in x for columns xcol and ycol.  Third order spline        ****/
/**** interpolation is used.                                                           ****/
/**** Return value: 1 if successfull, 0 if not successful                              ****/
/******************************************************************************************/
int spline_init_lut(double *&f_phs, double *&f_gain, int numrows, int tabsize, double *gain)
{
    int i;
    double xval, x_start, x_stop, u;
    char *chptr, *errmsg;
    SplineClass *spline;

    errmsg = CVECTOR(MAX_LINE_SIZE);

    spline = (SplineClass *) malloc(sizeof(SplineClass));

    if (f_phs[0] + 360.0 > f_phs[numrows-1]) {
        numrows++;
        f_phs  = (double *) realloc((void *) f_phs,  numrows*sizeof(double));
        f_gain = (double *) realloc((void *) f_gain, numrows*sizeof(double));
        f_phs[numrows-1]  = f_phs[0] + 360.0;
        f_gain[numrows-1] = f_gain[0];
    } else if ( (f_phs[0] + 360.0 != f_phs[numrows-1]) || (f_gain[0] != f_gain[numrows-1]) ) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine spline_init_lut()\n");
        chptr += sprintf(chptr, "Data does not seem to be periodic\n");
        PRMSG(stdout, errmsg);
        return(0);
    }

    spline->a = DVECTOR(numrows+2);
    spline->b = DVECTOR(numrows+2);
    spline->c = DVECTOR(numrows+2);
    spline->d = DVECTOR(numrows+2);
    spline->x = DVECTOR(numrows+2);

    /*****************************************************/
    /**** Convert phase from degrees to radians       ****/
    /**** Convert gain from dB to power gain          ****/
    /*****************************************************/
    for (i=0; i<=numrows-1; i++) {
        f_phs[i] *= PI/180.0;
        // f_gain[i] = exp(f_gain[i]*log(10.0)/10.0);
    }
    /*****************************************************/

    x_start = -PI;
    x_stop  =  PI;

    /*****************************************************/
    /***** Make spline                               *****/
    /*****************************************************/
    if (!spline->makesplinecoeffs(1,numrows,f_phs-1,f_gain-1)) {
        return(0);
    }

    for (i=0; i<=tabsize-1; i++) {
        u = (double) i/(tabsize-1);
        xval = x_start*(1.0-u) + x_stop*u;
        while(xval >= f_phs[0]+2*PI) { xval -= 2*PI; }
        while(xval < f_phs[0])       { xval += 2*PI; }
        gain[i] = spline->splineval(xval);
    }
    /*****************************************************/

    free(spline->a);
    free(spline->b);
    free(spline->c);
    free(spline->d);
    free(spline->x);
    free(spline);
    free(errmsg);

    return(1);
}
/******************************************************************************************/
/***  FUNCTION: spline_eval_lut                                                         ***/
/***  INPUTS:                                                                           ***/
/***  OUTPUTS:                                                                          ***/
/***                                                                                    ***/
double spline_eval_lut(double phase, double *gain_vec, int tabsize)
{
    double index, g1, g2, gval;
    int i1, i2;

    index = (phase+PI)*(tabsize-1)/(2*PI);

    i1 = (int) floor(index);
    i2 = i1 + 1;

    if (i1 < 0) {
        i1=0;
        i2=1;
    } else if (i2 > tabsize-1) {
        i1=tabsize-2;
        i2=tabsize;
    }

    g1 = gain_vec[i1];
    g2 = gain_vec[i2];

    gval = (index-i1)*g2 + (i2-index)*g1;

    return(gval);
}
/******************************************************************************************/
/**** FUNCTION: read_two_col                                                           ****/
/**** INPUTS: file "flname"                                                            ****/
/**** OUTPUTS: n, x[0...n-1], y[0...n-1]                                               ****/
/**** Reads two column tabular data from the file "flname".  The number of lines of    ****/
/**** data in the file is n.  This first column is read into the variables x[i] and    ****/
/**** the second column is read into the variables y[i] (i=0,1,...,n-1).  Lines        ****/
/**** beginning with the character '#' are ignored (to allow for comments).  Uses      ****/
/**** MAX_LINE_SIZE and MAX_PTS as the maximum number of characters per line and the   ****/
/**** maximum number of points in the file respectively.                               ****/
/**** Return value: 1 if successfull, 0 if not successful                              ****/
/******************************************************************************************/
int read_two_col(double *x, double *y, int *ptr_n, char *flname)
{
    char *line, *lnptr, *chptr, *errmsg;
    FILE *fp;

    line   = CVECTOR(MAX_LINE_SIZE);
    errmsg = CVECTOR(MAX_LINE_SIZE);

#define TMP_NEDELIM (lnptr[0] != ',')&&(lnptr[0] != ' ')&&(lnptr[0] != '\t')
#define TMP_EQDELIM (lnptr[0] == ',')||(lnptr[0] == ' ')||(lnptr[0] == '\t')

    if ( strcmp(flname,"stdin") == 0 ) {
        fp = stdin;
    } else if ( !(fp = fopen(flname, "rb")) ) {
        sprintf(errmsg, "ERROR: Unable to read from file 1 %s\n", flname);
        PRMSG(stdout, errmsg);
        return(0);
    }

    *ptr_n = 0;

    while ( fgetline(fp, line) ) {
        lnptr = line;
        while ( (lnptr[0] == ' ') || (lnptr[0] == '\t') ) lnptr++;
        if ( (lnptr[0] != '#') && (lnptr[0] != '\n') ) {
            if ((*ptr_n) >= MAX_PTS) {
                chptr = errmsg;
                chptr += sprintf(chptr, "ERROR in routine read_two_col()\n");
                chptr += sprintf(chptr, "Number of points in file %s exceeds MAX_PTS = %d\n", flname, MAX_PTS);
                PRMSG(stdout, errmsg);
                return(0);
            }

            x[(*ptr_n)] = atof(lnptr);
            while ( TMP_NEDELIM ) lnptr++;
            while ( TMP_EQDELIM ) lnptr++;
            y[(*ptr_n)] = atof(lnptr);

            (*ptr_n)++;
        }
    }
    if (fp != stdin) {
        fclose(fp);
    }

    free(line);
    free(errmsg);
#undef TMP_NEDELIM
#undef TMP_EQDELIM

    return(1);
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::redefine_system_bdy                                      ****/
/******************************************************************************************/
void NetworkClass::redefine_system_bdy(double threshold_db, double scan_fractional_area, int init_sample_resolution)
{
    int i;
    char *chptr;
    CoverageClass *cvg;
    PolygonClass *p;
    int poly_error = 0;

    switch_mode(CConst::simulateMode);

    if (error_state) {
        return;
    }
    num_coverage_analysis++;
    coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));
    coverage_list[num_coverage_analysis-1] = new CoverageClass(this, "redefine_system_bdy", CConst::layerCoverage);

    cvg = coverage_list[num_coverage_analysis-1];

    cvg->threshold  = exp(threshold_db*log(10.0)/10.0);
    cvg->init_sample_res = init_sample_resolution;
    cvg->scan_fractional_area = scan_fractional_area;

    for (i=cvg->scan_list->getSize(); i<2; i++) {
        cvg->scan_list->append(i);
    }
    for (i=cvg->scan_list->getSize()-1; i>=2; i--) {
        cvg->scan_list->del_elem(i);
    }

    run_coverage(num_coverage_analysis-1);

    // cvg->polygon_list[1] may be NULL
    if ( cvg->polygon_list[1] == NULL )
        return;

    p = cvg->polygon_list[1];

    /* in common case segment number is more than 1 */
    //    if (p->num_segment != 1) {
    //        chptr = msg;
    //        chptr += sprintf(chptr, "ERROR: Polynomial resulting from coverage analysis has %d segments\n", p->num_segment);
    //        PRMSG(stdout, msg);
    //        poly_error = 1;
    //    }

    if (p->comp_bdy_area() == 0.0) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR: Polynomial resulting from coverage analysis has area = 0\n");
        PRMSG(stdout, msg);
        poly_error = 1;
    }

    if ( (!error_state) && (!poly_error) ) {
        delete system_bdy;
        system_bdy = p->duplicate();
    }

    delete cvg;
    num_coverage_analysis--;
    coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));

    switch_mode(CConst::editGeomMode);

    if (poly_error) {
        error_state = 1;
    }

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: switch_mode                                                            ****/
/******************************************************************************************/
void NetworkClass::switch_mode(int new_mode)
{
    if ( (!error_state) && (new_mode==CConst::simulateMode) && (!system_bdy) ) {
        sprintf( msg, "No system boundary defined, cannot switch to SIMULATE mode\n");
        PRMSG(stdout, msg); error_state = 1;
    }
    if ( (!error_state) && (new_mode==CConst::simulateMode) && (num_cell == 0) ) {
        sprintf( msg, "ERROR: No cells defined, cannot switch to SIMULATE mode\n");
        PRMSG(stdout, msg); error_state = 1;
    }
    if ( (!error_state) && (new_mode==CConst::simulateMode) ) {
        check_parameters();
        check_prop_model();
        check_traffic();
    }
    if (!error_state) {
        if (new_mode == mode) {
            sprintf(msg, "WARNING: Mode already set to %s\n",
                (mode == CConst::editGeomMode) ? "EDIT_GEOMETRY" : "SIMULATE");
            PRMSG(stdout, msg);
            warning_state = 1;
        } else {
#if (HAS_GUI && HAS_MONTE_CARLO)
            if (new_mode == CConst::simulateMode) {
                GCallClass::setPixmap();
            } else if (mode == CConst::simulateMode) {
                GCallClass::deletePixmap();
            }
#endif
            mode = new_mode;
            sprintf(msg, "mode set to %s\n",
                (mode == CConst::editGeomMode) ? "EDIT_GEOMETRY" : "SIMULATE");
            PRMSG(stdout, msg);
            process_mode_change();
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: check_prop_model                                                       ****/
/******************************************************************************************/
int NetworkClass::check_prop_model()
{
    int cell_idx, sector_idx, pm_idx, num_error, num_warning;
    CellClass *cell;
    SectorClass *sector;
    PropModelClass *pm;

    int tot_num_sector = 0;
    int num_no_prop_model = 0;
    num_error = 0;
    num_warning = 0;
    for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if (sector->prop_model == -1) {
                sprintf( msg, "WARNING: CELL %d SECTOR %d has no propagation model, ignored during simulation.\n",
                    cell_idx, sector_idx);
                PRMSG(stdout, msg);
                num_warning++;
                num_no_prop_model++;
            }
            if (sector->antenna_height <= 0.0) {
                sprintf( msg, "ERROR: CELL %d SECTOR %d has antenna height = %15.10f, must be > 0.0.\n",
                    cell_idx, sector_idx, sector->antenna_height);
                PRMSG(stdout, msg);
                num_error++;
            }
            if (sector->tx_pwr < 0.0) {
                sprintf( msg, "ERROR: CELL %d SECTOR %d has TX POWER = %15.10f, must be >= 0.0.\n",
                    cell_idx, sector_idx, sector->tx_pwr);
                PRMSG(stdout, msg);
                num_error++;
            }
            tot_num_sector++;
        }
    }

    if (num_no_prop_model == tot_num_sector) {
        sprintf( msg, "ERROR: ALL cells int the system have unassigned propagation model.\n");
        PRMSG(stdout, msg);
        num_error++;
    }

    for(pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        pm = prop_model_list[pm_idx];
        if (pm->type() == CConst::PropSegment) {
            if (((SegmentPropModelClass *) pm)->num_clutter_type) {
                if (!map_clutter) {
                    sprintf( msg, "ERROR: PROP_MODEL %d \"%s\" uses clutter data, but no clutter map defined\n",
                        pm_idx, pm->get_strid());
                    PRMSG(stdout, msg);
                    num_error++;
                } else if (((SegmentPropModelClass *) pm)->num_clutter_type != map_clutter->num_clutter_type) {
                    sprintf( msg, "ERROR: PROP_MODEL %d \"%s\" uses %d clutter types, "
                                  "but current clutter map has %d clutter types defined.\n",
                        pm_idx, pm->get_strid(),
                        ((SegmentPropModelClass *) pm)->num_clutter_type, map_clutter->num_clutter_type);
                    PRMSG(stdout, msg);
                    num_error++;
                }
            }
        }
    }

    if (num_error) {
        error_state = 1;
    } else if (num_warning) {
        warning_state = 1;
    }

    return(num_error);
}
/******************************************************************************************/
/**** FUNCTION: check_traffic                                                          ****/
/******************************************************************************************/
int NetworkClass::check_traffic()
{
    int tt_idx, num_error;
    TrafficTypeClass *traffic_type;

    num_error = 0;
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        traffic_type = traffic_type_list[tt_idx];
        num_error += traffic_type->check(this);

#if 0
        int subnet_idx, found;
        int xval, yval;
        SubnetClass *subnet;
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];
            found =0;
            for (xval=subnet->minx+1; (xval<=subnet->maxx-1)&&(!found); xval++) {
                for (yval=subnet->miny+1; (yval<=subnet->maxy-1)&&(!found); yval++) {
                    if (subnet->p->in_bdy_area(xval, yval)) {
                        found = 1;
                    }
                }
            }
            if (!found) {
                sprintf(msg, "ERROR: Subnet area contains no simulation grid points\n"
                             "TRAFFIC = %s SUBNET = %s (%d, %d)\n",
                    traffic_type_list[tt_idx]->name(), subnet->strid, tt_idx, subnet_idx);
                PRMSG(stdout, msg);
                num_error++;
            }
        }
#endif
    }

    if (num_error) {
        error_state = 1;
    }

    return(num_error);
}
/******************************************************************************************/

#if 0
MOVED to coverage.cpp
/******************************************************************************************/
/**** FUNCTION: coverage_layer_scan_fn                                                 ****/
/******************************************************************************************/
void coverage_layer_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int idx, cell_idx, sector_idx;
    int n, *scan_list;
    double pwr;
    CellClass *cell;
    SectorClass *sector;
    CoverageClass *cvg = np->scan_cvg;

    scan_list = IVECTOR(np->total_num_sectors);

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {
        n = 0;
        for (idx=0; idx<=np->scan_cell_list->getSize()-1; idx++) {
            cell_idx = (*np->scan_cell_list)[idx];
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y);
                if (pwr > cvg->threshold) {
                    n++;
                }
            }
        }
        np->scan_array[posn_x][posn_y] = (n > cvg->scan_list->getSize()-1 ? cvg->scan_list->getSize()-1 : n) << 8;
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }

    free(scan_list);
}
/******************************************************************************************/
/**** FUNCTION: coverage_sir_layer_scan_fn                                             ****/
/******************************************************************************************/
void coverage_sir_layer_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int i, idx;
    int cell_idx, sector_idx;
    int n, pwr_list_size;
    double pwr, max_pwr, *pwr_list, threshold;
    CellClass *cell;
    SectorClass *sector;
    CoverageClass *cvg = np->scan_cvg;

    pwr_list = DVECTOR(np->total_num_sectors);

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {
        max_pwr = 0.0;
        pwr_list_size = 0;
        for (idx=0; idx<=np->scan_cell_list->getSize()-1; idx++) {
            cell_idx = (*np->scan_cell_list)[idx];
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y);
                pwr_list[pwr_list_size++] = pwr;
                if (pwr > max_pwr) {
                    max_pwr = pwr;
                }
            }
        }

        threshold = max_pwr / cvg->threshold;
        n = -1;
        for (i=0; i<=pwr_list_size-1; i++) {
            if (pwr_list[i] > threshold) {
                n++;
            }
        }
        np->scan_array[posn_x][posn_y] = (n > cvg->scan_list->getSize()-1 ? cvg->scan_list->getSize()-1 : n) << 8;
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }

    free(pwr_list);
}
/******************************************************************************************/
/**** FUNCTION: coverage_level_scan_fn                                                 ****/
/******************************************************************************************/
void coverage_level_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int idx, cell_idx, sector_idx, scan_type, min_i, max_i, new_i;
    CellClass *cell;
    SectorClass *sector;
    double pwr, max_pwr;
    CoverageClass *cvg = np->scan_cvg;

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {
        max_pwr = 0.0;
        for (idx=0; idx<=np->scan_cell_list->getSize()-1; idx++) {
            cell_idx = (*np->scan_cell_list)[idx];
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y);
                if (pwr > max_pwr) {
                    max_pwr = pwr;
                }
            }
        }
        if (max_pwr < cvg->level_list[0]) {
            scan_type = 0;
        } else if (max_pwr >= cvg->level_list[cvg->scan_list->getSize()-2]) {
            scan_type = cvg->scan_list->getSize()-1;
        } else {
            min_i = 0;
            max_i = cvg->scan_list->getSize()-2;
            while (max_i > min_i+1) {
                new_i = (min_i + max_i) / 2;
                if (max_pwr >= cvg->level_list[new_i]) {
                    min_i = new_i;
                } else {
                    max_i = new_i;
                }
            }
            scan_type = max_i;
        }
        np->scan_array[posn_x][posn_y] = scan_type << 8;
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: NetworkClass::adjust_coord_system                                      ****/
/******************************************************************************************/
void NetworkClass::adjust_coord_system(int new_startx, int new_starty, int new_npts_x, int new_npts_y)
{
    int cell_idx, map_layer_idx;
    int adjust_x = system_startx - new_startx;
    int adjust_y = system_starty - new_starty;
    CellClass *cell;

    system_bdy->translate(adjust_x, adjust_y);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        cell->posn_x += adjust_x;
        cell->posn_y += adjust_y;
    }
    for (map_layer_idx=0; map_layer_idx<=map_layer_list->getSize()-1; map_layer_idx++) {
        (*map_layer_list)[map_layer_idx]->shift(adjust_x, adjust_y);
    }

    shift_road_test_data(adjust_x, adjust_y);

    system_startx = new_startx;
    system_starty = new_starty;

    npts_x = new_npts_x;
    npts_y = new_npts_y;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::draw_polygons                                            ****/
/******************************************************************************************/
void NetworkClass::draw_polygons(double scan_fractional_area, ListClass<int> *&scan_idx_list, ListClass<int> *&polygon_list, ListClass<int> *&color_list)
{
    int posn_x, posn_y;
    MeshClass *mesh;

    int max_num_mesh_node = 0;

    for (posn_x=0; posn_x<=npts_x-1; posn_x++) {
        for (posn_y=0; posn_y<=npts_y-1; posn_y++) {
            if (scan_array[posn_x][posn_y] != CConst::NullScan) {
                max_num_mesh_node++;
            }
        }
    }

    mesh = new MeshClass(max_num_mesh_node, scan_array);

    time_t td;

    time(&td); printf("%sBEGIN creating initial Mesh\n\n", ctime(&td));
    /**************************************************************************************/
    /**** Create Initial Mesh                                                          ****/
    /**************************************************************************************/
    mesh->create_initial_mesh(this);
    /**************************************************************************************/
    time(&td); printf("%sDONE creating initial Mesh\n\n", ctime(&td));

    time(&td); printf("%sBEGIN simplifying Mesh\n\n", ctime(&td));
    /**************************************************************************************/
    /**** Simplify mesh                                                                ****/
    /**************************************************************************************/
    mesh->simplify(this, scan_fractional_area);
    /**************************************************************************************/
    time(&td); printf("%sDONE simplifying Mesh\n\n", ctime(&td));

    printf("NUM MESH_NODE = %d\n", mesh->num_mesh_node);

    time(&td); printf("%sBEGIN converting mesh to list of polygons\n\n", ctime(&td));
    /**************************************************************************************/
    /**** Convert mesh to list of polygons                                             ****/
    /**************************************************************************************/
    mesh->convert_to_polygons(polygon_list, color_list, scan_idx_list);
    /**************************************************************************************/
    time(&td); printf("%sEND converting mesh to list of polygons\n\n", ctime(&td));

    delete mesh;

    for (int idx = 0; idx <= scan_idx_list->getSize()-1; idx++) {
        (*scan_idx_list)[idx] = (*scan_idx_list)[idx] >> 8;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass:report_subnets                                           ****/
/******************************************************************************************/
void NetworkClass::report_subnets(int traffic_type_idx, int report_contained_cells, int report_traffic_cs_density, char *filename, int by_segment)
{
    int i, n, first_time, cell_idx, sector_idx, subnet_idx, contained, num_contained_cells;
    double area, total_area, traffic, total_traffic, total_meas_ctr;
    char *chptr, *separator, *name;
    SubnetClass *subnet;
    CellClass *cell;
    SectorClass *sector;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    n = 68;
    if (report_contained_cells) {
        n += 21;
    }

    separator = CVECTOR(n);
    for (i=0; i<=n-1; i++) { separator[i] = '-'; }
    separator[n] = (char) NULL;

    name = CVECTOR(1000);

    double mean_time    = traffic_type_list[traffic_type_idx]->get_mean_time();

    chptr = msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Subnet Report                                                   ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "TRAFFIC_TYPE:  %s\n", traffic_type_list[traffic_type_idx]->get_strid());
    chptr += sprintf(chptr, "MEAN_DURATION: %15.6f\n", mean_time);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "%-20s %15s %15s %15s", "NAME", "TRAFFIC", "AREA", "DENSITY");
    if (report_contained_cells) {
        chptr += sprintf(chptr, " %-20s", "CONTAINED_CELLS");
    }
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "%s\n", separator);
    chptr += sprintf(chptr, "%s\n", separator);
    PRMSG(fp, msg);

    total_traffic = 0.0;
    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
        subnet = subnet_list[traffic_type_idx][subnet_idx];
        total_area = subnet->p->comp_bdy_area()*resolution*resolution;
        for (i=0; i<=(by_segment ? subnet->p->num_segment-1 : 0); i++) {
            if (by_segment) {
                area = PolygonClass::comp_bdy_area(subnet->p->num_bdy_pt[i], subnet->p->bdy_pt_x[i], subnet->p->bdy_pt_y[i])*resolution*resolution;
                sprintf(name, "%s_%d", subnet->strid, i);
            } else {
                area = subnet->p->comp_bdy_area()*resolution*resolution;
                sprintf(name, "%s", subnet->strid);
            }
            traffic = subnet->arrival_rate*mean_time*area/total_area;
            total_traffic += traffic;
            chptr = msg;
            if (area == 0.0) {
                chptr += sprintf(chptr, "%-20s %15.6f %15.6f %15s", name, traffic, area, "UNDEFINED");
            } else {
                chptr += sprintf(chptr, "%-20s %15.6f %15.6f %15.6f", name, traffic, area, traffic/area);
            }
            if (report_contained_cells || report_traffic_cs_density) {
                num_contained_cells = 0;
                total_meas_ctr      = 0.0;
                first_time = 1;
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    if (by_segment) {
                        contained = PolygonClass::in_bdy_area(cell->posn_x, cell->posn_y, subnet->p->num_bdy_pt[i], subnet->p->bdy_pt_x[i], subnet->p->bdy_pt_y[i]);
                    } else {
                        contained = subnet->p->in_bdy_area(cell->posn_x, cell->posn_y);
                    }

                    // CG DBG
                    /*
                    printf( "contained : %d  report_contained_cells : %d \n",
                            contained, report_contained_cells );
                     */

                    if (contained) {
                        if (report_contained_cells) {
                            if (first_time) {
                                first_time = 0;
                            } else {
                                chptr += sprintf(chptr, ",");
                            }
                            chptr += sprintf(chptr, " %s", cell->view_name(cell_idx, preferences->report_cell_name_pref));

                            // CG DBG
                            // printf( "cell txt:  %s \n", cell->view_name(cell_idx, preferences->report_cell_name_pref) );


                        }
                        num_contained_cells++;
                        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                            sector = cell->sector_list[sector_idx];
                            total_meas_ctr += sector->meas_ctr_list[traffic_type_idx];
                        }
                    }
                }
                if (report_traffic_cs_density) {
                    if (area == 0.0) {
                    chptr += sprintf(chptr, " %d %s %s", num_contained_cells, "UNDEFINED", "UNDEFINED");
                    } else {
                    chptr += sprintf(chptr, " %d %15.10f %15.10f", num_contained_cells, num_contained_cells/area, total_meas_ctr/area);
                    }
                }
            }
            chptr += sprintf(chptr, "\n");
            PRMSG(fp, msg);
        }
    }

    area = system_bdy->comp_bdy_area();
    chptr = msg;
    chptr += sprintf(chptr, "%s\n", separator);
    chptr += sprintf(chptr, "%-20s %15.6f %15.6f %15.6f\n", "TOTAL (SYS_BDY)", total_traffic, area, total_traffic/area);
    chptr += sprintf(chptr, "%s\n", separator);
    PRMSG(fp, msg);

    if ( fp != stdout ) {
        fclose(fp);
    }

    free(separator);
    free(name);

    return;
}
/******************************************************************************************/
/**** FUNCTION: inlist                                                                 ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Returns 1 if val in {list[0], list[1], ..., list[n-1]} and 0 otherwise           ****/
int inlist(int *list, int val, int n)
{   int i;

    for (i=0; i<=n-1; i++) {
        if (list[i] == val) {
            return(1);
        }
    }
    return(0);
}
/******************************************************************************************/
/**** FUNCTION: ins_listtwo_elem                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Inserts (vala, valb) into {(a[0],b[0]), (a[1],b[1]) ..., (a[n-1],b[n-1])}        ****/
/**** If (vala,valb) is already in list and err=0 do nothing, else give ERROR and exit ****/
/**** The value returned is the position of val in the list.                           ****/
int ins_listtwo_elem(int *lista, int *listb, int vala, int valb, int *nptr, int err)
{   int i, found, retval;

    found  = 0;
    retval = -1;
    for (i=(*nptr)-1; (i>=0)&&(!found); i--) {
        if ( (lista[i] == vala) && (listb[i] == valb) ) {
            found = 1;
            retval = i;
        }
    }

    if (found) {
        if (err) {
            printf("ERROR in routine ins_listtwo_elem()");
            CORE_DUMP;
            exit(1);
        }
    } else {
        (*nptr)++;
        retval = (*nptr)-1;
        lista[retval] = vala;
        listb[retval] = valb;
    }
    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: del_listtwo_elem                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Inserts (vala, valb) into {(a[0],b[0]), (a[1],b[1]) ..., (a[n-1],b[n-1])}        ****/
/**** If (vala,valb) is already in list and err=0 do nothing, else give ERROR and exit ****/
/**** The value returned is the position of val in the list.                           ****/
void del_listtwo_elem(int *lista, int *listb, int vala, int valb, int *nptr, int err)
{   int i;

    for (i=(*nptr)-1; i>=0; i--) {
        if ( (lista[i] == vala) && (listb[i] == valb) ) {
            lista[i] = lista[(*nptr)-1];
            listb[i] = listb[(*nptr)-1];
            (*nptr)--;
            return;
        }
    }

    if (err) {
        printf("ERROR in routine del_listtwo_elem()");
        CORE_DUMP;
        exit(1);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: inlisttwo                                                              ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Returns 1 if (a,b) in {(x[0],y[0]), (x[1],y[1]), ..., (x[n-1],y[n-1)]} and 0     ****/
/**** otherwise                                                                        ****/
int inlisttwo(int *x, int *y, int a, int b, int n)
{   int i;

    for (i=0; i<=n-1; i++) {
        if ( (x[i] == a) && (y[i] == b) ) {
            return(1);
        }
    }
    return(0);
}
/******************************************************************************************/
/**** FUNCTION: line_segments_cross                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Determines whether or not two line segments cross.                               ****/
/**** LINE1: (x1,y1) ------- (x2,y2)                                                   ****/
/**** LINE2: (x3,y3) ------- (x4,y4)                                                   ****/
int line_segments_cross(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{   int s1, s2, s3, s4;

    s1 = (y3-y1)*(x2-x1) - (x3-x1)*(y2-y1);
    s2 = (y4-y1)*(x2-x1) - (x4-x1)*(y2-y1);
    s3 = (y1-y3)*(x4-x3) - (x1-x3)*(y4-y3);
    s4 = (y2-y3)*(x4-x3) - (x2-x3)*(y4-y3);

    if (    ( ((s1<=0)&&(s2>=0)) || ((s1>=0)&&(s2<=0)) )
         && ( ((s3<=0)&&(s4>=0)) || ((s3>=0)&&(s4<=0)) ) ) {
        return(1);
    }

    return(0);
}
/******************************************************************************************/
/**** FUNCTION: process_group_sector_cmd                                               ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::process_group_sector_cmd(char *sector_list_str, char *gw_csc_cs_list_str, char *name)
{
    int i, n;
    int cell_idx, sector_idx, scan_idx;
    char *chptr, *namestr;
    CellClass *cell;
    ListClass<int> *lc = new ListClass<int>(0);

    if (sector_list_str) {
        extract_sector_list(lc, sector_list_str, CConst::CellIdxRef);
    } else {
        extract_int_list(lc, gw_csc_cs_list_str);
        for (i=lc->getSize()-1; i>=0; i--) {
            uid_to_sector((*lc)[i], cell_idx, sector_idx);
            if (cell_idx == -1) {
                sprintf(msg, "WARNING: GW_CSC_CS = %.6d not found\n", (*lc)[i]);
                PRMSG(stdout, msg); warning_state = 1;
                lc->del_elem_idx(i);
            } else {
                scan_idx = (sector_idx << bit_cell) | cell_idx;
                (*lc)[i] = scan_idx;
            }
        }
    }

    lc->resize();
    lc->sort();

    if (name) {
        namestr = strdup(name);
    } else {
        char tmpstr[50];
        sprintf(tmpstr, "grp_%d", sector_group_list->getSize());
        namestr = strdup(tmpstr);
    }

    uniquify_str(namestr, sector_group_name_list);

    sector_group_list->append((void *) lc);
    sector_group_name_list->append(namestr);

#if 1
    chptr = msg;
    chptr += sprintf(chptr, "NUM SECTOR GROUPS: %d\n", sector_group_list->getSize());
    PRMSG(stdout, msg);
    int num_per_line = 100;
    int num;

    for (n=0; n<=sector_group_list->getSize()-1; n++) {
        chptr = msg;
        chptr += sprintf(chptr, "GROUP %d: \"%s\"", n, (*sector_group_name_list)[n]);
        num = 0;
        lc = (ListClass<int> *) ((*sector_group_list)[n]);
        for (i=0; i<=lc->getSize()-1; i++) {
            cell_idx   = (*lc)[i] & ((1<<bit_cell)-1);
            sector_idx = (*lc)[i] >> bit_cell;
            chptr += sprintf(chptr, " %d_%d", cell_idx, sector_idx);
            num++;
            if (num == num_per_line) {
                chptr += sprintf(chptr, "\n");
                PRMSG(stdout, msg);
                chptr = msg;
                num = 0;
            }
        }
        if (num) {
            chptr += sprintf(chptr, "\n");
            PRMSG(stdout, msg);
        }
    }
#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_sector_list                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_sector_list(ListClass<int> *int_list, char *sector_list_str, int searchField)
{
    int cell_idx, sector_idx, scan_idx;
    int sector_grp, found, i;
    CellClass *cell;
    SectorClass *sector;
    char *str, *str2, *ptr, *chptr;
    ListClass<int> *lc;

#if CDEBUG
    if (bit_cell == -1) {
        CORE_DUMP;
    }
#endif

    int_list->reset();

    if (strcmp(sector_list_str, "all") == 0) {
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            for (sector_idx=0; sector_idx<=cell_list[cell_idx]->num_sector-1; sector_idx++) {
                int_list->append((sector_idx << bit_cell) | cell_idx);
            }
        }
    } else if (strcmp(sector_list_str, "ungrouped") == 0) {
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            for (sector_idx=0; sector_idx<=cell_list[cell_idx]->num_sector-1; sector_idx++) {
                found = 0;
                scan_idx = (sector_idx << bit_cell) | cell_idx;

                for (sector_grp=0; (sector_grp<=sector_group_list->getSize()-1)&&(!found); sector_grp++) {
                    lc = (ListClass<int> *) (*sector_group_list)[sector_grp];
                    for (i=0; (i<=lc->getSize()-1)&&(!found); i++) {
                        if ((*lc)[i] == scan_idx) {
                            found = 1;
                        }
                    }
                }
                if (!found) {
                    int_list->append(scan_idx);
                }
            }
        }
    } else if (searchField == CConst::CellIdxRef) {
        while ( (str = strtok( ((int_list->getSize()==0)?sector_list_str:NULL), CHDELIM)) ) {
            ptr = str;
            while ((*ptr >= '0') && (*ptr <= '9')) {
                ptr++;
            }
            if (*ptr != '_') {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_sector_list()\n");
                chptr += sprintf(chptr, "Expecting '_', read '%c'\n", *ptr);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }

            *ptr = (char) NULL;
            ptr++;
            str2 = ptr;
            while ((*ptr >= '0') && (*ptr <= '9')) {
                ptr++;
            }
            if (*ptr) {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_sector_list()\n");
                chptr += sprintf(chptr, "Expecting NULL char, read '%c'\n", *ptr);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }

            cell_idx   = atoi(str);
            sector_idx = atoi(str2);

            if ( (cell_idx < 0) || (cell_idx > num_cell-1) ) {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_sector_list()\n");
                chptr += sprintf(chptr, "cell_idx = %d out of range [0,%d]\n", cell_idx, num_cell-1);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }

            if ( (sector_idx < 0) || (sector_idx > cell_list[cell_idx]->num_sector-1) ) {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_sector_list()\n");
                chptr += sprintf(chptr, "CELL = %d sector_idx = %d out of range [0,%d]\n",
                        cell_idx, sector_idx, cell_list[cell_idx]->num_sector-1);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }

            int_list->append((sector_idx << bit_cell) | cell_idx);
        }
    } else if (searchField == CConst::CellCSNumberRef) {
        extract_int_list(int_list, sector_list_str);
        for (i=int_list->getSize()-1; i>=0; i--) {
            uid_to_sector((*int_list)[i], cell_idx, sector_idx);
            if (cell_idx == -1) {
                sprintf(msg, "ERROR: GW_CSC_CS = %.6d not found\n", (*int_list)[i]);
                PRMSG(stdout, msg); error_state = 1;
                return;
            } else {
                (*int_list)[i] = (sector_idx << bit_cell) | cell_idx;
            }
        }
    } else if (searchField == CConst::CellHexCSIDRef) {
        unsigned char *csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
        while ( (str = strtok( ((int_list->getSize()==0)?sector_list_str:NULL), CHDELIM)) ) {
            hexstr_to_hex(csid_hex, str, PHSSectorClass::csid_byte_length);
            found = 0;
            for (cell_idx=0; (cell_idx<=num_cell-1)&&(!found); cell_idx++) {
                cell = cell_list[cell_idx];
                for (sector_idx=0; (sector_idx<=cell->num_sector-1)&&(!found); sector_idx++) {
                    sector = cell->sector_list[sector_idx];
                    if (strncmp((char *) ( (PHSSectorClass*) sector)->csid_hex, (char *) csid_hex, PHSSectorClass::csid_byte_length)==0) {
                        found = 1;
                        int_list->append((sector_idx << bit_cell) | cell_idx);
                    }
                }
            }
        }
    } else {
        CORE_DUMP;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_cell_list                                                      ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_cell_list(ListClass<int> *int_list, char *cell_list_str)
{
    int cell_idx, i;
    char *chptr;

    int_list->reset();

    if (strcmp(cell_list_str, "all") == 0) {
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            int_list->append(cell_idx);
        }
    } else {
        extract_int_list(int_list, cell_list_str);

        for (i=0; i<=int_list->getSize()-1; i++) {
            if ( ((*int_list)[i] < 0) || ((*int_list)[i] > num_cell-1) ) {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_cell_list()\n");
                chptr += sprintf(chptr, "cell_idx = %d out of range [0,%d]\n", (*int_list)[i], num_cell-1);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_pm_list                                                        ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_pm_list(ListClass<int> *int_list, char *pm_list_str)
{
    int pm_idx, i;
    char *chptr;

    int_list->reset();

    if (strcmp(pm_list_str, "all") == 0) {
        for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
            int_list->append(pm_idx);
        }
    } else {
        extract_int_list(int_list, pm_list_str);

        for (i=0; i<=int_list->getSize()-1; i++) {
            if ( ((*int_list)[i] < 0) || ((*int_list)[i] > num_prop_model-1) ) {
                chptr = msg;
                chptr += sprintf(chptr, "ERROR in routine extract_pm_list()\n");
                chptr += sprintf(chptr, "pm_idx = %d out of range [0,%d]\n", (*int_list)[i], num_prop_model-1);
                PRMSG(stdout, msg); error_state = 1;
                return;
            }
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_int_list                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_int_list(ListClass<int> *int_list, char *list_str)
{
    int i, ival;
    char *str, *ptr, *chptr;

    int_list->reset();

    while ( (str = strtok( ((int_list->getSize()==0)?list_str:NULL), CHDELIM)) ) {
        ptr = str;
        if ((*ptr == '+') || (*ptr == '-') ) {
            ptr++;
        }
        while ((*ptr >= '0') && (*ptr <= '9')) {
            ptr++;
        }
        if (*ptr) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: Unable to process integer list.  Expecting NULL char, read '%c'\n", *ptr);
            PRMSG(stdout, msg); error_state = 1;
            return;
        }

        ival = atoi(str);

        int_list->append(ival);
    }

    int_list->sort();

    for (i=0; i<=int_list->getSize()-2; i++) {
        if ((*int_list)[i] == (*int_list)[i+1]) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: Value %d specified more than once.\n", (*int_list)[i]);
            PRMSG(stdout, msg); error_state = 1;
            return;
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_double_list                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_double_list(ListClass<double> *dbl_list, char *list_str)
{
    int i;
    double dval;
    char *str, *chptr;

    dbl_list->reset();

    while ( (str = strtok( ((dbl_list->getSize()==0)?list_str:NULL), CHDELIM)) ) {

        dval = atof(str);

        dbl_list->append(dval);
    }

    dbl_list->sort();

    for (i=0; i<=dbl_list->getSize()-1; i++) {
        if ((*dbl_list)[i] == (*dbl_list)[i+1]) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: Value %12.10f specified more than once.\n", (*dbl_list)[i]);
            PRMSG(stdout, msg); error_state = 1;
            return;
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: extract_intint_list                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::extract_intint_list(ListClass<IntIntClass> *ii_list, char *list_str)
{
    int n, ival0, ival1;
    char *str, *ptr, *chptr;

    ii_list->reset();

    n = 0;
    while ( (str = strtok( ((n==0)?list_str:NULL), CHDELIM)) ) {
        ptr = str;
        if ((*ptr == '+') || (*ptr == '-') ) {
            ptr++;
        }
        while ((*ptr >= '0') && (*ptr <= '9')) {
            ptr++;
        }
        if (*ptr) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: Unable to process integer list.  Expecting NULL char, read '%c'\n", *ptr);
            PRMSG(stdout, msg); error_state = 1;
            return;
        }

        if ( (n & 1) == 0 ) {
            ival0 = atoi(str);
        } else {
            ival1 = atoi(str);

            ii_list->append( IntIntClass(ival0, ival1) );
        }
        n++;
    }

    if (n%1) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR: Number of integers specified is %d, value must be even.\n", n);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: get_cell_num                                                           ****/
/**** INPUTS: Cell reference id                                                        ****/
/**** OUTPUTS: Corresponding cell number                                               ****/
/**** If the string passed to this routine is numeric, the string is converted to a    ****/
/**** number and taken to be the cell number.                                          ****/
int get_cell_num(NetworkClass *np, char *str)
{
    int cell_idx, found;
    int cell_num = -1;
    CellClass *cell;

    found = 0;
    if ( (str[0] >= '0') && (str[0] <= '9') ) {
        cell_num = atoi(str);
        if ( (cell_num >= 0) && (cell_num<=np->num_cell-1) ) {
            found = 1;
        }
    } else {
        for (cell_idx=0; (cell_idx<=np->num_cell-1)&&(!found); cell_idx++) {
            cell = np->cell_list[cell_idx];
            if ( (cell->strid) && (strcmp(cell->strid, str)==0) ) {
                found = 1;
                cell_num = cell_idx;
            }
        }
    }

    if (!found) {
        sprintf(np->msg, "ERROR in routine get_cell_num(), String ID = %s not found\n", str);
        PRMSG(stdout, np->msg); np->error_state = 1;
    }

    return(cell_num);
}
/******************************************************************************************/
/**** FUNCTION: get_sector_num                                                         ****/
/**** INPUTS: Cell number and sector reference id                                      ****/
/**** OUTPUTS: Corresponding sector number                                             ****/
/**** If the string passed to this routine is numeric, the string is converted to a    ****/
/**** number and taken to be the sector number.                                        ****/
int get_sector_num(NetworkClass *np, int cell_idx, char *str)
{
    int sector_idx, found;
    int sector_num = -1;
    CellClass *cell;
    SectorClass *sector;

    cell = np->cell_list[cell_idx];

    found = 0;
    if ( (str[0] >= '0') && (str[0] <= '9') ) {
        sector_num = atoi(str);
        if ( (sector_num >= 0) && (sector_num<=cell->num_sector-1) ) {
            found = 1;
        }
    } else {
        for (sector_idx=0; (sector_idx<=cell->num_sector-1)&&(!found); sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if ( (sector->strid) && (strcmp(sector->strid, str)==0) ) {
                found = 1;
                sector_num = sector_idx;
            }
        }
    }

    if (!found) {
        sprintf(np->msg, "ERROR in routine get_sector_num(), Cell [%d] %s String ID = %s not found\n",
            cell_idx, CELL_STRID(cell_idx), str);
        PRMSG(stdout, np->msg); np->error_state = 1;
    }

    return(sector_num);
}
/******************************************************************************************/
/**** FUNCTION: get_pm_idx                                                             ****/
/**** INPUTS: Propagation Model strid                                                  ****/
/**** OUTPUTS: Corresponding index                                                     ****/
int NetworkClass::get_pm_idx(char *str, int error)
{
    int pm_idx, found;
    int pm_num = -1;
    PropModelClass *prop_model;

    found = 0;
    for (pm_idx=0; (pm_idx<=num_prop_model-1)&&(!found); pm_idx++) {
        prop_model = prop_model_list[pm_idx];
        if ( (prop_model->get_strid()) && (strcmp(prop_model->get_strid(), str)==0) ) {
            found = 1;
            pm_num = pm_idx;
        }
    }

    if ( (error) && (!found) ) {
        sprintf(msg, "ERROR in routine get_pm_idx(), Propagation Model String ID = %s not found\n", str);
        PRMSG(stdout, msg); error_state = 1;
    }

    return(pm_num);
}
/******************************************************************************************/
/**** FUNCTION: get_cvg_idx                                                            ****/
/**** INPUTS: Coverage analysis strid                                                  ****/
/**** OUTPUTS: Corresponding index                                                     ****/
int NetworkClass::get_cvg_idx(char *str, int error)
{
    int cvg_idx, found;
    int cvg_num = -1;
    CoverageClass *cvg;

    found = 0;
    for (cvg_idx=0; (cvg_idx<=num_coverage_analysis-1)&&(!found); cvg_idx++) {
        cvg = coverage_list[cvg_idx];
        if ( (cvg->strid) && (strcmp(cvg->strid, str)==0) ) {
            found = 1;
            cvg_num = cvg_idx;
        }
    }

    if ( (error) && (!found) ) {
        sprintf(msg, "ERROR in routine get_cvg_idx(), Coverage Analysis String ID = %s not found\n", str);
        PRMSG(stdout, msg); error_state = 1;
    }

    return(cvg_num);
}
/******************************************************************************************/
/**** FUNCTION: get_traffic_type_idx                                                   ****/
/**** INPUTS: Traffic Type strid                                                       ****/
/**** OUTPUTS: Corresponding traffic type index                                        ****/
int NetworkClass::get_traffic_type_idx(char *str, int error)
{
    int traffic_type_idx, found;
    int n = -1;
    TrafficTypeClass *traffic_type;

    found = 0;
    for (traffic_type_idx=0; (traffic_type_idx<=num_traffic_type-1)&&(!found); traffic_type_idx++) {
        traffic_type = traffic_type_list[traffic_type_idx];
        if ( (traffic_type->get_strid()) && (strcmp(traffic_type->get_strid(), str)==0) ) {
            found = 1;
            n = traffic_type_idx;
        }
    }

    if ( (error) && (!found) ) {
        sprintf(msg, "ERROR in routine get_traffic_type_idx(), Traffic Type String ID = %s not found\n", str);
        PRMSG(stdout, msg); error_state = 1;
    }

    return(n);
}
/******************************************************************************************/
/**** FUNCTION: get_subnet_idx                                                         ****/
/**** INPUTS: Subnet strid                                                             ****/
/**** OUTPUTS: Corresponding subnet index                                              ****/
int NetworkClass::get_subnet_idx(char *str, int traffic_type_idx, int error)
{
    int subnet_idx, found;
    int subnet_num = -1;
    SubnetClass *subnet;

    found = 0;
    for (subnet_idx=0; (subnet_idx<=num_subnet[traffic_type_idx]-1)&&(!found); subnet_idx++) {
        subnet = subnet_list[traffic_type_idx][subnet_idx];
        if ( (subnet->strid) && (strcmp(subnet->strid, str)==0) ) {
            found = 1;
            subnet_num = subnet_idx;
        }
    }

    if ( (error) && (!found) ) {
        sprintf(msg, "ERROR in routine get_subnet_idx(), Subnet String ID = %s not found\n", str);
        PRMSG(stdout, msg); error_state = 1;
    }

    return(subnet_num);
}
/******************************************************************************************/
/**** FUNCTION: add_polygon                                                            ****/
void NetworkClass::add_polygon(char *type_str, int traffic_type_idx, ListClass<IntIntClass> *ii_list)
{
    int i, tmp;
    int minx, maxx, miny, maxy;
    int cell_idx, map_layer_idx, cvg_idx, tt_idx, rtd_idx, pm_idx;
    CellClass *cell;
    PropModelClass *pm;
    char tmpstr[100];
    SubnetClass *subnet;

    if (PolygonClass::comp_bdy_area(ii_list) < 0.0) {
        ii_list->reverse();
    }

    if (strcmp(type_str, "subnet")==0) {

        num_subnet[traffic_type_idx]++;
        subnet_list[traffic_type_idx] = (SubnetClass **) realloc( (void *) (subnet_list[traffic_type_idx]), (num_subnet[traffic_type_idx])*sizeof(SubnetClass *));
        subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1] = new SubnetClass();
        subnet = subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1];

        subnet->p = new PolygonClass();
        subnet->p->num_segment = 1;
        subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
        subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
        subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

        subnet->strid = NULL;
        subnet->arrival_rate = 0.0;

        subnet->p->num_bdy_pt[0] = ii_list->getSize();
        subnet->p->bdy_pt_x[0] = IVECTOR(ii_list->getSize());
        subnet->p->bdy_pt_y[0] = IVECTOR(ii_list->getSize());

        for (i=0; i<=ii_list->getSize()-1; i++) {
            subnet->p->bdy_pt_x[0][i] = (*ii_list)[i].x();
            subnet->p->bdy_pt_y[0][i] = (*ii_list)[i].y();
        }

        subnet->p->comp_bdy_min_max(subnet->minx, subnet->maxx, subnet->miny, subnet->maxy);
        // subnet->color = default_color_list[(num_subnet[traffic_type_idx]-1) % num_default_color];
        subnet->color = hot_color->get_color(num_subnet[traffic_type_idx]-1, num_subnet[traffic_type_idx]);

        sprintf(tmpstr, "subnet_%d", num_subnet[traffic_type_idx]-1 );
        subnet->strid = strdup(tmpstr);
    } else if (strcmp(type_str, "system_bdy")==0) {
        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
            if (num_subnet[tt_idx]) {
                sprintf(msg, "ERROR: Unable to redefine system boundary for geometry that has subnets\n");
                PRMSG(stdout, msg);
                error_state = 1;
                return;
            }
        }
        system_bdy->num_bdy_pt[0] = ii_list->getSize();
        system_bdy->bdy_pt_x[0] = (int *) realloc((void *) system_bdy->bdy_pt_x[0], ii_list->getSize()*sizeof(int));
        system_bdy->bdy_pt_y[0] = (int *) realloc((void *) system_bdy->bdy_pt_y[0], ii_list->getSize()*sizeof(int));
        for (i=0; i<=ii_list->getSize()-1; i++) {
            system_bdy->bdy_pt_x[0][i] = (*ii_list)[i].x();
            system_bdy->bdy_pt_y[0][i] = (*ii_list)[i].y();
        }

        system_bdy->comp_bdy_min_max(minx, maxx, miny, maxy);
        system_startx += minx;
        system_starty += miny;
        npts_x = maxx - minx + 1;
        npts_y = maxy - miny+ 1;

        for (i=0; i<=ii_list->getSize()-1; i++) {
            system_bdy->bdy_pt_x[0][i] -= minx;
            system_bdy->bdy_pt_y[0][i] -= miny;
        }
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            cell->posn_x -= minx;
            cell->posn_y -= miny;
        }
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            (*road_test_data_list)[rtd_idx].posn_x -= minx;
            (*road_test_data_list)[rtd_idx].posn_y -= miny;
        }
        for (map_layer_idx=0; map_layer_idx<=map_layer_list->getSize()-1; map_layer_idx++) {
            (*map_layer_list)[map_layer_idx]->shift(-minx, -miny);
        }
        for (cvg_idx=0; cvg_idx<=num_coverage_analysis-1; cvg_idx++) {
            coverage_list[cvg_idx]->shift(-minx, -miny);
        }
        for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
            pm = prop_model_list[pm_idx];
            if (pm->is_clutter_model()) {
                ((GenericClutterPropModelClass *) pm)->offset_x -= minx;
                ((GenericClutterPropModelClass *) pm)->offset_y -= miny;
            }
        }

        if (map_clutter) {
            map_clutter->translate(-minx, -miny);
        }

        if (map_height) {
            map_height->translate(-minx, -miny);
        }

#if HAS_GUI
        if (map_background) {
            map_background->shift(-minx, -miny);
        }
#endif
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: convert_utm                                                            ****/
/******************************************************************************************/
void NetworkClass::convert_utm(double r, double utm_a, double utm_e, int utm_z, int utm_n)
{
    int bdy_pt_idx, cell_idx, traffic_type_idx;
    int new_system_startx, new_system_starty, maxx, maxy;
    int n;
    double lon_deg, lat_deg, utm_x, utm_y;
    char *chptr;
    CellClass *cell;

    int possible = 1;
    if (    (map_clutter)   || (map_height)
         || (map_layer_list->getSize()) || (num_coverage_analysis)
#if HAS_GUI
         || (map_background)
#endif
       ) {
        possible = 0;
    }

    for (traffic_type_idx=0; (traffic_type_idx<=num_traffic_type-1)&&(possible); traffic_type_idx++) {
        if (num_subnet[traffic_type_idx]) {
            possible = 0;
        }
    }

    if (road_test_data_list->getSize()) {
        possible = 0;
    }

    if (!possible) {
        sprintf(msg, "ERROR: Unable to convert to UTM coordinates\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (coordinate_system != CConst::CoordLONLAT) {
        sprintf(msg, "ERROR: conversion not yet supported\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    utm_equatorial_radius = utm_a;
    utm_eccentricity_sq   = utm_e;
    utm_zone              = utm_z;
    utm_north             = utm_n;

    if (r==0.0) {
        /**********************************************************************************/
        /**** Automatically determine resolution                                       ****/
        /**********************************************************************************/
        lon_deg = idx_to_x(system_bdy->bdy_pt_x[0][0]);
        lat_deg = idx_to_y(system_bdy->bdy_pt_y[0][0]);
        LLtoUTM( lon_deg, lat_deg, utm_x, utm_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
        double min_utm_x = utm_x;
        double max_utm_x = utm_x;
        double min_utm_y = utm_y;
        double max_utm_y = utm_y;
        for (bdy_pt_idx=1; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
            lon_deg = idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]);
            lat_deg = idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]);
            LLtoUTM( lon_deg, lat_deg, utm_x, utm_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);

            if (utm_x < min_utm_x) {
                min_utm_x = utm_x;
            } else if (utm_x > max_utm_x) {
                max_utm_x = utm_x;
            }
            if (utm_y < min_utm_y) {
                min_utm_y = utm_y;
            } else if (utm_y > max_utm_y) {
                max_utm_y = utm_y;
            }
        }
#if 0
        chptr = msg;
        chptr += sprintf(chptr, "UTM MIN_X: %15.10f MAX_X: %15.10f DX: %15.10f\n", min_utm_x, max_utm_x, max_utm_x-min_utm_x);
        chptr += sprintf(chptr, "UTM MIN_Y: %15.10f MAX_Y: %15.10f DY: %15.10f\n", min_utm_y, max_utm_y, max_utm_y-min_utm_y);
        PRMSG(stdout, msg);
#endif
        r = sqrt( (max_utm_x-min_utm_x)*(max_utm_y-min_utm_y) / 1.0e8 );
        if (r <= 1.0) {
            r = 1.0;
        } else {
            n = (int) ceil( log(r) / log(2.0) );
            r = (double) (1 << n);
        }
        /**********************************************************************************/
    }

    chptr = msg;
    chptr += sprintf(chptr, "Converting Geometry to UTM\n");
    chptr += sprintf(chptr, "Equatorial Radius: %15.10f\n", utm_a);
    chptr += sprintf(chptr, "Eccentricity Squared: %15.10f\n", utm_e);
    chptr += sprintf(chptr, "Zone: %d%c\n", utm_z, (utm_n ? 'N' : 'S'));
    chptr += sprintf(chptr, "Resolution: %15.10f\n", r);
    PRMSG(stdout, msg);

    /**************************************************************************************/
    /**** Convert System Boundary                                                      ****/
    /**************************************************************************************/
    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        lon_deg = idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]);
        lat_deg = idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]);
        LLtoUTM( lon_deg, lat_deg, utm_x, utm_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
        check_grid_val(utm_x, r, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx]);
        check_grid_val(utm_y, r, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx]);
    }

    system_bdy->comp_bdy_min_max(new_system_startx, maxx, new_system_starty, maxy);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        system_bdy->bdy_pt_x[0][bdy_pt_idx] -= new_system_startx;
        system_bdy->bdy_pt_y[0][bdy_pt_idx] -= new_system_starty;
    }

    /**************************************************************************************/

    /**************************************************************************************/
    /**** Convert Cell Positions                                                       ****/
    /**************************************************************************************/
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        lon_deg = idx_to_x(cell->posn_x);
        lat_deg = idx_to_y(cell->posn_y);
        LLtoUTM( lon_deg, lat_deg, utm_x, utm_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
        check_grid_val(utm_x, r, new_system_startx, &cell->posn_x);
        check_grid_val(utm_y, r, new_system_starty, &cell->posn_y);
    }
    /**************************************************************************************/

    coordinate_system = CConst::CoordUTM;
    system_startx = new_system_startx;
    system_starty = new_system_starty;
    resolution    = r;
    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
    npts_x = maxx - system_startx + 1;
    npts_y = maxy - system_starty + 1;

    return;
}
/******************************************************************************************/
/**** FUNCTION: flip_lon                                                               ****/
/******************************************************************************************/
void NetworkClass::flip_lon()
{
    int bdy_pt_idx, cell_idx, traffic_type_idx, rtd_idx, map_layer_idx, polygon_idx, line_idx;
    int segment_idx, pt_idx, text_idx;
    int new_system_startx, new_system_starty, maxx, maxy;
    int i, n;
    double lon_deg, lat_deg, utm_x, utm_y;
    char *chptr;
    CellClass *cell;
    RoadTestPtClass *rtp;
    MapLayerClass *ml;
    PolygonClass *polygon;
    LineClass *line;

    int possible = 1;
    if (    (map_clutter)   || (map_height)
         || (map_layer_list->getSize()) || (num_coverage_analysis)
#if HAS_GUI
         || (map_background)
#endif
       ) {
        possible = 0;
    }

    for (traffic_type_idx=0; (traffic_type_idx<=num_traffic_type-1)&&(possible); traffic_type_idx++) {
        if (num_subnet[traffic_type_idx]) {
            possible = 0;
        }
    }

    if (!possible) {
        sprintf(msg, "ERROR: Unable to flip LON\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (coordinate_system != CConst::CoordUTM) {
        sprintf(msg, "ERROR: conversion not yet supported\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "Flip Longitude\n");
    chptr += sprintf(chptr, "Zone: %d%c\n", utm_zone, (utm_north ? 'N' : 'S'));
    PRMSG(stdout, msg);

    utm_zone = 61 - utm_zone;

    chptr = msg;
    chptr += sprintf(chptr, "Zone: %d%c\n", utm_zone, (utm_north ? 'N' : 'S'));
    PRMSG(stdout, msg);

    /**************************************************************************************/
    /**** System Boundary                                                              ****/
    /**************************************************************************************/
    for (pt_idx=0; pt_idx<=system_bdy->num_bdy_pt[0]-1; pt_idx++) {
        utm_x = 1.0e6 - idx_to_x(system_bdy->bdy_pt_x[0][pt_idx]);
        check_grid_val(utm_x, resolution, 0, &system_bdy->bdy_pt_x[0][pt_idx]);
    }

    system_bdy->comp_bdy_min_max(new_system_startx, maxx, new_system_starty, maxy);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        system_bdy->bdy_pt_x[0][bdy_pt_idx] -= new_system_startx;
        system_bdy->bdy_pt_y[0][bdy_pt_idx] -= new_system_starty;
    }
    system_bdy->reverse();
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Cell Positions                                                               ****/
    /**************************************************************************************/
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        utm_x = 1.0e6 - idx_to_x(cell->posn_x);
        check_grid_val(utm_x, resolution, new_system_startx, &cell->posn_x);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Road Test Data                                                               ****/
    /**************************************************************************************/
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        utm_x = 1.0e6 - idx_to_x(rtp->posn_x);
        check_grid_val(utm_x, resolution, new_system_startx, &rtp->posn_x);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Map Layers                                                                   ****/
    /**************************************************************************************/
    for (map_layer_idx=0; map_layer_idx<=map_layer_list->getSize()-1; map_layer_idx++) {
        ml = (*map_layer_list)[map_layer_idx];
        for (polygon_idx=0; polygon_idx<=ml->num_polygon-1; polygon_idx++) {
            polygon = ml->polygon_list[polygon_idx];
            for (segment_idx=0; segment_idx<=polygon->num_segment-1; segment_idx++) {
                n = polygon->num_bdy_pt[segment_idx];
                for (i=0; i<=n-1; i++) {
                    utm_x = 1.0e6 - idx_to_x(polygon->bdy_pt_x[segment_idx][i]);
                    check_grid_val(utm_x, resolution, new_system_startx, &(polygon->bdy_pt_x[segment_idx][i]));
                }
            }
            polygon->reverse();
        }

        for (line_idx=0; line_idx<=ml->num_line-1; line_idx++) {
            line = ml->line_list[line_idx];
            for (i=0; i<=line->num_pt-1; i++) {
                utm_x = 1.0e6 - idx_to_x(line->pt_x[i]);
                check_grid_val(utm_x, resolution, new_system_startx, &(line->pt_x[i]));
            }
        }

        for (text_idx=0; text_idx<=ml->num_text-1; text_idx++) {
            utm_x = 1.0e6 - idx_to_x(ml->posn_x[text_idx]);
            check_grid_val(utm_x, resolution, new_system_startx, &(ml->posn_x[text_idx]));
        }

        ml->modified = 1;
    }
    /**************************************************************************************/

    system_startx = new_system_startx;

    return;
}
/******************************************************************************************/
/**** FUNCTION: hex_to_hexstr                                                          ****/
/******************************************************************************************/
int hex_to_hexstr(char *hexstr, unsigned char *hex, unsigned int byte_len)
{
    unsigned int i;

    static char tbl[] = "0123456789ABCDEF";

    if (hex) {
        for (i=0; i<=byte_len-1; i++) {
            hexstr[2*i  ] = tbl[(hex[i]>>4) & 0x0F];
            hexstr[2*i+1] = tbl[ hex[i]     & 0x0F];
        }
    } else {
        for (i=0; i<=2*byte_len-1; i++) {
            hexstr[i] = 'U';
        }
    }
    hexstr[2*byte_len] = (char) NULL;

    return(1);
}
/******************************************************************************************/
/**** FUNCTION: hexstr_to_hex                                                          ****/
/**** Return value: 1 if successfull, 0 if not successful                              ****/
/******************************************************************************************/
int hexstr_to_hex(unsigned char *retstr, char *str, unsigned int byte_len)
{
    unsigned int i;
    int n1, n2;
    char errmsg[MAX_LINE_SIZE], *chptr;

    if (strlen(str) < 2*byte_len) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine hexstr_to_hex()\n");
        chptr += sprintf(chptr, "HEX STRING: \"%s\" must contain at least %d bytes\n", str, byte_len);
        PRMSG(stdout, errmsg);
        return(0);
    }

    for (i=0; i<=byte_len-1; i++) {
        n1 = read_hex_char(str[2*i]);
        if (n1 == -1) { return(0); }
        n2 = read_hex_char(str[2*i+1]);
        if (n2 == -1) { return(0); }
        retstr[i] = ( (n1 << 4) | n2 ) & 255;
    }

    return(1);
}
/******************************************************************************************/
int read_hex_char(char hexchar)
{
    int retval;
    char errmsg[MAX_LINE_SIZE], *chptr;

    if ( (hexchar >= '0') && (hexchar <= '9') ) {
        retval = hexchar - '0';
    } else if ( (hexchar >= 'A') && (hexchar <= 'F') ) {
        retval = hexchar - 'A' + 10;
    } else if ( (hexchar >= 'a') && (hexchar <= 'f') ) {
        retval = hexchar - 'a' + 10;
    } else {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine read_hex_char()\n");
        chptr += sprintf(chptr, "INVALID Hex character: \'%c\'\n", hexchar);
        PRMSG(stdout, errmsg);
        retval = -1;
    }
    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: get_strid                                                              ****/
char *get_strid(char *str)
{
    char *chptr = str;
    char *start_posn = NULL;
    char *stop_posn = NULL;
    char *retstr;

    if (chptr) {
        while ( (*chptr) && (*chptr != '\"') ) { chptr++; }
        if (*chptr) { chptr++; start_posn = chptr; }

        while ( (*chptr) && (*chptr != '\"') ) { chptr++; }
        if (*chptr) { stop_posn = chptr; }

        if ( (start_posn) && (stop_posn) ) {
            *stop_posn = (char) NULL;
            retstr = strdup(start_posn);
        } else {
            retstr = NULL;
        }
    } else {
        retstr = NULL;
    }

    return(retstr);
}
/******************************************************************************************/
/**** FUNCTION: SectorClass::SectorClass                                               ****/
/******************************************************************************************/
SectorClass::SectorClass(CellClass *cell) : parent_cell(cell)
{
    if (num_traffic) {
        meas_ctr_list = DVECTOR(num_traffic);
    } else {
        meas_ctr_list = (double *) NULL;
    }
    set_default_parameters();
}
/******************************************************************************************/
/**** FUNCTION: SectorClass::set_default_parameters                                    ****/
/******************************************************************************************/
void SectorClass::set_default_parameters()
{
    int tti_idx;

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    strid             = (char *) NULL;
    comment           = (char *) NULL;
    antenna_height    = 22.0;
    tx_pwr            = 500.0;
    antenna_angle_rad = 0.0;
    antenna_type      = 0;
    prop_model        = -1;
    for (tti_idx=0; tti_idx<=num_traffic-1; tti_idx++) {
        meas_ctr_list[tti_idx] = 0.0;
    }
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    call_list         = (ListClass<void *> *) NULL;
    stat_count        = (StatCountClass *) NULL;
    active            = 1;

    GUI_FN(vis_rtd    = 0x00);
    /**************************************************************************************/

    return;
}
/******************************************************************************************/
/**** FUNCTION: SectorClass::duplicate                                                 ****/
/******************************************************************************************/
SectorClass *SectorClass::duplicate(int)
{
    SectorClass *new_sector = new SectorClass((CellClass *) NULL);

    copy_sector_values(new_sector);

    return(new_sector);
}
/******************************************************************************************/
/**** FUNCTION: SectorClass::copy_sector_values                                        ****/
/******************************************************************************************/
void SectorClass::copy_sector_values(SectorClass *new_sector)
{
    int tti_idx;

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    new_sector->strid             = (char *) NULL;
    new_sector->comment           = (char *) NULL;
    new_sector->antenna_type      = antenna_type;
    new_sector->antenna_height    = antenna_height;
    new_sector->tx_pwr            = tx_pwr;
    new_sector->antenna_angle_rad = antenna_angle_rad;
    new_sector->prop_model        = prop_model;

    for (tti_idx=0; tti_idx<=num_traffic-1; tti_idx++) {
        new_sector->meas_ctr_list[tti_idx] = meas_ctr_list[tti_idx];
    }
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    new_sector->call_list  = (ListClass<void *> *) NULL;
    new_sector->stat_count = (StatCountClass *) NULL;
    /**************************************************************************************/
}
/******************************************************************************************/
/**** FUNCTION: CellClass::duplicate                                                   ****/
/******************************************************************************************/
CellClass *CellClass::duplicate(const int x, const int y, int copy_csid)
{
    CellClass *new_cell = new CellClass();

    copy_cell_values(new_cell, x, y, copy_csid);

    return(new_cell);
}
/******************************************************************************************/
/**** FUNCTION: CellClass::copy_cell_values                                            ****/
/******************************************************************************************/
void CellClass::copy_cell_values(CellClass *new_cell, const int x, const int y, int copy_csid)
{
    int sector_idx;

    new_cell->posn_x = x;
    new_cell->posn_y = y;

    new_cell->color       = color;
    new_cell->num_sector  = num_sector;
    new_cell->sector_list = (SectorClass **) malloc(num_sector*sizeof(SectorClass *));

    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        new_cell->sector_list[sector_idx] = sector_list[sector_idx]->duplicate(copy_csid);
        new_cell->sector_list[sector_idx]->parent_cell = new_cell;
    }

    return;
}
/******************************************************************************************/
char *NetworkClass::technology_str()
{
    char *str;
    static char      phs_str[] = "PHS";
    static char    wcdma_str[] = "WCDMA";
    static char cdma2000_str[] = "CDMA2000";
    static char     wlan_str[] = "WLAN";

    switch( technology() ) {
        case CConst::PHS:      str =      phs_str; break;
        case CConst::WCDMA:    str =    wcdma_str; break;
        case CConst::CDMA2000: str = cdma2000_str; break;
        case CConst::WLAN:     str =     wlan_str; break;
        default: CORE_DUMP; break;
    }

    return(str);
}
/******************************************************************************************/

#if 0
Moved to global_fn.cpp
/******************************************************************************************/
/**** FUNCTION: uniquify_str                                                           ****/
/**** INPUTS: str, n, strlist                                                          ****/
/**** OUTPUTS: possibly modified str                                                   ****/
/**** If str is in strlist, append either "(2)", "(3)" ... to make str unique          ****/
void uniquify_str(char *&str, ListClass<char *> *strlist)
{
    int i, found, num = 1;
    char *tmpstr = CVECTOR(strlen(str) + 2 + 40);

    strcpy(tmpstr, str);

    do {
        found = 0;
        for (i=0; (i<=strlist->getSize()-1)&&(!found); i++) {
            if (strcmp(tmpstr, (*strlist)[i])==0) {
                found = 1;
            }
        }
        if (found) {
            num++;
            sprintf(tmpstr, "%s(%d)", str, num);
        }
    } while (found);

    if (num != 1) {
        free(str);
        str = strdup(tmpstr);
    }

    free(tmpstr);
}
/******************************************************************************************/
/**** FUNCTION: set_current_dir_from_file                                              ****/
/**** INPUTS: filename                                                                 ****/
/**** OUTPUTS:                                                                         ****/
/**** Set current working directory to that of the specified filename.                 ****/
void set_current_dir_from_file(char *filename)
{
    char *path = strdup(filename);
    int n = strlen(filename);
    int found = 0;
    int i;

    for (i=n-1; (i>=0)&&(!found); i--) {
        if ( (path[i] == '\\') || (path[i] == '/') ) {
            found = 1;
            path[i+1] = (char) NULL;
        }
    }

    if (found) {
        if (chdir(path) != 0) {
#if CDEBUG
            CORE_DUMP;
#endif
        }
    }

    free(path);
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: get_pwr_unit_offset                                                    ****/
/**** Returns offset to convert power value to dBm.                                    ****/
double get_pwr_unit_offset(int pwr_unit)
{
    double offset;

    switch(pwr_unit) {
        case CConst::PWRdBm:      offset = 0.0;                          break;
        case CConst::PWRdBW:      offset = 30.0;                         break;
        case CConst::PWRdBuV_107: offset = -110.0+10*log(2.0)/log(10.0); break;
        case CConst::PWRdBuV_113: offset = -110.0-10*log(2.0)/log(10.0); break;
        default: CORE_DUMP; break;
    }

    return(offset);
}
/******************************************************************************************/
/**** FUNCTION: pwr_name_to_unit                                                       ****/
/**** Returns offset to convert power value to dBm.                                    ****/
int pwr_name_to_unit(char *pwr_name)
{
    int pwr_unit;

         if (strcmp(pwr_name, "dBm"     )==0) { pwr_unit = CConst::PWRdBm;      }
    else if (strcmp(pwr_name, "dBW"     )==0) { pwr_unit = CConst::PWRdBW;      }
    else if (strcmp(pwr_name, "dBuV_107")==0) { pwr_unit = CConst::PWRdBuV_107; }
    else if (strcmp(pwr_name, "dBuV_113")==0) { pwr_unit = CConst::PWRdBuV_113; }
    else {
        pwr_unit = -1;
    }

    return(pwr_unit);
}
/******************************************************************************************/
/**** FUNCTION: pwr_unit_to_name                                                       ****/
/**** Returns offset to convert power value to dBm.                                    ****/
void pwr_unit_to_name(int pwr_unit, char *pwr_name)
{
    switch(pwr_unit) {
        case CConst::PWRdBm:      sprintf(pwr_name, "dBm");      break;
        case CConst::PWRdBW:      sprintf(pwr_name, "dBW");      break;
        case CConst::PWRdBuV_107: sprintf(pwr_name, "dBuV_107"); break;
        case CConst::PWRdBuV_113: sprintf(pwr_name, "dBuV_113"); break;
        default: CORE_DUMP; break;
    }

    return;
}
/******************************************************************************************/
