/******************************************************************************************/
/**** FILE: map_height.cpp                                                             ****/
/**** Michael Mandell 2/14/04                                                          ****/
/******************************************************************************************/

#include <string.h>
#include <math.h>
#include "cconst.h"
#include "WiSim.h"
#include "antenna.h"
#include "map_height.h"
#include "bin_io.h"
#include "polygon.h"
#include "utm_conversion.h"

/******************************************************************************************/
/**** FUNCTION: MapHeightClass::MapHeightClass                                         ****/
/******************************************************************************************/
MapHeightClass::MapHeightClass()
{
    offset_x          = 0;
    offset_y          = 0;
    npts_x            = 0;
    npts_y            = 0;
    map_sim_res_ratio = -1;
    hstarti           = 0;
    hsize             = 0;
    bytes_per_sample  = 0;
    h_resolution      = 0.0;
    data              = (unsigned char *) NULL;
}
MapHeightClass::~MapHeightClass()
{
    free(data);
}
/******************************************************************************************/
/**** FUNCTION: read_map_height                                                        ****/
/**** Open specified file and read map height data.                                    ****/
/**** Height data is stored in a binary format.                                        ****/
/******************************************************************************************/
void MapHeightClass::read_map_height(NetworkClass *np, char *filename, int force_read)
{
#if (DEMO == 0)
    unsigned int res_a, res_b;
    unsigned char *c;
    int i, j;
    int map_startx, map_starty, map_npts_x, map_npts_y;
    int xidx, yidx;
    int has_geometry;
    int utm_zone_mismatch = 0;
    int utm_zone, utm_north;
    double resolution;
    char *coord_sys_str, *str1, *chptr;
    double lon_deg, lat_deg, utm_x, utm_y;
    FILE *fp;

    unsigned int h_res_a, h_res_b, bit_width, bps;

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(np->msg, "ERROR: Unable to read from file 4 %s\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    if (!np->system_bdy) {
        // sprintf( np->msg, "ERROR: Geometry not defined\n");
        // PRMSG(stdout, np->msg); np->error_state = 1;
        // return;
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    sprintf(np->msg, "Reading map height file: %s\n", filename);
    PRMSG(stdout, np->msg);

    if (force_read) {
        sprintf(np->msg, "WARNING: map_height file \"%s\" COORDINATE_SYSTEM does not match\n", filename);
    } else {
        sprintf(np->msg, "ERROR: map_height file \"%s\" COORDINATE_SYSTEM does not match\n", filename);
    }

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
    }

    res_a = read_fs_uint(fp);
    res_b = read_fs_uint(fp);
    resolution = (double) res_a + ((double) res_b)/(1<<16)/(1<<16);
    if (!has_geometry) {
        np->resolution = resolution;
        np->res_digits = ( (np->resolution < 1.0) ? ((int) -floor(log(np->resolution)/log(10.0))) : 0 );
    }
    if (!check_grid_val(resolution, np->resolution, 0, &map_sim_res_ratio)) {
        sprintf(np->msg, "ERROR: invalid map_height file \"%s\"\n"
                         "ERROR: height file resolution is not integer multiple of simulation resolution\n"
                         "Height resolution = %15.10f simulation resolution = %15.10f\n",
                filename, resolution, np->resolution);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "Height x-y resolution %15.10f\n", resolution);
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
        sprintf(np->msg, "ERROR: invalid map_height file \"%s\"\n"
                         "Simulation and map grids are not properly alligned\n", filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }
#endif

    h_res_a = read_fs_uint(fp);
    h_res_b = read_fs_uint(fp);
    h_resolution = (double) h_res_a + ((double) h_res_b)/(1<<16)/(1<<16);

    sprintf(np->msg, "Height h_resolution %15.10f\n", h_resolution);
    PRMSG(stdout, np->msg);

    hstarti = read_fs_int(fp);
    hsize   = read_fs_int(fp);

    BITWIDTH(bit_width, hsize-1);
    bytes_per_sample = 1 + (bit_width-1)/8;
    bps = bytes_per_sample;
    c = (unsigned char *) malloc(bps);

    offset_x = map_startx * map_sim_res_ratio - np->system_startx;
    offset_y = map_starty * map_sim_res_ratio - np->system_starty;
    npts_x   = map_npts_x;
    npts_y   = map_npts_y;

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

            if (fread(c, bps, 1, fp) != 1) {
                sprintf(np->msg, "ERROR: invalid map_height file \"%s\"\n", filename);
                PRMSG(stdout, np->msg); np->error_state = 1;
                return;
            }

            if ( (0 <= i) && (i <= npts_x-1) && (0 <= j) && (j <= npts_y-1) ) {
                memcpy(&(data[(npts_x*j+i)*bps]), c, bps);
            }
        }
    }

    free(c);

    fclose(fp);

#if 0
    for (i=0; i<=npts_x*npts_y*bps-1; i++) {
        printf("Index = %4d Data = 0x%X = %u\n", i, data[i], data[i]);
    }
#endif

    free(coord_sys_str);

#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: MapHeightClass::translate                                              ****/
/******************************************************************************************/
void MapHeightClass::translate(int x, int y)
{
    offset_x += x;
    offset_y += y;
}
/******************************************************************************************/
