/******************************************************************************************/
/**** PROGRAM: wlan.cpp                                                                ****/
/******************************************************************************************/

#include "cconst.h"
#include "list.h"
#include "wlan.h"
#include "wlan_statistics.h"

#include <math.h>
#include <iostream>

#if HAS_GUI
#include "gconst.h"
#endif


/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
double      WLANSectorClass::bit_rate = 1000000;
ServiceType WLANNetworkClass::service_type = VoiceService;
//ServiceType WLANNetworkClass::service_type = DataService;
/******************************************************************************************/

/******************************************************************************************/
/**** Get / Set functions                                                              ****/
/******************************************************************************************/
const int WLANNetworkClass::technology() { return(CConst::WLAN); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::WLANSectorClass                                       ****/
/******************************************************************************************/
WLANSectorClass::WLANSectorClass(CellClass *cell) : SectorClass(cell)
{
    chan_idx = 1;

    set_default_parameters();
}
/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::~WLANSectorClass                                      ****/
/******************************************************************************************/
WLANSectorClass::~WLANSectorClass()
{
    // To be implemented.

    return;
}
/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::set_default_parameters                                ****/
/******************************************************************************************/
void WLANSectorClass::set_default_parameters()
{
    SectorClass::set_default_parameters();
}
/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::duplicate                                             ****/
/******************************************************************************************/
SectorClass *WLANSectorClass::duplicate(int copy_csid)
{
    WLANSectorClass *new_sector = new WLANSectorClass((CellClass *) NULL);

    copy_sector_values((SectorClass *) new_sector);

    return((SectorClass *) new_sector);
}
/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::copy_sector_values                                    ****/
/******************************************************************************************/
void WLANSectorClass::copy_sector_values(SectorClass *new_sector)
{
    SectorClass::copy_sector_values((SectorClass *) new_sector);

    ((WLANSectorClass *) new_sector)->chan_idx = chan_idx;
}
/******************************************************************************************/
/**** FUNCTION: WLANCellClass::WLANCellClass                                           ****/
/******************************************************************************************/
WLANCellClass::WLANCellClass(int p_num_sector) : CellClass()
{
    int sector_idx;
    num_sector = p_num_sector;
    sector_list = (SectorClass **) malloc((num_sector)*sizeof(SectorClass *));
    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        sector_list[sector_idx] = (SectorClass *) new WLANSectorClass(this);
    }
}
/******************************************************************************************/
/**** FUNCTION: WLANCellClass::view_name                                               ****/
/******************************************************************************************/
char *WLANCellClass::view_name(int cell_idx, int cell_name_pref)
{
    static char str[50];

    switch(cell_name_pref) {
        case CConst::CellIdxRef:
            sprintf(str, "cell_%d", cell_idx);
            break;
#if 0
        case CConst::CellCSNumberRef:
            sprintf(str, "%.6d", ((WLANSectorClass *) sector_list[0])->gw_csc_cs);
            break;
        case GConst::CellHexCSIDRef:
            if (((WLANSectorClass *) sector_list[0])->csid_hex) {
                hex_to_hexstr(str, ((WLANSectorClass *) sector_list[0])->csid_hex, WLANSectorClass::csid_byte_length);
            } else {
                sprintf(str, "**UNASSIGNED**");
            }
            break;
#endif
        default:
            CORE_DUMP; break;
    }

    return(str);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WLANCellClass::~WLANCellClass                                          ****/
/******************************************************************************************/
WLANCellClass::~WLANCellClass()
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WLANCellClass::duplicate                                               ****/
/******************************************************************************************/
CellClass *WLANCellClass::duplicate(const int x, const int y, int copy_csid)
{
    CellClass *new_cell = (CellClass *) new WLANCellClass();

    copy_cell_values(new_cell, x, y, copy_csid);

    return(new_cell);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::WLANNetworkClass                                     ****/
/******************************************************************************************/
WLANNetworkClass::WLANNetworkClass() : NetworkClass()
{
    set_default_parameters();

    subnet_naming_convention = CConst::CellIdxRef;

    use_rts_cts       = 0 ;                // Use RTS/CTS mechanism or not.
    phy_mac_conv      = 11;


    limit_retry       = 2 ;
    max_backoff_stage = 5 ;

    mechanism_block = CConst::RSSIMec;     // The mechanism of an call block ( LOW RSSI, HIGH INT, LOW SIR );

    rssi_threshold_call_block_db = -88.0;
    sir_threshold_call_block_db  =  26.0;
    int_threshold_call_block_db  =  10.0;

    num_freq = 11;

    ac_num_user   = 15;
    ac_activate   = 22000;
    ac_deactivate = 6000.0;

    wpm = new WLANExpoPM();
}

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::~WLANNetworkClass                                    ****/
/******************************************************************************************/
WLANNetworkClass::~WLANNetworkClass()
{
    tau_user_inter.clear();
    p_cond_coll_user_inter.clear();
    delete wpm;

    close_geometry();
}

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::set_default_parameters                               ****/
/******************************************************************************************/
void WLANNetworkClass::set_default_parameters()
{
    NetworkClass::set_default_parameters();

    frequency = 2400.0E6;  /* WLAN Frequency */
}

/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::st_comp_arrival_rate                                   ****/
/**** COPY from PHSSectorClass                                                         ****/
/******************************************************************************************/
double WLANSectorClass::st_comp_arrival_rate(int traffic_idx)
{
    double arrival_rate = 1;

    return (arrival_rate);
}

/******************************************************************************************/
/**** FUNCTION: WLANNetworkClass::process_mode_change                                  ****/
/******************************************************************************************/
void WLANNetworkClass::process_mode_change()
{
    NetworkClass::process_mode_change();
}


/******************************************************************************************
 * DESCRIPTION: Calculate phy_mac_convert by computing RSSI use a special propagation model.
 * RETURN:      Now, we return a default value which is defined in WLANNetworkClass,
 *              i.e. 1, 2, 5.5, 11.
 *****************************************************************************************/
double WLANNetworkClass::phy_mac_convert()
{
    return phy_mac_conv;
}


/******************************************************************************************/
/**** FUNCTION: display_setting                                                        ****/
/**** Display parameter settings                                                       ****/
/******************************************************************************************/
void WLANNetworkClass::display_settings(FILE *fp)
{
    char *chptr;

    NetworkClass::display_settings(fp);

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "WLAN GLOBAL SETTINGS \n");
    PRMSG(fp, msg);
    chptr = msg;
    chptr += sprintf(chptr,      "------------------------------------------------\n");
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf( chptr,     "USE RTS/CTS                   : %s\n",
        ( use_rts_cts == 0 ) ? "NO" : "YES" );
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf( chptr,     "CRITERION BLOCK               : " );
    if ( mechanism_block == CConst::RSSIMec ) {
        chptr += sprintf( chptr, "RSSI\n" );
        chptr += sprintf( chptr, "RSSI_THRESHOLD_CALL_BLOCK_DB  : %5.3f dB\n", rssi_threshold_call_block_db );
    }
    else if ( mechanism_block == CConst::SIRMec ) {
        chptr += sprintf( chptr, "SIR\n" );
        chptr += sprintf( chptr, "SIR_THRESHOLD_CALL_BLOCK_DB   : %5.3f dB\n", sir_threshold_call_block_db );
    }
    else if ( mechanism_block == CConst::INTMec ) {
        chptr += sprintf( chptr, "INT\n" );
        chptr += sprintf( chptr, "INT_THRESHOLD_CALL_BLOCK_DB   : %5.3f dB\n", int_threshold_call_block_db );
    }
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf( chptr,     "PHY / MAC CONVERT FACTOR      : %5.3f ", phy_mac_convert() );
    PRMSG(fp, msg);


    WLANTrafficTypeClass* traffic_type = (WLANTrafficTypeClass*) NULL;
    int tt_idx = 0;
    for ( tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++ ) {
        traffic_type = (WLANTrafficTypeClass*) traffic_type_list[tt_idx];
        chptr = msg;
        chptr += sprintf( chptr, "\n\n" );
        chptr += sprintf( chptr, "TRAFFIC TYPE SETTING   %s \n", traffic_type->get_strid() );
        chptr += sprintf( chptr, "------------------------------------------------\n");
        chptr += sprintf( chptr, "BITS PER PACKET               : %5d \n", traffic_type->bit_per_pkt );
        chptr += sprintf( chptr, "DATA RATE of USER             : %8.5f \n", traffic_type->user_data_rate );
        chptr += sprintf( chptr, "DATA RATE of AP               : %8.5f \n", traffic_type->basic_rate );
        chptr += sprintf( chptr, "LIMIT of RETRY                : %5d \n", limit_retry );
        chptr += sprintf( chptr, "TOTAL PKT TO DROP             : %5d \n", traffic_type->drop_total_pkt );
        chptr += sprintf( chptr, "THRESHOLD of PKT LOSS TO DROP : %5d \n", traffic_type->drop_error_pkt );

        // Data Service Setting
        chptr += sprintf( chptr, "SESSION_REQ_RATE              : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MEAN_NUM_SEG_PER_SESSION      : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MEAN_SEGMENT                  : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MEAN_SESSION                  : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MEAN_READ_TIME                : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MIN_SEG_SIZE                  : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MAX_SEG_SIZE                  : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "DOWN_UP_RATIO                 : %5d \n", traffic_type->drop_error_pkt );
        chptr += sprintf( chptr, "MEAN_SEG_READ_TOTAL           : %5d \n", traffic_type->drop_error_pkt );

        PRMSG(fp, msg);
    }

    chptr = msg;
    chptr += sprintf(chptr,      "------------------------------------------------\n");
    PRMSG(fp, msg);

    return;
}
/******************************************************************************************/

#if HAS_MONTE_CARLO
/******************************************************************************************/
/**** FUNCTION: print_call_statistics                                                  ****/
/**** Print measured statistics for call requests, calls blocked, and calls dropped    ****/
/******************************************************************************************/
void WLANNetworkClass::print_call_statistics(char *filename, char *sector_list_str, int format)
{
    std::cout << "WLANNetworkClass::print_call_statistics ... \n"; 

    int i, index, cell_idx, sector_idx;
    char *chptr;
    CellClass *cell;
    SectorClass *sector;
    StatClass* sp;
    WLANStatCountClass *grp_stat_count, *wlan_stat_count;
    ListClass<int> *int_list = new ListClass<int>(0);
    FILE *fp;

    double request_rate = 0.0;     // Number of Request in one sec;
    double block_rate   = 0.0;     // User's Perceived Blocking Rate;
    double connect_rate = 0.0;     // Connect Rate of Request;
    double hangup_rate  = 0.0;     // Hangup Rate of Request;
    double drop_rate    = 0.0;     // Drop Rate of Request;

    // Variables for Data Service statistics
    double segment_request_time_rate            = 0.0;   // Segment Request rate in times/sec
    double session_request_time_rate            = 0.0;   // Session Request rate in times/sec
    double session_request_rate                 = 0.0;
    double segment_hangup_rate                  = 0.0;
    double session_hangup_rate                  = 0.0;
    double segment_request_during_session_rate  = 0.0;
    double session_request_connect_rate         = 0.0;
    double session_request_timeout_rate         = 0.0;
    double session_request_block_rate           = 0.0;

    int    tt_idx = 0;  // Traffic type Index

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

    wlan_stat_count = (WLANStatCountClass *) stat_count;

    sp = stat;
    chptr = msg;
    chptr += sprintf(chptr, "\nSTATISTICAL_MEASUREMENT_DURATION    = %15.10f (sec)\n", sp->duration);
    PRMSG(fp, msg);


#if !MC_DBG
    std::cout << std::endl;
    for (tt_idx=0; tt_idx<num_traffic_type; tt_idx++) {
        // Voice Service Statistics
        if ( WLANNetworkClass::service_type == VoiceService ) {
            std::cout << "NUM REQ                     " << (double) wlan_stat_count->num_request[tt_idx]         << std::endl;
            std::cout << "NUM BLOCK                   " << (double) wlan_stat_count->num_request_block[tt_idx]   << std::endl;
            std::cout << "NUM CONNECT                 " << (double) wlan_stat_count->num_request_connect[tt_idx] << std::endl;
            std::cout << "NUM_SESSION_REQUEST_TIMEOUT " << (double) wlan_stat_count->num_session_request_timeout[tt_idx] << std::endl;
            std::cout << "NUM HANGUP                  " << (double) wlan_stat_count->num_hangup[tt_idx]          << std::endl;
            std::cout << "NUM DROP                    " << (double) wlan_stat_count->num_drop[tt_idx]            << std::endl;
            std::cout << "NUM CALL IN NETWORK         " << master_call_list->getSize()                           << std::endl;
        

            request_rate = (double) wlan_stat_count->num_request[tt_idx]         / sp->duration;
            block_rate   = (double) wlan_stat_count->num_request_block[tt_idx]   / wlan_stat_count->num_request[tt_idx];
            connect_rate = (double) wlan_stat_count->num_request_connect[tt_idx] / wlan_stat_count->num_request[tt_idx];
            drop_rate    = (double) wlan_stat_count->num_drop[tt_idx]            / wlan_stat_count->num_request[tt_idx];

            std::cout << std::endl;
            std::cout << "DURATION      " << sp->duration   << std::endl;
            //std::cout << "RATE REQ      " << request_rate   << std::endl;
            std::cout << "RATE BLOCK    " << block_rate     << std::endl;
            std::cout << "RATE CONNECT  " << connect_rate   << std::endl;
            std::cout << "RATE DROP     " << drop_rate      << std::endl;
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
            // Data Service Statistics
#if 0
            wlan_stat_count->num_segment_request_during_session[tt_idx] = 
                wlan_stat_count->num_segment_request_from_segment[tt_idx] 
                - wlan_stat_count->num_session_request[tt_idx];
#endif

            std::cout << std::endl;
            std::cout << "NUM_SEGMENT_REQUEST(BY SESSION)     " 
                      << (double) wlan_stat_count->num_segment_request_from_session[tt_idx] << std::endl;
            std::cout << "NUM_SESSION_REQUEST                 " 
                      << (double) wlan_stat_count->num_session_request[tt_idx] << std::endl;
            std::cout << "NUM_SEGMENT_HANGUP                  "
                      << (double) wlan_stat_count->num_segment_hangup[tt_idx]  << std::endl;
            std::cout << "NUM_SESSION_HANGUP                  "
                      << (double) wlan_stat_count->num_session_hangup[tt_idx]  << std::endl;
#if 0
            std::cout << "NUM_SEGMENT_REQUEST(BY SEGMENT)     "
                      << (double) wlan_stat_count->num_segment_request_during_session[tt_idx] << std::endl;
#endif
            std::cout << "NUM_SESSION_REQUEST_CONNECT         "
                      << (double) wlan_stat_count->num_session_request_connect[tt_idx] << std::endl;
            std::cout << "NUM_SESSION_REQUEST_TIMEOUT         "
                      << (double) wlan_stat_count->num_session_request_timeout[tt_idx] << std::endl;
            std::cout << "NUM_SESSION_REQUEST_BLOCK           "
                      << (double) wlan_stat_count->num_session_request_block[tt_idx] << std::endl;

            segment_request_time_rate           = (double) wlan_stat_count->num_segment_request_from_session[tt_idx]
                                                / sp->duration;
            session_request_time_rate           = (double) wlan_stat_count->num_session_request[tt_idx]
                                                / sp->duration;
            session_request_rate                = (double) wlan_stat_count->num_session_request[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            segment_hangup_rate                 = (double) wlan_stat_count->num_segment_hangup[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_hangup_rate                 = (double) wlan_stat_count->num_session_hangup[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
#if 0
            segment_request_during_session_rate = (double) wlan_stat_count->num_segment_request_during_session[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
#endif
            session_request_connect_rate        = (double) wlan_stat_count->num_session_request_connect[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_request_timeout_rate        = (double) wlan_stat_count->num_session_request_timeout[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_request_block_rate          = (double) wlan_stat_count->num_session_request_block[tt_idx]
                                                   / wlan_stat_count->num_segment_request_from_session[tt_idx];

            std::cout << std::endl;
            std::cout << "SEGMENT_REQUEST_TIME_RATE           " << segment_request_time_rate           << std::endl;
            std::cout << "SESSION_REQUEST_TIME_RATE           " << session_request_time_rate           << std::endl;
            std::cout << "SESSION_REQUEST_RATE                " << session_request_rate                << std::endl;
            std::cout << "SEGMENT_HANGUP_RATE                 " << segment_hangup_rate                 << std::endl;
            std::cout << "SESSION_HANGUP_RATE                 " << session_hangup_rate                 << std::endl;
            std::cout << "SEGMENT_REQUEST_DURING_SESSION_RATE " << segment_request_during_session_rate << std::endl;
            std::cout << "SESSION_REQUEST_CONNECT_RATE        " << session_request_connect_rate        << std::endl;
            std::cout << "SESSION_REQUEST_BLOCK_RATE          " << session_request_block_rate          << std::endl;
            std::cout << std::endl;
        }
    }
#endif

    // Static for special type of traffic.
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {

        if ( WLANNetworkClass::service_type == VoiceService ) {
            request_rate = (double) wlan_stat_count->num_request[tt_idx]         / sp->duration;
            block_rate   = (double) wlan_stat_count->num_request_block[tt_idx]   / wlan_stat_count->num_request[tt_idx];
            connect_rate = (double) wlan_stat_count->num_request_connect[tt_idx] / wlan_stat_count->num_request[tt_idx];
            hangup_rate  = (double) wlan_stat_count->num_hangup[tt_idx]          / wlan_stat_count->num_request[tt_idx];
            drop_rate    = (double) wlan_stat_count->num_drop[tt_idx]            / wlan_stat_count->num_request[tt_idx];


            if ( !request_rate ) request_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_USER_REQUEST_RATE             = %15.10f (num/sec)\n",
                    traffic_type_list[tt_idx]->get_strid(), request_rate );
            PRMSG(fp, msg);

            if ( !block_rate ) block_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_USER_PERCEIVED_BLOCKING_RATE  = %15.10f\n",
                    traffic_type_list[tt_idx]->get_strid(), block_rate );
            PRMSG(fp, msg);

            if ( !connect_rate ) connect_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_USER_CONNECT_RATE             = %15.10f\n",
                    traffic_type_list[tt_idx]->get_strid(), connect_rate );
            PRMSG(fp, msg);

#if 0
            if ( !hangup_rate ) hangup_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_USER_HANGUP_RATE             = %15.10f\n",
                    traffic_type_list[tt_idx]->get_strid(), hangup_rate );
            PRMSG(fp, msg);
#endif

            if ( !drop_rate ) drop_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_USER_DROP_RATE                = %15.10f\n",
                    traffic_type_list[tt_idx]->get_strid(), drop_rate );
            PRMSG(fp, msg);
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
            // Data Service Statistics
#if 0
            wlan_stat_count->num_segment_request_during_session[tt_idx] = 
                wlan_stat_count->num_segment_request_from_segment[tt_idx] 
                - wlan_stat_count->num_session_request[tt_idx];
#endif

            segment_request_time_rate           = (double) wlan_stat_count->num_segment_request_from_session[tt_idx]
                                                / sp->duration;
            session_request_time_rate           = (double) wlan_stat_count->num_session_request[tt_idx]
                                                / sp->duration;
            session_request_rate                = (double) wlan_stat_count->num_session_request[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            segment_hangup_rate                 = (double) wlan_stat_count->num_segment_hangup[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_hangup_rate                 = (double) wlan_stat_count->num_session_hangup[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
#if 0
            segment_request_during_session_rate = (double) wlan_stat_count->num_segment_request_during_session[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
#endif
            session_request_connect_rate        = (double) wlan_stat_count->num_session_request_connect[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_request_timeout_rate        = (double) wlan_stat_count->num_session_request_timeout[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];
            session_request_block_rate          = (double) wlan_stat_count->num_session_request_block[tt_idx]
                                                / wlan_stat_count->num_segment_request_from_session[tt_idx];

            if ( !segment_request_time_rate ) segment_request_time_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SEGMENT_REQUEST_TIME_RATE           = %15.10f (num/sec)\n",
                    traffic_type_list[tt_idx]->get_strid(), segment_request_time_rate );
            PRMSG(fp, msg);

            if ( !session_request_time_rate ) session_request_time_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SESSION_REQUEST_TIME_RATE           = %15.10f (num/sec)\n",
                    traffic_type_list[tt_idx]->get_strid(), session_request_time_rate );
            PRMSG(fp, msg);

            if ( !session_request_rate ) session_request_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SESSION_REQUEST_RATE                = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), session_request_rate );
            PRMSG(fp, msg);

            if ( !segment_hangup_rate ) segment_hangup_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SEGMENT_HANGUP_RATE                 = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), segment_hangup_rate );
            PRMSG(fp, msg);

            if ( !session_hangup_rate ) session_hangup_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SESSION_HANGUP_RATE                 = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), session_hangup_rate );
            PRMSG(fp, msg);

#if 0
            if ( !segment_request_during_session_rate ) segment_request_during_session_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SEGMENT_REQUEST_DURING_SESSION_RATE = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), segment_request_during_session_rate );
            PRMSG(fp, msg);
#endif

            if ( !session_request_connect_rate ) session_request_connect_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SESSION_REQUEST_CONNECT_RATE        = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), session_request_connect_rate );
            PRMSG(fp, msg);

            if ( !session_request_block_rate ) session_request_block_rate = 0.0;
            chptr = msg;
            chptr += sprintf(chptr, "%s_SESSION_REQUEST_BLOCK_RATE          = %15.10f \n",
                    traffic_type_list[tt_idx]->get_strid(), session_request_block_rate );
            PRMSG(fp, msg);
        }
    }


    // Static for APs.
    chptr = msg;
    if (int_list->getSize() == 0) {
        chptr += sprintf(chptr, "\nMEASUREMENTS FOR ALL APS IN THE SYSTEM\n");
        PRMSG(fp, msg);

        chptr = msg;
        chptr += sprintf(chptr, "------------------------------------------------");
        chptr += sprintf(chptr, "------------------------------------------------\n");
        PRMSG(fp, msg);

        if (format == 1)
            wlan_stat_count->print_stat_count(fp, "", stat->duration, 1);

        for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
            if (format == 0) {
                chptr = msg;
                chptr += sprintf(chptr, "CELL %d:\n", cell_idx);
                PRMSG(fp, msg);
            }
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                chptr = msg;
                if (format == 0) {
                    chptr += sprintf(chptr, "  AP %d:\n", sector_idx);
                } else {
                    chptr += sprintf(chptr, "AP_%d_%d\t:\t\t", cell_idx, sector_idx);
                }
                PRMSG(fp, msg);
                sector->stat_count->print_stat_count(fp, "\t", stat->duration, (format==0 ? 0 : 2));

                chptr = msg;
                chptr += sprintf(chptr, "\n");
                PRMSG(fp, msg);
            }
        }
        if (format == 1) {
            chptr = msg;
            chptr += sprintf(chptr, "------------------------------------------------");
            chptr += sprintf(chptr, "------------------------------------------------\n");
            PRMSG(fp, msg);

            chptr = msg;
            chptr += sprintf(chptr, "TOTAL\t:\t\t");
            PRMSG(fp, msg);
        }
        stat_count->print_stat_count(fp, "", stat->duration, (format==0 ? 0 : 2));
    } else {
        chptr += sprintf(chptr, "MEASUREMENTS FOR (%d) APS IN THE SYSTEM\n", int_list->getSize());
        chptr += sprintf(chptr, "AP_LIST:");
        for (i=0; i<=int_list->getSize()-1; i++) {
            cell_idx = (*int_list)[i] & ( (1 << bit_cell) - 1 );
            sector_idx = (*int_list)[i] >> bit_cell;
            chptr += sprintf(chptr, " %d_%d", cell_idx, sector_idx);
        }
        chptr += sprintf(chptr, "\n");
        PRMSG(fp, msg);

        grp_stat_count = new WLANStatCountClass();

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

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
#endif


/******************************************************************************************
 * DESCRIPTION: Compute SIR of a call/user
 * INPUT:       ap_idx: sector index of the cell
 *              (call_px, call_py): position of the given call/user.
 *****************************************************************************************/
double WLANNetworkClass::comp_sir_call( int ap_idx, int call_px, int call_py )
{
    double sir_call = 100;

    // Need to be implemented.

    return sir_call;
}


/******************************************************************************************
 * DESCRIPTION: Compute INT of a call/user
 * INPUT:       ap_idx: sector index of the cell
 *              (call_px, call_py): position of the given call/user.
 *****************************************************************************************/
double WLANNetworkClass::comp_int_call( int ap_idx, int call_px, int call_py )
{
    double int_call = 0;

    // Need to be implemented.

    return int_call; 
}


/******************************************************************************************/
/**** FUNCTION: check_parameters                                                       ****/
/******************************************************************************************/
int WLANNetworkClass::check_parameters()
{
//  int cell_idx, sector_idx, unused_freq_idx;
    int num_error;
//  CellClass *cell;
//  WLANSectorClass *sector;

    num_error = 0;

    if (num_error) {
        error_state = 1;
    }

    return(num_error);
}


/******************************************************************************************/
/**** FUNCTION: WLANTrafficTypeClass::WLANTrafficTypeClass                             ****/
/******************************************************************************************/
WLANTrafficTypeClass::WLANTrafficTypeClass(char *p_strid)
    : TrafficTypeClass(p_strid)
{
    duration_dist            = CConst::ExpoDist;

    mean_session_duration    = 100.0;
    mean_drop_duration       = 300.0;

    packet_loss_thr          = 0.05;
    duration_dropcall_thr    = 100;
    max_consecut_drop_pkt    = 5;

    user_data_rate           = 64;   // in kbps
    basic_rate               = 1 ;   // in Mbps
    phy_rate                 = 11;   // in Mbps
    hd_rate                  = 2;    // in Mbps
    down_up_ratio            = 9;

    bit_per_pkt              = 8084;
    drop_error_pkt           = 5;
    drop_total_pkt           = 128;

    // variables for data service
    // default values ?
    mean_num_seg_per_session = 10;
    mean_segment             = 20;
    mean_read_time           = 20;
    mean_session             = 200;
    min_seg_size             = 5;
    max_seg_size             = 5000;
    dur_compress             = 15;
    mean_seg_read_total      = mean_session / mean_num_seg_per_session;

}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WLANTrafficTypeClass::~WLANTrafficTypeClass                            ****/
/******************************************************************************************/
WLANTrafficTypeClass::~WLANTrafficTypeClass()
{
}
/******************************************************************************************/

/******************************************************************************************
 * Constructor function
 *****************************************************************************************/
WLANExpoPM::WLANExpoPM()
{
    // initial
    a = 2.5;
    c = 40.7;
}
/******************************************************************************************/
