/******************************************************************************************/
/**** FILE: map_layer.h                                                                ****/
/**** Michael Mandell 10/10/03                                                         ****/
/******************************************************************************************/

#ifndef MAP_LAYER_H
#define MAP_LAYER_H

class NetworkClass;
class PolygonClass;
class LineClass;

/******************************************************************************************/
/**** CLASS: MapLayerClass                                                             ****/
/******************************************************************************************/
class MapLayerClass
{
public:
    MapLayerClass();
    ~MapLayerClass();
    void read_map_layer(NetworkClass *np, char *filename, int force_read);
    void read_map_layer_mif(NetworkClass *np, char *filename, char *name, int filter);
    void save(NetworkClass *np, char *filename);
    int fix(PolygonClass *polygon, NetworkClass *np, int filter);
    int fix(LineClass *pline, NetworkClass *np, int filter);
    void shift(int x, int y);

    /* Valid map_layer types are: MAP_LAYER_POLY, MAP_LAYER_LIN */
    int color;
    char *name;

    /* POLYGONS */
    int num_polygon;
    PolygonClass **polygon_list;

    /* LINES */
    int num_line;
    LineClass **line_list;

    /* TEXT */
    int encoding;
    int *posn_x, *posn_y;
    int num_text;
    char **text;

    int modified;

private:
    /**************************************************************************************/
    /**** Constant Definitions                                                         ****/
    /**************************************************************************************/
    enum Encoding{
        EncodingASCII,        /* ASCII                                                    */
        EncodingGB            /* Chinese GB                                               */
    };
    /**************************************************************************************/
};
/******************************************************************************************/

#endif
