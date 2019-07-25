/******************************************************************************************/
/**** FILE: mesh.cpp                                                                   ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "mesh.h"
#include "intint.h"
#include "wisim.h"
#include "list.h"
#include "polygon.h"
#include "cconst.h"

#define DEBUG_SIMPLIFY_MESH        0
#define DEBUG_REMOVE_TINY_SEGMENTS 0

#if HAS_GUI
#   include "progress_slot.h"
#   include <qapplication.h>
    extern int use_gui;
#   if (DEBUG_SIMPLIFY_MESH || DEBUG_REMOVE_TINY_SEGMENTS)
#       include <qmessagebox.h>
#       include "visibility_window.h"
#       include "main_window.h"
#       include "command_window.h"
        extern MainWindowClass *main_window;
#   endif
#endif

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
int **             MeshNodeClass     :: scan_array            = (int **) NULL;
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ConnectionClass::ConnectionClass                                       ****/
/******************************************************************************************/
ConnectionClass::ConnectionClass()
{
   node = 0;
   cnum = 0;
}
ConnectionClass::ConnectionClass(int n, int c)
{
   node = n;
   cnum = c;
}
/******************************************************************************************/
/**** FUNCTION: ConnectionClass::~ConnectionClass                                      ****/
/******************************************************************************************/
ConnectionClass::~ConnectionClass()
{
}
/******************************************************************************************/
/**** FUNCTION: ConnectionClass::operator==                                            ****/
/******************************************************************************************/
int ConnectionClass::operator==(ConnectionClass& val) {
    if ((val.node == node) && (val.cnum == cnum)) {
        return(1);
    } else {
        return(0);
    }
}
/******************************************************************************************/
/**** FUNCTION: ConnectionClass::operator>                                             ****/
/******************************************************************************************/
int ConnectionClass::operator>(ConnectionClass& val) {
    if ( (val.node > node) || ( (val.node == node) && (val.cnum > cnum) ) ) {
        return(1);
    } else {
        return(0);
    }
}
/******************************************************************************************/
/**** FUNCTION: ConnectionClass::operator>                                             ****/
/******************************************************************************************/
std::ostream& operator<<(std::ostream& s, ConnectionClass& val) {
    s << "(" << val.node << "," << val.cnum << ")";
    return(s);
}
/******************************************************************************************/
/**** FUNCTION: MeshNodeClass::MeshNodeClass                                           ****/
/******************************************************************************************/
MeshNodeClass::MeshNodeClass(int p_posn_x, int p_posn_y)
{
    posn_x = p_posn_x;
    posn_y = p_posn_y;
    num_conn = 0;

    iconn = (ConnectionClass **) NULL;
    oconn = (ConnectionClass **) NULL;
    scan_idx = (int *) NULL;

    scan_array[posn_x][posn_y] |= 0x01;
}
/******************************************************************************************/
/**** FUNCTION: MeshNodeClass::~MeshNodeClass                                          ****/
/******************************************************************************************/
MeshNodeClass::~MeshNodeClass()
{
    int conn_idx;

    if (num_conn) {
        free(scan_idx);
        for (conn_idx=0; conn_idx<=num_conn-1; conn_idx++) {
            free(iconn[conn_idx]);
            free(oconn[conn_idx]);
        }
        free(iconn);
        free(oconn);
    }

    scan_array[posn_x][posn_y] &= ~0xFF;
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::MeshClass                                                   ****/
/******************************************************************************************/
MeshClass::MeshClass(int p_max_num_mesh_node, int **p_scan_array)
{
    max_num_mesh_node = p_max_num_mesh_node;

    mesh_node_list = (MeshNodeClass **) malloc(max_num_mesh_node * sizeof(MeshNodeClass *));
    num_mesh_node = 0;

    MeshNodeClass::scan_array = p_scan_array;
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::~MeshClass                                                  ****/
/******************************************************************************************/
MeshClass::~MeshClass()
{
    int mn;

    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            delete mesh_node_list[mn];
        }
    }
    free(mesh_node_list);
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::create_initial_mesh                                         ****/
/******************************************************************************************/
void MeshClass::create_initial_mesh(NetworkClass *np)
{
    int i, j, k, conn_idx, iconn_idx, oconn_idx;
    int posn_x, posn_y;
    int mn, n_mn;

    int *dx = IVECTOR(4);
    int *dy = IVECTOR(4);
    int *pdx = IVECTOR(4);
    int *pdy = IVECTOR(4);
    int *p  = IVECTOR(4);
    dx[0] =  1; dy[0] =  0; pdx[0] =  0; pdy[0] =  0;
    dx[1] =  0; dy[1] =  1; pdx[1] = -1; pdy[1] =  0;
    dx[2] = -1; dy[2] =  0; pdx[2] = -1; pdy[2] = -1;
    dx[3] =  0; dy[3] = -1; pdx[3] =  0; pdy[3] = -1;

    for (posn_x=0; posn_x<=np->npts_x-2; posn_x++) {
        for (posn_y=0; posn_y<=np->npts_y-2; posn_y++) {
            if (   (np->scan_array[posn_x][posn_y] != CConst::NullScan)
                 && (    (np->scan_array[posn_x+1][posn_y  ] == CConst::NullScan)
                      || (np->scan_array[posn_x  ][posn_y+1] == CConst::NullScan)
                      || (np->scan_array[posn_x+1][posn_y+1] == CConst::NullScan) ) ) {
                np->scan_array[posn_x][posn_y] = CConst::EdgeScan;
            }
        }
    }
    for (posn_x=0; posn_x<=np->npts_x-2; posn_x++) {
        if (np->scan_array[posn_x][np->npts_y-1] != CConst::NullScan) {
            np->scan_array[posn_x][np->npts_y-1] = CConst::EdgeScan;
        }
    }
    for (posn_y=0; posn_y<=np->npts_y-2; posn_y++) {
        if (np->scan_array[np->npts_x-1][posn_y] != CConst::NullScan) {
            np->scan_array[np->npts_x-1][posn_y] = CConst::EdgeScan;
        }
    }
    if (np->scan_array[np->npts_x-1][np->npts_y-1] != CConst::NullScan) {
        np->scan_array[np->npts_x-1][np->npts_y-1] = CConst::EdgeScan;
    }

#if 0
    printf("\n");
    for (int posn_y=np->npts_y-1; posn_y>=0; posn_y--) {
        for (int posn_x=0; posn_x<=np->npts_x-1; posn_x++) {
            printf(" %3d ", np->scan_array[posn_x][posn_y]);
        }
        printf("\n");
    }
    fflush(stdout);
#endif

    for (posn_x=0; posn_x<=np->npts_x-1; posn_x++) {
        for (posn_y=0; posn_y<=np->npts_y-1; posn_y++) {
            if (np->scan_array[posn_x][posn_y] != CConst::NullScan) {
                p[0] = np->scan_array[posn_x][posn_y];
                p[1] = (posn_x > 0)                   ? np->scan_array[posn_x-1][posn_y  ] : CConst::NullScan;
                p[2] = ((posn_x > 0) && (posn_y > 0)) ? np->scan_array[posn_x-1][posn_y-1] : CConst::NullScan;
                p[3] = (posn_y > 0)                   ? np->scan_array[posn_x  ][posn_y-1] : CConst::NullScan;

                p[0] &= ~0xFF; p[1] &= ~0xFF; p[2] &= ~0xFF; p[3] &= ~0xFF;

                if (    ((p[1] != p[0]) || (p[2] != p[0]) || (p[3] != p[0]))
                     && ( (p[0]>=0)||(p[1]>=0)||(p[2]>=0)||(p[3]>=0) ) ) {
                    mesh_node_list[num_mesh_node++] = new MeshNodeClass(posn_x, posn_y);
                    mn = num_mesh_node-1;
                    for (i=0; i<=3; i++) {
                        j = ((i==3) ? 0 : i+1);
                        mesh_node_list[mn]->num_conn += (((p[j] != p[i]) && (p[i] >= 0)) ? 1 : 0);
                    }
                }
            }
        }
    }

    for (mn=0; mn<=num_mesh_node-1; mn++) {
        mesh_node_list[mn]->iconn = (ConnectionClass **) malloc(mesh_node_list[mn]->num_conn*sizeof(ConnectionClass *));
        mesh_node_list[mn]->oconn = (ConnectionClass **) malloc(mesh_node_list[mn]->num_conn*sizeof(ConnectionClass *));
        mesh_node_list[mn]->scan_idx = IVECTOR(mesh_node_list[mn]->num_conn);
        for (conn_idx=0; conn_idx<=mesh_node_list[mn]->num_conn-1; conn_idx++) {
            mesh_node_list[mn]->iconn[conn_idx] = (ConnectionClass *) malloc(sizeof(ConnectionClass));
            mesh_node_list[mn]->oconn[conn_idx] = (ConnectionClass *) malloc(sizeof(ConnectionClass));
        }
        posn_x = mesh_node_list[mn]->posn_x;
        posn_y = mesh_node_list[mn]->posn_y;
        p[0] = np->scan_array[posn_x][posn_y];
        p[1] = (posn_x > 0)                   ? np->scan_array[posn_x-1][posn_y  ] : CConst::NullScan;
        p[2] = ((posn_x > 0) && (posn_y > 0)) ? np->scan_array[posn_x-1][posn_y-1] : CConst::NullScan;
        p[3] = (posn_y > 0)                   ? np->scan_array[posn_x  ][posn_y-1] : CConst::NullScan;

        p[0] &= ~0xFF; p[1] &= ~0xFF; p[2] &= ~0xFF; p[3] &= ~0xFF;

        conn_idx = 0;
        for (i=0; i<=3; i++) {
            j = ((i==3) ? 0 : i+1);
            k = ((j==3) ? 0 : j+1);
            if ((p[j] != p[i]) && (p[i] >= 0)) {
                mesh_node_list[mn]->iconn[conn_idx]->node = get_mesh_node(posn_x+dx[j], posn_y+dy[j]);
                conn_idx = ((conn_idx == mesh_node_list[mn]->num_conn-1) ? 0 : conn_idx+1);
            }
            if ((p[j] != p[i]) && (p[j] >= 0)) {
                mesh_node_list[mn]->oconn[conn_idx]->node = get_mesh_node(posn_x+dx[j], posn_y+dy[j]);
                mesh_node_list[mn]->scan_idx[conn_idx] = np->scan_array[posn_x+pdx[j]][posn_y+pdy[j]] & ~0xFF;
            }
        }
        if (conn_idx !=  0) {
            printf("ERROR generating connections for MESH_NODE_%d\n", mn);
            CORE_DUMP;
        }
    }
    for (mn=0; mn<=num_mesh_node-1; mn++) {
        for (oconn_idx=0; oconn_idx<=mesh_node_list[mn]->num_conn-1; oconn_idx++) {
            n_mn = mesh_node_list[mn]->oconn[oconn_idx]->node;
            int found = 0;
            for (iconn_idx=0; (iconn_idx<=mesh_node_list[n_mn]->num_conn-1)&&(!found); iconn_idx++) {
                if (mesh_node_list[n_mn]->iconn[iconn_idx]->node == mn) {
                    found = 1;
                    mesh_node_list[mn]->oconn[oconn_idx]->cnum = iconn_idx;
                    mesh_node_list[n_mn]->iconn[iconn_idx]->cnum = oconn_idx;

                }
            }
        }
    }

    free(dx);
    free(dy);
    free(pdx);
    free(pdy);
    free(p);
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::get_mesh_node                                               ****/
/******************************************************************************************/
int MeshClass::get_mesh_node(const int posn_x, const int posn_y)
{
    int retval = -1;
    int found  = 0;
    int mn, min_mn, max_mn;

    if (   (mesh_node_list[0]->posn_x == posn_x)
        && (mesh_node_list[0]->posn_y == posn_y) ) {
        found = 1;
        retval = 0;
    } else if (   (mesh_node_list[num_mesh_node-1]->posn_x == posn_x)
               && (mesh_node_list[num_mesh_node-1]->posn_y == posn_y) ) {
        found = 1;
        retval = num_mesh_node-1;
    } else {
        min_mn = 0;
        max_mn = num_mesh_node-1;
        while(!found) {
            mn = (min_mn + max_mn)/2;
            if (   (mesh_node_list[mn]->posn_x == posn_x)
                && (mesh_node_list[mn]->posn_y == posn_y) ) {
                found = 1;
                retval = mn;
            } else if (     (mesh_node_list[mn]->posn_x < posn_x)
                       || ( (mesh_node_list[mn]->posn_x == posn_x) && (mesh_node_list[mn]->posn_y < posn_y) ) ) {
                min_mn = mn;
            } else {
                max_mn = mn;
            }
        }
    }

    if (!found) {
        retval = -1;
CORE_DUMP;
    }

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::delete_mesh_node                                            ****/
/******************************************************************************************/
void MeshClass::delete_mesh_node(MeshNodeClass *&mesh_node)
{
    delete mesh_node;

    mesh_node = (MeshNodeClass *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::simplify                                                    ****/
/******************************************************************************************/
void MeshClass::simplify(NetworkClass *np, double scan_fractional_area)
{
    int posn_x, posn_y, total_area;
    int conn_idx;
    int mn;

#if HAS_GUI
    int curr_prog;
    int met_thr = 0;
    ((Q3ProgressDialog *) np->prog_bar)->setLabelText( qApp->translate("ProgressSlot", "Drawing Polygon Regions") + " ...");
#endif

#if HAS_GUI && DEBUG_SIMPLIFY_MESH
    int step = 0;
#endif

    remove_linear_mesh_pts();

    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            comp_mesh_node_metric(mn, np);
        }
    }
    // print_mesh(num_mesh_node, mesh_node_list);

    total_area = 0;
    for (posn_x=0; posn_x<=np->npts_x-1; posn_x++) {
        for (posn_y=0; posn_y<=np->npts_y-1; posn_y++) {
            if (np->scan_array[posn_x][posn_y] != CConst::NullScan) {
                total_area++;
            }
        }
    }
    printf("TOTAL_AREA = %d\n", total_area);

    int metric = 0;
    int max_metric = (int) floor(total_area*(1.0 - scan_fractional_area));
    int min_mn = 0;
    int min_poss_metric = 0;
    int min_metric;
    int found;

    int done = (max_metric ? 0 : 1);

    while (!done) {
        /**********************************************************************************/
        /**** Find min metric.  This can be made to run faster by pre-sorting mesh     ****/
        /**** based on metric.                                                         ****/
        /**********************************************************************************/
        min_metric = INVALID_METRIC;
        found = 0;
        mn = min_mn;
        for (int i=0; (i<=num_mesh_node-1)&&(!found); i++) {
            if (mesh_node_list[mn]) {
                if (mesh_node_list[mn]->metric < min_metric) {
                    min_metric = mesh_node_list[mn]->metric;
                    min_mn = mn;
                    if ( (min_metric==min_poss_metric) ) {
                        found = 1;
                    }
                }
            }

            mn = (mn == num_mesh_node-1 ? 0 : mn+1);
        }
        min_poss_metric = min_metric;
        /**********************************************************************************/

#if 0
        printf("===========================================================\n");
        printf("==== Begin Printing Mesh                               ====\n");
        printf("===========================================================\n");
        print_data();
        printf("===========================================================\n");
        printf("==== End Printing Mesh                               ====\n");
        printf("===========================================================\n");
#endif

#if HAS_GUI && DEBUG_SIMPLIFY_MESH

if (step >= 2657) {
        VisibilityList *vlist = main_window->visibility_window->visibility_list;
        VisibilityItem *vi;
        VisibilityCheckItem *vci;
        char line[100];
        int traffic_type_idx, subnet_idx;
        SubnetClass *subnet;

        for (traffic_type_idx=0; traffic_type_idx<=np->num_traffic_type-1; traffic_type_idx++) {
            vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
            vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
                delete vci;
            }
            main_window->visibility_window->vec_vis_subnet[traffic_type_idx] = (char *) realloc((void *) main_window->visibility_window->vec_vis_subnet[traffic_type_idx], (0)*sizeof(char));

            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                delete np->subnet_list[traffic_type_idx][subnet_idx];
            }
            if (np->num_subnet[traffic_type_idx]) {
                free(np->subnet_list[traffic_type_idx]);
            }
            np->num_subnet[traffic_type_idx] = 0;
        }

        ListClass<int> *scan_idx_list;
        PolygonClass **polygon_list;
        convert_to_polygons(polygon_list, scan_idx_list);
        int true_metric = comp_true_metric(np, polygon_list, scan_idx_list, scan_idx_list_size);
        np->convert_polygons_subnets(scan_idx_list, scan_idx_list_size, polygon_list);

        for (traffic_type_idx=0; traffic_type_idx<=np->num_traffic_type-1; traffic_type_idx++) {
            vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
            vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                subnet = np->subnet_list[traffic_type_idx][subnet_idx];
                vci = new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
            }
        }

        vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
        vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, 0);
        for (subnet_idx=0; subnet_idx<=np->num_subnet[0]-1; subnet_idx++) {
            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
        }
        vi->toggleChildren();

        qApp->processEvents();

        printf("%5d (%d <==> %d) %d\n", step, metric, true_metric, max_metric);
        // main_window->editor->resizeCanvas();
}

        step++;
#endif

        // For debugging, should always have (metric + min_metric >= 0)
        // Can use condition:
        // if ( (metric + min_metric <= max_metric) && (metric + min_metric >= 0) )

        if ( (metric + min_metric <= max_metric) ) {

#if 0
            printf("Deleting mesh node:\n");
            mesh_node_list[min_mn]->print_data(min_mn);
#endif
            int mna  = mesh_node_list[min_mn]->iconn[0]->node;
            int mnb  = mesh_node_list[min_mn]->oconn[0]->node;
            for (conn_idx=0; conn_idx<=mesh_node_list[min_mn]->num_conn-1; conn_idx++) {
                int mnin  = mesh_node_list[min_mn]->iconn[conn_idx]->node;
                int cnin  = mesh_node_list[min_mn]->iconn[conn_idx]->cnum;
                int mnout = mesh_node_list[min_mn]->oconn[conn_idx]->node;
                int cnout = mesh_node_list[min_mn]->oconn[conn_idx]->cnum;
                mesh_node_list[mnin]->oconn[cnin]->node = mnout;
                mesh_node_list[mnin]->oconn[cnin]->cnum = cnout;
                mesh_node_list[mnout]->iconn[cnout]->node = mnin;
                mesh_node_list[mnout]->iconn[cnout]->cnum = cnin;
            }
            delete_mesh_node(mesh_node_list[min_mn]);

            comp_mesh_node_metric(mna, np);
            if (mesh_node_list[mna]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mna]->metric; min_mn = mna; }
            comp_mesh_node_metric(mnb, np);
            if (mesh_node_list[mnb]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mnb]->metric; min_mn = mnb; }
            for (conn_idx=0; conn_idx<=mesh_node_list[mna]->num_conn-1; conn_idx++) {
                mn = mesh_node_list[mna]->iconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }

                mn = mesh_node_list[mna]->oconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }
            }
            for (conn_idx=0; conn_idx<=mesh_node_list[mnb]->num_conn-1; conn_idx++) {
                mn = mesh_node_list[mnb]->iconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }

                mn = mesh_node_list[mnb]->oconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }
            }

            metric += min_metric;
            // printf("METRIC = %d / %d\n", metric, max_metric);
        } else {
            done = 1;
        }

#if HAS_GUI
        if (use_gui) {
            if (metric >= met_thr) {
                curr_prog = (int) 100.0*metric / max_metric;
                // printf("Setting Progress Bar to %d\n", curr_prog);
                np->prog_bar->set_prog_percent(curr_prog);
                met_thr = (int) ( ((curr_prog + 5) / 100.0) * max_metric );
            }
        }
#endif
    }

    // print_mesh(num_mesh_node, mesh_node_list);
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::remove_tiny_segments                                        ****/
/******************************************************************************************/
void MeshClass::remove_tiny_segments(NetworkClass *np)
{
#if 0
    int posn_x, posn_y, total_area;
    int conn_idx;
    int mn_idx;
    MeshNodeClass *mn;

#if HAS_GUI
    int curr_prog;
    int met_thr = 0;
    ((Q3ProgressDialog *) np->prog_bar)->setLabelText( qApp->translate("ProgressSlot", "Fixing Polygon Segments") + " ...");
#endif

#if HAS_GUI && DEBUG_REMOVE_TINY_SEGMENTS
    int step = 0;
#endif

    for (mn_idx=0; mn_idx<=num_mesh_node-1; mn_idx++) {
        if (mesh_node_list[mn]) {
            mn = mesh_node_list[mn];
            for (conn_idx=0; conn_idx<=mn->num_conn-1; conn_idx++) {
                found = 0;
            }
        }
    }
    // print_mesh(num_mesh_node, mesh_node_list);

    total_area = 0;
    for (posn_x=0; posn_x<=np->npts_x-1; posn_x++) {
        for (posn_y=0; posn_y<=np->npts_y-1; posn_y++) {
            if (np->scan_array[posn_x][posn_y] != CConst::NullScan) {
                total_area++;
            }
        }
    }
    printf("TOTAL_AREA = %d\n", total_area);

    int metric = 0;
    int max_metric = (int) floor(total_area*(1.0 - scan_fractional_area));
    int min_mn = 0;
    int min_poss_metric = 0;
    int min_metric;
    int found;

    int done = (max_metric ? 0 : 1);

    while (!done) {
        /**********************************************************************************/
        /**** Find min metric.  This can be made to run faster by pre-sorting mesh     ****/
        /**** based on metric.                                                         ****/
        /**********************************************************************************/
        min_metric = INVALID_METRIC;
        found = 0;
        mn = min_mn;
        for (int i=0; (i<=num_mesh_node-1)&&(!found); i++) {
            if (mesh_node_list[mn]) {
                if (mesh_node_list[mn]->metric < min_metric) {
                    min_metric = mesh_node_list[mn]->metric;
                    min_mn = mn;
                    if ( (min_metric==min_poss_metric) ) {
                        found = 1;
                    }
                }
            }

            mn = (mn == num_mesh_node-1 ? 0 : mn+1);
        }
        min_poss_metric = min_metric;
        /**********************************************************************************/

#if 0
        printf("===========================================================\n");
        printf("==== Begin Printing Mesh                               ====\n");
        printf("===========================================================\n");
        print_data();
        printf("===========================================================\n");
        printf("==== End Printing Mesh                               ====\n");
        printf("===========================================================\n");
#endif

#if HAS_GUI && DEBUG_REMOVE_TINY_SEGMENTS

if (step >= 2657) {
        VisibilityList *vlist = main_window->visibility_window->visibility_list;
        VisibilityItem *vi;
        VisibilityCheckItem *vci;
        char line[100];
        int traffic_type_idx, subnet_idx;
        SubnetClass *subnet;

        for (traffic_type_idx=0; traffic_type_idx<=np->num_traffic_type-1; traffic_type_idx++) {
            vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
            vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
                delete vci;
            }
            main_window->visibility_window->vec_vis_subnet[traffic_type_idx] = (char *) realloc((void *) main_window->visibility_window->vec_vis_subnet[traffic_type_idx], (0)*sizeof(char));

            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                delete np->subnet_list[traffic_type_idx][subnet_idx];
            }
            if (np->num_subnet[traffic_type_idx]) {
                free(np->subnet_list[traffic_type_idx]);
            }
            np->num_subnet[traffic_type_idx] = 0;
        }

        ListClass<int> *scan_idx_list;
        PolygonClass **polygon_list;
        convert_to_polygons(polygon_list, scan_idx_list);
        int true_metric = comp_true_metric(np, polygon_list, scan_idx_list, scan_idx_list_size);
        np->convert_polygons_subnets(scan_idx_list, scan_idx_list_size, polygon_list);

        for (traffic_type_idx=0; traffic_type_idx<=np->num_traffic_type-1; traffic_type_idx++) {
            vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
            vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
            for (subnet_idx=0; subnet_idx<=np->num_subnet[traffic_type_idx]-1; subnet_idx++) {
                subnet = np->subnet_list[traffic_type_idx][subnet_idx];
                vci = new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
            }
        }

        vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
        vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, 0);
        for (subnet_idx=0; subnet_idx<=np->num_subnet[0]-1; subnet_idx++) {
            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
        }
        vi->toggleChildren();

        qApp->processEvents();

        printf("%5d (%d <==> %d) %d\n", step, metric, true_metric, max_metric);
        // main_window->editor->resizeCanvas();
}

        step++;
#endif

        // For debugging, should always have (metric + min_metric >= 0)
        // Can use condition:
        // if ( (metric + min_metric <= max_metric) && (metric + min_metric >= 0) )

        if ( (metric + min_metric <= max_metric) ) {

#if 0
            printf("Deleting mesh node:\n");
            mesh_node_list[min_mn]->print_data(min_mn);
#endif
            int mna  = mesh_node_list[min_mn]->iconn[0]->node;
            int mnb  = mesh_node_list[min_mn]->oconn[0]->node;
            for (conn_idx=0; conn_idx<=mesh_node_list[min_mn]->num_conn-1; conn_idx++) {
                int mnin  = mesh_node_list[min_mn]->iconn[conn_idx]->node;
                int cnin  = mesh_node_list[min_mn]->iconn[conn_idx]->cnum;
                int mnout = mesh_node_list[min_mn]->oconn[conn_idx]->node;
                int cnout = mesh_node_list[min_mn]->oconn[conn_idx]->cnum;
                mesh_node_list[mnin]->oconn[cnin]->node = mnout;
                mesh_node_list[mnin]->oconn[cnin]->cnum = cnout;
                mesh_node_list[mnout]->iconn[cnout]->node = mnin;
                mesh_node_list[mnout]->iconn[cnout]->cnum = cnin;
            }
            delete_mesh_node(mesh_node_list[min_mn]);

            comp_mesh_node_metric(mna, np);
            if (mesh_node_list[mna]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mna]->metric; min_mn = mna; }
            comp_mesh_node_metric(mnb, np);
            if (mesh_node_list[mnb]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mnb]->metric; min_mn = mnb; }
            for (conn_idx=0; conn_idx<=mesh_node_list[mna]->num_conn-1; conn_idx++) {
                mn = mesh_node_list[mna]->iconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }

                mn = mesh_node_list[mna]->oconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }
            }
            for (conn_idx=0; conn_idx<=mesh_node_list[mnb]->num_conn-1; conn_idx++) {
                mn = mesh_node_list[mnb]->iconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }

                mn = mesh_node_list[mnb]->oconn[conn_idx]->node;
                comp_mesh_node_metric(mn, np);
                if (mesh_node_list[mn]->metric <= min_poss_metric) { min_poss_metric  = mesh_node_list[mn]->metric; min_mn = mn; }
            }

            metric += min_metric;
            // printf("METRIC = %d / %d\n", metric, max_metric);
        } else {
            done = 1;
        }

#if HAS_GUI
        if (use_gui) {
            if (metric >= met_thr) {
                curr_prog = (int) 100.0*metric / max_metric;
                // printf("Setting Progress Bar to %d\n", curr_prog);
                np->prog_bar->set_prog_percent(curr_prog);
                met_thr = (int) ( ((curr_prog + 5) / 100.0) * max_metric );
            }
        }
#endif
    }

    // print_mesh(num_mesh_node, mesh_node_list);
#endif
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::remove_linear_mesh_pts                                      ****/
/******************************************************************************************/
void MeshClass::remove_linear_mesh_pts()
{
    int possible, mn, conn_idx;
    int ina, inb, outa, outb;
    int mna, mnb, x_0, y_0, x_1, y_1, x_2, y_2;
    int mnin, cnin, mnout, cnout;

    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            possible = 0;
            if (mesh_node_list[mn]->num_conn == 1) {
                possible = 1;
            } else if (mesh_node_list[mn]->num_conn == 2) {
                ina  = mesh_node_list[mn]->iconn[0]->node;
                inb  = mesh_node_list[mn]->iconn[1]->node;
                outa = mesh_node_list[mn]->oconn[0]->node;
                outb = mesh_node_list[mn]->oconn[1]->node;
                if ( ((ina==outa)&&(inb==outb)) || ((ina==outb)&&(inb==outa)) ) {
                    possible = 1;
                }
            }
            if (possible) {
                mna  = mesh_node_list[mn]->iconn[0]->node;
                mnb  = mesh_node_list[mn]->oconn[0]->node;
                x_0 = mesh_node_list[mn]->posn_x;
                y_0 = mesh_node_list[mn]->posn_y;
                x_1 = mesh_node_list[mna]->posn_x;
                y_1 = mesh_node_list[mna]->posn_y;
                x_2 = mesh_node_list[mnb]->posn_x;
                y_2 = mesh_node_list[mnb]->posn_y;
                if ( (y_1-y_0)*(x_2-x_0) != (y_2-y_0)*(x_1-x_0) ) {
                    possible = 0;
                }
                if (possible) {
                    for (conn_idx=0; conn_idx<=mesh_node_list[mn]->num_conn-1; conn_idx++) {
                        mnin  = mesh_node_list[mn]->iconn[conn_idx]->node;
                        cnin  = mesh_node_list[mn]->iconn[conn_idx]->cnum;
                        mnout = mesh_node_list[mn]->oconn[conn_idx]->node;
                        cnout = mesh_node_list[mn]->oconn[conn_idx]->cnum;
                        mesh_node_list[mnin]->oconn[cnin]->node = mnout;
                        mesh_node_list[mnin]->oconn[cnin]->cnum = cnout;
                        mesh_node_list[mnout]->iconn[cnout]->node = mnin;
                        mesh_node_list[mnout]->iconn[cnout]->cnum = cnin;
                    }
                    delete_mesh_node(mesh_node_list[mn]);
                }
            }

        }
    }
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::comp_mesh_node_metric                                       ****/
/******************************************************************************************/
void MeshClass::comp_mesh_node_metric(int mn, NetworkClass *np)
{
    int possible, i, c;
    int ina, inb, outa, outb, mna, mnb, mn_p;
    int x_0, y_0, x_1, y_1, x_2, y_2;
    int q, colora, colorb;
    int xa, ya, xb, yb, xc, yc;
    int tmp, sense;
    int x, y, ymin, ymax;
    int gcdval, dx, dy;
    int metric = 0;

    possible = 0;
    if (mesh_node_list[mn]->num_conn == 1) {
        possible = 1;
    } else if (mesh_node_list[mn]->num_conn == 2) {
        ina  = mesh_node_list[mn]->iconn[0]->node;
        inb  = mesh_node_list[mn]->iconn[1]->node;
        outa = mesh_node_list[mn]->oconn[0]->node;
        outb = mesh_node_list[mn]->oconn[1]->node;
        if ( ((ina==outa)&&(inb==outb)) || ((ina==outb)&&(inb==outa)) ) {
            possible = 1;
        }
    }

    if (possible) {
        mna  = mesh_node_list[mn]->iconn[0]->node;
        mnb  = mesh_node_list[mn]->oconn[0]->node;
        for (int conn_idx=0; conn_idx<=mesh_node_list[mna]->num_conn-1; conn_idx++) {
            if (   (mesh_node_list[mna]->iconn[conn_idx]->node == mnb)
                || (mesh_node_list[mna]->oconn[conn_idx]->node == mnb) ) {
                possible = 0;
            }
        }
    }
    if (possible) {
        x_0 = mesh_node_list[mn]->posn_x;
        y_0 = mesh_node_list[mn]->posn_y;
        x_1 = mesh_node_list[mna]->posn_x;
        y_1 = mesh_node_list[mna]->posn_y;
        x_2 = mesh_node_list[mnb]->posn_x;
        y_2 = mesh_node_list[mnb]->posn_y;

        c   = mesh_node_list[mn]->iconn[0]->cnum;
        mn_p = mesh_node_list[mna]->iconn[c]->node;
        x = mesh_node_list[mn_p]->posn_x;
        y = mesh_node_list[mn_p]->posn_y;
        q = (y-y_1)*(x_2-x_1) - (y_2-y_1)*(x-x_1);
        if ( (q == 0) && ( (x_2-x_1)*(x-x_1) + (y_2-y_1)*(y-y_1) > 0) ) {
            possible = 0;
        }

        c   = mesh_node_list[mn]->oconn[0]->cnum;
        mn_p = mesh_node_list[mnb]->oconn[c]->node;
        x = mesh_node_list[mn_p]->posn_x;
        y = mesh_node_list[mn_p]->posn_y;
        q = (y-y_2)*(x_1-x_2) - (y_1-y_2)*(x-x_2);
        if ( (q == 0) && ( (x_1-x_2)*(x-x_2) + (y_1-y_2)*(y-y_2) > 0) ) {
            possible = 0;
        }
    }

    if (possible) {
        q = (y_1-y_0)*(x_2-x_0) - (y_2-y_0)*(x_1-x_0);

        if (q == 0) {
            mesh_node_list[mn]->metric = 0;
            return;
        } else if (q > 0) {
            colora = mesh_node_list[mn]->scan_idx[0];
            if (mesh_node_list[mn]->num_conn == 2) {
                colorb = mesh_node_list[mn]->scan_idx[1];
            } else {
                colorb = CConst::NullScan;
            }
        } else {
            colorb = mesh_node_list[mn]->scan_idx[0];
            if (mesh_node_list[mn]->num_conn == 2) {
                colora = mesh_node_list[mn]->scan_idx[1];
            } else {
                colora = CConst::NullScan;
            }
        }

        xa = x_0; ya = y_0;
        xb = x_1; yb = y_1;
        xc = x_2; yc = y_2;

        if (xb < xa) {
            tmp = xa; xa = xb; xb = tmp;
            tmp = ya; ya = yb; yb = tmp;
        }

        if (xc < xa) {
            tmp = xa; xa = xc; xc = tmp;
            tmp = ya; ya = yc; yc = tmp;
        }

        if (xc < xb) {
            tmp = xb; xb = xc; xc = tmp;
            tmp = yb; yb = yc; yc = tmp;
        }

        sense = (yb-ya)*(xc-xa) - (yc-ya)*(xb-xa);

        for (x = xa+1; x<=xb-1; x++) {
            if (sense > 0) {
                ymin = ya + intdivfloor((yc-ya)*(x-xa), xc-xa) + 1;
                ymax = ya +  intdivceil((yb-ya)*(x-xa), xb-xa) - 1;
            } else {
                ymin = ya + intdivfloor((yb-ya)*(x-xa), xb-xa) + 1;
                ymax = ya +  intdivceil((yc-ya)*(x-xa), xc-xa) - 1;
            }
            for (y = ymin; y<=ymax; y++) {
                if ( (np->scan_array[x][y] & ~0xFF) == colora) {
                    metric++;
                } else if ( (np->scan_array[x][y] & ~0xFF) == colorb) {
                    metric--;
                }
                if (np->scan_array[x][y] & 0x01) {
                    possible = 0;
                }
            }
        }

        for (x = xb+1; x<=xc-1; x++) {
            if (sense > 0) {
                ymin = ya + intdivfloor((yc-ya)*(x-xa), xc-xa) + 1;
                ymax = yb +  intdivceil((yc-yb)*(x-xb), xc-xb) - 1;
            } else {
                ymin = yb + intdivfloor((yc-yb)*(x-xb), xc-xb) + 1;
                ymax = ya +  intdivceil((yc-ya)*(x-xa), xc-xa) - 1;
            }
            for (y = ymin; y<=ymax; y++) {
                if ( (np->scan_array[x][y] & ~0xFF) == colora) {
                    metric++;
                } else if ( (np->scan_array[x][y] & ~0xFF) == colorb) {
                    metric--;
                }
                if (np->scan_array[x][y] & 0x01) {
                    possible = 0;
                }
            }
        }

        if ( (xa < xb) && (xb < xc) ) {
            x = xb;
            if (sense > 0) {
                ymin = ya + intdivfloor((yc-ya)*(x-xa), xc-xa) + 1;
                ymax = yb - 1;
            } else {
                ymin = yb + 1;
                ymax = ya +  intdivceil((yc-ya)*(x-xa), xc-xa) - 1;
            }
            for (y = ymin; y<=ymax; y++) {
                if ( (np->scan_array[x][y] & ~0xFF) == colora) {
                    metric++;
                } else if ( (np->scan_array[x][y] & ~0xFF) == colorb) {
                    metric--;
                }
                if (np->scan_array[x][y] & 0x01) {
                    possible = 0;
                }
            }
        }
    }

    if (possible) {
#if 0
        if ( (x_1 != x_0) && (y_1 != y_0) ) {
            gcdval = gcd(abs(x_1-x_0), abs(y_1-y_0));
        } else if (x_1 != x_0) {
            gcdval = abs(x_1 - x_0);
        } else {
            gcdval = abs(y_1 - y_0);
        }
#else
        gcdval = gcd(abs(x_1-x_0), abs(y_1-y_0));
#endif
        dx = (x_1-x_0)/gcdval;
        dy = (y_1-y_0)/gcdval;
        for (i=1; i<=gcdval-1; i++) {
            x = x_0 + i*dx;
            y = y_0 + i*dy;
            if ( (np->scan_array[x][y] & ~0xFF) != colorb) {
                metric++;
            }
        }

#if 0
        if ( (x_2 != x_0) && (y_2 != y_0) ) {
            gcdval = gcd(abs(x_2-x_0), abs(y_2-y_0));
        } else if (x_2 != x_0) {
            gcdval = abs(x_2 - x_0);
        } else {
            gcdval = abs(y_2 - y_0);
        }
#else
        gcdval = gcd(abs(x_2-x_0), abs(y_2-y_0));
#endif
        dx = (x_2-x_0)/gcdval;
        dy = (y_2-y_0)/gcdval;
        for (i=1; i<=gcdval-1; i++) {
            x = x_0 + i*dx;
            y = y_0 + i*dy;
            if ( (np->scan_array[x][y] & ~0xFF) != colorb) {
                metric++;
            }
        }

        if ( (np->scan_array[x_0][y_0] & ~0xFF) != colorb) {
            metric++;
        }

        gcdval = gcd(abs(x_2-x_1), abs(y_2-y_1));
        dx = (x_2-x_1)/gcdval;
        dy = (y_2-y_1)/gcdval;
        for (i=1; i<=gcdval-1; i++) {
            x = x_1 + i*dx;
            y = y_1 + i*dy;
            if ( (np->scan_array[x][y] & ~0xFF) != colora) {
                metric--;
            }
        }

        if (colorb < 0) {
            metric *= 5;
        }
    } else {
        metric = INVALID_METRIC;
    }

    mesh_node_list[mn]->metric = metric;
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::convert_to_polygons                                         ****/
/******************************************************************************************/
void MeshClass::convert_to_polygons(ListClass<int> *&polygon_list, ListClass<int> *&color_list,
                                    ListClass<int> *&scan_idx_list)
{
    int i;
    int mn, n_mn, conn_idx, n_conn_idx;
    int max_idx, tmp_int, *tmp_pint;
    int n0, n1;
    double area, max_area;

    scan_idx_list = new ListClass<int>(num_mesh_node);
    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            for (conn_idx=0; conn_idx<=mesh_node_list[mn]->num_conn-1; conn_idx++) {
                scan_idx_list->ins_elem(mesh_node_list[mn]->scan_idx[conn_idx], 0);
            }
        }
    }
    polygon_list = new ListClass<int>(scan_idx_list->getSize());
    color_list   = new ListClass<int>(scan_idx_list->getSize());

#if 1
    ListClass<IntIntClass> *edgelist = new ListClass<IntIntClass>(0);

#if 0
// Edges where two polygon regions have flat side touching
    int iconn_idx, oconn_idx, found;
    int imn, omn;
    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            for (iconn_idx=0; iconn_idx<=mesh_node_list[mn]->num_conn-1; iconn_idx++) {
                imn = mesh_node_list[mn]->iconn[iconn_idx]->node;
                found = 0;
                for (oconn_idx=0; (oconn_idx<=mesh_node_list[mn]->num_conn-1)&&(!found); oconn_idx++) {
                    omn = mesh_node_list[mn]->oconn[oconn_idx]->node;
                    if ( 
                            (mesh_node_list[imn]->posn_x - mesh_node_list[mn]->posn_x)
                          * (mesh_node_list[omn]->posn_y - mesh_node_list[mn]->posn_y)
                         == (mesh_node_list[imn]->posn_y - mesh_node_list[mn]->posn_y)
                          * (mesh_node_list[omn]->posn_x - mesh_node_list[mn]->posn_x)
                       ) {
                        found = 1;
                        n0 = mesh_node_list[mn]->scan_idx[iconn_idx];
                        n1 = mesh_node_list[mn]->scan_idx[oconn_idx];
                        if (n1 < n0) {
                            tmp_int = n0;
                            n0 = n1;
                            n1 = tmp_int;
                        }
                        if (n0 != n1) {
                            edgelist->ins_elem( IntIntClass(n0, n1), 0);
                        }
                    }
                }
            }
        }
    }
#else
// Edges where two polygon regions touch in at least one point
    int conn_idx_1, conn_idx_2;
    for (mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            for (conn_idx_1=0; conn_idx_1<=mesh_node_list[mn]->num_conn-2; conn_idx_1++) {
                for (conn_idx_2=conn_idx_1+1; conn_idx_2<=mesh_node_list[mn]->num_conn-1; conn_idx_2++) {
                    n0 = mesh_node_list[mn]->scan_idx[conn_idx_1];
                    n1 = mesh_node_list[mn]->scan_idx[conn_idx_2];
                    if (n1 < n0) {
                        tmp_int = n0;
                        n0 = n1;
                        n1 = tmp_int;
                    }
                    if (n0 != n1) {
                        edgelist->ins_elem( IntIntClass(n0, n1), 0);
                    }
                }
            }
        }
    }

#endif
    edgelist->sort();
    // edgelist->printlist();
    // scan_idx_list->printlist();

    assign_colors(scan_idx_list, edgelist, color_list);
#endif

    ListClass<ConnectionClass> *connection_list = new ListClass<ConnectionClass>(2*num_mesh_node);
    // xxx int *mn_list      = IVECTOR(2*num_mesh_node);
    // xxx int *conn_list    = IVECTOR(2*num_mesh_node);
    PolygonClass *poly;

    for (int idx = 0; idx <= scan_idx_list->getSize()-1; idx++) {
        int scan_idx = (*scan_idx_list)[idx];
        poly = new PolygonClass();
        polygon_list->append( (int) poly );
        connection_list->reset();
        for (mn=0; mn<=num_mesh_node-1; mn++) {
            if (mesh_node_list[mn]) {
                for (conn_idx=0; conn_idx<=mesh_node_list[mn]->num_conn-1; conn_idx++) {
                    if (mesh_node_list[mn]->scan_idx[conn_idx] == scan_idx) {
                        connection_list->ins_elem(ConnectionClass(mn, conn_idx), 1);
                    }
                }
            }
        }

        int new_segment = 1;
        int num_segment = 0;
        int segment_start = 0;
        int pt_idx, n_pt_idx;
        ConnectionClass tmp_conn;
        for (pt_idx=0; pt_idx<=connection_list->getSize()-1; pt_idx++) {
            if (new_segment) {
                num_segment++;
                segment_start = pt_idx;
                new_segment = 0;
            }
            mn       = (*connection_list)[pt_idx].node; // xxxx mn_list[pt_idx];
            conn_idx = (*connection_list)[pt_idx].cnum; // xxxx conn_list[pt_idx];

            n_mn       = mesh_node_list[mn]->oconn[conn_idx]->node;
            n_conn_idx = mesh_node_list[mn]->oconn[conn_idx]->cnum;
            n_pt_idx = connection_list->get_index(ConnectionClass(n_mn, n_conn_idx), 1);

            if (n_pt_idx == segment_start) {
                new_segment = 1;
            } else {
                if ((pt_idx == connection_list->getSize()-1) || (n_pt_idx < pt_idx+1)) {
                    printf("ERROR in routine draw_polygons()\n");
                    CORE_DUMP;
                } else if (n_pt_idx > pt_idx+1) {
/*
xxxxxxxxx
                    tmpi = mn_list[pt_idx+1];
                    mn_list[pt_idx+1] = mn_list[n_pt_idx];
                    mn_list[n_pt_idx] = tmpi;

                    tmpi = conn_list[pt_idx+1];
                    conn_list[pt_idx+1] = conn_list[n_pt_idx];
                    conn_list[n_pt_idx] = tmpi;
*/

                    tmp_conn = (*connection_list)[pt_idx+1];
                    (*connection_list)[pt_idx+1] = (*connection_list)[n_pt_idx];
                    (*connection_list)[n_pt_idx] = tmp_conn;
                }
            }
        }
        poly->num_segment = num_segment;
        poly->num_bdy_pt  = IVECTOR(num_segment);
        poly->bdy_pt_x    = (int **) malloc(num_segment*sizeof(int *));
        poly->bdy_pt_y    = (int **) malloc(num_segment*sizeof(int *));
        int segment_idx = 0;
        int num_bdy_pt = 0;
        for (pt_idx=0; pt_idx<=connection_list->getSize()-1; pt_idx++) {
            num_bdy_pt++;
            mn       = (*connection_list)[pt_idx].node; // xxxx mn_list[pt_idx];
            conn_idx = (*connection_list)[pt_idx].cnum; // xxxx conn_list[pt_idx];

            n_mn       = mesh_node_list[mn]->oconn[conn_idx]->node;
            n_conn_idx = mesh_node_list[mn]->oconn[conn_idx]->cnum;
            if ( (pt_idx == connection_list->getSize()-1) || ((*connection_list)[pt_idx+1].node != n_mn) || ((*connection_list)[pt_idx+1].cnum != n_conn_idx) ) {
                poly->num_bdy_pt[segment_idx] = num_bdy_pt;
                num_bdy_pt = 0;
                segment_idx++;
            }
        }
        for (segment_idx=0; segment_idx<=poly->num_segment-1; segment_idx++) {
            poly->bdy_pt_x[segment_idx] = IVECTOR(poly->num_bdy_pt[segment_idx]);
            poly->bdy_pt_y[segment_idx] = IVECTOR(poly->num_bdy_pt[segment_idx]);
        }
        segment_idx = 0;
        num_bdy_pt = 0;
        for (pt_idx=0; pt_idx<=connection_list->getSize()-1; pt_idx++) {
            mn       = (*connection_list)[pt_idx].node; // xxxx mn_list[pt_idx];
            conn_idx = (*connection_list)[pt_idx].cnum; // xxxx conn_list[pt_idx];
            poly->bdy_pt_x[segment_idx][num_bdy_pt] = mesh_node_list[mn]->posn_x;
            poly->bdy_pt_y[segment_idx][num_bdy_pt] = mesh_node_list[mn]->posn_y;
            num_bdy_pt++;

            n_mn       = mesh_node_list[mn]->oconn[conn_idx]->node;
            n_conn_idx = mesh_node_list[mn]->oconn[conn_idx]->cnum;
            if ( (pt_idx == connection_list->getSize()-1) || ((*connection_list)[pt_idx+1].node != n_mn) || ((*connection_list)[pt_idx+1].cnum != n_conn_idx) ) {
                num_bdy_pt = 0;
                segment_idx++;
            }
        }

        if (num_segment) {
            max_idx = 0;
            max_area = PolygonClass::comp_bdy_area(poly->num_bdy_pt[0], poly->bdy_pt_x[0], poly->bdy_pt_y[0]);
            for (segment_idx=1; segment_idx<=num_segment-1; segment_idx++) {
                area = PolygonClass::comp_bdy_area(poly->num_bdy_pt[segment_idx], poly->bdy_pt_x[segment_idx], poly->bdy_pt_y[segment_idx]);
                if (area > max_area) {
                    max_area = area;
                    max_idx  = segment_idx;
                }
            }
            if (max_idx != 0) {
                tmp_int = poly->num_bdy_pt[max_idx];
                poly->num_bdy_pt[max_idx] = poly->num_bdy_pt[0];
                poly->num_bdy_pt[0] = tmp_int;

                tmp_pint = poly->bdy_pt_x[max_idx];
                poly->bdy_pt_x[max_idx] = poly->bdy_pt_x[0];
                poly->bdy_pt_x[0] = tmp_pint;

                tmp_pint = poly->bdy_pt_y[max_idx];
                poly->bdy_pt_y[max_idx] = poly->bdy_pt_y[0];
                poly->bdy_pt_y[0] = tmp_pint;
            }
        }
    }
    delete connection_list;
    delete edgelist;
}
/******************************************************************************************/
/**** FUNCTION: assign_colors                                                          ****/
/**** Graph coloring algorithm                                                         ****/
/******************************************************************************************/
void MeshClass::assign_colors(ListClass<int> *scan_idx_list, ListClass<IntIntClass> *edgelist,
                              ListClass<int> *color_list)
{
    int i, color, n0, n1, idx0, idx1;
    int max_idx, cont;

    ListClass<int> *order_list   = new ListClass<int>(scan_idx_list->getSize());
    ListClass<int> *worklist     = new ListClass<int>(scan_idx_list->getSize());

    for (i=0; i<=scan_idx_list->getSize()-1; i++) {
        color_list->append(-1);
    }

    color = 0;

    do {
        order_list->reset();
        worklist->reset();
        for (i=0; i<=color_list->getSize()-1; i++) {
            order_list->append(0);
            if ( (*color_list)[i] == -1) {
                worklist->append(i);
            }
        }

        cont = worklist->getSize();

        while(worklist->getSize()) {
            for (i=0; i<=color_list->getSize()-1; i++) {
                if (worklist->contains(i)) {
                    (*order_list)[i] = 0;
                } else {
                    (*order_list)[i] = -1;
                }
            }

            for (i=0; i<=edgelist->getSize()-1; i++) {
                n0 = (*edgelist)[i].getInt(0);
                n1 = (*edgelist)[i].getInt(1);
                idx0 = scan_idx_list->get_index(n0);
                idx1 = scan_idx_list->get_index(n1);

                if ( (worklist->contains(idx0)) && (worklist->contains(idx1)) ) {
                    (*order_list)[idx0]++;
                    (*order_list)[idx0]++;
                }
            }

            max_idx = 0;
            for (i=1; i<=order_list->getSize()-1; i++) {
                if ( (*order_list)[i] > (*order_list)[max_idx] ) {
                    max_idx = i;
                }
            }

            (*color_list)[max_idx] = color;
            worklist->del_elem(max_idx, 0);

            for (i=0; i<=edgelist->getSize()-1; i++) {
                n0 = (*edgelist)[i].getInt(0);
                n1 = (*edgelist)[i].getInt(1);
                if ( n0 == (*scan_idx_list)[max_idx] ) {
                    idx1 = scan_idx_list->get_index(n1);
                    worklist->del_elem(idx1, 0);
                }
                if ( n1 == (*scan_idx_list)[max_idx] ) {
                    idx0 = scan_idx_list->get_index(n0);
                    worklist->del_elem(idx0, 0);
                }
            }
        }

        color++;
    } while(cont);

    delete order_list;
    delete worklist;
}
/******************************************************************************************/
/**** FUNCTION: MeshClass::comp_true_metric                                            ****/
/******************************************************************************************/
int MeshClass::comp_true_metric(NetworkClass *np, PolygonClass **polygon_list, int *scan_idx_list, int scan_idx_list_size)
{
    int posn_x, posn_y, scan_idx, found, i, is_edge;

    int metric = 0;
    for (posn_x=0; posn_x<=np->npts_x-1; posn_x++) {
        for (posn_y=0; posn_y<=np->npts_y-1; posn_y++) {
            found = 0;
            for (i=0; (i<=scan_idx_list_size-1)&&(!found); i++) {
                if (polygon_list[i]->in_bdy_area(posn_x, posn_y, &is_edge)) {
                    found = 1;
                    scan_idx = scan_idx_list[i];
                } else if (is_edge) {
                    found = 1;
                }
            }
            if (   (( found) && (!is_edge) && (scan_idx != (np->scan_array[posn_x][posn_y] & ~0xFF)))
                || ((!found) && (np->scan_array[posn_x][posn_y]>=0         )) ) {
                metric++;
            }
        }
    }

    return(metric);
}
/******************************************************************************************/
/**** FUNCTION: print_mesh_node                                                        ****/
/******************************************************************************************/
void MeshNodeClass::print_data(int mn)
{
    printf("MESH_NODE_%d\n", mn);
    printf("    POSN: %d %d\n", posn_x, posn_y);
    printf("    METRIC: %d\n", metric);
    printf("    NUM_CONN: %d\n", num_conn);
    for (int conn_idx=0; conn_idx<=num_conn-1; conn_idx++) {
        printf("        CONN_%d: IN:%d_%d OUT: %d_%d SCAN_IDX: %d\n",
            conn_idx,
            iconn[conn_idx]->node, iconn[conn_idx]->cnum,
            oconn[conn_idx]->node, oconn[conn_idx]->cnum,
            scan_idx[conn_idx]);
    }
    fflush(stdout);

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_mesh                                                             ****/
/******************************************************************************************/
void MeshClass::print_data()
{
    for (int mn=0; mn<=num_mesh_node-1; mn++) {
        if (mesh_node_list[mn]) {
            mesh_node_list[mn]->print_data(mn);
        }
    }
    fflush(stdout);

    return;
}
/******************************************************************************************/
