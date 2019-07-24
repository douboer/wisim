/******************************************************************************************/
/**** PROGRAM: wlan_monte_carlo.cpp                                                    ****/
/****   - chengan douboer@gmail.com                                                    ****/
/******************************************************************************************/
#include <string.h>
#include <iostream>
#include <fstream>

#include "cconst.h"
#include "list.h"
#include "wlan.h"
#include "randomc.h"
#include "wlan_statistics.h"

#if HAS_GUI
#include "gconst.h"
#endif

const int plot_ac_delay = 1;

int WLANNetworkClass::current_event_idx = 0;

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::reset_base_stations                                  ****/
/******************************************************************************************/
void WLANNetworkClass::reset_base_stations(int option)
{

}

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::create_call_stat_count                               ****/
/******************************************************************************************/
StatCountClass * WLANNetworkClass::create_call_stat_count() {
    return new WLANStatCountClass();
}


/******************************************************************************************
 *   FUNCTION: WLANSessionClass::WLANSessionClass()
 *****************************************************************************************/
double WLANSessionClass::tx_pwr_mw = 100;
WLANSessionClass::WLANSessionClass()
{
    num_segment  = 100;
    seg_cnt_left = 100;

    //tx_pwr_mw = 100;  // Usually, WLAN equipment has an output power of 15 dBm (about 30 mW)
    tx_body_gain = -2;  // common (-3 ~ -10 ) dB
    tx_ant_gain  =  0;
    rx_body_gain = -2;
    rx_ant_gain  =  0;
}

/******************************************************************************************
 *   FUNCTION: WLANSessionClass::~WLANSessionClass()
 *****************************************************************************************/
WLANSessionClass::~WLANSessionClass()
{
}


/******************************************************************************************
 * DESCRIPTION: Compute signal in random position (x,y), which is radiated by a user station
 *              (here, Session).
 * INPUT:       (x,y): position want to compute receive power
 *              power for all (x,y).
 * RETURN:      receive power in dBm 
 *****************************************************************************************/
double WLANSessionClass::receive_pwr(WLANNetworkClass *wnp, int x, int y)
{

    WLANExpoPM* wpm = wnp->get_pm();

    double rx_pwr_dbm  = 0.0;
    double path_gain   = 0.0;
    //double dx = x - posn_x;
    //double dy = y - posn_y;
    //int dist = (int) sqrt(dx*dx+dy*dy);


#if 0
    int num_prop_model = np->num_prop_model;

    if ( num_prop_model > 0 ) {
        // Select a proper propagation model for terminal ??
        prop_model = 0;
    }
    else
    {
        prop_model = -1;
        return rx_pwr_dbm;
    }
#endif
    if ( !wnp->get_pm() )
        return rx_pwr_dbm;

    path_gain = wnp->get_pm()->prop_power_loss((NetworkClass*)wnp, this, x, y);
    double EiRP = 10.0*log(tx_pwr_mw)/log(10.0) + tx_body_gain + tx_ant_gain;
    rx_pwr_dbm = EiRP + path_gain;


    // DEBUG
#if 0
    std::cout << "tx_body_gain "  << tx_body_gain << std::endl;
    std::cout << "tx_ant_gain  "  << tx_ant_gain  << std::endl;
    std::cout << "tx_pwr_mw    "  << tx_pwr_mw    << std::endl;
    std::cout << "path_gain    "  << path_gain    << std::endl;
    std::cout << "EiRP         "  << EiRP         << std::endl;
    std::cout << "rx_pwr_dbm   "  << rx_pwr_dbm   << std::endl;
#endif

    return rx_pwr_dbm;
}

//The energy detect (ED) threshold is used to detect any other type of RF transmissions
//  during the clear channel assessment (CCA)
double WLANSessionClass::get_cca()
{
    return (-85.0-5.0*log(tx_pwr_mw/100.0)/log(10.0));
}



/******************************************************************************************/
/**** FUNCTION: NetworkClass::gen_event                                                ****/
/******************************************************************************************/
static bool cal_tau_fg = true;
void WLANNetworkClass::gen_event(EventClass *event)
{
    int i, flag, traffic_type_idx, subnet_idx;
    double total_rate, sum, r;
    WLANTrafficTypeClass *traffic_type;
    SubnetClass *subnet;

    //  If we cannot find the event type in this routine, let the type of event "request",
    //  or the master index will not be choosed, so the master_idx value is not correct,
    //  which will make the application crash
    //  XXXX
    //event->event_type = CConst::RequestEvent;

    WLANNetworkClass::current_event_idx ++;

    //  Need to be modified.
    //  Suppose that all traffic types have equel chance to arrive.
    //  As a first step, we get parameters from simple GUI.
    if ( num_traffic_type == 1 )
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[0];
    else if ( num_traffic_type > 1 ) {
#if MC_DBG
        if ( WLANNetworkClass::service_type == VoiceService ) {
            // use the "Voice" parameters defined in .geom file
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
            // use the "Data" parameters defined in .geom file
        }
#endif
        traffic_type_idx = (int) floor(rg->Random()*num_traffic_type);
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
    }

    double mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
    double mean_session             = traffic_type->mean_session;
    double mean_segment             = traffic_type->mean_segment;
    double mean_read_time           = traffic_type->mean_read_time;
    int    min_seg_size             = traffic_type->min_seg_size;
    int    max_seg_size             = traffic_type->max_seg_size;
    double down_up_ratio            = traffic_type->down_up_ratio;
    double mean_seg_read_total      = mean_session / mean_num_seg_per_session;

    int cell_idx = 0;
    int call_idx = 0;
    int ap_idx   = 0;

    std::vector <int> traffic_idx_ap_list;
    std::vector <int> num_traffic_type_list;
    double* ap_lambda_traffic_type_total = (double*) NULL;

    int num_traffic_ap_type = 0;
    int num_speci_traffic   = 0;
    bool   found = false;
    double error = 1E-5;

    WLANCellClass *cell = (WLANCellClass*) NULL;
    WLANSectorClass *ap = (WLANSectorClass*) NULL;


    /* get tau values under no interference
     * we can find tau by number of users under an AP
     * to speed up simulation process
     * only execute one time
     */
    if ( cal_tau_fg ) {
        int nu;
        double tau;
        double p_cond_coll;
        double err = 1E-10;
        tau_nu_no_inter.clear();
        for ( nu=0; nu<ac_num_user+2; nu++) {
            tau_iteration_no_inter( nu, p_cond_coll, tau, err);
            tau_nu_no_inter.push_back(tau);
#if !MC_DBG
            std::cout << "tau_nu_no_inter " << tau_nu_no_inter[nu] << std::endl;
#endif
        }

        cal_tau_fg = false;
    }

    // calculate tau_user_inter & p_cond_coll_user_inter before event comming
    // tau_user_inter => lambda_segment_user() => lambda_segment_net()
    tau_with_inter (1E-10);

    /*  time to next exponential event has mean:
     *  1 / ( SUM expo_arrival_rates + SUM N/expo_holding_times + SUM expo_drop_rates) 
     *
     *  The following loops get total arrival rate which is include request, drop, hangup.
     *  So, we can derive the random duration time of the coming event.
     ***********************************************************************************/

    // Sum arrival rate of Request event of an session in data service Sim.
    total_rate = total_arrival_rate;

    // Sum arrival rate of Segment Request event in data service Sim.
    // Generating Segment Request event is only for statistics.
    // We can do this statistics by recording the segments number generaed by
    // each session, which make the simulation more efficiency.
#if 0
    if ( WLANNetworkClass::service_type == DataService )
        total_rate += mean_num_seg_per_session * total_arrival_rate;
#endif

    // Sum arrival rate of Voice Service Drop event
    for ( cell_idx=0; cell_idx<num_cell && WLANNetworkClass::service_type == VoiceService; cell_idx++) {
        cell = (WLANCellClass*) cell_list[cell_idx];
        for ( ap_idx=0; ap_idx<cell->num_sector; ap_idx++ ) {
            ap = (WLANSectorClass*) cell->sector_list[ap_idx];
            total_rate += 1/drop_mean_duration_ap( ap, ap_lambda_traffic_type_total, 0 );

            //std::cout << "drop_mean_duration_ap( ap, ap_lambda_traffic_type_total, 0 ) " 
            //          << drop_mean_duration_ap( ap, ap_lambda_traffic_type_total, 0 ) << std::endl;
        }
    }

    // Sum arrival rate of Hangup event of voice service
    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1
            && WLANNetworkClass::service_type == VoiceService; traffic_type_idx++) {
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        if (traffic_type->duration_dist == CConst::ExpoDist) {
            total_rate += num_call_type[traffic_type_idx] / traffic_type->mean_session_duration;
        }
    }

    // DATA
    // Sum arrival rate of Hangup event of data service
    if ( WLANNetworkClass::service_type == DataService )
    {
        total_rate += lambda_segment_net();
    }


    /* The ploblem is that if the SegmentRequest, SegmentHangup and SessionRequest are not
     * independent in data service simulaiton, we cannot use exponential distribution to 
     * generate duration of an event.
     * I suppose that the above three R.V. are independent for the
     * milestone of wlan data service simulation development.
     *************************************************************************************/
    r = rg->Random();
    event->time = -log(r) / total_rate;


    // XXX
    //std::cout << "r value " << r << std::endl;
    //std::cout << "total_rate " << total_rate << std::endl;


    /*  The following loops Judge which type of the coming event by probability of 
     *  correspanding event type.
     *************************************************************************************/

    if ( (num_pending_event) && (abs_time + event->time >= pending_event_list[0]->time) ) {
        event->copy(pending_event_list[0], abs_time);
        delete pending_event_list[0];
        for (i=0; i<=num_pending_event-2; i++) {
            pending_event_list[i] = pending_event_list[i+1];
        }
        num_pending_event--;
    }
    else if ( master_call_list->getSize() == 0 ) {
        traffic_type_idx = (int) floor(rg->Random()*num_traffic_type);
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

        subnet_idx = (int) floor(rg->Random()*num_subnet[traffic_type_idx]);
        subnet = subnet_list[traffic_type_idx][subnet_idx];

        event->traffic_type_idx = traffic_type_idx;
        event->event_type = CConst::RequestEvent;

#if MC_DBG
        std::cout << "Session/Call Request Event (FIRST) \n";
#endif
        gen_event_location(event, subnet_idx);
    }
    else {
        r = rg->Random() * total_rate;

        flag = 0;
        sum  = 0.0;

        for (traffic_type_idx=0; (traffic_type_idx<=num_traffic_type-1)&&(!flag); traffic_type_idx++) {
            traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

            // Call(VOICE)/ Session(DATA) Request event
            for (subnet_idx=0; (subnet_idx<=num_subnet[traffic_type_idx]-1) && !flag ; subnet_idx++) {
                subnet = subnet_list[traffic_type_idx][subnet_idx];
                sum += subnet->arrival_rate;
                if (r <= sum) {
                    event->traffic_type_idx = traffic_type_idx;
                    event->event_type = CConst::RequestEvent;

#if MC_DBG
                    std::cout << "Request Event \n";
#endif
                    gen_event_location(event, subnet_idx);

                    flag = 1;
                }
            }

            /* Segment(DATA) Request event
             * Just for statistics, so we can realize the function by recording the random
             * segment number for each comming session.
             *************************************************************************/
#if 0
            for (subnet_idx=0; (subnet_idx<=num_subnet[traffic_type_idx]-1) && !flag
                    && WLANNetworkClass::service_type == DataService; subnet_idx++) {
                subnet = subnet_list[traffic_type_idx][subnet_idx];
                // Suppose
                sum += mean_num_seg_per_session * subnet->arrival_rate;

                if (r <= sum) {
                    event->traffic_type_idx = traffic_type_idx;
                    event->event_type   = CConst::SegmentRequestEvent;
#if MC_DBG
                    event->master_idx = (int) floor(rg->Random()*master_call_list->getSize());
                    std::cout << " event->master_idx " << event->master_idx << std::endl;
#endif
                    do {
                        event->master_idx = (int) floor(rg->Random()*master_call_list->getSize());
                    } while( ((WLANSessionClass *) (*master_call_list)[event->master_idx])
                            ->traffic_type_idx != traffic_type_idx);

#if MC_DBG
                    std::cout << "Segment Request Event \n";
#endif

                    flag = 1;
               }
            }
#endif

            /* Hangup Event : 
             *   For Voice Service it is hangup of a call;
             *   For Data Service it is hangup of a Segment, i.e. finish of a segment transmition;
             * Here for Voice Service.
             ***************************************************************************************/
            if ( traffic_type->duration_dist == CConst::ExpoDist && !flag && WLANNetworkClass::service_type == VoiceService ) {
                sum += num_call_type[traffic_type_idx] / traffic_type->mean_session_duration;
                if (r <= sum) {
                    event->traffic_type_idx = traffic_type_idx;
                    event->event_type = CConst::HangupEvent;

                    do {
                        event->master_idx = (int) floor(rg->Random()*master_call_list->getSize());
                    } while( ((WLANSessionClass *) (*master_call_list)[event->master_idx])->traffic_type_idx != traffic_type_idx
                            || event->master_idx >= master_call_list->getSize() );

#if !MC_DBG
                    std::cout << "Voice Hangup Event \n";

#if 0
                    WLANSessionClass* call = (WLANSessionClass *) (*master_call_list)[event->master_idx];
                    cell = (WLANCellClass*) cell_list[call->cell_idx];
                    ap   = (WLANSectorClass*) cell->sector_list[call->sector_idx];

                    std::cout << "GEN EVENT FUNCTION           " << std::endl;
                    std::cout << "event->master_idx(HANGUP)    " << event->master_idx        << std::endl;
                    std::cout << "call ->master_idx            " << call->master_idx         << std::endl;
                    std::cout << "call->call_idx               " << call->call_idx           << std::endl;
                    std::cout << "ap->call_list->getSize()     " << ap->call_list->getSize() << std::endl;
                    std::cout << std::endl;
#endif
#endif

                    flag = 1;
                }
            }
        }

        /*  DATA
         *  Loop for Data service Hangup Event and Voice Service Drop Event.
         *******************************************************************************************/
        for ( cell_idx=0; cell_idx<num_cell && !flag; cell_idx++) {
            cell = (WLANCellClass*) cell_list[cell_idx];
            for ( ap_idx=0; ap_idx<cell->num_sector; ap_idx++ ) {
                ap = (WLANSectorClass*) cell->sector_list[ap_idx];

                traffic_type_idx    = 0;
                num_traffic_ap_type = 0;
                num_speci_traffic   = 0;
                traffic_idx_ap_list.clear();
                num_traffic_type_list.clear();

                // Find number and type of traffic in an AP.
                for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
                    found = false;
                    num_speci_traffic = 0;
                    for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
                        WLANSessionClass* call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                        if ( call->traffic_type_idx == traffic_type_idx ) {
                            found = true;
                            num_speci_traffic ++;
                        }
                    }
                    if ( found ) {
                        num_traffic_ap_type ++;
                        traffic_idx_ap_list.push_back( traffic_type_idx );
                        num_traffic_type_list.push_back( num_speci_traffic );
                    }
                }

                if ( WLANNetworkClass::service_type == VoiceService ) {
                    // Get Voice service Drop Event lambda list of traffic type under an AP, i.e. ap_lambda_traffic_type_total.
                    ap_lambda_traffic_type_total = (double*) malloc( num_traffic_ap_type*sizeof(double) );
                    drop_mean_duration_ap( ap, ap_lambda_traffic_type_total, 1 );

#if MC_DBG
                    std::cout << "ap_lambda_traffic_type_total[0] " << ap_lambda_traffic_type_total[0] << std::endl;
#endif
                }

                // Judge the Event type.
                int seg_hangup_flg = 0;
                for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type && (!flag); traffic_type_idx++ ) {

                    // Sum Voice service Drop Event
                    if ( WLANNetworkClass::service_type == VoiceService ) {
                        sum += ap_lambda_traffic_type_total[traffic_type_idx];

                        // XXX
                        //std::cout << "sum " << sum << std::endl;
                        //std::cout << "ap_lambda_traffic_type_total[traffic_type_idx] " << ap_lambda_traffic_type_total[traffic_type_idx] << std::endl;

                        if ( r <= sum ) {
                            event->traffic_type_idx = traffic_idx_ap_list[traffic_type_idx];
                            event->cs_idx = cell_idx;
                            event->event_type = CConst::DropEvent;
#if !MC_DBG
                            std::cout << "Voice Drop Event \n";
#endif
                            flag = 1;
                        }
                    }

                    // Sum Data service Hangup Event
                    WLANSessionClass* call;
                    if ( WLANNetworkClass::service_type == DataService ) {
                        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
                            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                            sum += lambda_segment_user(call);
                            if ( r <= sum ) {
                                event->traffic_type_idx = traffic_idx_ap_list[traffic_type_idx];
                                event->cs_idx = cell_idx;
                                event->event_type = CConst::SegmentHangupEvent;

                                flag = 1;
                                seg_hangup_flg = 1;
                            }
                        }
                    }

                    if ( seg_hangup_flg == 1 )
                    {
                        event->master_idx = call->master_idx;
                    }
                    else if ( flag )
                    {
                        do {
                            // If we cannot find the special index of traffic type, do what?
                            // Maybe we shuold consider this condition when there are more than 
                            // one type of traffic in WLAN network.
                            call_idx = (int) floor(rg->Random()*(ap->call_list)->getSize());
                            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                            event->master_idx = call->master_idx;
                        }
                        while( call->traffic_type_idx != traffic_idx_ap_list[traffic_type_idx]
                                || event->master_idx >= master_call_list->getSize() );
                    }
                }

                free ( ap_lambda_traffic_type_total ); ap_lambda_traffic_type_total = NULL;
            }
        }
    }

    return;
}


/******************************************************************************************
 * DESCRIPTION: Calculate the the mean time to drop of a user's call for voice service.
 * INPUT:       ap
 *              ap_lambda_traffic_type_total: list of total lambda of all calls/users with 
 *                                            same type of traffic under an AP.
 *                                            with size number of traffic type of AP.
 *              option: 0 --  do not put lambda values to ap_lambda_traffic_type_total list;
 *                      1 --  put lambda values into ap_lambda_traffic_type_total list.
 * RETURN:      Mean time to drop of an AP.
 * NOTE:        Distribution of a traffic type must be exponential distribution in this implement.
 *              We can calculate mean time to drop of an AP for non exponential distribution
 *              conditions, if we can derived the joint distribution of multi-users from one
 *              call distribution.
 *****************************************************************************************/
double WLANNetworkClass::drop_mean_duration_ap( WLANSectorClass* ap, double* ap_lambda_traffic_type_total, int option )
{
    double user_data_rate = 64;   // in Mbps
    double basic_rate     = 1 ;   // in kbps
    double phy_rate       = 11;   // in Mbps
    double hd_rate        = 2 ;   // in Mbps

    double mean_drop_duration       = 1200;
    double mean_session_duration    = 100;
    double mean_num_seg_per_session = 10;
    double mean_segment             = 20;
    double mean_read_time           = 20;
    double mean_session             = 200;
    double min_seg_size             = 5;
    double max_seg_size             = 5000;
    double down_up_ratio            = 9;
    double mean_seg_read_total      = mean_session / mean_num_seg_per_session;

    int     drop_error_pkt  = 0;
    int     drop_total_pkt  = 0;
    int     bit_per_pkt     = 0;
//  double  lambda_drop     = 0.001;                      // lambda_drop = 1 / (mean_session_duration()).
    double  total_lambda_ap = 0.0;
    double  drop_md_ap      = 0.0;
    double  mean_T          = 0.0;
    double* p_drop_list     = (double*) NULL;
    double* lambda_drop_ap  = (double*) NULL;

    WLANSessionClass *call = (WLANSessionClass*) NULL;
    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;
    std::vector <WLANTrafficTypeClass*> traffic_ap_list;  // a list of traffic type under special AP.
    std::vector <int> num_traffic_type_list;              // a list of number of one traffic type call.

    int call_idx = 0;
    int traffic_type_idx = 0;
    int num_traffic_ap_type = 0;
    int num_speci_traffic   = 0;

    bool found;
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;

        mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
        mean_session             = traffic_type->mean_session;
        mean_segment             = traffic_type->mean_segment;
        mean_read_time           = traffic_type->mean_read_time;
        min_seg_size             = traffic_type->min_seg_size;
        max_seg_size             = traffic_type->max_seg_size;
        down_up_ratio            = traffic_type->down_up_ratio;
        mean_seg_read_total      = mean_session / mean_num_seg_per_session;

        found = false;
        num_speci_traffic = 0;
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            if ( call->traffic_type_idx == traffic_type_idx )
                found = true;
                num_speci_traffic ++;
        }
        if ( found ) {
            num_traffic_ap_type ++;
            traffic_ap_list.push_back( traffic_type );
            num_traffic_type_list.push_back(num_speci_traffic);
        }
    }

    p_drop_list    = (double*) malloc( num_traffic_ap_type*sizeof(double) );
    lambda_drop_ap = (double*) malloc( num_traffic_ap_type*sizeof(double) );

    prop_drop( ap, p_drop_list );

    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type; traffic_type_idx++ ) {
        traffic_type = traffic_ap_list[traffic_type_idx];

        drop_total_pkt        = traffic_type->drop_total_pkt;
        drop_error_pkt        = traffic_type->drop_error_pkt;
        bit_per_pkt           = traffic_type->bit_per_pkt;

        mean_drop_duration    = traffic_type->mean_drop_duration;
        mean_session_duration = traffic_type->mean_session_duration;

        // mean time to drop;
        // mean_T = ((double) (drop_total_pkt * bit_per_pkt)) / user_data_rate / 1000.0;
        mean_T = mean_drop_duration;
        lambda_drop_ap[traffic_type_idx] = -log(1-p_drop_list[traffic_type_idx]) / mean_T;

#if MC_DBG
        std::cout << "traffic_type_idx " << traffic_type_idx << std::endl;
        std::cout << "mean_T           " << mean_T         << std::endl;
        std::cout << "p_drop_list      " << p_drop_list[0] << std::endl;
        std::cout << "lambda_drop_ap   " << lambda_drop_ap[traffic_type_idx] << std::endl;
#endif

    }

    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type; traffic_type_idx++ ) {
        int num = num_traffic_type_list[traffic_type_idx];          // Call number of the special traffic type under given AP.
        double lambda_ap_type = num * lambda_drop_ap[traffic_type_idx];
        if ( option == 1 )
            ap_lambda_traffic_type_total[traffic_type_idx] = lambda_ap_type;
        total_lambda_ap += lambda_ap_type;                          // The same kind of traffic have the same lambda value.
    }
    drop_md_ap = 1./total_lambda_ap;

#if MC_DBG
    std::cout << "Total lambda of drop event an AP : " << total_lambda_ap << std::endl;
#endif

    free ( p_drop_list );    p_drop_list    = (double*) NULL;
    free ( lambda_drop_ap ); lambda_drop_ap = (double*) NULL;

#if MC_DBG
    std::cout << "Mean time to drop of an AP : " << drop_md_ap << std::endl;
#endif

    return drop_md_ap;
}


/******************************************************************************************
 * DESCRIPTION: Calculate the probability of a drop event to an user/call.
 *              Can be derived from prop_pkt_loss.
 *              surpose that drop_error_pkt outof drop_total_pkt packet loss => a call drop
 * INPUT        p_drop_list: a list of drop propability of correspanding traffic type under an AP
 * RETURN:      A list of drop probability of correspanding traffic type under an AP.
 *****************************************************************************************/
double* WLANNetworkClass::prop_drop( WLANSectorClass* ap, double* p_drop_list )
{
    double error = 1E-10;
    //  int n = 128;
    int j = 5;
    int i = 1;
    // Defined as double type to avoid overflow in solve n!.
    //  double fn  = 1.;      // factorial n     -->  n!
    //  double fj  = 1.;      // factorial j     -->  j!
    //  double fnj = 1.;      // factorial (n-j) -->  (n-j)!
    //  double pnj = 1.;      // pnj = n*(n-1)(n-2)....(n-j+1)
    double  cnj      = 1.0;   // cnj = fn / (fj*fnj) = pnj/fj
    double  pdrop    = 0.0;
    double  p_pkt_i  = 0.0;
    double  p_pkt_nj = 0.0;

    WLANSessionClass *call = (WLANSessionClass*) NULL;
    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;

    int    call_idx = 0;
    int    traffic_type_idx  = 0;
    int    num_speci_traffic = 0;
    int    num_traffic_ap_type = 0;                       // number of traffic type under specified AP.
    std::vector <WLANTrafficTypeClass*> traffic_ap_list;  // a list of traffic type under specified AP.
    std::vector <int> num_traffic_type_list;              // a list of number of one traffic type call.

    // No user under an AP
    if ( ap->call_list->getSize() == 0 )
    {
        return 0;
    }

    bool found;
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        found = false;
        num_speci_traffic = 0;
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            if ( call->traffic_type_idx == traffic_type_idx )
            {
                found = true;
                num_speci_traffic ++;
            }
        }
        if ( found ) {
            num_traffic_ap_type ++;
            traffic_ap_list.push_back( traffic_type );
            num_traffic_type_list.push_back(num_speci_traffic);
        }
    }

    double* p_cond_coll = (double*) malloc ( num_traffic_ap_type*sizeof(double) );
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type; traffic_type_idx++ )
        p_cond_coll[traffic_type_idx] = 0.25;

    double pkt_loss_rate = 0.0;
    //prop_pkt_loss( ap, p_cond_coll, error );
    throughput_computation( ap,pkt_loss_rate,error );

    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type; traffic_type_idx++ ) {
        int drop_error_pkt = traffic_ap_list[traffic_type_idx]->drop_error_pkt;
        int drop_total_pkt = traffic_ap_list[traffic_type_idx]->drop_total_pkt;

        for ( j=drop_error_pkt; j <= drop_total_pkt; j++ ) {
            double p   = 1.;
            double fj  = 1.;
            double pnj = 1.;
            //  Use 'C(n,j) = C(n, n-j)' to reduce the calculation amount.
            if ( j > drop_total_pkt / 2 ) {
                for ( i=j+1; i<=drop_total_pkt; i++ )
                    pnj *= i;
                for ( i=2; i<=drop_total_pkt-j; i++ )
                    fj *= i;
            } else {
                for ( i=(drop_total_pkt-j+1); i<=drop_total_pkt; i++ )
                    pnj *= i;
                for ( i=2; i<=j; i++ )
                    fj *= i;
            }

            cnj = pnj / fj;
            p *= cnj;

            //p_pkt_i  = pow(p_cond_coll[traffic_type_idx], j);
            //p_pkt_nj = pow(1-p_cond_coll[traffic_type_idx], drop_total_pkt-j);
            p_pkt_i  = pow(pkt_loss_rate, j);
            p_pkt_nj = pow(1-pkt_loss_rate, drop_total_pkt-j);

            p *= (p_pkt_i*p_pkt_nj);

            pdrop += p;
        }
        if ( pdrop > 1.0 ) pdrop = 1-1E-20;

        p_drop_list[traffic_type_idx] = pdrop;

#if MC_DBG
        std::cout << "-----------------------------------------------" << std::endl;
        std::cout << "pkt_loss_rate xxxxx " << pkt_loss_rate << std::endl;
        std::cout << "p_drop_list[] xxxxx " << p_drop_list[traffic_type_idx] << std::endl;
#endif
    }

    free ( p_cond_coll ); p_cond_coll = (double*) NULL;

    return p_drop_list;
}


/******************************************************************************************
 * DESCRIPTION: Calculate the probability of single packet loss for correspanding traffic
 *              type and AP.
 * INPUT:       ap: PKT loss is a list that correlate with the amount of traffic type under
 *                  the AP. We can find the number of traffic types from the given AP.
 *              err: iteration precision
 * RETURN:      times of iteration
 * NOTE:        Distribution of a traffic type must be exponential distribution.
 * MODIFY:      8/3/2005 -- modify method of solving non-linear system.
 * NOTE:        xxxxxx  implement with tau_iteration_no_inter()
 *****************************************************************************************/
int WLANNetworkClass::prop_pkt_loss( WLANSectorClass* ap, double* p_cond_coll, double err )
{
    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;
    int  num_user = ap->call_list->getSize();

    double* p;
    double* tau;

    // keep the result of tau and p_cond_coll solved by function tau_iteration_no_inter()
    // and then all traffic type of tau and p_cond_coll equal to these value.
    double t_tau         = 0;
    double t_p_cond_coll = 0;

    WLANSessionClass *call;

    double multi    = 1.0;
    double delta    = 0.0;
    double sum_err  = 0.0;
    double sum_p    = 0.0;
    int    times    = 0;
    int    call_idx = 0;
    int    traffic_type_idx    = 0;
    int    num_speci_traffic   = 0;
    int    num_traffic_ap_type = 0;                      // number of traffic type under specified AP.

    std::vector <WLANTrafficTypeClass*> traffic_ap_list; // a list of traffic type under specified AP.
    std::vector <int> num_traffic_type_list;             // a list of number of one traffic type call.

    bool found;

    /*  If there is only one call/use on an AP, variable no_competition is true.
     *  If so,  probability of drop is 0.
     *  bool no_competition = false;
     *******************************************************************************/
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        found = false;
        num_speci_traffic = 0;
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            if ( call->traffic_type_idx == traffic_type_idx )
            {
                found = true;
                num_speci_traffic ++;
            }
        }
        if ( found ) {
            num_traffic_ap_type ++;
            traffic_ap_list.push_back( traffic_type );
            num_traffic_type_list.push_back(num_speci_traffic);
        }
    }

    /* If num_traffic_ap_type == 1 , size of p_cond_coll is 1, p_cond_coll[0] = 0;
     * if num_traffic_ap_type == 0 , size of p_cond_coll is 0, p_cond_coll = 0/NULL;
     * And the following do-while computation is no need.
     *******************************************************************************/
    if ( num_traffic_ap_type == 1 && ap->call_list->getSize() == 1 ) {
        p_cond_coll[0] = 0.0;
        return times;
    }
    else if ( num_traffic_ap_type == 0 ) {
        p_cond_coll = (double*) NULL;
        return times ;
    }

    p   = (double*) malloc ( num_traffic_ap_type*sizeof(double) );
    tau = (double*) malloc ( num_traffic_ap_type*sizeof(double) );

    tau_iteration_no_inter(num_user,t_p_cond_coll,t_tau,err );

    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_ap_type; traffic_type_idx++ ) {
        p_cond_coll[traffic_type_idx] = t_p_cond_coll;
    }

    free (p);   p   = (double*) NULL;
    free (tau); tau = (double*) NULL;

    return times;
}


/******************************************************************************************
 * DESCRIPTION: Calculate mean time of segment duration(or lambda = 1/duration) for
 *              correspanding call/user
 * INPUT:       call
 * RETURN:      session lambda of call
 * NOTE:        original function --> lambda_segment_traffic_type()
 * CREATE:      10/28/2005
 *****************************************************************************************/
double WLANNetworkClass::lambda_segment_user( WLANSessionClass* call )
{
    double lambda_seg_user  = 1.0;

    double multi            = 1.0;
    int    times            = 0;
    int    call_idx         = 0;  // index for loop
    int    cell_idx         = 0;  // index for loop
    int    ap_idx           = 0;  // index for loop
    int    mc_idx           = 0;  // master call index
    int    traffic_type_idx = 0;

    WLANCellClass* cell = NULL;
    WLANSectorClass* ap = NULL;
    WLANTrafficTypeClass* traffic_type = NULL;

    int cell_i = call->cell_idx;
    int sector_i = call->sector_idx;
    traffic_type_idx = call->traffic_type_idx;
    traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

    double phy_rate     = traffic_type->phy_rate; // in Mbps
    double mean_segment = traffic_type->mean_segment;

    double match_factor = 1.0;  // adjust factor value
    cell = (WLANCellClass*) cell_list[cell_i];
    for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++)
    {
        ap = (WLANSectorClass*) cell->sector_list[ap_idx];
        int nu_ap = ap->call_list->getSize();
        double multi = 1.0;
        for ( call_idx=0; call_idx<nu_ap; call_idx++ )
        {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            mc_idx = call->master_idx; // get master call index
            multi *= (1-tau_user_inter[mc_idx]);
        }
    }

    mc_idx = call->master_idx; // get master call index
    lambda_seg_user = match_factor
        * ( phy_rate * tau_user_inter[mc_idx]*multi/(1-tau_user_inter[mc_idx]))
        / mean_segment;

    return  lambda_seg_user;
}



/******************************************************************************************
 * DESCRIPTION: Calculate total lambda value of segment duration(or lambda = 1/duration)
 *              for network
 * INPUT:
 * RETURN:      session lambda of network
 * NOTE:        using function -> lambda_segmnet_user()
 * CREATE:      10/28/2005
 *****************************************************************************************/
double WLANNetworkClass::lambda_segment_net()
{
    double lambda_seg_total = 0.0;
    int mc_i = 0;  // master call index

    WLANCellClass* cell    = NULL;
    WLANSectorClass* ap    = NULL;
    WLANSessionClass* call = NULL;

    int num_call = master_call_list->getSize();
    for ( mc_i=0; mc_i<num_call; mc_i++ ) {
        // get list of tau_user_no_inter
        call = (WLANSessionClass *) (*master_call_list)[mc_i];
        lambda_seg_total += lambda_segment_user( call );
    }

    return  lambda_seg_total;
}


/******************************************************************************************
 * DESCRIPTION: calculate the the mean time to drop of a user
 * INPUT:       call: a session/call/user
 * RETURN:      mean time to drop of an user
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::drop_mean_duration_user( WLANSessionClass* call )
{
    int call_i = call->master_idx;
    int traffic_type_idx = call->traffic_type_idx;
    double packet_loss_thr       = 0.05;
    double duration_dropcall_thr = 100;
    double max_consecut_drop_pkt = 5;
    double duration_drop_call_i  = 0.0;

    WLANTrafficTypeClass* traffic_type =
        (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

    packet_loss_thr       = traffic_type->packet_loss_thr;
    duration_dropcall_thr = traffic_type->duration_dropcall_thr;
    max_consecut_drop_pkt = traffic_type->max_consecut_drop_pkt;

    duration_drop_call_i = (double) duration_dropcall_thr /
        pow(p_cond_coll_user_inter[call_i]/packet_loss_thr, max_consecut_drop_pkt);

    return duration_drop_call_i;
}


/******************************************************************************************
 * DESCRIPTION: Compute the delay for a traffic type of user transmit a segment
 *              of data under an AP.
 * INPUT:       ap, ad_seg_type_f & ad_seg_type_r descripte in .h file
 * OUTPUT:      ad_seg_type_f : a list of segment access delay of specific traffic type under an AP.
 * RETURN:      void
 * NOTE  :      If ad_seg_type_f or ad_seg_type_r is NULL, the correspanding value not used.
 *****************************************************************************************/
double WLANNetworkClass::comp_mean_delay_a( WLANSectorClass* ap, double& ad_seg_type_r )
{
    double user_data_rate = 64;   // in Mbps
    double basic_rate     = 1 ;   // in kbps
    double phy_rate       = 11;   // in Mbps
    double hd_rate        = 2;    // in Mbps

    double mean_num_seg_per_session = 10;
    double mean_segment             = 20;
    double mean_read_time           = 20;
    double mean_session             = 200;
    double min_seg_size             = 5;
    double max_seg_size             = 5000;
    double down_up_ratio            = 9;
    double dur_compress             = 15;
    double mean_seg_read_total      = mean_session / mean_num_seg_per_session;

    double goodput      = 0.0  ;

    double sp           = 0.0;
    double tsum         = 0.0;
    double ac_tsum      = 0.0;
    double p_last_stage = 0.0;

    int    call_idx            = 0;
    int    traffic_type_idx    = 0;
    int    num_speci_traffic   = 0;
    int    num_traffic_ap_type = 0;                      // number of traffic type under specified AP.

    int num_user = ap->call_list->getSize();
    double ad_seg_type_f = 0.0;
    if (num_user == 0 ) {
        ad_seg_type_f = 0.0;
        ad_seg_type_r = 0.0;
        return ad_seg_type_f;
    }


    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;
    WLANSessionClass *call = (WLANSessionClass*) NULL;

    std::vector <WLANTrafficTypeClass*> traffic_ap_list; // a list of traffic type under specified AP.
    std::vector <int> num_traffic_type_list;             // a list of number of one traffic type call.

    bool found;
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;

        mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
        mean_session             = traffic_type->mean_session;
        mean_segment             = traffic_type->mean_segment;
        mean_read_time           = traffic_type->mean_read_time;
        min_seg_size             = traffic_type->min_seg_size;
        max_seg_size             = traffic_type->max_seg_size;
        down_up_ratio            = traffic_type->down_up_ratio;
        mean_seg_read_total      = mean_session / mean_num_seg_per_session;
        dur_compress             = traffic_type->dur_compress;

        found = false;
        num_speci_traffic = 0;
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            if ( call->traffic_type_idx == traffic_type_idx )
            {
                found = true;
                num_speci_traffic ++;
            }
        }
        if ( found ) {
            num_traffic_ap_type ++;
            traffic_ap_list.push_back( traffic_type );
            num_traffic_type_list.push_back(num_speci_traffic);
        }
    }

    //std::cout << "user_data_rate    " << user_data_rate   << std::endl;
    //std::cout << "dur_compress      " << dur_compress     << std::endl;
    //std::cout << "basic_rate        " << basic_rate       << std::endl;
    //std::cout << "phy_rate          " << phy_rate         << std::endl;
    //std::cout << "hd_rate           " << hd_rate          << std::endl;


    // NOTE: wt_ac_i is wait time for access control, which is a value correpanding to number of users under an AP,
    //       It means that we can devite the fix value from number of users.
    //       wt_i    is wait time for access delay and gitter statistics, which use the mechanism of random backoff.
    double wt_ac_i = 0.0;
    double wt_i    = 0.0;
    double pp      = 0.0;

    double mp_cond_coll = 0.0; // mean conditional collision probability of a user under the AP
    int mc_idx = 0;
    for ( call_idx=0; call_idx<num_user; call_idx++ )
    {
        call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
        mc_idx = call->master_idx; // get master call index

        mp_cond_coll += p_cond_coll_user_inter[mc_idx];
    }
    mp_cond_coll /= num_user;

    for ( int bkoff=0; bkoff<=max_backoff_stage; bkoff++ ) {
       wt_ac_i = (pow(2.0,bkoff)/2.0)*W*LS;                       // Fixed length of backoff windows
       wt_i    = (int) floor(rg->Random()*pow(2.0,bkoff)*W)*LS;   // Random choosen of backoff windows
       pp      = pow(mp_cond_coll,bkoff);            // probability of arriving the correspanding stage

       ac_tsum += pp*wt_ac_i;
       tsum    += pp*wt_i   ;
       sp      += pp        ;
    }

    // retry happend in last stage of backoff
    wt_ac_i = (pow(2.0,max_backoff_stage)/2)*W*LS;
    wt_i    = (int) floor(rg->Random()*pow(2.0,max_backoff_stage)*W)*LS;

    for ( int retry=1;retry<=limit_retry;retry++ ) {
        pp = pow(mp_cond_coll,retry)*pow(mp_cond_coll,limit_retry);
        p_last_stage += pp;
    }

    ac_tsum += p_last_stage*wt_ac_i;
    tsum    += p_last_stage*wt_i;
    sp      += p_last_stage;

    // delay in transmit a packet, it is a constant.
    double t_succ = 0.0;
    double t_coll = 0.0;
    goodput = (user_data_rate*1024*dur_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    if ( !use_rts_cts ) {
        // calculate time comsuption of success and collide in BASE mode
        //t_succ = (PAYLOAD+IPH)/phy_rate + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024)
        //    + SIFS + DIFS + 2*P_DELAY;
        t_succ = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024)
            + SIFS + DIFS + 2*P_DELAY;
        t_coll = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + DIFS + P_DELAY;
    }
    else
    {
        // calculate time comsuption of success and collide in RTS/CTS mode
        //t_succ = (PAYLOAD+IPH)/phy_rate + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024)
        //    + 3*SIFS + DIFS + 4*P_DELAY;
        t_succ = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024)
            + 3*SIFS + DIFS + 4*P_DELAY;
        t_coll = goodput + DIFS + P_DELAY;
    }

#if MC_DEBG
    ad_seg_type_f = (ac_tsum/sp) + t_coll + (num_user-1)*t_succ/2.0;
    ad_seg_type_r = (tsum/sp   ) + t_coll + (num_user-1)*t_succ/2.0;
#else
    ad_seg_type_f = (ac_tsum/sp) + (num_user-1)*t_succ/2.0;
    ad_seg_type_r = (tsum/sp   ) + (num_user-1)*t_succ/2.0;
#endif

    return ad_seg_type_f;
}


// another version of comp_mean_delay_a() for single type of users simulation
double WLANNetworkClass::comp_mean_delay_a( int num_user, double& ad_seg_type_r, double p_cond_coll )
{
    double user_data_rate = 64;   // in Mbps
    double basic_rate     = 1 ;   // in kbps
    double phy_rate       = 11;   // in Mbps
    double hd_rate        = 2;    // in Mbps

    double mean_num_seg_per_session = 10;
    double mean_segment             = 20;
    double mean_read_time           = 20;
    double mean_session             = 200;
    double min_seg_size             = 5;
    double max_seg_size             = 5000;
    double down_up_ratio            = 9;
    double dur_compress             = 15;
    double mean_seg_read_total      = mean_session / mean_num_seg_per_session;

    double goodput      = 0.0  ;

    double sp           = 0.0;
    double tsum         = 0.0;
    double ac_tsum      = 0.0;
    double p_last_stage = 0.0;

    double ad_seg_type_f = 0.0;
    if (num_user == 0 ) {
        ad_seg_type_f = 0.0;
        ad_seg_type_r = 0.0;
        return ad_seg_type_f;
    }

    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) traffic_type_list[0];
    user_data_rate = traffic_type->user_data_rate;
    basic_rate     = traffic_type->basic_rate;
    phy_rate       = traffic_type->phy_rate;
    hd_rate        = traffic_type->hd_rate;

    mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
    mean_session             = traffic_type->mean_session;
    mean_segment             = traffic_type->mean_segment;
    mean_read_time           = traffic_type->mean_read_time;
    min_seg_size             = traffic_type->min_seg_size;
    max_seg_size             = traffic_type->max_seg_size;
    down_up_ratio            = traffic_type->down_up_ratio;
    mean_seg_read_total      = mean_session / mean_num_seg_per_session;
    dur_compress             = traffic_type->dur_compress;

    // NOTE: wt_ac_i is wait time for access control, which is a value correpanding to number of users under an AP,
    //       It means that we can devite the fix value from number of users.
    //       wt_i    is wait time for access delay and gitter statistics, which use the mechanism of random backoff.
    double wt_ac_i = 0.0;
    double wt_i    = 0.0;
    double pp      = 0.0;

    //std::cout << "max_backoff_stage " << max_backoff_stage<< std::endl;
    //std::cout << "limit_retry       " << limit_retry      << std::endl;

    for ( int bkoff=0; bkoff<=max_backoff_stage; bkoff++ ) {
       wt_ac_i = (pow(2.0,bkoff)/2.0)*W*LS;                       // Fixed length of backoff windows
       wt_i    = (int) floor(rg->Random()*pow(2.0,bkoff)*W)*LS;   // Random choosen of backoff windows
       pp      = pow(p_cond_coll,bkoff);            // probability of arriving the correspanding stage

       ac_tsum += pp*wt_ac_i;
       tsum    += pp*wt_i   ;
       sp      += pp        ;
    }

    // retry happend in last stage of backoff
    wt_ac_i = (pow(2.0,max_backoff_stage)/2)*W*LS;
    wt_i    = (int) floor(rg->Random()*pow(2.0,max_backoff_stage)*W)*LS;

    for ( int retry=1;retry<=limit_retry;retry++ ) {
        pp = pow(p_cond_coll,retry)*pow(p_cond_coll,limit_retry);
        p_last_stage += pp;
    }

    ac_tsum += p_last_stage*wt_ac_i;
    tsum    += p_last_stage*wt_i;
    sp      += p_last_stage;

    // delay in transmit a packet, it is a constant.
    double t_succ = 0.0;
    double t_coll = 0.0;
    goodput = (user_data_rate*1024*dur_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    if ( !use_rts_cts ) {
        // calculate time comsuption of success and collide in BASE mode
        //t_succ = (PAYLOAD+IPH)/phy_rate + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024)
        //    + SIFS + DIFS + 2*P_DELAY;
        t_succ = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024)
            + SIFS + DIFS + 2*P_DELAY;
        t_coll = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + DIFS + P_DELAY;
    }
    else
    {
        // calculate time comsuption of success and collide in RTS/CTS mode
        //t_succ = (PAYLOAD+IPH)/phy_rate + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024)
        //    + 3*SIFS + DIFS + 4*P_DELAY;
        t_succ = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024)
            + 3*SIFS + DIFS + 4*P_DELAY;
        t_coll = goodput + DIFS + P_DELAY;
    }

#if MC_DEBG
    ad_seg_type_f = (ac_tsum/sp) + t_coll + (num_user-1)*t_succ/2.0;
    ad_seg_type_r = (tsum/sp   ) + t_coll + (num_user-1)*t_succ/2.0;
#else
    ad_seg_type_f = (ac_tsum/sp) + (num_user-1)*t_succ/2.0;
    ad_seg_type_r = (tsum/sp   ) + (num_user-1)*t_succ/2.0;
#endif

    return ad_seg_type_f;
}


/******************************************************************************************
 * DESCRIPTION: Compute the throughput for an AP.
 * INPUT:       ap
 *              normal_coll: 
 *                  save the value of normalized collide time in period of transmitting time.
 * RETURN:      throughput value of an AP ( throughput_ap )
 *****************************************************************************************/
double WLANNetworkClass::throughput_computation( WLANSectorClass* ap, double& normal_coll, double err )
{
    double user_data_rate = 64;   // in Mbps
    double basic_rate     = 1 ;   // in kbps
    double phy_rate       = 11;   // in Mbps
    double hd_rate        = 2;    // in Mbps

    double mean_num_seg_per_session = 10;
    double mean_segment             = 20;
    double mean_read_time           = 20;
    double mean_session             = 200;
    double min_seg_size             = 5;
    double max_seg_size             = 5000;
    double down_up_ratio            = 9;
    double mean_seg_read_total      = mean_session / mean_num_seg_per_session;
    double dur_compress             = 15;

    double  goodput   = 0.0;
    double  t_success = 0.0;
    double  t_collide = 0.0;

    double p_pro_tran_user     = 0.0;
    double p_pro_tran_channel  = 0.0;
    double p_pro_succ_channel  = 0.0;
    double p_pro_coll_channel  = 0.0;
    double p_pro_idle_channel  = 1.0;
    double p_sat_idle_channel  = 1.0;
    double p_sat_succ_channel  = 0.0;
    double p_sat_coll_channel  = 0.0;
    //double p_cond_coll         = 0.999;
    //double tau                 = 0.999;

    double throughput_channel  = 0.0;

    int    call_idx            = 0;
    int    traffic_type_idx    = 0;
    int    num_speci_traffic   = 0;
    int    num_traffic_ap_type = 0;   // number of traffic type under specified AP.
    int    num_user            = ap->call_list->getSize();

    if ( num_user == 0 ) {
        throughput_channel = 0.0;
        return throughput_channel;
    }

    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;
    WLANSessionClass *call = (WLANSessionClass*) NULL;

    std::vector <WLANTrafficTypeClass*> traffic_ap_list; // a list of traffic type under specified AP.
    std::vector <int> num_traffic_type_list;             // a list of number of one traffic type call.

    bool found;
    for ( traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;

        mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
        mean_session             = traffic_type->mean_session;
        mean_segment             = traffic_type->mean_segment;
        mean_read_time           = traffic_type->mean_read_time;
        min_seg_size             = traffic_type->min_seg_size;
        max_seg_size             = traffic_type->max_seg_size;
        down_up_ratio            = traffic_type->down_up_ratio;
        mean_seg_read_total      = mean_session / mean_num_seg_per_session;
        dur_compress             = traffic_type->dur_compress;

        found = false;
        num_speci_traffic = 0;
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
            call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
            if ( call->traffic_type_idx == traffic_type_idx )
            {
                found = true;
                num_speci_traffic ++;
            }
        }
        if ( found ) {
            num_traffic_ap_type ++;
            traffic_ap_list.push_back( traffic_type );
            num_traffic_type_list.push_back(num_speci_traffic);
        }
    }
    //std::cout << "user_data_rate    " << user_data_rate   << std::endl;
    //std::cout << "dur_compress      " << dur_compress     << std::endl;
    //std::cout << "basic_rate        " << basic_rate       << std::endl;
    //std::cout << "phy_rate          " << phy_rate         << std::endl;
    //std::cout << "hd_rate           " << hd_rate          << std::endl;

    goodput = (user_data_rate*1024*dur_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    //printf("goodput %f\n", goodput);

    if ( !use_rts_cts ) {
        // calculate time comsuption of success and collide in BASE mode
        t_success = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024)
            + SIFS + DIFS + 2*P_DELAY;
        t_collide = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + SIFS + P_DELAY;
    }
    else
    {
        // calculate time comsuption of success and collide in RTS/CTS mode
        t_success = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024)
            + 3*SIFS + DIFS + 4*P_DELAY;
        t_collide = goodput + SIFS + P_DELAY;
    }
    p_pro_tran_user = t_success/(dur_compress*1000);

    if (p_pro_tran_user<1)
        p_pro_idle_channel = pow((double)(1-p_pro_tran_user), num_user);
    else p_pro_idle_channel = 0.0;

    int mc_idx;

    //std::cout << "number of users:    " << num_user            << std::endl;
    for ( call_idx=0; call_idx<num_user; call_idx++ )
    {
        call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
        mc_idx = call->master_idx; // get master call index

        p_sat_idle_channel *= (1 - tau_user_inter[mc_idx]);

#if MC_DBG
        std::cout << "tau_user_inter[mc_idx]         " << tau_user_inter[mc_idx]         << std::endl;
        std::cout << "p_cond_coll_user_inter[mc_idx] " << p_cond_coll_user_inter[mc_idx] << std::endl;
#endif
    }
    p_pro_tran_channel = 1 - p_pro_idle_channel;
    for ( call_idx=0; call_idx<num_user; call_idx++ )
    {
        call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
        mc_idx = call->master_idx; // get master call index

        p_sat_succ_channel += tau_user_inter[mc_idx]*p_sat_idle_channel/(1-tau_user_inter[mc_idx]);
    }
    p_sat_coll_channel = 1-p_sat_idle_channel-p_sat_succ_channel;
    if ( p_sat_coll_channel < 0 ) p_sat_coll_channel = 0;

#if MC_DBG
    std::cout << "p_pro_idle_channel  " << p_pro_idle_channel  << std::endl;
    std::cout << "p_pro_tran_channel  " << p_pro_tran_channel  << std::endl;
    std::cout << "p_sat_idle_channel  " << p_sat_idle_channel  << std::endl;
    std::cout << "p_sat_succ_channel  " << p_sat_succ_channel  << std::endl;
    std::cout << "p_sat_coll_channel  " << p_sat_coll_channel  << std::endl;
#endif

#if 0
    // No interference algorithom
    if (p_pro_tran_user<1)
         p_pro_idle_channel = pow((double)(1-p_pro_tran_user), num_user);
    else p_pro_idle_channel = 0;

    p_pro_tran_channel = 1 - p_pro_idle_channel;

    tau_iteration_no_inter(num_user,p_cond_coll,tau,err );
    p_sat_idle_channel = pow((double)(1-tau,num_user));
    p_sat_succ_channel = num_user*tau*pow((double)(1-tau),num_user-1);
    p_sat_coll_channel = 1-p_sat_idle_channel-p_sat_succ_channel;
#endif

    p_pro_succ_channel = p_pro_tran_channel*p_sat_succ_channel*t_success
        /(p_sat_succ_channel*t_success+p_sat_coll_channel*t_collide);
    p_pro_coll_channel = p_pro_tran_channel*p_sat_coll_channel*t_collide
        /(p_sat_succ_channel*t_success+p_sat_coll_channel*t_collide);

    throughput_channel = 2*p_pro_succ_channel*goodput/t_success;

    //printf("t_succ %8.5f thrpt %8.5f\n", t_success,throughput_channel);

    normal_coll  = p_pro_coll_channel;

    /*
    printf("number_of_users %4d p %8.5f tau %8.5f p_pro_succ_channel %6.5f throughput_channel %6.4f  packet_loss_rate %6.4f\n",num_user, p_cond_coll, tau, p_pro_succ_channel, throughput_channel, normal_coll);
     */

    return throughput_channel;
}


/******************************************************************************************
 * DESCRIPTION: Solve probability of p_cond_coll & tau using secant method.
 * INPUT:       p_cond_coll: save the value of p_cond_coll result.
 *              tau:   save the value of tau result.
 *              err:   iteration error.
 * RETURN:      iteration error
 * NOTE:        p_cond_coll and tau save the results of iteration
 *              p_cond_coll: conditional collision probability, 
 *                  meaning that in a timeslot at least one of (n-1) remaining stations transmit.
 *              tau: a station transmits in a randomly chosen slot time.
 *****************************************************************************************/
double WLANNetworkClass::tau_iteration_no_inter( int num_user, double& p_cond_coll, double& tau, double err )
{
    double  p1 = 1E-10;
    double  p2 = 0.99999999;
    double  p3 = 0.99999999;
    double fp1 = 0.0;
    double fp2 = 0.0;
    double fp3 = 0.0;

    int iteration = 0;
    do {
        fp1 = tau_equations_no_inter(num_user, p1, tau);
        fp2 = tau_equations_no_inter(num_user, p2, tau);

        p3 = p2 - fp2*(p2-p1)/(fp2-fp1);
        fp3 = tau_equations_no_inter(num_user, p3, tau);
        if ( fp1*fp3 < 0 ) {
            p2 = p3;
        }
        else p1 = p3;
        //printf("\np1 %13.9f  p2 %13.9f  p3 %13.9f  \n", p1,p2,p3);
        //printf("\nptran %13.9f  \n", tau);

        iteration ++;
    } while ( fabs(fp3) > err && iteration < 100 );
    p_cond_coll = p3;

    //std::cout << "--------------------------" << std::endl;
    //std::cout << "ITERATION " << iteration    << std::endl;
    //std::cout << "--------------------------" << std::endl;

    return err;
}


/******************************************************************************************
 * DESCRIPTION: change equations of variable p_cond_coll & tau to fpcc expression.
 * INPUT:       p_cond_coll, save the value of p_cond_coll result.
 *              tau,   save the value of tau result.
 * RETURN:      the value of expression fpcc corresponding the input p_cond_coll.
 * NOTE:        p_cond_coll and tau save the results generated in process
 *****************************************************************************************/
double WLANNetworkClass::tau_equations_no_inter( int num_user, double& p_cond_coll, double& tau )
{
    // fpcc is an expression of input varable p_cond_coll
    double fpcc = 1;

    //The probability tau[j] of user j transmit in a random slot in APi
    tau = (1-pow(p_cond_coll, max_backoff_stage+limit_retry+1))*2*(1-2*p_cond_coll)
        /(W*(1-pow(2*p_cond_coll, max_backoff_stage+1))*(1-p_cond_coll)
          + (1-2*p_cond_coll)*(1-pow(p_cond_coll, max_backoff_stage+limit_retry+1))
          + W*pow(2.0,max_backoff_stage)*pow(p_cond_coll, max_backoff_stage+1)
            *(1-2*p_cond_coll)*(1-pow(p_cond_coll,limit_retry)));

    fpcc = p_cond_coll - (1-pow(1.0-tau, num_user-1));

    return fpcc;
}


/******************************************************************************************
 * DESCRIPTION: Solve probability tau (tau_user_inter) & p_cond_coll(p_cond_coll_user_inter)
 *              consider the affect of interference
 * INPUT:       err: iteration error.
 * RETURN:      iteration error
 *****************************************************************************************/
double WLANNetworkClass::tau_with_inter ( double err )
{
    int i,j;
    int times = 0;
    int num_call = master_call_list->getSize();
    double* tau_user_no_inter = (double*) malloc (num_call*sizeof(double));
    int** mx_a = (int**) malloc (num_call*sizeof(int*));
    for(i=0;i<num_call;i++) {
        mx_a[i] = (int*) malloc (num_call*sizeof(int));
        // initial:
        tau_user_no_inter[i] = 0.0;
    }
    tau_user_inter.clear();
    p_cond_coll_user_inter.clear();
    if ( num_call==0 )
    {
        free (tau_user_no_inter);
        for(i=0;i<num_call;i++) free (mx_a[i]);
        free (mx_a);

        return 0;
    }

    // get interference matrix & tau_user_no_inter
    get_inter_matrix( mx_a, tau_user_no_inter);

    double* tau_tmp = (double*) malloc (num_call*sizeof(double));
    for( i=0; i<num_call; i++ ) {
        tau_tmp[i] = tau_user_no_inter[i]; // initial values
        tau_user_inter.push_back(tau_user_no_inter[i]);
    }

#if MC_DBG
    // TEST inter matrix
    std::cout << "\n--- interference matrix ---\n";
    for(i=0;i<num_call;i++) {
        for(j=0;j<num_call;j++) {
            std::cout << mx_a[i][j] << " ";
        }
        std::cout << "\n";
    }
#endif

    // solve non-linear equations of tau
    double fi, fp;
    double sum_err, sum_tau;
    do {
        fp = 0.0;
        sum_err = 0.0;
        sum_tau   = 0.0;

        for ( i=0; i<num_call; i++ ) {
            fi = 1.0;
            for ( j=0; j<num_call; j++ ) {
                if (i == j)
                    continue;
                else
                    fi *= (1-mx_a[i][j]*tau_tmp[j]);
            }
            tau_user_inter[i] = tau_user_no_inter[i]*fi/(1.0+tau_user_no_inter[i]*fi*mx_a[i][i]);
        }

        for ( i=0; i<num_call; i++ ) {
            fp = tau_user_inter[i] - tau_tmp[i];

            // Iterate error
            sum_err += fp * fp;
            sum_tau += tau_user_inter[i] * tau_user_inter[i];

            tau_tmp[i] = tau_user_inter[i];
        }

        times ++;
    } while ( sum_err >= sum_tau * err && times < 150);

#if MC_DBG
    std::cout << "ITERATION  " << times << std::endl;
#endif

    // calculate probability of conditional collision with interference
    int cell_idx, ap_idx, call_idx;
    int mc_idx; // master call index
    WLANCellClass* cell    = NULL;
    WLANSectorClass* ap    = NULL;
    WLANSessionClass* call = NULL;
    for (cell_idx=0; cell_idx<num_cell; cell_idx++)
    {
        cell = (WLANCellClass*) cell_list[cell_idx];
        for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++)
        {
            ap = (WLANSectorClass*) cell->sector_list[ap_idx];
            int nu_ap = ap->call_list->getSize();
            double multi = 1.0;
            for ( call_idx=0; call_idx<nu_ap; call_idx++ )
            {
                call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                mc_idx = call->master_idx; // get master call index
                multi *= (1-tau_user_inter[mc_idx]);
            }

            for ( call_idx=0; call_idx<nu_ap; call_idx++ )
            {
                call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                mc_idx = call->master_idx; // get master call index
                double delta_tau;
                if ( tau_user_inter[mc_idx] < tau_user_no_inter[mc_idx] )
                    delta_tau = tau_user_no_inter[mc_idx] - tau_user_inter[mc_idx];
                else delta_tau = 0;
                multi /= (1.0-tau_user_inter[mc_idx]);
                if (multi > 1.0) multi = 1.0;
                p_cond_coll_user_inter.push_back(1.0 - multi + delta_tau);
            }
        }
    }
    
#if MC_DBG
    std::cout << "\n--- tau no inter ---\n";
    for(i=0;i<num_call;i++) {
        std::cout << tau_user_no_inter[i] << " ";
    }

    std::cout << "\n--- tau with inter---\n";
    std::cout << "num call " << num_call << std::endl; 
    for(i=0;i<num_call;i++) {
        std::cout << tau_user_inter[i] << " ";
    }
    std::cout << "\n\n\n";
#endif

    // free memory
    free (tau_user_no_inter);
    free (tau_tmp);
    for(i=0;i<num_call;i++) free (mx_a[i]);
    free (mx_a);

    return sum_err;
}



/******************************************************************************************
 * DESCRIPTION: get interence matrix
 * INPUT:       mx_a: is interference matrix, if user-i and user-j use same channel, 
 *                    and if user-i receive signal from user-j stronger than a given threshold,
 *                    then a(i,j)=1; else a(i,j)=0;
 *                    result get by this routine
 *              tau_user_no_inter: list of all users tau values in network
 *                                 length = num users in network (master_call_list->getSize())
 * RETURN:      iteration error
 *****************************************************************************************/
void WLANNetworkClass::get_inter_matrix( int** mx_a, double* tau_user_no_inter )
{
    int mc_i       = 0;  // call loop index, master call i
    int mc_j       = 0;  // call loop index, master call j
    int cell_idx_i = 0;
    int cell_idx_j = 0;
    int ap_idx     = 0;
    int nu_ap      = 0;  // number of users under AP
    int chan_idx_i = 1;
    int chan_idx_j = 1;

    WLANSessionClass* call_i = NULL;
    WLANSessionClass* call_j = NULL;
    WLANCellClass*    cell   = NULL;
    WLANSectorClass*  ap     = NULL;

    double cca = WLANSessionClass::get_cca();

    int num_call = master_call_list->getSize();
    if ( num_call == 0 )
    {
        tau_user_no_inter = NULL;
        mx_a = NULL;
    }

    bool flg = false;
    for ( mc_i=0; mc_i<num_call; mc_i++ ) {
        // get list of tau_user_no_inter
        call_i = (WLANSessionClass *) (*master_call_list)[mc_i];
        cell_idx_i = call_i->cell_idx;
        ap_idx = call_i->sector_idx;
        cell = (WLANCellClass*) cell_list[cell_idx_i];
        ap = (WLANSectorClass*) cell->sector_list[ap_idx];
        chan_idx_i = ap->chan_idx;
        nu_ap = ap->call_list->getSize();
        tau_user_no_inter[mc_i] = tau_nu_no_inter[nu_ap];

        // get interference matrix
        for ( mc_j=mc_i; mc_j<num_call; mc_j++ ) {
            double rx_pwr_db = 0.0;
            call_j = (WLANSessionClass *) (*master_call_list)[mc_j];
            cell_idx_j = call_j->cell_idx;
            ap_idx = call_j->sector_idx;
            cell = (WLANCellClass*) cell_list[cell_idx_j];
            ap = (WLANSectorClass*) cell->sector_list[ap_idx];
            chan_idx_j = ap->chan_idx;
            if ( mc_i != mc_j && cell_idx_i != cell_idx_j )
                rx_pwr_db = call_j->receive_pwr(this, call_i->posn_x, call_i->posn_y);

            if ( (chan_idx_i != chan_idx_j) || (cell_idx_i == cell_idx_j) || (mc_i == mc_j) || (rx_pwr_db < cca) )
            {
                mx_a[mc_i][mc_j] = 0;
                mx_a[mc_j][mc_i] = 0;
#if MC_DBG
                if ( mc_i != mc_j && cell_idx_i != cell_idx_j && rx_pwr_db <= cca ) {
                    std::cout << "Channel clear!\n";
                }
#endif

            }
            else
            {
                mx_a[mc_i][mc_j] = 1;
                mx_a[mc_j][mc_i] = 1;
#if MC_DBG
                if ( mc_i != mc_j && cell_idx_i != cell_idx_j ) {
                    std::cout << "Channel not clear!\n";
                }
#endif
            }
        }

    }
}


/******************************************************************************************
 * DESCRIPTION: Solve sigma PDF of bernoulli distribution.
 * INPUT:       m, n, p
 * RETURN:      pdf value
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::bino_p( int m, int n, double p )
{
    int    i;

    // Defined as double type to avoid overflow in solve n!.
    //double fnj = 1.; // factorial (n-j) -->  (n-j)!
    //double fn  = 1.; // factorial n     -->  n!
    double fj  = 1.;   // factorial j     -->  j!
    double pnj = 1.;   // pnj = n*(n-1)(n-2)....(n-j+1)
    double cnj = 1.0;  // cnj = fn / (fj*fnj) = pnj/fj
    double binop = 0.0;

    //  If m>n/2, use 'C(n,m) = C(n, n-m)' to reduce the calculation amount.
    if ( m > n / 2 ) {
        for ( i=m+1; i<=n; i++ )
            pnj *= i;
        for ( i=2; i<=n-m; i++ )
            fj *= i;
    } else {
        for ( i=(n-m+1); i<=n; i++ )
            pnj *= i;
        for ( i=2; i<=m; i++ )
            fj *= i;
    }

    cnj = pnj / fj;

    binop = cnj*pow(p,m)*pow(1-p,n-m);
    // std::cout << "binop " << binop << std::endl;

    return binop;
}



/******************************************************************************************
 * DESCRIPTION: Finding the index of segment of k to computation prop_tran_ahead(k)
 * INPUT:       k: value of k, which is vary from 1 to W*(2**m)
 * RETURN:      krange: segment index of k, if can not find the krange return -1
 * NOTE:
 *****************************************************************************************/
int WLANNetworkClass::find_krange( int k )
{
    int i;
    int    krange  = 0;
    double k_start = 0;
    for (i=0;i<=max_backoff_stage;i++){
        k_start = W*pow(2.0,i);
        if (k<=k_start)
        {
            krange = i;
            return krange;
        }
    }

    return -1;
}



/******************************************************************************************
 * DESCRIPTION: Finding the probability of user that transmitted ahead the user of position
 *              (b,k) (here, b is b-th stage, k is k-th backoff length in b-th stage)
 * INPUT:       k: value of k, which varies from 1 to W*(2^m)
 *              p_cond_coll: pass the value of p_cond_coll
 * RETURN:      probability of user that transmitted ahead the user of position (b,k)
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::prop_tran_ahead( double p_cond_coll, int k )
{
    int i;
    double pro_stage    = 0.0;
    double p_tran_ahead = -1  ;
    double sum          =  0.0;

    pro_stage = (1-pow(p_cond_coll, max_backoff_stage+limit_retry+1))/(1-p_cond_coll);

    int krange = find_krange(k);

    if ( krange == 0 )
    {
        for(i=0;i<=max_backoff_stage;i++) {
            sum += pow(p_cond_coll,i)*(k-1)/(pro_stage*(W*pow(2.0,i)));
        }
        for(i=1;i<=limit_retry;i++) {
            sum += pow(p_cond_coll,max_backoff_stage+i)*(k-1)/(pro_stage*W*pow(2.0,max_backoff_stage));
        }
    }
    else if ( krange == max_backoff_stage )
    {
        for(i=0;i<=max_backoff_stage-1;i++) {
            sum += pow(p_cond_coll,i)/pro_stage;
        }
        for(i=0;i<=limit_retry;i++) {
            sum += pow(p_cond_coll,max_backoff_stage+i)*(k-1)/(pro_stage*W*pow(2.0,max_backoff_stage));
        }
    }
    else if ( krange > 0 && krange < max_backoff_stage )
    {
        for(i=0;i<krange;i++) {
            sum += pow(p_cond_coll,i)/pro_stage;
        }
        for(i=krange;i<max_backoff_stage;i++) {
            sum += pow(p_cond_coll,i)*(k-1)/(pro_stage*W*pow(2.0,i));
        }
        for(i=1;i<=limit_retry;i++) {
            sum += pow(p_cond_coll,max_backoff_stage+i)*(k-1)/(pro_stage*W*pow(2.0,max_backoff_stage));
        }
    }
    else
    {
        return p_tran_ahead;
    }
    p_tran_ahead = sum;

    return p_tran_ahead;
}


/******************************************************************************************
 * DESCRIPTION: Time of frozen in stage b
 * INPUT:       num_user: number of users under an AP
 *              p_cond_coll: pass the value of p_cond_coll
 *              b: index of stage
 * RETURN:      Frozen time
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::frozen_stage( int num_user, double p_cond_coll, int b )
{
    int i;
    int stage = b;
    double frozen_stage_i = 0.0;

    if ( stage == 0 )
    {
        double pta1 = prop_tran_ahead(p_cond_coll,1);
        double pta2 = prop_tran_ahead(p_cond_coll,W);
        frozen_stage_i = (num_user-1)*(pta1+pta2);
        frozen_stage_i /= 4.;
    }
    else if ( stage == 1 )
    {
        double pta1 = prop_tran_ahead(p_cond_coll,1)/2.;
        double pta2 = prop_tran_ahead(p_cond_coll,W);
        double pta3 = prop_tran_ahead(p_cond_coll,2*W)/2.;
        frozen_stage_i = (num_user-1)*(pta1+pta3+pta2)/2.;
    }
    else
    {
        double pta1 = prop_tran_ahead(p_cond_coll,1);
        double pta3 = prop_tran_ahead(p_cond_coll,pow(2.0,stage)*W);
        frozen_stage_i = (num_user-1)*(pta1+pta3)/4.;

        for(i=0;i<stage;i++ ) {
            double pta2 = prop_tran_ahead(p_cond_coll,(int)pow(2.0,i)*W);
            frozen_stage_i += (num_user-1)*pta2/2.;
        }
    }

    return frozen_stage_i;
}



/******************************************************************************************
 * DESCRIPTION: Total time of backoff before stage b
 * INPUT        b: index of stage
 * RETURN:      Total backoff time
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::l_backoff( int b )
{
    int i;
    double l_bkoff = 0.0;

    for(i=0;i<b;i++)
    {
        l_bkoff += W*pow(2.0,i)/2.;
    }

    return l_bkoff;
}



/******************************************************************************************
 * DESCRIPTION: Total time of frozen in stage b
 * INPUT:       num_user: number of users under an AP
 *              p_cond_coll: pass the value of p_cond_coll
 *              b: index of stage
 * RETURN:      Total frozen time
 * NOTE:
 *****************************************************************************************/
double WLANNetworkClass::l_frozen( int num_user,double p_cond_coll,int b )
{
    int i;
    double l_frz = 0.0;
    int    stage = b;
    for(i=0;i<stage;i++)
    {
        l_frz += frozen_stage(num_user,p_cond_coll,i);
    }

    return l_frz;
}



/******************************************************************************************
 * DESCRIPTION: Second algorithm of mean delay computation
 * INPUT:       p_cond_coll: pass the value of p_cond_coll
 *              tau        : pass the value of tau
 * RETURN:      Mean delay
 * NOTE:        Now, the algorighm is too slowly for event based simulation
 *****************************************************************************************/
double WLANNetworkClass::comp_mean_delay_b( WLANSectorClass* ap )
{
    int i;
    double user_data_rate = 64;   // in Mbps
    double basic_rate     = 1 ;   // in kbps
    double phy_rate       = 11;   // in Mbps
    double hd_rate        = 2;    // in Mbps
    double duration_compress = 15;

    double mean_delay  = 0.0;
    double pro_stage   = 0.0;

    double goodput     = 0.0;
    double t_success   = 0.0;
    double t_collide   = 0.0;

    double p_sat_idle_channel = 1.0;
    double p_sat_succ_channel = 0.0;
    double p_sat_coll_channel = 0.0;

    int  num_user = ap->call_list->getSize();

    double mp_cond_coll = 0.0; // mean conditional collision probability of a user under the AP
    double mtau              = 0.0; // mean tau of a user under the AP
    int call_idx,mc_idx;
    WLANSessionClass * call = NULL;
    for ( call_idx=0; call_idx<num_user; call_idx++ )
    {
        WLANSessionClass * call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
        mc_idx = call->master_idx;  // get master call index

        mp_cond_coll += p_cond_coll_user_inter[mc_idx];
        mtau += tau_user_inter[mc_idx];
    }
    mp_cond_coll /= num_user;
    mtau /= num_user;
 
    pro_stage = (1-pow(mp_cond_coll, max_backoff_stage+limit_retry+1))/(1-mp_cond_coll);

    p_sat_idle_channel = pow(1-mtau,num_user);
    p_sat_succ_channel = num_user*mtau*pow(1-mtau,num_user-1);
    p_sat_coll_channel = 1-p_sat_idle_channel-p_sat_succ_channel;

    for ( int traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {

        WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;
        duration_compress = traffic_type->dur_compress;
    }

    goodput = (user_data_rate*1024*duration_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    if ( !use_rts_cts ) {
        t_success = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024) + SIFS + DIFS + 2*P_DELAY;
        t_collide = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + SIFS + P_DELAY;
    }
    else
    {
        t_success = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024) + 3*SIFS + DIFS + 4*P_DELAY;
        t_collide = goodput + SIFS + P_DELAY;
    }


    /*
      std::cout << "t_success          " << t_success          << std::endl;
      std::cout << "t_collide          " << t_collide          << std::endl;
      std::cout << "p_sat_succ_channel " << p_sat_succ_channel << std::endl;
      std::cout << "p_sat_coll_channel " << p_sat_coll_channel << std::endl;
    */

    double frozen_stage_m = frozen_stage(num_user,mp_cond_coll,max_backoff_stage);
    for( i=0;i<=max_backoff_stage;i++ )
    {
        double delay_j = 0.0;
        double pbk     = 0.0;
        double l_frz   = 0.0;
        double l_bk    = 0.0;

        pbk   = pow(mp_cond_coll,i)/(pow(2.0,i)*W*pro_stage); // probability get to stage i
        l_frz = l_frozen(num_user,mp_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,i)*W;j++ )
        {
            double delay_k  = 0.0;
            double pta      = 0.0;

            pta = prop_tran_ahead(mp_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_delay_k = 0.0;
                t_delay_k  = (l_bk+j)*LS;
                t_delay_k += (k+l_frz)*(1-2.*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_delay_k += (k+l_frz)*p_sat_coll_channel*t_collide/(p_sat_succ_channel+p_sat_coll_channel);

                t_delay_k *= bino_p(k, num_user-1, pta);
                delay_k += t_delay_k;
            }

            delay_j += delay_k;
        }
        mean_delay += pbk * delay_j;
    }


    for( i=1;i<=limit_retry;i++ )
    {
        double delay_j = 0.0;

        double pbk    = 0.0;
        double l_frz  = 0.0;
        double l_bk   = 0.0;

        pbk   = pow(mp_cond_coll,max_backoff_stage+i)/(pow(2.0,max_backoff_stage)*W);
        l_frz = l_frozen(num_user,mp_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,max_backoff_stage)*W;j++ )
        {
            double delay_k = 0.0;
            double pta     = 0.0;

            pta = prop_tran_ahead(mp_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_delay_k = 0.0;
                t_delay_k = (l_bk+j+(i-1)*pow(2.0,max_backoff_stage-1)*W)*LS;
                t_delay_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *(1-2*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_delay_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *p_sat_coll_channel*t_collide/(p_sat_succ_channel+p_sat_coll_channel);

                t_delay_k *= bino_p(k, num_user-1, pta);
                delay_k += t_delay_k;
            }

            delay_j += delay_k;
        }
        mean_delay += pbk*delay_j;
    }

    return mean_delay;
}

/******************************************************************************************
 * DESCRIPTION: algorithm of jitter computation, which is just like the algorithm of second
 *              mean delay algorithm
 * INPUT:       p_cond_coll: pass the value of p_cond_coll
 *              tau        : pass the value of tau
 * RETURN:      Jitter
 * NOTE:        Now, the algorighm is too slowly for event based simulation
 *              input 'WLANSectorClass' for future mix-mode  simulation
 *****************************************************************************************/
double WLANNetworkClass::comp_jitter( WLANSectorClass* ap )
{
    int i;
    double user_data_rate = 64;    // in Mbps
    double basic_rate     = 1 ;    // in kbps
    double phy_rate       = 11;    // in Mbps
    double hd_rate        = 2;     // in Mbps
    double duration_compress = 15;

    double jitter      = 0.0;
    double pro_stage   = 0.0;

    double goodput     = 0.0;
    double t_success   = 0.0;
    double t_collide   = 0.0;

    double p_sat_idle_channel = 1.0;
    double p_sat_succ_channel = 0.0;
    double p_sat_coll_channel = 0.0;

    int  num_user = ap->call_list->getSize();

    double mp_cond_coll = 0.0; // mean conditional collision probability of a user under the AP
    double mtau              = 0.0; // mean tau of a user under the AP
    int call_idx, mc_idx;
    WLANSessionClass* call = NULL;
    for ( call_idx=0; call_idx<num_user; call_idx++ )
    {
        call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
        mc_idx = call->master_idx;  // get master call index

        mp_cond_coll += p_cond_coll_user_inter[mc_idx];
        mtau += tau_user_inter[mc_idx];
    }
    mp_cond_coll /= num_user;
    mtau /= num_user;

    pro_stage = (1-pow(mp_cond_coll, max_backoff_stage+limit_retry+1))/(1-mp_cond_coll);

    for ( int traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;
        duration_compress = traffic_type->dur_compress;
    }

    p_sat_idle_channel = pow(1-mtau,num_user);
    p_sat_succ_channel = num_user*mtau*pow(1-mtau,num_user-1);
    p_sat_coll_channel = 1-p_sat_idle_channel-p_sat_succ_channel;

    goodput = (user_data_rate*1024*duration_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    if ( !use_rts_cts ) {
        t_success = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024) + SIFS + DIFS + 2*P_DELAY;
        t_collide = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + SIFS + P_DELAY;
    }
    else
    {
        t_success = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024) + 3*SIFS + DIFS + 4*P_DELAY;
        t_collide = goodput + SIFS + P_DELAY;
    }

    double ad_seg_type_r = 0.0;
    double frozen_stage_m = frozen_stage(num_user,mp_cond_coll,max_backoff_stage);
    double mean_delay = comp_mean_delay_a(ap, ad_seg_type_r);

    for( i=0;i<=max_backoff_stage;i++ )
    {
        double jitter_j = 0.0;
        double pbk      = 0.0;
        double l_frz    = 0.0;
        double l_bk     = 0.0;

        pbk   = pow(mp_cond_coll,i)/(pow(2.0,i)*W*pro_stage); // probability get to stage i
        l_frz = l_frozen(num_user,mp_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,i)*W;j++ )
        {
            double jitter_k = 0.0;
            double pta      = 0.0;

            pta = prop_tran_ahead(mp_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_jitter_k = 0.0;
                t_jitter_k = (l_bk+j)*LS;
                t_jitter_k += (k+l_frz)*(1-2*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_jitter_k += (k+l_frz)*p_sat_coll_channel*t_collide
                        /(p_sat_succ_channel+p_sat_coll_channel);

                t_jitter_k -= mean_delay;
                t_jitter_k = t_jitter_k*t_jitter_k;

                t_jitter_k *= bino_p(k, num_user-1, pta);
                jitter_k += t_jitter_k;
            }
            jitter_j += jitter_k;
        }
        jitter += pbk * jitter_j;
    }

    for( i=1;i<=limit_retry;i++ )
    {
        double jitter_j = 0.0;

        double pbk    = 0.0;
        double l_frz  = 0.0;
        double l_bk   = 0.0;

        pbk   = pow(mp_cond_coll,max_backoff_stage+i)/(pow(2.0,max_backoff_stage)*W);
        l_frz = l_frozen(num_user,mp_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,max_backoff_stage)*W;j++ )
        {
            double jitter_k = 0.0;
            double pta      = 0.0;

            pta = prop_tran_ahead(mp_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_jitter_k = 0.0;
                t_jitter_k = (l_bk+j+(i-1)*pow(2.0,max_backoff_stage-1)*W)*LS;
                t_jitter_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *(1-2*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_jitter_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *p_sat_coll_channel*t_collide/(p_sat_succ_channel+p_sat_coll_channel);

                t_jitter_k -= mean_delay;
                t_jitter_k = t_jitter_k*t_jitter_k;

                t_jitter_k *= bino_p(k, num_user-1, pta);
                jitter_k += t_jitter_k;
            }

            jitter_j += jitter_k;
        }
        jitter += pbk * jitter_j;
    }
    jitter = sqrt(jitter);

    return jitter;
}


// another version of comp_jitter() for single type of users simulation
double WLANNetworkClass::comp_jitter( int num_user, double  p_cond_coll, double tau )
{
    int i;
    double user_data_rate = 64;    // in Mbps
    double basic_rate     = 1 ;    // in kbps
    double phy_rate       = 11;    // in Mbps
    double hd_rate        = 2;     // in Mbps
    double duration_compress = 15;

    double jitter      = 0.0;
    double pro_stage   = 0.0;

    double goodput     = 0.0;
    double t_success   = 0.0;
    double t_collide   = 0.0;

    double p_sat_idle_channel = 1.0;
    double p_sat_succ_channel = 0.0;
    double p_sat_coll_channel = 0.0;

    pro_stage = (1-pow(p_cond_coll, max_backoff_stage+limit_retry+1))/(1-p_cond_coll);

    for ( int traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++ ) {
        WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];
        user_data_rate = traffic_type->user_data_rate;
        basic_rate     = traffic_type->basic_rate;
        phy_rate       = traffic_type->phy_rate;
        hd_rate        = traffic_type->hd_rate;
        duration_compress = traffic_type->dur_compress;
    }

    p_sat_idle_channel = pow(1-tau,num_user);
    p_sat_succ_channel = num_user*tau*pow(1-tau,num_user-1);
    p_sat_coll_channel = 1-p_sat_idle_channel-p_sat_succ_channel;

    goodput = (user_data_rate*1024*duration_compress/1000+IPH)*1E+6/(phy_rate*1024*1024);
    if ( !use_rts_cts ) {
        t_success = goodput + ((MH+PH)/hd_rate + L_ACK/basic_rate)*1E+6/(1024*1024) + SIFS + DIFS + 2*P_DELAY;
        t_collide = goodput + ((MH+PH)/hd_rate)*1E+6/(1024*1024) + SIFS + P_DELAY;
    }
    else
    {
        t_success = goodput + ((L_ACK+L_RTS+L_CTS)/basic_rate)*1E+6/(1024*1024) + 3*SIFS + DIFS + 4*P_DELAY;
        t_collide = goodput + SIFS + P_DELAY;
    }

    double ad_seg_type_r = 0.0;
    double frozen_stage_m = frozen_stage(num_user,p_cond_coll,max_backoff_stage);
    double mean_delay = comp_mean_delay_a(num_user,ad_seg_type_r,p_cond_coll);

    for( i=0;i<=max_backoff_stage;i++ )
    {
        double jitter_j = 0.0;
        double pbk      = 0.0;
        double l_frz    = 0.0;
        double l_bk     = 0.0;

        pbk   = pow(p_cond_coll,i)/(pow(2.0,i)*W*pro_stage); // probability get to stage i
        l_frz = l_frozen(num_user,p_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,i)*W;j++ )
        {
            double jitter_k = 0.0;
            double pta      = 0.0;

            pta = prop_tran_ahead(p_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_jitter_k = 0.0;
                t_jitter_k = (l_bk+j)*LS;
                t_jitter_k += (k+l_frz)*(1-2*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_jitter_k += (k+l_frz)*p_sat_coll_channel*t_collide
                        /(p_sat_succ_channel+p_sat_coll_channel);

                t_jitter_k -= mean_delay;
                t_jitter_k = t_jitter_k*t_jitter_k;

                t_jitter_k *= bino_p(k, num_user-1, pta);
                jitter_k += t_jitter_k;
            }
            jitter_j += jitter_k;
        }
        jitter += pbk * jitter_j;
    }

    for( i=1;i<=limit_retry;i++ )
    {
        double jitter_j = 0.0;

        double pbk    = 0.0;
        double l_frz  = 0.0;
        double l_bk   = 0.0;

        pbk   = pow(p_cond_coll,max_backoff_stage+i)/(pow(2.0,max_backoff_stage)*W);
        l_frz = l_frozen(num_user,p_cond_coll,i);
        l_bk  = l_backoff(i);
        for( int j=1;j<=pow(2.0,max_backoff_stage)*W;j++ )
        {
            double jitter_k = 0.0;
            double pta      = 0.0;

            pta = prop_tran_ahead(p_cond_coll,j);
            for( int k=0;k<num_user;k++ )
            {
                double t_jitter_k = 0.0;
                t_jitter_k = (l_bk+j+(i-1)*pow(2.0,max_backoff_stage-1)*W)*LS;
                t_jitter_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *(1-2*p_sat_coll_channel/(p_sat_succ_channel+p_sat_coll_channel))*t_success;
                t_jitter_k += (k+l_frz+(i-1)*frozen_stage_m)
                    *p_sat_coll_channel*t_collide/(p_sat_succ_channel+p_sat_coll_channel);

                t_jitter_k -= mean_delay;
                t_jitter_k = t_jitter_k*t_jitter_k;

                t_jitter_k *= bino_p(k, num_user-1, pta);
                jitter_k += t_jitter_k;
            }

            jitter_j += jitter_k;
        }
        jitter += pbk * jitter_j;
    }
    jitter = sqrt(jitter);

    return jitter;
}


/******************************************************************************************/
/**** FUNCTION: update_network                                                         ****/
/**** Update network by processing the current event and all events triggered by the   ****/
/**** current event.                                                                   ****/
/******************************************************************************************/
//static bool thr_opn_fg = true;
//static bool thr_cls_fg = true;
static bool sir_fg     = true;
static bool jitter_fg  = true;
std::vector<double> jitter_user;

void WLANNetworkClass::update_network(EventClass *event)
{
    int i;
    int tt_idx           =  0;
    int cell_idx         = -1;
    int call_idx         =  0;
    int ap_idx           = -1;
    int master_idx       = -1;
    int traffic_type_idx =  0;

    double error = 1E-10;

    WLANCellClass *cell    = NULL;
    WLANSessionClass *call = NULL;
    WLANSectorClass *ap    = NULL;

    double rssi_call = -113.;
    double sir_call  = -0.0;
    double int_call  =  113.;

    bool blocked_rssi = false;
    bool blocked_sir  = false;
    bool blocked_int  = false;
    bool delete_call  = false;

    stat->duration += event->time;
    abs_time       += event->time;
    traffic_type_idx = event->traffic_type_idx;
    if (traffic_type_idx<0 || traffic_type_idx>=num_traffic_type) traffic_type_idx = 0;


    WLANTrafficTypeClass *traffic_type = (WLANTrafficTypeClass*) traffic_type_list[traffic_type_idx];

    double ad_seg_type_f = 0.0;
    double ad_seg_type_r = 0.0;

    // Note : RequestEvent is Session Request to Data Service.
    if ( event->event_type == CConst::RequestEvent )
    {
        assign_sector(&cell_idx, &ap_idx, event, 1);

        cell = (WLANCellClass* ) cell_list[cell_idx];
        ap   = (WLANSectorClass *) cell->sector_list[ap_idx];

        if ( WLANNetworkClass::service_type == VoiceService ) {
            ((WLANStatCountClass *) stat_count)->num_request[traffic_type_idx] ++;
            ((WLANStatCountClass *) ap->stat_count)->num_request[traffic_type_idx] ++;
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
            ((WLANStatCountClass *) stat_count)->num_session_request[traffic_type_idx] ++;
            ((WLANStatCountClass *) ap->stat_count)->num_session_request[traffic_type_idx] ++;
        }

        int num_call = ap->call_list->getSize();
        double p_cond_coll = 0.0;
        double tau         = 0.0;
        tau_iteration_no_inter(num_call, p_cond_coll,tau,1E-10);

        // Update number of call request in network.
        if ( stat->plot_event ) {
            if ( WLANNetworkClass::service_type == VoiceService ) {
                fprintf(stat->fp_event, "CALL_REQUEST TIME = %10.15f TRAFFIC_TYPE = %s\n",
                        abs_time, traffic_type->name());
            }
            else if ( WLANNetworkClass::service_type == DataService ) {
                fprintf(stat->fp_event, "SESSION_REQUEST TIME = %10.15f TRAFFIC_TYPE = %s\n",
                        abs_time, traffic_type->name());
            }
            fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);
            fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                    idx_to_x(event->posn_x), idx_to_y(event->posn_y));

            fflush(stat->fp_event);
        }

        if ( mechanism_block == CConst::RSSIMec )
        {
            double rssi_pwr = ap->tx_pwr*ap->comp_prop(this, event->posn_x, event->posn_y);
            rssi_call = 10.0*log(rssi_pwr)/log(10.0);

#if MC_DBG
            std::cout << "RSSI CALL " << rssi_call << std::endl;
#endif
            if ( rssi_call < rssi_threshold_call_block_db )
                blocked_rssi = true;
        }
        else if ( mechanism_block == CConst::SIRMec )
        {
            sir_call = comp_sir_call( cell_idx, event->posn_x, event->posn_y );

#if MC_DBG
            if ( sir_call < sir_threshold_call_block_db )
                blocked_sir = true;
#endif
            blocked_rssi = false;
        }
        else if ( mechanism_block == CConst::INTMec )
        {
            int_call = comp_int_call( cell_idx, event->posn_x, event->posn_y );

#if MC_DBG
            if ( int_call > int_threshold_call_block_db )
                blocked_int = true;
#endif
            blocked_rssi = false;
        }

        // Request Timeout of corresponding traffic type of the AP.
        // When the delay ad_seg_type_f[i](fix windows) larger ac_activate, time out happenen.
        double actm_out = 0.0;
        std::vector <int> traffic_idx_ap_list;
        std::vector <int> num_traffic_type_list;
        int num_traffic_ap_type = 1;
        int num_speci_traffic   = 0;

        bool found;
        // Find number and type of traffic in an AP.
        for ( tt_idx=0; tt_idx<num_traffic_type; tt_idx++ ) {
            found = false;
            num_speci_traffic = 0;
            for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
                WLANSessionClass* call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                if ( call->traffic_type_idx == tt_idx ) {
                    found = true;
                    num_speci_traffic ++;
                }
            }
            if ( found ) {
                num_traffic_ap_type ++;
                traffic_idx_ap_list.push_back( tt_idx );
                num_traffic_type_list.push_back( num_speci_traffic );
            }
        }

        ad_seg_type_f = comp_mean_delay_a( ap, ad_seg_type_r );
        actm_out = ad_seg_type_f;

        // after printing this [ num_users <==> access control delay ]
        // we can control users to special numbers convinient
        if (  stat->plot_delay && cell_idx == 0 && ap_idx == 0 ) {
            fprintf(stat->fp_delay, "---- AP %3d NUM_USERS %3d TRAFFIC TYPE %3d  AC_DELAY = %10.15f ---- \n",
                0, num_call, 0, ad_seg_type_f );
        }

        if ( blocked_rssi || blocked_sir || blocked_int )
        {
            // Update number of blocked call request in network.
            // Need to be staticed individually ?
            if (stat->plot_event) {
                if ( WLANNetworkClass::service_type == VoiceService ) {
                    fprintf(stat->fp_event, "CALL_BLOCKED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                            abs_time, traffic_type->name());
                }
                else if ( WLANNetworkClass::service_type == DataService ) {
                    fprintf(stat->fp_event, "SESSION_BLOCKED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                            abs_time, traffic_type->name());
                }
                fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);
                fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                        idx_to_x(event->posn_x), idx_to_y(event->posn_y));

                fflush(stat->fp_event);
            }

            // Statistics
            if ( WLANNetworkClass::service_type == VoiceService ) {
                ((WLANStatCountClass *) stat_count)->num_request_block[traffic_type_idx] ++;
                ((WLANStatCountClass *) ap->stat_count)->num_request_block[traffic_type_idx] ++;
            }
            else if ( WLANNetworkClass::service_type == DataService ) {
                ((WLANStatCountClass *) stat_count)->num_session_request_block[traffic_type_idx] ++;
                ((WLANStatCountClass *) ap->stat_count)->num_session_request_block[traffic_type_idx] ++;
            }

            blocked_rssi = false;
            blocked_sir  = false;
            blocked_int  = false;

#if MC_DBG
            std::cout << "CALL/SESSION REQUEST BLOCK,  RSSI = " << rssi_call << std::endl;
#endif
        }
        else
        {
            if ( actm_out >= ac_activate || num_call >= ac_num_user )
            {
                /*  Request Timeout status. It will happenen when an AP is fairly busy.
                 *  Access contral mechanism for Data Service.
                 ********************************************************************************/
                if (stat->plot_event) {
                    if ( WLANNetworkClass::service_type == VoiceService ) {
                        fprintf(stat->fp_event, "SESSION_CONNECTED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type->name());
                    }
                    else if ( WLANNetworkClass::service_type == DataService ) {
                        fprintf(stat->fp_event, "CALL_CONNECTED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type->name());
                    }
                    fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);
                    fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                            idx_to_x(event->posn_x), idx_to_y(event->posn_y));

                    fflush(stat->fp_event);
                }

                ((WLANStatCountClass *) stat_count)->num_session_request_timeout[traffic_type_idx] ++;
                ((WLANStatCountClass *) ap->stat_count)->num_session_request_timeout[traffic_type_idx] ++;


#if MC_DBG
                std ::cout << " SESSION REQUEST TIMEOUT" << std::endl;
#endif
            }
            else
            {
                /*  Update number of successful call request in network.
                 *  Create a new Call, and append it to master_call_list.
                 *  Create a new Session for Data Service, which has random number of segments in.
                 ********************************************************************************/
                call = (WLANSessionClass *) malloc ( sizeof(WLANSessionClass) );
                call->posn_x           = event->posn_x;
                call->posn_y           = event->posn_y;
                call->cell_idx         = cell_idx;
                call->call_idx         = ap->call_list->getSize();
                call->sector_idx       = ap_idx;
                call->master_idx       = master_call_list->getSize();
                call->traffic_type_idx = traffic_type_idx;

                double mean_num_seg_per_session = traffic_type->mean_num_seg_per_session;
                double mean_session             = traffic_type->mean_session;
                double mean_segment             = traffic_type->mean_segment;
                double mean_read_time           = traffic_type->mean_read_time;
                double min_seg_size             = traffic_type->min_seg_size;
                double max_seg_size             = traffic_type->max_seg_size;
                double down_up_ratio            = traffic_type->down_up_ratio;
                double mean_seg_read_total      = mean_session / mean_num_seg_per_session;

                if ( WLANNetworkClass::service_type == DataService ) {
                    // generate random number of segment for the new coming session.
                    int    num_seg = 0;
                    double r(0.0), mean(1.0);
                    do {
                        r       = rg->Random();
                        mean    = mean_num_seg_per_session;
                        num_seg = (int) (log( r ) / log( mean/(1.0+mean) ));
                    } while ( num_seg == 0 );

                    ((WLANSessionClass*) call)->num_segment  = num_seg;

                    // Initializes the segment counter with number of segments.
                    ((WLANSessionClass*) call)->seg_cnt_left = num_seg;

                    ((WLANStatCountClass *) stat_count)
                        ->num_segment_request_from_session[traffic_type_idx] += (num_seg-1);
                    ((WLANStatCountClass *) ap->stat_count)
                        ->num_segment_request_from_session[traffic_type_idx] += (num_seg-1);
                }

                ap->call_list->append( call );
                master_call_list->append( call );
                num_call_type[traffic_type_idx] ++;


                if ( WLANNetworkClass::service_type == VoiceService ) {
                    ((WLANStatCountClass *) stat_count)->num_request_connect[traffic_type_idx] ++;
                    ((WLANStatCountClass *) ap->stat_count)->num_request_connect[traffic_type_idx] ++;
                }
                else if ( WLANNetworkClass::service_type == DataService ) {
                    ((WLANStatCountClass *) stat_count)->num_session_request_connect[traffic_type_idx] ++;
                    ((WLANStatCountClass *) ap->stat_count)->num_session_request_connect[traffic_type_idx] ++;
                }


                if (stat->plot_event) {
                    if ( WLANNetworkClass::service_type == VoiceService ) {
                        fprintf(stat->fp_event, "SESSION_CONNECTED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type->name());
                    }
                    else if ( WLANNetworkClass::service_type == DataService ) {
                        fprintf(stat->fp_event, "CALL_CONNECTED TIME = %10.15f TRAFFIC_TYPE = %s\n",
                                abs_time, traffic_type->name());
                    }
                    fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);
                    fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                            idx_to_x(event->posn_x), idx_to_y(event->posn_y));

                    fflush(stat->fp_event);
                }

#if MC_DBG
                std ::cout << " CALL/SESSION REQUEST CONNECTED" << std::endl;
#endif
            }
        }

        delete_call = false;
    }
    // For Voice Service only
    else if ( event->event_type == CConst::HangupEvent )
    {
        // Statistics number of call Hangup.
        // Delete the corresponding call.
        call     = (WLANSessionClass *) (*master_call_list)[event->master_idx];
        cell_idx = call->cell_idx;
        ap_idx   = call->sector_idx;

#if MC_DBG
        std::cout << "\nHangupEvent                 \n";
        std::cout << "size of master call         " << master_call_list->getSize() << std::endl;
        std::cout << "event->master_idx(HANGUP UPDATE)   " << event->master_idx        << std::endl;
        std::cout << "call->master_idx            " << call->master_idx         << std::endl;
        std::cout << "call->call_idx              " << call->call_idx           << std::endl;
        std::cout << "call->cell_idx              " << cell_idx                 << std::endl;
        std::cout << "call->ap_idx                " << call->sector_idx         << std::endl;
        //std::cout << "ap->call_list->getSize()    " << ap->call_list->getSize() << std::endl;
        std::cout << std::endl;
#endif

        cell = (WLANCellClass* ) cell_list[cell_idx];
        ap   = (WLANSectorClass*) cell->sector_list[ap_idx];

        if (stat->plot_event) {
            fprintf(stat->fp_event, "CALL_HANGUP TIME = %10.15f TRAFFIC_TYPE = %s\n",
                    abs_time, traffic_type->name());

            fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);
            fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                    idx_to_x(event->posn_x), idx_to_y(event->posn_y));

            fflush(stat->fp_event);
        }

        ((WLANStatCountClass *) stat_count)->num_hangup[traffic_type_idx] ++;
        ((WLANStatCountClass *) ap->stat_count)->num_hangup[traffic_type_idx] ++;

        delete_call = true;
    }
    // For Voice Service only
    else if ( event->event_type == CConst::DropEvent )
    {
        // Statistics number of call Hangup.
        // Delete the corresponding call.

        call     = (WLANSessionClass *) (*master_call_list)[event->master_idx];
        cell_idx = call->cell_idx;
        ap_idx   = call->sector_idx;
        cell     = (WLANCellClass* ) cell_list[cell_idx];
        ap       = (WLANSectorClass *) cell->sector_list[ap_idx];

        if (stat->plot_event) {
            fprintf(stat->fp_event, "CALL_DROP TIME = %10.15f TRAFFIC_TYPE = %s\n",
                    abs_time, traffic_type->name());

            fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);

            fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                    idx_to_x(event->posn_x), idx_to_y(event->posn_y));
            fflush(stat->fp_event);
        }

        ((WLANStatCountClass *) stat_count)->num_drop[traffic_type_idx] ++;
        ((WLANStatCountClass *) ap->stat_count)->num_drop[traffic_type_idx] ++;

        delete_call = true;
    }

    /* For Date Service
     * If Event type is SegmentRequestEvent, just do some statistics.
     ******************************************************************/
#if 0
    else if ( event->event_type == CConst::SegmentRequestEvent )
    {
        call     = (WLANSessionClass *) (*master_call_list)[event->master_idx];
        cell_idx = call->cell_idx;
        ap_idx   = call->sector_idx;

        cell     = cell_list[cell_idx];
        ap       = (WLANSectorClass *) cell->sector_list[ap_idx];

        if (stat->plot_event) {
            fprintf(stat->fp_event, "SEGMENT_REQUEST TIME = %10.15f TRAFFIC_TYPE = %s\n",
                    abs_time, traffic_type->name());

            fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);

            fflush(stat->fp_event);
        }

        // Note : num_segment_request = num_session_request_during_session + num_session_request;
        //        So, num_session_request_during_session can be devited.
        ((WLANStatCountClass *) stat_count)->num_segment_request_from_segment[traffic_type_idx] ++;
        ((WLANStatCountClass *) ap->stat_count)->num_segment_request_from_segment[traffic_type_idx] ++;

        delete_call = false;
    }
#endif

    // DATA
    else if ( event->event_type == CConst::SegmentHangupEvent )
    {
        call     = (WLANSessionClass *) (*master_call_list)[event->master_idx];
        cell_idx = call->cell_idx;
        ap_idx   = call->sector_idx;
        cell     = (WLANCellClass* ) cell_list[cell_idx];
        ap       = (WLANSectorClass* ) cell->sector_list[ap_idx];

        call->seg_cnt_left --;
        if ( call->seg_cnt_left <= 0 ) call->seg_cnt_left = 0;

        // If the all segment transmited( i.e. session buffer is empty ), end a session.
        if ( ((WLANSessionClass*) call)->seg_cnt_left == 0 ) {
            if (stat->plot_event) {
                fprintf(stat->fp_event, "SESSION_HANGUP TIME = %10.15f TRAFFIC_TYPE = %s\n",
                        abs_time, traffic_type_list[traffic_type_idx]->name());

                fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);

                fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                        idx_to_x(event->posn_x), idx_to_y(event->posn_y));
                fflush(stat->fp_event);
            }
#if MC_DBG
            std::cout << "Session Hangup Event \n";
#endif

            ((WLANStatCountClass *) stat_count)->num_session_hangup[traffic_type_idx] ++;
            ((WLANStatCountClass *) ap->stat_count)->num_session_hangup[traffic_type_idx] ++;

            ((WLANStatCountClass *) stat_count)->num_segment_hangup[traffic_type_idx] --;
            ((WLANStatCountClass *) ap->stat_count)->num_segment_hangup[traffic_type_idx] --;

            // Need to delete the session.
            delete_call = true;
        }
        else {
            delete_call = false;
        }

        if (stat->plot_event) {
            fprintf(stat->fp_event, "SEGMENT_HANGUP TIME = %10.15f TRAFFIC_TYPE = %s\n",
                    abs_time, traffic_type->name());

            fprintf(stat->fp_event, "AP = (%2d, %2d)\n", cell_idx, ap_idx);

            fprintf(stat->fp_event, "POSN = %10.15f, %10.15f\n",
                    idx_to_x(event->posn_x), idx_to_y(event->posn_y));
            fflush(stat->fp_event);
        }

        ((WLANStatCountClass *) stat_count)->num_segment_hangup[traffic_type_idx] ++;
        ((WLANStatCountClass *) ap->stat_count)->num_segment_hangup[traffic_type_idx] ++;

    }


    /*
     *  Voice Service : Delete the corresponding call if the type of the coming event 
     *                  is Drop Event or is Hangup Event.
     *  Data  Service : Delete the corresponding call if the type of the coming event 
     *                  is Session Hangup Event.
     *******************************************************************************/
    if ( delete_call ) {
        master_idx       = event->master_idx;
        call             = (WLANSessionClass *) (*master_call_list)[master_idx];
        cell_idx         = call->cell_idx;
        ap_idx           = call->sector_idx;
        call_idx         = call->call_idx;
        cell             = (WLANCellClass* ) cell_list[cell_idx];
        ap               = (WLANSectorClass *) cell->sector_list[ap_idx];
        traffic_type_idx = call->traffic_type_idx;

#if MC_DBG
        std::cout << "DELETE CALL                 " << std::endl;
        std::cout << "event->event_type           " << event->event_type        << std::endl;
        std::cout << "call->master_idx            " << call->master_idx         << std::endl;
        std::cout << "event->master_idx           " << event->master_idx        << std::endl;
        std::cout << "call->call_idx              " << call_idx                 << std::endl;
        std::cout << "ap->call_list->getSize()    " << ap->call_list->getSize() << std::endl;
        std::cout << std::endl;
#endif

        free( (WLANSessionClass *) (*(ap->call_list))[call_idx]);

        ap->call_list->del_elem_idx(call_idx);
        if (call_idx < ap->call_list->getSize()) {
            ((WLANSessionClass *) (*(ap->call_list))[call_idx])->call_idx = call_idx;
        }

        master_call_list->del_elem_idx(master_idx);
        if (master_idx < master_call_list->getSize()) {
            ((WLANSessionClass *) (*(master_call_list))[master_idx])->master_idx = master_idx;
        }

        num_call_type[traffic_type_idx] --;

        if (stat->plot_event) {
            fprintf(stat->fp_event, "\n");
            fflush(stat->fp_event);
        }

        delete_call = false;
    }

#if 0
    // Keeping the values of all ap's throughput in a list, it can save amount of computation by doing that.
    std::vector <double> thr_list;
#endif

    // Recording the sample results of throughput and delay per 100 events
    // Maybe record it per 5 is more fine.

    // calculate tau_user_inter & p_cond_coll_user_inter after event comming
    tau_with_inter (1E-10);

    if ( WLANNetworkClass::current_event_idx%5 == 0
            && ( stat->plot_throughput || stat->plot_delay || stat->plot_jitter || stat->plot_pkt_loss_rate ) )
    {
        //double throughput_total = 0.0;
        for ( cell_idx=0; cell_idx<num_cell; cell_idx++) {
            cell = (WLANCellClass*) cell_list[cell_idx];
            for ( ap_idx=0; ap_idx<cell->num_sector; ap_idx++ ) {
                ap = (WLANSectorClass*) cell->sector_list[ap_idx];
                int num_call   = ap->call_list->getSize();
                double p_cond_coll = 0.0;
                double tau         = 0.0;
                tau_iteration_no_inter(num_call, p_cond_coll,tau,1E-10);

                if ( stat->plot_delay )
                {
#if 0 //xxxxxx 8/22/05 CG
                    std::vector <int> traffic_idx_ap_list;
                    std::vector <int> num_traffic_type_list;

                    int num_traffic_ap_type = 0;
                    int num_speci_traffic   = 0;
                    traffic_idx_ap_list.clear();
                    num_traffic_type_list.clear();

                    bool found;
                    // Find number and type of traffic in an AP.
                    for ( tt_idx=0; tt_idx<num_traffic_type; tt_idx++ ) {
                        found = false;
                        num_speci_traffic = 0;
                        for ( call_idx=0; call_idx<ap->call_list->getSize(); call_idx++ ) {
                            WLANSessionClass* call = (WLANSessionClass *) (*(ap->call_list))[call_idx];
                            if ( call->traffic_type_idx == tt_idx ) {
                                found = true;
                                num_speci_traffic ++;
                            }
                        }
                        if ( found ) {
                            num_traffic_ap_type ++;
                            traffic_idx_ap_list.push_back( tt_idx );
                            num_traffic_type_list.push_back( num_speci_traffic );
                        }
                    }
#endif

                    // Here ad_seg_type_f is NULL, we only use ad_seg_type_r list which keeps the results of computation in function
                    ad_seg_type_f = comp_mean_delay_a( ap, ad_seg_type_r );

                    // AP xx  TRAFFIC TYPE xx DELAY = xx.
                    fprintf(stat->fp_delay, "AP %3d NUM_USERS %3d DELAY = %10.8f\n",
                            cell_idx, num_call, ad_seg_type_r );
                }
                // write throughput and packet loss rate information to corresponding files
                if ( stat->plot_throughput || stat->plot_pkt_loss_rate ) {
                    double pkt_loss_rate = 0.0;
                    double throughput = throughput_computation( ap,pkt_loss_rate,error );
                    //throughput_total += throughput;

                    if ( stat->plot_throughput ) {
                        fprintf(stat->fp_throughput, "AP %3d NUM_USERS %3d THROUGHPUT = %10.15f \n",
                                cell_idx, num_call, throughput );
                    }
                    if ( stat->plot_pkt_loss_rate )
                    {
                        fprintf(stat->fp_pkt_loss_rate, "AP %3d NUM_USERS %3d PACKET_LOSS_RATE = %10.15f \n",
                                cell_idx, num_call, pkt_loss_rate);
                    }
                }

                // write jitter information to 'fp_jitter'
                // too slow to call comp_jitter() function for each event
                // so, we establish a dictionary for jitter values and we can find the jitter value by number of users
                if ( stat->plot_jitter )
                {
#if MC_DBG
                    if (jitter_fg) {
                        jitter_user.clear();
                        double t_p_cond_coll = 0.0;
                        double t_tau         = 0.0;

                        for (i=0;i<=ac_num_user;i++) {
                            tau_iteration_no_inter(i, t_p_cond_coll,t_tau,1E-10);
                            jitter_user.push_back(comp_jitter( i, t_p_cond_coll, t_tau ));
                        }
                        jitter_fg = false;
                    }

                    fprintf(stat->fp_jitter, "AP %3d NUM_USERS %3d JITTER = %10.15f \n",
                            cell_idx, num_call, jitter_user[num_call] );
#endif

#if !MC_DBG
                    double jitter = comp_jitter( ap );
                    fprintf(stat->fp_jitter, "AP %3d NUM_USERS %3d JITTER = %10.15f \n",
                            cell_idx, num_call, jitter );
#endif
                }
            }
        }
    }

    if ( stat->plot_throughput == 1 ) {
        fprintf(stat->fp_throughput, "\n");
    }
    if ( stat->plot_delay == 1 ) {
        fprintf(stat->fp_delay, "\n");
    }
    if ( stat->plot_jitter == 1 ) {
        fprintf(stat->fp_jitter, "\n");
    }
    if ( stat->plot_pkt_loss_rate == 1 ) {
        fprintf(stat->fp_pkt_loss_rate, "\n");
    }


    // Test function comp_sir_aps, only executing one time.
#if !MC_DBG
    int      num       = 0;
    double   radius    = 120.0;
    double   threshold = 2000.0;
    double*  sir_list  = ( double* ) NULL;

    if ( sir_fg && current_event_idx == 500 ) {
        std::ofstream sir_aps_output;
        sir_aps_output.open("sir_aps.txt");
        if( !sir_aps_output )
            std::cout << "File sir_aps.txt not exist.\n";

        // Calculate SIR
        for (cell_idx=0; cell_idx<num_cell; cell_idx++)
            for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++) num ++;

        sir_list = (double*) malloc ( num * sizeof(double) );
        comp_sir_aps( sir_list, radius, threshold );

        // Print results to file.
        for ( i=0; i<num; i++ )
            sir_aps_output << "AP " << i << " SIR " << sir_list[i] << std::endl;

        sir_aps_output.close();
        sir_fg = false;

        free( sir_list ); sir_list = (double*) NULL;
    }
#endif


#if 0
    if ( thr_opn_fg ) {
        std::ofstream thr_ap_output;
        thr_ap_output.open("thr_single_ap.txt");
        if( !sir_aps_output )
            std::cout << "File \"thr_single_ap.txt\" not exist.\n";

        thr_opn_fg = false;
    }

    // Recording the throughput of an AP in the process of simulation.
    if ( current_event_idx >= 3000 && thr_cls_fg ) {
        for ( cell_idx=0; cell_idx<num_cell; cell_idx++) {
            cell = (WLANCellClass*) cell_list[cell_idx];
            for ( ap_idx=0; ap_idx<cell->num_sector; ap_idx++ ) {
                ap = (WLANSectorClass*) cell->sector_list[ap_idx];
                for ( tt_idx=0; tt_idx<num_traffic_type; tt_idx++ ) {
                    int num_call = ap->call_list->getSize();
                    thr_ap_output << "AP " << cell_idx << " num users " << num_call << " thr " ;
                }
            }
        }

        thr_ap_output.close();
    }

#endif

    tau_user_inter.clear();
    p_cond_coll_user_inter.clear();

    return;
}


#if !MC_DBG
/******************************************************************************************
 * DESCRIPTION: Call function sir_ap() to SIR of all APs.
 * INPUT:       radius:      Radius of planning
 *              r_threshold: threshold of distance we should consider the interference.
 * RETURN:      SIR of APs (dB).
 * NOTE:        The function of SIR computation do not consider the interference
 *              caused by users, So the result is only used for estimating the
 *              effective of interference.
 *              only for expo type propagation. To clutter prop model, signal value ?(R)
 *****************************************************************************************/
void WLANNetworkClass::comp_sir_aps( double* sir_list, double radius, double r_threshold )
{
    int cell_idx   = 0;
    int ap_idx   = 0;

    WLANCellClass*   cell   = (WLANCellClass*)   NULL;
    WLANSectorClass* sector = (WLANSectorClass*) NULL;

    int num = 0;
    for (cell_idx=0; cell_idx<num_cell; cell_idx++) {
        cell = (WLANCellClass*) cell_list[cell_idx];
        for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++) {
            sector = (WLANSectorClass*) cell->sector_list[ap_idx];
            sir_list[num] = sir_ap( sector, radius, r_threshold );
            num ++;
        }
    }
}
#endif


/******************************************************************************************
 * DESCRIPTION: Compute SIR of an ap. 
 *              interference comes from the other APs using same channel.
 * INPUT:       m_cell: Cell object 
 *              radius: Radius of planning
 *              r_threshold: threshold of distance we should consider the interference.
 * OUTPUT:
 * RETURN:      sir of corresponding ap (dB).
 * NOTE:        The function of SIR computation do not consider the interference
 *              caused by users, So the result is only used for estimating the
 *              effective of interference.
 *              only for expo type propagation. To clutter prop model, signal value ?(R)
 *****************************************************************************************/
double WLANNetworkClass::sir_ap( WLANSectorClass* m_ap, double radius, double r_threshold )
{
    double sir_db = 0.0;

    int posn_x   = 0;
    int posn_y   = 0;
    int ap_idx   = 0;
    int cell_idx = 0;
    int prop_model = m_ap->prop_model;
    int chan_idx   = m_ap->chan_idx;

    double dist   = 0.0;
    double signal = 0.0;
    double interf = 0.0;

    WLANCellClass*   cell   = (WLANCellClass*)   NULL;
    WLANSectorClass* sector = (WLANSectorClass*) NULL;
    PropModelClass *pm = prop_model_list[prop_model];

    cell = (WLANCellClass*) m_ap->parent_cell;
    posn_x = cell->posn_x;
    posn_y = cell->posn_y;

    // Suppose the antenna is omni.
    // Because the antenna gain is the factor of signal and interf, so we do not consider it.
    if ( pm->type() == CConst::PropExpo ) {
        for (cell_idx=0; cell_idx<num_cell; cell_idx++) {
            cell = (WLANCellClass*) cell_list[cell_idx];
            for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++) {
                sector = (WLANSectorClass*) cell->sector_list[ap_idx];
                if ( sector == m_ap ) {
                    // std::cout << "Find cell index: " << cell_idx << std::endl;
                }
                else {
                    int dx = cell->posn_x - posn_x;
                    int dy = cell->posn_y - posn_y;
                    dist = (double) sqrt((double)(dx*dx + dy*dy));

                    if ( sector->chan_idx == chan_idx && dist < r_threshold )
                        interf += m_ap->comp_prop(this, cell->posn_x, cell->posn_y);
                }
            }
        }

        // suppose the prop model is expo type, so the power loss be the same value in all direct.
        signal = m_ap->comp_prop(this, (int)(posn_x+radius), posn_y);
    }
    else
    {
        // To be implemented, such as Clutter Model.
    }

#if 0
    std::cout << "signal " << signal << std::endl;
    std::cout << "interf " << interf << std::endl;
#endif

    double sir = signal / interf;
    sir_db = 10.0 * log(sir)/log(10.0);

    return sir_db;
}

/******************************************************************************************
 * DESCRIPTION: Compute power gain (- path loss) of the line from user to an position (x,y)
 * INPUT:       user: radical user of signal
 *              (x,y): position want to compute power loss
 * RETURN:      power in dB
 *****************************************************************************************/
double WLANExpoPM::prop_power_loss(NetworkClass *np, WLANSessionClass* user, int x, int y)
{
    double prop_gain_db = 0.0;

    // propagation model is "pl= -10*a*log10(distance) - c;
    // free space a=2, c=40.07 in 2.4GHz
    // exponent and coefficient are private member, so cannot get it.

    double dx = user->posn_x - x;
    double dy = user->posn_y - y;

    double res = np->resolution;
    double dist = sqrt(res*res*(dx*dx+dy*dy));
    if ( dist<res ) dist = res;

    // DEBUG
#if 0
    std::cout << "res  " << res  << std::endl;
    std::cout << "dx   " << dx   << std::endl;
    std::cout << "dy   " << dy   << std::endl;
    std::cout << "dist " << dist << std::endl;
    std::cout << "a    " << a    << std::endl;
    std::cout << "c    " << c    << std::endl;
#endif

    prop_gain_db = -10.0*a*log10(dist) - c;

    return(prop_gain_db);
}
