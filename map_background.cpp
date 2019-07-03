/******************************************************************************************/
/**** FILE: map_background.cpp                                                         ****/
/**** Michael Mandell 2/24/04                                                          ****/
/******************************************************************************************/

#include <qstring.h>
#include <qimage.h>
#include <qlineedit.h>
#include <q3filedialog.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "antenna.h"
#include "cconst.h"
#include "WiSim.h"
#include "map_background.h"
#include "map_layer.h"
#include "polygon.h"
#include "utm_conversion.h"

#define size 100
#define MAP_DEBUG 0
#define DELIM " \"\t\n\\(),!#"

bool isNumeric(QString str);

/******************************************************************************************/
/**** FUNCTION: MapBackgroundClass::MapBackgroundClass                                 ****/
/******************************************************************************************/
MapBackgroundClass::MapBackgroundClass()
{
    mapImage = (QImage *) NULL;
}

MapBackgroundClass::~MapBackgroundClass()
{
    if (mapImage) {
        delete mapImage;
    }
}
/******************************************************************************************/
/**** FUNCTION: read_map_backgound                                                     ****/
/******************************************************************************************/
void MapBackgroundClass::read_map_background(NetworkClass *np, char *filename, char *fposn)
{
#if (DEMO == 0)
    int has_geometry;
    int first_point = 1;
    // char* str = (char* ) malloc ( (strlen(fposn)+5) * sizeof (char) );
    char* str = (char *) NULL;
    std::string tmp( fposn ); 
    std::string::size_type pos;

    if (!np->system_bdy) {
        has_geometry = 0;
    } else {
        has_geometry = 1;
    }

    if ( filename == NULL ) {
        pos = tmp.rfind('.');
        if ( pos == std::string::npos ) {
            sprintf( np->msg, "ERROR: no file extend name found \n" );
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        } else {
            tmp.replace( pos, tmp.capacity(), ".jpg" );
            str = strdup((char*) tmp.c_str());
        }
    } else {
        str = strdup(filename);
    }

    mapImage = new QImage;
    if ( ! mapImage->load( QString::fromLocal8Bit( str ), 0 ) ) {
        sprintf( np->msg, "ERROR: cannot open image file \"%s\"\n", str );
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }
    free ( str );

    int p_minx = 0;
    int p_miny = 0;
    int p_maxx = mapImage->width();
    int p_maxy = mapImage->height();

    int xmin_t, xmax_t, ymin_t, ymax_t;
    double utm_minx, utm_miny, utm_maxx, utm_maxy;

    int s_px = 0;
    int s_py = 0;
    int s_pxx = 0;
    int s_pyy = 0;
    double s_utmx = 0.0;
    double s_utmy = 0.0;
    double s_px_utmx = 0.0;
    double s_py_utmy = 0.0;

#if MAP_DEBUG
    double s_ppx = 0.0;
    double s_uux = 0.0;
    double s_ppy = 0.0;
    double s_uuy = 0.0;
    double s_ppppx = 0.0;
    double s_ppppy = 0.0;
#endif

    std::vector <double> utm_x;
    std::vector <double> utm_y;
    double utm_xi, utm_yi;

    double slop_t;
    double interc_t;
    std::vector <double> slop;
    std::vector <double> interc;

    /*
    char* lon;
    char* lat;
    char* px;
    char* py;
    */
    QString lon;
    QString lat;
    QString px;
    QString py;

    int  record_number = 0;
    bool isUtm         = false;
    //bool valid       = false;

    std::vector <double> f_lon;
    std::vector <double> f_lat;
    std::vector <int> f_px;
    std::vector <int> f_py;

    char buff[size];
    
    std::ifstream file_in( fposn );
    if( !file_in )
    {
        sprintf(np->msg, "ERROR: cannot open map position file \"%s\"\n", fposn);
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }

    while( file_in.getline(buff, size, '\n'))
    {
        if ((lon = strtok( buff, DELIM )) != (char*)NULL )
        {

            std::cout << "lon = " << lon.toStdString() << "\n";
            std::cout << "test isNum = " << isNumeric(lon) << "\n";
            /*
            std::cout << "test isNum = " << isNumeric("xxx") << "\n";
            std::cout << "test isNum = " << isNumeric("23") << "\n";
            std::cout << "test isNum = " << isNumeric("-23") << "\n";
            std::cout << "test isNum = " << isNumeric("-23.") << "\n";
            std::cout << "test isNum = " << isNumeric("-23.4") << "\n";
            */


            lat = strtok(NULL, DELIM);
            px  = strtok(NULL, DELIM);
            py  = strtok(NULL, DELIM);

            if ( !lon.isNull() && !lon.isEmpty() && !lat.isNull() && lat.isEmpty() )
                if ( lon ==  QString("CoordSys") && lat == QString("UTM") ) 
                { isUtm = true; }

            if ( isNumeric(lon)  && isNumeric(lat)
                    && isNumeric(px) &&  isNumeric(py) )
            {

#if 0
                std::cout << lon.toStdString()   << "   "
                          << lat   << "   "
                          << px    << "   "
                          << py    << "   "
                          << std::endl;
#endif

                f_lon.push_back( lon.toDouble() );    // <=>atof( lon );
                f_lat.push_back( lat.toDouble() );    // <=>atof( lat );
                f_px.push_back( px.toInt() );         // <=>atoi( px );
                f_py.push_back( py.toInt() );         // <=>atoi( py );
                
                record_number++;
            }
        }
    }
    if ( record_number == 2 ) 
        if ( f_px[0] == f_px[1] ) f_px[1] += 1;
        else if ( f_py[0] == f_py[1] ) f_py[1] += 1;

    if (!has_geometry) {
        np->coordinate_system = CConst::CoordUTM;
        np->utm_equatorial_radius = 6380725.0;
        np->utm_eccentricity_sq   = 0.006681000;
        np->resolution            = 1.0;
    }

    for (int i=0; i< record_number; i++)
    {
        if ( isUtm ) {
            utm_x.push_back( f_lon[i] );
            utm_y.push_back( f_lat[i] );
            utm_xi = f_lon[i];
            utm_yi = f_lat[i];
        } else {
            if ((!has_geometry) && (first_point)) {
                    getUTMZone(np->utm_zone, np->utm_north, f_lon[i], f_lat[i]);
                    first_point = 0;
            }
            LLtoUTM( f_lon[i], f_lat[i], utm_xi, utm_yi,
                    np->utm_zone, np->utm_north, np->utm_equatorial_radius,
                    np->utm_eccentricity_sq);
            utm_x.push_back( utm_xi );
            utm_y.push_back( utm_yi );
        }

        s_px   = s_px + f_px[i];
        s_py   = s_py + f_py[i];

        s_utmx = s_utmx + utm_xi;
        s_utmy = s_utmy + utm_yi;

        s_px_utmx = s_px_utmx + f_px[i]*utm_xi;
        s_py_utmy = s_py_utmy + f_py[i]*utm_yi;

        s_pxx = s_pxx + f_px[i]*f_px[i];
        s_pyy = s_pyy + f_py[i]*f_py[i];

#if MAP_DEBUG
        std::cout << "Label Pt" << i << "   ";
        std::cout << f_lon[i]   << "   "
                  << f_lat[i]   << "   "
                  << f_px[i]    << "   "
                  << f_py[i]    << "   "
                  << std::endl  << std::endl;
#endif
    }

#if MAP_DEBUG
    for (int i=0; i< record_number; i++)
    {
        s_ppx = s_ppx + f_px[i] - s_px/record_number;
        s_uux = s_uux + utm_x[i] - s_utmx/record_number;
        s_ppy = s_ppy + f_py[i] - s_py/record_number;
        s_uuy = s_uuy + utm_y[i] - s_utmy/record_number;
        s_ppppx = s_ppppx + s_ppx*s_ppx;
        s_ppppy = s_ppppy + s_ppy*s_ppy;
    }

    slop_t = s_ppx*s_uux;
    interc_t = ( s_utmx-slop_t*s_px )/record_number;
    slop.push_back( slop_t );
    interc.push_back( interc_t );

    slop_t = s_ppy*s_uuy;
    interc_t = ( s_utmy-slop_t*s_py )/record_number;
    slop.push_back( slop_t );
    interc.push_back( interc_t );
#endif

    slop_t = ( record_number*(s_px_utmx)-s_px*s_utmx )/
                ( record_number*s_pxx-s_px*s_px );
    interc_t = ( s_utmx-slop_t*s_px )/record_number;
    slop.push_back( slop_t );
    interc.push_back( interc_t );

    slop_t = ( record_number*(s_py_utmy)-s_py*s_utmy )/
                ( record_number*s_pyy-s_py*s_py );
    interc_t = ( s_utmy-slop_t*s_py )/record_number;
    slop.push_back( slop_t );
    interc.push_back( interc_t );

    utm_minx = slop[0]*p_minx+interc[0];
    utm_maxx = slop[0]*p_maxx+interc[0];
    utm_miny = slop[1]*p_maxy+interc[1];
    utm_maxy = slop[1]*p_miny+interc[1];

    if (!has_geometry) {
        check_grid_val(utm_minx, np->resolution, 0, &(np->system_startx));
        check_grid_val(utm_miny, np->resolution, 0, &(np->system_starty));
    }


//  std::cout << std::endl << "test record number : " << record_number << std::endl;
    check_grid_val( utm_minx, np->resolution, np->system_startx, &xmin_t);
    check_grid_val( utm_miny, np->resolution, np->system_starty, &ymin_t);
    check_grid_val( utm_maxx, np->resolution, np->system_startx, &xmax_t);
    check_grid_val( utm_maxy, np->resolution, np->system_starty, &ymax_t);

    xmin = xmin_t;
    xmax = xmax_t;
    ymin = ymin_t;
    ymax = ymax_t;

    if (!has_geometry) {
        np->system_bdy = new PolygonClass();
        np->system_bdy->num_segment = 1;
        np->system_bdy->num_bdy_pt = IVECTOR(np->system_bdy->num_segment);
        np->system_bdy->bdy_pt_x   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->bdy_pt_y   = (int **) malloc(np->system_bdy->num_segment * sizeof(int *));
        np->system_bdy->num_bdy_pt[0] = 4;
        np->system_bdy->bdy_pt_x[0] = IVECTOR(4);
        np->system_bdy->bdy_pt_y[0] = IVECTOR(4);
        np->npts_x = xmax - xmin + 1;
        np->npts_y = ymax - ymin + 1;
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

#if MAP_DEBUG
    printf("\n===========================================\n");
    printf("test  utm_minx= %f  and utm_maxy = %f\n", utm_minx, utm_maxy);
    printf("test  utm_maxx= %f  and utm_miny = %f\n", utm_maxx, utm_miny);

    printf("test  xmin = %d  and ymin = %d\n", xmin, ymin);
    printf("test  xmax = %d  and ymax = %d\n", xmax, ymax);
    printf("===========================================\n");
#endif

#endif

    return;
}

// check if the given string is numeric string
bool isNumeric(QString theString)
{
    bool isNum(false);

    //check the rest of the string
    for(uint i=0; i < theString.length(); i++)
    {
        //the first char can be a '-'
        if(i == 0 && theString.at(i) == '-')
            isNum = true;
        else if( theString.at(i).isDigit() )
            isNum = true;
        else if(theString.at(i) == '.')
            isNum = true;
        else
            return false;
    }

    return isNum;
}

/******************************************************************************************/
/**** FUNCTION: shift                                                                  ****/
/******************************************************************************************/
void MapBackgroundClass::shift(int dx, int dy)
{
    xmin += dx;
    xmax += dx;
    ymin += dy;
    ymax += dy;
}
#undef MAP_DEBUG   
#undef size
/******************************************************************************************/
