
/*******************************************************************************************
**** PROGRAM: map_clutter.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
/******************************************************************************************/
/**** FILE: map_clutter.cpp                                                            ****/
/**** Michael Mandell 2/14/04                                                          ****/
/******************************************************************************************/

#include <string.h>
#include <math.h>

#include "antenna.h"
#include "bin_io.h"
#include "cconst.h"
#include "wisim.h"
#include "hot_color.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "polygon.h"
#include "utm_conversion.h"

#define MAP_CLUTTER_FORMAT "1.0"
static char *header = "WISIM_CLUTTER_FILE";

/******************************************************************************************/
/**** FUNCTION: MapClutterClass::MapClutterClass                                       ****/
/******************************************************************************************/
MapClutterClass::MapClutterClass()
{
    num_clutter_type  = 0;
    offset_x          = 0;
    offset_y          = 0;
    npts_x            = 0;
    npts_y            = 0;
    map_sim_res_ratio = -1;
    description       = (char **) NULL;
    color             = (int *) NULL;
    data              = (unsigned char *) NULL;
}
MapClutterClass::~MapClutterClass()
{
    int clutter_type_idx;

    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        free(description[clutter_type_idx]);
    }
    free(description);
    free(color);
    free(data);
}
/******************************************************************************************/
/**** FUNCTION: read_map_clutter                                                       ****/
/**** Open specified file and read map clutter data.                                   ****/
/**** Clutter data is stored in a binary format.                                       ****/
/******************************************************************************************/
void MapClutterClass::read_map_clutter(NetworkClass *np, char *filename, int force_read)
{
#if (DEMO == 0)
    int has_geometry;
    char *format_str;
    FILE *fp;

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(np->msg, "ERROR: Unable to read from file 3 %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    if (!np->system_bdy) {
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    char *read_header = CVECTOR(strlen(header));
    if (fread(read_header, sizeof(char), strlen(header), fp) != strlen(header)) {
        sprintf(np->msg, "ERROR: invalid map_clutter file \"%s\", file header does not match\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1; fclose(fp);
        return;
    }
    read_header[strlen(header)] = (char) NULL;
    if (strcmp(read_header, header) != 0) {
        sprintf(np->msg, "ERROR: invalid map_clutter file \"%s\", file header does not match\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1; fclose(fp);
        return;
    }

    format_str = read_fs_str_allocate(fp);

    if (strcmp(format_str,"1.0")==0) {
        read_map_clutter_1_0(fp, np, filename, has_geometry, force_read);
    } else {
        sprintf(np->msg, "ERROR: map_clutter file \"%s\" has invalid format \"%s\"\n", filename, format_str);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

    free(read_header);
    free(format_str);

    fclose(fp);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: read_map_clutter_1_0                                                   ****/
/**** Read map clutter file with format 1.0                                            ****/
/**** Clutter data is stored in a binary format.                                       ****/
/******************************************************************************************/
void MapClutterClass::read_map_clutter_1_0(FILE *fp, NetworkClass *np, char *filename, int has_geometry, int force_read)
{
#if (DEMO == 0)
    unsigned int res_a, res_b;
    unsigned char c;
    int i, j, k;
    int map_startx, map_starty, map_npts_x, map_npts_y;
    int xidx, yidx;
    int utm_zone_mismatch = 0;
    int utm_zone, utm_north;
    double resolution;
    char *coord_sys_str, *str1, *chptr;
    double lon_deg, lat_deg, utm_x, utm_y;

    int color_rrr, color_ggg, color_bbb;
    int clutter_type_idx;

    sprintf(np->msg, "Reading map clutter file: %s\n", filename);
    PRMSG(stdout, np->msg);

    sprintf(np->msg, "%s: map_clutter file \"%s\" COORDINATE_SYSTEM does not match\n", (force_read ? "WARNING" : "ERROR"), filename);

    coord_sys_str = read_fs_str_allocate(fp);
    str1 = coord_sys_str;
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
    } else {
        sprintf(np->msg, "ERROR: invalid map_clutter file \"%s\" unrecognized coordinate system \"%s\"\n",
                filename, coord_sys_str);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    res_a = read_fs_uint(fp);
    res_b = read_fs_uint(fp);
    resolution = (double) res_a + ((double) res_b)/(1<<16)/(1<<16);
    if (!has_geometry) {
        np->resolution = resolution;
        np->res_digits = ( (np->resolution < 1.0) ? ((int) -floor(log(np->resolution)/log(10.0))) : 0 );
    }
    if (!check_grid_val(resolution, np->resolution, 0, &map_sim_res_ratio)) {
        sprintf(np->msg, "ERROR: invalid map_clutter file \"%s\"\n"
                         "ERROR: clutter file resolution is not integer multiple of simulation resolution\n"
                         "Clutter resolution = %15.10f simulation resolution = %15.10f\n",
                filename, resolution, np->resolution);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "Clutter resolution %15.10f\n", resolution);
    chptr += sprintf(chptr, "Simulation resolution %15.10f\n", np->resolution);
    chptr += sprintf(chptr, "Ratio: %d\n", map_sim_res_ratio);
    PRMSG(stdout, np->msg);

    map_startx = read_fs_int(fp);
    map_starty = read_fs_int(fp);

    if (utm_zone_mismatch) {
        UTMtoLL( map_startx*resolution, map_starty*resolution, lon_deg,  lat_deg, utm_zone, utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);
        LLtoUTM( lon_deg, lat_deg, utm_x,  utm_y, np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);
        check_grid_val(utm_x, resolution, 0, &map_startx);
        check_grid_val(utm_y, resolution, 0, &map_starty);
    }

    map_npts_x = read_fs_int(fp);
    map_npts_y = read_fs_int(fp);

    if (!has_geometry) {
        np->system_bdy = new PolygonClass();
        np->system_bdy->num_segment = 1;
        np->system_bdy->num_bdy_pt = IVECTOR(np->system_bdy->num_segment);
        np->system_bdy->bdy_pt_x   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->bdy_pt_y   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->num_bdy_pt[0] = 4;
        np->system_bdy->bdy_pt_x[0] = IVECTOR(4);
        np->system_bdy->bdy_pt_y[0] = IVECTOR(4);
        np->system_bdy->bdy_pt_x[0][0] = 0;
        np->system_bdy->bdy_pt_y[0][0] = 0;
        np->system_bdy->bdy_pt_x[0][1] = map_npts_x-1;
        np->system_bdy->bdy_pt_y[0][1] = 0;
        np->system_bdy->bdy_pt_x[0][2] = map_npts_x-1;
        np->system_bdy->bdy_pt_y[0][2] = map_npts_y-1;
        np->system_bdy->bdy_pt_x[0][3] = 0;
        np->system_bdy->bdy_pt_y[0][3] = map_npts_y-1;
        np->system_startx = map_startx;
        np->system_starty = map_starty;
        np->npts_x        = map_npts_x;
        np->npts_y        = map_npts_y;
        np->num_antenna_type = 1;
        np->antenna_type_list = (AntennaClass **) malloc(np->num_antenna_type*sizeof(AntennaClass *));
        np->antenna_type_list[0] = new AntennaClass(CConst::antennaOmni, "OMNI");
    }

#if 0
    if (   (np->system_startx % map_sim_res_ratio != 0)
        || (np->system_starty % map_sim_res_ratio != 0)
        || (np->npts_x        % map_sim_res_ratio != 0)
        || (np->npts_y        % map_sim_res_ratio != 0) ) {
        sprintf(np->msg, "ERROR: invalid map_clutter file \"%s\"\n"
                         "Simulation and map grids are not properly alligned\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }
#endif

    offset_x = map_startx * map_sim_res_ratio - np->system_startx;
    offset_y = map_starty * map_sim_res_ratio - np->system_starty;
    npts_x   = map_npts_x;
    npts_y   = map_npts_y;

    num_clutter_type = read_fs_int(fp);
    sprintf(np->msg, "Num Clutter Types = %d\n", num_clutter_type);
    PRMSG(stdout, np->msg);

    description = (char **) malloc((num_clutter_type)*sizeof(char *));
    color = IVECTOR(num_clutter_type);

    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        description[clutter_type_idx] = read_fs_str_allocate(fp);
        color_rrr = (int) read_fs_uchar(fp);
        color_ggg = (int) read_fs_uchar(fp);
        color_bbb = (int) read_fs_uchar(fp);
        color[clutter_type_idx] = (color_rrr << 16) | (color_ggg << 8) | color_bbb;
    }

    sprintf(np->msg, "INDEX    %-66s    RRR GGG BBB\n", "DESCRIPTION");
    PRMSG(stdout, np->msg);
    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        chptr = np->msg;
        chptr += sprintf(chptr, "%3d  ", clutter_type_idx);
        chptr += sprintf(chptr, "    ");
        chptr += sprintf(chptr, "%-66s", description[clutter_type_idx]);
        chptr += sprintf(chptr, "    ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx] >> 16)&255);
        chptr += sprintf(chptr, " ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx] >> 8)&255);
        chptr += sprintf(chptr, " ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx])&255);
        chptr += sprintf(chptr, "\n");
        PRMSG(stdout, np->msg);
    }
    strcpy(np->msg, "\n");
    PRMSG(stdout, np->msg);

    bps = (int) ceil( log(num_clutter_type+1-0.5) / log(256.0) );

    sprintf(np->msg, "Bytes Per Sample = %d\n", bps);
    PRMSG(stdout, np->msg);

    if ( !(data = (unsigned char *) malloc(npts_x*npts_y*bps*sizeof(unsigned char))) ) {
        sprintf(np->msg, "ERROR: Insufficient memory to read mapdata file %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }
    memset(data, 255, npts_x*npts_y*bps);

    for (yidx=0; yidx<=map_npts_y-1; yidx++) {
        j = yidx + map_starty - np->system_starty/map_sim_res_ratio - offset_y/map_sim_res_ratio;
        for (xidx=0; xidx<=map_npts_x-1; xidx++) {
            i = xidx + map_startx - np->system_startx/map_sim_res_ratio - offset_x/map_sim_res_ratio;
            for (k=0; k<=bps-1; k++) {
                c = read_fs_uchar(fp);
                if ( (0 <= i) && (i <= npts_x-1) && (0 <= j) && (j <= npts_y-1) ) {
                    data[(npts_x*j+i)*bps+k] = c;
                }
            }
        }
    }

    free(coord_sys_str);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: save_map_clutter                                                       ****/
/**** Save map clutter data to the specified file.                                     ****/
/**** Clutter data is stored in a binary format.                                       ****/
/******************************************************************************************/
void MapClutterClass::save_map_clutter(NetworkClass *np, char *filename)
{
#if (DEMO == 0)
    unsigned int res_a, res_b;
    int map_startx, map_starty;
    double resolution;
    char *chptr;
    FILE *fp;

    unsigned char color_rrr, color_ggg, color_bbb;
    int clutter_type_idx;

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
    write_fs_str(fp, MAP_CLUTTER_FORMAT);
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
    resolution = np->resolution * map_sim_res_ratio;
    res_a = (int) floor(resolution);
    res_b = (int) floor( (resolution - res_a)*(1<<16)*(1<<16) );
    write_fs_int(fp, res_a);
    write_fs_int(fp, res_b);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** XSTART, YSTART, XSIZE, YSIZE                                                 ****/
    /**************************************************************************************/
    map_startx = (np->system_startx + offset_x) / map_sim_res_ratio;
    map_starty = (np->system_starty + offset_y) / map_sim_res_ratio;

    write_fs_int(fp, map_startx);
    write_fs_int(fp, map_starty);

    write_fs_int(fp, npts_x);
    write_fs_int(fp, npts_y);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** NUM CLUTTER TYPE                                                             ****/
    /**************************************************************************************/
    write_fs_int(fp, num_clutter_type);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** DESCRIPTION & COLOR FOR EACH CLUTTER TYPE                                    ****/
    /**************************************************************************************/
    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        write_fs_str(fp, description[clutter_type_idx]);

        color_rrr = (unsigned char) ( (color[clutter_type_idx]>>16) & 0xFF);
        color_ggg = (unsigned char) ( (color[clutter_type_idx]>> 8) & 0xFF);
        color_bbb = (unsigned char) ( (color[clutter_type_idx]    ) & 0xFF);

        write_fs_uchar(fp, color_rrr);
        write_fs_uchar(fp, color_ggg);
        write_fs_uchar(fp, color_bbb);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** CLUTTER DATA                                                                 ****/
    /**************************************************************************************/
    bps = (int) ceil( log(num_clutter_type+1-0.5) / log(256.0) );

    sprintf(np->msg, "Bytes Per Sample = %d\n", bps);
    PRMSG(stdout, np->msg);

    if ((int) fwrite((void *) data, sizeof(char), npts_x*npts_y*bps, fp) != npts_x*npts_y*bps) {
        sprintf(np->msg, "ERROR: Clutter data not successfully written to file \"%s\"\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
    }
    /**************************************************************************************/

    fclose(fp);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: create_clutter_map                                                     ****/
/******************************************************************************************/
void MapClutterClass::create_clutter_map(NetworkClass *np, int p_map_sim_res_ratio, int p_num_clutter_type)
{
#if (DEMO == 0)
    int clutter_type_idx;
    int tt_idx;
    int adjust_sx, adjust_sy;
    int adjust_nx, adjust_ny;
    char *tmpstr, *chptr;

    if (!np->system_bdy) {
        sprintf( np->msg, "ERROR: Geometry not defined\n");
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    for (tt_idx=0; tt_idx<=np->num_traffic_type-1; tt_idx++) {
        if (np->num_subnet[tt_idx]) {
            sprintf(np->msg, "ERROR: Unable to redefine system boundary for geometry that has subnets\n");
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        }
    }

    sprintf(np->msg, "Creating clutter map\n");
    PRMSG(stdout, np->msg);

    tmpstr = CVECTOR(200);

    map_sim_res_ratio = p_map_sim_res_ratio;

    adjust_sx = MOD(np->system_startx, map_sim_res_ratio);
    adjust_sy = MOD(np->system_starty, map_sim_res_ratio);

    adjust_nx = adjust_sx + map_sim_res_ratio-1 - MOD(np->npts_x+adjust_sx-1, map_sim_res_ratio);
    adjust_ny = adjust_sy + map_sim_res_ratio-1 - MOD(np->npts_y+adjust_sy-1, map_sim_res_ratio);

    np->adjust_coord_system(np->system_startx-adjust_sx, np->system_starty-adjust_sy, 
                            np->npts_x+adjust_nx,        np->npts_y+adjust_ny); 

    if (p_num_clutter_type == -1) {
        num_clutter_type = (np->npts_x / map_sim_res_ratio) * (np->npts_y / map_sim_res_ratio);
        printf("NUM_CLUTTER_TYPE SET TO: %d\n", num_clutter_type);
    } else {
        num_clutter_type = p_num_clutter_type;
    }

    description = (char **) malloc((num_clutter_type)*sizeof(char *));
    color = IVECTOR(num_clutter_type);

    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        sprintf(tmpstr, "clutter_type_%d", clutter_type_idx);
        description[clutter_type_idx] = strdup(tmpstr);
        // color[clutter_type_idx] = np->default_color_list[clutter_type_idx % np->num_default_color];
        color[clutter_type_idx] = np->hot_color->get_color(clutter_type_idx, num_clutter_type);

    }

    sprintf(np->msg, "INDEX    %-66s    RRR GGG BBB\n", "DESCRIPTION");
    PRMSG(stdout, np->msg);
    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        chptr = np->msg;
        chptr += sprintf(chptr, "%3d  ", clutter_type_idx);
        chptr += sprintf(chptr, "    ");
        chptr += sprintf(chptr, "%-66s", description[clutter_type_idx]);
        chptr += sprintf(chptr, "    ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx] >> 16)&255);
        chptr += sprintf(chptr, " ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx] >> 8)&255);
        chptr += sprintf(chptr, " ");
        chptr += sprintf(chptr, "%3d", (color[clutter_type_idx])&255);
        chptr += sprintf(chptr, "\n");
        PRMSG(stdout, np->msg);
    }
    strcpy(np->msg, "\n");
    PRMSG(stdout, np->msg);

    bps = (int) ceil( log(num_clutter_type+1-0.5) / log(256.0) );
    npts_x = np->npts_x/map_sim_res_ratio;
    npts_y = np->npts_y/map_sim_res_ratio;
    if ( !(data = (unsigned char *) malloc(npts_x*npts_y*bps*sizeof(unsigned char))) ) {
        sprintf(np->msg, "ERROR: Insufficient memory to create clutter map\n");
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }
    memset(data, 0, npts_x*npts_y*bps);

    free(tmpstr);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: set_clutter_type                                                       ****/
/******************************************************************************************/
void MapClutterClass::set_clutter_type(int clutter_type_idx, ListClass<IntIntClass> *ii_list)
{
#if (DEMO == 0)
    int i, j, k;
    int minx, maxx, miny, maxy;

    PolygonClass *p;

    p = new PolygonClass(ii_list);

    p->comp_bdy_min_max(minx, maxx, miny, maxy);

    if (minx < offset_x) { minx = offset_x; }
    if (miny < offset_y) { miny = offset_y; }
    if (maxx > offset_x + map_sim_res_ratio*npts_x-1) { maxx = offset_x + map_sim_res_ratio*npts_x-1; }
    if (maxy > offset_y + map_sim_res_ratio*npts_y-1) { maxy = offset_y + map_sim_res_ratio*npts_y-1; }

    minx -= MOD(minx-offset_x, map_sim_res_ratio);
    miny -= MOD(miny-offset_y, map_sim_res_ratio);
    maxx -= MOD(maxx-offset_x, map_sim_res_ratio);
    maxy -= MOD(maxy-offset_y, map_sim_res_ratio);

    for (i=minx; i<=maxx; i+=map_sim_res_ratio) {
        for (j=miny; j<=maxy; j+=map_sim_res_ratio) {
            if (p->in_bdy_area(i, j)) {
                printf("Setting Clutter Type %d for point: %d %d\n", clutter_type_idx, i, j);
                for (k=0; k<=bps-1; k++) {
                    data[(npts_x*((j-offset_y)/map_sim_res_ratio)+((i-offset_x)/map_sim_res_ratio))*bps+k]
                        = (unsigned char) ( (clutter_type_idx >> (8*(bps-1-k))) & 255 );
                }
            }
        }
    }

    delete p;

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: set_init_clutter_type                                                  ****/
/******************************************************************************************/
void MapClutterClass::set_init_clutter_type()
{
#if (DEMO == 0)
    int i, j, k, clutter_type_idx;

    clutter_type_idx = 0;
    for (j=0; j<=npts_y-1; j++) {
        for (i=0; i<=npts_x-1; i++) {
            for (k=0; k<=bps-1; k++) {
                data[(npts_x*(npts_y-1-j)+i)*bps+k] = (unsigned char) ( (clutter_type_idx >> (8*(bps-1-k))) & 255 );
            }
            clutter_type_idx = (clutter_type_idx + 1) % num_clutter_type;
        }
    }

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: MapClutterClass::translate                                             ****/
/******************************************************************************************/
void MapClutterClass::translate(int x, int y)
{
    offset_x += x;
    offset_y += y;
}
/******************************************************************************************/
/**** FUNCTION: MapClutterClass::get_clutter_type                                      ****/
/******************************************************************************************/
int MapClutterClass::get_clutter_type(int map_i, int map_j)
{
    int k, clutter_type;

    if ( (map_i>=0) && (map_i<=npts_x-1) && (map_j>=0) && (map_j<=npts_y-1) ) {
        clutter_type = 0;
        for (k=0; k<=bps-1; k++) {
            clutter_type = (clutter_type<<8) | ((int) data[(npts_x*(npts_y-1-map_j) + map_i)*bps + k]);
        }
    } else {
        clutter_type = -1;
    }

    return(clutter_type);
}
/******************************************************************************************/
/**** FUNCTION: MapClutterClass::create_map_bdy                                        ****/
/******************************************************************************************/
PolygonClass *MapClutterClass::create_map_bdy()
{
    ListClass<IntIntClass> *ii_list = new ListClass<IntIntClass>(4);

    ii_list->append(IntIntClass(offset_x-1,                          offset_y-1));
    ii_list->append(IntIntClass(offset_x + map_sim_res_ratio*npts_x, offset_y-1));
    ii_list->append(IntIntClass(offset_x + map_sim_res_ratio*npts_x, offset_y + map_sim_res_ratio*npts_y));
    ii_list->append(IntIntClass(offset_x-1,                          offset_y + map_sim_res_ratio*npts_y));

    PolygonClass *map_bdy = new PolygonClass(ii_list);
    delete ii_list;

    return(map_bdy);
}
/******************************************************************************************/
/**** FUNCTION: MapClutterClass::split                                                 ****/
/******************************************************************************************/
void MapClutterClass::split(NetworkClass *np)
{
    int clutter_type_idx;
    int *old_color, old_num_clutter_type, old_clutter_type_idx;
    int old_i, old_j, i, j;
    char *tmpstr = CVECTOR(200);

    if ( (map_sim_res_ratio&1) == 1 ) {
        sprintf(np->msg, "ERROR: map_sim_res_ratio = %d, value must be even.\n", map_sim_res_ratio);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }


    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        free(description[clutter_type_idx]);
    }
    free(description);

    map_sim_res_ratio /= 2;
    old_num_clutter_type = num_clutter_type;

    npts_x *= 2;
    npts_y *= 2;

    num_clutter_type = npts_x * npts_y;

    bps = (int) ceil( log(num_clutter_type+1-0.5) / log(256.0) );

    description = (char **) malloc((num_clutter_type)*sizeof(char *));
    old_color = color;
    color = IVECTOR(num_clutter_type);

    for (clutter_type_idx=0; clutter_type_idx<=num_clutter_type-1; clutter_type_idx++) {
        sprintf(tmpstr, "clutter_type_%d", clutter_type_idx);
        description[clutter_type_idx] = strdup(tmpstr);

        i = clutter_type_idx % npts_x;
        j = clutter_type_idx / npts_x;

        old_i = i / 2;
        old_j = j / 2;
        old_clutter_type_idx = old_j*(npts_x/2) + old_i;
        if (old_clutter_type_idx <= old_num_clutter_type-1) {
            color[clutter_type_idx] = old_color[old_clutter_type_idx];
        } else {
            color[clutter_type_idx] = 0x101010;
        }
    }

    if ( !(data = (unsigned char *) realloc(data, npts_x*npts_y*bps*sizeof(unsigned char))) ) {
        sprintf(np->msg, "ERROR: Insufficient memory to split clutter map\n");
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    set_init_clutter_type();

    free(old_color);
    free(tmpstr);

    return;
}
/******************************************************************************************/
