/******************************************************************************************/
/**** PROGRAM: subnet.cpp                                                              ****/
/**** Michael Mandell 1/14/02                                                          ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iostream>

#include "cconst.h"
#include "list.h"
#include "WiSim.h"
#include "hot_color.h"
#include "intintint.h"
#include "posn_scan.h"
#include "polygon.h"

#if HAS_GUI
#include <qapplication.h>

#include "progress_slot.h"
extern int use_gui;
#endif


/******************************************************************************************/
/**** FUNCTION: create_subnets                                                         ****/
/******************************************************************************************/
void NetworkClass::create_subnets(double scan_fractional_area, int init_sample_res, ListClass<int> *ml_list, int num_max, int has_threshold, double threshold_db, double dmax, int closest)
{
#if (DEMO == 0)
    int i, tt_idx, cell_idx, sector_idx, sector_grp_idx, scan_idx, found;
    CellClass *cell;
    SectorClass *sector;
    ListClass<int> *lc;
    char *chptr;

    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        if (num_subnet[tt_idx]) {
            sprintf(msg, "ERROR: geometry already contains subnets\n");
            PRMSG(stdout, msg); error_state = 1;
            return;
        }
    }

    chptr = msg;
    chptr += sprintf(chptr, "Creating subnets:\n");
    chptr += sprintf(chptr, "Scan fractional area: %12.7f\n", scan_fractional_area);
    PRMSG(stdout, msg);

    GUI_FN(prog_bar = new ProgressSlot(0, "Progress Bar"));
    GUI_FN(prog_bar->setOffsetWeight(0, 75));

    scan_array = (int **) malloc(npts_x*sizeof(int *));
    for (i=0; i<=npts_x-1; i++) {
        scan_array[i] = IVECTOR(npts_y);
    }

    excl_ml_list = ml_list;
    scan_has_threshold = has_threshold;
    if (has_threshold) {
        scan_threshold = exp(threshold_db*log(10.0)/10.0);
    }
    check_grid_val(dmax, resolution, 0, &scan_max_dist);

    scan_cell_list = new ListClass<int>(num_cell);
    for (i=0; i<=num_cell-1; i++) {
        scan_cell_list->append(i);
    }

    if (closest) {
        posn_scan = new PosnScanClass();
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                scan_idx = (sector_idx << bit_cell) | cell_idx;
                found = 0;
                for (sector_grp_idx=0; (sector_grp_idx<=sector_group_list->getSize()-1)&&(!found); sector_grp_idx++) {
                    lc = (ListClass<int> *) (*sector_group_list)[sector_grp_idx];
                    if (lc->contains_sorted(scan_idx)) {
                        scan_idx = (*lc)[0];
                        found = 1;
                    }
                }
                posn_scan->add_point(cell->posn_x, cell->posn_y, scan_idx);
            }
        }
        posn_scan->comp_x_list();

        scan_area(this, init_sample_res, subnet_closest_scan_fn, 0, npts_x-1, 0, npts_y-1);

        delete posn_scan;
        posn_scan = (PosnScanClass *) NULL;
    } else if (num_max == 1) {
        scan_area(this, init_sample_res, subnet_scan_fn, 0, npts_x-1, 0, npts_y-1);
    } else {
        subnet_num_max = num_max;
        subnet_idx_list = IVECTOR(num_max);
        subnet_pwr_list = DVECTOR(num_max);
        subnet_dsq_list = DVECTOR(num_max);
        scan_area(this, init_sample_res, subnet_num_max_scan_fn, 0, npts_x-1, 0, npts_y-1);
        free(subnet_idx_list);
        free(subnet_pwr_list);
        free(subnet_dsq_list);
        subnet_idx_list = (int *) NULL;
        subnet_pwr_list = (double *) NULL;
    }
    excl_ml_list = (ListClass<int> *) NULL;

    delete scan_cell_list;
    scan_cell_list = (ListClass<int> *) NULL;

    GUI_FN(prog_bar->setOffsetWeight(75, 25));

#if 0
    printf("\n");
    for (int posn_y=npts_y-1; posn_y>=0; posn_y--) {
        for (int posn_x=0; posn_x<=npts_x-1; posn_x++) {
            printf("POSN: %d %d  COLOR: %3d\n", posn_x, posn_y, scan_array[posn_x][posn_y]);
        }
        printf("\n");
    }
    fflush(stdout);
#endif

#if 0
    printf("\n");
    for (int posn_y=1612; posn_y<=1621; posn_y++) {
        for (int posn_x=71; posn_x<=73; posn_x++) {
            printf("POSN: %d %d  COLOR: %3d ", posn_x, posn_y, scan_array[posn_x][posn_y]);
        }
        printf("\n");
    }
    fflush(stdout);
#endif

    ListClass<int> *scan_idx_list;
    ListClass<int> *polygon_list;
    ListClass<int> *color_list;

    draw_polygons(scan_fractional_area, scan_idx_list, polygon_list, color_list);

    convert_polygons_subnets(scan_idx_list, polygon_list, color_list);

    delete scan_idx_list;
    delete color_list;

    for (i=0; i<=npts_x-1; i++) {
        free(scan_array[i]);
    }
    free(scan_array);

#if 0
// Do not delete traffic from subnets

    int cell_idx, sector_idx;
    CellClass   *cell;
    SectorClass *sector;

    SectorClass::num_traffic = 0;
    free(SectorClass::traffic_type_idx_list);
    SectorClass::traffic_type_idx_list = (int *) NULL;

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            free(sector->traffic_list);
            sector->traffic_list = (double *) NULL;
        }
    }
#endif

    GUI_FN(delete prog_bar);

#endif
    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::convert_polygons_subnets                                 ****/
/******************************************************************************************/
void NetworkClass::convert_polygons_subnets(ListClass<int> *scan_idx_list, ListClass<int> *polygon_list, ListClass<int> *color_list)
{
    int traffic_idx;
    int i, c_idx, s_idx, found, sector_grp, scan_idx, tt_idx;
    char tmpstr[100];
    SectorClass *sector;
    SubnetClass *subnet;
    ListClass<int> *lc;

    for (traffic_idx=0; traffic_idx<=SectorClass::num_traffic-1; traffic_idx++) {
        tt_idx = SectorClass::traffic_type_idx_list[traffic_idx];
        num_subnet[tt_idx] = scan_idx_list->getSize();
        subnet_list[tt_idx] = (SubnetClass **) malloc(num_subnet[tt_idx]*sizeof(SubnetClass *));
        for (int subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet_list[tt_idx][subnet_idx] = new SubnetClass();
            subnet = subnet_list[tt_idx][subnet_idx];
            subnet->p = ((PolygonClass *) (*polygon_list)[subnet_idx])->duplicate();
            subnet->p->comp_bdy_min_max(subnet->minx, subnet->maxx, subnet->miny, subnet->maxy);
            // subnet->color = default_color_list[(*color_list)[subnet_idx] % num_default_color];
            subnet->color = hot_color->get_color((*color_list)[subnet_idx], num_subnet[tt_idx]);
            // xxxxx subnet->color = default_color_list[subnet_idx % num_default_color];

            scan_idx = (*scan_idx_list)[subnet_idx];
            c_idx = scan_idx & ((1<<bit_cell)-1);
            s_idx = scan_idx >> bit_cell;
            sector = cell_list[c_idx]->sector_list[s_idx];
            subnet->arrival_rate = sector->st_comp_arrival_rate(traffic_idx);

            found = 0;
            for (sector_grp=0; (sector_grp<=sector_group_list->getSize()-1)&&(!found); sector_grp++) {
                lc = (ListClass<int> *) (*sector_group_list)[sector_grp];
                if ((*lc)[0] == scan_idx) {
                    found = 1;
                    for (i=1; i<=lc->getSize()-1; i++) {
                        c_idx = (*lc)[i] & ((1<<bit_cell)-1);
                        s_idx = (*lc)[i] >> bit_cell;
                        sector = cell_list[c_idx]->sector_list[s_idx];
                        subnet->arrival_rate += sector->st_comp_arrival_rate(traffic_idx);
                    }
                    strcpy(tmpstr, (*sector_group_name_list)[sector_grp]);
                }
            }
            if (!found) {
                strcpy(tmpstr, cell_list[c_idx]->view_name(c_idx, subnet_naming_convention));
            }
            subnet->strid = strdup(tmpstr);
        }
    }

    for (int idx = 0; idx <= scan_idx_list->getSize()-1; idx++) {
        delete ((PolygonClass *) (*polygon_list)[idx]);
    }
    delete polygon_list;
}
/******************************************************************************************/
/**** FUNCTION: scan_area                                                              ****/
/******************************************************************************************/
void scan_area(NetworkClass *np, int scan_res, void (*scan_fn)(NetworkClass *, int, int), int xmin, int xmax, int ymin, int ymax)
{
    int posn_x, posn_y;
    int scan_idx = CConst::NullScan;
    int tx, ty, do_scan, first_time, e;
    char *chptr;

#if HAS_GUI
    int curr_prog;
    int nx_thr = 0;
    int nx = 0;
    int tot_x = 3*np->npts_x - (np->npts_x / scan_res);
    ((Q3ProgressDialog *) np->prog_bar)->setLabelText(qApp->translate("ProgressSlot", "Scanning Area") + " ...");
#endif

    time_t td;
    time(&td);

    chptr = np->msg;
    chptr += sprintf(chptr, "NPTS_X: %d\n", np->npts_x);
    chptr += sprintf(chptr, "NPTS_Y: %d\n", np->npts_y);
    chptr += sprintf(chptr, "%s", ctime(&td));
    chptr += sprintf(chptr, "BEGIN: scanning area ...\n");
    PRMSG(stdout, np->msg);

    first_time = 1;
    while(scan_res) {

#if CDEBUG
        chptr = np->msg;
        chptr += sprintf(chptr, "Scanning with scan_res = %d ...", scan_res);
        PRMSG(stdout, np->msg);
#endif

        /*
         *  scan_array is number of npts_x * npts_y two dimension array
         *  here posn_x,posn_y is iterator for npts not UTM coordination
         */
        for (e = (first_time ? 0 : 1); e<=3; e++) {
            tx = e>>1;
            ty = e&1;
            for (posn_x=tx*scan_res; posn_x<=np->npts_x-1; posn_x+=2*scan_res) {
                for (posn_y=ty*scan_res; posn_y<=np->npts_y-1; posn_y+=2*scan_res) {
                    do_scan = 1;
                    if ( (posn_x < xmin) || (posn_x > xmax) || (posn_y < ymin) || (posn_y > ymax) ) {
                        do_scan  = 0;
                        scan_idx = CConst::NullScan;
                    }
                    // if do_scan=0, no need to judge (x,y) position - Chengan
                    //if (!np->system_bdy->in_bdy_area(posn_x, posn_y)) {
                    if (do_scan && !np->system_bdy->in_bdy_area(posn_x, posn_y)) {
                        do_scan  = 0;
                        scan_idx = CConst::NullScan;
                    }

                    // CGDBG
#if 0
                    //if(posn_x%10==0) 
                    {
                        sprintf(np->msg, "tx %d ty %d posn_x %d posn_y %d \n", tx,ty,posn_x,posn_y);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                    }
#endif

                    if ((!first_time)&&(do_scan)) {
                        if (   (tx==0) && (ty==1)
                            && (posn_x >= 2*scan_res) && (posn_x <= np->npts_x-1-2*scan_res)
                            && (posn_y >=   scan_res) && (posn_y <= np->npts_y-1-scan_res) ) {
                            scan_idx = np->scan_array[posn_x][posn_y+scan_res];
                            if (    (np->scan_array[posn_x           ][posn_y-scan_res] == scan_idx)
                                 && (np->scan_array[posn_x+2*scan_res][posn_y+scan_res] == scan_idx)
                                 && (np->scan_array[posn_x+2*scan_res][posn_y-scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-2*scan_res][posn_y+scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-2*scan_res][posn_y-scan_res] == scan_idx) ) {
                                do_scan = 0;
                            }
                        } else if (    (tx==1) && (ty==0)
                                    && (posn_x >=   scan_res) && (posn_x <= np->npts_x-1-scan_res)
                                    && (posn_y >= 2*scan_res) && (posn_y <= np->npts_y-1-2*scan_res) ) {
                            scan_idx = np->scan_array[posn_x+scan_res][posn_y];
                            if (    (np->scan_array[posn_x-scan_res][posn_y           ] == scan_idx)
                                 && (np->scan_array[posn_x+scan_res][posn_y+2*scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-scan_res][posn_y+2*scan_res] == scan_idx)
                                 && (np->scan_array[posn_x+scan_res][posn_y-2*scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-scan_res][posn_y-2*scan_res] == scan_idx) ) {
                                do_scan = 0;
                            }
                        } else if (    (tx==1) && (ty==1)
                                    && (posn_x >=   scan_res) && (posn_x <= np->npts_x-1-scan_res)
                                    && (posn_y >=   scan_res) && (posn_y <= np->npts_y-1-scan_res) ) {
                            scan_idx = np->scan_array[posn_x+scan_res][posn_y];
                            if (    (np->scan_array[posn_x+scan_res][posn_y+scan_res] == scan_idx)
                                 && (np->scan_array[posn_x         ][posn_y+scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-scan_res][posn_y+scan_res] == scan_idx)
                                 && (np->scan_array[posn_x-scan_res][posn_y         ] == scan_idx)
                                 && (np->scan_array[posn_x-scan_res][posn_y-scan_res] == scan_idx)
                                 && (np->scan_array[posn_x         ][posn_y-scan_res] == scan_idx)
                                 && (np->scan_array[posn_x+scan_res][posn_y-scan_res] == scan_idx) ) {
                                do_scan = 0;
                            }
                        }
                    }

                    if (do_scan) {
                        (*scan_fn)(np, posn_x, posn_y);
                    } else {
                        np->scan_array[posn_x][posn_y] = scan_idx;
                    }

                    // CGDBG
#if 0
                    if(do_scan==0)
                    {
                        sprintf(np->msg, "!do_scan - e %d tx %d ty %d np->scan_array[%d][%d]=%d scan_idx %d \n", e, tx,ty,posn_x,posn_y,np->scan_array[posn_x][posn_y],scan_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                    }
                    else
                    {
                        sprintf(np->msg, "do_scan - e %d tx %d ty %d np->scan_array[%d][%d]=%d scan_idx %d \n", e, tx,ty,posn_x,posn_y,np->scan_array[posn_x][posn_y],scan_idx);
                        PRMSG(stdout, np->msg); np->error_state = 1;
                    }
#endif
                }
#if HAS_GUI
                nx++;
                if (use_gui) {
                    if (nx >= nx_thr) {
                        curr_prog = (int) 100.0*nx / tot_x;
#if (CGDEBUG)
                            //printf("Setting Progress Bar to %d\n", curr_prog);
#endif
                        np->prog_bar->set_prog_percent(curr_prog);
                        nx_thr = (int) ( ((curr_prog + 1) / 100.0) * tot_x );
                    }
                }
#endif
            }
        }
        scan_res >>= 1;
        first_time = 0;
#if (CDEBUG)
        time(&td);
        chptr = np->msg;
        chptr += sprintf(chptr, "done\n");
        chptr += sprintf(chptr, "%s", ctime(&td));
        PRMSG(stdout, np->msg);
#endif
    }
    time(&td);
    chptr = np->msg;
    chptr += sprintf(chptr, "DONE: scanning area ...\n");
    chptr += sprintf(chptr, "%s", ctime(&td));
    PRMSG(stdout, np->msg);

    return;
}
/******************************************************************************************/
/**** FUNCTION: subnet_scan_fn                                                         ****/
/******************************************************************************************/
void subnet_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int i, cell_idx, sector_idx, scan_idx, map_layer_idx, found, sector_grp_idx;
    double pwr, max_pwr;
    CellClass *cell;
    SectorClass *sector;
    ListClass<int> *lc;

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {

        found = 0;
        for (i=0; (i<=np->excl_ml_list->getSize()-1)&&(!found); i++) {
            map_layer_idx = (*(np->excl_ml_list))[i];
            if (np->in_map_layer(map_layer_idx, posn_x, posn_y)) {
                found = 1;
            }
        }

        if (!found) {
            max_pwr = -1.0;
            for (i=0; i<=np->scan_cell_list->getSize()-1; i++) {
                cell_idx = (*np->scan_cell_list)[i];
                cell = np->cell_list[cell_idx];
                for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                    sector = cell->sector_list[sector_idx];
                    pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y, np->scan_max_dist);
                    if (pwr > max_pwr) {
                        max_pwr = pwr;
                        scan_idx = (sector_idx << np->bit_cell) | cell_idx;
                        found = 0;
                        for (sector_grp_idx=0; (sector_grp_idx<=np->sector_group_list->getSize()-1)&&(!found); sector_grp_idx++) {
                            lc = (ListClass<int> *) (*(np->sector_group_list))[sector_grp_idx];
                            if (lc->contains_sorted(scan_idx)) {
                                scan_idx = (*lc)[0];
                                found = 1;
                            }
                        }
                        np->scan_array[posn_x][posn_y] = scan_idx << 8;
                    }
                }
            }
            if ( (max_pwr < 0.0) || ((np->scan_has_threshold) && (max_pwr < np->scan_threshold)) ) {
                np->scan_array[posn_x][posn_y] = CConst::NullScan;
            }
        } else {
            np->scan_array[posn_x][posn_y] = CConst::NullScan;
        }
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/
/**** FUNCTION: subnet_closest_scan_fn                                                 ****/
/******************************************************************************************/
void subnet_closest_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int found, d1, d2;
    IntIntIntClass *pt;

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {
        d1 = 0;
        d2 = 1;
        found = 0;
        while (!found) {
            pt = np->posn_scan->get_closest_pt(posn_x, posn_y, d1, d2);
            if (pt) {
                found = 1;
            } else {
                d1 = d2+1;
                d2 *= 2;
            }
        }
        pt = np->posn_scan->get_closest_pt(posn_x, posn_y, d2+1, (3*d2+1)/2);

        np->scan_array[posn_x][posn_y] = (pt->z()) << 8;
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/
/**** FUNCTION: subnet_num_max_scan_fn                                                 ****/
/******************************************************************************************/
void subnet_num_max_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int i, k, idx, cell_idx, sector_idx, scan_idx, map_layer_idx, found, sector_grp_idx;
    double pwr;
    CellClass *cell;
    SectorClass *sector;
    ListClass<int> *lc;

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {

        found = 0;
        for (i=0; (i<=np->excl_ml_list->getSize()-1)&&(!found); i++) {
            map_layer_idx = (*(np->excl_ml_list))[i];
            if (np->in_map_layer(map_layer_idx, posn_x, posn_y)) {
                found = 1;
            }
        }

        if (!found) {
            for (i=0; i<=np->subnet_num_max-1; i++) {
                np->subnet_pwr_list[i] = -1.0;
            }
            for (idx=0; idx<=np->scan_cell_list->getSize()-1; idx++) {
                cell_idx = (*np->scan_cell_list)[i];
                cell = np->cell_list[cell_idx];
                for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                    sector = cell->sector_list[sector_idx];
                    pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y);
                    i = 0;
                    while((i<=np->subnet_num_max-1)&&(pwr <= np->subnet_pwr_list[i])) {
                        i++;
                    }
                    if (i<=np->subnet_num_max-1) {
                        for (k=np->subnet_num_max-1; k>=i+1; k--) {
                            np->subnet_pwr_list[k] = np->subnet_pwr_list[k-1];
                            np->subnet_dsq_list[k] = np->subnet_dsq_list[k-1];
                            np->subnet_idx_list[k] = np->subnet_idx_list[k-1];
                        }
                        np->subnet_pwr_list[i] = pwr;
                        np->subnet_dsq_list[i] = (double) (cell->posn_x - posn_x)*(cell->posn_x - posn_x)
                                               + (double) (cell->posn_y - posn_y)*(cell->posn_y - posn_y);
                        scan_idx = (sector_idx << np->bit_cell) | cell_idx;
                        found = 0;
                        for (sector_grp_idx=0; (sector_grp_idx<=np->sector_group_list->getSize()-1)&&(!found); sector_grp_idx++) {
                            lc = (ListClass<int> *) (*(np->sector_group_list))[sector_grp_idx];
                            if (lc->contains_sorted(scan_idx)) {
                                scan_idx = (*lc)[0];
                                found = 1;
                            }
                        }
                        np->subnet_idx_list[i] = scan_idx << 8;
                    }
                }
            }

            k = 0;
            for (i=1; i<=np->subnet_num_max-1; i++) {
                if (np->subnet_dsq_list[i] < np->subnet_dsq_list[k]) {
                    k = i;
                }
            }
            np->scan_array[posn_x][posn_y] = np->subnet_idx_list[k];
        } else {
            np->scan_array[posn_x][posn_y] = CConst::NullScan;
        }
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/
