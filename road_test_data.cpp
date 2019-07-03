/*
******************************************************************
* FILE: road_test_data.cpp
*
******************************************************************
*/
#include <math.h>
#include <string.h>
#include <time.h>

#include "antenna.h"
#include "cconst.h"
#include "WiSim.h"
#include "clutter_data_analysis.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "phs.h"
#include "polygon.h"
#include "prop_model.h"
#include "road_test_data.h"
#include "utm_conversion.h"

#if HAS_GUI
#include <qapplication.h>
#include "main_window.h"
#include "progress_slot.h"
extern int use_gui;
extern MainWindowClass *main_window;
#endif

#define ROAD_TEST_DATA_FORMAT "1.2"
const double MIN_TEST_POWER = 26.0;

/******************************************************************************************/
/**** FUNCTION: RoadTestPtClass::RoadTestPtClass                                       ****/
/******************************************************************************************/
RoadTestPtClass::RoadTestPtClass(int x, int y, double pwr, int c_idx, int s_idx)
{
    posn_x     = x;
    posn_y     = y;
    pwr_db     = pwr;
    cell_idx   = c_idx;
    sector_idx = s_idx;
}
/******************************************************************************************/
/**** FUNCTION: RoadTestPtClass::~RoadTestPtClass                                      ****/
/******************************************************************************************/
RoadTestPtClass::~RoadTestPtClass()
{
}
/******************************************************************************************/
/**** FUNCTION: RoadTestPtClass::operator==                                            ****/
/******************************************************************************************/
int RoadTestPtClass::operator==(RoadTestPtClass& val) {
    if ((val.pwr_db == pwr_db) && (val.cell_idx == cell_idx) && (val.sector_idx == sector_idx)
                               && (val.posn_x   == posn_x  ) && (val.posn_y     == posn_y    )) {
        return(1);
    } else {
        return(0);
    }
}
/******************************************************************************************/
/**** FUNCTION: RoadTestPtClass::operator>                                             ****/
/******************************************************************************************/
int RoadTestPtClass::operator>(RoadTestPtClass& val)
{
    int retval;

    if (sort_type == CConst::byPwrSort) {
        if (      (pwr_db >  val.pwr_db)
             || ( (pwr_db == val.pwr_db) && (cell_idx > val.cell_idx) )
             || ( (pwr_db == val.pwr_db) && (cell_idx == val.cell_idx) && (sector_idx > val.sector_idx) )
             || ( (pwr_db == val.pwr_db) && (cell_idx == val.cell_idx) && (sector_idx == val.sector_idx) && (posn_x > val.posn_x) )
             || ( (pwr_db == val.pwr_db) && (cell_idx == val.cell_idx) && (sector_idx == val.sector_idx) && (posn_x == val.posn_x) && (posn_y > val.posn_y) )
           ) {
            retval = 1;
        } else {
            retval = 0;
        }
    } else if (sort_type == CConst::bySectorSort) {
        if (      (cell_idx > val.cell_idx)
             || ( (cell_idx == val.cell_idx) && (sector_idx > val.sector_idx) )
             || ( (cell_idx == val.cell_idx) && (sector_idx == val.sector_idx) && (pwr_db > val.pwr_db) )
             || ( (cell_idx == val.cell_idx) && (sector_idx == val.sector_idx) && (pwr_db == val.pwr_db) && (posn_x > val.posn_x) )
             || ( (cell_idx == val.cell_idx) && (sector_idx == val.sector_idx) && (pwr_db == val.pwr_db) && (posn_x == val.posn_x) && (posn_y > val.posn_y) )
           ) {
            retval = 1;
        } else {
            retval = 0;
        }
    } else {
        CORE_DUMP;
    }

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: RoadTestPtClass::operator<<                                            ****/
/******************************************************************************************/
std::ostream& operator<<(std::ostream& s, RoadTestPtClass& val) {
    s << "(" << val.pwr_db << "," << val.cell_idx << "," << val.sector_idx << "," << val.posn_x << "," << val.posn_y << ")";
    return(s);
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::read_road_test_data                                      ****/
/**** Open specified file and read road test data.                                     ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void NetworkClass::read_road_test_data(char *filename, char *force_fmt)
{
    int linenum, file_size, bytes_read, num_bytes;
    char *str1;
    char *format_str;
    FILE *fp;

    if (num_cell) {
        BITWIDTH(bit_cell, num_cell-1);
    } else {
        sprintf(msg, "WARNING: num_cell = 0 no data read from file \"%s\"\n", filename);
        PRMSG(stdout, msg); warning_state = 1;
        return;
    }

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file 8 %s\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    file_size = get_file_size(filename);
    sprintf(msg, "Reading road_test_data file: %s (TOTAL %d bytes)\n", filename, file_size);
    PRMSG(stdout, msg);

    enum state_enum {
        STATE_FORMAT,
        STATE_READ_VERSION
    };

    state_enum state;

    state = STATE_FORMAT;
    linenum = 0;
    bytes_read = 0;

    if (!force_fmt) {
    while ( (state != STATE_READ_VERSION) && (num_bytes = fgetline(fp, line)) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        bytes_read+=num_bytes;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_FORMAT:
                    if (strcmp(str1, "FORMAT:") != 0) {
                        sprintf(msg, "WARNING: road test data file \"%s\"\n"
                                         "Assuming format 1.1\n", filename);
                        PRMSG(stdout, msg); warning_state = 1;

                        fclose(fp);
                        if ( !(fp = fopen(filename, "rb")) ) {
                            sprintf(msg, "ERROR: cannot open road test data file %s\n", filename);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        format_str = strdup("1.1");
                        state = STATE_READ_VERSION;
                    } else {
                        str1 = strtok(NULL, CHDELIM);
                        format_str = strdup(str1);
                        state = STATE_READ_VERSION;
                    }
                    break;
                default:
                    sprintf(msg, "ERROR: invalid road test data file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_READ_VERSION) {
        sprintf(msg, "ERROR: invalid road test data file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }
    } else {
        format_str = strdup(force_fmt);
    }

    if (strcmp(format_str,"1.1")==0) {
        ((PHSNetworkClass *) this)->read_road_test_data_1_1(fp, line, filename, linenum);
    } else if (strcmp(format_str,"1.2")==0) {
        ((PHSNetworkClass *) this)->read_road_test_data_1_2(fp, line, filename, linenum, file_size, bytes_read);
    } else {
        sprintf(msg, "ERROR: road test data file \"%s\" has invalid format \"%s\"\n", filename, format_str);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    if (error_state == 1) { return; }


    free(line);
    free(format_str);

    fclose(fp);

    return;
}
/******************************************************************************************/
/*
******************************************************************
* FUNCTION: read_road_test_data_1_1
* Open specified file and read road test data.
* Blank lines are ignored.
* Lines beginning with "#" are treated as comments.
******************************************************************
*/
void NetworkClass::read_road_test_data_1_1(FILE *fp, char *line, char *filename, int linenum)
{
#if (DEMO == 0)
    int cell_idx                  = -1;
    int sector_idx                = -1;
    int rt_idx                    = -1;
    int num_road_test_pt          = -1;
    int found_outside_bdy         = 0;
    int rtd_coordinate_system;
    double rtd_utm_equatorial_radius;
    double rtd_utm_eccentricity_sq;
    int rtd_utm_zone;
    int rtd_utm_north;
    int posn_x, posn_y;
    SectorClass *sector           = (SectorClass *) NULL;
    char *str1, str[1000];
    double rtd_u, rtd_v, geom_x, geom_y;
    double pwr_db, pwr_unit_offset;

    pwr_unit_offset = get_pwr_unit_offset(CConst::PWRdBuV_113);

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_CSID,
        STATE_NUM_ROAD_TEST_PT,
        STATE_ROAD_TEST_PT
    };

    state_enum state;

    /**************************************************************************************/
    /**** state:                                                                       ****/
    /****     STATE_COORDINATE_SYSTEM        Looking for COORDINATE_SYSTEM:            ****/
    /****     STATE_CSID:                    Looking for CSID:                         ****/
    /****     STATE_NUM_ROAD_TEST_PT:        Looking for NUM_ROAD_TEST_PT:             ****/
    /****     STATE_ROAD_TEST_PT:            Looking for ROAD_TEST_PT_i:               ****/
    /**************************************************************************************/

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            rtd_coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            rtd_coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            rtd_utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            rtd_utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            rtd_utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                rtd_utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                rtd_utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strncmp(str1, "LON_LAT", 7)==0) {
                            rtd_coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Unrecognized \"COORDINATE_SYSTEM:\" specified: %s\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    if ( (rtd_coordinate_system != coordinate_system) &&
                            ( (rtd_coordinate_system == CConst::CoordGeneric) || (coordinate_system == CConst::CoordGeneric) ) ) {
                        sprintf(msg, "ERROR: Cannot convert coordinates from %s to %s\n",
                            ( (rtd_coordinate_system == CConst::CoordGeneric) ? "GENERIC" :
                              (rtd_coordinate_system == CConst::CoordUTM)     ? "UTM"     :
                              (rtd_coordinate_system == CConst::CoordLONLAT)  ? "LON_LAT" : "UNKNOWN" ),
                            ( (    coordinate_system == CConst::CoordGeneric) ? "GENERIC" :
                              (    coordinate_system == CConst::CoordUTM)     ? "UTM"     :
                              (    coordinate_system == CConst::CoordLONLAT)  ? "LON_LAT" : "UNKNOWN" ));
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else if ( (rtd_coordinate_system == CConst::CoordUTM) && (coordinate_system == CConst::CoordUTM) )  {
                        if (
                                (rtd_utm_equatorial_radius != utm_equatorial_radius)
                             || (rtd_utm_eccentricity_sq != utm_eccentricity_sq)
                             || (rtd_utm_zone != utm_zone)
                             || (rtd_utm_north != utm_north) ) {
                            sprintf(msg, "ERROR: UTM coordinate system parameters not matched\n");
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }
                    state = STATE_CSID;
                    break;
                case STATE_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (!str1) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n", filename);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                         "CSID: %s has improper length\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    uid_to_sector(str1, cell_idx, sector_idx);

                    if (cell_idx != -1) {
                        sector = cell_list[cell_idx]->sector_list[sector_idx];
                    } else {
                        sector = (SectorClass *) NULL;
                    }

                    state = STATE_NUM_ROAD_TEST_PT;
                    break;
                case STATE_NUM_ROAD_TEST_PT:
                    if (strcmp(str1, "NUM_ROAD_TEST_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ROAD_TEST_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);

                    if (str1) {
                        num_road_test_pt = atoi(str1);
                    } else {
                        num_road_test_pt = -1;
                    }

                    if (num_road_test_pt < 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                         "num_road_test_pt = %d must be >= 0\n",
                                filename, num_road_test_pt);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (num_road_test_pt) {
                        rt_idx = 0;
                        state = STATE_ROAD_TEST_PT;
                    } else {
                        state = STATE_CSID;
                    }
                    break;
                case STATE_ROAD_TEST_PT:
                    sprintf(str, "ROAD_TEST_PT_%d:", rt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (sector) {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            rtd_u = atof(str1);
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                             "Road test point %d has no posn_x specified\n",
                                    filename, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            rtd_v = atof(str1);
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                             "Road test point %d has no posn_y specified\n",
                                    filename, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        if ( (rtd_coordinate_system == CConst::CoordLONLAT) && (coordinate_system == CConst::CoordUTM) ) {
                            LLtoUTM( rtd_u, rtd_v,
                                     geom_x,  geom_y,
                                     utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
                        } else if ( (rtd_coordinate_system == coordinate_system) ) {
                            geom_x = rtd_u;
                            geom_y = rtd_v;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                             "Error encountered in coordinate conversion\n",
                                    filename);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        check_grid_val(geom_x, resolution, system_startx, &posn_x);
                        check_grid_val(geom_y, resolution, system_starty, &posn_y);

                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            pwr_db = atof(str1);
                            pwr_db += pwr_unit_offset;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                             "Road test point %d has no power specified\n",
                                    filename, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        road_test_data_list->append( RoadTestPtClass(posn_x, posn_y, pwr_db, cell_idx, sector_idx) );

                        if ( (!found_outside_bdy) && (!system_bdy->in_bdy_area(posn_x, posn_y)) ) {
                            sprintf(msg, "WARNING: road_test_data file \"%s\" contains points outside the system boundary area\n"
                                             "Cell %d Sector %d Road_test_pt %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                                    filename, cell_idx, sector_idx, rt_idx, idx_to_x(posn_x), idx_to_y(posn_y));
                            PRMSG(stdout, msg); warning_state = 1;
                            found_outside_bdy = 1;
                        }
                    }

                    rt_idx++;
                    if (rt_idx >= num_road_test_pt) {
                        state = STATE_CSID;
                    }
                    break;
                default:
                    sprintf(msg, "ERROR: invalid road_test_data file \"%s\"\n"
                                     "Invalid state (%d) encountered\n", filename, state);
                    PRMSG(stdout, msg); error_state = 1;
                    return;
                    break;
            }
        }
    }

    RoadTestPtClass::sort_type = CConst::byPwrSort;
    road_test_data_list->sort();

#endif
    return;
}
/*
******************************************************************
* FUNCTION: read_road_test_data_1_2
* Open specified file and read road test data.
* Blank lines are ignored.
* Lines beginning with "#" are treated as comments.
******************************************************************
*/
void NetworkClass::read_road_test_data_1_2(FILE *fp, char *line, char *filename, int linenum, int file_size, int bytes_read)
{
#if (DEMO == 0)
    int cell_idx                  = -1;
    int sector_idx                = -1;
    int rt_idx                    = -1;
    int num_road_test_pt          = -1;
    int found_outside_bdy         = 0;
    int num_bytes;
    int pwr_unit;
    int rtd_coordinate_system;
    double rtd_utm_equatorial_radius;
    double rtd_utm_eccentricity_sq;
    int rtd_utm_zone;
    int rtd_utm_north;
    int posn_x, posn_y;
    SectorClass *sector           = (SectorClass *) NULL;
    char *str1, str[1000], *chptr;
    double rtd_u, rtd_v, geom_x, geom_y;
    double pwr_db, pwr_unit_offset;
    time_t td;

    int i;
    int curr_prog;
    int byte_num_thr = 0;

    extern char* WiSim_home;
    
    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_PWR_MEAS_UNIT,
        STATE_CSID,
        STATE_NUM_ROAD_TEST_PT,
        STATE_ROAD_TEST_PT
    };

    state_enum state;

    /**************************************************************************************/
    /**** state:                                                                       ****/
    /****     STATE_COORDINATE_SYSTEM        Looking for COORDINATE_SYSTEM:            ****/
    /****     STATE_CSID:                    Looking for CSID:                         ****/
    /****     STATE_NUM_ROAD_TEST_PT:        Looking for NUM_ROAD_TEST_PT:             ****/
    /****     STATE_ROAD_TEST_PT:            Looking for ROAD_TEST_PT_i:               ****/
    /**************************************************************************************/

    GUI_FN(prog_bar = new ProgressSlot(0, "Progress Bar", qApp->translate("ProgressSlot", "Reading Road Test Data") + " ..."));

    state = STATE_COORDINATE_SYSTEM;

    //open log file to print both the csid in cell table but not in the RT data and the reverse.
    //added by liutao, 060802
    char *line_indoor = CVECTOR(MAX_LINE_SIZE);
    char *chptr1;
    FILE* fp_csid;
    if ( NULL == (fp_csid = fopen("csid_no_exist.log","w+")) )
        return;

    fprintf( fp_csid, "List of the csid in RT data but not in the cell table:\n");

    ListClass<char *> *no_csid = new ListClass<char *>(0);

    //read indoor cs list
    ListClass<char* > *indoor_list = new ListClass<char *>(0);
    ListClass<int > *no_csid_indoor = new ListClass<int>(0);
    
    FILE* fp_indoor;
    char filename_indoor[200];
    sprintf(filename_indoor, "%s%c%s", WiSim_home, FPATH_SEPARATOR, "IndoorList.txt");
    if ( NULL == (fp_indoor = fopen(filename_indoor,"r")) ) {
        sprintf(msg, "WARNING: no indoor file found");
        PRMSG(stdout, msg);  
    }
    else {
	      while ( fgetline(fp_indoor, line_indoor) ) {
	          if(line_indoor && line_indoor!="") {
                chptr1 = strdup(line_indoor);
    	          indoor_list->append(chptr1);
    	       }
	       }
	  }    
	    
    //end. liutao, 060802

    while ( (num_bytes = fgetline(fp, line)) ) {
        linenum++;
        bytes_read+=num_bytes;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            rtd_coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            rtd_coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            rtd_utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            rtd_utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            rtd_utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                rtd_utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                rtd_utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strncmp(str1, "LON_LAT", 7)==0) {
                            rtd_coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Unrecognized \"COORDINATE_SYSTEM:\" specified: %s\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    if ( (rtd_coordinate_system != coordinate_system) &&
                            ( (rtd_coordinate_system == CConst::CoordGeneric) || (coordinate_system == CConst::CoordGeneric) ) ) {
                        sprintf(msg, "ERROR: Cannot convert coordinates from %s to %s\n",
                            ( (rtd_coordinate_system == CConst::CoordGeneric) ? "GENERIC" :
                              (rtd_coordinate_system == CConst::CoordUTM)     ? "UTM"     :
                              (rtd_coordinate_system == CConst::CoordLONLAT)  ? "LON_LAT" : "UNKNOWN" ),
                            ( (    coordinate_system == CConst::CoordGeneric) ? "GENERIC" :
                              (    coordinate_system == CConst::CoordUTM)     ? "UTM"     :
                              (    coordinate_system == CConst::CoordLONLAT)  ? "LON_LAT" : "UNKNOWN" ));
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else if ( (rtd_coordinate_system == CConst::CoordUTM) && (coordinate_system == CConst::CoordUTM) )  {
                        if (
                                (rtd_utm_equatorial_radius != utm_equatorial_radius)
                             || (rtd_utm_eccentricity_sq != utm_eccentricity_sq)
                             || (rtd_utm_zone != utm_zone)
                             || (rtd_utm_north != utm_north) ) {
                            sprintf(msg, "ERROR: UTM coordinate system parameters not matched\n");
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }
                    state = STATE_PWR_MEAS_UNIT;
                    break;
                case STATE_PWR_MEAS_UNIT:
                    if (strcmp(str1, "PWR_MEAS_UNIT:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"PWR_MEAS_UNIT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (!str1) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\", no pwr unit specified\n", filename, linenum);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    pwr_unit = pwr_name_to_unit(str1);

                    if (pwr_unit == -1) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\", unrecognized power unit \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    pwr_unit_offset = get_pwr_unit_offset(pwr_unit);

                    state = STATE_CSID;
                    break;
                case STATE_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (!str1) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\", no CSID value specified\n", filename, linenum);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "CSID: %s has improper length\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    uid_to_sector(str1, cell_idx, sector_idx);

                    if (cell_idx != -1) {
                        sector = cell_list[cell_idx]->sector_list[sector_idx];
                    } else {
                        sector = (SectorClass *) NULL;
                        //print those cs not in the cell_list. Added by liutao, 060802
                        fprintf(fp_csid, str1);
                        fprintf(fp_csid, "\n");
                        chptr = strdup(str1);
                        no_csid->append(chptr);
                            
                        int found = 0;
                        if (fp_indoor) {
                            for( int i=0; i<indoor_list->getSize(); i++ ) {
                                    unsigned char *csid_hex1 = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                                    unsigned char *csid_hex2 = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                                    hexstr_to_hex(csid_hex1, str1, PHSSectorClass::csid_byte_length);
                                    hexstr_to_hex(csid_hex2, (*indoor_list)[i], PHSSectorClass::csid_byte_length);
		                            if(0 == strncmp((char*)csid_hex1,(char*)csid_hex2, PHSSectorClass::csid_byte_length)) {
		                                found = 1;
		                                break;
		                            }
		                        }
		                    }
                        if(found)
                            no_csid_indoor->append(1);
                        else
                            no_csid_indoor->append(0);
                        //num_no_csid++;
                        //end. liutao, 060802
                    }

                    state = STATE_NUM_ROAD_TEST_PT;
                    break;
                case STATE_NUM_ROAD_TEST_PT:
                    if (strcmp(str1, "NUM_ROAD_TEST_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ROAD_TEST_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);

                    if (str1) {
                        num_road_test_pt = atoi(str1);
                    } else {
                        num_road_test_pt = -1;
                    }

                    if (num_road_test_pt < 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "num_road_test_pt = %d must be >= 0\n",
                                filename, linenum, num_road_test_pt);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (num_road_test_pt) {
                        rt_idx = 0;
                        state = STATE_ROAD_TEST_PT;
                    } else {
                        state = STATE_CSID;
                    }
                    break;
                case STATE_ROAD_TEST_PT:
                    sprintf(str, "ROAD_TEST_PT_%d:", rt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (sector) {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            rtd_u = atof(str1);
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Road test point %d has no posn_x specified\n",
                                    filename, linenum, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            rtd_v = atof(str1);
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Road test point %d has no posn_y specified\n",
                                    filename, linenum, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        if ( (rtd_coordinate_system == CConst::CoordLONLAT) && (coordinate_system == CConst::CoordUTM) ) {
                            LLtoUTM( rtd_u, rtd_v,
                                     geom_x,  geom_y,
                                     utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
                        } else if ( (rtd_coordinate_system == coordinate_system) ) {
                            geom_x = rtd_u;
                            geom_y = rtd_v;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Error encountered in coordinate conversion\n",
                                    filename, linenum);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        check_grid_val(geom_x, resolution, system_startx, &posn_x);
                        check_grid_val(geom_y, resolution, system_starty, &posn_y);

                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            pwr_db = atof(str1);
                            pwr_db += pwr_unit_offset;
                        } else {
                            sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                             "Road test point %d has no power specified\n",
                                    filename, linenum, rt_idx);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }

                        road_test_data_list->append( RoadTestPtClass(posn_x, posn_y, pwr_db, cell_idx, sector_idx) );

                        if ( (!found_outside_bdy) && (!system_bdy->in_bdy_area(posn_x, posn_y)) ) {
                            sprintf(msg, "WARNING: road_test_data file \"%s\" contains points outside the system boundary area\n"
                                             "Cell %d Sector %d Road_test_pt %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                                    filename, cell_idx, sector_idx, rt_idx, idx_to_x(posn_x), idx_to_y(posn_y));
                            PRMSG(stdout, msg); warning_state = 1;
                            found_outside_bdy = 1;
                        }
                    }

                    rt_idx++;
                    if (rt_idx >= num_road_test_pt) {
                        state = STATE_CSID;
                    }
                    break;
                default:
                    sprintf(msg, "ERROR: invalid road_test_data file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg); error_state = 1;
                    return;
                    break;
            }
        }
        if (bytes_read >= byte_num_thr) {
            curr_prog = (int) 100.0*bytes_read / file_size;
            byte_num_thr = (int) ( ((curr_prog + 5) / 100.0) * file_size );
#if HAS_GUI
            if (use_gui) {
                // printf("Setting Progress Bar to %d\n", curr_prog);
                prog_bar->set_prog_percent(curr_prog);
            } else {
#endif
                time(&td);
#if HAS_GUI
            }
#endif
        }
    }
    GUI_FN(delete prog_bar);

    /*
    FILE* fp_rtp_all;
    fp_rtp_all = fopen("rtp_all.txt","w+");

    for(int ii=0; ii<road_test_data_list->getSize()-1; ii++) {
        RoadTestPtClass *rtp = &( (*road_test_data_list)[ii] );
        if(rtp->cell_idx==127) {
            fprintf(fp_rtp_all,"%d\t%d\n",rtp->posn_x, rtp->posn_y);
        }
    }
    fclose(fp_rtp_all);
    */

    //added by liutao, 060802
    sprintf(msg, "------------------------------------------------------");
    PRMSG(stdout, msg);
    sprintf(msg, "List of the csid in RT data but not in the cell table:");
    PRMSG(stdout, msg);
    for(i=0; i<no_csid->getSize(); i++) {
        sprintf(msg, "%s", (*no_csid)[i]);
        if ((*no_csid_indoor)[i])
            strcat(msg, "\tindoor");
        PRMSG(stdout,msg);
    }

    /*
    for(int i=0; i<num_no_csid; i++) {
        sprintf(msg, "%s", no_csid[i]);
        if (no_csid_indoor[i])
            strcat(msg, "\tindoor");
        PRMSG(stdout,msg);
    }
    */

    sprintf(msg, "------------------------------------------------------");
    PRMSG(stdout, msg);
    
    for(i=0; i<=no_csid->getSize()-1; i++) {
        free((*no_csid)[i]);
    }
    delete no_csid;

    //csid in cell table but not in the RT data
    
    int* cell_has_rtd=(int*)malloc(num_cell*sizeof(int));
    for(i=0; i<num_cell; i++)
        cell_has_rtd[i]=0;

    RoadTestPtClass *rtp;
    for( int rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++ ) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_has_rtd[rtp->cell_idx] = 1;
    }

    //print to output file
    fprintf( fp_csid, "\nList of the csid in cell table but not in the RT data: \n");
    sprintf(msg, "------------------------------------------------------");
    PRMSG(stdout, msg);
    sprintf(msg, "List of the csid in cell table but not in the RT data:");
    PRMSG(stdout, msg);

    char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
    for( i=0; i<num_cell; i++) {
        if( !cell_has_rtd[i] ) {
            SectorClass* sector = cell_list[i]->sector_list[0]; 
            hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
            fprintf( fp_csid, hexstr );
            fprintf( fp_csid, "\n" );
            sprintf( msg, "%s", hexstr );
            PRMSG( stdout, msg );
        }
    }
    
    free(hexstr);
    free(cell_has_rtd);
    if(fp_csid)
        fclose(fp_csid);

    if(fp_indoor)
        fclose(fp_indoor);
    sprintf(msg, "------------------------------------------------------");
    PRMSG(stdout, msg);

    //end. liutao, 060802

    RoadTestPtClass::sort_type = CConst::byPwrSort;
    road_test_data_list->sort();

#endif
    return;
}
/*
******************************************************************
* FUNCTION: save_road_test_data
******************************************************************
*/
void NetworkClass::save_road_test_data(char *filename)
{
#if (DEMO == 0)

    int cell_idx                  = -1;
    int sector_idx                = -1;
    int rtd_idx                   = -1;
    int rtd_idx_0                 = -1;
    int found_n;
    char *chptr;
    PHSSectorClass *sector;
    RoadTestPtClass *rtp, *n_rtp;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to writing to file \"%s\"\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "# PHS Road Test Data File Generated by WISIM\n");
    chptr += sprintf(chptr, "FORMAT: %s\n", ROAD_TEST_DATA_FORMAT);
    chptr += sprintf(chptr, "\n");
    if (coordinate_system == CConst::CoordGeneric) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: GENERIC\n");
    } else if (coordinate_system == CConst::CoordUTM) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: UTM:%.0f:%.9f:%d:%c\n", utm_equatorial_radius, utm_eccentricity_sq,
            utm_zone, (utm_north ? 'N' : 'S'));
    } else if (coordinate_system == CConst::CoordLONLAT) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: LON_LAT\n");
    } else {
        CORE_DUMP;
    }
    chptr += sprintf(chptr, "PWR_MEAS_UNIT: dBm\n");
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "# LINE FORMAT: posn_x posn_x meas_pwr_db\n");
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    RoadTestPtClass::sort_type = CConst::bySectorSort;
    road_test_data_list->sort();

    int n_rtd_idx = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        if (rtd_idx == n_rtd_idx) {
            cell_idx   = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            found_n = 0;
            while(!found_n) {
                if (n_rtd_idx <= road_test_data_list->getSize()-1) {
                    n_rtp = &( (*road_test_data_list)[n_rtd_idx] );
                    if ( (n_rtp->cell_idx != cell_idx) || (n_rtp->sector_idx != sector_idx) ) {
                        found_n = 1;
                    } else {
                        n_rtd_idx++;
                    }
                } else {
                    found_n = 1;
                }
            }
            sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];

            if (sector->csid_hex) {
                chptr = msg;
                chptr += sprintf(chptr, "CSID: ");
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
                chptr += sprintf(chptr, "\n");
                chptr += sprintf(chptr, "    NUM_ROAD_TEST_PT: %d\n", n_rtd_idx-rtd_idx);
                PRMSG(fp, msg);
            } else {
                sprintf(msg, "WARNING: CELL %d SECTOR %d has road test data but no CSID, data ignored\n", cell_idx, sector_idx);
                PRMSG(stdout, msg); warning_state = 1;
            }
            rtd_idx_0 = rtd_idx;
        }

        if (sector->csid_hex) {
            chptr = msg;
            chptr += sprintf(chptr, "        ROAD_TEST_PT_%d: %9.7f %9.7f %9.7f\n", rtd_idx-rtd_idx_0, idx_to_x(rtp->posn_x), idx_to_y(rtp->posn_y), rtp->pwr_db);
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

    RoadTestPtClass::sort_type = CConst::byPwrSort;
    road_test_data_list->sort();

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::filt_road_test_data                                      ****/
/******************************************************************************************/
void NetworkClass::filter_rtd(ListClass<IntIntClass> *ii_list)
{
    int rtd_idx;
    PolygonClass *p = new PolygonClass(ii_list);
    RoadTestPtClass      *rtp;

    rtd_idx = 0;
    while(rtd_idx<=road_test_data_list->getSize()-1) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        if (p->in_bdy_area(rtp->posn_x, rtp->posn_y)) {
            rtd_idx++;
        } else {
            road_test_data_list->del_elem_idx(rtd_idx, 1);
        }
    }

    delete p;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::filt_road_test_data                                      ****/
/******************************************************************************************/
void NetworkClass::filt_road_test_data(ListClass<int> *scan_index_list, double angle_deg, double width_deg)
{
#if (DEMO == 0)
    int cell_idx;
    int sector_idx;
    int scan_idx;
    int rtd_idx;
    int del;

    CellClass            *cell   = NULL;
    SectorClass          *sector = NULL;
    RoadTestPtClass      *rtp;

    double dx, dy, ddx, ddy;

    double angle_rad, width_rad, c, s, theta;

    angle_rad = angle_deg * PI / 180.0;
    width_rad = width_deg * PI / 180.0;

    c = cos(angle_rad);
    s = sin(angle_rad);

    rtd_idx = 0;
    while(rtd_idx<=road_test_data_list->getSize()-1) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        if (scan_index_list->contains(scan_idx)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];

            dx = (double) rtp->posn_x - cell->posn_x;
            dy = (double) rtp->posn_y - cell->posn_y;

            ddx = dx*c + dy*s;
            ddy = dy*c - dx*s;

            theta = atan2(ddy, ddx);
            if (theta < 0.0) { theta += 2*PI; }
            del = ( (theta < width_rad) ? 0 : 1 );

            if (del) {
                road_test_data_list->del_elem_idx(rtd_idx, 1);
            } else {
                rtd_idx++;
            }
        }
    }
#endif
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::comp_prop_model_segment_angle()                          ****/
/******************************************************************************************/
void NetworkClass::comp_prop_model_segment_angle(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err, const int ignore_fs_check)
{
#if (DEMO == 0)
    //simple variable definition
    int    i;                 //loop variable
    int    cell_idx;          //cell loop variable
    int    sector_idx;        //sector loop variable
    int    scan_idx;
    int    ang_idx;
    int    rtd_idx;           //road test point loop variable for each sector
    int    pt_idx;            //road test point loop variable for total
    int    clutter_idx;       //clutter type loop varialbe
    int    num_road_test_pt;  //the number of the test point of all the cell.
    int    search_range;      //the number of the angle should to search.
    int    all_omni;
    int    scan_list_idx;
    double height;            //the height of the base station of each sector
    double min_height;        //the average height of CS antenna to compute propagation model
    double error = 0.0;       //error of the propagation model.
    double dx;
    double dy;
    double dz;
    double logd;
    double dmax;
    double rsq, sumrsq;
    double minb = 0.0;
    double maxb = 0.0;

    //the largest distance point with signal and the smallest distance point without signal
    double        max_logd = 0.0;

    //struct variable definition
    double               *dist_vector;    //array of each clutter's distance
    CellClass            *cell   = NULL;         //pointer of cell struct
    SectorClass          *sector = NULL;         //pointer of sector struct
    SectorClass          *adjust_sector = NULL;  //sector which have been adjust it's angle
    RoadTestPtClass      *rtp;                   //pointer of the test point

    //matrix and array of the first step singular value decomposition
    double **mx_a;  //matrix A for the second step, MxN:N is clutter number plus 2
    double *vec_b;  //vector B for the first step, Mx1
    double *vec_logd;//vector logd for the first step
    double *vec_cos;
    double *vec_sin;

    all_omni = 1;
    for (scan_list_idx=0; (scan_list_idx<=num_scan_index-1)&&(all_omni); scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        if (!antenna_type_list[cell_list[cell_idx]->sector_list[sector_idx]->antenna_type]->get_is_omni()) {
            all_omni = 0;
        }
    }
    if (all_omni) {
        adjust_angles = 0;
    }

    /*
    *************************************************************************
    * Allocate struct for the propagaton model.
    *************************************************************************
    */
    char strr[100];
    int  index = 0;
    for ( i=0; i<num_prop_model; i++ ) {
        if (num_scan_index == 1) {
            if ( strncmp(prop_model_list[i]->get_strid(), "PM", 2) == 0 ) {
                index++;
            }
        } else {
            if ( strncmp(prop_model_list[i]->get_strid(), "GPM", 3) == 0 ) {
                index++;
            }
        }
    }
    if (num_scan_index == 1) {
        sprintf(strr, "PM_%d", index);
    } else {
        sprintf(strr, "GPM_%d", index);
    }
    SegmentAnglePropModelClass *pm =  new SegmentAnglePropModelClass(map_clutter, strr);
    pm->useheight = useheight;
    pm->n_angle = 8;
    pm->use_n_y[0] = 1;
    pm->y[0] = (double *) realloc((void *) pm->y[0], (pm->use_n_y[0] ? pm->n_angle : 1)*sizeof(double));
    pm->use_n_start_slope = 1;
    pm->start_slope = (double *) realloc((void *) pm->start_slope, (pm->use_n_start_slope ? pm->n_angle : 1)*sizeof(double));
    pm->use_n_final_slope = 1;
    pm->final_slope = (double *) realloc((void *) pm->final_slope, (pm->use_n_final_slope ? pm->n_angle : 1)*sizeof(double));
    pm->cc = DVECTOR(pm->n_angle);
    pm->ss = DVECTOR(pm->n_angle);
    for (i=0; i<=pm->n_angle-1; i++) {
        pm->cc[i] = cos(PI*(2*i+1)/pm->n_angle);
        pm->ss[i] = sin(PI*(2*i+1)/pm->n_angle);
    }

    //get the number of road test points
    num_road_test_pt = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        scan_idx = (rtp->sector_idx << bit_cell) | rtp->cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            num_road_test_pt++;
        }
    }

    // Allocate struct for the clutter coefficient.
    cell_idx   = scan_index_list[0] & ((1<<bit_cell)-1);
    sector_idx = scan_index_list[0] >> bit_cell;
    if (num_road_test_pt < 10) {
        sprintf(msg, "ERROR LEVEL 0: sector %d_%d num_road_test_pt = %d < 10.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<30 && num_road_test_pt>=10 ) {
        sprintf(msg, "ERROR LEVEL 1: sector %d_%d num_road_test_pt = %d < 30.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<100 && num_road_test_pt>=30 ) {
        sprintf(msg, "ERROR LEVEL 2: sector %d_%d num_road_test_pt = %d < 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else {
        sprintf(msg, "OK: sector %d_%d num_road_test_pt = %d > 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
    }

    if (!map_clutter) {
        //sprintf(msg, "WARNING: No Clutter Data Defined.  Computing model without clutter data!\n");
        //PRMSG(stdout, msg); warning_state = 1;
        pm->num_clutter_type = 0;
    } else {
        pm->num_clutter_type = map_clutter->num_clutter_type;
    }

    dist_vector = DVECTOR(pm->num_clutter_type);
    pm->vec_k   = DVECTOR(pm->num_clutter_type+2*pm->useheight);

    // Allocate Vectors and matrix
    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        mx_a[pt_idx] = DVECTOR(pm->num_clutter_type+2*pm->useheight);
        for (clutter_idx=0; clutter_idx<=pm->num_clutter_type+2*pm->useheight-1; clutter_idx++) {
            mx_a[pt_idx][clutter_idx] = 0.0;
        }
    }

    vec_logd = DVECTOR(num_road_test_pt);
    vec_cos  = DVECTOR(num_road_test_pt);
    vec_sin  = DVECTOR(num_road_test_pt);
    vec_b = DVECTOR(num_road_test_pt);  //for optimize_clutter_coeffs will
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        vec_b[pt_idx]   = 0.0;  //the vector is initial to zero too.
        vec_logd[pt_idx] = 0.0;  //the vector is initial to zero too.
    }

#if DEBUG_COMP_PROP_MODEL
    // Begin to start the rountines,print the start time.
    time_t td;
    time(&td);
    printf("Current time is %s", ctime(&td));
    printf("Propagation model is computed for following CELL:");
    for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        printf(" %d_%d", cell_idx, sector_idx);
    }
    printf("\n");
#endif

    /*
    *************************************************************************
    * Prepare the vector logd and matrix for the SVD
    *************************************************************************
    */
    pt_idx = 0;
    min_height = -1.0;
    sumrsq = 0;

    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;

        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];

            height = sector->antenna_height;

            if ( (min_height == -1.0) || (height < min_height) ) {
                min_height = height;
            }

            if (pm->num_clutter_type) {
                get_path_clutter(cell->posn_x,cell->posn_y,rtp->posn_x,rtp->posn_y,dist_vector);
            }

            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = -sector->antenna_height;
            rsq  = dx*dx + dy*dy + dz*dz;
            logd = 0.5*log( rsq ) / log(10.0);
            vec_logd[pt_idx] = logd;
            vec_cos[pt_idx]  = dx / sqrt(dx*dx+dy*dy);
            vec_sin[pt_idx]  = dy / sqrt(dx*dx+dy*dy);
            sumrsq += dx*dx+dy*dy;

            if( pm->useheight ) {
                mx_a[pt_idx][0] = log(height)/log(10.0);
                mx_a[pt_idx][1] = log(height)/log(10.0)*logd;

                for (i=0; i<=pm->num_clutter_type-1; i++) {
                    mx_a[pt_idx][i+2] = dist_vector[i];
                }
            } else {
                for (i=0; i<=pm->num_clutter_type-1; i++) {
                    mx_a[pt_idx][i] = dist_vector[i];
                }
            }

            pt_idx = pt_idx + 1;
        }
    }
    dmax = 0.5*log(2*sumrsq/num_road_test_pt) / log(10.0);

    if ( min_height<=0.0 ) {
        sprintf(msg, "ERROR: Antenna height less than 0!\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    //used for search the optimal antenna angle
    const int    profile_search_range = 36;
    const int    fine_search_range    = 20;
    const double profile_search_step  = 2*PI / profile_search_range;
    const double fine_search_step     = PI / 180.0;
    double minimum_rms          = -1.0;
    double optimal_angle        = 0.0;
    double profile_angle        = 0.0;
    double original_angle       = 0.0;

#if DEBUG_ANTENNA_ANGLE_SEARCH
    FILE *fpsearchangle;
    fpsearchangle = fopen("search_antenna_angle.txt","w");

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                fprintf(fpsearchangle,"The original antenna angle of the sector is %f\n",sector->antenna_angle_rad);
            }
        }
    }
#endif

    /********************************************************************************/
    /**** Profile search for fine optimal angle of the antenna                   ****/
    /********************************************************************************/
    if ( adjust_angles == 1 ) {
        search_range = profile_search_range;
    } else {
        search_range = 0;
    }

    for( ang_idx=0; ang_idx<search_range; ang_idx++) {

        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;

            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }

            adjust_sector->antenna_angle_rad = profile_search_step * ang_idx;
        }

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx           = pt_idx + 1;
            }
        }

        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {
            if ( (minimum_rms == -1) || (sqrt(error/num_road_test_pt) < minimum_rms) ) {
                minimum_rms = sqrt(error/num_road_test_pt);
                profile_angle = adjust_sector->antenna_angle_rad;
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"profile search:  %15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /**** Fine search for fine optimal angle of the antenna                      ****/
    /********************************************************************************/
    if ( (adjust_angles == 1) && (minimum_rms != -1) ) {
        search_range = fine_search_range;
    } else {
        search_range = 0;
    }

    for( ang_idx=0; ang_idx<search_range; ang_idx++) {

        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;

            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }

            adjust_sector->antenna_angle_rad = fine_search_step * (ang_idx - fine_search_range/2 ) + profile_angle ;
        }

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx           = pt_idx + 1;
            }
        }

        //first step optimization
        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {

            if( sqrt(error/num_road_test_pt) < minimum_rms ) {
                minimum_rms   = sqrt(error/num_road_test_pt);
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"fine search:%15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /*** after search the optimal angle, begin to compute the propagation model  ****/
    /********************************************************************************/
    for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        scan_idx   = scan_index_list[scan_list_idx];
        cell_idx   = scan_idx & ((1<<bit_cell)-1);
        sector_idx = scan_idx >> bit_cell;
        cell = cell_list[cell_idx];
        sector = cell->sector_list[sector_idx];
        if ( adjust_angles == 1 ) {
            sector->antenna_angle_rad = optimal_angle;
        }
    }

    pt_idx = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = -sector->antenna_height;
            AntennaClass *antenna = antenna_type_list[sector->antenna_type];
            double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
            double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
            vec_b[pt_idx]   = rtp->pwr_db - gain_db - tx_pwr_db;
            if ( (pt_idx == 0) || (vec_b[pt_idx] < minb) ) {
                minb = vec_b[pt_idx];
            }
            if ( (pt_idx == 0) || (vec_b[pt_idx] > maxb) ) {
                maxb = vec_b[pt_idx];
            }
            pt_idx = pt_idx + 1;
        }
    }

#if 0
    char str[100];
    //sprintf(str,"roadtest_%d_%d.txt",scan_index_list[0]&((1<<bit_cell)-1),scan_index_list[0]>>bit_cell);
    sprintf(str,"roadtest.txt");
    FILE *fprtd = fopen(str,"w");

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        fprintf(fprtd,"%f %f\n",vec_logd[pt_idx],vec_b[pt_idx]);
    }

    fclose(fprtd);
#endif

    /*******************************************************************/
    /*** the first step optimization, when for the last operation,  ****/
    /*** should check the parameters                                ****/
    /*******************************************************************/
    if (!pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {
        sprintf(msg, "%s: Problems creating prop_model for sector group (1).\n", (err ? "ERROR" : "WARNING"));
        PRMSG(stdout, msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
            sprintf(msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, msg);
        }
        sprintf(msg, "\n");
        PRMSG(stdout, msg);

        if ( adjust_angles == 1 ) {
            adjust_sector->antenna_angle_rad = original_angle;
        }

        if (err) {
            printf("error \n");
            error_state = 1;
        } else {
            warning_state = 1;
        }
        return;
    }

#if 0
    // the second step optimization
    pm->optimize_clutter_coeffs(num_road_test_pt,mx_a,vec_b,vec_logd);

    // compute the error
    error = pm->compute_error(num_road_test_pt,mx_a,vec_b,vec_logd, 1, "err_dist.txt");
#endif

    for (i=0; i<=num_road_test_pt-1; i++) {
        free(mx_a[i]);
    }
    free(mx_a);
    free(vec_b);
    free(vec_logd);
    free(vec_cos);
    free(vec_sin);
    free(dist_vector);

#if DEBUG_ANTENNA_ANGLE_SEARCH
    fprintf(fpsearchangle,"Optimal antenna angle is %15.10f\t", optimal_angle);
    fprintf(fpsearchangle,"Minimum RMS ERROR is %15.10f\n", minimum_rms);
    fclose(fpsearchangle);
#endif

    if (dmax < pm->x[pm->num_inflexion-1] + log(1.1)/log(10.0)) {
        dmax = pm->x[pm->num_inflexion-1] + log(1.1)/log(10.0);
    }

    pm->add_cutoff_slope(dmax, minb, maxb+30, -50.0);

    //If it's not adjust the antenna angle, then at the end we condition the propagation model
    //with the test point which have no signal for the appoint test point.
    if ( adjust_angles == 0 ) {

        //search the largest distance test point with signal
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx   = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                double logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                if ( logd > max_logd ) {
                    max_logd = logd;
                }
            }
        }

        num_prop_model++;
        prop_model_list = (PropModelClass **) realloc((void *) prop_model_list, (num_prop_model)*sizeof(PropModelClass *));
        prop_model_list[num_prop_model-1] = (PropModelClass *) pm;

        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell_list[cell_idx]->sector_list[sector_idx]->prop_model = num_prop_model-1;
        }
    }

    if ( adjust_angles == 1 ) {
        delete pm;
    }

#if DEBUG_COMP_PROP_MODEL
    time(&td);
    printf("%s\n", ctime(&td));
#endif

#endif
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::comp_prop_model_with_theta()                             ****/
/******************************************************************************************/
void NetworkClass::comp_prop_model_with_theta(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err)
{
#if (DEMO == 0)
    //simple variable definition
    int    i;                 //loop variable
    int    cell_idx;          //cell loop variable
    int    sector_idx;        //sector loop variable
    int    scan_idx;
    int    ang_idx;
    int    rtd_idx;           //road test point loop variable for each sector
    int    pt_idx;            //road test point loop variable for total
    int    clutter_idx;       //clutter type loop varialbe
    int    num_road_test_pt;  //the number of the test point of all the cell.
    int    search_range;      //the number of the angle should to search.
    int    all_omni;
    int    scan_list_idx;
    double height;            //the height of the base station of each sector
    double min_height;        //the average height of CS antenna to compute propagation model
    double error = 0.0;       //error of the propagation model.
    double dx;
    double dy;
    double dz;
    double logd;
    double dmax;
    double minb = 0.0;
    double maxb = 0.0;

    //the largest distance point with signal and the smallest distance point without signal
    double        max_logd = 0.0;
    double        max_logd_power = 0.0;

    //struct variable definition
    double               *dist_vector;           //array of each clutter's distance
    CellClass            *cell   = NULL;         //pointer of cell struct
    SectorClass          *sector = NULL;         //pointer of sector struct
    SectorClass          *adjust_sector = NULL;  //sector which have been adjust it's angle
    RoadTestPtClass      *rtp;                   //pointer of the test point

    //matrix and array of the first step singular value decomposition
    double **mx_a;  //matrix A for the second step, MxN:N is clutter number plus 2
    double *vec_b;  //vector B for the first step, Mx1
    double *vec_logd;//vector logd for the first step
    double *vec_cos;
    double *vec_sin;

    all_omni = 1;
    for (scan_list_idx=0; (scan_list_idx<=num_scan_index-1)&&(all_omni); scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        if (!antenna_type_list[cell_list[cell_idx]->sector_list[sector_idx]->antenna_type]->get_is_omni()) {
            all_omni = 0;
        }
    }
    if (all_omni) {
        adjust_angles = 0;
    }

    /*
    *************************************************************************
    * Allocate struct for the propagaton model.
    *************************************************************************
    */
    char strr[100];
    int  index = 0;
    for ( i=0; i<num_prop_model; i++ ) {
        if ( strncmp(prop_model_list[i]->get_strid(), "PM", 2) == 0 ) {
            index++;
        }
    }
    sprintf(strr, "PM_%d", index);
    SegmentWithThetaPropModelClass *pm =  new SegmentWithThetaPropModelClass(map_clutter, strr);
    pm->useheight = useheight;
    pm->n_series_y[0] = 6;
    pm->c_series_y[0] = (double *) realloc((void *) pm->c_series_y[0], pm->n_series_y[0]*sizeof(double));
    pm->s_series_y[0] = (double *) realloc((void *) pm->s_series_y[0], pm->n_series_y[0]*sizeof(double));
    pm->n_series_start_slope = 6;
    pm->c_series_start_slope = (double *) realloc((void *) pm->c_series_start_slope, pm->n_series_start_slope*sizeof(double));
    pm->s_series_start_slope = (double *) realloc((void *) pm->s_series_start_slope, pm->n_series_start_slope*sizeof(double));
    pm->n_series_final_slope = 6;
    pm->c_series_final_slope = (double *) realloc((void *) pm->c_series_final_slope, pm->n_series_final_slope*sizeof(double));
    pm->s_series_final_slope = (double *) realloc((void *) pm->s_series_final_slope, pm->n_series_final_slope*sizeof(double));

    //get the number of road test points
    num_road_test_pt = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        scan_idx = (rtp->sector_idx << bit_cell) | rtp->cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            num_road_test_pt++;
        }
    }

    // Allocate struct for the clutter coefficient.
    cell_idx   = scan_index_list[0] & ((1<<bit_cell)-1);
    sector_idx = scan_index_list[0] >> bit_cell;
    if (num_road_test_pt < 10) {
        sprintf(msg, "ERROR LEVEL 0: sector %d_%d num_road_test_pt = %d < 10.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<30 && num_road_test_pt>=10 ) {
        sprintf(msg, "ERROR LEVEL 1: sector %d_%d num_road_test_pt = %d < 30.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<100 && num_road_test_pt>=30 ) {
        sprintf(msg, "ERROR LEVEL 2: sector %d_%d num_road_test_pt = %d < 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else {
        sprintf(msg, "OK: sector %d_%d num_road_test_pt = %d > 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
    }

    if (!map_clutter) {
        //sprintf(msg, "WARNING: No Clutter Data Defined.  Computing model without clutter data!\n");
        //PRMSG(stdout, msg);
        pm->num_clutter_type = 0;
    } else {
        pm->num_clutter_type = map_clutter->num_clutter_type;
    }

    dist_vector = DVECTOR(pm->num_clutter_type);
    pm->vec_k   = DVECTOR(pm->num_clutter_type+2*pm->useheight);

    // Allocate Vectors and matrix
    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        mx_a[pt_idx] = DVECTOR(pm->num_clutter_type+2*pm->useheight);
        for (clutter_idx=0; clutter_idx<=pm->num_clutter_type+2*pm->useheight-1; clutter_idx++) {
            mx_a[pt_idx][clutter_idx] = 0.0;
        }
    }

    vec_logd = DVECTOR(num_road_test_pt);
    vec_cos  = DVECTOR(num_road_test_pt);
    vec_sin  = DVECTOR(num_road_test_pt);
    vec_b = DVECTOR(num_road_test_pt);  //for optimize_clutter_coeffs will
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        vec_b[pt_idx]   = 0.0;  //the vector is initial to zero too.
        vec_logd[pt_idx] = 0.0;  //the vector is initial to zero too.
    }

#if DEBUG_COMP_PROP_MODEL
    // Begin to start the rountines,print the start time.
    time_t td;
    time(&td);
    printf("Current time is %s", ctime(&td));
    printf("Propagation model is computed for following CELL:");
    for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        printf(" %d_%d", cell_idx, sector_idx);
    }
    printf("\n");
#endif

    /*
    *************************************************************************
    * Prepare the vector logd and matrix for the SVD
    *************************************************************************
    */
    pt_idx = 0;
    min_height = -1.0;

    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;

        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];

            height = sector->antenna_height;

            if ( (min_height == -1.0) || (height < min_height) ) {
                min_height = height;
            }

            if (pm->num_clutter_type) {
                get_path_clutter(cell->posn_x,cell->posn_y,rtp->posn_x,rtp->posn_y,dist_vector);
            }

            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = -sector->antenna_height;
            logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
            vec_logd[pt_idx] = logd;
            vec_cos[pt_idx]  = dx / sqrt(dx*dx+dy*dy);
            vec_sin[pt_idx]  = dy / sqrt(dx*dx+dy*dy);
            if ( (pt_idx == 0) || (logd > dmax) ) {
                dmax = logd;
            }

            if( pm->useheight ) {
                mx_a[pt_idx][0] = log(height)/log(10.0);
                mx_a[pt_idx][1] = log(height)/log(10.0)*logd;

                for (i=0; i<=pm->num_clutter_type-1; i++) {
                    mx_a[pt_idx][i+2] = dist_vector[i];
                }
            } else {
                for (i=0; i<=pm->num_clutter_type-1; i++) {
                    mx_a[pt_idx][i] = dist_vector[i];
                }
            }

            pt_idx = pt_idx + 1;
        }
    }

    if ( min_height<=0.0 ) {
        sprintf(msg, "ERROR: Antenna height less than 0!\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    //used for search the optimal antenna angle
    const int    profile_search_range = 36;
    const int    fine_search_range    = 20;
    const double profile_search_step  = 2*PI / profile_search_range;
    const double fine_search_step     = PI / 180.0;
    double minimum_rms          = -1.0;
    double optimal_angle        = 0.0;
    double profile_angle        = 0.0;
    double original_angle       = 0.0;

#if DEBUG_ANTENNA_ANGLE_SEARCH
    FILE *fpsearchangle;
    fpsearchangle = fopen("search_antenna_angle.txt","w");

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                fprintf(fpsearchangle,"The original antenna angle of the sector is %f\n",sector->antenna_angle_rad);
            }
        }
    }
#endif

    /********************************************************************************/
    /**** Profile search for fine optimal angle of the antenna                   ****/
    /********************************************************************************/
    if ( adjust_angles == 1 ) {
        search_range = profile_search_range;
    } else {
        search_range = 0;
    }

    for( ang_idx=0; ang_idx<search_range; ang_idx++) {
        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;

            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }

            adjust_sector->antenna_angle_rad = profile_search_step * ang_idx;
        }

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx           = pt_idx + 1;
            }
        }

        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {

            if ( (minimum_rms == -1) || (sqrt(error/num_road_test_pt) < minimum_rms) ) {
                minimum_rms = sqrt(error/num_road_test_pt);
                profile_angle = adjust_sector->antenna_angle_rad;
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"profile search:  %15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /**** Fine search for fine optimal angle of the antenna                      ****/
    /********************************************************************************/
    if ( (adjust_angles == 1) && (minimum_rms != -1) ) {
        search_range = fine_search_range;
    } else {
        search_range = 0;
    }

    for( ang_idx=0; ang_idx<search_range; ang_idx++) {

        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;

            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }

            adjust_sector->antenna_angle_rad = fine_search_step * (ang_idx - fine_search_range/2 ) + profile_angle ;
        }

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx           = pt_idx + 1;
            }
        }

        //first step optimization
        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {

            if( sqrt(error/num_road_test_pt) < minimum_rms ) {
                minimum_rms   = sqrt(error/num_road_test_pt);
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"fine search:%15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /*** after search the optimal angle, begin to compute the propagation model  ****/
    /********************************************************************************/
    for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        scan_idx   = scan_index_list[scan_list_idx];
        cell_idx   = scan_idx & ((1<<bit_cell)-1);
        sector_idx = scan_idx >> bit_cell;
        cell = cell_list[cell_idx];
        sector = cell->sector_list[sector_idx];
        if ( adjust_angles == 1 ) {
            sector->antenna_angle_rad = optimal_angle;
        }
    }

    pt_idx = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = -sector->antenna_height;
            AntennaClass *antenna = antenna_type_list[sector->antenna_type];
            double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
            double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
            vec_b[pt_idx]   = rtp->pwr_db - gain_db - tx_pwr_db;
            if ( (pt_idx == 0) || (vec_b[pt_idx] < minb) ) {
                minb = vec_b[pt_idx];
            }
            if ( (pt_idx == 0) || (vec_b[pt_idx] > maxb) ) {
                maxb = vec_b[pt_idx];
            }
            pt_idx = pt_idx + 1;
        }
    }

#if 0
    char str[100];
    //sprintf(str,"roadtest_%d_%d.txt",scan_index_list[0]&((1<<bit_cell)-1),scan_index_list[0]>>bit_cell);
    sprintf(str,"roadtest.txt");
    FILE *fprtd = fopen(str,"w");

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        fprintf(fprtd,"%f %f\n",vec_logd[pt_idx],vec_b[pt_idx]);
    }

    fclose(fprtd);
#endif

    /*******************************************************************/
    /*** the first step optimization, when for the last operation,  ****/
    /*** should check the parameters                                ****/
    /*******************************************************************/
    if (!pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,error,vec_cos, vec_sin)) {
        sprintf(msg, "%s: Problems creating prop_model for sector group (2).\n", (err ? "ERROR" : "WARNING"));
        PRMSG(stdout, msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
            sprintf(msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, msg);
        }
        sprintf(msg, "\n");
        PRMSG(stdout, msg);

        if ( adjust_angles == 1 ) {
            adjust_sector->antenna_angle_rad = original_angle;
        }

        if (err) {
            printf("error \n");
            error_state = 1;
        }
        return;
    }

#if 0
    // the second step optimization
    pm->optimize_clutter_coeffs(num_road_test_pt,mx_a,vec_b,vec_logd);

    // compute the error
    error = pm->compute_error(num_road_test_pt,mx_a,vec_b,vec_logd, 1, "err_dist.txt");
#endif

    for (i=0; i<=num_road_test_pt-1; i++) {
        free(mx_a[i]);
    }
    free(mx_a);
    free(vec_b);
    free(vec_logd);
    free(vec_cos);
    free(vec_sin);
    free(dist_vector);

#if DEBUG_ANTENNA_ANGLE_SEARCH
    fprintf(fpsearchangle,"Optimal antenna angle is %15.10f\t", optimal_angle);
    fprintf(fpsearchangle,"Minimum RMS ERROR is %15.10f\n", minimum_rms);
    fclose(fpsearchangle);
#endif

    pm->add_cutoff_slope(dmax, minb, maxb+30, -50.0);

    //If it's not adjust the antenna angle, then at the end we condition the propagation model
    //with the test point which have no signal for the appoint test point.
    if ( adjust_angles == 0 ) {

        //search the largest distance test point with signal
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx   = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                double logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                if ( logd > max_logd ) {
                    max_logd = logd;
                    max_logd_power = pm->y[0] + pm->final_slope*(max_logd - pm->x[0]);
                }
            }
        }

        num_prop_model++;
        prop_model_list = (PropModelClass **) realloc((void *) prop_model_list, (num_prop_model)*sizeof(PropModelClass *));
        prop_model_list[num_prop_model-1] = (PropModelClass *) pm;

        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell_list[cell_idx]->sector_list[sector_idx]->prop_model = num_prop_model-1;
        }
    }

    if ( pm->num_inflexion == 1 ) {
        printf("x[0]=%f,y[0]=%f,s=%f,f=%f\n", pm->x[0],pm->y[0],pm->start_slope,pm->final_slope);
    } else if ( pm->num_inflexion == 2 ) {
        printf("x[0]=%f,y[0]=%f,x[1]=%f,y[1]=%f,s=%f,f=%f\n", pm->x[0],pm->y[0],pm->x[1],pm->y[1],pm->start_slope,pm->final_slope);
    }

    if ( adjust_angles == 1 ) {
        delete pm;
    }

#if DEBUG_COMP_PROP_MODEL
    time(&td);
    printf("%s\n", ctime(&td));
#endif

#endif
}


#define CVX_HULL_DEBUG 0
/*  Constructure FUNCTION.
 */
Point::Point( int a, int b)
{
    found = false;
    x=a;
    y=b;
}

/* Return: >0 if P2 left of the line through P0 to P1;
 *         =0 if P2 on the line;
 *         <0 if P2 right of the line;
 */
double isLeft( Point& P0, Point& P1, Point& P2 )
{
    double judge_val;
    judge_val = (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
    return judge_val;
}

/*    FUNCTION : comp_cvx() compute convex polygon by input points vector.
 *    Return the convex polygon's vertex.
 */
std::vector<Point>& comp_cvx( std::vector<Point>& m_pt_vec, std::vector<Point>& result_pt_vec, int n )
{
    bool   flag   = true;
    int    pt_idx = 0;
    Point  ptt1;
    Point  ptt2;
    //std::vector<Point> result_pt_vec;

    /* sort all points choose pt0 as a center; */
    m_pt_vec = pt_sorting( m_pt_vec, n );

    /* compute convex polygon boundry */
    result_pt_vec.clear();
    result_pt_vec.push_back(m_pt_vec[0]);
    result_pt_vec.push_back(m_pt_vec[1]);
    result_pt_vec.push_back(m_pt_vec[2]);
    for ( pt_idx=3; pt_idx<(int)m_pt_vec.size(); pt_idx++ )
    {
        int num = (int)result_pt_vec.size();
        ptt1 = result_pt_vec[num-2];
        ptt2 = result_pt_vec[num-1];
        if ( isLeft(ptt1, ptt2, m_pt_vec[pt_idx])>0 )
        {
            result_pt_vec.push_back( m_pt_vec[pt_idx] );
        }

        while (  isLeft(ptt1, ptt2, m_pt_vec[pt_idx])<=0 ) {
            flag = false;
            result_pt_vec.pop_back();
            if ( result_pt_vec.size() == 2 ) {
                break;
            }

            num = result_pt_vec.size();
            ptt1 = result_pt_vec[num-2];
            ptt2 = result_pt_vec[num-1];
        }

        if ( !flag )
            result_pt_vec.push_back( m_pt_vec[pt_idx] );
    }

    return result_pt_vec;
}


/*   FUNCTION : pt_sorting() Sorting the given points list with increasing counter-clockwise angle.
 *   Return the sorted list of points.
 */
std::vector<Point>& pt_sorting(std::vector<Point>& m_pt_vec,  int n )
{
    int pt_idx    = 0;                      //loop index
    int pt_idx1   = 0;                      //loop index
    int pt_idx2   = 0;                      //loop index
    int found_idx = 0;                      //index of the rightmost point which we found in loop
    Point pt0 = m_pt_vec[0];                //save the value of P0 to m_pt_vec[0], suppose m_pt_vec[0] is the rightmost-lowest point;
    Point pt1;

    /* find the lowest-rightmost point P0 */
    for ( pt_idx=1; pt_idx<(int)m_pt_vec.size(); pt_idx++)
    {
        if (  m_pt_vec[pt_idx].y < pt0.y )
        {
            pt0 = m_pt_vec[pt_idx];
            found_idx = pt_idx;
        }
        else if ( fabs(m_pt_vec[pt_idx].y - pt0.y) < 1e-15 )
            if ( m_pt_vec[pt_idx].x > pt0.x )
            {
                pt0 = m_pt_vec[pt_idx];
                found_idx = pt_idx;
            }
    }

    /* Changing the position between the found_idx point and the first point in the points vector.  ||  update the vector */
    m_pt_vec[found_idx].x = m_pt_vec[0].x;
    m_pt_vec[found_idx].y = m_pt_vec[0].y;
    m_pt_vec[0].x = pt0.x;
    m_pt_vec[0].y = pt0.y;

#if CVX_HULL_DEBUG
    for ( int i=0; i<(int)m_pt_vec.size(); i++ )
        std::cout << "debug pt_sorting, find rightmost lowest point : (" << m_pt_vec[i].x <<"," << m_pt_vec[i].y << ")" << std::endl;
    std::cout << std::endl << " debug pt_sorting found rightmost-lowest point : (" << pt0.x << "," << pt0.y << ")" << std::endl;
    std::cout << " debug pt_sorting found rightmost-lowest point : (" << m_pt_vec[0].x << "," << m_pt_vec[0].y << ")" << std::endl;
    std::cout << " debug pt_sorting found orignal point : (" << m_pt_vec[found_idx].x << "," << m_pt_vec[found_idx].y << ")" << std::endl << std::endl;
#endif

    pt0 = m_pt_vec[0];
    pt1 = m_pt_vec[1];          //get the leftmost point
    found_idx = 1;
    for ( pt_idx1=1; pt_idx1<n-1; pt_idx1++) {
        found_idx = pt_idx1;

        for ( pt_idx2=pt_idx1+1; pt_idx2<n; pt_idx2++ )
            if ( isLeft( pt0, pt1, m_pt_vec[pt_idx2] ) < 0  &&  m_pt_vec[pt_idx2].found == false )
            {
                pt1 = m_pt_vec[pt_idx2];
                found_idx = pt_idx2;
            }

        if ( pt_idx1 != found_idx ) {
            m_pt_vec[found_idx] = m_pt_vec[pt_idx1];
            m_pt_vec[pt_idx1] = pt1;
        }

        m_pt_vec[pt_idx1].found = true;
        pt1 = m_pt_vec[pt_idx1+1];
    }
#if CVX_HULL_DEBUG
    for ( int i=0; i<(int)m_pt_vec.size(); i++ ) {
        std::cout << "debug pt_sorting, find point : (" << m_pt_vec[i].x <<"," << m_pt_vec[i].y << ")" << std::endl;
    }
    std::cout << "debug pt_sorting size " <<  m_pt_vec.size() << std::endl;
#endif

    return m_pt_vec;
}

/**  FUNCTION : polygon_tangents() find polygon's tangents
 *   Input:  pt_vec is an array of vertices for a convex polygon;
 *   Output: rpoint is the vertic of rightmost tangent;
 *           lpoint is the vertic of leftmost tangent;
 *   Return true if pt is outside the polygon;
 *   Return false if pt is inside the polygon;                          */
bool polygon_tangents( Point& pt, int n, std::vector<Point>& pt_vec, Point& rpoint, Point& lpoint )
{
    double prev = 0.0;
    double next = 0.0;
    bool   rfound = false;    // If rightmost tangent point found, rfound equal true;
    bool   lfound = false;    // If leftmost tangent point found, rfound equal true;

    int r_idx = 0;            // Index of rightmost tangent point pt_vec[r_idx]
    int l_idx = 0;            // Index of leftmost tangent point pt_vec[l_idx]
    int pt_idx;

    //  Judge whether the first point pt_vec[0]( = pt_vec[n] ) is the tangent vertex by the pt_vec[n-1], pt_vec[1].
    prev = isLeft(pt_vec[n-1], pt_vec[0], pt);
    next = isLeft(pt_vec[0], pt_vec[1], pt);
    if ( ( prev <= 0 ) && ( next > 0 ) )
        rfound = true;     // If pt_vec[0] is rightmost vertex;
    else if ( ( prev > 0 ) && ( next <= 0 ) )
        lfound = true;     // If pt_vec[0] is leftmost vertex;

    // Judge whether the points from pt_vec[1], pt_vec[n-1] are the tangents vertex.
    prev = next;
    for ( pt_idx=1; pt_idx<n; pt_idx++ ) {
        next = isLeft( pt_vec[pt_idx], pt_vec[pt_idx+1], pt );
        if ( ( prev <= 0 ) && ( next > 0 ) ) {
            if ( isLeft(pt, pt_vec[pt_idx], pt_vec[r_idx]) >= 0 ) {
                r_idx = pt_idx;
                rfound = true;
            }
        } else if ( ( prev > 0 ) && ( next <= 0 ) ) {
            if ( isLeft(pt, pt_vec[pt_idx], pt_vec[l_idx]) <= 0 ) {
                l_idx = pt_idx;
                lfound = true;
            }
        }
        if ( rfound && lfound )
        {
           rpoint.x = pt_vec[r_idx].x;
           rpoint.y = pt_vec[r_idx].y;
           lpoint.x = pt_vec[l_idx].x;
           lpoint.y = pt_vec[l_idx].y;
           return true;                  // Found the tangents, so the point pt is outside the polygon.
        }
        prev = next;
    }

    return false;                       // Donnot find the tangents, so the point pt is inside the polygon.
}

/*  Moved from gen_clutter.cpp, Modified, east direction reference direction, CCW return angle value [0, 2*PI].
 */
double get_angle( const int start_x, const int start_y, const int end_x, const int end_y )
{
    int heading;
    double angle;
    double slope;

    if ( start_x > end_x ) heading = 1;
    else heading = 0;

    if ( start_x==end_x ) {
        if ( start_y<end_y ) angle = PI/2.0;
        else angle = 3.0*PI / 2.0;
    } else {
        slope = (double)(end_y-start_y)/(end_x-start_x);
        angle = atan( slope);
        if ( slope >= 0 ) angle = heading*PI + angle;
        if ( slope < 0 ) angle = 2.0*PI - heading*PI + angle;
    }

    return(angle);
}


/*
******************************************************************
* FUNCTION: comp_prop_model
* According to the read data, compute the pathloss power and the
* distance of each clutter, get the coefficient of the propagation
* model by sigular valued discomposition. and print the result.
* If model can't be computed and err=1 set error_state.  If err=0
* dont set error state.
******************************************************************
*/
void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, double min_logd_threshold, const int err, const int ignore_fs_check)
{
#if (DEMO == 0)
    //simple variable definition
    int    i;                 //loop variable
    int    cell_idx;          //cell loop variable
    int    sector_idx;        //sector loop variable
    int    scan_idx;
    int    ang_idx;
    int    rtd_idx;           //road test point loop variable for each sector
    int    pt_idx;            //road test point loop variable for total
    int    clutter_idx;       //clutter type loop varialbe
    int    num_road_test_pt;  //the number of the test point of all the cell.
    int    search_range;      //the number of the angle should to search.
    int    all_omni;
    int    scan_list_idx;
    double height;            //the height of the base station of each sector
    double min_height;        //the average height of CS antenna to compute propagation model
    double error = 0.0;       //error of the propagation model.
    double dx;
    double dy;
    double dz;
    double logd, min_logd;

    //the largest distance point with signal and the smallest distance point without signal
    double        max_logd = 0.0;
    double        max_logd_power = 0.0;

    //struct variable definition
    double               *dist_vector;           //array of each clutter's distance
    CellClass            *cell   = NULL;         //pointer of cell struct
    SectorClass          *sector = NULL;         //pointer of sector struct
    SectorClass          *adjust_sector = NULL;  //sector which have been adjust it's angle
    RoadTestPtClass      *rtp;                   //pointer of the test point

    //matrix and array of the first step singular value decomposition
    double **mx_a;  //matrix A for the second step, MxN:N is clutter number plus 2
    double *vec_b;  //vector B for the first step, Mx1
    double *vec_logd;//vector logd for the first step

    all_omni = 1;
    for (scan_list_idx=0; (scan_list_idx<=num_scan_index-1)&&(all_omni); scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        if (!antenna_type_list[cell_list[cell_idx]->sector_list[sector_idx]->antenna_type]->get_is_omni()) {
            all_omni = 0;
        }
    }
    if (all_omni) {
        adjust_angles = 0;
    }

    /*
    *************************************************************************
    * Allocate struct for the propagaton model.
    *************************************************************************
    */
    char strr[100];
    int  index = 0;
    for ( i=0; i<num_prop_model; i++ ) {
        if (num_scan_index == 1) {
            if ( strncmp(prop_model_list[i]->get_strid(), "PM", 2) == 0 ) {
                index++;
            }
        } else {
            if ( strncmp(prop_model_list[i]->get_strid(), "GPM", 3) == 0 ) {
                index++;
            }
        }
    }
    if (num_scan_index == 1) {
        sprintf(strr, "PM_%d", index);
    } else {
        sprintf(strr, "GPM_%d", index);
    }
    SegmentPropModelClass *pm =  new SegmentPropModelClass(map_clutter, strr);
    pm->useheight = useheight;

    //get the number of the road test points
    num_road_test_pt = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        scan_idx = (rtp->sector_idx << bit_cell) | rtp->cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            num_road_test_pt++;
        }
    }

    // Allocate struct for the clutter coefficient.
    cell_idx   = scan_index_list[0] & ((1<<bit_cell)-1);
    sector_idx = scan_index_list[0] >> bit_cell;
    if (num_road_test_pt < 10) {
        sprintf(msg, "ERROR LEVEL 0: sector %d_%d num_road_test_pt = %d < 10.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<30 && num_road_test_pt>=10 ) {
        sprintf(msg, "ERROR LEVEL 1: sector %d_%d num_road_test_pt = %d < 30.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else if ( num_road_test_pt<100 && num_road_test_pt>=30 ) {
        sprintf(msg, "ERROR LEVEL 2: sector %d_%d num_road_test_pt = %d < 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
        //error_state = 1;
        return;
    } else {
        sprintf(msg, "OK: sector %d_%d num_road_test_pt = %d > 100.\n", cell_idx, sector_idx, num_road_test_pt);
        PRMSG(stdout, msg);
    }

    if (!map_clutter) {
        //sprintf(msg, "WARNING: No Clutter Data Defined.  Computing model without clutter data!\n");
        //PRMSG(stdout, msg); warning_state = 1;
        pm->num_clutter_type = 0;
    } else {
        pm->num_clutter_type = map_clutter->num_clutter_type;
    }

    dist_vector = DVECTOR(pm->num_clutter_type);
    pm->vec_k   = DVECTOR(pm->num_clutter_type+2*pm->useheight);

    // Allocate Vectors and matrix
    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        mx_a[pt_idx] = DVECTOR(pm->num_clutter_type+2*pm->useheight);
        for (clutter_idx=0; clutter_idx<=pm->num_clutter_type+2*pm->useheight-1; clutter_idx++) {
            mx_a[pt_idx][clutter_idx] = 0.0;
        }
    }

    vec_logd = DVECTOR(num_road_test_pt);
    vec_b = DVECTOR(num_road_test_pt);  //for optimize_clutter_coeffs will
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        vec_b[pt_idx]   = 0.0;  //the vector is initial to zero too.
        vec_logd[pt_idx] = 0.0;  //the vector is initial to zero too.
    }

#if DEBUG_COMP_PROP_MODEL
    // Begin to start the rountines,print the start time.
    time_t td;
    time(&td);
    printf("Current time is %s", ctime(&td));
    printf("Propagation model is computed for following CELL:");
    for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
        sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
        printf(" %d_%d", cell_idx, sector_idx);
    }
    printf("\n");
#endif

    /*
    *************************************************************************
    * Prepare the vector logd and matrix for the SVD
    *************************************************************************
    */
    pt_idx = 0;
    min_height = -1.0;
    min_logd = 0.0;
    int num_ignore_min_logd_threshold = 0;

    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;

        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];

            height = sector->antenna_height;

            if ( (min_height == -1.0) || (height < min_height) ) {
                min_height = height;
            }

            if (pm->num_clutter_type) {
                get_path_clutter(cell->posn_x,cell->posn_y,rtp->posn_x,rtp->posn_y,dist_vector);
            }

            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = - sector->antenna_height;
            logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);

            if (logd >= min_logd_threshold) {
                vec_logd[pt_idx] = logd;

                if ( (pt_idx == 0) || (logd < min_logd) ) {
                    min_logd = logd;
                }

                if( pm->useheight ) {
                    mx_a[pt_idx][0] = log(height)/log(10.0);
                    mx_a[pt_idx][1] = log(height)/log(10.0)*logd;

                    for (i=0; i<=pm->num_clutter_type-1; i++) {
                        mx_a[pt_idx][i+2] = dist_vector[i];
                    }
                } else {
                    for (i=0; i<=pm->num_clutter_type-1; i++) {
                        mx_a[pt_idx][i] = dist_vector[i];
                    }
                }

                pt_idx = pt_idx + 1;
            } else {
                num_ignore_min_logd_threshold++;
            }
        }
    }

    if (num_ignore_min_logd_threshold) {
        sprintf(msg, "WARNING: %d Road test points ignored because distance is less than %f\n", num_ignore_min_logd_threshold, exp(min_logd_threshold*log(10.0)));
        PRMSG(stdout, msg); warning_state = 1;
    }

    if ( min_height<=0.0 ) {
        sprintf(msg, "ERROR: Antenna height less than 0!\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    //used for search the optimal antenna angle
    const int    profile_search_range = 36;
    const int    fine_search_range    = 20;
    const double profile_search_step  = 2*PI / profile_search_range;
    const double fine_search_step     = PI / 180.0;
    double minimum_rms    = -1.0;
    double optimal_angle  = 0.0;
    double profile_angle  = 0.0;
    double original_angle = 0.0;

    // Judge the search range by the Cell position and the corresponding road test points;
    int    limit_range        = 0;     // Search range of angle;
    double limit_ang          = 0.0;   // The maximum angle need to be search;
    //double start_ang        = 0.0;   // The starting point of angle to search;
    int    adjust_sector_idx  = 0;
    int    adjust_cell_idx    = 0;     // The index of cell to be adjust angle;
    bool   limited            = false; //
    bool   div_angle          = false; //
    double leftmost_angle     = 0.0;
    double rightmost_angle    = 0.0;
    CellClass* adjust_cell    = NULL;  //pointer of cell struct

    if ( num_scan_index == 1 && adjust_angles == 1 ) {
        std::vector<Point> pt_vec;           // Vector of inputting road test points;
        std::vector<Point> poly_pt_vec;      // Vector of returned convex polygon vertexes;
        Point tmp_pt( 0, 0 );                // Keep a road test point;

        scan_idx   = scan_index_list[0];
        adjust_cell_idx = scan_idx & ((1<<bit_cell)-1);
        adjust_sector_idx = scan_idx >> bit_cell;
        adjust_cell = cell_list[adjust_cell_idx];
        adjust_sector = adjust_cell->sector_list[adjust_sector_idx];

        pt_vec.clear();
        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            if ( adjust_cell_idx == cell_idx ) {
                tmp_pt.x = rtp->posn_x;
                tmp_pt.y = rtp->posn_y;
                pt_vec.push_back( tmp_pt );
                pt_idx = pt_idx + 1;
            }
        }
        int cnt = (int) pt_vec.size();
        poly_pt_vec = comp_cvx( pt_vec, poly_pt_vec, cnt );

        Point  pt;
        Point  lpt;
        Point  rpt;
        bool   flag;
        cnt = (int) poly_pt_vec.size();
        pt.x = adjust_cell->posn_x;
        pt.y = adjust_cell->posn_y;
        flag = polygon_tangents( pt, cnt, poly_pt_vec, rpt, lpt );

        pt_vec.clear();
        poly_pt_vec.clear();

        if ( flag ) {
            leftmost_angle   = ceil ( get_angle( (const int)(pt.x), (const int)(pt.y), (const int)(lpt.x), (const int)(lpt.y) ) * 180./PI );
            rightmost_angle  = floor( get_angle( (const int)(pt.x), (const int)(pt.y), (const int)(rpt.x), (const int)(rpt.y) ) * 180./PI );

#if !CVX_HULL_DEBUG
            std::cout << " ======= DEBUG =======" << std::endl;
            std::cout << " Index of adjust cell " << adjust_cell_idx << std::endl;
            std::cout << " Right most angle     " << rightmost_angle << std::endl;
            std::cout << " Left most angle      " << leftmost_angle  << std::endl;
#endif
            leftmost_angle  *= (PI/180.);
            rightmost_angle *= (PI/180.);

            limited = true;
        }
    }

#if DEBUG_ANTENNA_ANGLE_SEARCH
    FILE *fpsearchangle;
    fpsearchangle = fopen("search_antenna_angle.txt","w");

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                fprintf(fpsearchangle,"The original antenna angle of the sector is %f\n",sector->antenna_angle_rad);
            }
        }
    }
#endif

    /********************************************************************************/
    /**** Profile search for fine optimal angle of the antenna                   ****/
    /********************************************************************************/
    if ( adjust_angles == 1 )
        if ( limited ) {
            if ( rightmost_angle >= 0 && rightmost_angle <= PI ) {
                div_angle = false;
                limit_ang = leftmost_angle - rightmost_angle;
                limit_range = (int) ceil ( limit_ang / profile_search_step );
            } else if ( rightmost_angle >= PI && rightmost_angle < 2.0*PI ) {
                if ( leftmost_angle >= 0 && leftmost_angle < PI ) {
                    div_angle = true;
                    limit_ang = 2.0*PI + leftmost_angle - rightmost_angle;
                    limit_range = (int) ceil ( limit_ang / profile_search_step );
                } else if ( leftmost_angle > PI && leftmost_angle < 2.0*PI ) {
                    div_angle = false;
                    limit_ang = leftmost_angle - rightmost_angle;
                    limit_range = (int) ceil ( limit_ang / profile_search_step );
                }
            }
            search_range = limit_range;
        } else {
            search_range = profile_search_range;
        }
    else
        search_range = 0;

    int idx = 0;
    for( ang_idx=0; ang_idx<search_range; ang_idx++) {
        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }
        }

            if ( !limited ) {
                adjust_sector->antenna_angle_rad = profile_search_step * ang_idx;
            } else {
                if ( div_angle ) {
                    if ( adjust_sector->antenna_angle_rad <= leftmost_angle ) {
                        adjust_sector->antenna_angle_rad = profile_search_step * ang_idx;
                    } else if ( adjust_sector->antenna_angle_rad > leftmost_angle &&  adjust_sector->antenna_angle_rad < 2.0*PI ) {
                        adjust_sector->antenna_angle_rad = rightmost_angle + profile_search_step * idx;
                        idx ++;
                    }
                } else {
                    adjust_sector->antenna_angle_rad = rightmost_angle + profile_search_step * idx;
                    idx ++;
                }
            }

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                if (logd >= min_logd_threshold) {
                    AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                    double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                    double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                    vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                    pt_idx           = pt_idx + 1;
                }
            }
        }

        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,0.0,error)) {
            if ( (minimum_rms == -1) || (sqrt(error/num_road_test_pt) < minimum_rms) ) {
                minimum_rms = sqrt(error/num_road_test_pt);
                profile_angle = adjust_sector->antenna_angle_rad;
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"profile search:  %15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /**** Fine search for fine optimal angle of the antenna                      ****/
    /********************************************************************************/
    bool div_up   = false;
    bool div_down = false;
    if ( (adjust_angles == 1) && (minimum_rms != -1) ) {
        search_range = fine_search_range;
        if ( profile_angle >= 0.0 && profile_angle < 10.*PI/180 ) div_up = true;
        else if ( profile_angle >= 350.*PI/180.0 && profile_angle < 2.*PI ) div_down = true;
        else { div_up = false; div_down = false; }
    } else {
        search_range = 0;
    }

    idx = 0;
    for( ang_idx=0; ang_idx<search_range; ang_idx++) {

        for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            scan_idx   = scan_index_list[scan_list_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            adjust_sector = sector;
            if( ang_idx == 0 ) {
                original_angle = adjust_sector->antenna_angle_rad;
            }
        }

        if ( div_up ) {
            if ( adjust_sector->antenna_angle_rad <= ( 10.*PI/180. + profile_angle ) ) {
                adjust_sector->antenna_angle_rad = fine_search_step * ang_idx;
            } else {
                idx ++;
                adjust_sector->antenna_angle_rad = 2.0*PI - fine_search_step * idx;
            }
        } else if ( div_down ) {
            if ( adjust_sector->antenna_angle_rad >= ( profile_angle - 10.*PI/180. ) ) {
                adjust_sector->antenna_angle_rad = 2.0*PI - fine_search_step * idx;
            } else {
                adjust_sector->antenna_angle_rad = fine_search_step * idx;
                idx ++;
            }
        } else
            adjust_sector->antenna_angle_rad = fine_search_step * (ang_idx - fine_search_range/2 ) + profile_angle ;

        pt_idx = 0;
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                if (logd >= min_logd_threshold) {
                    AntennaClass *antenna = antenna_type_list[adjust_sector->antenna_type];
                    double gain_db   = antenna->gainDB(dx, dy, dz, adjust_sector->antenna_angle_rad);
                    double tx_pwr_db = 10.0*log(adjust_sector->tx_pwr)/log(10.0);
                    vec_b[pt_idx]    = rtp->pwr_db - gain_db - tx_pwr_db;
                    pt_idx           = pt_idx + 1;
                }
            }
        }

        //first step optimization
        if (pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,0.0,error)) {

            if( sqrt(error/num_road_test_pt) < minimum_rms ) {
                minimum_rms   = sqrt(error/num_road_test_pt);
                optimal_angle = adjust_sector->antenna_angle_rad;
            }

#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"fine search:%15.10f\t", adjust_sector->antenna_angle_rad*180/PI);
            fprintf(fpsearchangle,"%15.10f\n", sqrt(error/num_road_test_pt));
#endif
        } else {
#if DEBUG_ANTENNA_ANGLE_SEARCH
            fprintf(fpsearchangle,"ERROR: When angle is %f, prop_model can not be computed\n",adjust_sector->antenna_angle_rad*180/PI);
#endif
        }
    }

    /********************************************************************************/
    /*** after search the optimal angle, begin to compute the propagation model  ****/
    /********************************************************************************/
    for( scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        scan_idx   = scan_index_list[scan_list_idx];
        cell_idx   = scan_idx & ((1<<bit_cell)-1);
        sector_idx = scan_idx >> bit_cell;
        cell = cell_list[cell_idx];
        sector = cell->sector_list[sector_idx];
        if ( adjust_angles == 1 ) {
            sector->antenna_angle_rad = optimal_angle;
        }
    }

    double avg_max_logd = 0.0;
    double* cell_max_logd = (double*)malloc(num_cell*sizeof(double));
    for(int i=0; i<num_cell; i++)
        cell_max_logd[i] = 0.0;

    pt_idx = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        if (inlist(scan_index_list, scan_idx, num_scan_index)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            dx = (rtp->posn_x - cell->posn_x) * resolution;
            dy = (rtp->posn_y - cell->posn_y) * resolution;
            dz = -sector->antenna_height;
            logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
            if (logd >= min_logd_threshold) {
                if(logd > cell_max_logd[cell_idx]) //tyc
                    cell_max_logd[cell_idx] = logd;
                    
                AntennaClass *antenna = antenna_type_list[sector->antenna_type];
                double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
                double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
                vec_b[pt_idx]   = rtp->pwr_db - gain_db - tx_pwr_db;
                pt_idx = pt_idx + 1;
            }
        }
    }

    double sum_max_logd = 0.0;
    for(int i=0; i<num_cell; i++)
        sum_max_logd += cell_max_logd[i];
    avg_max_logd = sum_max_logd / num_scan_index;    

#if 0
    char str[100];
    //sprintf(str,"roadtest_%d_%d.txt",scan_index_list[0]&((1<<bit_cell)-1),scan_index_list[0]>>bit_cell);
    sprintf(str,"roadtest.txt");
    FILE *fprtd = fopen(str,"w");

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        fprintf(fprtd,"%f %f\n",vec_logd[pt_idx],vec_b[pt_idx]);
    }

    fclose(fprtd);
#endif

    /*******************************************************************/
    /*** the first step optimization, when for the last operation,  ****/
    /*** should check the parameters                                ****/
    /*******************************************************************/
    if (!pm->fit_data(num_road_test_pt,vec_logd,vec_b,min_height,avg_max_logd,error)) {
        sprintf(msg, "%s: Problems creating prop_model for sector group (3).\n", (err ? "ERROR" : "WARNING"));
        PRMSG(stdout, msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
            sprintf(msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, msg);
        }
        sprintf(msg, "\n");
        PRMSG(stdout, msg);

        if ( adjust_angles == 1 ) {
            adjust_sector->antenna_angle_rad = original_angle;
        }

        if (err) {
            printf("error \n");
            error_state = 1;
        } else {
            warning_state = 1;
        }
        return;
    }

    // the second step optimization
    pm->optimize_clutter_coeffs(num_road_test_pt,mx_a,vec_b,vec_logd);

    // compute the error
    // error = pm->compute_error(num_road_test_pt,mx_a,vec_b,vec_logd, 1, "err_dist.txt");

    for (i=0; i<=num_road_test_pt-1; i++) {
        free(mx_a[i]);
    }
    free(mx_a);
    free(vec_b);
    free(vec_logd);
    free(dist_vector);

    if (!pm->adjust_near_field(min_logd, min_logd_threshold, frequency, msg, ignore_fs_check)) {
        PRMSG(stdout, msg);

        sprintf(msg, "%s: Problems creating prop_model for sector group (4).\n", (err ? "ERROR" : "WARNING"));
        PRMSG(stdout, msg);
        for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
            cell_idx   = scan_index_list[scan_list_idx] & ((1<<bit_cell)-1);
            sector_idx = scan_index_list[scan_list_idx] >> bit_cell;
            sprintf(msg, "%d_%d ", cell_idx, sector_idx);
            PRMSG(stdout, msg);
        }
        sprintf(msg, "\n");
        PRMSG(stdout, msg);

        if ( adjust_angles == 1 ) {
            adjust_sector->antenna_angle_rad = original_angle;
        }

        if (err) {
            printf("error \n");
            error_state = 1;
        } else {
            warning_state = 1;
        }
    }

#if DEBUG_ANTENNA_ANGLE_SEARCH
    fprintf(fpsearchangle,"Optimal antenna angle is %15.10f\t", optimal_angle);
    fprintf(fpsearchangle,"Minimum RMS ERROR is %15.10f\n", minimum_rms);
    fclose(fpsearchangle);
#endif

    //If it's not adjust the antenna angle, then at the end we condition the propagation model
    //with the test point which have no signal for the appoint test point.
#if 0
    if ( adjust_angles == 0 ) {
xxxxx use cutoff_slope in fit_data()
        //search the largest distance test point with signal
        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*road_test_data_list)[rtd_idx] );
            cell_idx   = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (inlist(scan_index_list, scan_idx, num_scan_index)) {
                cell = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                dx = (rtp->posn_x - cell->posn_x) * resolution;
                dy = (rtp->posn_y - cell->posn_y) * resolution;
                dz = -sector->antenna_height;
                double logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                if ( logd > max_logd ) {
                    max_logd = logd;
                    max_logd_power = pm->y[0] + pm->final_slope*(max_logd - pm->x[0]);
                }
            }
        }

        //search the smallest distance test point without signal
        double min_slope = pm->final_slope;
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            for (sector_idx=0; sector_idx<=cell_list[cell_idx]->num_sector-1; sector_idx++) {
                sector = cell_list[cell_idx]->sector_list[sector_idx];  //sector for test point
                scan_idx = (sector_idx << bit_cell) | cell_idx;

                if (!inlist(scan_index_list, scan_idx, num_scan_index)) {
                    for (i=0; i<num_scan_index; i++) {
                        cell = cell_list[scan_idx&((1<<bit_cell)-1)];  //cell in scan_index_list
                        for(rt_idx=0; rt_idx<sector->num_road_test_pt; rt_idx++) {
                            road_test_pt = sector->road_test_pt_list[rt_idx];
                            double dx = (road_test_pt->posn_x - cell->posn_x) * resolution;
                            double dy = (road_test_pt->posn_y - cell->posn_y) * resolution;
                            double dz = - cell->sector_list[scan_idx>>bit_cell]->antenna_height;
                            double logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
                            if ( (logd-max_logd)>0.001 ) {
                                AntennaClass *antenna = antenna_type_list[cell->sector_list[scan_idx>>bit_cell]->antenna_type];
                                double gain_db   = antenna->gainDB(dx, dy, dz, cell->sector_list[scan_idx>>bit_cell]->antenna_angle_rad);
                                double tx_pwr_db = 10.0*log(cell->sector_list[scan_idx>>bit_cell]->tx_pwr)/log(10.0);
                                double slope = (max_logd_power-(MIN_TEST_POWER-gain_db-tx_pwr_db))/(max_logd-logd);
                                if ( slope<min_slope ) {
                                    min_slope = slope;
                                }
                            }
                        }
                    }
                }
            }
        }

        if ( pm->final_slope > min_slope ) {
            pm->num_inflexion = 2;
            pm->final_slope = min_slope;
            pm->x = ( double *) realloc((void *) pm->x, pm->num_inflexion*sizeof(double));
            pm->y = ( double *) realloc((void *) pm->y, pm->num_inflexion*sizeof(double));
            pm->x[1] = max_logd;
            pm->y[1] = max_logd_power;
        }
    }
#endif
    // CG DBG
    /*
    printf("pm->num_clutter_type+2*pm->useheight = %d \n", pm->num_clutter_type+2*pm->useheight);
    for (int i=0; i<pm->num_clutter_type+2*pm->useheight;i++)
        printf("vec_k[%d] = %5.3f \n", i, pm->vec_k[i]);
     */


    num_prop_model++;
    prop_model_list = (PropModelClass **) realloc((void *) prop_model_list, (num_prop_model)*sizeof(PropModelClass *));
    prop_model_list[num_prop_model-1] = (PropModelClass *) pm;

    for (scan_list_idx=0; scan_list_idx<=num_scan_index-1; scan_list_idx++) {
        scan_idx = scan_index_list[scan_list_idx];
        cell_idx   = scan_idx & ((1<<bit_cell)-1);
        sector_idx = scan_idx >> bit_cell;
        cell_list[cell_idx]->sector_list[sector_idx]->prop_model = num_prop_model-1;
    }

#if DEBUG_COMP_PROP_MODEL
    time(&td);
    printf("%s\n", ctime(&td));
#endif

#endif
}

#undef CVX_HULL_DEBUG



void get_road_test_pt_posn(RoadTestPtClass *road_test_pt, int &posn_x, int &posn_y)
{
#if (DEMO == 0)
    posn_x = road_test_pt->posn_x;
    posn_y = road_test_pt->posn_y;
#endif
}
/******************************************************************************************/
/**** FUNCTION: SegmentPropModelClass::adjust_near_field                               ****/
/******************************************************************************************/

#define ADJUST_PM 1
#define PRINT_STD 0
int SegmentPropModelClass::adjust_near_field(double min_logd, double logr0, double frequency, char *msg, int ignore_fs_check)
{
#if ADJUST_PM
    int i;

    const double speed_of_light = 299792458.0;
    const double PL1_db = 20.0*log(speed_of_light / (4*PI*frequency))/log(10.0);
    const double PL0_db = PL1_db - 20*logr0;

    double p2, d2;

// CG DBG - show propagation model parameters
/**************************************************
#if PRINT_STD
    if ( get_strid() != NULL )
        printf("adjust_near_field() %s \n", get_strid());
    printf("\nmin_logd    = %8.5f \n", min_logd  );
    printf("logr0       = %8.5f \n", logr0       );
    printf("PL0_db      = %8.5f \n", PL0_db      );
    printf("PL1_db      = %8.5f \n", PL1_db      );
    printf("start_slope = %8.5f \n", start_slope );
    printf("x[0]        = %8.5f \n", x[0]        );
    printf("y[0]        = %8.5f \n", y[0]        );
    printf("x[1]        = %8.5f \n", x[1]        );
    printf("y[1]        = %8.5f \n", y[1]        );

    double dd = x[0];
    double Y1 = y[0] + start_slope*(dd-x[0]);
    double Y2 = PL1_db - 20*dd;

    printf("Our PM      = %8.5f \n", Y1);
    printf("Free space  = %8.5f \n", Y2);
#else
    if ( get_strid() != NULL )
    {
        sprintf(msg, "\nadjust_near_field() %s \n", get_strid());
        PRMSG(stdout, msg);
    }
    sprintf(msg, "\nmin_logd    = %8.5f \n", min_logd  );
    PRMSG(stdout, msg);

    sprintf(msg, "logr0       = %8.5f \n", logr0       );
    PRMSG(stdout, msg);

    sprintf(msg, "PL0_db      = %8.5f \n", PL0_db      );
    PRMSG(stdout, msg);

    sprintf(msg, "PL1_db      = %8.5f \n", PL1_db      );
    PRMSG(stdout, msg);

    sprintf(msg, "start_slope = %8.5f \n", start_slope );
    PRMSG(stdout, msg);

    sprintf(msg, "x[0]        = %8.5f \n", x[0]        );
    PRMSG(stdout, msg);

    sprintf(msg, "y[0]        = %8.5f \n", y[0]        );
    PRMSG(stdout, msg);

    sprintf(msg, "x[1]        = %8.5f \n", x[1]        );
    PRMSG(stdout, msg);

    sprintf(msg, "y[1]        = %8.5f \n", y[1]        );
    PRMSG(stdout, msg);

    double dd = x[0];
    double Y1 = y[0] + start_slope*(dd-x[0]);
    double Y2 = PL1_db - 20*dd;

    sprintf(msg, "Our PM      = %8.5f \n", Y1);
    PRMSG(stdout, msg);

    sprintf(msg, "Free space  = %8.5f \n", Y2);
    PRMSG(stdout, msg);
#endif
***************************************************/

    if (min_logd < logr0) {
        sprintf(msg, "WARNING: min_logd < logr0 in adjust_near_field()\n");
        return(0);
    }

    if (x[0] < min_logd) {
        sprintf(msg, "WARNING: \"%s\" x[0] < min_logd in adjust_near_field()\n", get_strid());
        return(0);
    }

    /*
    if (!ignore_fs_check) {
        if (y[0] + start_slope*(min_logd-x[0]) > PL1_db - 20*min_logd) {
            sprintf(msg, "WARNING: \"%s\" power exceeds that of free-space model\n", get_strid());
            return(0);
        }
    }
    */

    d2 = (min_logd + logr0)/2;
    if (y[0] + start_slope*(d2-x[0]) > PL1_db - 20*d2) {
        d2 = (PL1_db-y[0]+start_slope*x[0]) / (start_slope+20.0);
    }
    p2 = y[0] + start_slope*(d2-x[0]);

    num_inflexion+=2;
    x = ( double *) realloc((void *) x, num_inflexion*sizeof(double));
    y = ( double *) realloc((void *) y, num_inflexion*sizeof(double));
    for (i=num_inflexion-1; i>=2; i--) {
        x[i] = x[i-2];
        y[i] = y[i-2];
    }

    x[1] = d2;
    y[1] = p2;
    x[0] = logr0;
    y[0] = PL0_db;
    start_slope = 0.0;

// CG DBG - show PM parameters after adjusting near field
/***************************************************
    double slop = 0.;
#if PRINT_STD
    printf("\nstart_slope = %8.5f \n", start_slope );
#else
    sprintf(msg, "\nstart_slope = %8.5f \n", start_slope );
    PRMSG(stdout, msg);
#endif
    for (i=0;i<num_inflexion;i++) {
#if PRINT_STD
        printf("(x[%d], y[%d]) = (%6.3f, %6.3f) \n", i, i, x[i], y[i]);
#else
        sprintf(msg, "(x[%d], y[%d]) = (%6.3f, %6.3f) \n", i, i, x[i], y[i]);
        PRMSG(stdout, msg);
#endif
        if (i<num_inflexion-1)
        {
            slop = (y[i+1]-y[i])/(x[i+1]-x[i]);
            //printf("x[%d] = %6.3f, x[%d] = %6.3f, y[%d] = %6.3f, y[%d] = %6.3f \n",
            //        i+1, x[i+1], i, x[i], i+1, y[i+1], i, y[i]);
#if PRINT_STD
            printf("SLOPE[%d] = %8.5f \n", i+1, slop);
#else
            sprintf(msg, "SLOPE[%d] = %8.5f \n", i+1, slop);
            PRMSG(stdout, msg);
#endif

            if (slop > 0) {
                sprintf(msg, "WARNING: \"%s\" segment %d slope > 0 \n", get_strid(), i+2);
                PRMSG(stdout, msg);
            }
        }
    }
#if PRINT_STD
    printf("final_slope = %8.5f \n", final_slope );
#else
    sprintf(msg, "final_slope = %8.5f \n", final_slope );
    PRMSG(stdout, msg);
#endif
    printf("--------------------------------\n");
***************************************************/

#endif

    return(1);
}
/******************************************************************************************/



/*
*****************************************************************************************
* FUNCTION: svd_solve
* mx_a (M x N) vec_x (N) vec_b (M)
* Use singular value decomposition to solve matrix equation: Ax=b.
***************************************************************************************
*/
double svd_solve(double **mx_a, double *vec_b, int M, int N, double *vec_x, int show_prog)
{
#if (DEMO == 0)
    int i, j, k, n;
    double **mx_u, **mx_v, **mx_ainv, *diag_w, error, val;

    // CG DBG
#if (HAS_GUI && show_prog)
    ProgressSlot* prog_bar;
    int curr_prog = 0;
    prog_bar = new ProgressSlot(0, "Progress Bar", qApp->translate("ProgressSlot", "Running Clutter Simulation - SVD Solve ") + "...");
#endif

    /*
    **********************************************************************************
    * Allocate Vectors
    **********************************************************************************
    */
    mx_u    = (double **) malloc(M*sizeof(double *));
    mx_v    = (double **) malloc(N*sizeof(double *));
    mx_ainv = (double **) malloc(N*sizeof(double *));

    for (i=0; i<=M-1; i++) {
        mx_u[i] = DVECTOR(N);
    }

    for (i=0; i<=N-1; i++) {
        mx_v[i] = DVECTOR(N);
        mx_ainv[i] = DVECTOR(M);
    }

    diag_w = DVECTOR(N);


    /*
    **********************************************************************************
    * Copy mx_a to mx_u
    **********************************************************************************
    */
    for (i=0; i<=M-1; i++) {
        for (j=0; j<=N-1; j++) {
            mx_u[i][j] = mx_a[i][j];
        }
    }

#if (HAS_GUI && show_prog)
    curr_prog = 15;
    prog_bar->set_prog_percent(curr_prog);
#endif

    /*
    **********************************************************************************
    * Perform Singular value decomposition
    **********************************************************************************
    */
    my_svdcmpz(mx_u, M, N, diag_w, mx_v);

#if (HAS_GUI && show_prog)
    curr_prog = 50;
    prog_bar->set_prog_percent(curr_prog);
#endif

    /*
    **********************************************************************************
    * Compute pseudo-inverse of mx_a[][]
    **********************************************************************************
    */
    for (n=0; n<=N-1; n++) {
        for (j=0; j<=N-1; j++) {
            if (fabs(diag_w[j]) > 1.0e-8) {
                mx_v[n][j] /= diag_w[j];
            } else {
                mx_v[n][j] = 0.0;
            }
        }
    }

#if (HAS_GUI && show_prog)
    curr_prog = 60;
    prog_bar->set_prog_percent(curr_prog);
#endif

    for (n=0; n<=N-1; n++) {
        for (j=0; j<=M-1; j++) {
            mx_ainv[n][j] = 0.0;
            for (k=0; k<=N-1; k++) {
                mx_ainv[n][j] += mx_v[n][k]*mx_u[j][k];
            }
        }
    }

#if (HAS_GUI && show_prog)
    curr_prog = 70;
    prog_bar->set_prog_percent(curr_prog);
#endif

    /*
    **********************************************************************************
    * Compute vec_x = mx_ainv * vec_b
    **********************************************************************************
    */
    for (n=0; n<=N-1; n++) {
        vec_x[n] = 0.0;
        for (j=0; j<=M-1; j++) {
            vec_x[n] += mx_ainv[n][j]*vec_b[j];
        }
    }


#if (HAS_GUI && show_prog)
    curr_prog = 80;
    prog_bar->set_prog_percent(curr_prog);
#endif

    /*
    **********************************************************************************
    * Compute error = |Ax-b|^2
    **********************************************************************************
    */
    error = 0.0;
    for (j=0; j<=M-1; j++) {
        val = 0.0;
        for (n=0; n<=N-1; n++) {
            val += mx_a[j][n]*vec_x[n];
        }
        val -= vec_b[j];
        error += val*val;
    }

#if (HAS_GUI && show_prog)
    curr_prog = 90;
    prog_bar->set_prog_percent(curr_prog);
#endif

    /*
    **********************************************************************************
    * Free Vectors
    **********************************************************************************
    */
    for (i=0; i<=M-1; i++) {
        free(mx_u[i]);
    }

    for (i=0; i<=N-1; i++) {
        free(mx_v[i]);
        free(mx_ainv[i]);
    }

    free(mx_u);
    free(mx_v);
    free(mx_ainv);
    free(diag_w);

#if (HAS_GUI && show_prog)
    curr_prog = 100;
    prog_bar->set_prog_percent(curr_prog);
#endif

#if (HAS_GUI && show_prog)
    delete prog_bar;
#endif
    return(error);
#else
    return(0.0);
#endif
}


/*
******************************************************************
* FUNCTION: comp_prop_error
* Compute RMS error betweed propagaion model and road test data.
******************************************************************
*/
void NetworkClass::comp_prop_error(char *filename)
{
#if (DEMO == 0)
    int i, cell_idx, sector_idx, scan_idx, rtd_idx, pm_idx;
    double power, power_db;
    double total_error = 0.0;
    CellClass *cell;
    SectorClass *sector;
    RoadTestPtClass *rtp;
    PropModelClass *pm;
    ListClass<int> *scan_index_list = new ListClass<int>(num_cell);
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        CellClass *cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            scan_index_list->append(scan_idx);
        }
    }

    double *error_list   = DVECTOR(scan_index_list->getSize());
    int    *num_rtp_list = IVECTOR(scan_index_list->getSize());

    for (i=0; i<=scan_index_list->getSize()-1; i++) {
        error_list[i] = 0.0;
        num_rtp_list[i] = 0;
    }
    total_error = 0.0;

    PolygonClass **map_bdy = (PolygonClass **) malloc(num_prop_model*sizeof(PolygonClass *));
    for ( pm_idx=0; pm_idx<num_prop_model; pm_idx++ ) {
        pm = prop_model_list[pm_idx];
        if ( pm->is_clutter_model() ) {
            map_bdy[pm_idx] = ((GenericClutterPropModelClass *)pm)->create_map_bdy();
        } else {
            map_bdy[pm_idx] = (PolygonClass *) NULL;
        }
    }

    int num_insys_rtp = 0;
    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        cell = cell_list[cell_idx];
        sector = cell->sector_list[sector_idx];
        i = scan_index_list->get_index(scan_idx);
        pm_idx = sector->prop_model;

        if (pm_idx != -1) {
            if ( (!map_bdy[pm_idx]) || (map_bdy[pm_idx]->in_bdy_area(rtp->posn_x, rtp->posn_y)) ) {
                power = sector->tx_pwr*sector->comp_prop(this, rtp->posn_x, rtp->posn_y);
                power_db = 10.0*log(power)/log(10.0);

                error_list[i] += (power_db - rtp->pwr_db)*(power_db - rtp->pwr_db);
                total_error   += (power_db - rtp->pwr_db)*(power_db - rtp->pwr_db);
                num_rtp_list[i]++;
                num_insys_rtp ++;
            }
        }
    }

    for ( pm_idx=0; pm_idx<num_prop_model; pm_idx++ ) {
        if (map_bdy[pm_idx]) {
            delete map_bdy[pm_idx];
        }
    }
    free(map_bdy);

    char *chptr;
    char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
    for (i=0; i<=scan_index_list->getSize()-1; i++) {
        scan_idx = (*scan_index_list)[i];
        cell_idx   = scan_idx & ((1<<bit_cell)-1);
        sector_idx = scan_idx >> bit_cell;
        cell = cell_list[cell_idx];
        sector = cell->sector_list[sector_idx];
        if ( technology() == CConst::PHS )
            hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
        chptr = msg;
        chptr += sprintf(chptr, "(%d) CELL %3d SECTOR %3d CSID %s: NUM_ROAD_TEST_PT: %4d ",
                         i, cell_idx, sector_idx, hexstr, num_rtp_list[i]);
        if (sector->prop_model == -1) {
            chptr += sprintf(chptr, "PROP_MODEL UNASSIGNED\n");
        } else {
            chptr += sprintf(chptr, "RMS_ERROR: %8.5f dB\n",
                             (num_rtp_list[i] ? sqrt(error_list[i]/num_rtp_list[i]) : 0.0));
        }
        PRMSG(fp, msg);
    }
    free(hexstr);

    sprintf(msg, "\nTOTAL: NUM_ROAD_TEST_PT: %d RMS_ERROR: %8.5f dB\n",
            num_insys_rtp, (num_insys_rtp ? sqrt(total_error/num_insys_rtp) : 0.0));
    PRMSG(fp, msg);

    free(error_list);
    free(num_rtp_list);
    delete scan_index_list;

    if (fp != stdout) {
        fclose(fp);
    }
#endif
}


/******************************************************************************************/
/**** FUNCTION: set_unassigned_prop_model                                              ****/
/**** Set prop_model for all sectors that do not have an prop_model assigned.  The     ****/
/**** algorithm used is to select the sector with an antenna height closest to the     ****/
/**** sector under consideration, and use the corresponding prop_model.                ****/
/******************************************************************************************/
void NetworkClass::set_unassigned_prop_model()
{
#if (DEMO == 0)
    int num_assigned, num_unassigned;
    int cell_idx, sector_idx, scan_idx;
    int u_cell_idx, u_sector_idx;
    int sel_cell_idx, sel_sector_idx;
    int idx_u, idx_a, use;

    double h, h_sel, dsq, dsq_sel;

    CellClass *cell, *u_cell, *sel_cell;
    SectorClass *sector, *u_sector, *sel_sector;

    num_assigned = 0;
    num_unassigned = 0;

    for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if (sector->prop_model == -1) {
                num_unassigned++;
            } else {
                num_assigned++;
            }
        }
    }

    if (num_unassigned == 0) {
        sprintf(msg, "WARNING: NO SECTORS WITH UNASSIGNED PROP_MODEL FOUND.\n");
        PRMSG(stdout, msg); warning_state = 1;
    } else if (num_assigned == 0) {
        sprintf(msg, "ERROR: ALL SECTORS HAVE UNASSIGNED PROP_MODEL.\n");
        PRMSG(stdout, msg); error_state = 1;
    } else {
        int *a_list = IVECTOR(num_assigned);
        int *u_list = IVECTOR(num_unassigned);

        num_assigned = 0;
        num_unassigned = 0;
        for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                scan_idx = (sector_idx << bit_cell) | cell_idx;
                if (sector->prop_model == -1) {
                    u_list[num_unassigned++] = scan_idx;
                } else {
                    a_list[num_assigned++] = scan_idx;
                }
            }
        }

        for (idx_u=0; idx_u<=num_unassigned-1; idx_u++) {
            u_cell_idx   = u_list[idx_u] & ((1<<bit_cell)-1);
            u_sector_idx = u_list[idx_u] >> bit_cell;
            u_cell   = cell_list[u_cell_idx];
            u_sector = u_cell->sector_list[u_sector_idx];

            sel_cell_idx   = a_list[0] & ((1<<bit_cell)-1);
            sel_sector_idx = a_list[0] >> bit_cell;
            sel_cell   = cell_list[sel_cell_idx];
            sel_sector = sel_cell->sector_list[sel_sector_idx];
            for (idx_a=1; idx_a<=num_assigned-1; idx_a++) {
                cell_idx   = a_list[idx_a] & ((1<<bit_cell)-1);
                sector_idx = a_list[idx_a] >> bit_cell;
                cell   = cell_list[cell_idx];
                sector = cell->sector_list[sector_idx];
                h     = fabs(    sector->antenna_height - u_sector->antenna_height);
                h_sel = fabs(sel_sector->antenna_height - u_sector->antenna_height);

                use = 0;
                if ( h < h_sel ) {
                    use = 1;
                } else if (h == h_sel) {
                    dsq = (cell->posn_x-u_cell->posn_x)*(cell->posn_x-u_cell->posn_x)
                        + (cell->posn_y-u_cell->posn_y)*(cell->posn_y-u_cell->posn_y);

                    dsq_sel = (sel_cell->posn_x-u_cell->posn_x)*(sel_cell->posn_x-u_cell->posn_x)
                            + (sel_cell->posn_y-u_cell->posn_y)*(sel_cell->posn_y-u_cell->posn_y);
                    if ( dsq < dsq_sel ) {
                        use = 1;
                    }
                }
                if (use) {
                    sel_cell_idx   = cell_idx;
                    sel_sector_idx = sector_idx;
                    sel_cell   = cell_list[sel_cell_idx];
                    sel_sector = sel_cell->sector_list[sel_sector_idx];
                }
            }
            u_sector->prop_model = sel_sector->prop_model;
            sprintf(msg, "CELL %d SECTOR %d ASSIGNED PROP_MODEL %d (SAME AS CELL %d SECTOR %d)\n",
                     u_cell_idx, u_sector_idx, u_sector->prop_model,
                     sel_cell_idx, sel_sector_idx);
            PRMSG(stdout, msg);
        }

        free(a_list);
        free(u_list);
    }
#endif
    return;
}

/*
**************************************************************************************
* FUNCTION: NetworkCLass::shift_road_test_data
**************************************************************************************
*/
void NetworkClass::shift_road_test_data(double shift_x, double shift_y)
{
#if (DEMO == 0)
    int delta_x, delta_y;

    check_grid_val(shift_x, resolution, 0, &delta_x);
    check_grid_val(shift_y, resolution, 0, &delta_y);

    shift_road_test_data(delta_x, delta_y);

#endif
}

void NetworkClass::shift_road_test_data(int delta_x, int delta_y)
{
#if (DEMO == 0)
    int rtd_idx;
    RoadTestPtClass *rtp;

    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        rtp->posn_x += delta_x;
        rtp->posn_y += delta_y;
    }
#endif
}

/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::fit_data()                                 ****/
/******************************************************************************************/
int SegmentAnglePropModelClass::fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double &error, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    int i;                 //loop variable
    int pt_idx;            //test point loop variable
    int var_idx;           //test point loop variable
    int found;             //found opmized flag. 0 unfound;1 found

    double avg_val;
    double tol;            //error tolerance
    double dmin;           //the minimum distance of the test point
    double dmax;           //the maximun distance of the test point
    double val_y;          //used for err_fn_of_x

    double xmin,f,fmin;
    double ax,bx,cx;
    double fa,fb,fc;

    double x0,x1,x2,x3;    //the distance value for fine estimate
    double f1,f2;          //the error value for fine estimate

    double *vec_x;         //vector save the result of the SVD
    double *vec_s;         //vector save the result of the SVD when compute single propagation model
    vec_x = DVECTOR(3*(n_angle+1));
    vec_s = DVECTOR(2*(n_angle+1));

    ax = 0;
    bx = 0;
    cx = 0;
    fb = 0;

    // Search the minimum and maximum of the log distance
    dmin = dmax = vec_logd[0];
    for (pt_idx=1; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] < dmin) {
            dmin = vec_logd[pt_idx];
        }

        if (vec_logd[pt_idx] > dmax) {
            dmax = vec_logd[pt_idx];
        }
    }


    // debug the profile range search
#if 0
    FILE *fppre = fopen("pre.txt","w");

    ax = dmin + (dmax-dmin)*4/50.0;
    fa = err_fn_of_x(num_road_test_pt, ax, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    bx = dmin + (dmax-dmin)*5/50.0;
    fb = err_fn_of_x(num_road_test_pt, bx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    found = 0;
    for (i=6; (i<=46)&&(!found); i++)
    {
        cx = dmin + (dmax-dmin)*i/50.0;
        fc = err_fn_of_x(num_road_test_pt, cx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        fprintf(fppre,"%f %f\n",cx,fc);
    }

    fclose(fppre);
#endif

    //profile search the value of the inflexion
    xmin = dmin;
    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

#define PRECISE 50
    // FILE *fppre = fopen("pre.txt","w");
    for(i=1;i<=PRECISE;i++) {
        val_y = dmin + (dmax-dmin)*i/PRECISE;
        f = err_fn_of_x(num_road_test_pt, val_y, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        if( (fmin - f) > 1E-8 ) {
            fmin = f;
            xmin = val_y;
        }
        // fprintf(fppre,"%f %f\n",val_y,f);
    }
    // fclose(fppre);

    if ( (xmin < dmin*0.9 + dmax*0.1) || (xmin > dmin*0.1 + dmax*0.9) ) {
        found = 0; //can not search the inflexion
    } else {
        bx = xmin;
        fb = fmin;

        ax = xmin - (dmax-dmin)/PRECISE;
        fa = err_fn_of_x(num_road_test_pt, ax, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

        cx = xmin + (dmax-dmin)/PRECISE;
        fc = err_fn_of_x(num_road_test_pt, cx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

        if( fb<fa && fb<fc ) {
            found = 1;
        } else {
            found = 0;
        }
    }

#undef PRECISE

    // If cannot find the minimum value,then use the single propagation model,which assume the
    // slope of the first part is zero.
    if (!found)
    {
        fmin = err_fn_of_s2( num_road_test_pt, vec_logd, vec_s, vec_b, min_height, vec_cos, vec_sin);

        var_idx = 0;
        x[0] = log(min_height)/log(10.0);
        avg_val = vec_s[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            y[0][i] = avg_val + vec_s[var_idx++];
        }
        avg_val = vec_s[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            start_slope[i] = avg_val + vec_s[var_idx++];
            final_slope[i] = start_slope[i];
        }

#if 0
        double s2_max = -8.0;
        if ( final_slope >= s2_max ) {
            //printf("ERROR: profile search,Singular linear model final_slope = %7.5f >= %7.5f\n", final_slope, s2_max);
            error = -1.0;
            free(vec_x);
            free(vec_s);
            return(0);
        }
#endif

        error = fmin;
        free(vec_x);
        free(vec_s);
        return(1);
    }

    // search the opimized value for fine range
    #define R 0.61803399
    #define C (1.0-R)
    #define SHFT2(a,b,c) (a)=(b);(b)=(c);
    #define SHFT3(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

    x0 = ax;
    x3 = cx;
    x2 = bx;

    x1 = bx-C*(bx-ax);
    f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
    f2 = fb;

    tol = 1.0e-6;
    while (fabs(x3-x0) > tol*(fabs(x1)+fabs(x2))) {
        if (f2 < f1) {
            SHFT3(x0,x1,x2,R*x1+C*x3)
            f1 = f2;
            f2 = err_fn_of_x(num_road_test_pt, x2, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        } else {
            SHFT3(x3,x2,x1,R*x2+C*x0)
            f2 = f1;
            f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        }
    }

    if (f1 < f2) {
        xmin=x1;
    } else {
        xmin=x2;
    }
    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    #undef C
    #undef R
    #undef SHFT2
    #undef SHFT3

    /*
    ********************************************************************************************************
    * If val_s1 or val_2 of the propagation model is not proper, then use singular linear model to replace
    * the two segment line model. The new model which val_y is fixed to log(h) and val_s1 is fixed to zero.
    ********************************************************************************************************
    */
    double s1_max = 0.0;
    double s2_max = -0.1;
    if ( /* vec_x[2+2*(n_series_y[0]+n_series_start_slope)]>s2_max || vec_x[1+2*n_series_y[0]]>s1_max */ 0 ) {
        //printf("WARNING: After fine search,start_slope=%f or final_slope=%f is not proper!\n",start_slope,final_slope);

        fmin = err_fn_of_s2(num_road_test_pt,vec_logd,vec_s,vec_b,min_height, vec_cos, vec_sin);

        var_idx = 0;
        x[0] = log(min_height)/log(10.0);
        avg_val = vec_s[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            y[0][i] = avg_val + vec_s[var_idx++];
        }
        avg_val = vec_s[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            start_slope[i] = avg_val + vec_s[var_idx++];
            final_slope[i] = start_slope[i];
        }

        for (i=0; i<=n_angle-1; i++) {
            if ( final_slope[i] >= s2_max ) {
                //printf("ERROR: profile search,Singular linear model final_slope = %7.5f >= %7.5f\n", final_slope, s2_max);
                error = -1.0;
                free(vec_x);
                free(vec_s);
                return(0);
            }
        }

        error = fmin;
    } else {

        var_idx = 0;
        x[0] = xmin;
        avg_val = vec_x[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            y[0][i] = avg_val + vec_x[var_idx++];
        }
        avg_val = vec_x[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            start_slope[i] = avg_val + vec_x[var_idx++];
        }
        avg_val = vec_x[var_idx++];
        for (i=0; i<=n_angle-1; i++) {
            final_slope[i] = avg_val + vec_x[var_idx++];
        }

        error = fmin;
    }

    free(vec_x);
    free(vec_s);
#endif
    return(1);
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::fit_data()                             ****/
/******************************************************************************************/
int SegmentWithThetaPropModelClass::fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double &error, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    int i;                 //loop variable
    int pt_idx;            //test point loop variable
    int var_idx;           //test point loop variable
    int found;             //found opmized flag. 0 unfound;1 found

    double tol;            //error tolerance
    double dmin;           //the minimum distance of the test point
    double dmax;           //the maximun distance of the test point
    double val_y;          //used for err_fn_of_x

    double xmin,f,fmin;
    double ax,bx,cx;
    double fa,fb,fc;

    double x0,x1,x2,x3;    //the distance value for fine estimate
    double f1,f2;          //the error value for fine estimate

    double *vec_x;         //vector save the result of the SVD
    double *vec_s;         //vector save the result of the SVD when compute single propagation model
    vec_x = DVECTOR(3 + 2*(n_series_y[0] + n_series_start_slope + n_series_final_slope));
    vec_s = DVECTOR(2 + 2*(n_series_y[0] + n_series_start_slope));

    ax = 0;
    bx = 0;
    cx = 0;
    fb = 0;

    // Search the minimum and maximum of the log distance
    dmin = dmax = vec_logd[0];
    for (pt_idx=1; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] < dmin) {
            dmin = vec_logd[pt_idx];
        }

        if (vec_logd[pt_idx] > dmax) {
            dmax = vec_logd[pt_idx];
        }
    }


    // debug the profile range search
#if 0
    FILE *fppre = fopen("pre.txt","w");

    ax = dmin + (dmax-dmin)*4/50.0;
    fa = err_fn_of_x(num_road_test_pt, ax, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    bx = dmin + (dmax-dmin)*5/50.0;
    fb = err_fn_of_x(num_road_test_pt, bx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    found = 0;
    for (i=6; (i<=46)&&(!found); i++)
    {
        cx = dmin + (dmax-dmin)*i/50.0;
        fc = err_fn_of_x(num_road_test_pt, cx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        fprintf(fppre,"%f %f\n",cx,fc);
    }

    fclose(fppre);
#endif

    //profile search the value of the inflexion
    xmin = dmin;
    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

#define PRECISE 50
    // FILE *fppre = fopen("pre.txt","w");
    for(i=1;i<=PRECISE;i++) {
        val_y = dmin + (dmax-dmin)*i/PRECISE;
        f = err_fn_of_x(num_road_test_pt, val_y, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        if( (fmin - f) > 1E-8 ) {
            fmin = f;
            xmin = val_y;
        }
        // fprintf(fppre,"%f %f\n",val_y,f);
    }
    // fclose(fppre);

    if ( (xmin < dmin*0.9 + dmax*0.1) || (xmin > dmin*0.1 + dmax*0.9) ) {
        found = 0; //can not search the inflexion
    } else {
        bx = xmin;
        fb = fmin;

        ax = xmin - (dmax-dmin)/PRECISE;
        fa = err_fn_of_x(num_road_test_pt, ax, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

        cx = xmin + (dmax-dmin)/PRECISE;
        fc = err_fn_of_x(num_road_test_pt, cx, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

        if( fb<fa && fb<fc ) {
            found = 1;
        } else {
            found = 0;
        }
    }

#undef PRECISE

    // If cannot find the minimum value,then use the single propagation model,which assume the
    // slope of the first part is zero.
    if (!found)
    {
        fmin = err_fn_of_s2( num_road_test_pt, vec_logd, vec_s, vec_b, min_height, vec_cos, vec_sin);

        var_idx = 0;
        x[0] = log(min_height)/log(10.0);
        y[0] = vec_s[var_idx++];
        for (i=0; i<=n_series_y[0]-1; i++) {
            c_series_y[0][i] = vec_s[var_idx++];
            s_series_y[0][i] = vec_s[var_idx++];
        }
        start_slope = vec_s[var_idx++];
        final_slope = start_slope;
        for (i=0; i<=n_series_start_slope-1; i++) {
            c_series_start_slope[i] = vec_s[var_idx++];
            s_series_start_slope[i] = vec_s[var_idx++];
            c_series_final_slope[i] = c_series_start_slope[i];
            s_series_final_slope[i] = s_series_start_slope[i];
        }

#if 0
        double s2_max = -8.0;
        if ( final_slope >= s2_max ) {
            //printf("ERROR: profile search,Singular linear model final_slope = %7.5f >= %7.5f\n", final_slope, s2_max);
            error = -1.0;
            return(0);
        }
#endif

        error = fmin;
        return(1);
    }

    // search the opimized value for fine range
    #define R 0.61803399
    #define C (1.0-R)
    #define SHFT2(a,b,c) (a)=(b);(b)=(c);
    #define SHFT3(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

    x0 = ax;
    x3 = cx;
    x2 = bx;

    x1 = bx-C*(bx-ax);
    f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
    f2 = fb;

    tol = 1.0e-6;
    while (fabs(x3-x0) > tol*(fabs(x1)+fabs(x2))) {
        if (f2 < f1) {
            SHFT3(x0,x1,x2,R*x1+C*x3)
            f1 = f2;
            f2 = err_fn_of_x(num_road_test_pt, x2, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        } else {
            SHFT3(x3,x2,x1,R*x2+C*x0)
            f2 = f1;
            f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b, vec_cos, vec_sin);
        }
    }

    if (f1 < f2) {
        xmin=x1;
    } else {
        xmin=x2;
    }
    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b, vec_cos, vec_sin);

    #undef C
    #undef R
    #undef SHFT2
    #undef SHFT3

    /*
    ********************************************************************************************************
    * If val_s1 or val_2 of the propagation model is not proper, then use singular linear model to replace
    * the two segment line model. The new model which val_y is fixed to log(h) and val_s1 is fixed to zero.
    ********************************************************************************************************
    */
    double s1_max = 0.0;
    double s2_max = -0.1;
    if ( /* vec_x[2+2*(n_series_y[0]+n_series_start_slope)]>s2_max || vec_x[1+2*n_series_y[0]]>s1_max */ 0 ) {
        //printf("WARNING: After fine search,start_slope=%f or final_slope=%f is not proper!\n",start_slope,final_slope);

        fmin = err_fn_of_s2(num_road_test_pt,vec_logd,vec_s,vec_b,min_height, vec_cos, vec_sin);

        var_idx = 0;
        x[0] = log(min_height)/log(10.0);
        y[0] = vec_s[var_idx++];
        for (i=0; i<=n_series_y[0]-1; i++) {
            c_series_y[0][i] = vec_s[var_idx++];
            s_series_y[0][i] = vec_s[var_idx++];
        }
        start_slope = vec_s[var_idx++];
        final_slope = start_slope;
        for (i=0; i<=n_series_start_slope-1; i++) {
            c_series_start_slope[i] = vec_s[var_idx++];
            s_series_start_slope[i] = vec_s[var_idx++];
            c_series_final_slope[i] = c_series_start_slope[i];
            s_series_final_slope[i] = s_series_start_slope[i];
        }

        if ( final_slope >= s2_max ) {
            //printf("ERROR: profile search,Singular linear model final_slope = %7.5f >= %7.5f\n", final_slope, s2_max);
            error = -1.0;
            return(0);
        }

        error = fmin;
    } else {

        var_idx = 0;
        x[0] = xmin;
        y[0] = vec_x[var_idx++];
        for (i=0; i<=n_series_y[0]-1; i++) {
            c_series_y[0][i] = vec_x[var_idx++];
            s_series_y[0][i] = vec_x[var_idx++];
        }
        start_slope = vec_x[var_idx++];
        for (i=0; i<=n_series_start_slope-1; i++) {
            c_series_start_slope[i] = vec_x[var_idx++];
            s_series_start_slope[i] = vec_x[var_idx++];
        }
        final_slope = vec_x[var_idx++];
        for (i=0; i<=n_series_final_slope-1; i++) {
            c_series_final_slope[i] = vec_x[var_idx++];
            s_series_final_slope[i] = vec_x[var_idx++];
        }

        error = fmin;
    }

    free(vec_x);
    free(vec_s);
#endif
    return(1);
}

/*
*****************************************************************
* FUNCTION: SegmentPropModelClass::fit_data
* the first step opmization and get the parameter of N, s, f and
* point pair x and y.
*****************************************************************
*/
int SegmentPropModelClass::fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double avg_max_logd, double &error)
{
    /****************************************************************************
     * CG CMT 11/24/2006 XXXXX
     * Need to rewrite this section later:
     *    - not simplified replace two slope model with single slope model,
     *      if the there are not proper PM parameters
     *    - use SVD to traverse to find suboptimun serial of parameters
     *    - delete some unwanted code
     ****************************************************************************/

#if (DEMO == 0)
    int i;                 //loop variable
    int pt_idx;            //test point loop variable
    int found;             //found opmized flag. 0 unfound;1 found

    double tol;            //error tolerance
    double dmin;           //the minimum distance of the test point
    double dmax;           //the maximun distance of the test point
    double val_y;          //used for err_fn_of_x

    double xmin,f,fmin;
    double ax,bx,cx;
    double fa,fb,fc;

    double x0,x1,x2,x3;    //the distance value for fine estimate
    double f1,f2;          //the error value for fine estimate

    double *vec_x;         //vector save the result of the SVD
    double *vec_s;         //vector save the result of the SVD when compute single propagation model
    vec_x = DVECTOR(3);
    vec_s = DVECTOR(2);

    ax = 0;
    bx = 0;
    cx = 0;
    fb = 0;

    // Search the minimum and maximum of the log distance
    dmin = dmax = vec_logd[0];
    for (pt_idx=1; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] < dmin) {
            dmin = vec_logd[pt_idx];
        }

        if (vec_logd[pt_idx] > dmax) {
            dmax = vec_logd[pt_idx];
        }
    }

    //profile search the value of the inflexion
    xmin = dmin;

    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b);

#define PRECISE 50
    // FILE *fppre = fopen("pre.txt","w");
    for(i=1;i<=PRECISE;i++) {
        val_y = dmin + (dmax-dmin)*i/PRECISE;

        f = err_fn_of_x(num_road_test_pt, val_y, vec_logd, vec_x, vec_b);

        if( (fmin - f) > 1E-8 ) {
            fmin = f;
            xmin = val_y;
        }

        // fprintf(fppre,"%f %f\n",val_y,f);
    }
    // fclose(fppre);

    if ( (xmin < dmin*0.9 + dmax*0.1) || (xmin > dmin*0.1 + dmax*0.9) ) {
        found = 0; //can not search the inflexion
    } else {
        bx = xmin;
        fb = fmin;

        ax = xmin - (dmax-dmin)/PRECISE;
        fa = err_fn_of_x(num_road_test_pt, ax, vec_logd, vec_x, vec_b);

        cx = xmin + (dmax-dmin)/PRECISE;
        fc = err_fn_of_x(num_road_test_pt, cx, vec_logd, vec_x, vec_b);

        if( fb<fa && fb<fc ) {
            found = 1;
        } else {
            found = 0;
        }
    }

#undef PRECISE

    // If cannot find the minimum value,then use the single slope propagation model
    if (!found)
    {
        fmin = err_fn_of_s2( num_road_test_pt, vec_logd, vec_s, vec_b, min_height);

        //double s2_max = -8.0;
        // CG
        /*
        double s2_max = 0.0;
        if ( vec_s[1] >= s2_max ) {
            //printf("ERROR: profile search,Singular linear model final_slope = %7.5f >= %7.5f\n", final_slope, s2_max);
            // CG
            printf("ERROR: profile search,Singular linear model first_slope = %7.5f >= %7.5f\n", vec_s[1], s2_max);
            error = -1.0;
            return(0);
        }
        */

        num_inflexion = 2;
        x = ( double *) realloc((void *) x, num_inflexion*sizeof(double));
        y = ( double *) realloc((void *) y, num_inflexion*sizeof(double));

        x[0] = (dmin + dmax) / 2;
        y[0] = vec_s[0] + (x[0] - log(min_height)/log(10.0))*vec_s[1];
        x[1] = dmax;
        y[1] = y[0] + (x[1] - x[0])*vec_s[1];

        start_slope    = vec_s[1];
        final_slope    = cutoff_slope;

        error = fmin;
        return(1);
    }

    // search the opimized value for fine range
    #define R 0.61803399
    #define C (1.0-R)
    #define SHFT2(a,b,c) (a)=(b);(b)=(c);
    #define SHFT3(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

    x0 = ax;
    x3 = cx;
    x2 = bx;

    x1 = bx-C*(bx-ax);
    f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b);
    f2 = fb;

    tol = 1.0e-6;
    while (fabs(x3-x0) > tol*(fabs(x1)+fabs(x2))) {
        if (f2 < f1) {
            SHFT3(x0,x1,x2,R*x1+C*x3)
            f1 = f2;
            f2 = err_fn_of_x(num_road_test_pt, x2, vec_logd, vec_x, vec_b);
        } else {
            SHFT3(x3,x2,x1,R*x2+C*x0)
            f2 = f1;
            f1 = err_fn_of_x(num_road_test_pt, x1, vec_logd, vec_x, vec_b);
        }
    }

    if (f1 < f2) {
        xmin=x1;
    } else {
        xmin=x2;
    }

    fmin = err_fn_of_x(num_road_test_pt, xmin, vec_logd, vec_x, vec_b);

    #undef C
    #undef R
    #undef SHFT2
    #undef SHFT3

    /*
    ********************************************************************************************************
    * If val_s1 or val_2 of the propagation model is not proper, then use singular linear model to replace
    * the two segment line model. The new model which val_y is fixed to log(h) and val_s1 is fixed to zero.
    ********************************************************************************************************
    */
    //double s1_max = 0.0;
    // CG
    double s1_max = -1.0;
    double s2_max = -0.1;
    // use single slope model
    if ( /* final_slope>start_slope || */ vec_x[2]>s2_max || vec_x[1]>s1_max ) {
        //printf("WARNING: After fine search,start_slope=%f or final_slope=%f is not proper!\n",start_slope,final_slope);

        // CG DBG
        //std::cout << " tpm 1 " << std::endl;

        fmin = err_fn_of_s2(num_road_test_pt,vec_logd,vec_s,vec_b,min_height);

        num_inflexion = 2;
        x = ( double *) realloc((void *) x, num_inflexion*sizeof(double));
        y = ( double *) realloc((void *) y, num_inflexion*sizeof(double));

        // choose random point in RTD range
        x[0] = (dmin + dmax) / 2;
        y[0] = vec_s[0] + (x[0] - log(min_height)/log(10.0))*vec_s[1];
        // 
        x[1] = dmax;
        y[1] = y[0] + (x[1] - x[0])*vec_s[1];

        start_slope    = vec_s[1];
        final_slope    = cutoff_slope;
        error          = fmin;
    }
    // use two slope model
    else
    {
        // CG DBG
        //std::cout << " tpm 2 " << std::endl;

        num_inflexion = 2;
        x = ( double *) realloc((void *) x, num_inflexion*sizeof(double));
        y = ( double *) realloc((void *) y, num_inflexion*sizeof(double));

        x[0] = xmin;
        y[0] = vec_x[0];

        if (avg_max_logd>0.01) {
            x[1] = avg_max_logd;        
            y[1] = y[0] + (avg_max_logd - x[0])*vec_x[2];
        }
        else
        {
            x[1] = dmax;
            y[1] = y[0] + (dmax - x[0])*vec_x[2];
        }
        
        start_slope    = vec_x[1];
        final_slope    = cutoff_slope;
        error          = fmin;
    }

    // CG DBG
    /*
    for (int i=0;i<3;i++) {
        printf(" -- vec_x[%d] = %5.3f \n", i, vec_x[i]);
    }

    for (int i=0;i<2;i++) {
        printf(" -- vec_s[%d] = %5.3f \n", i, vec_s[i]);
    }
    */

    free(vec_x);
    free(vec_s);
#endif

    return(1);
}


/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::err_fn_of_x()                              ****/
/******************************************************************************************/
double SegmentAnglePropModelClass::err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x, double *vec_b, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    // construct matrix A according to the val_y
    int i, k, pt_idx, var_idx, a_seg, a_seg_p1;
    double alpha;
    double **mx_a  = (double **) NULL;
    int *freq_a    = (int     *) NULL;
    double tol = 0.05;
    int min_pt_col = 10;

    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));
    freq_a = IVECTOR(3*(n_angle+1));

    for (i=0; i<=(n_angle+1)-1; i++) {
        freq_a[i] = 0;
    }

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {

        comp_angular_seg(vec_cos[pt_idx], vec_sin[pt_idx], a_seg, a_seg_p1, alpha);

        mx_a[pt_idx] = DVECTOR(3*(n_angle+1));
        var_idx = 0;
        mx_a[pt_idx][var_idx++] = 1.0;
        for (i=0; i<=n_angle-1; i++) {
            if (i == a_seg) {
                mx_a[pt_idx][var_idx] = alpha;
            } else if (i == a_seg_p1) {
                mx_a[pt_idx][var_idx] = 1.0 - alpha;
            } else {
                mx_a[pt_idx][var_idx] = 0.0;
            }

            if (fabs(mx_a[pt_idx][var_idx]) > tol) {
                freq_a[var_idx]++;
            }
            var_idx++;
        }
        if (vec_logd[pt_idx] < val_y) {
            mx_a[pt_idx][var_idx] = vec_logd[pt_idx] - val_y;
        } else {
            mx_a[pt_idx][var_idx] = 0.0;
        }
        var_idx++;
        for (i=0; i<=n_angle-1; i++) {
            if (vec_logd[pt_idx] < val_y) {
                if (i == a_seg) {
                    mx_a[pt_idx][var_idx] = (vec_logd[pt_idx] - val_y)*alpha;
                } else if (i == a_seg_p1) {
                    mx_a[pt_idx][var_idx] = (vec_logd[pt_idx] - val_y)*(1.0-alpha);
                } else {
                    mx_a[pt_idx][var_idx] = 0.0;
                }
            } else {
                mx_a[pt_idx][var_idx] = 0.0;
            }
            var_idx++;
        }
        if (vec_logd[pt_idx] > val_y) {
            mx_a[pt_idx][var_idx] = vec_logd[pt_idx] - val_y;
        } else {
            mx_a[pt_idx][var_idx] = 0.0;
        }
        var_idx++;
        for (i=0; i<=n_angle-1; i++) {
            if (vec_logd[pt_idx] > val_y) {
                if (i == a_seg) {
                    mx_a[pt_idx][var_idx] = (vec_logd[pt_idx] - val_y)*alpha;
                } else if (i == a_seg_p1) {
                    mx_a[pt_idx][var_idx] = (vec_logd[pt_idx] - val_y)*(1.0-alpha);
                } else {
                    mx_a[pt_idx][var_idx] = 0.0;
                }
            } else {
                mx_a[pt_idx][var_idx] = 0.0;
            }
            var_idx++;
        }
    }

    for (i=1; i<=n_angle; i++) {
        if (freq_a[i] < min_pt_col) {
            for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
                for (k=0; k<=2; k++) {
                    mx_a[pt_idx][i + k*(1 + n_angle)] = 0.0;
                }
            }
        }
    }

#if 1
printf("MATRIX A\n");
printf("%18s ", "Y_AVG");
for (i=0; i<=n_angle-1; i++) {
    printf("             Y[%2d] ", i);
}
printf("%18s ", "S_AVG ");
for (i=0; i<=n_angle-1; i++) {
    printf("             S[%2d] ", i);
}
printf("%18s ", "F_AVG ");
for (i=0; i<=n_angle-1; i++) {
    printf("             F[%2d] ", i);
}
printf("\n");
for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
    for (i=0; i<=3*(n_angle + 1)-1; i++) {
        printf("%18.10f ", mx_a[pt_idx][i]);
    }
    printf("\n");
}
printf("\n");
#endif

    double error = svd_solve(mx_a, vec_b, num_road_test_pt, 3*(n_angle + 1), vec_x);

#if 1
printf("VECTOR X\n");
printf("%18s ", "Y_AVG");
for (i=0; i<=n_angle-1; i++) {
    printf("             Y[%2d] ", i);
}
printf("%18s ", "S_AVG ");
for (i=0; i<=n_angle-1; i++) {
    printf("             S[%2d] ", i);
}
printf("%18s ", "F_AVG ");
for (i=0; i<=n_angle-1; i++) {
    printf("             F[%2d] ", i);
}
printf("\n");
for (i=0; i<=3*(n_angle + 1)-1; i++) {
    printf("%18.10f ", vec_x[i]);
}
printf("\n");
#endif

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        free(mx_a[pt_idx]);
    }
    free(mx_a);

    return(error);
#else
    return(0.0);
#endif
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::err_fn_of_x()                          ****/
/******************************************************************************************/
double SegmentWithThetaPropModelClass::err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x, double *vec_b, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    // construct matrix A according to the val_y
    int i, pt_idx, var_idx;
    double **mx_a = NULL;
    double *cc, *ss;
    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));

    int n_max_series = n_series_y[0];
    if (n_series_start_slope > n_max_series) {
        n_max_series = n_series_start_slope;
    }
    if (n_series_final_slope > n_max_series) {
        n_max_series = n_series_final_slope;
    }
    cc = DVECTOR(n_max_series);
    ss = DVECTOR(n_max_series);

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        for (i=0; i<=n_max_series-1; i++) {
            if (i==0) {
                cc[i] = vec_cos[pt_idx];
                ss[i] = vec_sin[pt_idx];
            } else {
                cc[i] = cc[i-1]*cc[0] - ss[i-1]*ss[0];
                ss[i] = cc[i-1]*ss[0] + ss[i-1]*cc[0];
            }
        }

        mx_a[pt_idx] = DVECTOR(3 + 2*(n_series_y[0] + n_series_start_slope + n_series_final_slope));
        var_idx = 0;
        mx_a[pt_idx][var_idx++] = 1.0;
        for (i=0; i<=n_series_y[0]-1; i++) {
            mx_a[pt_idx][var_idx++] = cc[i];
            mx_a[pt_idx][var_idx++] = ss[i];
        }
        if (vec_logd[pt_idx] < val_y) {
            mx_a[pt_idx][var_idx++] = vec_logd[pt_idx] - val_y;
        } else {
            mx_a[pt_idx][var_idx++] = 0.0;
        }
        for (i=0; i<=n_series_start_slope-1; i++) {
            if (vec_logd[pt_idx] < val_y) {
                mx_a[pt_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*cc[i];
                mx_a[pt_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*ss[i];
            } else {
                mx_a[pt_idx][var_idx++] = 0.0;
                mx_a[pt_idx][var_idx++] = 0.0;
            }
        }
        if (vec_logd[pt_idx] > val_y) {
            mx_a[pt_idx][var_idx++] = vec_logd[pt_idx] - val_y;
        } else {
            mx_a[pt_idx][var_idx++] = 0.0;
        }
        for (i=0; i<=n_series_final_slope-1; i++) {
            if (vec_logd[pt_idx] > val_y) {
                mx_a[pt_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*cc[i];
                mx_a[pt_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*ss[i];
            } else {
                mx_a[pt_idx][var_idx++] = 0.0;
                mx_a[pt_idx][var_idx++] = 0.0;
            }
        }
    }

    double error = svd_solve(mx_a, vec_b, num_road_test_pt, 3 + 2*(n_series_y[0] + n_series_start_slope + n_series_final_slope), vec_x);

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        free(mx_a[pt_idx]);
    }
    free(mx_a);
    free(cc);
    free(ss);

    return(error);
#else
    return(0.0);
#endif
}

/*
**************************************************************************************
* FUNCTION: SegmentPropModelClass::err_fn_of_x
*           Search the optimal distance for the inflexion of the propagation model
**************************************************************************************
*/
double SegmentPropModelClass::err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x, double *vec_b)
{
#if (DEMO == 0)
    // construct matrix A according to the val_y
    int pt_idx;
    double **mx_a = NULL;
    mx_a = (double **) malloc(num_road_test_pt*sizeof(double *));

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        mx_a[pt_idx] = DVECTOR(3);
        mx_a[pt_idx][0] = 1.0;
        if (vec_logd[pt_idx] < val_y) {
            mx_a[pt_idx][1] = vec_logd[pt_idx] - val_y;
            mx_a[pt_idx][2] = 0.0;
        } else {
            mx_a[pt_idx][1] = 0.0;
            mx_a[pt_idx][2] = vec_logd[pt_idx] - val_y;
        }
    }

    double error = svd_solve(mx_a, vec_b, num_road_test_pt, 3, vec_x);

    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        free(mx_a[pt_idx]);
    }
    free(mx_a);

    return(error);
#else
    return(0.0);
#endif
}

/******************************************************************************************/
/**** FUNCTION: SegmentAnglePropModelClass::err_fn_of_s2()                         ****/
/******************************************************************************************/
double SegmentAnglePropModelClass::err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b,double min_height, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    int    i, pt_idx, var_idx, a_seg, a_seg_p1;
    int    valid_number;
    int    valid_idx;
    double alpha;
    double val_y;
    double error;      //the error of the model
    double *vec_sb = NULL;
    double **mx_sa = NULL;
    int    *freq_a = (int *) NULL;
    double tol = 0.05;
    int min_pt_col = 10;

    val_y = log(min_height)/log(10.0);

    valid_number = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if ( vec_logd[pt_idx] >= val_y ) {
            valid_number = valid_number + 1;
        }
    }

    vec_sb = DVECTOR(valid_number);
    mx_sa = (double **) malloc(valid_number*sizeof(double *));
    freq_a = IVECTOR(2*(n_angle+1));

    for (i=0; i<=2*(n_angle+1)-1; i++) {
        freq_a[i] = 0;
    }

    valid_idx = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] >= val_y ) {
            comp_angular_seg(vec_cos[pt_idx], vec_sin[pt_idx], a_seg, a_seg_p1, alpha);

            mx_sa[valid_idx] = DVECTOR(2*(n_angle+1));
            var_idx = 0;
            mx_sa[valid_idx][var_idx++] = 1.0;
            for (i=0; i<=n_angle-1; i++) {
                if (i == a_seg) {
                    mx_sa[valid_idx][var_idx] = alpha;
                } else if (i == a_seg_p1) {
                    mx_sa[valid_idx][var_idx] = 1.0 - alpha;
                } else {
                    mx_sa[valid_idx][var_idx] = 0.0;
                }

                if (fabs(mx_sa[valid_idx][var_idx]) > tol) {
                    freq_a[var_idx]++;
                }
                var_idx++;
            }

            mx_sa[valid_idx][var_idx] = vec_logd[pt_idx] - val_y;
            if (fabs(mx_sa[valid_idx][var_idx]) > tol) {
                freq_a[var_idx]++;
            }
            var_idx++;
            for (i=0; i<=n_angle-1; i++) {
                if (i == a_seg) {
                    mx_sa[valid_idx][var_idx] = (vec_logd[pt_idx] - val_y)*alpha;
                } else if (i == a_seg_p1) {
                    mx_sa[valid_idx][var_idx] = (vec_logd[pt_idx] - val_y)*(1.0-alpha);
                } else {
                    mx_sa[valid_idx][var_idx] = 0.0;
                }

                if (fabs(mx_sa[valid_idx][var_idx]) > tol) {
                    freq_a[var_idx]++;
                }
                var_idx++;
            }
            vec_sb[valid_idx] = vec_b[pt_idx];
            valid_idx = valid_idx + 1;
        }
    }

    for (i=1; i<=2*(n_angle+1)-1; i++) {
        if (freq_a[i] < min_pt_col) {
            for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
                mx_sa[pt_idx][i] = 0.0;
            }
        }
    }

    error = svd_solve(mx_sa, vec_sb, valid_number, 2*(n_angle + 1), vec_x);

    for (pt_idx=0; pt_idx<=valid_number-1; pt_idx++) {
        free(mx_sa[pt_idx]);
    }
    free(mx_sa);
    free(vec_sb);
    free(freq_a);

    return(error);
#else
    return(0.0);
#endif
}
/******************************************************************************************/
/**** FUNCTION: SegmentWithThetaPropModelClass::err_fn_of_s2()                         ****/
/******************************************************************************************/
double SegmentWithThetaPropModelClass::err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b,double min_height, double *vec_cos, double *vec_sin)
{
#if (DEMO == 0)
    int    i, pt_idx, var_idx;
    int    valid_number;
    int    valid_idx;
    double val_y;
    double error;      //the error of the model
    double *vec_sb = NULL;
    double **mx_sa = NULL;
    double *cc, *ss;

    val_y = log(min_height)/log(10.0);

    valid_number = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if ( vec_logd[pt_idx] >= val_y ) {
            valid_number = valid_number + 1;
        }
    }

    vec_sb = DVECTOR(valid_number);
    mx_sa = (double **) malloc(valid_number*sizeof(double *));

    int n_max_series = n_series_y[0];
    if (n_series_start_slope > n_max_series) {
        n_max_series = n_series_start_slope;
    }
    cc = DVECTOR(n_max_series);
    ss = DVECTOR(n_max_series);

    valid_idx = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] >= val_y ) {
            for (i=0; i<=n_max_series-1; i++) {
                if (i==0) {
                    cc[i] = vec_cos[pt_idx];
                    ss[i] = vec_sin[pt_idx];
                } else {
                    cc[i] = cc[i-1]*cc[0] - ss[i-1]*ss[0];
                    ss[i] = cc[i-1]*ss[0] + ss[i-1]*cc[0];
                }
            }
            mx_sa[valid_idx] = DVECTOR(2 + 2*(n_series_y[0] + n_series_start_slope));
            var_idx = 0;
            mx_sa[valid_idx][var_idx++] = 1.0;
            for (i=0; i<=n_series_y[0]-1; i++) {
                mx_sa[valid_idx][var_idx++] = cc[i];
                mx_sa[valid_idx][var_idx++] = ss[i];
            }

            mx_sa[valid_idx][var_idx++] = vec_logd[pt_idx] - val_y;
            for (i=0; i<=n_series_start_slope-1; i++) {
                mx_sa[valid_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*cc[i];
                mx_sa[valid_idx][var_idx++] = (vec_logd[pt_idx] - val_y)*ss[i];
            }
            vec_sb[valid_idx] = vec_b[pt_idx];
            valid_idx = valid_idx + 1;
        }
    }

    error = svd_solve(mx_sa, vec_sb, valid_number, 2 + 2*(n_series_y[0] + n_series_start_slope), vec_x);

    for (pt_idx=0; pt_idx<=valid_number-1; pt_idx++) {
        free(mx_sa[pt_idx]);
    }
    free(mx_sa);
    free(vec_sb);
    free(cc);
    free(ss);

    return(error);
#else
    return(0.0);
#endif
}

/**************************************************************************************
* FUNCTION: SegmentPropModelClass::err_fn_of_s2
**************************************************************************************/
double SegmentPropModelClass::err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b,double min_height)
{
#if (DEMO == 0)
    int    pt_idx;     //test point loop count
    int    valid_number;
    int    valid_idx;
    double val_y;
    double error;      //the error of the model
    double *vec_sb = NULL;
    double **mx_sa = NULL;

    val_y = log(min_height)/log(10.0);

    valid_number = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if ( vec_logd[pt_idx] >= val_y ) {
            valid_number = valid_number + 1;
        }
    }

    vec_sb = DVECTOR(valid_number);
    mx_sa = (double **) malloc(valid_number*sizeof(double *));

    valid_idx = 0;
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if (vec_logd[pt_idx] >= val_y ) {
            mx_sa[valid_idx] = DVECTOR(2);
            mx_sa[valid_idx][0] = 1.0;
            mx_sa[valid_idx][1] = vec_logd[pt_idx] - val_y;
            vec_sb[valid_idx] = vec_b[pt_idx];
            valid_idx = valid_idx + 1;
        }
    }

    error = svd_solve(mx_sa, vec_sb, valid_number, 2, vec_x);

    for (pt_idx=0; pt_idx<=valid_number-1; pt_idx++) {
        free(mx_sa[pt_idx]);
    }
    free(mx_sa);
    free(vec_sb);

    return(error);
#else
    return(0.0);
#endif
}

/*
*********************************************************************
* FUNCTION: SegmentPropModelClass::optimize_clutter_coeffs
*  Optimize the clutter coefficient and the base station antenna
*  height if there are more than two base station.
*********************************************************************
*/
double SegmentPropModelClass::optimize_clutter_coeffs(int num_road_test_pt, double **mx_a,double *vec_b,double *vec_logd)
{
#if (DEMO == 0)
    int      pt_idx;           // test pointer loop variable
    int      done;
    double   error;            // error value
    double   *power;           // power of the test point.
    double   prop_db;          // prop model power

    power = DVECTOR(num_road_test_pt);
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        power[pt_idx] = 0.0;
    }

    //through the first step optimization, get the vec_b of second step optimization
    for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
        if ( vec_logd[pt_idx]<=x[0] ) {
            prop_db = y[0] + start_slope*(vec_logd[pt_idx]-x[0]);
        } else if ( vec_logd[pt_idx]>x[num_inflexion-1] ) {
            prop_db = y[num_inflexion-1] + final_slope*(vec_logd[pt_idx]-x[num_inflexion-1]);
        } else {
            done = 0;
            for ( int idx=0; (idx<num_inflexion-1)&&(!done); idx++ ) {
                if ( (vec_logd[pt_idx]>x[idx]) && (vec_logd[pt_idx]<=x[idx+1]) ) {
                    prop_db = y[idx] + ((y[idx]-y[idx+1])/(x[idx]-x[idx+1]))*(vec_logd[pt_idx]-x[idx]);
                    done = 1;
                }
            }
        }

        power[pt_idx] = vec_b[pt_idx] - prop_db;
    }

    error = svd_solve(mx_a, power, num_road_test_pt, num_clutter_type+2*useheight, vec_k);

#if DEBUG_COMP_PROP_MODEL
    printf("###################################################\n");
    printf("#  Step 2: Optimization of Clutter Coefficients   #\n");
    printf("###################################################\n");
    printf("MIN METRIC = %15.10f\n", error);
    printf("RMS ERROR  = %15.10f\n", sqrt(error/num_road_test_pt));
    printf("\n\n");
#endif

    free(power);

    return(error);
#else
    return(0.0);
#endif
}

/*
**************************************************************************************
* FUNCTION: SegmentPropModelClass::compute_error
*  According to the parameter to calculate the error of the propagation model.
**************************************************************************************
*/
double SegmentPropModelClass::compute_error(int num_road_test_pt, double **clutter_dis,double *power, double *vec_logd, int plot, char *flname)
{
#if (DEMO == 0)
    int       pt_idx;             //road test point loop count
    int       clutter_idx;        //clutter loop count
    double    diff;               //the difference
    double    dist;               //the distance
    double    slope;              //the slope of the line
    FILE      *fp;                //file pointer
    fp = NULL;                    //initial the file pointer

    if (plot) {
        if ( !(fp = fopen(flname, "w")) ) {
            fprintf(stderr, "\nERROR: Unable to write to file %s\n", flname);
            exit(1);
        }
    }

    // CG DBG
    /*
    printf("pm->num_clutter_type+2*pm->useheight = %d \n", num_clutter_type+2*useheight);
    for (int i=0; i<num_clutter_type+2*useheight;i++)
        printf("vec_k[%d] = %5.3f \n", i, vec_k[i]);
     */


    double error = 0.0;
    if ( start_slope <= 0.00001 && start_slope>=-0.00001 ) {
        for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
            if( vec_logd[pt_idx] > x[0] ) {
                diff = y[0] + (vec_logd[pt_idx] - x[0])*final_slope;
                for (clutter_idx=0; clutter_idx<=num_clutter_type+2*useheight-1; clutter_idx++) {
                    diff += vec_k[clutter_idx]*clutter_dis[pt_idx][clutter_idx];
                }

                diff -= power[pt_idx];
                error += diff*diff;

                if (plot) {
                    dist = exp(vec_logd[pt_idx]*log(10.0));
                    fprintf(fp, "%15.10f %15.10f\n", dist, diff);
                }
            }
        }
    } else {
        for (pt_idx=0; pt_idx<=num_road_test_pt-1; pt_idx++) {
            if( vec_logd[pt_idx] < x[0]) {
                slope = start_slope;
            } else {
                slope = final_slope;
            }

            diff = y[0] + (vec_logd[pt_idx] - x[0])*slope;

            for (clutter_idx=0; clutter_idx<=num_clutter_type+2*useheight-1; clutter_idx++) {
                diff += vec_k[clutter_idx]*clutter_dis[pt_idx][clutter_idx];
            }

            diff -= power[pt_idx];
            error += diff*diff;

            if (plot) {
                dist = exp(vec_logd[pt_idx]*log(10.0));
                fprintf(fp, "%15.10f %15.10f\n", dist, diff);
            }
        }
    }

    if (plot) {
        fprintf(fp, "\n");
        fclose(fp);
    }

    return(error);
#else
    return(0.0);
#endif
}
