/******************************************************************************************/
/**** FILE: phs_assign_channel.cpp                                                     ****/
/******************************************************************************************/
/**** Functions for Chanel Assignment Algorithm                                        ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#include "cconst.h"
#include "phs.h"
#include "traffic_type.h"
#include "statistics.h"
#include "list.h"

/******************************************************************************************/
/**** FUNCTION: assign_channel_generic                                                 ****/
/**** This routine picks the channel that the current COMM_REQUEST event is assigned   ****/
/**** in the specified cell.  If the channel assignment algorithm cannot successfully  ****/
/**** assign a channel  the return value is determined as follows:                     ****/
/****     BLOCKED_PHYS_CHAN  if blocked due to lack of physical channels               ****/
/****     BLOCKED_SIR_CS     if blocked due to lack of signal quality at CS            ****/
/****                                                                                  ****/
/**** The following algorithms are implemented:                                        ****/
/****     SIRDCA:    channels must have SIR > sir_thr, chan with highest SIR selected  ****/
/****     IntDCA:    channels must have INT < int_thr, chan with lowest  INT selected  ****/
/****     SIRIntDCA: channels must have (SIR > sir_thr) AND (INT < int_thr),           ****/
/****                chan with lowest  INT selected                                    ****/
/****     IntSIRDCA: channels must have (SIR > sir_thr) AND (INT < int_thr),           ****/
/****                chan with highest SIR selected                                    ****/
/******************************************************************************************/
void PHSSectorClass::assign_channel_generic(PHSNetworkClass *np, EventClass *event, int ps_best_channel, int *channel_list, int algorithm)
{
    int i, k, n, done, unused_freq_idx;
    int possible_slot, possible_freq;
    int slot, freq, channel;
    int call_idx, num_attempt;
    int num_phys_chan = 0;
    int start_i = -1;
    int found;
    double sir, interference, signal_quality, *max_signal_quality_list;
    CallClass   *call;

    num_attempt = ((PHSTrafficTypeClass *) np->traffic_type_list[event->traffic_type_idx])->get_num_attempt(event->event_type);
    max_signal_quality_list = DVECTOR(num_attempt);

    for (i=0; i<=num_attempt-1; i++) {
        max_signal_quality_list[i] = -1.0;
        channel_list[i] = BLOCKED_PHYS_CHAN;
    }
    if (call_list->getSize() < num_physical_tx*np->num_slot-1) {
        for (slot=0; slot<=np->num_slot-1; slot++) {
            possible_slot = 1;
            n = ( (slot == cntl_chan_eff_tch_slot) ? 1 : 0);
            if (n == num_physical_tx) {
                possible_slot = 0;
            }
            for (call_idx=0; (call_idx<=call_list->getSize()-1) && (possible_slot); call_idx++) {
                call = (CallClass *) (*call_list)[call_idx];
                if ( ((call->channel) & ((1<<np->bit_slot)-1)) == slot) {
                    n++;
                    if (n == num_physical_tx) {
                        possible_slot = 0;
                    }
                }
            }
            if (possible_slot) {
                for (freq=0; freq<=np->num_freq-1; freq++) {
                    channel = (freq << np->bit_slot) | slot;
                    possible_freq = 1;

#if (REUSE_CHAN_SECTOR == 0)
                    int i_sector_idx;
                    SectorClass *i_sector;
                    for (i_sector_idx=0; (i_sector_idx<=parent_cell->num_sector-1) && (possible_freq); i_sector_idx++) {
                        i_sector = parent_cell->sector_list[i_sector_idx];
                        for (call_idx=0; (call_idx<=i_sector->call_list->getSize()-1) && (possible_freq); call_idx++) {
                            call = (CallClass *) (*(i_sector->call_list))[call_idx];
                            if (call->channel == channel) {
                                possible_freq = 0;
                            }
                        }
                    }
#else
                    for (call_idx=0; (call_idx<=call_list->getSize()-1) && (possible_freq); call_idx++) {
                        call = (CallClass *) (*call_list)[call_idx];
                        if (call->channel == channel) {
                            possible_freq = 0;
                        }
                    }
#endif
                    if (freq == np->cntl_chan_freq) {
                        possible_freq = 0;
                    }

                    for (unused_freq_idx=0; (unused_freq_idx<=num_unused_freq-1) && (possible_freq); unused_freq_idx++) {
                        if (unused_freq[unused_freq_idx] == freq) {
                            possible_freq = 0;
                        }
                    }

                    if (possible_freq) {
                        num_phys_chan++;
                        sir = comp_sir_cs(np, event->posn_x, event->posn_y, channel, &interference);
                        switch (algorithm) {
                            case CConst::SIRDCA:
                                if (sir < np->sir_threshold_call_request_cs) {
                                    possible_freq = 0;
                                } else {
                                    signal_quality = sir;
                                }
                                break;
                            case CConst::IntDCA:
                                if (interference > np->int_threshold_call_request_cs) {
                                    possible_freq = 0;
                                } else {
                                    signal_quality = -interference;
                                }
                                break;
                            case CConst::SIRIntDCA:
                                if ((sir < np->sir_threshold_call_request_cs) || (interference > np->int_threshold_call_request_cs) ) {
                                    possible_freq = 0;
                                } else {
                                    signal_quality = -interference;
                                }
                                break;
                            case CConst::IntSIRDCA:
                                if ((sir < np->sir_threshold_call_request_cs) || (interference > np->int_threshold_call_request_cs) ) {
                                    possible_freq = 0;
                                } else {
                                    signal_quality = sir;
                                }
                                break;
                            default: CORE_DUMP; break;
                        }
                    }

                    if (np->stat->plot_event) {
                        if (channel == ps_best_channel) {
                            if (possible_freq) {
                                if ( (algorithm == CConst::IntDCA) || (algorithm == CConst::SIRIntDCA) ) {
                                    fprintf(np->stat->fp_event, "CS: BEST_CHANNEL INTERFERENCE = %12.10e\n", -signal_quality);
                                } else {
                                    fprintf(np->stat->fp_event, "CS: BEST_CHANNEL SIR = %12.10e\n", signal_quality);
                                }
                            } else {
                                fprintf(np->stat->fp_event, "CS: BEST_CHANNEL NOT AVAILABLE\n");
                            }
                        }
                    }

                    if (possible_freq) {
                        i = 0;
                        done = 0;
                        while( (i<=num_attempt-1)&&(!done) ) {
                            if (    (channel == ps_best_channel) || (channel_list[i] < 0)
                                 || ((channel_list[i] != ps_best_channel) && (signal_quality > max_signal_quality_list[i])) ) {
                                for (k=num_attempt-1; k>=i+1; k--) {
                                    max_signal_quality_list[k] = max_signal_quality_list[k-1];
                                    channel_list[k] = channel_list[k-1];
                                }
                                max_signal_quality_list[i] = signal_quality;
                                channel_list[i] = channel;
                                done = 1;
                            } else {
                                i++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (num_phys_chan) {
        found = 0;
        for (i=0; (i<=num_attempt-1)&&(!found); i++) {
            if (channel_list[i] == BLOCKED_PHYS_CHAN) {
                found = 1;
                start_i = i;
            }
        }

        if (found) {
            for (i=start_i; (i<=num_attempt-1)&&(i<=start_i+num_phys_chan-1); i++) {
                channel_list[i] = BLOCKED_SIR_CS;
            }
        }
    }

    if (np->stat->plot_event) {
        for (i=0; i<=num_attempt-1; i++) {
            if (channel_list[i] == BLOCKED_PHYS_CHAN) {
                fprintf(np->stat->fp_event, "CS: CHANNEL[%d] = BLOCKED_PHYS_CHANNEL\n", i);
            } else if (channel_list[i] == BLOCKED_SIR_CS) {
                fprintf(np->stat->fp_event, "CS: CHANNEL[%d] = BLOCKED_SIR_CS\n", i);
            } else {
                if ( (algorithm == CConst::IntDCA) || (algorithm == CConst::SIRIntDCA) ) {
                    fprintf(np->stat->fp_event, "CS: CHANNEL[%d] = %d (%d,%d) INTERFERENCE = %12.10e\n", i, channel_list[i],
                        (channel_list[i] >> np->bit_slot),
                        (channel_list[i]) & ((1<<np->bit_slot)-1), -max_signal_quality_list[i]);
                } else {
                    fprintf(np->stat->fp_event, "CS: CHANNEL[%d] = %d (%d,%d) SIR = %12.10e\n", i, channel_list[i],
                        (channel_list[i] >> np->bit_slot),
                        (channel_list[i]) & ((1<<np->bit_slot)-1), max_signal_quality_list[i]);
                }
            }
        }
    }


#if 0
    printf("POSN: %12.10f %12.10f\n", event->posn_x, event->posn_y);
    for (i=0; i<=num_attempt-1; i++) {
        printf("CHANNEL: %d SIGNAL_QUALITY %12.10f\n", channel_list[i], max_signal_quality_list[i]);
    }
#endif

    free(max_signal_quality_list);

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: assign_channel_melco                                                   ****/
/**** This routine picks the channel that the current COMM_REQUEST event is assigned   ****/
/**** in the specified cell using the algorithm used by the MELCO 500 mW base station. ****/
/**** If the channel assignment algorithm cannot successfully  assign a channel, the   ****/
/**** return value is determined as follows:                                           ****/
/****     BLOCKED_PHYS_CHAN  if blocked due to lack of physical channels               ****/
/****     BLOCKED_SIR_CS     if blocked due to sir below threshold at CS               ****/
/******************************************************************************************/
void PHSSectorClass::assign_channel_melco(PHSNetworkClass *np, EventClass *event, int *channel_list)
{
    int i, k, n, done, unused_freq_idx;
    int possible_slot, possible_freq;
    int slot, freq, channel;
    int call_idx, i_sector_idx, num_attempt;
    double sir, interference, *min_cs_int_list;
    SectorClass *i_sector;
    CallClass   *call;

    num_attempt = ((PHSTrafficTypeClass *) np->traffic_type_list[event->traffic_type_idx])->get_num_attempt(event->event_type);
    min_cs_int_list     = DVECTOR(num_attempt);

    for (i=0; i<=num_attempt-1; i++) {
        min_cs_int_list[i] = -1.0;
        channel_list[i] = BLOCKED_PHYS_CHAN;
    }
    if (call_list->getSize() < num_physical_tx*np->num_slot-1) {
        for (slot=0; slot<=np->num_slot-1; slot++) {
            possible_slot = 1;
            n = ( (slot == cntl_chan_eff_tch_slot) ? 1 : 0);
            if (n == num_physical_tx) {
                possible_slot = 0;
            }
            for (call_idx=0; (call_idx<=call_list->getSize()-1) && (possible_slot); call_idx++) {
                call = (CallClass *) (*call_list)[call_idx];
                if ( ((call->channel) & ((1<<np->bit_slot)-1)) == slot) {
                    n++;
                    if (n == num_physical_tx) {
                        possible_slot = 0;
                    }
                }
            }
            if (possible_slot) {
                for (freq=0; freq<=np->num_freq-1; freq++) {
                    channel = (freq << np->bit_slot) | slot;
                    possible_freq = 1;

#if (REUSE_CHAN_SECTOR == 0)
                    for (i_sector_idx=0; (i_sector_idx<=parent_cell->num_sector-1) && (possible_freq); i_sector_idx++) {
                        i_sector = parent_cell->sector_list[i_sector_idx];
                        for (call_idx=0; (call_idx<=i_sector->call_list->getSize()-1) && (possible_freq); call_idx++) {
                            call = (CallClass *) (*(i_sector->call_list))[call_idx];
                            if (call->channel == channel) {
                                possible_freq = 0;
                            }
                        }
                    }
#else
                    for (call_idx=0; (call_idx<=sector->call_list->getSize()-1) && (possible_freq); call_idx++) {
                        call = (CallClass *) (*(sector->call_list))[call_idx];
                        if (call->channel == channel) {
                            possible_freq = 0;
                        }
                    }
#endif
                    if (freq == np->cntl_chan_freq) {
                        possible_freq = 0;
                    }

                    for (unused_freq_idx=0; (unused_freq_idx<=num_unused_freq-1) && (possible_freq); unused_freq_idx++) {
                        if (unused_freq[unused_freq_idx] == freq) {
                            possible_freq = 0;
                        }
                    }

                    if (possible_freq) {
                        sir = comp_sir_cs(np, event->posn_x, event->posn_y, channel, &interference);
                        i = 0;
                        done = 0;
                        while( (i<=num_attempt-1)&&(!done) ) {
                            if (interference < min_cs_int_list[i]) {
                                for (k=num_attempt-1; k>=i+1; k--) {
                                    min_cs_int_list[k] = min_cs_int_list[k-1];
                                    channel_list[k] = channel_list[k-1];
                                }
                                min_cs_int_list[i] = interference;
                                channel_list[i] = channel;
                                done = 1;
                            } else {
                                i++;
                            }
                        }
                    }
                }
            }
        }
    }

    done = 0;
    for (i=0; (i<=num_attempt-1)&&(!done); i++) {
        if (channel_list[i] == BLOCKED_PHYS_CHAN) {
            done = 1;
        } else if (min_cs_int_list[i] < np->int_threshold_call_request_cs) {
            channel_list[i] = BLOCKED_SIR_CS;
        }
    }

#if 0
    printf("POSN: %12.10f %12.10f\n", event->posn_x, event->posn_y);
    for (i=0; i<=num_attempt-1; i++) {
        printf("CHANNEL: %d SIR %12.10f\n", channel_list[i], min_cs_int_list[i]);
    }
#endif

    free(min_cs_int_list);

    return;
}
/******************************************************************************************/

/******************************************************************************************/
int PHSNetworkClass::get_ps_best_channel(EventClass *event)
{
    int found, best_channel, slot, freq, channel;
    double min_interference, interference;

    found = 0;
    best_channel = -1;
    min_interference = 0.0;

    for (slot=0; (slot<=num_slot-1)&&(!found); slot++) {
        for (freq=0; freq<=num_freq-1; freq++) {
            if (freq != cntl_chan_freq) {
                channel = (freq << bit_slot) | slot;
                comp_sir_ps(-1, -1, event->posn_x, event->posn_y, channel, &interference);
                if ( (interference < min_interference) || (best_channel == -1) ) {
                    min_interference = interference;
                    best_channel = channel;
                    if (min_interference == 0.0) { found = 1; }
                }
            }
        }
    }

    if (stat->plot_event) {
        fprintf(stat->fp_event, "PS_BEST_CHANNEL = %d (%d,%d) INTERFERENCE = %12.10e\n", best_channel,
            (best_channel >> bit_slot),
            (best_channel) & ((1<<bit_slot)-1), min_interference);
    }

    return(best_channel);
}
/******************************************************************************************/
