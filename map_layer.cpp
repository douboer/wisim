/******************************************************************************************/
/**** FILE: map_layer.cpp                                                              ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "antenna.h"
#include "cconst.h"
#include "wisim.h"
#include "map_layer.h"
#include "bin_io.h"
#include "list.h"
#include "utm_conversion.h"
#include "polygon.h"

/******************************************************************************************/
/**** FUNCTION: MapLayerClass::MapLayerClass                                           ****/
/******************************************************************************************/
MapLayerClass::MapLayerClass()
{
    color        = 0;
    name         = (char *) NULL;

    num_polygon  = 0;
    polygon_list = (PolygonClass **) NULL;

    num_line     = 0;
    line_list    = (LineClass **) NULL;

    encoding     = EncodingASCII;
    posn_x       = (int *) NULL;
    posn_y       = (int *) NULL;
    num_text     = 0;
    text         = (char **) NULL;

    modified     = 0;
}
/******************************************************************************************/
/**** FUNCTION: MapLayerClass::~MapLayerClass                                           ****/
/******************************************************************************************/
MapLayerClass::~MapLayerClass()
{
    int idx;

    if (name) {
        free(name);
    }

    for (idx=0; idx<=num_polygon-1; idx++) {
        delete polygon_list[idx];
    }

    if (num_polygon) {
        free(polygon_list);
    }

    for (idx=0; idx<=num_line-1; idx++) {
        delete line_list[idx];
    }

    if (num_line) {
        free(line_list);
    }

    for (idx=0; idx<=num_text-1; idx++) {
        free(text[idx]);
    }

    if (num_text) {
        free(posn_x);
        free(posn_y);
        free(text);
    }
}
/******************************************************************************************/
/**** FUNCTION: read_map_layer_mif                                                     ****/
/**** Read .mif file directly.                                                         ****/
/**** FILTER OPTION:                                                                   ****/
/**** 0: Read all object in MIF file into map layer.                                   ****/
/**** 1: Only read objects that lie in rectangular bounding region around system bdy.  ****/
/******************************************************************************************/
void MapLayerClass::read_map_layer_mif(NetworkClass *np, char *filename, char *m_name, int filter)
{
    int n, state;
    int segment_idx = -1;
    int pt_idx      = -1;
    int use;
    int first_point = 1;
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
    int has_geometry;
    int brush_type;
    char *str1, str[1000];
    char *chptr;
    PolygonClass *polygon = (PolygonClass *) NULL;
    LineClass *pline = (LineClass *) NULL;
    double lon_deg, lat_deg, utm_x, utm_y;
    FILE *fp;

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(np->msg, "ERROR: Unable to read from file 5 %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    if (!np->system_bdy) {
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    sprintf(np->msg, "Reading MIF file: %s\n", filename);
    PRMSG(stdout, np->msg);

    enum state_enum {
        STATE_IDLE,
        STATE_BEGIN_POLY_SEGMENT,
        STATE_READING_POLY_SEGMENT,
        STATE_POLY_BRUSH,
        STATE_READING_LINE_SEGMENT,
        STATE_DONE,
    };

    if (!has_geometry) {
        np->coordinate_system = CConst::CoordUTM;
        np->utm_equatorial_radius = 6380725.0;
        np->utm_eccentricity_sq   = 0.006681000;
        np->resolution            = 1.0;
        filter = 0;
    }

    if (m_name) {
        name = strdup(m_name);
    } else {
        sprintf(str, "map_layer_%d", np->map_layer_list->getSize());
        name = strdup(str);
    }

    state = STATE_IDLE;

    while ( fgetline(fp, line) ) {
//        printf("%s", line);
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            chptr = np->msg;
            switch(state) {
                case STATE_IDLE:
                    if (strcmp(str1, "Region") == 0) {
                        polygon = new PolygonClass();
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            polygon->num_segment = atoi(str1);
                        } else {
                            polygon->num_segment = 0;
                        }
                        polygon->num_bdy_pt = IVECTOR(polygon->num_segment);
                        polygon->bdy_pt_x   = (int **) malloc(polygon->num_segment * sizeof(int *));
                        polygon->bdy_pt_y   = (int **) malloc(polygon->num_segment * sizeof(int *));
                        state = STATE_BEGIN_POLY_SEGMENT;
                        segment_idx = 0;
                    } else if (strcmp(str1, "Pline") == 0) {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            if (strcmp(str1, "Multiple") == 0) {
                                n = 0;
                            } else {
                                n = atoi(str1);
                            }
                        } else {
                            n = 0;
                        }
                        if (n) {
                            pline = new LineClass();
                            pline->num_pt = n;
                            pline->pt_x = IVECTOR(pline->num_pt);
                            pline->pt_y = IVECTOR(pline->num_pt);
                            pt_idx = 0;
                            state = STATE_READING_LINE_SEGMENT;
                        }
                    } else if (strcmp(str1, "Line") == 0) {
                        pline = new LineClass();
                        pline->num_pt = 2;
                        pline->pt_x = IVECTOR(pline->num_pt);
                        pline->pt_y = IVECTOR(pline->num_pt);

                        for (pt_idx=0; pt_idx<=1; pt_idx++) {
                            str1 = strtok(NULL, CHDELIM);
                            lon_deg = atof(str1);
                            str1 = strtok(NULL, CHDELIM);
                            lat_deg = atof(str1);
                            if ((!has_geometry) && (first_point)) {
                                getUTMZone(np->utm_zone, np->utm_north, lon_deg, lat_deg);
                            }
                            LLtoUTM( lon_deg, lat_deg,
                                     utm_x,  utm_y,
                                     np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);
                            if ((!has_geometry) && (first_point)) {
                                check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                                check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                                first_point = 0;
                            }

                            check_grid_val(utm_x, np->resolution, np->system_startx, &(pline->pt_x[pt_idx]));
                            check_grid_val(utm_y, np->resolution, np->system_starty, &(pline->pt_y[pt_idx]));
                            if (pline->pt_x[pt_idx] < min_x) { min_x = pline->pt_x[pt_idx]; }
                            if (pline->pt_x[pt_idx] > max_x) { max_x = pline->pt_x[pt_idx]; }
                            if (pline->pt_y[pt_idx] < min_y) { min_y = pline->pt_y[pt_idx]; }
                            if (pline->pt_y[pt_idx] > max_y) { max_y = pline->pt_y[pt_idx]; }
                        }
                        use = fix(pline, np, filter);
                        if ((use) && (pline->num_pt)) {
                            num_line++;
                            line_list = (LineClass **) realloc( (void *) line_list, num_line*sizeof(LineClass *));
                            line_list[num_line-1] = pline;
                        } else {
                            delete pline;
                            pline = new LineClass();
                        }
                    }
                    break;
                case STATE_BEGIN_POLY_SEGMENT:
                    if (str1) {
                        polygon->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        polygon->num_bdy_pt[segment_idx] = 0;
                    }
                    polygon->bdy_pt_x[segment_idx] = IVECTOR(polygon->num_bdy_pt[segment_idx]);
                    polygon->bdy_pt_y[segment_idx] = IVECTOR(polygon->num_bdy_pt[segment_idx]);
                    pt_idx = 0;
                    state = STATE_READING_POLY_SEGMENT;
                    break;
                case STATE_READING_POLY_SEGMENT:
                    lon_deg = atof(str1);
                    str1 = strtok(NULL, CHDELIM);
                    lat_deg = atof(str1);

                    if ((!has_geometry) && (first_point)) {
                        getUTMZone(np->utm_zone, np->utm_north, lon_deg, lat_deg);
                    }
                    LLtoUTM( lon_deg, lat_deg,
                             utm_x,  utm_y,
                             np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);

                    if ((!has_geometry) && (first_point)) {
                        check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                        check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                        first_point = 0;
                    }

                    check_grid_val(utm_x, np->resolution, np->system_startx, &(polygon->bdy_pt_x[segment_idx][pt_idx]));
                    check_grid_val(utm_y, np->resolution, np->system_starty, &(polygon->bdy_pt_y[segment_idx][pt_idx]));
                    if (polygon->bdy_pt_x[segment_idx][pt_idx] < min_x) { min_x = polygon->bdy_pt_x[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_x[segment_idx][pt_idx] > max_x) { max_x = polygon->bdy_pt_x[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_y[segment_idx][pt_idx] < min_y) { min_y = polygon->bdy_pt_y[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_y[segment_idx][pt_idx] > max_y) { max_y = polygon->bdy_pt_y[segment_idx][pt_idx]; }
                    pt_idx++;
                    if (pt_idx == polygon->num_bdy_pt[segment_idx]) {
                        segment_idx++;
                        if (segment_idx == polygon->num_segment) {
                            use = fix(polygon, np, filter);
                            if ((use) && (polygon->num_segment)) {
                                // num_polygon++;
                                // polygon_list = (PolygonClass **) realloc( (void *) polygon_list, num_polygon*sizeof(PolygonClass *));
                                // polygon_list[num_polygon-1] = polygon;
                                state = STATE_POLY_BRUSH;
                            } else {
                                delete polygon;
                                polygon = (PolygonClass *) NULL;
                                state = STATE_IDLE;
                            }
                        } else {
                            state = STATE_BEGIN_POLY_SEGMENT;
                        }
                    }
                    break;
                case STATE_POLY_BRUSH:
                    if (strcmp(str1, "Brush") == 0) {
                        str1 = strtok(NULL, " \t\n(),");
                        brush_type = (str1 ? atoi(str1) : 2);
                        if (brush_type == 1) {
                            // Transparent, convert each segment to line.
                            for (segment_idx=0; segment_idx<=polygon->num_segment-1; segment_idx++) {
                                n = polygon->num_bdy_pt[segment_idx];
                                pline = new LineClass();
                                pline->num_pt = n+1;
                                pline->pt_x = polygon->bdy_pt_x[segment_idx];
                                pline->pt_y = polygon->bdy_pt_y[segment_idx];
                                pline->pt_x = (int *) realloc( (int *) pline->pt_x, (n+1)*sizeof(int));
                                pline->pt_y = (int *) realloc( (int *) pline->pt_y, (n+1)*sizeof(int));
                                pline->pt_x[n] = pline->pt_x[0];
                                pline->pt_y[n] = pline->pt_y[0];

                                num_line++;
                                line_list = (LineClass **) realloc( (void *) line_list, num_line*sizeof(LineClass *));
                                line_list[num_line-1] = pline;

                                polygon->num_bdy_pt[segment_idx] = 0;
                                polygon->bdy_pt_x[segment_idx] = (int *) NULL;
                                polygon->bdy_pt_y[segment_idx] = (int *) NULL;
                            }
                            delete polygon;
                            polygon = (PolygonClass *) NULL;
                        } else {
                            // Opaque, (brush_type = 2)
                            num_polygon++;
                            polygon_list = (PolygonClass **) realloc( (void *) polygon_list, num_polygon*sizeof(PolygonClass *));
                            polygon_list[num_polygon-1] = polygon;
                        }

                        state = STATE_IDLE;
                    }
                    break;
                case STATE_READING_LINE_SEGMENT:
                    lon_deg = atof(str1);
                    str1 = strtok(NULL, CHDELIM);
                    lat_deg = atof(str1);
                    if ((!has_geometry) && (first_point)) {
                        getUTMZone(np->utm_zone, np->utm_north, lon_deg, lat_deg);
                    }
                    LLtoUTM( lon_deg, lat_deg,
                             utm_x,  utm_y,
                             np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);
                    if ((!has_geometry) && (first_point)) {
                        check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                        check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                        first_point = 0;
                    }

                    check_grid_val(utm_x, np->resolution, np->system_startx, &(pline->pt_x[pt_idx]));
                    check_grid_val(utm_y, np->resolution, np->system_starty, &(pline->pt_y[pt_idx]));
                    if (pline->pt_x[pt_idx] < min_x) { min_x = pline->pt_x[pt_idx]; }
                    if (pline->pt_x[pt_idx] > max_x) { max_x = pline->pt_x[pt_idx]; }
                    if (pline->pt_y[pt_idx] < min_y) { min_y = pline->pt_y[pt_idx]; }
                    if (pline->pt_y[pt_idx] > max_y) { max_y = pline->pt_y[pt_idx]; }
                    pt_idx++;
                    if (pt_idx == pline->num_pt) {
                        use = fix(pline, np, filter);
                        if ((use) && (pline->num_pt)) {
                            num_line++;
                            line_list = (LineClass **) realloc( (void *) line_list, num_line*sizeof(LineClass *));
                            line_list[num_line-1] = pline;
                        } else {
                            delete pline;
                            pline = new LineClass();
                        }
                        state = STATE_IDLE;
                    }
                    break;
                default:
                    chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                    chptr += sprintf(chptr, "Invalid state (%d) encountered\n", state);
                    PRMSG(stdout, np->msg); np->error_state = 1;
                    return;
                    break;
            }
        }
    }

    fclose(fp);
    free(line);

    polygon->remove_duplicate_points(-1);

    if ((num_polygon==0) && (num_line==0) && (num_text==0)) {
        sprintf(np->msg, "ERROR: MIF file \"%s\" contains no objects\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    if (!has_geometry) {
        shift(-min_x, -min_y);
        np->system_startx += min_x;
        np->system_starty += min_y;
        np->system_bdy = new PolygonClass();
        np->system_bdy->num_segment = 1;
        np->system_bdy->num_bdy_pt = IVECTOR(np->system_bdy->num_segment);
        np->system_bdy->bdy_pt_x   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->bdy_pt_y   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->num_bdy_pt[0] = 4;
        np->system_bdy->bdy_pt_x[0] = IVECTOR(4);
        np->system_bdy->bdy_pt_y[0] = IVECTOR(4);
        np->npts_x = max_x - min_x + 1;
        np->npts_y = max_y - min_y + 1;
        np->system_bdy->bdy_pt_x[0][0] = 0;
        np->system_bdy->bdy_pt_y[0][0] = 0;
        np->system_bdy->bdy_pt_x[0][1] = np->npts_x-1;
        np->system_bdy->bdy_pt_y[0][1] = 0;
        np->system_bdy->bdy_pt_x[0][2] = np->npts_x-1;
        np->system_bdy->bdy_pt_y[0][2] = np->npts_y-1;
        np->system_bdy->bdy_pt_x[0][3] = 0;
        np->system_bdy->bdy_pt_y[0][3] = np->npts_y-1;
        np->num_antenna_type = 1;
        np->antenna_type_list = (AntennaClass **) malloc(np->num_antenna_type*sizeof(AntennaClass *));
        np->antenna_type_list[0] = new AntennaClass(CConst::antennaOmni, "OMNI");
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: read_map_layer                                                         ****/
/**** Open specified file and read map layer data.                                     ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void MapLayerClass::read_map_layer(NetworkClass *np, char *filename, int force_read)
{
    int i, n, state;
    int polygon_idx = -1;
    int line_idx = -1;
    int segment_idx = -1;
    int pt_idx      = -1;
    int first_point = 1;
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
    int has_geometry;
    int utm_zone_mismatch = 0;
    int utm_zone, utm_north;
    char *str1, str[1000];
    char *chptr;
    PolygonClass *polygon = (PolygonClass *) NULL;
    LineClass *pline = (LineClass *) NULL;
    double utm_x, utm_y;
    FILE *fp;

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(np->msg, "ERROR: Unable to read from file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    if (!np->system_bdy) {
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    sprintf(np->msg, "Reading map layer file: %s\n", filename);
    PRMSG(stdout, np->msg);

    enum state_enum {
        STATE_LAYER_NAME,
        STATE_COORDINATE_SYSTEM,
        STATE_ENCODING,
        STATE_DEFAULT_COLOR,

        STATE_NUM_POLYGON,
        STATE_POLYGON,
        STATE_NUM_SEGMENT,
        STATE_SEGMENT,
        STATE_NUM_POLY_PT,
        STATE_POLY_PT,

        STATE_NUM_LINE,
        STATE_LINE,
        STATE_NUM_LINE_PT,
        STATE_LINE_PT,

        STATE_NUM_TEXT,
        STATE_TEXT,
        STATE_DONE,
    };

    if (!has_geometry) {
        np->resolution            = 1.0;
    }

    state = STATE_LAYER_NAME;

    while ( fgetline(fp, line) ) {
//        printf("%s", line);
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            chptr = np->msg;
            switch(state) {
                case STATE_LAYER_NAME:
                    if (strcmp(str1, "LAYER_NAME:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"LAYER_NAME:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        name = strdup(str1);
                        state = STATE_COORDINATE_SYSTEM;
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "No LAYER_NAME specified\n");
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    break;
                case STATE_COORDINATE_SYSTEM:
                    if (strcmp(str1, "COORDINATE_SYSTEM:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"COORDINATE_SYSTEM:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        sprintf(np->msg, "ERROR: invalid map_layer file \"%s\"\nCOORDINATE_SYSTEM does not match\n", filename);
                        if (strcmp(str1, "GENERIC")==0) {
                            if (has_geometry) {
                                if (np->coordinate_system != CConst::CoordGeneric) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; } else { np->warning_state = 1; }
                                }
                            } else {
                                np->coordinate_system = CConst::CoordGeneric;
                            }
                        } else if (strncmp(str1, "UTM:", 4)==0) {
                            if (has_geometry) {
                                if (np->coordinate_system != CConst::CoordUTM) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; }
                                }
                            } else {
                                np->coordinate_system = CConst::CoordUTM;
                            }
                            str1 = strtok(str1+4, ":");
                            if (has_geometry) {
                                if (np->utm_equatorial_radius != atof(str1)) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; }
                                }
                            } else {
                                np->utm_equatorial_radius = atof(str1);
                            }
                            str1 = strtok(NULL, ":");
                            if (has_geometry) {
                                if (np->utm_eccentricity_sq != atof(str1)) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; }
                                }
                            } else {
                                np->utm_eccentricity_sq = atof(str1);
                            }
                            str1 = strtok(NULL, ":");
                            utm_zone = atoi(str1);
                            if (has_geometry) {
                                if (np->utm_zone != utm_zone) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; }
                                    utm_zone_mismatch = 1;
                                }
                            } else {
                                np->utm_zone = utm_zone;
                            }
                            str1 = strtok(NULL, ":");
                            if (has_geometry) {
                                if (strcmp(str1, (np->utm_north ? "N" : "S")) != 0) {
                                    PRMSG(stdout, np->msg);
                                    if (!force_read) { np->error_state = 1; return; }
                                    utm_zone_mismatch = 1;
                                    if (strcmp(str1, "N") == 0) {
                                        utm_north = 1;
                                    } else if (strcmp(str1, "S") == 0) {
                                        utm_north = 0;
                                    } else {
                                        utm_north = np->utm_north;
                                    }
                                }
                            } else {
                                np->utm_north = (strcmp(str1,"N")==0 ? 1 : 0);
                            }
                        }
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "No COORDINATE_SYSTEM specified\n");
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    state = STATE_ENCODING;
                    break;
                case STATE_ENCODING:
                    if (strcmp(str1,"ENCODING:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"ENCODING:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (strcmp(str1,"GB") == 0)
                          encoding = EncodingGB;
                    else if (strcmp(str1, "ASCII") == 0)
                          encoding = EncodingASCII;
                    else {
                          chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                          chptr += sprintf(chptr, "encoding=\"%s\" must be GB or ASCII\n", str1);
                          PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    state = STATE_DEFAULT_COLOR;
                    break;
                case STATE_DEFAULT_COLOR:
                    if (strcmp(str1, "DEFAULT_COLOR:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"DEFAULT_COLOR:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    color = 0;
                    for (i=0; i<=2; i++) {
                        str1 = strtok(NULL, CHDELIM);
                        if (str1) {
                            n = atoi(str1);
                            if ( (n<0) || (n>255) ) {
                                chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                                chptr += sprintf(chptr, "Invalid DEFLAULT_COLOR specification\n");
                                chptr += sprintf(chptr, "Color component %d not in the range [0,255]\n", n);
                                PRMSG(stdout, np->msg); np->error_state = 1;
                                return;
                            }
                            color = color | (n << (8*(2-i)));
                        } else {
                            chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                            chptr += sprintf(chptr, "Invalid DEFLAULT_COLOR specification\n");
                            PRMSG(stdout, np->msg); np->error_state = 1;
                            return;
                        }
                    }
                    state = STATE_NUM_POLYGON;
                    break;
                case STATE_NUM_POLYGON:
                    if (strcmp(str1, "NUM_POLYGON:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_POLYGON:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_polygon = atoi(str1);
                    } else {
                        num_polygon = 0;
                    }

                    if (num_polygon) {
                        polygon_list = (PolygonClass **) malloc(num_polygon * sizeof(PolygonClass *));
                        polygon_idx = 0;
                        state = STATE_POLYGON;
                    } else {
                        state = STATE_NUM_LINE;
                    }
                    break;
                case STATE_POLYGON:
                    sprintf(str, "POLYGON_%d:", polygon_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    polygon_list[polygon_idx] = new PolygonClass();
                    polygon = polygon_list[polygon_idx];
                    state = STATE_NUM_SEGMENT;
                    break;
                case STATE_NUM_SEGMENT:
                    if (strcmp(str1, "NUM_SEGMENT:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_SEGMENT:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        polygon->num_segment = atoi(str1);
                    } else {
                        polygon->num_segment = 0;
                    }

                    if (polygon->num_segment < 1) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "polygon->num_segment = %d must be >= 1\n", polygon->num_segment);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    polygon->num_bdy_pt = IVECTOR(polygon->num_segment);
                    polygon->bdy_pt_x   = (int **) malloc(polygon->num_segment * sizeof(int *));
                    polygon->bdy_pt_y   = (int **) malloc(polygon->num_segment * sizeof(int *));
                    segment_idx = 0;
                    state = STATE_SEGMENT;
                    break;
                case STATE_SEGMENT:
                    sprintf(str, "SEGMENT_%d:", segment_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    state = STATE_NUM_POLY_PT;
                    break;
                case STATE_NUM_POLY_PT:
                    if (strcmp(str1, "NUM_PT:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_PT:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        polygon->num_bdy_pt[segment_idx] = atoi(str1);
                    } else {
                        polygon->num_bdy_pt[segment_idx] = 0;
                    }

                    n = 3;
                    if ( polygon->num_bdy_pt[segment_idx] < n ) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "polygon->num_bdy_pt[%d] = %d must be >= %d\n", segment_idx, polygon->num_bdy_pt[segment_idx], n);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    polygon->bdy_pt_x[segment_idx] = IVECTOR(polygon->num_bdy_pt[segment_idx]);
                    polygon->bdy_pt_y[segment_idx] = IVECTOR(polygon->num_bdy_pt[segment_idx]);
                    pt_idx = 0;
                    state = STATE_POLY_PT;
                    break;
                case STATE_POLY_PT:
                    sprintf(str, "PT_%d:", pt_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_x = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for polygon %d segment %d point %d\n",
                                         polygon_idx, segment_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_y = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for polygon %d segment %d point %d\n",
                                         polygon_idx, segment_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    if ((!has_geometry) && (first_point)) {
                        check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                        check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                        first_point = 0;
                    }

                    check_grid_val(utm_x, np->resolution, np->system_startx, &(polygon->bdy_pt_x[segment_idx][pt_idx]));
                    check_grid_val(utm_y, np->resolution, np->system_starty, &(polygon->bdy_pt_y[segment_idx][pt_idx]));
                    if (polygon->bdy_pt_x[segment_idx][pt_idx] < min_x) { min_x = polygon->bdy_pt_x[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_x[segment_idx][pt_idx] > max_x) { max_x = polygon->bdy_pt_x[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_y[segment_idx][pt_idx] < min_y) { min_y = polygon->bdy_pt_y[segment_idx][pt_idx]; }
                    if (polygon->bdy_pt_y[segment_idx][pt_idx] > max_y) { max_y = polygon->bdy_pt_y[segment_idx][pt_idx]; }
                    pt_idx++;
                    if (pt_idx <= polygon->num_bdy_pt[segment_idx]-1) {
                        state = STATE_POLY_PT;
                    } else {
                        segment_idx++;
                        if (segment_idx <= polygon->num_segment-1) {
                            state = STATE_SEGMENT;
                        } else {
                            polygon_idx++;
                            if (polygon_idx <= num_polygon-1) {
                                state = STATE_POLYGON;
                            } else {
                                state = STATE_NUM_LINE;
                            }
                        }
                    }
                    break;
                case STATE_NUM_LINE:
                    if (strcmp(str1, "NUM_LINE:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_LINE:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_line = atoi(str1);
                    } else {
                        num_line = 0;
                    }

                    if (num_line) {
                        line_list = (LineClass **) malloc(num_line * sizeof(LineClass *));
                        line_idx = 0;
                        state = STATE_LINE;
                    } else {
                        state = STATE_NUM_TEXT;
                    }
                    break;
                case STATE_LINE:
                    sprintf(str, "LINE_%d:", line_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    line_list[line_idx] = new LineClass();
                    pline = line_list[line_idx];
                    state = STATE_NUM_LINE_PT;
                    break;
                case STATE_NUM_LINE_PT:
                    if (strcmp(str1, "NUM_PT:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_PT:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        pline->num_pt = atoi(str1);
                    } else {
                        pline->num_pt = 0;
                    }

                    if ( pline->num_pt < 2 ) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "pline->num_pt = %d must be >= 2\n", pline->num_pt);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    pline->pt_x = IVECTOR(pline->num_pt);
                    pline->pt_y = IVECTOR(pline->num_pt);
                    pt_idx = 0;
                    state = STATE_LINE_PT;
                    break;
                case STATE_LINE_PT:
                    sprintf(str, "PT_%d:", pt_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_x = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for line %d point %d\n",
                                         line_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_y = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for line %d point %d\n",
                                         polygon_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    if ((!has_geometry) && (first_point)) {
                        check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                        check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                        first_point = 0;
                    }

                    check_grid_val(utm_x, np->resolution, np->system_startx, &(pline->pt_x[pt_idx]));
                    check_grid_val(utm_y, np->resolution, np->system_starty, &(pline->pt_y[pt_idx]));
                    if (pline->pt_x[pt_idx] < min_x) { min_x = pline->pt_x[pt_idx]; }
                    if (pline->pt_x[pt_idx] > max_x) { max_x = pline->pt_x[pt_idx]; }
                    if (pline->pt_y[pt_idx] < min_y) { min_y = pline->pt_y[pt_idx]; }
                    if (pline->pt_y[pt_idx] > max_y) { max_y = pline->pt_y[pt_idx]; }
                    pt_idx++;
                    if (pt_idx <= pline->num_pt-1) {
                        state = STATE_LINE_PT;
                    } else {
                        line_idx++;
                        if (line_idx <= num_line-1) {
                            state = STATE_LINE;
                        } else {
                            state = STATE_NUM_TEXT;
                        }
                    }
                    break;
                case STATE_NUM_TEXT:
                    if (strcmp(str1,"NUM_TEXT:") != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"NUM_TEXT:\" NOT \"%s\"\n", str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        num_text = atoi(str1);
                    } else {
                        num_text = 0;
                    }
                    posn_x = IVECTOR(num_text);
                    posn_y = IVECTOR(num_text);
                    text = (char **) malloc(num_text*sizeof(char *));
                    pt_idx = 0;
                    state = STATE_TEXT;
                    break;
                case STATE_TEXT:
                    sprintf(str, "T_%d:", pt_idx);
                    if (strcmp(str1, str) != 0) {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Expecting \"%s\" NOT \"%s\"\n", str, str1);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_x = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for text point %d\n", pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        utm_y = atof(str1);
                    } else {
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Invalid coordinate specification for text point %d\n", pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
                    }

                    if ((!has_geometry) && (first_point)) {
                        check_grid_val(utm_x, np->resolution, 0, &(np->system_startx));
                        check_grid_val(utm_y, np->resolution, 0, &(np->system_starty));
                        first_point = 0;
                    }

                    check_grid_val(utm_x, np->resolution, np->system_startx, &(posn_x[pt_idx]));
                    check_grid_val(utm_y, np->resolution, np->system_starty, &(posn_y[pt_idx]));
                    if (posn_x[pt_idx] < min_x) { min_x = posn_x[pt_idx]; }
                    if (posn_x[pt_idx] > max_x) { max_x = posn_x[pt_idx]; }
                    if (posn_y[pt_idx] < min_y) { min_y = posn_y[pt_idx]; }
                    if (posn_y[pt_idx] > max_y) { max_y = posn_y[pt_idx]; }
                    str1 = strtok(NULL, CHDELIM);
                    text[pt_idx] = strdup(str1);
                    pt_idx++;
                    if (pt_idx <= num_text-1) {
                        state = STATE_TEXT;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_DONE:
                    chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                    chptr += sprintf(chptr, "File contains data past end of polygon list\n");
                    PRMSG(stdout, np->msg); np->error_state = 1;
                    return;
                    break;
                default:
                    chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                    chptr += sprintf(chptr, "Invalid state (%d) encountered\n", state);
                    PRMSG(stdout, np->msg); np->error_state = 1;
                    return;
                    break;
            }
        }
    }

    for (polygon_idx=0; polygon_idx<=num_polygon-1; polygon_idx++) {
        polygon = polygon_list[polygon_idx];
        for (segment_idx=polygon->num_segment-1; segment_idx>=0; segment_idx--) {
            if (PolygonClass::comp_bdy_area(polygon->num_bdy_pt[segment_idx], polygon->bdy_pt_x[segment_idx], polygon->bdy_pt_y[segment_idx]) == 0.0) {
                free(polygon->bdy_pt_x[segment_idx]);
                free(polygon->bdy_pt_y[segment_idx]);
                for (i=segment_idx; i<=polygon->num_segment-2; i++) {
                    polygon->bdy_pt_x[i] = polygon->bdy_pt_x[i+1];
                    polygon->bdy_pt_y[i] = polygon->bdy_pt_y[i+1];
                }
                polygon->num_segment--;
                if (polygon->num_segment) {
                    polygon->bdy_pt_x = (int **) realloc( (void *) polygon->bdy_pt_x, polygon->num_segment*sizeof(int *));
                    polygon->bdy_pt_y = (int **) realloc( (void *) polygon->bdy_pt_y, polygon->num_segment*sizeof(int *));
                } else {
                    free(polygon->bdy_pt_x);
                    free(polygon->bdy_pt_y);
                    polygon->bdy_pt_x = (int **) NULL;
                    polygon->bdy_pt_y = (int **) NULL;
                }
            }
        }
    }

    for (polygon_idx=num_polygon-1; polygon_idx>=0; polygon_idx--) {
        if (polygon_list[polygon_idx]->num_segment == 0) {
            delete polygon_list[polygon_idx];
            for (i=polygon_idx; i<=num_polygon-2; i++) {
                polygon_list[i] = polygon_list[i+1];
            }
            num_polygon--;
        }
    }
    if (num_polygon) {
        polygon_list = (PolygonClass **) realloc( (void *) polygon_list, num_polygon*sizeof(PolygonClass *));
    } else {
        free(polygon_list);
        polygon_list = (PolygonClass **) NULL;
    }

    if (has_geometry) {
        PolygonClass *rect_bdy = new PolygonClass();
        rect_bdy->num_segment = 1;
        rect_bdy->num_bdy_pt  = (int *) malloc(sizeof(int));
        rect_bdy->bdy_pt_x = (int **) malloc(sizeof(int *));
        rect_bdy->bdy_pt_y = (int **) malloc(sizeof(int *));
        rect_bdy->bdy_pt_x[0] = (int *) malloc(4*sizeof(int));
        rect_bdy->bdy_pt_y[0] = (int *) malloc(4*sizeof(int));
        rect_bdy->num_bdy_pt[0]  = 4;
        rect_bdy->bdy_pt_x[0][0] = 0;
        rect_bdy->bdy_pt_y[0][0] = 0;
        rect_bdy->bdy_pt_x[0][1] = np->npts_x-1;
        rect_bdy->bdy_pt_y[0][1] = 0;
        rect_bdy->bdy_pt_x[0][2] = np->npts_x-1;
        rect_bdy->bdy_pt_y[0][2] = np->npts_y-1;
        rect_bdy->bdy_pt_x[0][3] = 0;
        rect_bdy->bdy_pt_y[0][3] = np->npts_y-1;

        int found = 0;
        for (polygon_idx=0; (polygon_idx<=num_polygon-1)&&(!found); polygon_idx++) {
            polygon = polygon_list[polygon_idx];
            for (segment_idx=0; (segment_idx<=polygon->num_segment-1)&&(!found); segment_idx++) {
                for (pt_idx=1; (pt_idx<=polygon->num_bdy_pt[segment_idx]-1)&&(!found); pt_idx++) {
                    if (!rect_bdy->in_bdy_area(polygon->bdy_pt_x[segment_idx][pt_idx], polygon->bdy_pt_y[segment_idx][pt_idx])) { /* system_bdy */
#if 1
                        chptr = np->msg;
                        chptr += sprintf(chptr, "WARNING: map layer file \"%s\" contains points outside the system boundary area\n", filename);
                        chptr += sprintf(chptr, "Polygon %d segment %d pt %d is not contained in the system boundary area\n", polygon_idx, segment_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->warning_state = 1;
                        found = 1;
#else
                        chptr = np->msg;
                        chptr += sprintf(chptr, "ERROR: invalid map layer file \"%s\"\n", filename);
                        chptr += sprintf(chptr, "Polygon %d segment %d pt %d is not contained in the system boundary area\n", polygon_idx, segment_idx, pt_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                        return;
#endif
                    }
                }
            }
#if 0
            printf("POLYGON: %d AREA = %15.10f\n", polygon_idx, comp_bdy_area(polygon));
#endif
        }

        delete rect_bdy;
    } else {
        shift(-min_x, -min_y);
        np->system_startx += min_x;
        np->system_starty += min_y;
        np->system_bdy = new PolygonClass();
        np->system_bdy->num_segment = 1;
        np->system_bdy->num_bdy_pt = IVECTOR(np->system_bdy->num_segment);
        np->system_bdy->bdy_pt_x   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->bdy_pt_y   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->num_bdy_pt[0] = 4;
        np->system_bdy->bdy_pt_x[0] = IVECTOR(4);
        np->system_bdy->bdy_pt_y[0] = IVECTOR(4);
        np->npts_x = max_x - min_x + 1;
        np->npts_y = max_y - min_y + 1;
        np->system_bdy->bdy_pt_x[0][0] = 0;
        np->system_bdy->bdy_pt_y[0][0] = 0;
        np->system_bdy->bdy_pt_x[0][1] = np->npts_x-1;
        np->system_bdy->bdy_pt_y[0][1] = 0;
        np->system_bdy->bdy_pt_x[0][2] = np->npts_x-1;
        np->system_bdy->bdy_pt_y[0][2] = np->npts_y-1;
        np->system_bdy->bdy_pt_x[0][3] = 0;
        np->system_bdy->bdy_pt_y[0][3] = np->npts_y-1;
        np->num_antenna_type = 1;
        np->antenna_type_list = (AntennaClass **) malloc(np->num_antenna_type*sizeof(AntennaClass *));
        np->antenna_type_list[0] = new AntennaClass(CConst::antennaOmni, "OMNI");
    }

    fclose(fp);
    free(line);

    if ((num_polygon==0) && (num_line==0) && (num_text==0)) {
        sprintf(np->msg, "ERROR: MIF file \"%s\" contains no objects\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: save                                                                   ****/
/**** Save map later to the specified file.                                            ****/
/******************************************************************************************/
void MapLayerClass::save(NetworkClass *np, char *filename)
{
    int polygon_idx = -1;
    int line_idx = -1;
    int segment_idx = -1;
    int pt_idx      = -1;
    char *chptr;
    PolygonClass *polygon = (PolygonClass *) NULL;
    LineClass *pline = (LineClass *) NULL;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(np->msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "# WISIM MAP LAYER FILE\n");
    chptr += sprintf(chptr, "LAYER_NAME: %s\n", name);
    chptr += sprintf(chptr, "\n");

    if (np->coordinate_system == CConst::CoordGeneric) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: GENERIC\n");
    } else if (np->coordinate_system == CConst::CoordUTM) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: UTM:%.0f:%.9f:%d:%c\n", np->utm_equatorial_radius, np->utm_eccentricity_sq,
            np->utm_zone, (np->utm_north ? 'N' : 'S'));
    } else if (np->coordinate_system == CConst::CoordLONLAT) {
        chptr += sprintf(chptr, "COORDINATE_SYSTEM: LON_LAT\n");
    } else {
        CORE_DUMP;
    }

    chptr += sprintf(chptr, "ENCODING: %s\n", (encoding == EncodingGB ? "GB" : "ASCII"));
    chptr += sprintf(chptr, "DEFAULT_COLOR: %d %d %d\n", (color >> 16)&255, (color >> 8)&255, color&255);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    chptr = np->msg;
    chptr += sprintf(chptr, "NUM_POLYGON: %d\n", num_polygon);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    for (polygon_idx=0; polygon_idx<=num_polygon-1; polygon_idx++) {
        polygon = polygon_list[polygon_idx];

        chptr = np->msg;
        chptr += sprintf(chptr, "POLYGON_%d:\n", polygon_idx);
        chptr += sprintf(chptr, "    NUM_SEGMENT: %d\n", polygon->num_segment);
        PRMSG(fp, np->msg);

        for (segment_idx=0; segment_idx<=polygon->num_segment-1; segment_idx++) {
            chptr = np->msg;
            chptr += sprintf(chptr, "    SEGMENT_%d:\n", segment_idx);
            chptr += sprintf(chptr, "        NUM_PT: %d\n", polygon->num_bdy_pt[segment_idx]);
            PRMSG(fp, np->msg);

            for (pt_idx=0; pt_idx<=polygon->num_bdy_pt[segment_idx]-1; pt_idx++) {
                chptr = np->msg;
                chptr += sprintf(chptr, "            PT_%d: %15.13f %15.13f\n",
                    pt_idx, np->idx_to_x(polygon->bdy_pt_x[segment_idx][pt_idx]),
                            np->idx_to_y(polygon->bdy_pt_y[segment_idx][pt_idx]));
                PRMSG(fp, np->msg);
            }
        }

        chptr = np->msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, np->msg);
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "NUM_LINE: %d\n", num_line);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    for (line_idx=0; line_idx<=num_line-1; line_idx++) {
        pline = line_list[line_idx];

        chptr = np->msg;
        chptr += sprintf(chptr, "LINE_%d:\n", line_idx);
        chptr += sprintf(chptr, "    NUM_PT: %d\n", pline->num_pt);
        PRMSG(fp, np->msg);

        for (pt_idx=0; pt_idx<=pline->num_pt-1; pt_idx++) {
            chptr = np->msg;
            chptr += sprintf(chptr, "        PT_%d: %15.13f %15.13f\n",
                pt_idx, np->idx_to_x(pline->pt_x[pt_idx]),
                        np->idx_to_y(pline->pt_y[pt_idx]));
            PRMSG(fp, np->msg);
        }

        chptr = np->msg;
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, np->msg);
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "NUM_TEXT: %d\n", num_text);
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, np->msg);

    for (pt_idx=0; pt_idx<=num_text-1; pt_idx++) {
        chptr = np->msg;
        chptr += sprintf(chptr, "    T_%d: %15.13f %15.13f %s\n",
            pt_idx, np->idx_to_x(posn_x[pt_idx]),
                    np->idx_to_y(posn_y[pt_idx]), text[pt_idx]);
        PRMSG(fp, np->msg);
    }

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: MapLayerClass::fix                                                     ****/
/**** Fix polygon.                                                                     ****/
/******************************************************************************************/
int MapLayerClass::fix(PolygonClass *polygon, NetworkClass *np, int filter)
{
    int segment_idx, num_move, i, n, found, *pt_list_x, *pt_list_y;

    if (filter) {
        found = 0;
        for (segment_idx=0; (segment_idx<=polygon->num_segment-1)&&(!found); segment_idx++) {
            n = polygon->num_bdy_pt[segment_idx];
            pt_list_x = polygon->bdy_pt_x[segment_idx];
            pt_list_y = polygon->bdy_pt_y[segment_idx];
            for (i=0; (i<=n-1)&&(!found); i++) {
                if (   (pt_list_x[i] >= 0) && (pt_list_x[i] <= np->npts_x-1)
                    && (pt_list_y[i] >= 0) && (pt_list_y[i] <= np->npts_y-1) ) {
                    found = 1;
                }
            }
        }
    } else {
        found = 1;
    }

    if (found) {
        for (segment_idx=polygon->num_segment-1; segment_idx>=0; segment_idx--) {
            polygon->remove_duplicate_points(segment_idx);

            if ( polygon->num_bdy_pt[segment_idx] < 3 ) {
                if (polygon->num_bdy_pt[segment_idx]) {
                    free(polygon->bdy_pt_x[segment_idx]);
                    free(polygon->bdy_pt_y[segment_idx]);
                }
                num_move = polygon->num_segment - segment_idx - 1;
                if (num_move) {
                    memmove((void *) (polygon->num_bdy_pt + segment_idx), (void *) (polygon->num_bdy_pt + segment_idx + 1), num_move*sizeof(int));
                    memmove((void *) (polygon->bdy_pt_x   + segment_idx), (void *) (polygon->bdy_pt_x   + segment_idx + 1), num_move*sizeof(int *));
                    memmove((void *) (polygon->bdy_pt_y   + segment_idx), (void *) (polygon->bdy_pt_y   + segment_idx + 1), num_move*sizeof(int *));
                }
                polygon->num_segment--;
                polygon->num_bdy_pt = (int * ) realloc((void *) polygon->num_bdy_pt, polygon->num_segment*sizeof(int));
                polygon->bdy_pt_x   = (int **) realloc((void *) polygon->bdy_pt_x,   polygon->num_segment*sizeof(int *));
                polygon->bdy_pt_y   = (int **) realloc((void *) polygon->bdy_pt_y,   polygon->num_segment*sizeof(int *));
            }
        }
    }

    return(found);
}
/******************************************************************************************/
/**** FUNCTION: MapLayerClass::fix                                                     ****/
/**** Fix line.                                                                        ****/
/******************************************************************************************/
int MapLayerClass::fix(LineClass *pline, NetworkClass *np, int filter)
{
    int pt_idx, found;

    if (filter) {
        found = 0;
        for (pt_idx=0; (pt_idx<=pline->num_pt-1)&&(!found); pt_idx++) {
            if (   (pline->pt_x[pt_idx] >= 0) && (pline->pt_x[pt_idx] <= np->npts_x-1)
                && (pline->pt_y[pt_idx] >= 0) && (pline->pt_y[pt_idx] <= np->npts_y-1) ) {
                found = 1;
            }
        }
    } else {
        found = 1;
    }

    return(found);
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::remove_duplicate_points                                  ****/
/**** Remove duplicate points from a segment of a polygon.                             ****/
/******************************************************************************************/
void PolygonClass::remove_duplicate_points(int segment_idx)
{
    int i, n, next_i, num_delete;
    int n1, n2, posn, num_move;
    int *pt_list_x, *pt_list_y;

    static int *delete_list = (int *) NULL;
    static int delete_list_allocation = 0;

    if (segment_idx == -1) {
        if (delete_list_allocation) {
            free(delete_list);
            delete_list = (int *) NULL;
            delete_list_allocation = 0;
        }
    } else {
        n = num_bdy_pt[segment_idx];
        pt_list_x = bdy_pt_x[segment_idx];
        pt_list_y = bdy_pt_y[segment_idx];

        if (n > delete_list_allocation) {
            while (n > delete_list_allocation) {
                delete_list_allocation = 2*delete_list_allocation+1;
            }
            delete_list = (int *) realloc((void *) delete_list, delete_list_allocation*sizeof(int));
        }

        num_delete = 0;
        for (i=0; i<=n-1; i++) {
            next_i = (i == n-1 ? 0 : i+1);
            if (   (pt_list_x[i] == pt_list_x[next_i])
                && (pt_list_y[i] == pt_list_y[next_i]) ) {
                delete_list[num_delete] = i;
                num_delete++;
            }
        }
        if (num_delete) {

            posn = delete_list[0];
            for (i=0; i<=num_delete-1; i++) {
                n1 = delete_list[i];
                n2 = (i==num_delete-1 ? n : delete_list[i+1]);
                num_move = n2-n1-1;
                if (num_move) {
                    memmove((void *) (pt_list_x + posn), (void *) (pt_list_x + n1 + 1), num_move*sizeof(int));
                    memmove((void *) (pt_list_y + posn), (void *) (pt_list_y + n1 + 1), num_move*sizeof(int));
                    posn += num_move;
                }
            }

            num_bdy_pt[segment_idx] -= num_delete;
            pt_list_x = (int *) realloc((void *) pt_list_x, num_bdy_pt[segment_idx]*sizeof(int));
            pt_list_y = (int *) realloc((void *) pt_list_y, num_bdy_pt[segment_idx]*sizeof(int));
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: in_map_layer                                                           ****/
/**** Determine whether or not the specified point is covered by the specified map     ****/
/**** layer.                                                                           ****/
/******************************************************************************************/
int NetworkClass::in_map_layer(int map_layer_idx, int posn_x, int posn_y)
{
    int polygon_idx, found;
    MapLayerClass *map_layer;

    map_layer = (*map_layer_list)[map_layer_idx];

    found = 0;

    for (polygon_idx=0; (polygon_idx<=map_layer->num_polygon-1)&&(!found); polygon_idx++) {
        if (map_layer->polygon_list[polygon_idx]->in_bdy_area(posn_x, posn_y)) {
            found = 1;
        }
    }

    return(found);
}
/******************************************************************************************/
/**** FUNCTION: MapLayerClass::shift                                                   ****/
/******************************************************************************************/
void MapLayerClass::shift(int x, int y)
{
    int idx;

    for (idx=0; idx<=num_polygon-1; idx++) {
        polygon_list[idx]->translate(x, y);
    }

    for (idx=0; idx<=num_line-1; idx++) {
        line_list[idx]->translate(x, y);
    }

    for (idx=0; idx<=num_text-1; idx++) {
        posn_x[idx] += x;
        posn_y[idx] += y;
    }
}
/******************************************************************************************/
