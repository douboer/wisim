
/******************************************************************************************
**** PROGRAM: map_background.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef MAP_BACKGROUND_H
#define MAP_BACKGROUND_H

#include <stdlib.h>

class NetworkClass;
class QImage;

/******************************************************************************************/
/**** CLASS: MapBackgroundClass                                                        ****/
/******************************************************************************************/
class MapBackgroundClass
{
public:
    MapBackgroundClass();
    ~MapBackgroundClass();
    void read_map_background(NetworkClass *np, char *filename, char *fposn);
    void shift(int x, int y);

    QImage *mapImage;
    int xmin, xmax, ymin, ymax;
};
/******************************************************************************************/


#endif
