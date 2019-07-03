/******************************************************************************************/
/**** PROGRAM: phs_monte_carlo.cpp                                                     ****/
/******************************************************************************************/

#include <math.h>
#include <string.h>

#include "cconst.h"
#include "list.h"
#include "phs.h"
#include "randomc.h"
#include "phs_statistics.h"

#if HAS_GUI
#include "gconst.h"
#endif

/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::reset_base_stations                                   ****/
/******************************************************************************************/
void PHSNetworkClass::reset_base_stations(int option)
{
    if (cs_dca_alg == CConst::MelcoDCA) {
        reset_melco_params(option);
    }
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::create_call_stat_count                                ****/
/******************************************************************************************/
StatCountClass * PHSNetworkClass::create_call_stat_count() {
    return (StatCountClass *) new PHSStatCountClass();
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::gen_event                                                ****/
/******************************************************************************************/
void PHSNetworkClass::gen_event(EventClass *event)
{
    int i, flag, traffic_type_idx, subnet_idx;
    double total_rate, sum, r;
    TrafficTypeClass *traffic_type;
    SubnetClass *subnet;

    /* time to next exponential event has mean:
        1 / ( SUM expo_arrival_rates + SUM N/expo_holding_times ) */

    total_rate = total_arrival_rate;
    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        traffic_type = traffic_type_list[traffic_type_idx];
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            total_rate += num_call_type[traffic_type_idx] / traffic_type->mean_time;
        }
    }

    r = rg->Random();

    event->time = -log(r) / total_rate;

    if ( (num_pending_event) && (abs_time + event->time >= pending_event_list[0]->time) ) {
        event->copy(pending_event_list[0], abs_time);
        delete pending_event_list[0];
        for (i=0; i<=num_pending_event-2; i++) {
            pending_event_list[i] = pending_event_list[i+1];
        }
        num_pending_event--;
    } else {
        r = rg->Random() * total_rate;

        flag = 0;
        sum = 0.0;
        for (traffic_type_idx=0; (traffic_type_idx<=num_traffic_type-1)&&(!flag); traffic_type_idx++) {
            traffic_type = traffic_type_list[traffic_type_idx];
            for (subnet_idx=0; (subnet_idx<=num_subnet[traffic_type_idx]-1)&&(!flag); subnet_idx++) {
                subnet = subnet_list[traffic_type_idx][subnet_idx];
                sum += subnet->arrival_rate;
                if (r <= sum) {
                    event->traffic_type_idx = traffic_type_idx;
                    event->event_type   = CConst::RequestEvent;
                    gen_event_location(event, subnet_idx);
                    flag = 1;
                }
            }
            if ( (traffic_type->duration_dist == CConst::ExpoDist)&&(!flag) ) {
                sum += num_call_type[traffic_type_idx] / traffic_type->mean_time;
                if (r <= sum) {
                    event->traffic_type_idx = traffic_type_idx;
                    event->event_type   = CConst::HangupEvent;
                    do {
                        event->master_idx = (int) floor(rg->Random()*master_call_list->getSize());
                    } while( ((CallClass *) (*master_call_list)[event->master_idx])->traffic_type_idx != traffic_type_idx);
                    flag = 1;
                }
            }
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: update_network                                                         ****/
/**** Update network by processing the current event and all events triggered by the   ****/
/**** current event.                                                                   ****/
/******************************************************************************************/
void PHSNetworkClass::update_network(EventClass *event)
{
    int i, call_idx, master_idx, n, found, flag, assigned, initreq;
    int *cell_idx_list, *sector_idx_list;
    int cell_idx           = -1;
    int sector_idx         = -1;
    int i_cell_idx         = -1;
    int i_sector_idx       = -1;
    int i_call_idx         = -1;
    int event_idx          = -1;
    int *channel_list, sector_list_index, sector_changed;
    int channel_list_index = -1;
    int *call_check_list, num_call_check, have_event;
    int *call_modified_list, num_call_modified;
    int channel            = -1;
    int delete_call;
    int position           = -1;
    int traffic_type_idx   = -1;
    int ps_best_channel;
    int attempt, num_attempt;
    double min_time, max_time;
    double sir_cs, sir_ps, int_cs, int_ps;
    CellClass *cell, *i_cell;
    PHSSectorClass *sector = (PHSSectorClass *) NULL;
    PHSSectorClass *i_sector;
    CallClass *call, *i_call;
    EventClass *event_ptr;

    int mna = ((PHSTrafficTypeClass *) traffic_type_list[0])->get_num_attempt(CConst::RequestEvent);
    if (((PHSTrafficTypeClass *) traffic_type_list[0])->get_num_attempt(CConst::HandoverEvent) > mna) {
        mna = ((PHSTrafficTypeClass *) traffic_type_list[0])->get_num_attempt(CConst::HandoverEvent);
    }
    for (traffic_type_idx=1; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        if (((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent) > mna) {
            mna = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
        }
        if (((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent) > mna) {
            mna = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        }
    }

    call_check_list    = IVECTOR(master_call_list->getSize());
    call_modified_list = IVECTOR(master_call_list->getSize() + 1);
    cell_idx_list      = IVECTOR(mna);
    sector_idx_list    = IVECTOR(mna);
    channel_list       = IVECTOR(mna);

    num_call_check = 0;
    num_call_modified = 0;
    have_event = 1;
    delete_call = 0;

    stat->duration += event->time;
    abs_time       += event->time;

    while ((have_event) || (num_call_check)) {

        if (have_event) {
            traffic_type_idx = event->traffic_type_idx;
            have_event = 0;
            if ( (event->event_type == CConst::RequestEvent)
                || ( (event->event_type == CConst::HandoverEvent)  && (!inlist(call_modified_list, event->master_idx, num_call_modified)) ) ) {
                num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(event->event_type);
                if (num_attempt) {
                    assign_sector(cell_idx_list, sector_idx_list, event, num_attempt);
                }
                sector_list_index = 0;
                sector_changed    = 1;
                if (((PHSTrafficTypeClass *)traffic_type_list[traffic_type_idx])->ps_meas_best_channel) {
                    ps_best_channel = get_ps_best_channel(event);
                } else {
                    ps_best_channel = (num_freq << bit_slot) | (num_slot-1);
                }
                if (stat->plot_event) {
                    if (event->event_type == CConst::RequestEvent) {
                        fprintf(stat->fp_event, "CALL_REQUEST TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type_list[traffic_type_idx]->name());
                    } else {
                        fprintf(stat->fp_event, "CALL_HANDOVER TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type_list[traffic_type_idx]->name());
                        call = (CallClass *) (*master_call_list)[event->master_idx];
                        cell_idx = call->cell_idx;
                        sector_idx = call->sector_idx;
                        fprintf(stat->fp_event, "CS = (%2d, %2d)\n", cell_idx, sector_idx);
                        fprintf(stat->fp_event, "CHANNEL = %d (%d,%d)\n", call->channel,
                            (call->channel >> bit_slot),
                            (call->channel) & ((1<<bit_slot)-1));
                    }
                    fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                            idx_to_x(event->posn_x), idx_to_y(event->posn_y));
                    fflush(stat->fp_event);
                }
                assigned = 0;
                for (attempt=0; (attempt<=num_attempt-1)&&(!assigned); attempt++) {
                    if (sector_changed) {
                        cell_idx       = cell_idx_list[sector_list_index];
                        sector_idx     = sector_idx_list[sector_list_index];
                        cell           = cell_list[cell_idx];
                        sector         = (PHSSectorClass *) cell->sector_list[sector_idx];
                        sector_changed = 0;
                        if (cs_dca_alg == CConst::MelcoDCA) {
                            sector->assign_channel_melco(this, event, channel_list);
                        } else {
                            sector->assign_channel_generic(this, event, ps_best_channel, channel_list, cs_dca_alg);
                        }
                        channel_list_index = 0;
                        initreq = 1;
                    } else {
                        initreq = 0;
                        channel_list_index++;
                    }
                    if (event->event_type == CConst::RequestEvent) {
                        ((PHSStatCountClass *) stat_count)->num_request[traffic_type_idx][attempt]++;
                        ((PHSStatCountClass *) sector->stat_count)->num_request[traffic_type_idx][attempt]++;
                        if (initreq) {
                            ((PHSStatCountClass *) stat_count)->num_initreq[traffic_type_idx]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_initreq[traffic_type_idx]++;
                        } else {
                            ((PHSStatCountClass *) stat_count)->num_rereq[traffic_type_idx]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_rereq[traffic_type_idx]++;
                        }
                    } else {
                        ((PHSStatCountClass *) stat_count)->num_handover[traffic_type_idx][attempt]++;
                        ((PHSStatCountClass *) sector->stat_count)->num_handover[traffic_type_idx][attempt]++;
                        if (initreq) {
                            ((PHSStatCountClass *) stat_count)->num_initreq[traffic_type_idx]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_initreq[traffic_type_idx]++;
                        } else {
                            ((PHSStatCountClass *) stat_count)->num_rereq[traffic_type_idx]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_rereq[traffic_type_idx]++;
                        }
                    }
                    channel  = channel_list[channel_list_index];
                    if (stat->plot_event) {
                        fprintf(stat->fp_event, "ATTEMPT_%d\n", attempt);
                        fprintf(stat->fp_event, "CS = (%2d, %2d)\n", cell_idx, sector_idx);
                        if (channel >= 0) {
                            fprintf(stat->fp_event, "CS => CHANNEL = %d (%d,%d)\n", channel,
                                (channel >> bit_slot),
                                (channel) & ((1<<bit_slot)-1));
                        } else {
                            fprintf(stat->fp_event, "CS => %s\n",
                                    channel == BLOCKED_PHYS_CHAN ? "BLOCKED_PHYS_CHAN" :
                                    channel == BLOCKED_SIR_CS    ? "BLOCKED_SIR_CS"    :  "XXXXXX");
                        }
                        fflush(stat->fp_event);
                    }
                    if (channel >= 0) {
                        sir_ps = comp_sir_ps(cell_idx, sector_idx, event->posn_x, event->posn_y, channel, &int_ps);
                        if (stat->plot_event) {
                            fprintf(stat->fp_event, "PS Interference Level: %15.10e\n", int_ps);
                        }
                        if (ps_dca_alg == CConst::SIRDCA) {
                            if (sir_ps < sir_threshold_call_request_ps) {
                                channel = BLOCKED_SIR_PS;
                            }
                        } else if (ps_dca_alg == CConst::IntDCA) {
                            if (int_ps > int_threshold_call_request_ps) {
                                channel = BLOCKED_SIR_PS;
                            }
                        } else if (ps_dca_alg == CConst::IntSIRDCA) {
                            if (   (sir_ps < sir_threshold_call_request_ps)
                                || (int_ps > int_threshold_call_request_ps) ) {
                                channel = BLOCKED_SIR_PS;
                            }
                        }
                        if (stat->plot_event) {
                            if (channel >= 0) {
                                fprintf(stat->fp_event, "PS: CHANNEL ASSIGNED\n");
                            } else {
                                fprintf(stat->fp_event, "PS: BLOCKED_SIR_PS\n");
                            }
                        }
                    }

                    if (channel >= 0) {
                        assigned = 1;
                        if (event->event_type == CConst::RequestEvent) {
                            call                   = (CallClass *) malloc(sizeof(CallClass));
                            sector->call_list->append( (void *) call );
                            call->posn_x           = event->posn_x;
                            call->posn_y           = event->posn_y;
                            call->channel          = channel;
                            call->cell_idx         = cell_idx;
                            call->call_idx         = sector->call_list->getSize()-1;
                            call->sector_idx       = sector_idx;
                            call->master_idx       = master_call_list->getSize();
                            call->traffic_type_idx = traffic_type_idx;

                            if (sector->has_access_control) {
                                if ( (sector->active == 1)
                                && (sector->num_physical_tx*num_slot-1 - sector->call_list->getSize() == ac_hide_thr) ) {
                                    event_ptr = new EventClass();
                                    event_ptr->time = abs_time + ac_hide_timer;
                                    event_ptr->event_type = CConst::ACHideSectorEvent;
                                    event_ptr->traffic_type_idx = -1;
                                    event_ptr->master_idx = -1;
                                    event_ptr->cs_idx = (sector_idx << bit_cell) | cell_idx;

                                    insert_pending_event(event_ptr);
                                } else if ( (sector->active == 0)
                                && (sector->num_physical_tx*num_slot-1 - sector->call_list->getSize() == ac_use_thr-1) ) {
                                    found = 0;
                                    for (i=0; (i<=num_pending_event-1)&&(!found); i++) {
                                        if ( (pending_event_list[i]->event_type == CConst::ACUseSectorEvent)
                                           && (pending_event_list[i]->cs_idx == ((sector_idx << bit_cell) | cell_idx)) ) {
                                            found = 1;
                                            position = i;
                                        }
                                    }
                                    if (!found) {
                                        sprintf(msg, "ERROR\n");
                                        PRMSG(stdout, msg); error_state = 1;
                                        return;
                                    }
                                    delete pending_event_list[position];
                                    num_pending_event--;
                                    for (i=position; i<=num_pending_event-1; i++) {
                                        pending_event_list[i] = pending_event_list[i+1];
                                    }
                                }
                            }

                            master_call_list->append( (void *) call);
                            num_call_type[traffic_type_idx]++;
                            ((PHSStatCountClass *) stat_count)->num_request_assign[traffic_type_idx][attempt]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_request_assign[traffic_type_idx][attempt]++;
#if 0
                            printf("ASSIGNED CHANNEL: %d\n", channel);
#endif
                        } else {
                            master_idx = event->master_idx;
                            call = (CallClass *) (*master_call_list)[master_idx];
                            call->channel    = channel;
                            ((PHSStatCountClass *) stat_count)->num_handover_assign[traffic_type_idx][attempt]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_handover_assign[traffic_type_idx][attempt]++;
#if 0
                            printf("RE-ASSIGNED CHANNEL: %d\n", channel);
#endif
                        }
                        master_idx = call->master_idx;
                        call_modified_list[num_call_modified++] = master_idx;

                        if ( (event->event_type == CConst::RequestEvent)&&(traffic_type_list[traffic_type_idx]->duration_dist != CConst::ExpoDist) ) {
                            event_ptr = new EventClass();
                            min_time = traffic_type_list[traffic_type_idx]->min_time;
                            max_time = traffic_type_list[traffic_type_idx]->max_time;
                            event_ptr->time = abs_time + min_time + rg->Random()*(max_time - min_time);
                            event_ptr->event_type = CConst::HangupEvent;
                            event_ptr->traffic_type_idx = traffic_type_idx;
                            event_ptr->master_idx = master_idx;
                            event_ptr->posn_x = event->posn_x;
                            event_ptr->posn_y = event->posn_y;

                            insert_pending_event(event_ptr);
                        }

                        for (i_cell_idx=0; i_cell_idx<=num_cell-1; i_cell_idx++) {
                            i_cell = cell_list[i_cell_idx];
                            for (i_sector_idx=0; i_sector_idx<=i_cell->num_sector-1; i_sector_idx++) {
                                i_sector = (PHSSectorClass *) i_cell->sector_list[i_sector_idx];
                                found = 0;
                                for (i_call_idx=0; (i_call_idx<=i_sector->call_list->getSize()-1)&&(!found); i_call_idx++) {
                                    i_call = (CallClass *) (*(i_sector->call_list))[i_call_idx];
                                    if ( (i_call->channel == channel) && (i_call->master_idx != master_idx) ) {
                                        found = 1;
                                        flag = 0;
                                        for (i=num_call_check-1; (i>=0)&&(!flag); i--) {
                                            if (call_check_list[i] == i_call->master_idx) {
                                                flag = 1;
                                            }
                                        }
                                        if (!flag) {
                                            call_check_list[num_call_check++] = i_call->master_idx;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (event->event_type == CConst::RequestEvent) {
                            ((PHSStatCountClass *) stat_count)->num_request_block[traffic_type_idx][attempt]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_request_block[traffic_type_idx][attempt]++;
                            if (channel == BLOCKED_PHYS_CHAN) {
                                ((PHSStatCountClass *) stat_count)->num_request_blocked_phys_chan[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_request_blocked_phys_chan[traffic_type_idx][attempt]++;
                            } else if (channel == BLOCKED_SIR_CS) {
                                ((PHSStatCountClass *) stat_count)->num_request_blocked_sir_cs[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_request_blocked_sir_cs[traffic_type_idx][attempt]++;
                            } else {
                                ((PHSStatCountClass *) stat_count)->num_request_blocked_sir_ps[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_request_blocked_sir_ps[traffic_type_idx][attempt]++;
                            }
#if 0
xxxxxxxxxxxxxx
                            if (stat->plot_event) {
                                fprintf(stat->fp_event, "REQUEST_BLOCKED_%d\t%s\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                                    attempt, traffic_type_list[traffic_type_idx]->name(),
                                    abs_time, idx_to_x(event->posn_x), idx_to_y(event->posn_y), cell_idx, sector_idx);
                            }
#endif
                        } else {
                            ((PHSStatCountClass *) stat_count)->num_handover_block[traffic_type_idx][attempt]++;
                            ((PHSStatCountClass *) sector->stat_count)->num_handover_block[traffic_type_idx][attempt]++;
                            if (channel == BLOCKED_PHYS_CHAN) {
                                ((PHSStatCountClass *) stat_count)->num_handover_blocked_phys_chan[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_handover_blocked_phys_chan[traffic_type_idx][attempt]++;
                            } else if (channel == BLOCKED_SIR_CS) {
                                ((PHSStatCountClass *) stat_count)->num_handover_blocked_sir_cs[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_handover_blocked_sir_cs[traffic_type_idx][attempt]++;
                            } else {
                                ((PHSStatCountClass *) stat_count)->num_handover_blocked_sir_ps[traffic_type_idx][attempt]++;
                                ((PHSStatCountClass *) sector->stat_count)->num_handover_blocked_sir_ps[traffic_type_idx][attempt]++;
                            }
#if 0
xxxxxxxxxxxxxxxxx
                            if (stat->plot_event) {
                                fprintf(stat->fp_event, "RETRY_BLOCKED_%d\t%s\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                                    attempt, traffic_type_list[traffic_type_idx]->name(),
                                    abs_time, idx_to_x(event->posn_x), idx_to_y(event->posn_y), cell_idx, sector_idx);
                                fflush(stat->fp_event);
                            }
#endif
                        }
                        if ( (channel == BLOCKED_PHYS_CHAN) || (channel == BLOCKED_SIR_CS) ) {
                            sector_list_index++;
                            sector_changed = 1;
                        }
                    }
                }

                if (!assigned) {
                    if (event->event_type == CConst::HandoverEvent) {
                        master_idx = event->master_idx;
                        call = (CallClass *) (*master_call_list)[master_idx];
                        call->channel    = channel;
                        ((PHSStatCountClass *) stat_count)->num_drop[traffic_type_idx]++;
                        ((PHSStatCountClass *) sector->stat_count)->num_drop[traffic_type_idx]++;
#if 0
xxxxxxxxxx
                        if (stat->plot_event) {
                            fprintf(stat->fp_event, "CALL_DROP\t%s\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                                traffic_type_list[traffic_type_idx]->name(), abs_time,
                                idx_to_x(event->posn_x), idx_to_y(event->posn_y), call->cell_idx, call->sector_idx);
                            fflush(stat->fp_event);
                        }
#endif
                        delete_call = 1;
                    }
                }
            } else if (event->event_type == CConst::HandoverEvent) {
                master_idx = event->master_idx;
                call = (CallClass *) (*master_call_list)[master_idx];
                cell_idx = call->cell_idx;
                sector_idx = call->sector_idx;
                cell = cell_list[cell_idx];
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];

                ((PHSStatCountClass *) stat_count)->num_mult_handover_drop[traffic_type_idx]++;
                ((PHSStatCountClass *) sector->stat_count)->num_mult_handover_drop[traffic_type_idx]++;

                ((PHSStatCountClass *) stat_count)->num_drop[traffic_type_idx]++;
                ((PHSStatCountClass *) sector->stat_count)->num_drop[traffic_type_idx]++;

                if (stat->plot_event) {
                    fprintf(stat->fp_event, "MULT_HANDOVER_DROP\t%s\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                        traffic_type_list[traffic_type_idx]->name(), abs_time,
                        idx_to_x(event->posn_x), idx_to_y(event->posn_y), call->cell_idx, call->sector_idx);
                    fflush(stat->fp_event);
                }

                delete_call = 1;
            } else if ( (event->event_type == CConst::ACHideSectorEvent) || (event->event_type == CConst::ACUseSectorEvent) ) {
                cell_idx = event->cs_idx & ((1<<bit_cell)-1);
                cell = cell_list[cell_idx];
                sector_idx = event->cs_idx >> bit_cell;
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                sector->active = ( (event->event_type == CConst::ACHideSectorEvent) ? 0 : 1 );
                if (stat->plot_event) {
                    if (event->event_type == CConst::ACHideSectorEvent) {
                        fprintf(stat->fp_event, "AC_HIDE_SECTOR\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                            abs_time, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y), cell_idx, sector_idx);
                    } else if (event->event_type == CConst::ACUseSectorEvent) {
                        fprintf(stat->fp_event, "AC_USE_SECTOR\t%10.15f\t%10.15f\t%10.15f\t%2d\t%d\n",
                            abs_time, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y), cell_idx, sector_idx);
                    }
                    fflush(stat->fp_event);
                }
            }

            if ((event->event_type == CConst::HangupEvent) || (delete_call)) {
                master_idx = event->master_idx;
                call = (CallClass *) (*master_call_list)[master_idx];
                cell_idx = call->cell_idx;
                sector_idx = call->sector_idx;
                call_idx = call->call_idx;
                cell = cell_list[cell_idx];
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                traffic_type_idx = call->traffic_type_idx;

                if (stat->plot_event) {
                    if (event->event_type == CConst::HangupEvent) {
                        fprintf(stat->fp_event, "CALL_HANGUP TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type_list[traffic_type_idx]->name());
                        fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                                idx_to_x(call->posn_x), idx_to_y(call->posn_y));
                        fprintf(stat->fp_event, "CS = (%2d, %2d)\n", cell_idx, sector_idx);
                        fprintf(stat->fp_event, "CHANNEL = %d (%d,%d)\n", call->channel,
                            (call->channel >> bit_slot),
                            (call->channel) & ((1<<bit_slot)-1));
                    } else if (event->event_type == CConst::HandoverEvent) {
                        fprintf(stat->fp_event, "CALL_DROPPED\n");
                    } else {
                        fprintf(stat->fp_event, "XXXXXXXXXXXX\n");
                    }
                    fflush(stat->fp_event);
                }

                free( (CallClass *) (*(sector->call_list))[call_idx]);
                sector->call_list->del_elem_idx(call_idx);
                if (call_idx < sector->call_list->getSize()) {
                    ((CallClass *) (*(sector->call_list))[call_idx])->call_idx = call_idx;
                }

                found = 0;
                for (i=0; (i<=num_pending_event-1)&&(!found); i++) {
                    if (pending_event_list[i]->master_idx == master_idx) {
                        event_idx = i;
                        found = 1;
                    }
                }
                if (found) {
                    delete pending_event_list[event_idx];
                    for (i=event_idx; i<=num_pending_event-2; i++) {
                        pending_event_list[i] = pending_event_list[i+1];
                    }
                    num_pending_event--;
                }

                master_call_list->del_elem_idx(master_idx);

                if (master_idx < master_call_list->getSize()) {
                    ((CallClass *) (*(master_call_list))[master_idx])->master_idx = master_idx;

                    found = 0;
                    for (i=0; (i<=num_call_check-1)&&(!found); i++) {
                        if (call_check_list[i] == master_call_list->getSize()) {
                            call_check_list[i] = master_idx;
                            found = 1;
                        }
                    }
                    found = 0;
                    for (i=0; (i<=num_call_modified-1)&&(!found); i++) {
                        if (call_modified_list[i] == master_call_list->getSize()) {
                            call_modified_list[i] = master_idx;
                            found = 1;
                        }
                    }
                    found = 0;
                    for (i=0; (i<=num_pending_event-1)&&(!found); i++) {
                        if (pending_event_list[i]->master_idx == master_call_list->getSize()) {
                            pending_event_list[i]->master_idx = master_idx;
                            found = 1;
                        }
                    }
                }
                num_call_type[traffic_type_idx]--;
                if (sector->has_access_control) {
                    if (    (sector->active == 0)
                         && (sector->num_physical_tx*num_slot-1 - sector->call_list->getSize() == ac_use_thr) ) {
                        event_ptr = new EventClass();
                        event_ptr->time = abs_time + ac_use_timer;
                        event_ptr->event_type = CConst::ACUseSectorEvent;
                        event_ptr->traffic_type_idx = -1;
                        event_ptr->cs_idx = (sector_idx << bit_cell) | cell_idx;
                        event_ptr->master_idx = -1;

                        insert_pending_event(event_ptr);
                    } else if (    (sector->active == 1)
                         && (sector->num_physical_tx*num_slot-1 - sector->call_list->getSize() == ac_hide_thr+1) ) {
                        found = 0;
                        for (i=0; (i<=num_pending_event-1)&&(!found); i++) {
                            if ( (pending_event_list[i]->event_type == CConst::ACHideSectorEvent)
                               && (pending_event_list[i]->cs_idx == ((sector_idx << bit_cell) | cell_idx)) ) {
                                found = 1;
                                position = i;
                            }
                        }
                        if (!found) { printf("ERROR\n"); CORE_DUMP; exit(1); }
                        delete pending_event_list[position];
                        num_pending_event--;
                        for (i=position; i<=num_pending_event-1; i++) {
                            pending_event_list[i] = pending_event_list[i+1];
                        }
                    }
                }
                delete_call = 0;
            }
            if (stat->plot_event) {
                fprintf(stat->fp_event, "\n");
                fflush(stat->fp_event);
            }
        }
        while((num_call_check) && (!have_event)) {
            /* randomize list */
            // n = (int) floor(ran3(&seed, 0)*(num_call_check));
            n = (int) floor(rg->Random()*(num_call_check));
            master_idx = call_check_list[n];
            call_check_list[n] = call_check_list[num_call_check-1];
            num_call_check--;
            call = (CallClass *) (*master_call_list)[master_idx];
            cell_idx = call->cell_idx;
            sector_idx = call->sector_idx;
            sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];
            sir_cs = sector->comp_sir_cs(this, call->posn_x, call->posn_y, call->channel, &int_cs);
            sir_ps = comp_sir_ps(cell_idx, sector_idx, call->posn_x, call->posn_y, call->channel, &int_ps);

            if (   ( (cs_dca_alg == CConst::SIRDCA)    &&     (sir_cs < sir_threshold_call_drop_cs) )
                || ( (cs_dca_alg == CConst::IntDCA)    &&     (int_cs > int_threshold_call_drop_cs) )
                || ( (cs_dca_alg == CConst::SIRIntDCA) && (   (sir_cs < sir_threshold_call_drop_cs)
                                                           || (int_cs > int_threshold_call_drop_cs) ) )
                || ( (cs_dca_alg == CConst::IntSIRDCA) && (   (sir_cs < sir_threshold_call_drop_cs)
                                                           || (int_cs > int_threshold_call_drop_cs) ) )

                || ( (ps_dca_alg == CConst::SIRDCA)    && (sir_ps < sir_threshold_call_drop_ps) )
                || ( (ps_dca_alg == CConst::IntDCA)    && (int_ps > int_threshold_call_drop_ps) )
                || ( (ps_dca_alg == CConst::IntSIRDCA) && (   (sir_ps < sir_threshold_call_drop_ps)
                                                           || (int_ps > int_threshold_call_drop_ps) ) ) ) {
                event->posn_x = call->posn_x;
                event->posn_y = call->posn_y;
                event->master_idx = master_idx;
                event->event_type = CConst::HandoverEvent;
                event->traffic_type_idx = traffic_type_idx;
                have_event = 1;
#if 0
                printf("CALL RETRY for CALL %d POSN %10.5e %10.5e\n",
                    master_idx, idx_to_x(event->posn_x), idx_to_y(event->posn_y));
#endif
            }
        }
    }

    free(call_check_list);
    free(call_modified_list);
    free(cell_idx_list);
    free(sector_idx_list);
    free(channel_list);

    return;
}
/******************************************************************************************/
/**** FUNCTION: adjust_offered_traffic                                                 ****/
/******************************************************************************************/
void PHSNetworkClass::adjust_offered_traffic(int num_init_events, int num_run_events, char *filename)
{
    int *scan_idx_list, num_scan_idx;
    int tt_idx, cell_idx, sector_idx, ss_idx, scan_idx, traffic_type_idx;
    int subnet_idx, total_num_request, num_attempt, attempt, cont;
    double **offered_ctr, **carried_ctr, metric, weighted_sqerr;
    char str[100];
    double **meas_ctr;
    CellClass   *cell;
    SectorClass *sector;
    SubnetClass *subnet;
    FILE *fp;

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    scan_idx_list = IVECTOR(total_num_sectors);

    meas_ctr = (double **) malloc( SectorClass::num_traffic * sizeof(double *));
    offered_ctr = (double **) malloc( SectorClass::num_traffic * sizeof(double *));
    carried_ctr = (double **) malloc( SectorClass::num_traffic * sizeof(double *));
    for (tt_idx=0; tt_idx<=SectorClass::num_traffic-1; tt_idx++) {
        meas_ctr[tt_idx] = DVECTOR(total_num_sectors);
        offered_ctr[tt_idx] = DVECTOR(total_num_sectors);
        carried_ctr[tt_idx] = DVECTOR(total_num_sectors);
    }

    num_scan_idx = 0;
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            for (tt_idx=0; tt_idx<=sector->num_traffic-1; tt_idx++) {
                meas_ctr[tt_idx][num_scan_idx] = sector->meas_ctr_list[tt_idx];
            }
            scan_idx_list[num_scan_idx++] = (sector_idx << bit_cell) | cell_idx;
        }
    }

    /**************************************************************************************/
    /**** Initialize offered_ctr to be meas_ctr                                        ****/
    /**************************************************************************************/
    for (ss_idx=0; ss_idx<=total_num_sectors-1; ss_idx++) {
        for (tt_idx=0; tt_idx<=SectorClass::num_traffic-1; tt_idx++) {
            offered_ctr[tt_idx][ss_idx] = meas_ctr[tt_idx][ss_idx];
        }
    }
    /**************************************************************************************/

    do {
        /**********************************************************************************/
        /**** Set traffic in Subnets                                                   ****/
        /**********************************************************************************/
        for (ss_idx=0; ss_idx<=total_num_sectors-1; ss_idx++) {
            scan_idx = scan_idx_list[ss_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            //sprintf(str, "%d_%d", cell_idx, sector_idx);
            cell = cell_list[cell_idx];
            PHSSectorClass *sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            char *chptr;
            chptr = str;
            if (sector->csid_hex) {
                for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                    chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
                }
            }
            //sprintf(str, "%s", chptr);
            FILE *fp;
            fp=fopen("templog.txt","a+");
            fprintf(fp,"%s\n",str);
            fflush(fp);
            fclose(fp);
            for (int tmp_tt_idx=0; tmp_tt_idx<=SectorClass::num_traffic-1; tmp_tt_idx++) {
                traffic_type_idx = sector->traffic_type_idx_list[tmp_tt_idx];
                subnet_idx = get_subnet_idx(str, traffic_type_idx, 1);
                if (subnet_idx!=-1){
                    subnet = subnet_list[traffic_type_idx][subnet_idx];
                    /*FILE *fp;
                    fp=fopen("templog.txt","a+");
                    fprintf(fp,"%d\n",ss_idx);
                    fprintf(fp,"%d\n",sector_idx);
                    fprintf(fp,"%d\n",tmp_tt_idx);
                    fprintf(fp,"%f\n",offered_ctr[tmp_tt_idx][ss_idx]);
                    fprintf(fp,"str=%s\n",str);
                    fprintf(fp,"%d\n",traffic_type_idx);
                    fprintf(fp,"%d\n",subnet_idx);
                    //fprintf(fp,"%f",subnet->arrival_rate);
                    fflush(fp);
                    fclose(fp);*/
                    subnet->arrival_rate = offered_ctr[tmp_tt_idx][ss_idx];
                }
            }
        }
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Run Simulation                                                           ****/
        /**********************************************************************************/
        switch_mode(CConst::editGeomMode);
        rg->RandomInit(547893675);
        switch_mode(CConst::simulateMode);
        run_simulation(1, num_init_events, -1.0);
        reset_call_statistics(0);
        run_simulation(1, num_run_events, -1.0);
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Get carried_ctr                                                          ****/
        /**********************************************************************************/
        for (ss_idx=0; ss_idx<=total_num_sectors-1; ss_idx++) {
            scan_idx = scan_idx_list[ss_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            sprintf(str, "%d_%d", cell_idx, sector_idx);
            for (tt_idx=0; tt_idx<=SectorClass::num_traffic-1; tt_idx++) {
                traffic_type_idx = sector->traffic_type_idx_list[tt_idx];
                total_num_request = 0;
                num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
                for (attempt=0; attempt<=num_attempt-1; attempt++) {
                    total_num_request += ((PHSStatCountClass *) sector->stat_count)->num_request[traffic_type_idx][attempt];
                }
                num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
                for (attempt=0; attempt<=num_attempt-1; attempt++) {
                    total_num_request += ((PHSStatCountClass *) sector->stat_count)->num_handover[traffic_type_idx][attempt];
                }

                carried_ctr[tt_idx][ss_idx] = (double) total_num_request / stat->duration;
            }
        }
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Compute metric and update offered_ctr if continue                        ****/
        /**********************************************************************************/
        double weight = 1.0 / (total_num_sectors * SectorClass::num_traffic);
        metric = 0.0;
        sprintf(msg, "%4s %6s %15s %15s %15s %15s\n",
            "CELL", "SECTOR", "TRAFFIC_TYPE",
            "OFFERED_TRAFFIC", "EFF_TRAFFIC", "MEAS_EFF_TRAFFIC WEIGHTED_SQERR");
        PRMSG(fp, msg);
        for (ss_idx=0; ss_idx<=total_num_sectors-1; ss_idx++) {
            scan_idx = scan_idx_list[ss_idx];
            cell_idx   = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;
            for (tt_idx=0; tt_idx<=SectorClass::num_traffic-1; tt_idx++) {
                traffic_type_idx = sector->traffic_type_idx_list[tt_idx];
                weighted_sqerr = weight * sqerr(carried_ctr[tt_idx][ss_idx], meas_ctr[tt_idx][ss_idx]);

                sprintf(msg, "%-4d %-6d %15s %15.10f %15.10f %15.10f %15.10f\n",
                    cell_idx, sector_idx, traffic_type_list[traffic_type_idx]->strid,
                    offered_ctr[tt_idx][ss_idx], carried_ctr[tt_idx][ss_idx], meas_ctr[tt_idx][ss_idx], weighted_sqerr);
                PRMSG(fp, msg);

                metric += weighted_sqerr;
            }
        }
        sprintf(msg, "METRIC = %15.10f\n", metric);
        PRMSG(fp, msg);

        if (metric < 0.02) {
            cont = 0;
        } else {
            cont = 1;

            for (ss_idx=0; ss_idx<=total_num_sectors-1; ss_idx++) {
                for (tt_idx=0; tt_idx<=SectorClass::num_traffic-1; tt_idx++) {
                    offered_ctr[tt_idx][ss_idx] += (meas_ctr[tt_idx][ss_idx] - carried_ctr[tt_idx][ss_idx])*0.1;
                    if (offered_ctr[tt_idx][ss_idx] < 0.0) { offered_ctr[tt_idx][ss_idx] = 0.0; }
                }
            }
        }
        /**********************************************************************************/

    } while (cont);

    if (fp != stdout) {
        fclose(fp);
    }

}
/******************************************************************************************/
/**** FUNCTION: print_call_statistics                                                  ****/
/**** Print measured statistics for call requests, calls blocked, and calls dropped    ****/
/******************************************************************************************/
void PHSNetworkClass::print_call_statistics(char *filename, char *sector_list_str, int format)
{
    int i, index, cell_idx, sector_idx;
    char *chptr;
    CellClass *cell;
    SectorClass *sector;
    StatClass *sp;
    PHSStatCountClass *grp_stat_count, *phs_stat_count;
    ListClass<int> *int_list = new ListClass<int>(0);
    FILE *fp;

    if (!filename) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    if (sector_list_str) {
        extract_sector_list(int_list, sector_list_str, CConst::CellIdxRef);
        if (error_state) {
            return;
        }
    }

    phs_stat_count = (PHSStatCountClass *) stat_count;

    sp = stat;
    chptr = msg;
    chptr += sprintf(chptr, "STATISTICAL_MEASUREMENT_DURATION = %15.10f (sec)\n", sp->duration);
    PRMSG(fp, msg);

    double upbr;      // User's Perceived Blocking Rate
    double drop_rate; // Drop Rate
    int num_attempt, traffic_type_idx, tot_num_assigned;
    char *hexstr = CVECTOR(2*((PHSSectorClass *) sector)->csid_byte_length);
    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
        if (phs_stat_count->num_request[traffic_type_idx][0]) {
            upbr = (double) phs_stat_count->num_request_block[traffic_type_idx][num_attempt-1] / phs_stat_count->num_request[traffic_type_idx][0];
        } else {
            upbr = 0.0;
        }
        chptr = msg;
        chptr += sprintf(chptr, "%s_USER_PERCEIVED_BLOCKING_RATE = %15.10f\n", traffic_type_list[traffic_type_idx]->get_strid(), upbr);
        PRMSG(fp, msg);

        tot_num_assigned = 0;
        for (i=0; i<=num_attempt-1; i++) {
            tot_num_assigned += phs_stat_count->num_request_assign[traffic_type_idx][i];
        }

        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        if (tot_num_assigned) {
            drop_rate = (double) phs_stat_count->num_drop[traffic_type_idx] / tot_num_assigned;
        } else {
            drop_rate = 0.0;
        }
        chptr = msg;
        chptr += sprintf(chptr, "%s_USER_DROP_RATE = %15.10f\n", traffic_type_list[traffic_type_idx]->get_strid(), drop_rate);
        PRMSG(fp, msg);
    }

    chptr = msg;
    if (int_list->getSize() == 0) {
        chptr += sprintf(chptr, "MEASUREMENTS FOR ALL SECTORS IN THE SYSTEM\n");
        PRMSG(fp, msg);
        if (format == 1) {
            chptr = msg;
            chptr += sprintf(chptr, "CELL\tSECTOR\tCSID\tGW_CSC_CS\t");
            PRMSG(fp, msg);
            phs_stat_count->print_stat_count(fp, "", stat->duration, 1);
        }
        for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
            if (format == 0) {
                chptr = msg;
                chptr += sprintf(chptr, "CELL %d:\n", cell_idx);
                PRMSG(fp, msg);
            }
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, PHSSectorClass::csid_byte_length);
                chptr = msg;
                if (format == 0) {
                    chptr += sprintf(chptr, "    SECTOR %d:\n", sector_idx);
                    chptr += sprintf(chptr, "    CSID: %s:\n", hexstr);
                    chptr += sprintf(chptr, "    GW_CSC_CS: %.6d:\n", ((PHSSectorClass *) sector)->gw_csc_cs);
                } else {
                    chptr += sprintf(chptr, "%d\t%d\t'%s\t'%.6d\t", cell_idx, sector_idx, hexstr, ((PHSSectorClass *) sector)->gw_csc_cs);
                }
                PRMSG(fp, msg);
                sector->stat_count->print_stat_count(fp, "        ", stat->duration, (format==0 ? 0 : 2));
            }
        }
        if (format == 1) {
            chptr = msg;
            chptr += sprintf(chptr, "TOTAL\t\t\t\t");
            PRMSG(fp, msg);
        }
        stat_count->print_stat_count(fp, "", stat->duration, (format==0 ? 0 : 2));
    } else {
        chptr += sprintf(chptr, "MEASUREMENTS FOR (%d) SECTORS IN THE SYSTEM\n", int_list->getSize());
        chptr += sprintf(chptr, "SECTOR_LIST:");
        for (i=0; i<=int_list->getSize()-1; i++) {
            cell_idx = (*int_list)[i] & ( (1 << bit_cell) - 1 );
            sector_idx = (*int_list)[i] >> bit_cell;
            chptr += sprintf(chptr, " %d_%d", cell_idx, sector_idx);
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        grp_stat_count = new PHSStatCountClass();

        for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                index = (sector_idx << bit_cell) | cell_idx;
                if (int_list->contains(index)) {
                    grp_stat_count->add_stat_count(grp_stat_count, sector->stat_count);
                }
            }
        }
        if (format == 0) {
            grp_stat_count->print_stat_count(fp, "", stat->duration, 0);
        } else {
            grp_stat_count->print_stat_count(fp, "", stat->duration, 1);
            grp_stat_count->print_stat_count(fp, "", stat->duration, 2);
        }

        delete grp_stat_count;
    }

    delete int_list;
    free(hexstr);

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
