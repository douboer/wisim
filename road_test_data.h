/*
*******************************************************************
* FILE: road_test_data.h
*******************************************************************
*/

#ifndef ROAD_TEST_H
#define ROAD_TEST_H

#include <iostream>
#include <vector>

#define DEBUG_ANTENNA_ANGLE_SEARCH   0 /* Debug functions for searching the antenna angle */

#if 0
#define DEBUG                        0 /* Whether or not to compile in debug mode         */

#define CSID_LENGTH                 10 /* Length of Hexadecimal CSID.                     */
#define MAX_NUM_CS                1000 /* Length of Hexadecimal CSID.                     */
#endif

class RoadTestPtClass
{
public:
    RoadTestPtClass(int x = 0, int y = 0, double pwr = 0.0, int c_idx = -1, int s_idx = -1);
    ~RoadTestPtClass();
    int operator==(RoadTestPtClass& val);
    int operator>(RoadTestPtClass& val);
    friend std::ostream& operator<<(std::ostream& s, RoadTestPtClass& val);

    int      posn_x;             // x-postion of the road test point 
    int      posn_y;             // y-positon of the road test point
    double   pwr_db;             // received power for the road test point (in db format)
    int      cell_idx;           // corresponding cell_idx
    int      sector_idx;         // corresponding sector_idx

    static   int    sort_type;
    static   int    num_level;
    static   double *level_list;
    static   int    *color_list;
};


class RxCellTableClass
{
public:
    int      rx_cell_idx;       //rx cell idx
    int      num_tx_cell;       //tx cell number of the rx cell
    int      *tx_cell_table;    //all the tx cell idx of the rx cell 
    double   *pwr_db;           //power attenuation in db format
};


class Point {
public:
    Point() { x = 0; y = 0; found = false; }
    Point( int a, int b); 
    //~Point() {};
    double  x, y;
    bool    found;
};

/*
*******************************************************************
* Function Declarations                       
*******************************************************************
*/
void    my_svdcmpz(double **a, int m, int n, double *w, double **v);
double  svd_solve(double **mx_a, double *vec_b, int M, int N, double *vec_x, int show_prog = 0);

double isLeft( Point&, Point&, Point& );
std::vector<Point>& pt_sorting( std::vector<Point>&, int );
std::vector<Point>& comp_cvx( std::vector<Point>&, std::vector<Point>&, int );
bool polygon_tangents( Point& pt, int n, std::vector<Point>& pt_vec, Point& rpoint, Point& lpoint );
double get_angle( const int start_x, const int start_y, const int end_x, const int end_y );

#endif
