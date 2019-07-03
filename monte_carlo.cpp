/******************************************************************************************/
/**** FILE: monte_carlo.cpp                                                            ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#include "cconst.h"
#include "WiSim.h"
#include "list.h"
#include "polygon.h"
#include "randomc.h"
#include "statistics.h"
#include "traffic_type.h"

#if HAS_GUI
#include <qapplication.h>
#include "progress_slot.h"
extern int use_gui;
#endif

/******************************************************************************************/
/**** FUNCTION: Virtual Functions                                                      ****/
/******************************************************************************************/
void               NetworkClass      :: gen_event(EventClass *)                    { CORE_DUMP; return; }
void               NetworkClass      :: update_network(EventClass *)               { CORE_DUMP; return; }
void               NetworkClass      :: reset_base_stations(int)                   { CORE_DUMP; return; }
void               NetworkClass      :: print_call_statistics(char *, char *, int) { CORE_DUMP; return; }
StatCountClass *   NetworkClass      :: create_call_stat_count() { CORE_DUMP; return( (StatCountClass *) NULL); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: run_simulation                                                         ****/
/**** OPTION:                                                                          ****/
/****     0: run for time_duration >= time (ignore n)                                  ****/
/****     1: run for exactly n events      (ignore time)                               ****/
/******************************************************************************************/
void NetworkClass::run_simulation(int option, int n, double time_duration)
{
    int tt_idx, subnet_idx;
    double end_time;
    int event_num;
    SubnetClass *subnet;
    EventClass *event;
    time_t td;

    int curr_prog;
    int event_num_thr = 0;
    double time_thr = abs_time;

    if (!system_bdy) {
        sprintf( msg, "ERROR: geometry not defined\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

#if (DEMO)
    if ((option != 1) || (n > 50000)) {
        sprintf( msg, "ERROR: DEMO version can run for maximum of 50000 events\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }
#endif

    total_arrival_rate = 0.0;
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        for (subnet_idx=0; subnet_idx<=num_subnet[tt_idx]-1; subnet_idx++) {
            subnet = subnet_list[tt_idx][subnet_idx];
            total_arrival_rate += subnet->arrival_rate;
        }
    }

    if (total_arrival_rate == 0.0) {
        sprintf(msg, "ERROR: Simulation has no specified traffic.  Either no subnets exist or all subnets have arrival_rate zero\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    event = new EventClass();

    if (option == 0) {
        end_time = abs_time + time_duration;
    } else {
        end_time = 0.0;
    }

    event_num = 0;

    GUI_FN(prog_bar = new ProgressSlot(0, "Progress Bar", qApp->translate("ProgressSlot", "Running Simulation") + " ..."));

    while(    ((option==0)&&(abs_time < end_time))
           || ((option==1)&&(event_num < n)) ) {
#if 0
        printf("EVENT: %d\n", event_num);
#endif
        gen_event(event);
        update_stats(event);

        update_network(event);

        event_num++;

#if 0
        time(&td);
        printf("EVENT_NUM = %d : %s", event_num, ctime(&td));
        fflush(stdout);
#endif

        if (    ( (option==1) && (event_num >= event_num_thr) )
             || ( (option==0) && (abs_time  >=      time_thr) ) ) {
            if (option==1) {
                curr_prog = (int) 100.0*event_num / n;
                event_num_thr = (int) ( ((curr_prog + 5) / 100.0) * n );
            } else {
                curr_prog = (int) ( 100.0* (time_duration-(end_time-abs_time))/time_duration );
                time_thr = end_time - (1.0 - (curr_prog+5)/100.0)*time_duration;
                if (curr_prog > 100) { curr_prog = 100; }
            }
#if HAS_GUI
            if (use_gui) {
                // printf("Setting Progress Bar to %d\n", curr_prog);
                prog_bar->set_prog_percent(curr_prog);
            } else {
#endif
                time(&td);
                printf("PROGRESS = %3d%% : %s", curr_prog, ctime(&td));
                fflush(stdout);
#if HAS_GUI
            }
#endif
        }
    }
    GUI_FN(delete prog_bar);

    delete event;

    return;
}
/******************************************************************************************/
/**** FUNCTION: gen_event_location                                                     ****/
/******************************************************************************************/
void NetworkClass::gen_event_location(EventClass *event, int subnet_idx)
{
    double r;
    int x, y;
    int traffic_type_idx;
    int in_area, is_edge;
    SubnetClass *subnet;

    /* call request at location determined as follows:                                    */
    /*    subnet_i     prob = t_i where t_i is the traffic                                */
    /*    system_area  prob = 1 - SUM{t_i}                                                */

    traffic_type_idx = event->traffic_type_idx;
    subnet = subnet_list[traffic_type_idx][subnet_idx];

    do {
        r = rg->Random();
        x = subnet->minx + (int) floor((subnet->maxx - subnet->minx + 1)*r);
        r = rg->Random();
        y = subnet->miny + (int) floor((subnet->maxy - subnet->miny + 1)*r);
        in_area = subnet->p->in_bdy_area(x, y, &is_edge);
    } while (!(in_area || is_edge));

    event->posn_x = x;
    event->posn_y = y;

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: update_stats                                                           ****/
/******************************************************************************************/
void NetworkClass::update_stats(EventClass *event)
{
    int index, num_crr_minus_1;
    int master_idx_1, master_idx_2;
    int delta_x, delta_y;
    double crr_1, crr_2, *p_crr;
    double v, min_crr, time;
    CellClass *cell_1, *cell_2;
    CallClass *call_1, *call_2;

    if (stat->plot_num_comm) {
        fprintf(stat->fp_num_comm, "%10.5e %d\n", abs_time, num_call_type[0]); // xxxxxx fix to allow plotting for any type
        fflush(stat->fp_num_comm);
    }

#if 0
// xxxxxx Disable CRR measurement, does't make sense for WLAN, seldom used for PHS.
// xxxxxx If we decide to continue to support, make PHS specific.
    if (stat->measure_crr) {
        p_crr = stat->p_crr;
        num_crr_minus_1 = stat->num_crr - 1;
        min_crr = stat->min_crr;
        v = stat->n_minus_1_over_max_minus_min_crr;
        time = event->time;

        for (master_idx_1=0; master_idx_1<=master_call_list->getSize()-2; master_idx_1++) {
            call_1 = (CallClass *) (*master_call_list)[master_idx_1];
            cell_1 = cell_list[call_1->cell_idx];
#if CRR_COMM_ONLY
            if (call_1->type == COMM_TYPE) {
#endif
            for (master_idx_2=master_idx_1+1; master_idx_2<=master_call_list->getSize()-1; master_idx_2++) {
                call_2 = (CallClass *) (*master_call_list)[master_idx_2];
#if CRR_COMM_ONLY
                if (call_2->type == COMM_TYPE) {
#endif
                if (call_2->channel == call_1->channel) {
                    cell_2 = cell_list[call_2->cell_idx];

                    delta_x = call_2->posn_x - cell_1->posn_x;
                    delta_y = call_2->posn_y - cell_1->posn_y;
                    crr_1 = sqrt((double) delta_x*delta_x + delta_y*delta_y) * (rec_avg_cell_radius);

                    delta_x = call_1->posn_x - cell_2->posn_x;
                    delta_y = call_1->posn_y - cell_2->posn_y;
                    crr_2 = sqrt((double) delta_x*delta_x + delta_y*delta_y) * (rec_avg_cell_radius);

#if (CDEBUG)
if ( (crr_1 < 1.0/(2*avg_cell_radius)) || (crr_2 < 1.0/(2*avg_cell_radius)) ) {
    printf("CELL [%d]: %15.10e %15.10e\n", call_1->cell_idx, idx_to_x(cell_1->posn_x), idx_to_y(cell_1->posn_y));
    printf("CELL [%d]: %15.10e %15.10e\n", call_1->cell_idx, idx_to_x(cell_2->posn_x), idx_to_y(cell_2->posn_y));
    printf("CALL [%d]: %15.10e %15.10e\n",     master_idx_1, idx_to_x(call_1->posn_x), idx_to_y(call_1->posn_y));
    printf("CALL [%d]: %15.10e %15.10e\n",     master_idx_2, idx_to_x(call_2->posn_x), idx_to_y(call_2->posn_y));
}
#endif

#if 0
                    index = (int) floor(  (crr_1 - stat->min_crr)*(stat->num_crr-1)
                                  * (stat->rec_max_min_crr) + 0.5 );
#else
                    index = (int) floor(  (crr_1 - min_crr)*v + 0.5 );
#endif
                    if (index < 0) {
                        index = 0;
                    } else if (index > num_crr_minus_1) {
                        index = num_crr_minus_1;
                    }
                    p_crr[index] += time;

#if 0
                    index = (int) floor(  (crr_2 - stat->min_crr)*(stat->num_crr-1)
                                  * (stat->rec_max_min_crr) + 0.5 );
#else
                    index = (int) floor(  (crr_2 - min_crr)*v + 0.5 );
#endif
                    if (index < 0) {
                        index = 0;
                    } else if (index > num_crr_minus_1) {
                        index = num_crr_minus_1;
                    }
                    p_crr[index] += time;
                }
#if CRR_COMM_ONLY
                }
#endif
            }
#if CRR_COMM_ONLY
            }
#endif
        }
    }
#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: plot_crr_cdf                                                           ****/
/******************************************************************************************/
void NetworkClass::plot_crr_cdf(char *filename)
{
    int i;
    double norm, sum, crr, cdf;
    FILE *fp;

    if (strcmp(filename, "") == 0) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    norm = 0.0;
    for (i=0; i<=stat->num_crr-1; i++) {
        norm += stat->p_crr[i];
    }

    sum = 0.0;
    for (i=0; i<=stat->num_crr-1; i++) {
        crr = ( stat->max_crr*i + stat->min_crr*(stat->num_crr-1-i) ) / (stat->num_crr-1);
        sum += stat->p_crr[i];
        cdf = sum / norm;
        if (cdf > 1.0e-10) {
            sprintf(msg, "%15.10e %15.10e\n", crr, cdf);
            PRMSG(fp, msg);
        }
    }

    strcpy(msg, "\n");
    PRMSG(fp, msg);

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: assign_sector                                                          ****/
/**** This routine picks the cell/sector that the current REQUEST event is assigned.   ****/
/**** Alist of the n sectors which have the largest reveive power at the user is       ****/
/**** computed.                                                                        ****/
/******************************************************************************************/
void NetworkClass::assign_sector(int *cell_idx_ptr, int *sector_idx_ptr, EventClass *event, int n)
{
    int cell_idx, sector_idx, idx, found, i;
    int p_idx = -1;
    double pwr, *p;
    char *chptr;
    CellClass *cell;
    SectorClass *sector;

    p = DVECTOR(n);
    for (i=0; i<=n-1; i++) {
        p[i] = -1.0;
    }

    for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            if (sector->active) {
                pwr = sector->tx_pwr*sector->comp_prop(this, event->posn_x, event->posn_y);
                found = 0;
                for (idx=0; (idx<=n-1)&&(!found); idx++) {
                    if (pwr > p[idx]) {
                        found = 1;
                        p_idx = idx;
                    }
                }
                if (found) {
                    for (i=n-1; i>=p_idx+1; i--) {
                        p[i] = p[i-1];
                        cell_idx_ptr[i] = cell_idx_ptr[i-1];
                        sector_idx_ptr[i] = sector_idx_ptr[i-1];
                    }
                    p[p_idx] = pwr;
                    cell_idx_ptr[p_idx] = cell_idx;
                    sector_idx_ptr[p_idx] = sector_idx;
                }
            }
        }
    }

    if (p[n-1] == -1.0) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR routine assign_sector()\n");
        chptr += sprintf(chptr, "PS cannot see minimum required %d CS\n", n);
        chptr += sprintf(chptr, "This is most likely due to overuse of access control\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    free(p);

    if (memoryless_ps) {
        for (i=2; i<=n-1; i++) {
            cell_idx_ptr[i]   = cell_idx_ptr[i&1];
            sector_idx_ptr[i] = sector_idx_ptr[i&1];
        }
    }

#if 0
    printf("POSSIBLE (CELL, SECTOR):");
    for (i=0; i<=n-1; i++) {
        printf(" (%d, %d)", cell_idx_ptr[i], sector_idx_ptr[i]);
    }
    printf("\n");
#endif

    return;
}

/******************************************************************************************/
/**** FUNCTION: comp_sir_ps                                                            ****/
/**** This routine computes the Signal-to-Interference ratio (SIR) at the PS for the   ****/
/**** CS located at (cell_idx, sector_idx) and a PS located at position (x, y) on the  ****/
/**** specified channel.  If cell_idx == -1, only interference is computed.            ****/
/******************************************************************************************/
double NetworkClass::comp_sir_ps(int cell_idx, int sector_idx, int x, int y, int channel, double *ptr_interference)
{
    int i_cell_idx, i_sector_idx, call_idx, found;
    double signal, interference, sir;
    CellClass *i_cell;
    SectorClass *i_sector;
    CallClass *call;

    SectorClass *sector;

    if (cell_idx != -1) {
        sector = cell_list[cell_idx]->sector_list[sector_idx];
        signal = (sector->tx_pwr)*sector->comp_prop(this, x, y);
    } else {
        sector = (SectorClass *) NULL;
        signal = 0.0;
    }

    interference = 0.0;

    for (i_cell_idx=0; i_cell_idx<=num_cell-1; i_cell_idx++) {
        i_cell   = cell_list[i_cell_idx];
        for (i_sector_idx=0; i_sector_idx<=i_cell->num_sector-1; i_sector_idx++) {
            if ( (i_cell_idx != cell_idx) || (i_sector_idx != sector_idx) ) {
                i_sector = i_cell->sector_list[i_sector_idx];
                found = 0;
                for (call_idx=0; (call_idx<=i_sector->call_list->getSize()-1)&&(!found); call_idx++) {
                    call = (CallClass *) (*(i_sector->call_list))[call_idx];
                    if (call->channel == channel) {
                        found = 1;
                        interference += i_sector->tx_pwr*i_sector->comp_prop(this, x, y);
                    }
                }
            }
        }
    }

    if (interference > 0.0) {
        sir = signal / interference;
    } else {
        sir = MAX_SIR;
    }

    *ptr_interference = interference;

    return(sir);
}

/******************************************************************************************/
/**** FUNCTION: print_call_status                                                      ****/
/**** Print info for all calls in the network.                                         ****/
/******************************************************************************************/
void NetworkClass::print_call_status(FILE *fp)
{
    int master_idx, cell_idx, sector_idx, traffic_type_idx;
    char *chptr;
    CallClass *call;

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "CALL STATUS\n");
    chptr += sprintf(chptr, "TIME = %15.10f\n", abs_time);
    chptr += sprintf(chptr, "NUM CALLS = %d\n", master_call_list->getSize());
    PRMSG(fp, msg);

    for (master_idx=0; master_idx<=master_call_list->getSize()-1; master_idx++) {
        call = (CallClass *) (*master_call_list)[master_idx];
        cell_idx = call->cell_idx;
        sector_idx = call->sector_idx;
        traffic_type_idx = call->traffic_type_idx;
        chptr = msg;
        chptr += sprintf(chptr, "(%4d) %s CELL %3d SECTOR %2d POSN %10.5e %10.5e CHANNEL %3d\n",
            master_idx, traffic_type_list[traffic_type_idx]->name(),
            cell_idx, sector_idx, idx_to_x(call->posn_x), idx_to_y(call->posn_y), call->channel);
        PRMSG(fp, msg);
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: insert_pending_event                                                   ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** Insert event into the pending event list.                                        ****/
void NetworkClass::insert_pending_event(EventClass *event_ptr)
{
    int i, found, position;

    if (num_pending_event+1 >= alloc_size_pending_event_list) {
        alloc_size_pending_event_list += 100;
        pending_event_list = (EventClass **) realloc((void *) pending_event_list, alloc_size_pending_event_list*sizeof(EventClass *));
    }

    found = 0;
    position = -1;
    for (i=0; (i<=num_pending_event-1)&&(!found); i++) {
        if (event_ptr->time < pending_event_list[i]->time) {
            found = 1;
            position = i;
        }
    }
    if (!found) { position = num_pending_event; }
    num_pending_event++;
    for (i=num_pending_event-1; i>=position+1; i--) {
        pending_event_list[i] = pending_event_list[i-1];
    }
    pending_event_list[position] = event_ptr;

    return;
}
/******************************************************************************************/
