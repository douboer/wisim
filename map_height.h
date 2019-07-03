/******************************************************************************************/
/**** FILE: map_height.h                                                               ****/
/**** Michael Mandell 2/14/04                                                          ****/
/******************************************************************************************/

#ifndef MAP_HEIGHT_H
#define MAP_HEIGHT_H

#include <stdlib.h>

class NetworkClass;


/******************************************************************************************/
/**** CLASS: MapHeightClass                                                            ****/
/******************************************************************************************/
class MapHeightClass
{
public:
    MapHeightClass();
    ~MapHeightClass();
    void read_map_height(NetworkClass *np, char *filename, int force_read = 0);
    void translate(int x, int y);

    int offset_x, offset_y, npts_x, npts_y;
    int hstarti, hsize, bytes_per_sample;
    int map_sim_res_ratio;
    double h_resolution;
    unsigned char *data;
};
/******************************************************************************************/

#endif
