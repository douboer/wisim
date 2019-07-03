/******************************************************************************************/
/**** PROGRAM: geometry.cpp                                                            ****/
/**** Michael Mandell 1/14/02                                                          ****/
/******************************************************************************************/
/**** Functions for reading/writing geometry files.                                    ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#include "cdma2000.h"
#include "cconst.h"
#include "phs.h"
#include "wcdma.h"
#include "wlan.h"
#include "WiSim.h"
#include "antenna.h"
#include "prop_model.h"
#include "clutter_data_analysis.h"
#include "polygon.h"
#include "st_param.h"
#include "list.h"

#include <QDebug>

#if HAS_GUI
#include "WiSim_gui.h"
#endif

#define PHS_GEOMETRY_FORMAT "1.5"
#define WCDMA_GEOMETRY_FORMAT "1.0"
#define CDMA2000_GEOMETRY_FORMAT "1.0"
#define WLAN_GEOMETRY_FORMAT "1.0"

/******************************************************************************************/
/**** FUNCTION: NetworkClass::read_geometry                                            ****/
/**** Open specified file and read cellular geometry.                                  ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/**** Version/technology support added.                                                ****/
/******************************************************************************************/
void NetworkClass::read_geometry(char *filename, char *WiSim_home, char *force_fmt)
{
    int i;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int in_bdy, edge;
    int linenum;
    CellClass *cell                = (CellClass   *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    char *str1, *line;
    char *antenna_filepath;
    char *format_str = (char *) NULL;
    FILE *fp;

    line = CVECTOR(MAX_LINE_SIZE);

    if (system_bdy) {
        sprintf(msg, "ERROR: geometry already defined\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: cannot open geometry file %s\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    strcpy(msg, "Reading geometry file: ");
    strcat(msg, filename);
    PRMSG(stdout, msg);

    antenna_filepath = CVECTOR( strlen(WiSim_home) + strlen("/antenna/") );
    sprintf(antenna_filepath, "%s%cantenna%c", WiSim_home, FPATH_SEPARATOR, FPATH_SEPARATOR);

    enum state_enum {
        STATE_TECHNOLOGY,
        STATE_FORMAT,
        STATE_READ_VERSION
    };

    state_enum state;

    state = STATE_TECHNOLOGY;
    linenum = 0;

    if (!force_fmt) {
    while ( (state != STATE_READ_VERSION) && fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_TECHNOLOGY:
                    if (strcmp(str1, "TECHNOLOGY:") != 0) {
#if 1
                        sprintf(msg, "WARNING: geometry file \"%s\"\n"
                                         "Assuming format 1.1\n", filename);
                        PRMSG(stdout, msg); warning_state = 1;

                        fclose(fp);
                        if ( !(fp = fopen(filename, "rb")) ) {
                            sprintf(msg, "ERROR: cannot open geometry file %s\n", filename);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        format_str = strdup("1.1");
                        state = STATE_READ_VERSION;
#else
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TECHNOLOGY:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
#endif
                    } else {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (strcmp(str1, technology_str()) != 0) {
                                sprintf(msg, "ERROR: geometry file \"%s(%d)\"\n"
                                                 "\"TECHNOLOGY:\" specification \"%s\" not \"%s\"\n",
                                                 filename, linenum, str1, technology_str());
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No \"TECHNOLOGY:\" specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        state = STATE_FORMAT;
                    }
                    break;
                case STATE_FORMAT:
                    if (strcmp(str1, "FORMAT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"FORMAT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    format_str = strdup(str1);
                    state = STATE_READ_VERSION;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_READ_VERSION) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }
    } else {
        format_str = strdup(force_fmt);
    }

    if (technology() == CConst::PHS) {
        if (strcmp(format_str,"1.0")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_0(fp, antenna_filepath, line, filename, linenum);
        } else if (strcmp(format_str,"1.1")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_1(fp, antenna_filepath, line, filename, linenum);
        } else if (strcmp(format_str,"1.2")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_2(fp, antenna_filepath, line, filename, linenum);
        } else if (strcmp(format_str,"1.3")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_3(fp, antenna_filepath, line, filename, linenum);
        } else if (strcmp(format_str,"1.4")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_4(fp, antenna_filepath, line, filename, linenum);
        } else if (strcmp(format_str,"1.5")==0) {
            ((PHSNetworkClass *) this)->read_geometry_1_5(fp, antenna_filepath, line, filename, linenum);
        } else {

            sprintf(msg, "ERROR: geometry file \"%s\" has invalid PHS format \"%s\"\n", filename, format_str);
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
    } else if (technology() == CConst::WCDMA) {
        if (strcmp(format_str,"1.0")==0) {
            ((WCDMANetworkClass *) this)->read_geometry_1_0(fp, antenna_filepath, line, filename, linenum);
        } else {
            sprintf(msg, "ERROR: geometry file \"%s\" has invalid WCDMA format \"%s\"\n", filename, format_str);
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
    } else if (technology() == CConst::CDMA2000) {
        if (strcmp(format_str,"1.0")==0) {
            ((CDMA2000NetworkClass *) this)->read_geometry_1_0(fp, antenna_filepath, line, filename, linenum);
        } else {
            sprintf(msg, "ERROR: geometry file \"%s\" has invalid CDMA2000 format \"%s\"\n", filename, format_str);
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
    } else if (technology() == CConst::WLAN) {
        if (strcmp(format_str,"1.0")==0) {
            ((WLANNetworkClass *) this)->read_geometry_1_0(fp, antenna_filepath, line, filename, linenum);
        } else {
            sprintf(msg, "ERROR: geometry file \"%s\" has invalid WLAN format \"%s\"\n", filename, format_str);
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
    } else {
        CORE_DUMP;
    }

    if (error_state == 1) { return; }

    if (coordinate_system == CConst::CoordGeneric) {
        sprintf(msg, "COORDINATE SYSTEM: GENERIC\n");
    } else if (coordinate_system == CConst::CoordUTM) {
        sprintf(msg, "COORDINATE SYSTEM: UTM\n"
                         "EQUATORIAL RADIUS: %.0f\n"
                         "ECCENTRICITY SQ: %.9f\n"
                         "ZONE = %d%c\n",
        utm_equatorial_radius, utm_eccentricity_sq, utm_zone, (utm_north ? 'N' : 'S'));
    }
    PRMSG(stdout, msg);

    double system_area = system_bdy->comp_bdy_area();
    sprintf(msg, "SYSTEM AREA = %15.10e (grid pts^2) = %15.10e (m^2)\n",
            system_area, system_area*resolution*resolution);
    PRMSG(stdout, msg);

    if (num_cell) {
        avg_cell_radius = sqrt( system_area/( PI * (num_cell)) );
        rec_avg_cell_radius = 1.0 / avg_cell_radius;
        sprintf(msg, "AVG CELL RADIUS = %15.10e (grid pts) = %15.10e (m)\n", avg_cell_radius, avg_cell_radius*resolution);
        PRMSG(stdout, msg);
    }

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        if (!system_bdy->in_bdy_area(cell->posn_x, cell->posn_y, &edge)) {
#if 1
            sprintf(msg, "WARNING: Cell %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                    cell_idx, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
            PRMSG(stdout, msg); warning_state = 1;
#else
            sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
                             "Cell %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                    filename, cell_idx, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
            PRMSG(stdout, msg);
            error_state = 1;
            return;
#endif
        }
        for (i=0; i<=cell_idx-1; i++) {
            if ( (cell_list[i]->posn_x == cell->posn_x) && (cell_list[i]->posn_y == cell->posn_y) ) {
#if 1
                sprintf(msg, "WARNING: Cells %d and %d are both located at the same position (%12.7f, %12.7f)\n",
                        cell_idx, i, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
                PRMSG(stdout, msg); warning_state = 1;
#else
                sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
                                 "Cells %d and %d are both located at the same position (%12.7f, %12.7f)\n",
                        filename, cell_idx, i, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
                PRMSG(stdout, msg);
                error_state = 1;
                return;
#endif
            }
        }
    }

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
                for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                    in_bdy = system_bdy->in_bdy_area(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx], subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx], &edge);
                    if (!(in_bdy || edge)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
                                         "Traffic Type %s Subnet %d segment %d bdy_pt %d is not contained in the system boundary area\n",
                                filename, traffic_type_list[tt_idx]->name(), subnet_idx, segment_idx, bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                }
            }
            subnet->p->comp_bdy_min_max(subnet->minx, subnet->maxx, subnet->miny, subnet->maxy);
        }
    }

    total_num_sectors = 0;

    free(antenna_filepath);
    free(line);
    free(format_str);

    fclose(fp);

    return;
}
/******************************************************************************************/
/**** PHS                                                                              ****/
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_5                                     ****/
/**** Read PHS format 1.5 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_5(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, n, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int st_param_idx             = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    PHSSectorClass *sector         = (PHSSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_CSID_FORMAT,
        STATE_ST_DATA_PERIOD,

        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_BM_IDX,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_NUM_ST_PARAM,
        STATE_ST_PARAM,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            if ((utm_equatorial_radius < 6.0e6) || (utm_equatorial_radius > 7.0e6)) {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n"
                                                 "Equatorial radius = %f must be between %f and %f\n", filename, linenum, utm_equatorial_radius, 6.0e6, 7.0e6);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CSID_FORMAT;
                    break;
                case STATE_CSID_FORMAT:
                    if (strcmp(str1, "CSID_FORMAT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID_FORMAT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        csid_format = atoi(str1);
                    } else {
                        csid_format = -1;
                    }
                    if ((csid_format < 0)||(csid_format > num_csid_format-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "csid_format = %d must be between 0 and %d\n",
                                filename, linenum, csid_format, num_csid_format-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ST_DATA_PERIOD;
                    break;
                case STATE_ST_DATA_PERIOD:
                    if (strcmp(str1, "ST_DATA_PERIOD:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ST_DATA_PERIOD:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        PHSSectorClass::st_data_period = atoi(str1);
                    } else {
                        PHSSectorClass::st_data_period = -2;
                    }
                    if ((PHSSectorClass::st_data_period < -1)||(PHSSectorClass::st_data_period > 23)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "st_data_period = %d must be between 0 and 23\n",
                                filename, linenum, PHSSectorClass::st_data_period);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new PHSTrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);
                    if (!subnet->strid) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "SUBNET_%d has no name\n", filename, linenum, subnet_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        num_prop_model = 0;
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
                        prop_model_list[pm_idx] = (PropModelClass *) NULL;
                    }
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "RSQUARED")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new RSquaredPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelFullClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERSYMFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterSymFullPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPO")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPOSLOPE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTEREXPOLINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterExpoLinearPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERGLOBAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterGlobalPropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_BM_IDX;
                    break;
                case STATE_CELL_BM_IDX:
                    if (strcmp(str1, "BM_IDX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BM_IDX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->bm_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (cell->bm_idx < 0) || (cell->bm_idx > cell->num_bm-1) ) {
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" has invalid BM_IDX = %d, using value 0\n",
                                         filename, linenum, cell->bm_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                        cell->bm_idx = 0;
                    }

                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        state = STATE_NUM_ST_PARAM;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        state = STATE_NUM_ST_PARAM;
                    }
                    break;
                case STATE_NUM_ST_PARAM:
                    if (strcmp(str1, "NUM_ST_PARAM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ST_PARAM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        n = atoi(str1);
                        if ( (n != 0) && (n != sector->st_param_list->getSize()) ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Value \"NUM_ST_PARAM:\" = %d specified, value must be 0 or %d\n",
                                         filename, linenum, n, sector->st_param_list->getSize());
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ST_PARAM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    st_param_idx = 0;
                    if (n > 0) {
                        sector->st_data = IVECTOR(n);
                        state = STATE_ST_PARAM;
                    } else if (n == 0) {
                        sector->st_data = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_ST_PARAM=%d must be >= 0 \n", filename, linenum, n);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_ST_PARAM:
                    sprintf(str, "ST_PARAM_%d:", st_param_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        sector->st_data[st_param_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ST_PARAM_%d\" specified\n", filename, linenum, st_param_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    st_param_idx++;
                    if (st_param_idx <= sector->st_param_list->getSize()-1) {
                        state = STATE_ST_PARAM;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_4                                     ****/
/**** Read PHS format 1.4 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_4(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    PHSSectorClass *sector         = (PHSSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_CSID_FORMAT,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_BM_IDX,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            if ((utm_equatorial_radius < 6.0e6) || (utm_equatorial_radius > 7.0e6)) {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n"
                                                 "Equatorial radius = %f must be between %f and %f\n", filename, linenum, utm_equatorial_radius, 6.0e6, 7.0e6);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CSID_FORMAT;
                    break;
                case STATE_CSID_FORMAT:
                    if (strcmp(str1, "CSID_FORMAT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID_FORMAT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        csid_format = atoi(str1);
                    } else {
                        csid_format = -1;
                    }
                    if ((csid_format < 0)||(csid_format > num_csid_format-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "csid_format = %d must be between 0 and %d\n",
                                filename, linenum, csid_format, num_csid_format-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new PHSTrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);
                    if (!subnet->strid) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "SUBNET_%d has no name\n", filename, linenum, subnet_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        num_prop_model = 0;
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
                        prop_model_list[pm_idx] = (PropModelClass *) NULL;
                    }
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "RSQUARED")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new RSquaredPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelFullClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERSYMFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterSymFullPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPO")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPOSLOPE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTEREXPOLINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterExpoLinearPropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_BM_IDX;
                    break;
                case STATE_CELL_BM_IDX:
                    if (strcmp(str1, "BM_IDX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BM_IDX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->bm_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (cell->bm_idx < 0) || (cell->bm_idx > cell->num_bm-1) ) {
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" has invalid BM_IDX = %d, using value 0\n",
                                         filename, linenum, cell->bm_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                        cell->bm_idx = 0;
                    }

                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_3                                     ****/
/**** Read PHS format 1.3 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_3(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    PHSSectorClass *sector         = (PHSSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_BM_IDX,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            if ((utm_equatorial_radius < 6.0e6) || (utm_equatorial_radius > 7.0e6)) {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n"
                                                 "Equatorial radius = %f must be between %f and %f\n", filename, linenum, utm_equatorial_radius, 6.0e6, 7.0e6);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new PHSTrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);
                    if (!subnet->strid) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "SUBNET_%d has no name\n", filename, linenum, subnet_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_BM_IDX;
                    break;
                case STATE_CELL_BM_IDX:
                    if (strcmp(str1, "BM_IDX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BM_IDX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->bm_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (cell->bm_idx < 0) || (cell->bm_idx > cell->num_bm-1) ) {
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" has invalid BM_IDX = %d, using value 0\n",
                                         filename, linenum, cell->bm_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                        cell->bm_idx = 0;
                    }

                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_2                                     ****/
/**** Read PHS format 1.2 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_2(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    PHSSectorClass *sector         = (PHSSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new PHSTrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if ( (p_prop_model_strid) && (p_prop_model_strid[0] != (char) NULL) ) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);

#if 0
                        cell->sector_list[sector_idx]->stat_count = (StatCountClass *) NULL;
                        // cell->sector_list[sector_idx]->stat_count = (StatCountClass *) malloc(sizeof(StatCountClass));
                        cell->sector_list[sector_idx]->num_road_test_pt = 0;
                        cell->sector_list[sector_idx]->sync_level       = -1;
                        cell->sector_list[sector_idx]->num_call = 0;
                        cell->sector_list[sector_idx]->call_list = (CallClass **) NULL;
                        cell->sector_list[sector_idx]->active = 1;
                        // stat_count = cell->sector_list[sector_idx]->stat_count;
                        // reset_stat_count(stat_count, num_attempt_assign_channel, 1);
#endif
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_1                                     ****/
/**** Read PHS format 1.1 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_1(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    double *arrival_rate_list      = (double *) NULL;
    CellClass *cell                = (CellClass   *) NULL;
    PHSSectorClass *sector         = (PHSSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_ARRIVAL_RATE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_TRAFFIC,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_TRAFFIC,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
        linenum++;
        str1 = strtok(line, CHDELIM);

        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        tt_idx = 0;
                        arrival_rate_list = DVECTOR(num_traffic_type);
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new PHSTrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_ARRIVAL_RATE;
                    break;
                case STATE_TRAFFIC_TYPE_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ARRIVAL_RATE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        arrival_rate_list[tt_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ARRIVAL_RATE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);

                    state = STATE_SUBNET_TRAFFIC;
                    break;
                case STATE_SUBNET_TRAFFIC:
                    if (strcmp(str1, "TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        subnet->arrival_rate = arrival_rate_list[tt_idx]*tmpd;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:

                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:

                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);

                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {

                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }

                        } else {

                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);

#if 0
                        cell->sector_list[sector_idx]->stat_count = (StatCountClass *) NULL;
                        // cell->sector_list[sector_idx]->stat_count = (StatCountClass *) malloc(sizeof(StatCountClass));
                        cell->sector_list[sector_idx]->num_road_test_pt = 0;
                        cell->sector_list[sector_idx]->sync_level       = -1;
                        cell->sector_list[sector_idx]->num_call = 0;
                        cell->sector_list[sector_idx]->call_list = (CallClass **) NULL;
                        cell->sector_list[sector_idx]->active = 1;
                        // stat_count = cell->sector_list[sector_idx]->stat_count;
                        // reset_stat_count(stat_count, num_attempt_assign_channel, 1);
#endif
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_TRAFFIC;
                    }
                    break;
                case STATE_SECTOR_TRAFFIC:
                    sprintf(str, "TRAFFIC_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        sector->meas_ctr_list[tti_idx] = tmpd*arrival_rate_list[tt_idx];
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_TRAFFIC;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    free(arrival_rate_list);

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::read_geometry_1_0                                     ****/
/**** Read PHS format 1.0 geometry file.                                               ****/
/******************************************************************************************/
void PHSNetworkClass::read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int maxx, maxy;
    CellClass *cell            = (CellClass   *) NULL;
    PHSSectorClass *sector     = (PHSSectorClass *) NULL;
    SubnetClass *subnet        = (SubnetClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_FREQ,
        STATE_NUM_SLOT,
        STATE_NUM_CNTL_CHAN_SLOT,
        STATE_CNTL_CHAN_FREQ,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_COMM_SUBNET,
        STATE_COMM_SUBNET,
        STATE_COMM_SUBNET_TRAFFIC,
        STATE_COMM_SUBNET_COLOR,
        STATE_COMM_SUBNET_NUM_SEGMENT,
        STATE_COMM_SUBNET_SEGMENT,
        STATE_COMM_SUBNET_NUM_BDY_PT,
        STATE_COMM_SUBNET_BDY_PT,
        STATE_NUM_LREG_SUBNET,
        STATE_LREG_SUBNET,
        STATE_LREG_SUBNET_TRAFFIC,
        STATE_LREG_SUBNET_COLOR,
        STATE_LREG_SUBNET_NUM_SEGMENT,
        STATE_LREG_SUBNET_SEGMENT,
        STATE_LREG_SUBNET_NUM_BDY_PT,
        STATE_LREG_SUBNET_BDY_PT,
        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_COMM_TRAFFIC,
        STATE_LREG_TRAFFIC,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    /**************************************************************************************/
    /**** state:                                                                       ****/
    /****     STATE_COORDINATE_SYSTEM:       Looking for COORDINATE_SYSTEM:            ****/
    /****     STATE_RESOLUTION:              Looking for NUM_RESOLUTION:               ****/
    /****     STATE_NUM_FREQ:                Looking for NUM_FREQ:                     ****/
    /****     STATE_NUM_SLOT:                Looking for NUM_SLOT:                     ****/
    /****     STATE_NUM_CNTL_CHAN_SLOT:      Looking for NUM_CNTL_CHAN_SLOT:           ****/
    /****     STATE_NUM_CNTL_FREQ:           Looking for NUM_CNTL_FREQ:                ****/
    /****     STATE_NUM_BDY_PT:              Looking for NUM_BDY_PTS:                  ****/
    /****     STATE_BDY_PT:                  Looking for BDY_PT_i:                     ****/
    /****     STATE_NUM_COMM_SUBNET          Looking for NUM_COMM_SUBNET:              ****/
    /****     STATE_COMM_SUBNET              Looking for SUBNET_i:                     ****/
    /****     STATE_COMM_SUBNET_TRAFFIC      Looking for TRAFFIC:                      ****/
    /****     STATE_COMM_SUBNET_NUM_BDY_PT   Looking for NUM_BDY_PT:                   ****/
    /****     STATE_COMM_SUBNET_BDY_PT       Looking for BDY_PT_i:                     ****/
    /****     STATE_NUM_LREG_SUBNET          Looking for NUM_LREG_SUBNET:              ****/
    /****     STATE_LREG_SUBNET              Looking for SUBNET_i:                     ****/
    /****     STATE_LREG_SUBNET_TRAFFIC      Looking for TRAFFIC:                      ****/
    /****     STATE_LREG_SUBNET_NUM_BDY_PT   Looking for NUM_BDY_PT:                   ****/
    /****     STATE_LREG_SUBNET_BDY_PT       Looking for BDY_PT_i:                     ****/
    /****     STATE_NUM_ANTENNA_TYPE:        Looking for NUM_ANTENNA_TYPE:             ****/
    /****     STATE_ANTENNA_TYPE:            Looking for ANTENNA_TYPE_i:               ****/
    /****     STATE_NUM_PROP_MODEL:          Looking for NUM_PROP_MODEL:               ****/
    /****     STATE_PROP_MODEL:              Looking for PROP_MODEL_i:                 ****/
    /****     STATE_NUM_CELL:                Looking for NUM_CELL:                     ****/
    /****     STATE_CELL:                    Looking for CELL_i                        ****/
    /****     STATE_POSN:                    Looking for POSN_i                        ****/
    /****     STATE_NUM_SECTOR:              Looking for NUM_SECTOR:                   ****/
    /****     STATE_SECTOR:                  Looking for SECTOR_i:                     ****/
    /****     STATE_SECTOR_COMMENT:          Looking for COMMENT:                      ****/
    /****     STATE_SECTOR_CSID:             Looking for CSID:                         ****/
    /****     STATE_SECTOR_CS_NUMBER:        Looking for CS_NUMBER:                    ****/
    /****     STATE_COMM_TRAFFIC:            Looking for COMM_TRAFFIC:                 ****/
    /****     STATE_LREG_TRAFFIC:            Looking for LREG_TRAFFIC:                 ****/
    /****     STATE_ANGLE_DEG:               Looking for ANGLE_DEG:                    ****/
    /****     STATE_SEC_ANTENNA_TYPE:        Looking for ANTENNA_TYPE:                 ****/
    /****     STATE_ANTENNA_HEIGHT:          Looking for ANTENNA_HEIGHT:               ****/
    /****     STATE_SEC_PROP_MODEL:          Looking for PROP_MODEL:                   ****/
    /****     STATE_TX_POWER:                Looking for TX_POWER:                     ****/
    /****     STATE_NUM_PHYSICAL_TX:         Looking for NUM_PHYSICAL_TX:              ****/
    /****     STATE_HAS_ACCESS_CONTROL       Looking for HAS_ACCESS_CONTROL:           ****/
    /****     STATE_CNTL_CHAN_SLOT           Looking for CNTL_CHAN_SLOT:               ****/
    /****     STATE_NUM_UNUSED_FREQ          Looking for NUM_UNUSED_FREQ:              ****/
    /****     STATE_UNUSED_FREQ              Looking for UNUSED_FREQ_i:                ****/
    /****     STATE_DONE:                    Done                                      ****/
    /**************************************************************************************/

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_FREQ;
                    break;
                case STATE_NUM_FREQ:
                    if (strcmp(str1, "NUM_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_freq = atoi(str1);
                    } else {
                        num_freq = 0;
                    }
                    if (num_freq <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_freq = %d must be > 0\n", filename, linenum, num_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SLOT;
                    break;
                case STATE_NUM_SLOT:
                    if (strcmp(str1, "NUM_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_slot = atoi(str1);
                    } else {
                        num_slot = 0;
                    }
                    if (num_slot <= 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_slot = %d must be > 0\n", filename, linenum, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_CNTL_CHAN_SLOT;
                    break;
                case STATE_NUM_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "NUM_CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cntl_chan_slot = atoi(str1);
                    } else {
                        num_cntl_chan_slot = 0;
                    }
                    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
                            filename, linenum, num_cntl_chan_slot, num_slot);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_FREQ;
                    break;
                case STATE_CNTL_CHAN_FREQ:
                    if (strcmp(str1, "CNTL_CHAN_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cntl_chan_freq = atoi(str1);
                    } else {
                        cntl_chan_freq = -1;
                    }
                    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cntl_chan_freq = %d must be between 0 and %d\n",
                                filename, linenum, cntl_chan_freq, num_freq-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_COMM_SUBNET;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                        num_traffic_type = 2;
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        traffic_type_list[0] = (TrafficTypeClass *) new PHSTrafficTypeClass("COMM");
                        traffic_type_list[1] = (TrafficTypeClass *) new PHSTrafficTypeClass("LREG");
                        ((PHSTrafficTypeClass *) traffic_type_list[1])->num_attempt_handover = 0;

                        traffic_type_list[0]->color = 16711680;
                        traffic_type_list[1]->color = 16777215;

                        traffic_type_list[0]->duration_dist = CConst::ExpoDist;
                        traffic_type_list[1]->duration_dist = CConst::UnifDist;
                        num_subnet[0] = 0;
                        num_subnet[1] = 0;
                    }
                    break;
                case STATE_NUM_COMM_SUBNET:
                    if (strcmp(str1, "NUM_COMM_SUBNET:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_COMM_SUBNET:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[0] = atoi(str1);
                    } else {
                        num_subnet[0] = 0;
                    }

                    if (num_subnet[0] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_comm_subnet = %d must be >= 0\n", filename, linenum, num_subnet[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[0]) {
                        subnet_list[0] = (SubnetClass **) malloc((num_subnet[0])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[0]-1; subnet_idx++) {
                            subnet_list[0][subnet_idx]    = new SubnetClass();
                            subnet_list[0][subnet_idx]->p = new PolygonClass();
                        }
                        subnet_idx = 0;
                        state = STATE_COMM_SUBNET;
                    } else {
                        subnet_list[0] = (SubnetClass **) NULL;
                        state = STATE_NUM_LREG_SUBNET;
                    }
                    break;
                case STATE_COMM_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[0][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);

                    state = STATE_COMM_SUBNET_TRAFFIC;
                    break;
                case STATE_COMM_SUBNET_TRAFFIC:
                    if (strcmp(str1, "TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_COMM_SUBNET_COLOR;
                    break;
                case STATE_COMM_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_COMM_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_COMM_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_COMM_SUBNET_SEGMENT;
                    break;
                case STATE_COMM_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_COMM_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_COMM_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_COMM_SUBNET_BDY_PT;
                    break;
                case STATE_COMM_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_COMM_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_COMM_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[0]-1) {
                                state = STATE_COMM_SUBNET;
                            } else {
                                state = STATE_NUM_LREG_SUBNET;
                            }
                        }
                    }
                    break;
                case STATE_NUM_LREG_SUBNET:
                    if (strcmp(str1, "NUM_LREG_SUBNET:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_LREG_SUBNET:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[1] = atoi(str1);
                    } else {
                        num_subnet[1] = 0;
                    }

                    if (num_subnet[1] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_lreg_subnet = %d must be >= 0\n", filename, linenum, num_subnet[1]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[1]) {
                        subnet_list[1] = (SubnetClass **) malloc((num_subnet[1])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[1]-1; subnet_idx++) {
                            subnet_list[1][subnet_idx]    = new SubnetClass();
                            subnet_list[1][subnet_idx]->p = new PolygonClass();
                        }
                        subnet_idx = 0;
                        state = STATE_LREG_SUBNET;
                    } else {
                        subnet_list[1] = (SubnetClass **) NULL;
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_LREG_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[1][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);

                    state = STATE_LREG_SUBNET_TRAFFIC;
                    break;
                case STATE_LREG_SUBNET_TRAFFIC:
                    if (strcmp(str1, "TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_LREG_SUBNET_COLOR;
                    break;
                case STATE_LREG_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_LREG_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_LREG_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_LREG_SUBNET_SEGMENT;
                    break;
                case STATE_LREG_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_LREG_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_LREG_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BYD_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n",filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_LREG_SUBNET_BDY_PT;
                    break;
                case STATE_LREG_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d\" specified \n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_LREG_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_LREG_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[1]-1) {
                                state = STATE_LREG_SUBNET;
                            } else {
                                state = STATE_NUM_ANTENNA_TYPE;
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        prop_model_strid = (char *) NULL;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_CELL;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new PHSCellClass();
                    }
                    cell_idx = 0;

                    SectorClass::num_traffic = 2;
                    SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    SectorClass::traffic_type_idx_list[0] = 0;
                    SectorClass::traffic_type_idx_list[1] = 1;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(cell);

#if 0
                        cell->sector_list[sector_idx]->stat_count = (StatCountClass *) NULL;
                        // cell->sector_list[sector_idx]->stat_count = (StatCountClass *) malloc(sizeof(StatCountClass));
                        cell->sector_list[sector_idx]->num_road_test_pt = 0;
                        cell->sector_list[sector_idx]->sync_level       = -1;
                        cell->sector_list[sector_idx]->num_call = 0;
                        cell->sector_list[sector_idx]->call_list = (CallClass **) NULL;
                        cell->sector_list[sector_idx]->active = 1;
                        // stat_count = cell->sector_list[sector_idx]->stat_count;
                        // reset_stat_count(stat_count, num_attempt_assign_channel, 1);
#endif
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_SECTOR_CSID;
                    break;
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_COMM_TRAFFIC;
                    break;
                case STATE_COMM_TRAFFIC:
                    if (strcmp(str1, "COMM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "NULL")==0) {
                            sector->meas_ctr_list[0] = 0.0;
                        } else {
                            sector->meas_ctr_list[0] = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COMM_TRAFFIC:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_LREG_TRAFFIC;
                    break;
                case STATE_LREG_TRAFFIC:
                    if (strcmp(str1, "LREG_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"LREG_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "NULL")==0) {
                            sector->meas_ctr_list[1] = 0.0;
                        } else {
                            sector->meas_ctr_list[1] = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"LREG_TRAFFIC:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANGLE_DEG;
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_PHYSICAL_TX;
                    break;
                case STATE_NUM_PHYSICAL_TX:
                    if (strcmp(str1, "NUM_PHYSICAL_TX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PHYSICAL_TX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_physical_tx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PHYSICAL_TX:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_HAS_ACCESS_CONTROL;
                    break;
                case STATE_HAS_ACCESS_CONTROL:
                    if (strcmp(str1, "HAS_ACCESS_CONTROL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HAS_ACCESS_CONTROL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->has_access_control = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HAS_ACCESS_CONTROL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CNTL_CHAN_SLOT;
                    break;
                case STATE_CNTL_CHAN_SLOT:
                    if (strcmp(str1, "CNTL_CHAN_SLOT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CNTL_CHAN_SLOT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->cntl_chan_slot = atoi(str1);
                    } else {
                        sector->cntl_chan_slot = -1;
                    }

                    if ((sector->cntl_chan_slot < -1)||(sector->cntl_chan_slot > num_cntl_chan_slot-1)) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->cntl_chan_slot = %d must be between 0 and %d\n",
                                filename, linenum, sector->cntl_chan_slot, num_cntl_chan_slot-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector->cntl_chan_eff_tch_slot = 
                        ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
                    state = STATE_NUM_UNUSED_FREQ;
                    break;
                case STATE_NUM_UNUSED_FREQ:
                    if (strcmp(str1, "NUM_UNUSED_FREQ:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_UNUSED_FREQ:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->num_unused_freq = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_UNUSED_FREQ:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx = 0;
                    if (sector->num_unused_freq > 0) {
                        sector->unused_freq = IVECTOR(sector->num_unused_freq);
                        state = STATE_UNUSED_FREQ;
                    } else if (sector->num_unused_freq == 0) {
                        sector->unused_freq = (int *) NULL;
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "NUM_UNUSED_FREQ=%d must be >= 0 \n", filename, linenum,sector->num_unused_freq);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case STATE_UNUSED_FREQ:
                    sprintf(str, "UNUSED_FREQ_%d:", unused_freq_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ival = atoi(str1);
                        if ((ival < 0) || (ival > num_freq-1)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "%s = %d must be between 0 and %d\n", filename, linenum, str, ival, num_freq-1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->unused_freq[unused_freq_idx] = ival;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"UNUSED_FREQ_%d\" specified\n", filename, linenum,unused_freq_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    unused_freq_idx++;
                    if (unused_freq_idx <= sector->num_unused_freq-1) {
                        state = STATE_UNUSED_FREQ;
                    } else {
                        sector_idx++;
                        if (sector_idx <= cell->num_sector-1) {
                            state = STATE_SECTOR;
                        } else {
                            cell_idx++;  
                            if (cell_idx <= num_cell-1) {
                                state = STATE_CELL;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_geometry                                                         ****/
/**** Display geometry information.  This is useful to verify that this information    ****/
/**** was correctly read in from the geometry file.                                    ****/
/**** FORMAT OPTIONS:                                                                  ****/
/**** fmt = 0: Use geometry file format                                                ****/
/**** fmt = 1: Use format for plotting data with visual basic                          ****/
/**** 5/29/03: fmt=1 option no longer supported.                                       ****/
/**** 9/14/05: Removed fmt parameter.                                                  ****/
/******************************************************************************************/
void PHSNetworkClass::print_geometry(char *filename)
{
#if (DEMO == 0)
    int i, n, bdy_pt_idx, cell_idx, sector_idx;
    int ant_idx, subnet_idx, segment_idx, pm_idx, tt_idx, tti_idx;
    int antenna_type;
    CellClass *cell;
    PHSSectorClass *sector;
    SubnetClass *subnet;
    AntennaClass *antenna;
    PHSTrafficTypeClass *traffic_type;
    char *chptr;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (!system_bdy) {
        sprintf( msg, "ERROR: Geometry not defined\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "# WISIM GEOMETRY FILE\n");
    chptr += sprintf(chptr, "TECHNOLOGY: PHS\n");
    chptr += sprintf(chptr, "FORMAT: %s\n", PHS_GEOMETRY_FORMAT);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
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
    chptr += sprintf(chptr, "RESOLUTION: %9.7f\n", resolution);
    chptr += sprintf(chptr, "NUM_FREQ: %d\n", num_freq);
    chptr += sprintf(chptr, "NUM_SLOT: %d\n", num_slot);
    chptr += sprintf(chptr, "NUM_CNTL_CHAN_SLOT: %d\n", num_cntl_chan_slot);
    chptr += sprintf(chptr, "CNTL_CHAN_FREQ: %d\n", cntl_chan_freq);
    chptr += sprintf(chptr, "CSID_FORMAT: %d\n", csid_format);
    chptr += sprintf(chptr, "ST_DATA_PERIOD: %d\n", PHSSectorClass::st_data_period);
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_BDY_PT: %d\n", system_bdy->num_bdy_pt[0]);
    PRMSG(fp, msg);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "BDY_PT_%d: %15.13f %15.13f\n",
            bdy_pt_idx, idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]), idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]));
        PRMSG(fp, msg);
    }
    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "NUM_TRAFFIC_TYPE: %d\n", num_traffic_type);
    PRMSG(fp, msg);
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        traffic_type = (PHSTrafficTypeClass *) traffic_type_list[tt_idx];
        chptr = msg;
        chptr += sprintf(chptr, "TRAFFIC_TYPE_%d:", tt_idx);
        if (traffic_type->strid) {
            chptr += sprintf(chptr, " \"%s\"", traffic_type->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    COLOR: %d\n", traffic_type->color);
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: EXPONENTIAL:%15.13f\n", traffic_type->mean_time);
        } else if (traffic_type->duration_dist == CConst::UnifDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: UNIFORM:%15.13f:%15.13f\n",
                traffic_type->min_time, traffic_type->max_time);
        } else {
            sprintf(msg, "ERROR: geometry already defined\n");
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "NUM_SUBNET_%d: %d\n", tt_idx, num_subnet[tt_idx]);
        PRMSG(fp, msg);
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];

            chptr = msg;
            chptr += sprintf(chptr, "SUBNET_%d:", subnet_idx);
            if (subnet->strid) {
                chptr += sprintf(chptr, " \"%s\"", subnet->strid);
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "# Contained cells:");
            n = 0;
            for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                cell = cell_list[cell_idx];
                if (subnet->p->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    chptr += sprintf(chptr, " %d", cell_idx);
                    n++;
                }
            }
            if (n == 0) { 
                chptr += sprintf(chptr, " NONE");
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "    ARRIVAL_RATE: %9.7f\n", subnet->arrival_rate);
            chptr += sprintf(chptr, "    COLOR: %d\n", subnet->color);
            chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", subnet->p->num_segment);
            PRMSG(fp, msg);
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
                chptr = msg;
                chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
                chptr += sprintf(chptr, "        NUM_BDY_PT: %d\n", subnet->p->num_bdy_pt[segment_idx]);
                PRMSG(fp, msg);
                for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                    chptr = msg;
                    chptr += sprintf(chptr, "            BDY_PT_%d: %15.13f %15.13f\n", bdy_pt_idx,
                        idx_to_x(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]),
                        idx_to_y(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]));
                    PRMSG(fp, msg);
                }
            }
        }
        chptr = msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);
    }

    sprintf(msg, "NUM_ANTENNA_TYPE: %d\n", num_antenna_type);
    PRMSG(fp, msg);
    for (ant_idx=0; ant_idx<=num_antenna_type-1; ant_idx++) {
        antenna = antenna_type_list[ant_idx];
        antenna_type = antenna->get_type();
        if (antenna_type == CConst::antennaOmni) {
            sprintf(msg, "ANTENNA_TYPE_%d: OMNI\n", ant_idx);
        } else {
            sprintf(msg, "ANTENNA_TYPE_%d: %s %s\n", ant_idx,
                (antenna_type == CConst::antennaLUT_H) ? "LUT_H" :
                (antenna_type == CConst::antennaLUT_V) ? "LUT_V" : "LUT",
                antenna->get_filename());
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    sprintf(msg, "NUM_PROP_MODEL: %d\n", num_prop_model);
    PRMSG(fp, msg);
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "PROP_MODEL_%d:", pm_idx);
        if (prop_model_list[pm_idx]->get_strid()) {
            chptr += sprintf(chptr, " \"%s\"", prop_model_list[pm_idx]->get_strid());
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        prop_model_list[pm_idx]->print_params(fp, msg, CConst::geometryFileType);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_TRAFFIC: %d\n", SectorClass::num_traffic);
    PRMSG(fp, msg);

    for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "    TRAFFIC_TYPE_IDX_%d: %d\n",   tti_idx, SectorClass::traffic_type_idx_list[tti_idx]);
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_CELL: %d\n", num_cell);
    PRMSG(fp, msg);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];

        chptr = msg;
        chptr += sprintf(chptr, "CELL_%d:", cell_idx);
        if (cell->strid) {
            chptr += sprintf(chptr, " \"%s\"", cell->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    POSN: %9.7f %9.7f\n", idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
        chptr += sprintf(chptr, "    BM_IDX: %d\n", cell->bm_idx);
        chptr += sprintf(chptr, "    COLOR: %d\n", cell->color);
        PRMSG(fp, msg);

        chptr = msg;
        chptr += sprintf(chptr, "    NUM_SECTOR: %d\n", cell->num_sector);
        PRMSG(fp, msg);

        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            chptr = msg;
            chptr += sprintf(chptr, "    SECTOR_%d:\n", sector_idx);
            chptr += sprintf(chptr, "        COMMENT: %s\n", (sector->comment ? sector->comment : ""));
            chptr += sprintf(chptr, "        CSID:");
            if (sector->csid_hex) {
                chptr += sprintf(chptr, " ");
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
            }
            chptr += sprintf(chptr, "\n");
            chptr += sprintf(chptr, "        CS_NUMBER: %d\n", sector->gw_csc_cs);

            for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
                chptr += sprintf(chptr, "        MEAS_CTR_%d: %.7f\n",   tti_idx, sector->meas_ctr_list[tti_idx]);
            }
            chptr += sprintf(chptr, "        ANGLE_DEG: %.7f\n",      sector->antenna_angle_rad*180.0/PI);
            chptr += sprintf(chptr, "        ANTENNA_TYPE: %d\n",       sector->antenna_type);
            chptr += sprintf(chptr, "        ANTENNA_HEIGHT: %.7f\n", sector->antenna_height);
            chptr += sprintf(chptr, "        PROP_MODEL: %d\n",         sector->prop_model);
            chptr += sprintf(chptr, "        TX_POWER: %.7f\n",       sector->tx_pwr);
            chptr += sprintf(chptr, "        NUM_PHYSICAL_TX: %d\n",    sector->num_physical_tx);
            chptr += sprintf(chptr, "        HAS_ACCESS_CONTROL: %d\n", sector->has_access_control);
            chptr += sprintf(chptr, "        CNTL_CHAN_SLOT: %d\n",     sector->cntl_chan_slot);
            chptr += sprintf(chptr, "        NUM_UNUSED_FREQ: %d\n",    sector->num_unused_freq);
            PRMSG(fp, msg);
            for (i=0; i<=sector->num_unused_freq-1; i++) {
                chptr = msg;
                chptr += sprintf(chptr, "            UNUSED_FREQ_%d: %d\n", i, sector->unused_freq[i]);
                PRMSG(fp, msg);
            }
            n = (sector->st_data ? sector->st_param_list->getSize() : 0);
            chptr = msg;
            chptr += sprintf(chptr, "        NUM_ST_PARAM: %d\n", n);
            PRMSG(fp, msg);
            for (i=0; i<=n-1; i++) {
                chptr = msg;
                chptr += sprintf(chptr, "            ST_PARAM_%d: %d\n", i, sector->st_data[i]);
                PRMSG(fp, msg);
            }
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

#endif

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** WCDMA                                                                            ****/
/******************************************************************************************/
/**** FUNCTION: WCDMANetworkClass::read_geometry_1_0                                   ****/
/**** Read WCDMA format 1.0 geometry file.                                             ****/
/******************************************************************************************/
void WCDMANetworkClass::read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    WCDMASectorClass *sector         = (WCDMASectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new WCDMATrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s(%d)\" contains %s CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new WCDMACellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new WCDMASectorClass(cell);

#if 0
                        cell->sector_list[sector_idx]->stat_count = (StatCountClass *) NULL;
                        // cell->sector_list[sector_idx]->stat_count = (StatCountClass *) malloc(sizeof(StatCountClass));
                        cell->sector_list[sector_idx]->num_road_test_pt = 0;
                        cell->sector_list[sector_idx]->sync_level       = -1;
                        cell->sector_list[sector_idx]->num_call = 0;
                        cell->sector_list[sector_idx]->call_list = (CallClass **) NULL;
                        cell->sector_list[sector_idx]->active = 1;
                        // stat_count = cell->sector_list[sector_idx]->stat_count;
                        // reset_stat_count(stat_count, num_attempt_assign_channel, 1);
#endif
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (WCDMASectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    state = STATE_ANGLE_DEG;
                    break;
#if 0
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*PHSSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, PHSSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
#endif
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector_idx++;
                    if (sector_idx <= cell->num_sector-1) {
                        state = STATE_SECTOR;
                    } else {
                        cell_idx++;  
                        if (cell_idx <= num_cell-1) {
                            state = STATE_CELL;
                        } else {
                            state = STATE_DONE;
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_geometry                                                         ****/
/**** Display geometry information.  This is useful to verify that this information    ****/
/**** was correctly read in from the geometry file.                                    ****/
/**** FORMAT OPTIONS:                                                                  ****/
/**** fmt = 0: Use geometry file format                                                ****/
/**** fmt = 1: Use format for plotting data with visual basic                          ****/
/**** 5/29/03: fmt=1 option no longer supported.                                       ****/
/**** 9/14/05: Removed fmt parameter.                                                  ****/
/******************************************************************************************/
void WCDMANetworkClass::print_geometry(char *filename)
{
#if (DEMO == 0)
    int n, bdy_pt_idx, cell_idx, sector_idx;
    int ant_idx, subnet_idx, segment_idx, pm_idx, tt_idx, tti_idx;
    int antenna_type;
    CellClass *cell;
    PHSSectorClass *sector;
    SubnetClass *subnet;
    AntennaClass *antenna;
    TrafficTypeClass *traffic_type;
    char *chptr;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (!system_bdy) {
        sprintf( msg, "ERROR: Geometry not defined\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "# Geometry Data\n");
    chptr += sprintf(chptr, "TECHNOLOGY: WCDMA\n");
    chptr += sprintf(chptr, "FORMAT: %s\n", WCDMA_GEOMETRY_FORMAT);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
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
    chptr += sprintf(chptr, "RESOLUTION: %9.7f\n", resolution);
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_BDY_PT: %d\n", system_bdy->num_bdy_pt[0]);
    PRMSG(fp, msg);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "BDY_PT_%d: %15.13f %15.13f\n",
            bdy_pt_idx, idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]), idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]));
        PRMSG(fp, msg);
    }
    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "NUM_TRAFFIC_TYPE: %d\n", num_traffic_type);
    PRMSG(fp, msg);
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        traffic_type = traffic_type_list[tt_idx];
        chptr = msg;
        chptr += sprintf(chptr, "TRAFFIC_TYPE_%d:", tt_idx);
        if (traffic_type->strid) {
            chptr += sprintf(chptr, " \"%s\"", traffic_type->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    COLOR: %d\n", traffic_type->color);
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: EXPONENTIAL:%15.13f\n", traffic_type->mean_time);
        } else if (traffic_type->duration_dist == CConst::UnifDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: UNIFORM:%15.13f:%15.13f\n",
                traffic_type->min_time, traffic_type->max_time);
        } else {
            sprintf(msg, "ERROR: Invalid duration distribution\n");
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "NUM_SUBNET_%d: %d\n", tt_idx, num_subnet[tt_idx]);
        PRMSG(fp, msg);
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];

            chptr = msg;
            chptr += sprintf(chptr, "SUBNET_%d:", subnet_idx);
            if (subnet->strid) {
                chptr += sprintf(chptr, " \"%s\"", subnet->strid);
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "# Contained cells:");
            n = 0;
            for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                cell = cell_list[cell_idx];
                if (subnet->p->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    chptr += sprintf(chptr, " %d", cell_idx);
                    n++;
                }
            }
            if (n == 0) { 
                chptr += sprintf(chptr, " NONE");
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "    ARRIVAL_RATE: %9.7f\n", subnet->arrival_rate);
            chptr += sprintf(chptr, "    COLOR: %d\n", subnet->color);
            chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", subnet->p->num_segment);
            PRMSG(fp, msg);
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
                chptr = msg;
                chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
                chptr += sprintf(chptr, "        NUM_BDY_PT: %d\n", subnet->p->num_bdy_pt[segment_idx]);
                PRMSG(fp, msg);
                for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                    chptr = msg;
                    chptr += sprintf(chptr, "            BDY_PT_%d: %15.13f %15.13f\n", bdy_pt_idx,
                        idx_to_x(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]),
                        idx_to_y(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]));
                    PRMSG(fp, msg);
                }
            }
        }
        chptr = msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);
    }

    sprintf(msg, "NUM_ANTENNA_TYPE: %d\n", num_antenna_type);
    PRMSG(fp, msg);
    for (ant_idx=0; ant_idx<=num_antenna_type-1; ant_idx++) {
        antenna = antenna_type_list[ant_idx];
        antenna_type = antenna->get_type();
        if (antenna_type == CConst::antennaOmni) {
            sprintf(msg, "ANTENNA_TYPE_%d: OMNI\n", ant_idx);
        } else {
            sprintf(msg, "ANTENNA_TYPE_%d: %s %s\n", ant_idx,
                (antenna_type == CConst::antennaLUT_H) ? "LUT_H" :
                (antenna_type == CConst::antennaLUT_V) ? "LUT_V" : "LUT",
                antenna->get_filename());
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    sprintf(msg, "NUM_PROP_MODEL: %d\n", num_prop_model);
    PRMSG(fp, msg);
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "PROP_MODEL_%d:", pm_idx);
        if (prop_model_list[pm_idx]->get_strid()) {
            chptr += sprintf(chptr, " \"%s\"", prop_model_list[pm_idx]->get_strid());
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        prop_model_list[pm_idx]->print_params(fp, msg, CConst::geometryFileType);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_TRAFFIC: %d\n", SectorClass::num_traffic);
    PRMSG(fp, msg);

    for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "    TRAFFIC_TYPE_IDX_%d: %d\n",   tti_idx, SectorClass::traffic_type_idx_list[tti_idx]);
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_CELL: %d\n", num_cell);
    PRMSG(fp, msg);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];

        chptr = msg;
        chptr += sprintf(chptr, "CELL_%d:", cell_idx);
        if (cell->strid) {
            chptr += sprintf(chptr, " \"%s\"", cell->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    POSN: %9.7f %9.7f\n", idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
        chptr += sprintf(chptr, "    COLOR: %d\n", cell->color);
        PRMSG(fp, msg);

        chptr = msg;
        chptr += sprintf(chptr, "    NUM_SECTOR: %d\n", cell->num_sector);
        PRMSG(fp, msg);

        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            chptr = msg;
            chptr += sprintf(chptr, "    SECTOR_%d:\n", sector_idx);
            chptr += sprintf(chptr, "        COMMENT: %s\n", (sector->comment ? sector->comment : ""));
#if 0
            chptr += sprintf(chptr, "        CSID:");
            if (sector->csid_hex) {
                chptr += sprintf(chptr, " ");
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
            }
            chptr += sprintf(chptr, "\n");
            chptr += sprintf(chptr, "        CS_NUMBER: %d\n", sector->gw_csc_cs);

            for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
                chptr += sprintf(chptr, "        MEAS_CTR_%d: %.7f\n",   tti_idx, sector->meas_ctr_list[tti_idx]);
            }
#endif
            chptr += sprintf(chptr, "        ANGLE_DEG: %.7f\n",      sector->antenna_angle_rad*180.0/PI);
            chptr += sprintf(chptr, "        ANTENNA_TYPE: %d\n",       sector->antenna_type);
            chptr += sprintf(chptr, "        ANTENNA_HEIGHT: %.7f\n", sector->antenna_height);
            chptr += sprintf(chptr, "        PROP_MODEL: %d\n",         sector->prop_model);
            chptr += sprintf(chptr, "        TX_POWER: %.7f\n",       sector->tx_pwr);
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

#endif

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** CDMA2000                                                                         ****/
/******************************************************************************************/
/**** FUNCTION: CDMA2000NetworkClass::read_geometry_1_0                                ****/
/**** Read CDMA2000 format 1.0 geometry file.                                          ****/
/******************************************************************************************/
void CDMA2000NetworkClass::read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i, ival;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int unused_freq_idx          = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    CDMA2000SectorClass *sector    = (CDMA2000SectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_BM_IDX,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_TX_POWER,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            if ((utm_equatorial_radius < 6.0e6) || (utm_equatorial_radius > 7.0e6)) {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n"
                                                 "Equatorial radius = %f must be between %f and %f\n", filename, linenum, utm_equatorial_radius, 6.0e6, 7.0e6);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new CDMA2000TrafficTypeClass(p_strid);
                    traffic_type = traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);
                    if (!subnet->strid) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "SUBNET_%d has no name\n", filename, linenum, subnet_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        num_prop_model = 0;
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
                        prop_model_list[pm_idx] = (PropModelClass *) NULL;
                    }
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "RSQUARED")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new RSquaredPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelFullClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERSYMFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterSymFullPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPO")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPOSLOPE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTEREXPOLINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterExpoLinearPropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s\" contains %d CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename, num_cell);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new CDMA2000CellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_BM_IDX;
                    break;
                case STATE_CELL_BM_IDX:
                    if (strcmp(str1, "BM_IDX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BM_IDX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->bm_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (cell->bm_idx < 0) || (cell->bm_idx > cell->num_bm-1) ) {
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" has invalid BM_IDX = %d, using value 0\n",
                                         filename, linenum, cell->bm_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                        cell->bm_idx = 0;
                    }

                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new CDMA2000SectorClass(cell);
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (CDMA2000SectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    tti_idx = 0;
                    state = STATE_SECTOR_MEAS_CTR;
                    break;
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector_idx++;
                    if (sector_idx <= cell->num_sector-1) {
                        state = STATE_SECTOR;
                    } else {
                        cell_idx++;  
                        if (cell_idx <= num_cell-1) {
                            state = STATE_CELL;
                        } else {
                            state = STATE_DONE;
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_geometry                                                         ****/
/**** Display geometry information.  This is useful to verify that this information    ****/
/**** was correctly read in from the geometry file.                                    ****/
/******************************************************************************************/
void CDMA2000NetworkClass::print_geometry(char *filename)
{
#if (DEMO == 0)
    int n, bdy_pt_idx, cell_idx, sector_idx;
    int ant_idx, subnet_idx, segment_idx, pm_idx, tt_idx, tti_idx;
    int antenna_type;
    CellClass *cell;
    PHSSectorClass *sector;
    SubnetClass *subnet;
    AntennaClass *antenna;
    TrafficTypeClass *traffic_type;
    char *chptr;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (!system_bdy) {
        sprintf( msg, "ERROR: Geometry not defined\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "# Geometry Data\n");
    chptr += sprintf(chptr, "TECHNOLOGY: CDMA2000\n");
    chptr += sprintf(chptr, "FORMAT: %s\n", CDMA2000_GEOMETRY_FORMAT);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
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
    chptr += sprintf(chptr, "RESOLUTION: %9.7f\n", resolution);
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_BDY_PT: %d\n", system_bdy->num_bdy_pt[0]);
    PRMSG(fp, msg);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "BDY_PT_%d: %15.13f %15.13f\n",
            bdy_pt_idx, idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]), idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]));
        PRMSG(fp, msg);
    }
    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "NUM_TRAFFIC_TYPE: %d\n", num_traffic_type);
    PRMSG(fp, msg);
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        traffic_type = traffic_type_list[tt_idx];
        chptr = msg;
        chptr += sprintf(chptr, "TRAFFIC_TYPE_%d:", tt_idx);
        if (traffic_type->strid) {
            chptr += sprintf(chptr, " \"%s\"", traffic_type->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    COLOR: %d\n", traffic_type->color);
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: EXPONENTIAL:%15.13f\n", traffic_type->mean_time);
        } else if (traffic_type->duration_dist == CConst::UnifDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: UNIFORM:%15.13f:%15.13f\n",
                traffic_type->min_time, traffic_type->max_time);
        } else {
            sprintf(msg, "ERROR: Invalid duration distribution\n");
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "NUM_SUBNET_%d: %d\n", tt_idx, num_subnet[tt_idx]);
        PRMSG(fp, msg);
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];

            chptr = msg;
            chptr += sprintf(chptr, "SUBNET_%d:", subnet_idx);
            if (subnet->strid) {
                chptr += sprintf(chptr, " \"%s\"", subnet->strid);
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "# Contained cells:");
            n = 0;
            for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                cell = cell_list[cell_idx];
                if (subnet->p->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    chptr += sprintf(chptr, " %d", cell_idx);
                    n++;
                }
            }
            if (n == 0) { 
                chptr += sprintf(chptr, " NONE");
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "    ARRIVAL_RATE: %9.7f\n", subnet->arrival_rate);
            chptr += sprintf(chptr, "    COLOR: %d\n", subnet->color);
            chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", subnet->p->num_segment);
            PRMSG(fp, msg);
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
                chptr = msg;
                chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
                chptr += sprintf(chptr, "        NUM_BDY_PT: %d\n", subnet->p->num_bdy_pt[segment_idx]);
                PRMSG(fp, msg);
                for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                    chptr = msg;
                    chptr += sprintf(chptr, "            BDY_PT_%d: %15.13f %15.13f\n", bdy_pt_idx,
                        idx_to_x(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]),
                        idx_to_y(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]));
                    PRMSG(fp, msg);
                }
            }
        }
        chptr = msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);
    }

    sprintf(msg, "NUM_ANTENNA_TYPE: %d\n", num_antenna_type);
    PRMSG(fp, msg);
    for (ant_idx=0; ant_idx<=num_antenna_type-1; ant_idx++) {
        antenna = antenna_type_list[ant_idx];
        antenna_type = antenna->get_type();
        if (antenna_type == CConst::antennaOmni) {
            sprintf(msg, "ANTENNA_TYPE_%d: OMNI\n", ant_idx);
        } else {
            sprintf(msg, "ANTENNA_TYPE_%d: %s %s\n", ant_idx,
                (antenna_type == CConst::antennaLUT_H) ? "LUT_H" :
                (antenna_type == CConst::antennaLUT_V) ? "LUT_V" : "LUT",
                antenna->get_filename());
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    sprintf(msg, "NUM_PROP_MODEL: %d\n", num_prop_model);
    PRMSG(fp, msg);
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "PROP_MODEL_%d:", pm_idx);
        if (prop_model_list[pm_idx]->get_strid()) {
            chptr += sprintf(chptr, " \"%s\"", prop_model_list[pm_idx]->get_strid());
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        prop_model_list[pm_idx]->print_params(fp, msg, CConst::geometryFileType);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_TRAFFIC: %d\n", SectorClass::num_traffic);
    PRMSG(fp, msg);

    for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "    TRAFFIC_TYPE_IDX_%d: %d\n",   tti_idx, SectorClass::traffic_type_idx_list[tti_idx]);
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_CELL: %d\n", num_cell);
    PRMSG(fp, msg);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];

        chptr = msg;
        chptr += sprintf(chptr, "CELL_%d:", cell_idx);
        if (cell->strid) {
            chptr += sprintf(chptr, " \"%s\"", cell->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    POSN: %9.7f %9.7f\n", idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
        chptr += sprintf(chptr, "    COLOR: %d\n", cell->color);
        PRMSG(fp, msg);

        chptr = msg;
        chptr += sprintf(chptr, "    NUM_SECTOR: %d\n", cell->num_sector);
        PRMSG(fp, msg);

        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            chptr = msg;
            chptr += sprintf(chptr, "    SECTOR_%d:\n", sector_idx);
            chptr += sprintf(chptr, "        COMMENT: %s\n", (sector->comment ? sector->comment : ""));
#if 0
            chptr += sprintf(chptr, "        CSID:");
            if (sector->csid_hex) {
                chptr += sprintf(chptr, " ");
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
            }
            chptr += sprintf(chptr, "\n");
            chptr += sprintf(chptr, "        CS_NUMBER: %d\n", sector->gw_csc_cs);

            for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
                chptr += sprintf(chptr, "        MEAS_CTR_%d: %.7f\n",   tti_idx, sector->meas_ctr_list[tti_idx]);
            }
#endif
            chptr += sprintf(chptr, "        ANGLE_DEG: %.7f\n",      sector->antenna_angle_rad*180.0/PI);
            chptr += sprintf(chptr, "        ANTENNA_TYPE: %d\n",       sector->antenna_type);
            chptr += sprintf(chptr, "        ANTENNA_HEIGHT: %.7f\n", sector->antenna_height);
            chptr += sprintf(chptr, "        PROP_MODEL: %d\n",         sector->prop_model);
            chptr += sprintf(chptr, "        TX_POWER: %.7f\n",       sector->tx_pwr);
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

#endif

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** WLAN                                                                             ****/
/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::read_geometry_1_0                                    ****/
/**** Read WLAN format 1.0 geometry file.                                              ****/
/******************************************************************************************/
void WLANNetworkClass::read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum)
{
    int i;
    int *iptr                    = (int *) NULL;
    int bdy_pt_idx               = -1;
    int cell_idx                 = -1;
    int sector_idx               = -1;
    int ant_idx                  = -1;
    int subnet_idx               = -1;
    int segment_idx              = -1;
    int tt_idx                   = -1;
    int pm_idx                   = -1;
    int param_idx                = -1;
    int num_prop_param           = -1;
    int tti_idx                  = -1;
    int num_unnamed_prop_model   = 0;
    int maxx, maxy;
    CellClass *cell                = (CellClass   *) NULL;
    WLANSectorClass *sector         = (WLANSectorClass *) NULL;
    SubnetClass *subnet            = (SubnetClass *) NULL;
    WLANTrafficTypeClass *traffic_type = (WLANTrafficTypeClass *) NULL;
    char *str1, str[1000], *chptr;
    char *p_prop_model_strid;
    char *prop_model_strid = (char *) NULL;
    char *p_strid;
    double tmpd;
    double *dptr               = (double *) NULL;

    int type = CConst::NumericInt;

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_RESOLUTION,
        STATE_NUM_BDY_PT,
        STATE_BDY_PT,
        STATE_NUM_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE,
        STATE_TRAFFIC_TYPE_COLOR,
        STATE_TRAFFIC_TYPE_DURATION_DIST,
        STATE_TRAFFIC_TYPE_BIT_PER_PKT,
        STATE_TRAFFIC_TYPE_USER_DATA_RATE,
        STATE_TRAFFIC_TYPE_BASIC_RATE,
        STATE_TRAFFIC_TYPE_PHY_RATE,
        STATE_TRAFFIC_TYPE_HD_RATE,
        STATE_TRAFFIC_TYPE_MEAN_SESSION_DURATION,
        STATE_TRAFFIC_TYPE_DROP_TOTAL_PKT,
        STATE_TRAFFIC_TYPE_DROP_ERROR_PKT,
        STATE_TRAFFIC_TYPE_MEAN_SEGMENT,

        STATE_NUM_SUBNET,
        STATE_SUBNET,
        STATE_SUBNET_ARRIVAL_RATE,
        STATE_SUBNET_COLOR,
        STATE_SUBNET_NUM_SEGMENT,
        STATE_SUBNET_SEGMENT,
        STATE_SUBNET_NUM_BDY_PT,
        STATE_SUBNET_BDY_PT,

        STATE_NUM_ANTENNA_TYPE,
        STATE_ANTENNA_TYPE,
        STATE_NUM_PROP_MODEL,
        STATE_PROP_MODEL_TYPE,
        STATE_PROP_MODEL_PARAM,
        STATE_PROP_MODEL,
        STATE_NUM_TRAFFIC,
        STATE_TRAFFIC_TYPE_IDX,

        STATE_NUM_CELL,
        STATE_CELL,
        STATE_POSN,
        STATE_CELL_BM_IDX,
        STATE_CELL_COLOR,
        STATE_NUM_SECTOR,
        STATE_SECTOR,
        STATE_SECTOR_COMMENT,
        STATE_SECTOR_CSID,
        STATE_SECTOR_CS_NUMBER,
        STATE_SECTOR_MEAS_CTR,
        STATE_ANGLE_DEG,
        STATE_SEC_ANTENNA_TYPE,
        STATE_ANTENNA_HEIGHT,
        STATE_SEC_PROP_MODEL,
        STATE_SEC_CHANNEL,
        STATE_TX_POWER,
        STATE_NUM_PHYSICAL_TX,
        STATE_HAS_ACCESS_CONTROL,
        STATE_CNTL_CHAN_SLOT,
        STATE_NUM_UNUSED_FREQ,
        STATE_UNUSED_FREQ,
        STATE_DONE
    };

    state_enum state;

    state = STATE_COORDINATE_SYSTEM;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            utm_equatorial_radius = atof(str1);
                            if ((utm_equatorial_radius < 6.0e6) || (utm_equatorial_radius > 7.0e6)) {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n"
                                                 "Equatorial radius = %f must be between %f and %f\n", filename, linenum, utm_equatorial_radius, 6.0e6, 7.0e6);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                            str1 = strtok(NULL, ":");
                            utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                utm_north = 0;
                            } else {
                                sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, msg);
                                error_state = 1;
                                return;
                            }
                        } else if (strcmp(str1, "LON_LAT")==0) {
                            coordinate_system = CConst::CoordLONLAT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_RESOLUTION;
                    break;
                case STATE_RESOLUTION:
                    if (strcmp(str1, "RESOLUTION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"RESOLUTION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        resolution = atof(str1);
                    } else {
                        resolution = 0.0;
                    }
                    if (resolution <= 0.0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "resolution = %15.10f must be > 0.0\n", filename, linenum, resolution);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );
                    state = STATE_NUM_BDY_PT;
                    break;
                case STATE_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    system_bdy = new PolygonClass();
                    system_bdy->num_segment = 1;
                    system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
                    system_bdy->bdy_pt_x   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    system_bdy->bdy_pt_y   = (int **) malloc(system_bdy->num_segment * sizeof(int *));
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        system_bdy->num_bdy_pt[0] = atoi(str1);
                    } else {
                        system_bdy->num_bdy_pt[0] = 0;
                    }

                    if (system_bdy->num_bdy_pt[0] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "system_bdy->num_bdy_pt[0] = %d must be >= 3\n", filename, linenum, system_bdy->num_bdy_pt[0]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    system_bdy->bdy_pt_x[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    system_bdy->bdy_pt_y[0] = IVECTOR(system_bdy->num_bdy_pt[0]);
                    bdy_pt_idx = 0;
                    state = STATE_BDY_PT;
                    break;
                case STATE_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_x[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, X = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n",filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, 0, &system_bdy->bdy_pt_y[0][bdy_pt_idx])) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "BDY_PT_%d, Y = %15.13f is not an integer multiple of "
                                             "resolution=%9.7f\n", filename, linenum, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= system_bdy->num_bdy_pt[0]-1) {
                        state = STATE_BDY_PT;
                    } else {
                        state = STATE_NUM_TRAFFIC_TYPE;

                        system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
                        npts_x = maxx - system_startx + 1;
                        npts_y = maxy - system_starty + 1;

                        for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
                            system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
                            system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC_TYPE:
                    if (strcmp(str1, "NUM_TRAFFIC_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_traffic_type = atoi(str1);
                    } else {
                        num_traffic_type = 0;
                    }

                    if (num_traffic_type < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic_type = %d must be >= 0\n", filename, linenum, num_traffic_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_traffic_type) {
                        traffic_type_list = (TrafficTypeClass **) malloc((num_traffic_type)*sizeof(TrafficTypeClass *));
                        num_subnet = IVECTOR(num_traffic_type);
                        for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
                            num_subnet[tt_idx] = 0;
                        }
                        subnet_list = (SubnetClass ***) malloc((num_traffic_type)*sizeof(SubnetClass **));
                        tt_idx = 0;
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        state = STATE_NUM_ANTENNA_TYPE;
                    }
                    break;
                case STATE_TRAFFIC_TYPE:
                    sprintf(str, "TRAFFIC_TYPE_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_strid = (char *) NULL;
                        }
                    }

                    traffic_type_list[tt_idx] = (TrafficTypeClass *) new WLANTrafficTypeClass(p_strid);
                    traffic_type = (WLANTrafficTypeClass *) traffic_type_list[tt_idx];
                    state = STATE_TRAFFIC_TYPE_COLOR;
                    break;
                case STATE_TRAFFIC_TYPE_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DURATION_DIST;
                    break;
                case STATE_TRAFFIC_TYPE_DURATION_DIST:
                    if (strcmp(str1, "DURATION_DIST:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DURATION_DIST:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strncmp(str1, "EXPONENTIAL:", 12)==0) {
                            traffic_type->duration_dist = CConst::ExpoDist;
                            str1 = strtok(str1+12, ":");
                            traffic_type->mean_time = atof(str1);
                        } else if (strncmp(str1, "UNIFORM:", 8)==0) {
                            traffic_type->duration_dist = CConst::UnifDist;
                            str1 = strtok(str1+8, ":");
                            traffic_type->min_time = atof(str1);
                            str1 = strtok(NULL, ":");
                            traffic_type->max_time = atof(str1);
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DURATION_DIST:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_TRAFFIC_TYPE_BIT_PER_PKT;
                    break;
                case STATE_TRAFFIC_TYPE_BIT_PER_PKT:
                    if (strcmp(str1, "BIT_PER_PKT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BIT_PER_PKT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->bit_per_pkt = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BIT_PER_PKT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_USER_DATA_RATE;
                    break;
                case STATE_TRAFFIC_TYPE_USER_DATA_RATE:
                    if (strcmp(str1, "USER_DATA_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"USER_DATA_RATE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->user_data_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"USER_DATA_RATE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_BASIC_RATE;
                    break;
                case STATE_TRAFFIC_TYPE_BASIC_RATE:
                    if (strcmp(str1, "BASIC_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BASIC_RATE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->basic_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BASIC_RATE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_PHY_RATE;
                    break;
                case STATE_TRAFFIC_TYPE_PHY_RATE:
                    if (strcmp(str1, "PHY_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PHY_RATE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->phy_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PHY_RATE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_HD_RATE;
                    break;
                case STATE_TRAFFIC_TYPE_HD_RATE:
                    if (strcmp(str1, "HD_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"HD_RATE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->hd_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"HD_RATE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_MEAN_SESSION_DURATION;
                    break;
                case STATE_TRAFFIC_TYPE_MEAN_SESSION_DURATION:
                    if (strcmp(str1, "MEAN_SESSION_DURATION:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"MEAN_SESSION_DURATION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->mean_session_duration = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"MEAN_SESSION_DURATION:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DROP_TOTAL_PKT;
                    break;
                case STATE_TRAFFIC_TYPE_DROP_TOTAL_PKT:
                    if (strcmp(str1, "DROP_TOTAL_PKT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DROP_TOTAL_PKT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->drop_total_pkt = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DROP_TOTAL_PKT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_DROP_ERROR_PKT;
                    break;
                case STATE_TRAFFIC_TYPE_DROP_ERROR_PKT:
                    if (strcmp(str1, "DROP_ERROR_PKT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"DROP_ERROR_PKT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->drop_error_pkt = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"DROP_ERROR_PKT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TRAFFIC_TYPE_MEAN_SEGMENT;
                    break;
                case STATE_TRAFFIC_TYPE_MEAN_SEGMENT:
                    if (strcmp(str1, "MEAN_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"MEAN_SEGMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        num_traffic_type = tt_idx+1;
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        traffic_type->mean_segment = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"MEAN_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tt_idx++;
                    if (tt_idx <= num_traffic_type-1) {
                        state = STATE_TRAFFIC_TYPE;
                    } else {
                        tt_idx = 0;
                        state = STATE_NUM_SUBNET;
                    }
                    break;
                case STATE_NUM_SUBNET:
                    sprintf(str, "NUM_SUBNET_%d:", tt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_subnet[tt_idx] = atoi(str1);
                    } else {
                        num_subnet[tt_idx] = 0;
                    }

                    if (num_subnet[tt_idx] < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "num_subnet[%d] = %d must be >= 0\n",
                                     filename, linenum, tt_idx, num_subnet[tt_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_subnet[tt_idx]) {
                        subnet_list[tt_idx] = (SubnetClass **) malloc((num_subnet[tt_idx])*sizeof(SubnetClass *));
                        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
                            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
                        }
                        subnet_idx = 0;
                        state = STATE_SUBNET;
                    } else {
                        subnet_list[tt_idx] = (SubnetClass **) NULL;
                        tt_idx++;
                        if (tt_idx <= num_traffic_type-1) {
                            state = STATE_NUM_SUBNET;
                        } else {
                            state = STATE_NUM_ANTENNA_TYPE;
                        }
                    }
                    break;
                case STATE_SUBNET:
                    sprintf(str, "SUBNET_%d:", subnet_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    subnet = subnet_list[tt_idx][subnet_idx];

                    str1 = strtok(NULL, "");
                    subnet->strid = get_strid(str1);
                    if (!subnet->strid) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "SUBNET_%d has no name\n", filename, linenum, subnet_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SUBNET_ARRIVAL_RATE;
                    break;
                case STATE_SUBNET_ARRIVAL_RATE:
                    if (strcmp(str1, "ARRIVAL_RATE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->arrival_rate = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_COLOR;
                    break;
                case STATE_SUBNET_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_SEGMENT;
                    break;
                case STATE_SUBNET_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p = new PolygonClass();
                        subnet->p->num_segment = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_segment < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->p->num_segment = %d must be >= 1\n", filename, linenum, subnet->p->num_segment);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
                    subnet->p->bdy_pt_x   = (int **) malloc(subnet->p->num_segment*sizeof(int *));
                    subnet->p->bdy_pt_y   = (int **) malloc(subnet->p->num_segment*sizeof(int *));

                    segment_idx = 0;
                    state = STATE_SUBNET_SEGMENT;
                    break;
                case STATE_SUBNET_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SUBNET_NUM_BDY_PT;
                    break;
                case STATE_SUBNET_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        subnet->p->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (subnet->p->num_bdy_pt[segment_idx] < 3) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "subnet->num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, subnet->p->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    subnet->p->bdy_pt_x[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);
                    subnet->p->bdy_pt_y[segment_idx] = IVECTOR(subnet->p->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_SUBNET_BDY_PT;
                    break;
                case STATE_SUBNET_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, X = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(msg, "ERROR: Invalid geometry file \"%s(%d)\"\n"
                                             "SUBNET %d SEGMENT %d PT %d, Y = %15.13f is not an integer multiple of"
                                             "resolution=%9.7f\n",filename, linenum, subnet_idx, segment_idx, bdy_pt_idx, tmpd, resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= subnet->p->num_bdy_pt[segment_idx]-1) {
                        state = STATE_SUBNET_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= subnet->p->num_segment-1) {
                            state = STATE_SUBNET_SEGMENT;
                        } else {
                            subnet_idx++;
                            if (subnet_idx <= num_subnet[tt_idx]-1) {
                                state = STATE_SUBNET;
                            } else {
                                tt_idx++;
                                if (tt_idx <= num_traffic_type-1) {
                                    state = STATE_NUM_SUBNET;
                                } else {
                                    state = STATE_NUM_ANTENNA_TYPE;
                                }
                            }
                        }
                    }
                    break;
                case STATE_NUM_ANTENNA_TYPE:
                    if (strcmp(str1, "NUM_ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_antenna_type < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_antenna_type = %d must be >= 1\n", filename, linenum, num_antenna_type);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));

                    ant_idx = 0;
                    state = STATE_ANTENNA_TYPE;
                    break;
                case STATE_ANTENNA_TYPE:
                    sprintf(str, "ANTENNA_TYPE_%d:", ant_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    int antenna_type;
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "OMNI")==0) {
                            antenna_type = CConst::antennaOmni;
                        } else if (strcmp(str1, "LUT_H")==0) {
                            antenna_type = CConst::antennaLUT_H;
                        } else if (strcmp(str1, "LUT_V")==0) {
                            antenna_type = CConst::antennaLUT_V;
                        } else if (strcmp(str1, "LUT")==0) {
                            antenna_type = CConst::antennaLUT;
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "INVALID \"ANTENNA_TYPE_%d:\" specified: %s\n", filename, linenum,ant_idx,str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE_%d:\" specified for %s\n", filename, linenum,ant_idx,str);
                        PRMSG(stdout, msg);
                        if (ant_idx==0) {
                            free(antenna_type_list);
                        }
                        num_antenna_type = ant_idx;
                        error_state = 1;
                        return;
                    }


                    if (antenna_type == CConst::antennaOmni) {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type, "OMNI");
                    } else {
                        antenna_type_list[ant_idx] = new AntennaClass(antenna_type);
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (!(antenna_type_list[ant_idx]->readFile(antenna_filepath, str1))) {
                                num_antenna_type = ant_idx+1;
                                error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "No Antenna file specified\n", filename, linenum);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    }

                    ant_idx++;
                    state = (ant_idx <= num_antenna_type-1 ? STATE_ANTENNA_TYPE : STATE_NUM_PROP_MODEL);

                    break;
                case STATE_NUM_PROP_MODEL:
                    if (strcmp(str1, "NUM_PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (num_prop_model < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_prop_model = %d must be >= 1\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
                    pm_idx = 0;
                    if (pm_idx == num_prop_model) {
                        state = STATE_NUM_TRAFFIC;
                    } else {
                        state = STATE_PROP_MODEL;
                    }
                    break;
                case STATE_PROP_MODEL:
                    sprintf(str, "PROP_MODEL_%d:", pm_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "");
                    p_prop_model_strid = (char *) NULL;
                    if (str1) {
                        chptr = str1;
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            chptr++;
                            p_prop_model_strid = chptr;
                        }
                        while( (*chptr != '\"') && (*chptr) ) { chptr++; }
                        if (*chptr) {
                            *chptr = (char) NULL;
                        } else {
                            p_prop_model_strid = (char *) NULL;
                        }
                    }

                    if (p_prop_model_strid) {
                        prop_model_strid = strdup(p_prop_model_strid);
                    } else {
                        sprintf(str, "UNNAMED_PROP_MODEL_%d", num_unnamed_prop_model);
                        prop_model_strid = strdup(str);
                        num_unnamed_prop_model++;

                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" PROP_MODEL_%d has no name, using name \"%s\"\n",
                            filename, linenum, pm_idx, str);
                        PRMSG(stdout, msg); warning_state = 1;
                    }

                    state = STATE_PROP_MODEL_TYPE;
                    break;
                case STATE_PROP_MODEL_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "EXPONENTIAL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "PW_LINEAR")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "TERRAIN")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_WITH_THETA")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentWithThetaPropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "SEGMENT_ANGLE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new SegmentAnglePropModelClass(map_clutter, prop_model_strid);
                        } else if (strcmp(str1, "CLUTTER")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelFullClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERSYMFULL")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterSymFullPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPO")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoPropModelClass(prop_model_strid);
                        } else if (strcmp(str1, "CLUTTERWTEXPOSLOPE")==0) {
                            prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(prop_model_strid);
                        } else {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "Unrecognized propagation model type: \"%s\"\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        if (prop_model_strid) { free(prop_model_strid); }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No propagation model type specified.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_PROP_MODEL_PARAM;
                    param_idx = 0;

                    break;
                case STATE_PROP_MODEL_PARAM:
                    prop_model_list[pm_idx]->get_prop_model_param_ptr(param_idx, str, type, iptr, dptr);

                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (type == CConst::NumericDouble) {
                            *dptr = atof(str1);
                        } else if (type == CConst::NumericInt) {
                            *iptr = atoi(str1);
                        } else {
                            CORE_DUMP;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Propagation model parameter value missing.\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    num_prop_param = prop_model_list[pm_idx]->comp_num_prop_param();

                    param_idx++;
                    if (param_idx == num_prop_param) {
                        pm_idx++;
                        if (pm_idx == num_prop_model) {
                            state = STATE_NUM_TRAFFIC;
                        } else {
                            state = STATE_PROP_MODEL;
                        }
                    }
                    break;
                case STATE_NUM_TRAFFIC:
                    if (strcmp(str1, "NUM_TRAFFIC:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_TRAFFIC:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::num_traffic = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic < 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_traffic = %d must be >= 0\n", filename, linenum, num_prop_model);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (SectorClass::num_traffic) {
                        SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
                    } else {
                        SectorClass::traffic_type_idx_list = (int *) NULL;
                    }

                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }
                    break;
                case STATE_TRAFFIC_TYPE_IDX:
                    sprintf(str, "TRAFFIC_TYPE_IDX_%d:", tti_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        SectorClass::traffic_type_idx_list[tti_idx] = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_TRAFFIC:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_NUM_CELL;
                    } else {
                        state = STATE_TRAFFIC_TYPE_IDX;
                    }

                    break;
                case STATE_NUM_CELL:
                    if (strcmp(str1, "NUM_CELL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_CELL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_cell = atoi(str1);
#if DEMO
                        if (num_cell > 22) {
                            sprintf(msg, "ERROR: geometry file \"%s(%d)\" contains %s CELLS\n"
                                         "DEMO version allows a maximum of 22 CELLS\n", filename);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
#endif
                    }

#if 0
                    if (num_cell == 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "num_cell = %d must be > 0\n", filename, linenum, num_cell);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    } else {
                        BITWIDTH(bit_cell, num_cell-1);
                    }
#endif
                    bit_cell = -1;

                    if (num_cell) {
                        cell_list = (CellClass **) malloc((num_cell)*sizeof(CellClass *));
                    } else {
                        cell_list = (CellClass **) NULL;
                    }
                    for (i=0; i<=num_cell-1; i++) {
                        cell_list[i] = (CellClass *) new WLANCellClass();
                    }
                    cell_idx = 0;

                    if (num_cell) {
                        state = STATE_CELL;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_CELL:
                    sprintf(str, "CELL_%d:", cell_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    cell = cell_list[cell_idx];

                    str1 = strtok(NULL, "");
                    cell->strid = get_strid(str1);

                    state = STATE_POSN;
                    break;
                case STATE_POSN:
                    if (strcmp(str1, "POSN:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"POSN:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_startx, &cell->posn_x)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN x is not an integer multiple of "
                                             "resolution=%15.10f\n",filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, resolution, system_starty, &cell->posn_y)) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "POSN y is not an integer multiple of "
                                             "resolution=%15.10f\n", filename, linenum,resolution);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"POSN:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_CELL_BM_IDX;
                    break;
                case STATE_CELL_BM_IDX:
                    if (strcmp(str1, "BM_IDX:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"BM_IDX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->bm_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (cell->bm_idx < 0) || (cell->bm_idx > cell->num_bm-1) ) {
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" has invalid BM_IDX = %d, using value 0\n",
                                         filename, linenum, cell->bm_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                        cell->bm_idx = 0;
                    }

                    state = STATE_CELL_COLOR;
                    break;
                case STATE_CELL_COLOR:
                    if (strcmp(str1, "COLOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COLOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->color = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"COLOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        cell->num_sector = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"NUM_SECTOR:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if (cell->num_sector < 1) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "cell->num_sector = %d must be >= 1\n", filename, linenum, cell->num_sector);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        cell->sector_list[sector_idx] = (SectorClass *) new WLANSectorClass(cell);
                    }
                    sector_idx = 0;
                    state = STATE_SECTOR;
                    break;
                case STATE_SECTOR:
                    sprintf(str, "SECTOR_%d:", sector_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector = (WLANSectorClass *) cell->sector_list[sector_idx];
                    state = STATE_SECTOR_COMMENT;
                    break;
                case STATE_SECTOR_COMMENT:
                    if (strcmp(str1, "COMMENT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"COMMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        sector->comment = strdup(str1);
                    } else {
                        sector->comment = (char *) NULL;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
#if 0
                case STATE_SECTOR_CSID:
                    if (strcmp(str1, "CSID:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CSID:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, "\n");
                    if (str1) {
                        if ( strlen(str1) < 2*WLANSectorClass::csid_byte_length ) {
                            sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                             "CSID: %s has improper length\n", filename, linenum, str1);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        sector->csid_hex = (unsigned char *) malloc(WLANSectorClass::csid_byte_length);
                        if (!hexstr_to_hex(sector->csid_hex, str1, WLANSectorClass::csid_byte_length)) {
                            error_state = 1;
                            return;
                        }
                    } else {
                        sector->csid_hex = (unsigned char *) NULL;
                        sprintf(msg, "WARNING: geometry file \"%s(%d)\" CELL %d SECTOR %d has no CSID\n",
                            filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg); warning_state = 1;
                    }
                    state = STATE_SECTOR_CS_NUMBER;
                    break;
                case STATE_SECTOR_CS_NUMBER:
                    if (strcmp(str1, "CS_NUMBER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CS_NUMBER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->gw_csc_cs = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CS_NUMBER:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx = 0;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
#endif
                case STATE_SECTOR_MEAS_CTR:
                    sprintf(str, "MEAS_CTR_%d:", tti_idx);
                    tt_idx = SectorClass::traffic_type_idx_list[tti_idx];
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->meas_ctr_list[tti_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"%s\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, str, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    tti_idx++;
                    if (tti_idx == SectorClass::num_traffic) {
                        state = STATE_ANGLE_DEG;
                    } else {
                        state = STATE_SECTOR_MEAS_CTR;
                    }
                    break;
                case STATE_ANGLE_DEG:
                    if (strcmp(str1, "ANGLE_DEG:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANGLE_DEG:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_angle_rad = atof(str1)*PI/180.0;
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANGLE_DEG:\" specified for (CELL, SECTOR) = (%d, %d)\n",
                                         filename, linenum, cell_idx, sector_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_ANTENNA_TYPE;
                    break;
                case STATE_SEC_ANTENNA_TYPE:
                    if (strcmp(str1, "ANTENNA_TYPE:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_type = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_TYPE:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->antenna_type < 0) || (sector->antenna_type > num_antenna_type-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->antenna_type = %d must be between 0 and %d\n", filename, linenum, sector->antenna_type, num_antenna_type-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ANTENNA_HEIGHT;
                    break;
                case STATE_ANTENNA_HEIGHT:
                    if (strcmp(str1, "ANTENNA_HEIGHT:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"ANTENNA_HEIGHT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->antenna_height = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"ANTENNA_HEIGHT:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_PROP_MODEL;
                    break;
                case STATE_SEC_PROP_MODEL:
                    if (strcmp(str1, "PROP_MODEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"PROP_MODEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->prop_model = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"PROP_MODEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->prop_model < -1) || (sector->prop_model > num_prop_model-1) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->prop_model = %d must be between 0 and %d\n",
                                         filename, linenum, sector->prop_model, num_prop_model-1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_SEC_CHANNEL;
                    break;
                case STATE_SEC_CHANNEL:
                    if (strcmp(str1, "CHANNEL:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"CHANNEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->chan_idx = atoi(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"CHANEL:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    if ( (sector->chan_idx < 1) || (sector->chan_idx > 11) ) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "sector->chan_idx = %d must be between 1 and 11\n",
                                         filename, linenum, sector->chan_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_TX_POWER;
                    break;
                case STATE_TX_POWER:
                    if (strcmp(str1, "TX_POWER:") != 0) {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "Expecting \"TX_POWER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sector->tx_pwr = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                         "No \"TX_POWER:\" specified\n", filename, linenum);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    sector_idx++;
                    if (sector_idx <= cell->num_sector-1) {
                        state = STATE_SECTOR;
                    } else {
                        cell_idx++;  
                        if (cell_idx <= num_cell-1) {
                            state = STATE_CELL;
                        } else {
                            state = STATE_DONE;
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR: invalid geometry file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_geometry                                                         ****/
/**** Display geometry information.  This is useful to verify that this information    ****/
/**** was correctly read in from the geometry file.                                    ****/
/**** FORMAT OPTIONS:                                                                  ****/
/**** fmt = 0: Use geometry file format                                                ****/
/**** fmt = 1: Use format for plotting data with visual basic                          ****/
/**** 5/29/03: fmt=1 option no longer supported.                                       ****/
/**** 9/14/05: Removed fmt parameter.                                                  ****/
/******************************************************************************************/
void WLANNetworkClass::print_geometry(char *filename)
{
#if (DEMO == 0)
    int n, bdy_pt_idx, cell_idx, sector_idx;
    int ant_idx, subnet_idx, segment_idx, pm_idx, tt_idx, tti_idx;
    int antenna_type;
    CellClass *cell;
    WLANSectorClass *sector;
    SubnetClass *subnet;
    AntennaClass *antenna;
    WLANTrafficTypeClass *traffic_type;
    char *chptr;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (!system_bdy) {
        sprintf( msg, "ERROR: Geometry not defined\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "# Geometry Data\n");
    chptr += sprintf(chptr, "TECHNOLOGY: WLAN\n");
    chptr += sprintf(chptr, "FORMAT: %s\n", WLAN_GEOMETRY_FORMAT);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
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
    chptr += sprintf(chptr, "RESOLUTION: %9.7f\n", resolution);
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_BDY_PT: %d\n", system_bdy->num_bdy_pt[0]);
    PRMSG(fp, msg);

    for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "BDY_PT_%d: %15.13f %15.13f\n",
            bdy_pt_idx, idx_to_x(system_bdy->bdy_pt_x[0][bdy_pt_idx]), idx_to_y(system_bdy->bdy_pt_y[0][bdy_pt_idx]));
        PRMSG(fp, msg);
    }
    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "NUM_TRAFFIC_TYPE: %d\n", num_traffic_type);
    PRMSG(fp, msg);
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        traffic_type = (WLANTrafficTypeClass *) traffic_type_list[tt_idx];
        chptr = msg;
        chptr += sprintf(chptr, "TRAFFIC_TYPE_%d:", tt_idx);
        if (traffic_type->strid) {
            chptr += sprintf(chptr, " \"%s\"", traffic_type->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    COLOR: %d\n", traffic_type->color);
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: EXPONENTIAL:%15.13f\n", traffic_type->mean_time);
        } else if (traffic_type->duration_dist == CConst::UnifDist) {
            chptr += sprintf(chptr, "    DURATION_DIST: UNIFORM:%15.13f:%15.13f\n",
                traffic_type->min_time, traffic_type->max_time);
        } else {
            sprintf(msg, "ERROR: Invalid duration distribution\n");
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
        chptr += sprintf(chptr, "    BIT_PER_PKT: %d\n",                 traffic_type->bit_per_pkt);
        chptr += sprintf(chptr, "    USER_DATA_RATE: %15.13f\n",         traffic_type->user_data_rate);
        chptr += sprintf(chptr, "    BASIC_RATE: %15.13f\n",             traffic_type->basic_rate);
        chptr += sprintf(chptr, "    PHY_RATE: %15.13f\n",               traffic_type->phy_rate);
        chptr += sprintf(chptr, "    HD_RATE: %15.13f\n",                traffic_type->hd_rate);
        chptr += sprintf(chptr, "    MEAN_SESSION_DURATION: %15.13f\n",  traffic_type->mean_session_duration);
        chptr += sprintf(chptr, "    DROP_TOTAL_PKT: %d\n",              traffic_type->drop_total_pkt);
        chptr += sprintf(chptr, "    DROP_ERROR_PKT: %d\n",              traffic_type->drop_error_pkt);
        chptr += sprintf(chptr, "    MEAN_SEGMENT: %15.13f\n",           traffic_type->mean_segment);

        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "NUM_SUBNET_%d: %d\n", tt_idx, num_subnet[tt_idx]);
        PRMSG(fp, msg);
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];

            chptr = msg;
            chptr += sprintf(chptr, "SUBNET_%d:", subnet_idx);
            if (subnet->strid) {
                chptr += sprintf(chptr, " \"%s\"", subnet->strid);
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "# Contained cells:");
            n = 0;
            for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                cell = cell_list[cell_idx];
                if (subnet->p->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    chptr += sprintf(chptr, " %d", cell_idx);
                    n++;
                }
            }
            if (n == 0) { 
                chptr += sprintf(chptr, " NONE");
            }
            chptr += sprintf(chptr, "\n");

            chptr += sprintf(chptr, "    ARRIVAL_RATE: %9.7f\n", subnet->arrival_rate);
            chptr += sprintf(chptr, "    COLOR: %d\n", subnet->color);
            chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", subnet->p->num_segment);
            PRMSG(fp, msg);
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
                chptr = msg;
                chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
                chptr += sprintf(chptr, "        NUM_BDY_PT: %d\n", subnet->p->num_bdy_pt[segment_idx]);
                PRMSG(fp, msg);
                for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                    chptr = msg;
                    chptr += sprintf(chptr, "            BDY_PT_%d: %15.13f %15.13f\n", bdy_pt_idx,
                        idx_to_x(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]),
                        idx_to_y(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]));
                    PRMSG(fp, msg);
                }
            }
        }
        chptr = msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);
    }

    sprintf(msg, "NUM_ANTENNA_TYPE: %d\n", num_antenna_type);
    PRMSG(fp, msg);
    for (ant_idx=0; ant_idx<=num_antenna_type-1; ant_idx++) {
        antenna = antenna_type_list[ant_idx];
        antenna_type = antenna->get_type();
        if (antenna_type == CConst::antennaOmni) {
            sprintf(msg, "ANTENNA_TYPE_%d: OMNI\n", ant_idx);
        } else {
            sprintf(msg, "ANTENNA_TYPE_%d: %s %s\n", ant_idx,
                (antenna_type == CConst::antennaLUT_H) ? "LUT_H" :
                (antenna_type == CConst::antennaLUT_V) ? "LUT_V" : "LUT",
                antenna->get_filename());
        }
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);

    sprintf(msg, "NUM_PROP_MODEL: %d\n", num_prop_model);
    PRMSG(fp, msg);
    for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "PROP_MODEL_%d:", pm_idx);
        if (prop_model_list[pm_idx]->get_strid()) {
            chptr += sprintf(chptr, " \"%s\"", prop_model_list[pm_idx]->get_strid());
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        prop_model_list[pm_idx]->print_params(fp, msg, CConst::geometryFileType);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_TRAFFIC: %d\n", SectorClass::num_traffic);
    PRMSG(fp, msg);

    for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "    TRAFFIC_TYPE_IDX_%d: %d\n",   tti_idx, SectorClass::traffic_type_idx_list[tti_idx]);
        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "NUM_CELL: %d\n", num_cell);
    PRMSG(fp, msg);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];

        chptr = msg;
        chptr += sprintf(chptr, "CELL_%d:", cell_idx);
        if (cell->strid) {
            chptr += sprintf(chptr, " \"%s\"", cell->strid);
        }
        chptr += sprintf(chptr, "\n");
        chptr += sprintf(chptr, "    POSN: %9.7f %9.7f\n", idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
        chptr += sprintf(chptr, "    BM_IDX: %d\n", cell->bm_idx);
        chptr += sprintf(chptr, "    COLOR: %d\n", cell->color);
        PRMSG(fp, msg);

        chptr = msg;
        chptr += sprintf(chptr, "    NUM_SECTOR: %d\n", cell->num_sector);
        PRMSG(fp, msg);

        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (WLANSectorClass *) cell->sector_list[sector_idx];
            chptr = msg;
            chptr += sprintf(chptr, "    SECTOR_%d:\n", sector_idx);
            chptr += sprintf(chptr, "        COMMENT: %s\n", (sector->comment ? sector->comment : ""));
#if 0
            chptr += sprintf(chptr, "        CSID:");
            if (sector->csid_hex) {
                chptr += sprintf(chptr, " ");
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
            }
            chptr += sprintf(chptr, "\n");
            chptr += sprintf(chptr, "        CS_NUMBER: %d\n", sector->gw_csc_cs);
#endif

            for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
                chptr += sprintf(chptr, "        MEAS_CTR_%d: %.7f\n",   tti_idx, sector->meas_ctr_list[tti_idx]);
            }
            chptr += sprintf(chptr, "        ANGLE_DEG: %.7f\n",      sector->antenna_angle_rad*180.0/PI);
            chptr += sprintf(chptr, "        ANTENNA_TYPE: %d\n",     sector->antenna_type);
            chptr += sprintf(chptr, "        ANTENNA_HEIGHT: %.7f\n", sector->antenna_height);
            chptr += sprintf(chptr, "        PROP_MODEL: %d\n",       sector->prop_model);
            chptr += sprintf(chptr, "        CHANNEL: %d\n",          sector->chan_idx);
            chptr += sprintf(chptr, "        TX_POWER: %.7f\n",       sector->tx_pwr);
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

#endif

    return;
}
/******************************************************************************************/
