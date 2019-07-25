/******************************************************************************************/
/**** FILE: WiSim.h                                                                  ****/
/**** Michael Mandell 1/15/02                                                          ****/
/******************************************************************************************/

#include <QString>
//Added by qt3to4:
#include "global_defines.h"
#include "charstr.h"

#ifndef WISIM_H
#define WISIM_H

#include <stdio.h>
#include <stdlib.h>

#ifndef __linux__
// #include <wtypes.h>
#endif

#ifndef HAS_GUI
#define HAS_GUI                      1 /* Whether or not to compile with GUI              */
#endif

#ifndef HAS_ORACLE
#define HAS_ORACLE                   1 /* Whether or not to compile with ORACLE database  */
#endif

#ifndef DEMO
#define DEMO                         0 /* Whether or not to compile DEMO version          */
#endif

#ifndef HAS_MONTE_CARLO
#define HAS_MONTE_CARLO              1 /* Whether or not to compile with monte-carlo      */
#endif

#ifndef HAS_CLUTTER
#define HAS_CLUTTER                  1 /* Whether or not to compile with clutter          */
#endif

#define WISIM_RELEASE              "190626" /* WISIM_RELEASE version, ex "040531" */

#ifndef NO_PWLINEAR
#define NO_PWLINEAR                  0 /* Whether or not to include piecewise linear model*/
#endif

#ifndef CDEBUG
#define CDEBUG                       1 /* Whether or not to compile in debug mode         */
                                       /* Note that DEBUG is used by Qt, so CDEBUG is     */
                                       /* used to avoid conflict.                         */
#endif

#ifndef TR_EMBED
#define TR_EMBED                     1 /* Whether or not to use embeded Qt translation.   */
                                       /* Embeded translation is compiled into executable.*/
                                       /* If not embeded, qm file read at run time.       */
#endif

#define MAX_SIR                 100000 /* Max SIR to use when there are no interferers    */
#define MAX_SEED             999999999 /* Max value for ran3() seed (MBIG - 1)            */
#define BLOCKED_PHYS_CHAN           -1 /* Call blocked due to lack of physical channel    */
#define BLOCKED_SIR_CS              -2 /* Call blocked due to SIR below threshold at CS   */
#define BLOCKED_SIR_PS              -3 /* Call blocked due to SIR below threshold at PS   */
#define MAX_PTS                   1000 /* Used in routine read_two_col()                  */
#define MAX_NUM_MAP_LAYER           25 /* Max number of map layers                        */

#define REUSE_CHAN_SECTOR            0 /* Whether or not a channel can be used in         */
                                       /* different sectors of the same cell at the same  */
                                       /* time.                                           */

#define DEBUG_DRAW_POLYGON           1 /* Debug functions for drawing polygons            */
#define DEBUG_COMP_PROP_MODEL        1 /* Debug functions for computing prop_model        */

#define VERSION        "2.8.0"
/******************************************************************************************/

/******************************************************************************************/
/**** Define preprocessor constants                                                    ****/
/******************************************************************************************/

// xxxx move to cconst.h

#define LEVEL_PRIORITY     22
#define SLOT_PRIORITY      23
#define FORCED             24
#define NOT_FORCED         25

#define INVALID_METRIC     0x3FFFFFFF

/******************************************************************************************/

#define TECHNOLOGY_NAME(nnn) ((nnn) == CConst::PHS   ? "phs"   : \
                              (nnn) == CConst::WCDMA ? "wcdma" : \
                              (nnn) == CConst::WLAN  ? "wlan"  : \
                              "" )

#define CELL_STRID(xxx)   (np->cell_list[xxx]->strid ? np->cell_list[xxx]->strid : "")

#define SECTOR_STRID(xxx, yyy) \
    (np->cell_list[xxx]->sector_list[yyy]->strid ? np->cell_list[xxx]->sector_list[yyy]->strid : "")

#if HAS_GUI
#define GUI_FN(xxx) if (use_gui) { xxx; } 
#define PRMSG(fff, xxx) print_gui_message(fff, xxx)
#else
#define GUI_FN(xxx)
#define PRMSG(fff, xxx) fprintf(fff, "%s", xxx)
#endif

#if HAS_ORACLE
#define DB_FN(xxx) xxx 
#else
#define DB_FN(xxx)
#endif

class CellClass;
class CoverageClass;
class HotColorClass;
class IntIntClass;
class MapClutterClass;
class MapHeightClass;
class MapLayerClass;
class NetworkClass;
class PHSNetworkClass;
class PolygonClass;
class PosnScanClass;
class PrefClass;
class RoadTestPtClass;
class StatClass;
class StatCountClass;
class StrIntClass;
class TrafficTypeClass;
class TRandomMersenne;

template<class T> class ListClass;

#if HAS_GUI
    class MapBackgroundClass;
    class ProgressSlot;
    class QBitmap;
#endif

#if HAS_MONTE_CARLO
    class MatchDataClass;
#endif

/******************************************************************************************/
/**** Define data structures                                                           ****/
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: EventClass                                                                ****/
/******************************************************************************************/
class EventClass
{
public:
    EventClass();
    ~EventClass();

    void copy(EventClass *, double abs_time);

    int event_type, traffic_type_idx;

    double time;
    int posn_x, posn_y;

    int cs_idx;
    int master_idx;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CallClass                                                                 ****/
/******************************************************************************************/
class CallClass
{
public:
    int posn_x, posn_y;
    int call_idx, cell_idx, sector_idx, master_idx;
    int traffic_type_idx;

#if 1
    int channel;
#endif
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: SectorClass                                                               ****/
/******************************************************************************************/
class SectorClass
{
public:
    SectorClass(CellClass *cell);
    virtual ~SectorClass();
    virtual SectorClass *duplicate(int copy_csid);
    virtual void set_default_parameters();
    virtual int get_ival(int param_type);
    virtual void copy_sector_values(SectorClass *new_sector);
    virtual double st_comp_arrival_rate(int traffic_idx);

    double comp_sir_cs(NetworkClass *np, int x, int y, int channel, double *ptr_interference);
    double comp_prop(NetworkClass *np, int x, int y, int max_dist = 0);

    CellClass *parent_cell;

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    char *strid;
    char *comment;
    int antenna_type;
    double antenna_height, tx_pwr;
    double antenna_angle_rad;
    int prop_model;

    static int num_traffic;
    static int *traffic_type_idx_list;
    double *meas_ctr_list;
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    ListClass<void *> *call_list; /* list of ptr's to CallClass objects                */
    int road_test_pt_color;
    StatCountClass *stat_count;
    int active;

#if HAS_GUI
    char vis_rtd;
#endif
    /**********************************************************************************/
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CellClass                                                                 ****/
/******************************************************************************************/
class CellClass
{
public:
    CellClass();
    virtual ~CellClass();
    virtual CellClass *duplicate(const int x, const int y, int copy_csid);
    void copy_cell_values(CellClass *new_cell, const int x, const int y, int copy_csid);

    virtual char *view_name(int cell_idx, int cell_name_pref);

    int posn_x, posn_y;
    int num_sector;
    SectorClass **sector_list;
    char *strid;
    int color;
    int bm_idx;
    static int text_color;
    static int num_bm;

#if 0
// xxxxxxxxx move to class Cell
    static int num_sizes;
    static int size_list[];
    static QBitmap **bm_list;
    static QBitmap **selected_bm_list;
    static void set_bitmaps(int size_idx);
    static void clear_bitmaps();
#endif
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: LineClass                                                                 ****/
/******************************************************************************************/
class LineClass
{
public:
    LineClass();
    ~LineClass();
    void translate(int x, int y);

    int num_pt;
    int *pt_x, *pt_y;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: SubnetClass                                                               ****/
/******************************************************************************************/
class SubnetClass
{
public:
    SubnetClass();
    ~SubnetClass();
    char *strid;
    double arrival_rate;
    int minx, maxx, miny, maxy;
    PolygonClass *p;
    int color;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CchRssiTableClass                                                         ****/
/******************************************************************************************/
class CchRssiTableClass
{
public:
    int table_size;
    int *sector_rx;
    int *sector_tx;
    double *rssi;  /* the RSSI received at sector_rx from sector_tx */
};
/******************************************************************************************/

class PropModelClass;
class AntennaClass;


/******************************************************************************************/
/**** CLASS: NetworkClass                                                              ****/
/******************************************************************************************/
class NetworkClass
{
public:
    NetworkClass();
    virtual ~NetworkClass();
    virtual const int technology();
    char *technology_str();
    void check_technology(const char *param_name, int valid_tech);
    int process_command(char *line);
    void read_road_test_data(char *filename, char *force_fmt);
    void read_road_test_data_1_1(FILE *fp, char *line, char *filename, int linenum);
    void read_road_test_data_1_2(FILE *fp, char *line, char *filename, int linenum, int file_size, int bytes_read);
    void shift_road_test_data(double x, double y);
    void shift_road_test_data(int delta_x, int delta_y);
    void save_road_test_data(char *filename);
    void close_geometry();
    void run_coverage(int cvg_idx);
    void check_file(char *filename);
    int read_clutter_model(char *filename, int force_read);
    int read_clutter_model_1_0(FILE *fp, char *filename, int force_read);

    void create_subnets(double scan_fractional_area, int init_sample_res, ListClass<int> *ml_list, int num_max, int has_threshold, double threshold_db, double dmax, int closest);
    void draw_polygons(double scan_fractional_area, ListClass<int> *&scan_idx_list, ListClass<int> *&polygon_list, ListClass<int> *&color_list);
    void prune_grp(int, int *, int *);
    void add_polygon(char *type_str, int traffic_type_idx, ListClass<IntIntClass> *ii_list);
    int in_map_layer(int map_layer_idx, int posn_x, int posn_y);
    virtual void process_mode_change();
    void process_group_sector_cmd(char *sector_list_str, char *gw_csc_cs_list_str, char *name);

    void extract_sector_list(   ListClass<int>         *int_list, char *sector_list_str, int searchField);
    void extract_cell_list(     ListClass<int>         *int_list, char *cell_list_str);
    void extract_pm_list(       ListClass<int>         *int_list, char *pm_list_str);
    void extract_int_list(      ListClass<int>         *int_list, char *list_str);
    void extract_double_list(   ListClass<double>      *dbl_list, char *list_str);
    void extract_intint_list(   ListClass<IntIntClass> *ii_list,  char *list_str);

    void get_path_clutter(int x0, int y0, int x1, int y1, double *dist);
    void comp_prop_model(int num_scan_list, int *scan_list, int useheight, int adjust_angles, double min_logd_threshold, const int err, const int cont_on_err);
    void comp_prop_model_with_theta(int num_scan_list, int *scan_list, int useheight, int adjust_angles, const int err);
    void comp_prop_model_segment_angle(int num_scan_list, int *scan_list, int useheight, int adjust_angles, const int err, const int cont_on_err);
    void filt_road_test_data(ListClass<int> *scan_index_list, double angle_deg, double width_deg);
    void filter_rtd(ListClass<IntIntClass> *ii_lis);
    void comp_prop_error(char *filename);
    void set_unassigned_prop_model();
    void redefine_system_bdy(double threshold_db, double scan_fractional_area, int init_sample_resolution);
    void switch_mode(int new_mode);
    void report_subnets(int traffic_type_idx, int report_contained_cells, int report_traffic_cs_density, char *filename, int by_segment);
    void convert_polygons_subnets(ListClass<int> *scan_idx_list, ListClass<int> *polygon_list, ListClass<int> *color_list);
    void convert_utm(double r, double utm_a, double utm_e, int utm_zone, int utm_north);
    void flip_lon();
    void adjust_coord_system(int new_startx, int new_starty, int new_npts_x, int new_npts_y);
    void import_st_data(int format, char *cscfile, char *filename, int period);
    void delete_st_data();

    int get_traffic_type_idx(char *str, int error);
    int get_subnet_idx(char *str, int traffic_type_idx, int error);
    int get_cvg_idx(char *str, int error);
    int get_pm_idx(char *str, int error);

    void gen_clutter( ListClass<int> *scan_index_list, int useheight, int scope, int type, double clutter_slope, double clutter_intercept, int clutter_sim_res_ratio );
    void gen_clutter( ListClass<int> *scan_index_list, int useheight, int type, double clutter_slope, double clutter_intercept, int clutter_sim_res_ratio );

    void report_prop_model_param(int param, int model, char *filename);
    void delete_cell(int *ptr_s, int n); //// xxxxxxxxxxxxxxxxxxxxx DELETE
    void delete_cell(ListClass<int> *int_list);
    void delete_prop_model(ListClass<int> *int_list);

#if HAS_ORACLE
    void quit_db();
#endif

#if HAS_MONTE_CARLO
    void run_simulation(int, int, double);
    void gen_event_location(EventClass *event, int subnet_idx);
    void update_stats(EventClass *);
    void plot_crr_cdf(char *);
    void assign_sector(int *, int *, EventClass *, int);
    double comp_sir_ps(int, int, int, int, int, double *ptr_interference);
    void print_call_status(FILE *fp);
    void insert_pending_event(EventClass *);
    void reset_system();
    void reset_call_statistics(int allocate);
    void read_match_data(char *filename, char *force_fmt, char *checkPointFile);
    void read_match_data_1_1(FILE *fp, char *line, char *filename, int linenum);
    void set_match_data(int it, int rt);
    void run_match();
    void setParam(char *paramName, double paramValue);

    virtual void gen_event(EventClass *);
    virtual void update_network(EventClass *);
    virtual void reset_base_stations(int);
    virtual void print_call_statistics(char *filename, char *sector_list_str, int format);
    virtual StatCountClass * create_call_stat_count();
#endif

    double idx_to_x(int x);
    double idx_to_y(int y);
    void read_geometry(char *, char *, char *);
    void define_geometry(char *coordinate_system_str, char *system_bdy_str);

    // void read_cch_rssi_table(char *filename);
    // void write_cch_rssi_table(char *filename);
    int check_prop_model();

    virtual void print_geometry(char *);
#if HAS_ORACLE
    virtual void read_geometry_db(char *);
    virtual void print_geometry_db();
#endif

    virtual void set_default_parameters();
    virtual void display_settings(FILE *fp);
    virtual void uid_to_sector(char *uid, int &cell_idx, int &sector_idx);
    virtual void uid_to_sector(int   uid, int &cell_idx, int &sector_idx);
    virtual int  sector_to_uid(char *uid, int cell_idx, int sector_idx);
    virtual int Sum_st(int st_idx);
    virtual char * pa_to_str(int pa);

    FILE *input_stream;
    FILE *fl;
    int mode;                /* MODE_EDIT, MODE_SIMULATE                                  */
    const char **opt_msg;
    int error_state;
    int warning_state;
    int max_sync_level;
    double cch_allocation_threshold_db,        cch_allocation_threshold;
    double sync_level_allocation_threshold_db, sync_level_allocation_threshold;
    int cch_allocation_mode;
    int cch_selection_mode;
    CchRssiTableClass *cch_rssi_table;

    int coordinate_system, utm_zone, utm_north;
    double utm_equatorial_radius, utm_eccentricity_sq;

    int num_cell;            /* number of cells in the network                            */
    CellClass **cell_list;

    int *num_subnet;
    SubnetClass ***subnet_list;

    double avg_cell_radius, rec_avg_cell_radius;
    int bit_cell;

    int *num_call_type;      /* total number of calls of traffic_type in the network      */
    ListClass<void *> *master_call_list; /* master list of all calls in the network,      */
                                         /* list of ptr's to CallClass objects            */
    double abs_time;
    int seed;
    TRandomMersenne *rg;

    double resolution;       /* simulation grid spacing (meters)                          */
    int system_startx;       /* min x-coord (meters) divided by resolution                */
    int system_starty;       /* min y-coord (meters) divided by resolution                */
    double frequency;        /* RF frequency in Hz                                        */
    int npts_x, npts_y;
    int res_digits;

    PolygonClass *system_bdy;
    static int system_bdy_color;

    int num_antenna_type;
    AntennaClass **antenna_type_list;
    int num_prop_model;
    PropModelClass **prop_model_list;

    StatClass *stat;
    StatCountClass *stat_count;
    int num_pending_event, alloc_size_pending_event_list;
    EventClass **pending_event_list;
    int memoryless_ps;
    int total_num_sectors;
    
#if HAS_MONTE_CARLO
    MatchDataClass *matchData;
#endif

    ListClass<int> *excl_ml_list;
    int subnet_num_max, *subnet_idx_list;
    double *subnet_pwr_list;
    double *subnet_dsq_list;

    int **scan_array;

    ListClass<void *> *sector_group_list;
    ListClass<char *> *sector_group_name_list;

    ListClass<MapLayerClass *> *map_layer_list;

    MapClutterClass *map_clutter;
    MapHeightClass *map_height;

    int scan_has_threshold;
    double scan_threshold;
    int scan_max_dist;
    ListClass<int> *scan_cell_list;
    PosnScanClass *posn_scan;

    int num_freq;            /* total number of frequencies avail in the network          */

    /**************************************************************************************/
    /**** Coverage Analysis                                                            ****/
    /**************************************************************************************/
    int num_coverage_analysis;
    CoverageClass **coverage_list, *scan_cvg;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Traffic Types                                                                ****/
    /**************************************************************************************/
    int num_traffic_type;
    TrafficTypeClass **traffic_type_list;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Road Test Data                                                               ****/
    /**************************************************************************************/
    ListClass<RoadTestPtClass> *road_test_data_list;
    /**************************************************************************************/
    double add_rtd_pt_threshold;

    ListClass<StrIntClass> *report_cell_name_opt_list;

    char *prompt;
    char *line_buf;
    char *msg;

    // xxxxxx int num_default_color, *default_color_list;

    HotColorClass *hot_color;

    PrefClass *preferences;

    double total_arrival_rate;

#if HAS_GUI
    ProgressSlot *prog_bar;
    MapBackgroundClass *map_background;
    void check_road_test_data();
#endif

protected:
    virtual int check_parameters();
    int subnet_naming_convention;

private:
    int check_traffic();
    void set_total_arrival_rate(int traffic_type_idx, double total_arrival_rate);
    void delete_all_unused_prop_model();
    void readCscFile(char *cscfile, ListClass<CharStrClass> *ipStrList, ListClass<int> *gwCscList);
    void readSTDataFile(char *filename, int format, int period, ListClass<CharStrClass> *ipStrList, ListClass<int> *gwCscList);

    void parse_command(char *command_ptr, const char *kwstrptr, char **cmd_optlist);
    void require_param(char *command, const char *param_name, char *param_value);
    void check_valid_mode(char *command, int valid_modes);
    void check_param_value(int& param_int_value, char *command, const char *param_name, char *param_value,
        const int min_value, const int max_value, const int default_value);
    void check_param_value(double& param_double_value, char *command, const char *param_name, char *param_value,
        const double min_value, const double max_value, const double default_value);
    void check_param_value(int& param_int_value, char *command, const char *param_name, char *param_value,
        const char **valid_value_list, const int default_value);
};

/******************************************************************************************/
/**** Function Declarations                                                            ****/
/******************************************************************************************/
// void scan_geometry(NetworkClass *);

void scan_area(NetworkClass *np, int scan_res, void (*scan_fn)(NetworkClass *, int, int), int xmin, int xmax, int ymin, int ymax );
void subnet_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void subnet_closest_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void subnet_num_max_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void coverage_layer_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void coverage_sir_layer_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void coverage_level_scan_fn(NetworkClass *np, int posn_x, int posn_y);
void coverage_pa_scan_fn(NetworkClass *np, int posn_x, int posn_y);

int spline_init_lut(char *flname, int tabsize, double *gain);
int spline_init_lut(double *&f_phs, double *&f_gain, int numrows, int tabsize, double *gain);
double spline_eval_lut(double, double *, int);

int inlist(int *, int, int);
int  ins_listtwo_elem(int *, int *, int, int, int *, int);
void del_listtwo_elem(int *, int *, int, int, int *, int);
int  inlisttwo(int *, int *, int, int, int);
int num_ptr_triangle(int, int);
int line_segments_cross(int, int, int, int, int, int, int, int);
int hexstr_to_hex(unsigned char *retstr, char *str, unsigned int byte_len);
int hex_to_hexstr(char *hexstr, unsigned char *hex, unsigned int byte_len);
int read_hex_char(char hexchar);
char *get_strid(char *str);
void set_options(int argc, char **argv);
int get_next_command(FILE *input_stream, char *command);
void proc_xy_list(int &n, double *x, double *y, char *str, int &allocation_size, int &error_state);
void sort2z(int n, int *arr, int *brr);
void sort2z(int n, double *arr, int *brr);
void isortz(int n, int *arr);
void dsortz(int n, double *arr);
int intdivfloor(int m, int n);
int intdivceil( int m, int n);
int gcd(int a, int b);
int check_grid_val(double val, double res, int offset, int *gdir_val_ptr);
// double ran3(int *, int reset);
int fgetline(FILE *, char *);
int get_file_size(char *);
int read_two_col(double *, double *, int *, char *);
QString get_environment_variable(char *varname);

#if HAS_GUI
void print_gui_message(FILE *fp, char *line);
#endif

void clear_memory(CoverageClass *cvg, int flag);
void clear_memory(CchRssiTableClass *tbl);

void get_road_test_pt_posn(RoadTestPtClass *road_test_pt, int &posn_x, int &posn_y);
double sqerr(double x, double y);
void uniquify_str(char *&str, ListClass<char *> *strlist);
void set_current_dir_from_file(char *);
double get_pwr_unit_offset(int pwr_unit);
int pwr_name_to_unit(char *pwr_name);
void pwr_unit_to_name(int pwr_unit, char *pwr_name);
/******************************************************************************************/
/**** END: Function Declarations                                                       ****/
/******************************************************************************************/

#endif
