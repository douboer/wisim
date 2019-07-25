/******************************************************************************************/
/**** PROGRAM: coverage.cpp                                                            ****/
/**** Michael Mandell 1/20/04                                                          ****/
/******************************************************************************************/
/**** Functions for reading/writing coverage analysis files.                           ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "antenna.h"
#include "wisim.h"
#include "coverage.h"
#include "cconst.h"
#include "hot_color.h"
#include "polygon.h"
#include "list.h"
#include "pref.h"
#include "prop_model.h"

#if HAS_GUI
#include "progress_slot.h"
extern int use_gui;
#endif


#define INCLUDE_COVERAGE_DB 1

#define COVERAGE_FORMAT "1.2"

/******************************************************************************************/
/**** FUNCTION: CoverageClass::CoverageClass                                           ****/
/******************************************************************************************/
CoverageClass::CoverageClass()
{
    strid             = (char *) NULL;
    level_list        = (double *) NULL;
    color_list        = (int *) NULL;
    polygon_list      = (PolygonClass **) NULL;
    has_threshold     = 0;

    clipped_region    = 0;
    dmax              = 0.0;
    cell_list         = (ListClass<int> *) NULL;
    scan_list         = (ListClass<int> *) NULL;

    eqv_num_sector    = 0.0;
    use_gpm           = 0;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::CoverageClass                                           ****/
/******************************************************************************************/
CoverageClass::CoverageClass(NetworkClass *np, char *param_strid, int param_type)
{
    int i, n, cell_idx, sector_idx, scan_idx;
    CellClass *cell;
    SectorClass *sector;

    strid             = strdup(param_strid);
    type              = param_type;
    has_threshold     = 0;
    level_list        = (double *) NULL;
    color_list        = (int *) NULL;
    polygon_list      = (PolygonClass **) NULL;
    init_sample_res   = 16;
    scan_fractional_area = 0.995;
    scan_list = new ListClass<int>(1);

    switch (type) {
        case CConst::layerCoverage:
            scan_list->append(0);
            threshold     = exp(26.0*log(10.0)/10.0);
            use_gpm = 0;
            break;
        case CConst::levelCoverage:
            threshold     = -1.0;
            n = 2;
            level_list = (double *) realloc((void *) level_list, (n-1)*sizeof(double));
            for (i=0; i<=n-1; i++) {
                scan_list->append(i);
                if ( (i >= 1) ) {
                    level_list[i-1] = exp(80.0*log(10.0)/10.0);
                }
            }
            use_gpm = 0;
            break;
        case CConst::sirLayerCoverage:
            scan_list->append(0);
            threshold     = exp(17.0*log(10.0)/10.0);
            use_gpm = 0;
            break;
        case CConst::pagingAreaCoverage:
            for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                    sector = cell->sector_list[sector_idx];
                    scan_idx = sector->get_ival(CConst::SectorPagingArea);
                    if ( scan_idx != -1 ) {
                        scan_list->ins_elem(scan_idx, 0);
                    }
                }
            }
            scan_list->sort();
            use_gpm = 1;
            break;
        default:
            CORE_DUMP;
            break;
    }
    color_list = (int *) realloc((void *) color_list, scan_list->getSize()*sizeof(int));
    for (i=0; i<=scan_list->getSize()-1; i++) {
        // color_list[i] = np->default_color_list[i % np->num_default_color];
        //color_list[i] = np->hot_color->get_color(i, scan_list->getSize());
        color_list[i] = np->hot_color->cal_color_value(i, scan_list->getSize());
    }

    clipped_region    = 0;
    dmax              = 0.0;
    cell_list         = (ListClass<int> *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::~CoverageClass                                          ****/
/******************************************************************************************/
CoverageClass::~CoverageClass()
{
    int p_idx;

    if (polygon_list) {
        for (p_idx=0; p_idx<=scan_list->getSize()-1; p_idx++) {
            if (polygon_list[p_idx]) {
                delete(polygon_list[p_idx]);
            }
        }
    }

    if (polygon_list) {
        free(polygon_list);
    }

    if (scan_list) {
        delete scan_list;
    }

    free(strid);

    if (level_list) {
        free(level_list);
    }

    if (color_list) {
        free(color_list);
    }

    if (cell_list) {
        delete cell_list;
    }
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::read_coverage                                           ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void CoverageClass::read_coverage(NetworkClass *np, char *filename, char *force_fmt)
{
#if (DEMO == 0)
    int linenum;
    char *str1, *line;
    char *format_str = (char *) NULL;
    FILE *fp;

    line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(np->msg, "ERROR: cannot open coverage file %s\n", filename);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

    strcpy(np->msg, "Reading coverage file: ");
    strcat(np->msg, filename);
    PRMSG(stdout, np->msg);

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
                        sprintf(np->msg, "WARNING: coverage file \"%s\"\n"
                                         "Assuming format 1.1\n", filename);
                        PRMSG(stdout, np->msg); np->warning_state = 1;

                        fclose(fp);
                        if ( !(fp = fopen(filename, "rb")) ) {
                            sprintf(np->msg, "ERROR: cannot open coverage file %s\n", filename);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                        format_str = strdup("1.1");
                        state = STATE_READ_VERSION;
#else
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"TECHNOLOGY:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
#endif
                    } else {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (strcmp(str1, np->technology_str()) != 0) {
                                sprintf(np->msg, "ERROR: coverage file \"%s(%d)\"\n"
                                                 "\"TECHNOLOGY:\" specification \"%s\" not \"%s\"\n",
                                                 filename, linenum, str1, np->technology_str());
                                PRMSG(stdout, np->msg);
                                np->error_state = 1;
                                return;
                            }
                        } else {
                            sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                             "No \"TECHNOLOGY:\" specified\n", filename, linenum);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                        state = STATE_FORMAT;
                    }
                    break;
                case STATE_FORMAT:
                    if (strcmp(str1, "FORMAT:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"FORMAT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    format_str = strdup(str1);
                    state = STATE_READ_VERSION;
                    break;
                default:
                    sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, np->msg);
                    np->error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_READ_VERSION) {
        sprintf(np->msg, "ERROR: invalid coverage file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }
    } else {
        format_str = strdup(force_fmt);
    }

    if (strcmp(format_str,"1.1")==0) {
        read_coverage_1_1(np, fp, line, filename, linenum);
    } else if (strcmp(format_str,"1.2")==0) {
        read_coverage_1_2(np, fp, line, filename, linenum);
    } else {
        sprintf(np->msg, "ERROR: coverage file \"%s\" has invalid format \"%s\"\n", filename, format_str);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

    if (np->error_state == 1) { return; }

    free(line);
    free(format_str);

    fclose(fp);

#endif
    return;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::read_coverage_1_1                                       ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void CoverageClass::read_coverage_1_1(NetworkClass *np, FILE *fp, char *line, char *filename, int linenum)
{
#if (DEMO == 0)
    int level_idx                = -1;
    int polygon_idx              = -1;
    int segment_idx              = -1;
    int bdy_pt_idx               = -1;
    int scan_type_idx            = -1;
    int cell_idx, sector_idx;
    int ns;
    int n;
    int num_scan_type;
    int num_cvg_sector;
    double tmpd;
    char *str1, str[1000];

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_TYPE,
        STATE_COVERAGE_NAME,
        STATE_INIT_SAMPLE_RES,
        STATE_SCAN_FRACTIONAL_AREA,
        STATE_CLIPPED_REGION,
        STATE_DMAX,
        STATE_NUM_SECTOR,
        STATE_CSID,
        STATE_THRESHOLD,
        STATE_NUM_LEVEL,
        STATE_LEVEL,
        STATE_NUM_SCAN_TYPE,
        STATE_SCAN_TYPE,
        STATE_POLYGON,
        STATE_POLYGON_NUM_SEGMENT,
        STATE_POLYGON_SEGMENT,
        STATE_POLYGON_NUM_BDY_PT,
        STATE_POLYGON_BDY_PT,
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
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            np->coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            np->coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            np->utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            np->utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            np->utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                np->utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                np->utm_north = 0;
                            } else {
                                sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, np->msg);
                                np->error_state = 1;
                                return;
                            }
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_TYPE;
                    break;
                case STATE_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if ( str1 ) {
                        if (strcmp(str1, "Layer")==0) {
                            type = CConst::layerCoverage;
                        } else if (strcmp(str1, "Level")==0) {
                            type = CConst::levelCoverage;
                        } else if (strcmp(str1, "SirLayer")==0) {
                            type = CConst::sirLayerCoverage;
                        } else if (strcmp(str1, "PagingArea")==0) {
                            type = CConst::pagingAreaCoverage;
                        } else {
                            sprintf(np->msg, "ERROR: Invalid TYPE = \"%s\"\n", str1);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    }
                    state = STATE_COVERAGE_NAME;
                    break;
                case STATE_COVERAGE_NAME:
                    if (strcmp(str1, "COVERAGE_NAME:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"COVERAGE_NAME:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        strid = strdup(str1);
                    } else {
                        sprintf(np->msg, "ERROR: coverage name is empty\n");
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_INIT_SAMPLE_RES;
                    break;
                case STATE_INIT_SAMPLE_RES:
                    if (strcmp(str1, "INIT_SAMPLE_RES:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"INIT_SAMPLE_RES:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    init_sample_res = atoi(str1);
                    state = STATE_SCAN_FRACTIONAL_AREA;
                    break;
                case STATE_SCAN_FRACTIONAL_AREA:
                    if (strcmp(str1, "SCAN_FRACTIONAL_AREA:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"SCAN_FRACTIONAL_AREA:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    scan_fractional_area = atof(str1);
                    state = STATE_CLIPPED_REGION;
                    break;
                case STATE_CLIPPED_REGION:
                    if (strcmp(str1, "CLIPPED_REGION:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"CLIPPED_REGION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    clipped_region = atoi(str1);
                    if (clipped_region) {
                        state = STATE_DMAX;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_DMAX:
                    if (strcmp(str1, "DMAX:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"DMAX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    dmax = atof(str1);
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    num_cvg_sector = atoi(str1);
                    if (num_cvg_sector < 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "num_sector = %d must be > 0\n", filename, linenum, num_cvg_sector);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    cell_list = new ListClass<int>(num_cvg_sector);

                    if (cell_list->getSize() < num_cvg_sector) {
                        state = STATE_CSID;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_CSID:
                    sprintf(str, "CSID_%d:", cell_list->getSize());
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    np->uid_to_sector(str1, cell_idx, sector_idx);
                    if (cell_idx != -1) {
                        cell_list->append(cell_idx);
                    }
                    if (cell_list->getSize() < num_cvg_sector) {
                        state = STATE_CSID;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_THRESHOLD:
                    if (strcmp(str1, "THRESHOLD:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"THRESHOLD:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if ( type == CConst::levelCoverage ) {
                        threshold = atof(str1);
                        state = STATE_NUM_LEVEL;
                    } else {
                        threshold = exp(log(10.0)*atof(str1)/10.0);
                        state = STATE_NUM_SCAN_TYPE;
                    }
                    break;
                case STATE_NUM_LEVEL:
                    if (strcmp(str1, "NUM_LEVEL:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_LEVEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    n = atoi(str1)+1;
                    level_list = DVECTOR(n-1);
                    level_idx = 0;
                    state = STATE_LEVEL;
                    break;
                case STATE_LEVEL:
                    sprintf(str, "LEVEL_%d:", level_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        level_list[level_idx] = exp(log(10.0)*atof(str1)/10.0);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"LEVEL_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    level_idx++;
                    if (level_idx <= n - 2 ) {
                        state = STATE_LEVEL;
                    } else {
                        state = STATE_NUM_SCAN_TYPE;
                    }
                    break;
                case STATE_NUM_SCAN_TYPE:
                    if (strcmp(str1, "NUM_SCAN_TYPE:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SCAN_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_scan_type = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_SCAN_TYPE:\" specified\n" , filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    if (num_scan_type <= 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\""
                                         "STATE_NUM_SCAN_TYPE = %d must be > 0\n", filename, linenum, num_scan_type);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    scan_list = new ListClass<int>(num_scan_type);
                    color_list = IVECTOR(num_scan_type);
                    polygon_list = (PolygonClass **) malloc( num_scan_type*sizeof(PolygonClass *) );
                    scan_type_idx = 0;
                    polygon_idx = 0;
                    state = STATE_SCAN_TYPE;
                    break;
                case STATE_SCAN_TYPE:
                    sprintf(str, "SCAN_TYPE_%d:", scan_type_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        scan_list->append(atoi(str1));
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"SCAN_TYPE_%d:\" specified\n" , filename, linenum, scan_type_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        color_list[scan_type_idx] = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"SCAN_TYPE_%d:\" specified\n" , filename, linenum, scan_type_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    scan_type_idx++;
                    if (scan_type_idx <= num_scan_type - 1 ) {
                        state = STATE_SCAN_TYPE;
                    } else {
                        state = STATE_POLYGON;
                    }
                    break;
                case STATE_POLYGON:
                    sprintf(str, "POLYGON_%d:", polygon_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    polygon_list[polygon_idx] = (PolygonClass *) NULL;
                    state = STATE_POLYGON_NUM_SEGMENT;
                    break;
                case STATE_POLYGON_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ns = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (ns > 0) {
                        polygon_list[polygon_idx] = new PolygonClass();
                        polygon_list[polygon_idx]->num_segment = ns;
                    } else if (ns < 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                     "polygon_list[polygon_idx]->num_segment = %d must be >= 1\n", filename, linenum, ns);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (ns) {
                        polygon_list[polygon_idx]->num_bdy_pt = IVECTOR(polygon_list[polygon_idx]->num_segment);
                        polygon_list[polygon_idx]->bdy_pt_x   = (int **) malloc(polygon_list[polygon_idx]->num_segment*sizeof(int *));
                        polygon_list[polygon_idx]->bdy_pt_y   = (int **) malloc(polygon_list[polygon_idx]->num_segment*sizeof(int *));

                        segment_idx = 0;
                        state = STATE_POLYGON_SEGMENT;
                    } else {
                        polygon_idx++;
                        if (polygon_idx <= num_scan_type-1) {
                            state = STATE_POLYGON;
                        } else {
                            state = STATE_DONE;
                        }
                    }
                    break;
                case STATE_POLYGON_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_POLYGON_NUM_BDY_PT;
                    break;
                case STATE_POLYGON_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        polygon_list[polygon_idx]->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (polygon_list[polygon_idx]->num_bdy_pt[segment_idx] < 3) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    polygon_list[polygon_idx]->bdy_pt_x[segment_idx] = IVECTOR(polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);
                    polygon_list[polygon_idx]->bdy_pt_y[segment_idx] = IVECTOR(polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_POLYGON_BDY_PT;
                    break;
                case STATE_POLYGON_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, np->resolution, np->system_startx, &(polygon_list[polygon_idx]->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                     "Boundary point x=%d, is not an integer multiple of"
					     "resolution=%15.10f\n",filename, linenum,bdy_pt_idx,np->resolution);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, np->resolution, np->system_starty, &(polygon_list[polygon_idx]->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                     "Boundary point y=%d, is not an integer multiple of"
					     "resolution=%15.10f\n",filename, linenum,bdy_pt_idx,np->resolution);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= polygon_list[polygon_idx]->num_bdy_pt[segment_idx]-1) {
                        state = STATE_POLYGON_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= polygon_list[polygon_idx]->num_segment-1) {
                            state = STATE_POLYGON_SEGMENT;
                        } else {
                            polygon_idx++;
                            if (polygon_idx <= num_scan_type-1) {
                                state = STATE_POLYGON;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
		                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, np->msg);
                    np->error_state = 1;
                    return;
                    break;
                default:
                    sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, np->msg);
                    np->error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(np->msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

#endif
    return;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::read_coverage_1_2                                       ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void CoverageClass::read_coverage_1_2(NetworkClass *np, FILE *fp, char *line, char *filename, int linenum)
{
#if (DEMO == 0)
    int level_idx                = -1;
    int polygon_idx              = -1;
    int segment_idx              = -1;
    int bdy_pt_idx               = -1;
    int scan_type_idx            = -1;
    int cell_idx, sector_idx;
    int ns;
    int n;
    int num_scan_type;
    int num_cvg_sector;
    double tmpd;
    char *str1, str[1000];

    enum state_enum {
        STATE_COORDINATE_SYSTEM,
        STATE_TYPE,
        STATE_COVERAGE_NAME,
        STATE_INIT_SAMPLE_RES,
        STATE_SCAN_FRACTIONAL_AREA,
        STATE_EQV_NUM_SECTOR,
        STATE_CLIPPED_REGION,
        STATE_DMAX,
        STATE_NUM_SECTOR,
        STATE_CSID,
        STATE_THRESHOLD,
        STATE_NUM_LEVEL,
        STATE_LEVEL,
        STATE_NUM_SCAN_TYPE,
        STATE_SCAN_TYPE,
        STATE_POLYGON,
        STATE_POLYGON_NUM_SEGMENT,
        STATE_POLYGON_SEGMENT,
        STATE_POLYGON_NUM_BDY_PT,
        STATE_POLYGON_BDY_PT,
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
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        if (strcmp(str1, "GENERIC")==0) {
                            np->coordinate_system = CConst::CoordGeneric;
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            np->coordinate_system = CConst::CoordUTM;
                            str1 = strtok(str1+4, ":");
                            np->utm_equatorial_radius = atof(str1);
                            str1 = strtok(NULL, ":");
                            np->utm_eccentricity_sq = atof(str1);
                            str1 = strtok(NULL, ":");
                            np->utm_zone = atoi(str1);
                            str1 = strtok(NULL, ":");
                            if (strcmp(str1, "N")==0) {
                                np->utm_north = 1;
                            } else if (strcmp(str1, "S")==0) {
                                np->utm_north = 0;
                            } else {
                                sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                                 "Invalid \"COORDINATE_SYSTEM:\" specification\n", filename, linenum);
                                PRMSG(stdout, np->msg);
                                np->error_state = 1;
                                return;
                            }
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "No \"COORDINATE_SYSTEM:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_TYPE;
                    break;
                case STATE_TYPE:
                    if (strcmp(str1, "TYPE:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if ( str1 ) {
                        if (strcmp(str1, "Layer")==0) {
                            type = CConst::layerCoverage;
                        } else if (strcmp(str1, "Level")==0) {
                            type = CConst::levelCoverage;
                        } else if (strcmp(str1, "SirLayer")==0) {
                            type = CConst::sirLayerCoverage;
                        } else if (strcmp(str1, "PagingArea")==0) {
                            type = CConst::pagingAreaCoverage;
                        } else {
                            sprintf(np->msg, "ERROR: Invalid TYPE = \"%s\"\n", str1);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    }
                    state = STATE_COVERAGE_NAME;
                    break;
                case STATE_COVERAGE_NAME:
                    if (strcmp(str1, "COVERAGE_NAME:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"COVERAGE_NAME:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        strid = strdup(str1);
                    } else {
                        sprintf(np->msg, "ERROR: coverage name is empty\n");
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_INIT_SAMPLE_RES;
                    break;
                case STATE_INIT_SAMPLE_RES:
                    if (strcmp(str1, "INIT_SAMPLE_RES:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"INIT_SAMPLE_RES:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    init_sample_res = atoi(str1);
                    state = STATE_SCAN_FRACTIONAL_AREA;
                    break;
                case STATE_SCAN_FRACTIONAL_AREA:
                    if (strcmp(str1, "SCAN_FRACTIONAL_AREA:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"SCAN_FRACTIONAL_AREA:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    scan_fractional_area = atof(str1);
                    if ( type==CConst::layerCoverage ) {
                        state = STATE_EQV_NUM_SECTOR;
                    } else {
                        state = STATE_CLIPPED_REGION;
                    }
                    break;
                case STATE_EQV_NUM_SECTOR:
                    if (strcmp(str1, "EQV_NUM_SECTOR:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"EQV_NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    eqv_num_sector = atof(str1);
                    state = STATE_CLIPPED_REGION;
                    break;
                case STATE_CLIPPED_REGION:
                    if (strcmp(str1, "CLIPPED_REGION:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"CLIPPED_REGION:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    clipped_region = atoi(str1);
                    if (clipped_region) {
                        state = STATE_DMAX;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_DMAX:
                    if (strcmp(str1, "DMAX:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"DMAX:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    dmax = atof(str1);
                    state = STATE_NUM_SECTOR;
                    break;
                case STATE_NUM_SECTOR:
                    if (strcmp(str1, "NUM_SECTOR:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SECTOR:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    num_cvg_sector = atoi(str1);
                    if (num_cvg_sector < 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "num_sector = %d must be > 0\n", filename, linenum, num_cvg_sector);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    cell_list = new ListClass<int>(num_cvg_sector);

                    if (cell_list->getSize() < num_cvg_sector) {
                        state = STATE_CSID;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_CSID:
                    sprintf(str, "CSID_%d:", cell_list->getSize());
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    np->uid_to_sector(str1, cell_idx, sector_idx);
                    if (cell_idx != -1) {
                        cell_list->append(cell_idx);
                    }
                    if (cell_list->getSize() < num_cvg_sector) {
                        state = STATE_CSID;
                    } else {
                        if (type == CConst::levelCoverage) {
                            state = STATE_NUM_LEVEL;
                        } else {
                            state = STATE_THRESHOLD;
                        }
                    }
                    break;
                case STATE_THRESHOLD:
                    if (strcmp(str1, "THRESHOLD:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"THRESHOLD:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if ( type == CConst::levelCoverage ) {
                        threshold = atof(str1);
                        state = STATE_NUM_LEVEL;
                    } else {
                        threshold = exp(log(10.0)*atof(str1)/10.0);
                        state = STATE_NUM_SCAN_TYPE;
                    }
                    break;
                case STATE_NUM_LEVEL:
                    if (strcmp(str1, "NUM_LEVEL:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_LEVEL:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    n = atoi(str1)+1;
                    level_list = DVECTOR(n-1);
                    level_idx = 0;
                    state = STATE_LEVEL;
                    break;
                case STATE_LEVEL:
                    sprintf(str, "LEVEL_%d:", level_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        level_list[level_idx] = exp(log(10.0)*atof(str1)/10.0);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"LEVEL_%d:\" specified\n" , filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    level_idx++;
                    if (level_idx <= n - 2 ) {
                        state = STATE_LEVEL;
                    } else {
                        state = STATE_NUM_SCAN_TYPE;
                    }
                    break;
                case STATE_NUM_SCAN_TYPE:
                    if (strcmp(str1, "NUM_SCAN_TYPE:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SCAN_TYPE:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_scan_type = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_SCAN_TYPE:\" specified\n" , filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    if (num_scan_type <= 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\""
                                         "STATE_NUM_SCAN_TYPE = %d must be > 0\n", filename, linenum, num_scan_type);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    scan_list = new ListClass<int>(num_scan_type);
                    color_list = IVECTOR(num_scan_type);
                    polygon_list = (PolygonClass **) malloc( num_scan_type*sizeof(PolygonClass *) );
                    scan_type_idx = 0;
                    polygon_idx = 0;
                    state = STATE_SCAN_TYPE;
                    break;
                case STATE_SCAN_TYPE:
                    sprintf(str, "SCAN_TYPE_%d:", scan_type_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        scan_list->append(atoi(str1));
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"SCAN_TYPE_%d:\" specified\n" , filename, linenum, scan_type_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        color_list[scan_type_idx] = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"SCAN_TYPE_%d:\" specified\n" , filename, linenum, scan_type_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    scan_type_idx++;
                    if (scan_type_idx <= num_scan_type - 1 ) {
                        state = STATE_SCAN_TYPE;
                    } else {
                        state = STATE_POLYGON;
                    }
                    break;
                case STATE_POLYGON:
                    sprintf(str, "POLYGON_%d:", polygon_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    polygon_list[polygon_idx] = (PolygonClass *) NULL;
                    state = STATE_POLYGON_NUM_SEGMENT;
                    break;
                case STATE_POLYGON_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        ns = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_SEGMENT:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (ns > 0) {
                        polygon_list[polygon_idx] = new PolygonClass();
                        polygon_list[polygon_idx]->num_segment = ns;
                    } else if (ns < 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                     "polygon_list[polygon_idx]->num_segment = %d must be >= 1\n", filename, linenum, ns);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (ns) {
                        polygon_list[polygon_idx]->num_bdy_pt = IVECTOR(polygon_list[polygon_idx]->num_segment);
                        polygon_list[polygon_idx]->bdy_pt_x   = (int **) malloc(polygon_list[polygon_idx]->num_segment*sizeof(int *));
                        polygon_list[polygon_idx]->bdy_pt_y   = (int **) malloc(polygon_list[polygon_idx]->num_segment*sizeof(int *));

                        segment_idx = 0;
                        state = STATE_POLYGON_SEGMENT;
                    } else {
                        polygon_idx++;
                        if (polygon_idx <= num_scan_type-1) {
                            state = STATE_POLYGON;
                        } else {
                            state = STATE_DONE;
                        }
                    }
                    break;
                case STATE_POLYGON_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    state = STATE_POLYGON_NUM_BDY_PT;
                    break;
                case STATE_POLYGON_NUM_BDY_PT:
                    if (strcmp(str1, "NUM_BDY_PT:") != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "Expecting \"NUM_BDY_PT:\" NOT \"%s\"\n", filename, linenum,str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        polygon_list[polygon_idx]->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"NUM_BDY_PT:\" specified\n", filename, linenum);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    if (polygon_list[polygon_idx]->num_bdy_pt[segment_idx] < 3) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "num_bdy_pt = %d must be >= 3\n",
                                filename, linenum, polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    polygon_list[polygon_idx]->bdy_pt_x[segment_idx] = IVECTOR(polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);
                    polygon_list[polygon_idx]->bdy_pt_y[segment_idx] = IVECTOR(polygon_list[polygon_idx]->num_bdy_pt[segment_idx]);

                    bdy_pt_idx = 0;
                    state = STATE_POLYGON_BDY_PT;
                    break;
                case STATE_POLYGON_BDY_PT:
                    sprintf(str, "BDY_PT_%d:", bdy_pt_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, np->resolution, np->system_startx, &(polygon_list[polygon_idx]->bdy_pt_x[segment_idx][bdy_pt_idx]))) {
                            sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                     "Boundary point x=%d, is not an integer multiple of"
					     "resolution=%15.10f\n",filename, linenum,bdy_pt_idx,np->resolution);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tmpd = atof(str1);
                        if (!check_grid_val(tmpd, np->resolution, np->system_starty, &(polygon_list[polygon_idx]->bdy_pt_y[segment_idx][bdy_pt_idx]))) {
                            sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                     "Boundary point y=%d, is not an integer multiple of"
					     "resolution=%15.10f\n",filename, linenum,bdy_pt_idx,np->resolution);
                            PRMSG(stdout, np->msg);
                            np->error_state = 1;
                            return;
                        }
                    } else {
                        sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
			                 "No \"BDY_PT_%d:\" specified\n", filename, linenum,bdy_pt_idx);
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
                    }

                    bdy_pt_idx++;
                    if (bdy_pt_idx <= polygon_list[polygon_idx]->num_bdy_pt[segment_idx]-1) {
                        state = STATE_POLYGON_BDY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= polygon_list[polygon_idx]->num_segment-1) {
                            state = STATE_POLYGON_SEGMENT;
                        } else {
                            polygon_idx++;
                            if (polygon_idx <= num_scan_type-1) {
                                state = STATE_POLYGON;
                            } else {
                                state = STATE_DONE;
                            }
                        }
                    }
                    break;
                case STATE_DONE:
                    sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
		                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, np->msg);
                    np->error_state = 1;
                    return;
                    break;
                default:
                    sprintf(np->msg, "ERROR: invalid coverage file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, np->msg);
                    np->error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(np->msg, "ERROR: invalid geometry file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass:write_coverage                                           ****/
/******************************************************************************************/
void CoverageClass::write_coverage(NetworkClass *np, char *filename)
{

#if (DEMO == 0)
    int i;
    int bdy_pt_idx;
    int segment_idx, num_segment;
    char *chptr;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(np->msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "# Coverage Data\n");
    chptr += sprintf(chptr, "TECHNOLOGY: %s\n", np->technology_str());
    chptr += sprintf(chptr, "FORMAT: %s\n", COVERAGE_FORMAT);
    chptr += sprintf(chptr, "\n");
    if (np->coordinate_system == CConst::CoordGeneric) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: GENERIC\n");
    } else if (np->coordinate_system == CConst::CoordUTM) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: UTM:%.0f:%.9f:%d:%c\n", np->utm_equatorial_radius,
                         np->utm_eccentricity_sq,np->utm_zone, (np->utm_north ? 'N' : 'S'));
    }
    chptr += sprintf(chptr, "\n");
    switch ( type ) {
        case CConst::layerCoverage:      chptr += sprintf(chptr, "TYPE: Layer\n");      break;
        case CConst::levelCoverage:      chptr += sprintf(chptr, "TYPE: Level\n");      break;
        case CConst::sirLayerCoverage:   chptr += sprintf(chptr, "TYPE: SirLayer\n");   break;
        case CConst::pagingAreaCoverage: chptr += sprintf(chptr, "TYPE: PagingArea\n"); break;
        default: break;
    }
    chptr += sprintf(chptr, "COVERAGE_NAME:        %s\n", strid);
    chptr += sprintf(chptr, "INIT_SAMPLE_RES:      %d\n", init_sample_res);
    chptr += sprintf(chptr, "SCAN_FRACTIONAL_AREA: %f\n", scan_fractional_area);
    if ( type==CConst::layerCoverage ) {
        chptr += sprintf(chptr, "EQV_NUM_SECTOR: %15.3f\n", eqv_num_sector);
    }
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "CLIPPED_REGION: %d\n", clipped_region);
    PRMSG(fp, np->msg);
    if (clipped_region) {
        chptr = np->msg;
        chptr += sprintf(chptr, "DMAX: %15.13f\n", dmax);
        chptr += sprintf(chptr, "NUM_SECTOR: %d\n", cell_list->getSize());
        PRMSG(fp, np->msg);
        for (i=0; i<=cell_list->getSize()-1; i++) {
            chptr = np->msg;
            chptr += sprintf(chptr, "CSID_%d: ", i);
            chptr += np->sector_to_uid(chptr, (*cell_list)[i], 0);
            chptr += sprintf(chptr, "\n");
            PRMSG(fp, np->msg);
        }
    }
    chptr = np->msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    if ( type==CConst::levelCoverage ) {
        chptr += sprintf(chptr, "NUM_LEVEL: %d\n", scan_list->getSize() - 1 );
        for( i=0; i<scan_list->getSize()-1; i++ ) {
            chptr += sprintf(chptr, "LEVEL_%d: %f\n", i, 10.0*log(level_list[i])/log(10.0) );
        }
    } else {
        chptr += sprintf(chptr, "THRESHOLD: %f\n", 10.0*log(threshold)/log(10.0) );
    }
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "NUM_SCAN_TYPE: %d\n", scan_list->getSize() );
    for( i=0; i<=scan_list->getSize()-1; i++ ) {
        chptr += sprintf(chptr, "SCAN_TYPE_%d: %d %d\n", i, (*scan_list)[i], color_list[i]);
    }
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    for( i=0; i<=scan_list->getSize()-1; i++ ) {
        if (polygon_list[i]) {
            num_segment = polygon_list[i]->num_segment;
        } else {
            num_segment = 0;
        }

        chptr = np->msg;
        chptr += sprintf(chptr, "POLYGON_%d:\n", i);
        chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", num_segment);
        PRMSG(fp, np->msg);
        for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
            chptr = np->msg;
            chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
            chptr += sprintf(chptr, "        NUM_BDY_PT: %d\n", polygon_list[i]->num_bdy_pt[segment_idx]);
            PRMSG(fp, np->msg);
            for (bdy_pt_idx=0; bdy_pt_idx<=polygon_list[i]->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                chptr = np->msg;
                chptr += sprintf(chptr, "            BDY_PT_%d: %15.13f %15.13f\n", bdy_pt_idx,
                    np->idx_to_x(polygon_list[i]->bdy_pt_x[segment_idx][bdy_pt_idx]),
                    np->idx_to_y(polygon_list[i]->bdy_pt_y[segment_idx][bdy_pt_idx]));
                PRMSG(fp, np->msg);
            }
        }
    }
    chptr = np->msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    if ( fp != stdout ) {
        fclose(fp);
    }
#endif

    return;
}

/******************************************************************************************/
/**** FUNCTION: CoverageClass:set_num_digit                                            ****/
/******************************************************************************************/
void CoverageClass::set_num_digit(double pwr_offset)
{
    int scan_type_idx, scan_val, nd;
    double level;

    num_digit = 1;
    if (type == CConst::levelCoverage) {
        for (scan_type_idx=0; scan_type_idx<=scan_list->getSize()-2; scan_type_idx++) {
            scan_val = (*scan_list)[scan_type_idx];
            level = 10*log(level_list[scan_val  ])/log(10.0)-pwr_offset;

            if (fabs(level) < 1.0) {
                nd = 1;
            } else {
                nd = (int) floor( log(fabs(level))/log(10.0) + 1.0);
            }
            if (level < 0.0) { nd++; }
            if (nd > num_digit) { num_digit = nd; }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass:scan_label                                               ****/
/******************************************************************************************/
void CoverageClass::scan_label(NetworkClass *np, char *label, double pwr_offset, int scan_type_idx)
{
    int scan_val;
    char *chptr;
    static char fmt[10];

    sprintf(fmt, "%%%d.%df", num_digit+2, 1);

    chptr = label;

    scan_val = (*scan_list)[scan_type_idx];
    switch (type) {
        case CConst::layerCoverage:
        case CConst::sirLayerCoverage:
            sprintf(label, "L %s= %d", (scan_val == scan_list->getSize()-1 ? ">" : ""), scan_val);
            break;
        case CConst::levelCoverage:
            if (scan_val == 0) {
                chptr += sprintf(chptr, "Below ");
                chptr += sprintf(chptr, fmt, 10*log(level_list[0])/log(10.0)-pwr_offset);
            } else if (scan_val == scan_list->getSize()-1) {
                chptr += sprintf(chptr, "Above ");
                chptr += sprintf(chptr, fmt, 10*log(level_list[scan_val-1])/log(10.0)-pwr_offset);
            } else {
                chptr += sprintf(chptr, fmt, 10*log(level_list[scan_val-1])/log(10.0)-pwr_offset);
                chptr += sprintf(chptr, " to ");
                chptr += sprintf(chptr, fmt, 10*log(level_list[scan_val  ])/log(10.0)-pwr_offset);
            }
            break;
        case CConst::pagingAreaCoverage:
            chptr += sprintf(chptr, "%s", np->pa_to_str(scan_val));
            break;
        default:
            CORE_DUMP;
            break;
    }
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass:report_coverage                                          ****/
/******************************************************************************************/
void CoverageClass::report_coverage(NetworkClass *np, char *filename, int pwr_unit)
{
    int i, scan_type_idx;
    char *chptr, *label;
    double *area_list;
    double total_area, total_weighted_area, avg_radius, pwr_offset;
    char *pwr_str = CVECTOR(100);
    char *tmpstr  = CVECTOR(100);
    FILE *fp;

    if (pwr_unit == -1) {
        pwr_unit    = np->preferences->pwr_unit;
        strcpy(pwr_str, np->preferences->pwr_str_short);
        pwr_offset  = np->preferences->pwr_offset;
    } else {
        pwr_offset = get_pwr_unit_offset(pwr_unit);

        pwr_unit_to_name(pwr_unit, pwr_str);

        for (i=0; (pwr_str[i]) && (pwr_str[i] != '_'); i++) {}
        pwr_str[i] = (char) NULL;
    }

    area_list = DVECTOR(scan_list->getSize());
    label     = CVECTOR(100);

    for( i=0; i<=scan_list->getSize()-1; i++ ) {
        if (polygon_list[i]) {
            area_list[i] = polygon_list[i]->comp_bdy_area()*np->resolution*np->resolution;
        } else {
            area_list[i] = 0.0;
        }
    }

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(np->msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "#### Coverage Report                                                 ####\n");
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "COVERAGE_NAME:        %s\n", strid);
    switch ( type ) {
        case CConst::layerCoverage:      chptr += sprintf(chptr, "TYPE: Layer\n");      break;
        case CConst::levelCoverage:      chptr += sprintf(chptr, "TYPE: Level\n");      break;
        case CConst::sirLayerCoverage:   chptr += sprintf(chptr, "TYPE: SirLayer\n");   break;
        case CConst::pagingAreaCoverage: chptr += sprintf(chptr, "TYPE: PagingArea\n"); break;
        default: break;
    }
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    switch ( type ) {
        case CConst::layerCoverage:      chptr += sprintf(chptr, "%-20s", "NUMBER OF LAYERS");      break;
        case CConst::levelCoverage:
                                         sprintf(tmpstr, "SIGNAL LEVEL (%s)", pwr_str);
                                         chptr += sprintf(chptr, "%-20s", tmpstr);                  break;
        case CConst::sirLayerCoverage:   chptr += sprintf(chptr, "%-20s", "NUMBER OF INTERFERERS"); break;
        case CConst::pagingAreaCoverage: chptr += sprintf(chptr, "%-20s", "PAGING AREA");           break;
        default: break;
    }
    if (np->coordinate_system == CConst::CoordLONLAT) {
        chptr += sprintf(chptr, " %15s\n", "AREA");
    } else {
        chptr += sprintf(chptr, " %15s\n", "AREA (m^2)");
    }
    chptr += sprintf(chptr, "------------------------------------\n");
    chptr += sprintf(chptr, "------------------------------------\n");
    PRMSG(fp, np->msg);

    total_area = 0.0;
    total_weighted_area = 0.0;
    for (scan_type_idx=0; scan_type_idx<=scan_list->getSize()-1; scan_type_idx++) {
        scan_label(np, label, pwr_offset, scan_type_idx);

        total_area += area_list[scan_type_idx];
        total_weighted_area += scan_type_idx * area_list[scan_type_idx];

        chptr = np->msg;
        chptr += sprintf(chptr, "%-20s %15.3f\n", label, area_list[scan_type_idx]);
        PRMSG(fp, np->msg);
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "------------------------------------\n");
    chptr += sprintf(chptr, "%-20s %15.3f\n", "TOTAL", total_area);
    chptr += sprintf(chptr, "------------------------------------\n");
    PRMSG(fp, np->msg);

    if ( (type == CConst::layerCoverage) && (eqv_num_sector > 0.0) ) {
        avg_radius = sqrt(total_weighted_area / (PI * eqv_num_sector));
        chptr = np->msg;
        chptr += sprintf(chptr, "AVG CELL RADIUS: %15.3f\n", avg_radius);
        PRMSG(fp, np->msg);
    }

    if ( fp != stdout ) {
        fclose(fp);
    }

    free(area_list);
    free(label);
    free(pwr_str);
    free(tmpstr);

    return;
}
/******************************************************************************************/
/**** FUNCTION: CoverageClass::shift                                                   ****/
/******************************************************************************************/
void CoverageClass::shift(int x, int y)
{
    int idx;

    for (idx=0; idx<=scan_list->getSize()-1; idx++) {
        if (polygon_list[idx]) {
            polygon_list[idx]->translate(x, y);
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: run_coverage                                                           ****/
/******************************************************************************************/
void NetworkClass::run_coverage(int p_cvg_idx)
{
    int i, j, expand, has_gpm;
    int xmin, xmax, ymin, ymax;
    int cell_idx, sector_idx;
    char *chptr;
    CellClass *cell;
    SectorClass *sector;
    AntennaClass *antenna;
    CoverageClass *cvg = coverage_list[p_cvg_idx];
    PropModelClass *pm;

    if (cvg->polygon_list) {
        sprintf(msg, "ERROR: coverage already exists\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "Running Coverage Analysis:\n");
    chptr += sprintf(chptr, "Analysis type %s\n", (cvg->type == CConst::layerCoverage      ? "LAYER"       :
                                                   cvg->type == CConst::levelCoverage      ? "LEVEL"       :
                                                   cvg->type == CConst::sirLayerCoverage   ? "SIR_LAYER"   :
                                                   cvg->type == CConst::pagingAreaCoverage ? "PAGING_AREA" :
                                                                                             "UNKNOWN"     ) );
    chptr += sprintf(chptr, "Scan fractional area: %12.7f\n", cvg->scan_fractional_area);
    PRMSG(stdout, msg);

    GUI_FN(prog_bar = new ProgressSlot(0, "Progress Bar"));
    GUI_FN(prog_bar->setOffsetWeight(0, 75));

    /* compute the region of coverage plotting
     *   xmin-dmax,ymin-dmax,xmax+dmax,ymax+dmax
     */
    if (cvg->clipped_region) {
        printf("CELL_LIST_SIZE = %d\n", cvg->cell_list->getSize());
        for (i=0; i<=cvg->cell_list->getSize()-1; i++) {
            cell_idx = (*cvg->cell_list)[i];
            cell = cell_list[cell_idx];
            if (i==0) {
                xmin = cell->posn_x;
                xmax = cell->posn_x;
                ymin = cell->posn_y;
                ymax = cell->posn_y;
            } else {
                if (cell->posn_x > xmax) {
                    xmax = cell->posn_x;
                } else if (cell->posn_x < xmin) {
                    xmin = cell->posn_x;
                }
                if (cell->posn_y > ymax) {
                    ymax = cell->posn_y;
                } else if (cell->posn_y < ymin) {
                    ymin = cell->posn_y;
                }
            }
        }
        expand = (int) ceil(cvg->dmax / resolution);
        xmin -= expand;
        xmax += expand;
        ymin -= expand;
        ymax += expand;

        // CG - the position of the cell is out of the system boundry ?
        if (xmin < 0       ) { xmin = 0;        }
        if (xmax > npts_x-1) { xmax = npts_x-1; }
        if (ymin < 0       ) { ymin = 0;        }
        if (ymax > npts_y-1) { ymax = npts_y-1; }
    } else {
        xmin = 0;
        xmax = npts_x-1;
        ymin = 0;
        ymax = npts_y-1;
    }

    // malloc the space of npts_x * npts_y two dimension array
    // where initial scan_array ???
    scan_array = (int **) malloc(npts_x*sizeof(int *));
    for (i=0; i<=npts_x-1; i++) {
        scan_array[i] = IVECTOR(npts_y);
    }

    if (cvg->cell_list) {
        // select cells to run coverage
        scan_cell_list = cvg->cell_list;
    } else {
        // run coverage for system
        scan_cell_list = new ListClass<int>(num_cell);
        for (i=0; i<=num_cell-1; i++) {
            scan_cell_list->append(i);
        }
    }

    if (cvg->use_gpm == 0) {
        for (i=scan_cell_list->getSize()-1; i>=0; i--) {
            cell_idx = (*scan_cell_list)[i];
            cell = cell_list[cell_idx];
            has_gpm = 0;
            for (sector_idx=0; (sector_idx<=cell->num_sector-1)&&(!has_gpm); sector_idx++) {
                sector = cell->sector_list[sector_idx];
                if (sector->prop_model != -1) {
                    pm = prop_model_list[sector->prop_model];
                    if ( strncmp(pm->get_strid(), "GPM", 3) == 0 ) {
                        has_gpm = 1;
                    }
                }
            }
            if (has_gpm) {
                scan_cell_list->del_elem_idx(i);
            }
        }
        scan_cell_list->sort();
    }

    // eqv_num_sector
    cvg->eqv_num_sector = 0.0;
    for (i=0; i<=scan_cell_list->getSize()-1; i++) {
        cell_idx = (*scan_cell_list)[i];
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            antenna = antenna_type_list[sector->antenna_type];
            cvg->eqv_num_sector += antenna->h_width / 360.0;
        }
    }

    scan_cvg = coverage_list[p_cvg_idx];
    if (cvg->type == CConst::layerCoverage) {
        scan_area(this, cvg->init_sample_res, coverage_layer_scan_fn, xmin, xmax, ymin, ymax);
    } else if (cvg->type == CConst::sirLayerCoverage) {
        scan_area(this, cvg->init_sample_res, coverage_sir_layer_scan_fn, xmin, xmax, ymin, ymax);
    } else if (cvg->type == CConst::levelCoverage) {
        scan_area(this, cvg->init_sample_res, coverage_level_scan_fn, xmin, xmax, ymin, ymax);
    } else if (cvg->type == CConst::pagingAreaCoverage) {
        scan_area(this, cvg->init_sample_res, coverage_pa_scan_fn, xmin, xmax, ymin, ymax);
    } else {
        CORE_DUMP;
    }

    if (!cvg->cell_list) {
        delete scan_cell_list;
        scan_cell_list = (ListClass<int> *) NULL;
    }

    GUI_FN(prog_bar->setOffsetWeight(75, 25));

    ListClass<int> *scan_idx_list;
    ListClass<int> *polygon_list;
    ListClass<int> *color_list;

    draw_polygons(cvg->scan_fractional_area, scan_idx_list, polygon_list, color_list);
    sort3(scan_idx_list, polygon_list, color_list);

    cvg->polygon_list = (PolygonClass **) malloc(cvg->scan_list->getSize()*sizeof(PolygonClass *));
    for (i=0; i<=cvg->scan_list->getSize()-1; i++) {
        j = scan_idx_list->get_index( (*(cvg->scan_list))[i], 0);
        if (j == -1) {
            cvg->polygon_list[i] = (PolygonClass *) NULL;
        } else {
            cvg->polygon_list[i] = (PolygonClass *) (*polygon_list)[j];
        }

        /*Tianych for PA color error
        if (cvg->type == CConst::pagingAreaCoverage) {
            // cvg->color_list[i] = default_color_list[(*color_list)[j] % num_default_color];
            cvg->color_list[i] = hot_color->get_color((*color_list)[j], cvg->scan_list->getSize());
        } else {
            // cvg->color_list[i] = default_color_list[i % num_default_color];
            cvg->color_list[i] = hot_color->get_color(i, cvg->scan_list->getSize());
        }
        */
        //cvg->color_list[i] = hot_color->get_color(i, cvg->scan_list->getSize());        
        // Chengan rewrite 190630
        cvg->color_list[i] = hot_color->cal_color_value(i, cvg->scan_list->getSize());        
    }

    delete scan_idx_list;
    delete polygon_list;
    delete color_list;

    for (i=0; i<=npts_x-1; i++) {
        free(scan_array[i]);
    }
    free(scan_array);

    GUI_FN(delete prog_bar);

    return;
}
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

    //if (np->system_bdy->in_bdy_area(posn_x, posn_y))
    {
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
    }
    //else {
    //    np->scan_array[posn_x][posn_y] = CConst::NullScan;
    //}

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

    // have been judged in function scan_area(), and this function is called by scan_area , so no need to judge again
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
            // 两分法查找 CG
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
#if 0
        sprintf(np->msg, "coverage_level_scan_fn() - scan_array[%d][%d] = %d \n",posn_x,posn_y,np->scan_array[posn_x][posn_y]);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
#endif

    } else {
#if 0
        sprintf(np->msg, "coverage_level_scan_fn() - outof boundray scan_array[%d][%d] = %d \n",posn_x,posn_y,np->scan_array[posn_x][posn_y]);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
#endif
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/


