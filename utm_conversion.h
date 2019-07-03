/******************************************************************************************/
/**** PROGRAM: utm_conversions.h                                                       ****/
/**** Michael Mandell 3/6/03                                                           ****/
/******************************************************************************************/
/**** Functions for converting between Longitude/Latitude and UTM coordinates.         ****/
/**** Code downloaded on 3/1/03 from:                                                  ****/
/****     http://www.gpsy.com/gpsinfo/geotoutm/                                        ****/
/******************************************************************************************/

void LLtoUTM( double lon_deg, double lat_deg,
              double &x,  double &y,
              const int zone, const int northernHemisphere, const double a, const double eccSquared);

void UTMtoLL( const double x, const double y,
              double &lon,  double &lat,
              const int zone, const int northernHemisphere, const double a, const double eccSquared);

void getUTMZone (int &zone_num, int &zone_north, double lon_deg, double lat_deg);
