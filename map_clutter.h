
/*******************************************************************************************
**** PROGRAM: map_clutter.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
/******************************************************************************************/
/**** FILE: map_clutter.h                                                              ****/
/**** Michael Mandell 2/14/04                                                          ****/
/******************************************************************************************/

#ifndef MAP_CLUTTER_H
#define MAP_CLUTTER_H

#include <stdlib.h>

class NetworkClass;
class IntIntClass;
class PolygonClass;

template<class T> class ListClass;

/******************************************************************************************/
/**** CLASS: MapClutterClass                                                           ****/
/******************************************************************************************/
class MapClutterClass
{
public:
    MapClutterClass();
    ~MapClutterClass();
    void read_map_clutter(NetworkClass *np, char *filename, int force_read = 0);
    void read_map_clutter_1_0(FILE *fp, NetworkClass *np, char *filename, int has_geometry, int force_read);
    void save_map_clutter(NetworkClass *np, char *filename);
    void create_clutter_map(NetworkClass *np, int map_sim_res_ratio, int num_clutter_type);
    void set_clutter_type(int clutter_type_idx, ListClass<IntIntClass> *ii_list);
    void set_init_clutter_type();
    void translate(int x, int y);
    int get_clutter_type(int map_i, int map_j);
    PolygonClass *create_map_bdy();
    void split(NetworkClass *np);

    int offset_x, offset_y, npts_x, npts_y;
    int num_clutter_type;
    int map_sim_res_ratio;
    char **description;
    int *color;
    int bps; /* Bytes per sample */

    friend class sectorparamDia;
    friend class ClutterPropModelClass;
    friend class NetworkClass;

private:
    unsigned char *data;
};
/******************************************************************************************/

#endif
