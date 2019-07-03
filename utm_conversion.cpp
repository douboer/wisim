/******************************************************************************************/
/**** PROGRAM: utm_conversions.cpp                                                     ****/
/**** Michael Mandell 3/6/03                                                           ****/
/******************************************************************************************/
/**** Functions for converting between Longitude/Latitude and UTM coordinates.         ****/
/**** Code downloaded on 3/1/03 from:                                                  ****/
/****     http://www.gpsy.com/gpsinfo/geotoutm/                                        ****/
/******************************************************************************************/

#include <math.h>
#include "WiSim.h"
#include "utm_conversion.h"

void LLtoUTM( double lon_deg, double lat_deg,
              double &x,  double &y,
              const int zone, const int northernHemisphere, const double a, const double eccSquared)
{
    double k0 = 0.9996;

    double LongOrigin;
    double eccPrimeSquared;
    double N, T, C, A, M;
    double lon_rad, lat_rad;

    lon_rad = lon_deg*PI/180.0;
    lat_rad = lat_deg*PI/180.0;

    LongOrigin = ((zone - 1)*6 - 180 + 3)*PI/180.0;  //+3 puts origin in middle of zone

    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    N = a/sqrt(1-eccSquared*sin(lat_rad)*sin(lat_rad));
    T = tan(lat_rad)*tan(lat_rad);
    C = eccPrimeSquared*cos(lat_rad)*cos(lat_rad);
    A = cos(lat_rad)*(lon_rad-LongOrigin);

    M = a*((1 - eccSquared/4 - 3*eccSquared*eccSquared/64 - 5*eccSquared*eccSquared*eccSquared/256)*lat_rad 
            - (3*eccSquared/8 + 3*eccSquared*eccSquared/32 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*lat_rad)
            + (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*lat_rad) 
            - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*lat_rad));

    x = (double)(k0*N*(A+(1-T+C)*A*A*A/6
      + (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
      + 500000.0);

    y = (double)(k0*(M+N*tan(lat_rad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
      + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));

    if(!northernHemisphere) {
        y += 10000000.0; //10000000 meter offset for southern hemisphere
    }
}

void UTMtoLL( double x, double y,
              double &lon,  double &lat,
              const int zone, const int northernHemisphere, const double a, const double eccSquared)
{
    double k0 = 0.9996;
    double eccPrimeSquared;
    double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
    double N1, T1, C1, R1, D, M;
    double LongOrigin;
    double mu, phi1Rad;

    x -= 500000.0; //remove 500,000 meter offset for longitude

    if(!northernHemisphere) {
        y -= 10000000.0; //remove 10,000,000 meter offset used for southern hemisphere
    }

    LongOrigin = (zone - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    M = y / k0;
    mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

    phi1Rad = mu    + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu) 
                + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
                +(151*e1*e1*e1/96)*sin(6*mu);
    // phi1 = phi1Rad*rad2deg;

    N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
    T1 = tan(phi1Rad)*tan(phi1Rad);
    C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
    R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
    D = x/(N1*k0);

    lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
                    +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
    lat *= 180.0/PI;

    lon = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
                    *D*D*D*D*D/120)/cos(phi1Rad);
    lon = LongOrigin + lon * 180.0/PI;

    return;
}

void getUTMZone(int &zone_num, int &zone_north, double lon_deg, double lat_deg)
{
    double tmp;

    tmp = lon_deg + 180.0;
    while (tmp < 0.0) { tmp += 360.0; }

    zone_num = (((int) floor(tmp/6)) % 60) + 1;

    zone_north = ((lat_deg >= 0) ? 1 : 0);

    return;
}
