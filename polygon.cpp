/******************************************************************************************/
/**** PROGRAM: polygon.cpp                                                             ****/
/******************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "WiSim.h"
#include "intint.h"
#include "list.h"
#include "polygon.h"

/******************************************************************************************/
/**** FUNCTION: PolygonClass::PolygonClass                                             ****/
/******************************************************************************************/
PolygonClass::PolygonClass()
{
    num_segment = 0;
    num_bdy_pt  = (int  *) NULL;
    bdy_pt_x    = (int **) NULL;
    bdy_pt_y    = (int **) NULL;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::PolygonClass                                             ****/
/******************************************************************************************/
PolygonClass::PolygonClass(ListClass<IntIntClass> *ii_list)
{
    int i;

    num_segment = 1;
    num_bdy_pt  = IVECTOR(num_segment);
    num_bdy_pt[0] = ii_list->getSize();

    bdy_pt_x    = (int **) malloc(num_segment*sizeof(int *));
    bdy_pt_y    = (int **) malloc(num_segment*sizeof(int *));

    bdy_pt_x[0] = IVECTOR(num_bdy_pt[0]);
    bdy_pt_y[0] = IVECTOR(num_bdy_pt[0]);

    for (i=0; i<=num_bdy_pt[0]-1; i++) {
        bdy_pt_x[0][i] = (*ii_list)[i].x();
        bdy_pt_y[0][i] = (*ii_list)[i].y();
    }
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::~PolygonClass                                            ****/
/******************************************************************************************/
PolygonClass::~PolygonClass()
{
    int segment_idx;

    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        free(bdy_pt_x[segment_idx]);
        free(bdy_pt_y[segment_idx]);
    }
    if (num_segment) {
        free(bdy_pt_x);
        free(bdy_pt_y);
        free(num_bdy_pt);
    }
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::comp_bdy_min_max                                         ****/
/**** Find minx, maxx, miny, maxy for a list of bdy points.                            ****/
/******************************************************************************************/
void PolygonClass::comp_bdy_min_max(int &minx, int &maxx, int &miny, int &maxy, const int segment_idx)
{
    int i;
    int n = num_bdy_pt[segment_idx];
    int *x = bdy_pt_x[segment_idx];
    int *y = bdy_pt_y[segment_idx];

    minx = x[0];
    maxx = x[0];
    miny = y[0];
    maxy = y[0];
    for (i=1; i<=n-1; i++) {
        minx = (x[i] < minx ? x[i] : minx);
        maxx = (x[i] > maxx ? x[i] : maxx);
        miny = (y[i] < miny ? y[i] : miny);
        maxy = (y[i] > maxy ? y[i] : maxy);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::comp_bdy_min_max                                         ****/
/**** Find minx, maxx, miny, maxy for a list of bdy points.                            ****/
/******************************************************************************************/
void PolygonClass::comp_bdy_min_max(int &minx, int &maxx, int &miny, int &maxy)
{
    int segment_idx;
    int i_minx, i_maxx, i_miny, i_maxy;

    comp_bdy_min_max(minx, maxx, miny, maxy, 0);

    for (segment_idx=1; segment_idx<=num_segment-1; segment_idx++) {
        comp_bdy_min_max(i_minx, i_maxx, i_miny, i_maxy, segment_idx);
        minx = (i_minx < minx ? i_minx : minx);
        maxx = (i_maxx > maxx ? i_maxx : maxx);
        miny = (i_miny < miny ? i_miny : miny);
        maxy = (i_maxy > maxy ? i_maxy : maxy);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::translate                                                ****/
/******************************************************************************************/
void PolygonClass::translate(int x, int y)
{
    int i, segment_idx, n;

    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        n = num_bdy_pt[segment_idx];
        for (i=0; i<=n-1; i++) {
            bdy_pt_x[segment_idx][i] += x;
            bdy_pt_y[segment_idx][i] += y;
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::reverse                                                  ****/
/******************************************************************************************/
void PolygonClass::reverse()
{
    int i, segment_idx, n, tmp_x, tmp_y;

    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        n = num_bdy_pt[segment_idx];
        for (i=0; i<=n/2-1; i++) {
            tmp_x = bdy_pt_x[segment_idx][i];
            tmp_y = bdy_pt_y[segment_idx][i];
            bdy_pt_x[segment_idx][i] = bdy_pt_x[segment_idx][n-1-i];
            bdy_pt_y[segment_idx][i] = bdy_pt_y[segment_idx][n-1-i];
            bdy_pt_x[segment_idx][n-1-i] = tmp_x;
            bdy_pt_y[segment_idx][n-1-i] = tmp_y;
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::comp_bdy_area                                            ****/
/**** Compute area of a PolygonClass                                                   ****/
/******************************************************************************************/
double PolygonClass::comp_bdy_area()
{
    int segment_idx;
    double area;

    area = 0.0;

    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        area += comp_bdy_area(num_bdy_pt[segment_idx], bdy_pt_x[segment_idx], bdy_pt_y[segment_idx]);
    }

    return(area);
}
/******************************************************************************************/
/**** FUNCTION: in_bdy_area                                                            ****/
/**** Determine whether or not a given point lies within the bounded area              ****/
/******************************************************************************************/
int PolygonClass::in_bdy_area(const int a, const int b, int *edge)
{
    int segment_idx, n;
    int is_edge;

    if (edge) { *edge = 0; }
    n = 0;
    for (segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        n += in_bdy_area(a, b, num_bdy_pt[segment_idx], bdy_pt_x[segment_idx], bdy_pt_y[segment_idx], &is_edge);
        if (is_edge) {
            if (edge) { *edge = 1; }
            return(0);
        }
    }

    return(n&1);
}
/******************************************************************************************/
/**** FUNCTION: PolygonClass::duplicate                                                ****/
/******************************************************************************************/
PolygonClass *PolygonClass::duplicate()
{
    PolygonClass *new_polygon = new PolygonClass();

    new_polygon->num_segment = num_segment;

    new_polygon->num_bdy_pt = IVECTOR(num_segment);
    new_polygon->bdy_pt_x   = (int **) malloc(num_segment*sizeof(int *));
    new_polygon->bdy_pt_y   = (int **) malloc(num_segment*sizeof(int *));

    for (int segment_idx=0; segment_idx<=num_segment-1; segment_idx++) {
        new_polygon->num_bdy_pt[segment_idx] = num_bdy_pt[segment_idx];
        new_polygon->bdy_pt_x[segment_idx]   = IVECTOR(num_bdy_pt[segment_idx]);
        new_polygon->bdy_pt_y[segment_idx]   = IVECTOR(num_bdy_pt[segment_idx]);
        for (int bdy_pt_idx=0; bdy_pt_idx<=num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
            new_polygon->bdy_pt_x[segment_idx][bdy_pt_idx]   = bdy_pt_x[segment_idx][bdy_pt_idx];
            new_polygon->bdy_pt_y[segment_idx][bdy_pt_idx]   = bdy_pt_y[segment_idx][bdy_pt_idx];
        }
    }

    return(new_polygon);
}
/******************************************************************************************/

