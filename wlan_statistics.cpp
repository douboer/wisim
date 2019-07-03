/******************************************************************************************/
/**** FILE: wlan_statistics.cpp                                                        ****/
/******************************************************************************************/

#include "cconst.h"
#include "wlan_statistics.h"
#include "traffic_type.h"
#include "WiSim.h"
#include "wlan.h"

#include <string.h>
#include <iostream>

/******************************************************************************************/
/**** FUNCTION: WLANStatCountClass::WLANStatCountClass                                   ****/
/******************************************************************************************/
WLANStatCountClass::WLANStatCountClass()
{
    num_request         = (int *) malloc(num_traffic_type * sizeof(int));
    num_request_block   = (int *) malloc(num_traffic_type * sizeof(int));
    num_request_connect = (int *) malloc(num_traffic_type * sizeof(int));
    num_hangup          = (int *) malloc(num_traffic_type * sizeof(int));
    num_drop            = (int *) malloc(num_traffic_type * sizeof(int));


    // For Data Service statistics
    num_segment_request_from_session    = (int *) malloc(num_traffic_type * sizeof(int));
    //num_segment_request_from_segment  = (int *) malloc(num_traffic_type * sizeof(int));
    num_segment_hangup                  = (int *) malloc(num_traffic_type * sizeof(int));
    num_session_request                 = (int *) malloc(num_traffic_type * sizeof(int));
    num_session_request_connect         = (int *) malloc(num_traffic_type * sizeof(int));
    num_session_request_timeout         = (int *) malloc(num_traffic_type * sizeof(int));
    num_session_request_block           = (int *) malloc(num_traffic_type * sizeof(int));
    num_session_hangup                  = (int *) malloc(num_traffic_type * sizeof(int));
    //num_segment_request_during_session  = (int *) malloc(num_traffic_type * sizeof(int));

    reset();
}
/******************************************************************************************/
/**** FUNCTION: WLANStatCountClass::~WLANStatCountClass                                ****/
/******************************************************************************************/
WLANStatCountClass::~WLANStatCountClass()
{
    free( num_request );
    free( num_request_block );
    free( num_request_connect );
    free( num_hangup );
    free( num_drop );

    free( num_segment_request_from_session );
    //free( num_segment_request_from_segment );
    free( num_segment_hangup );
    free( num_session_request );
    free( num_session_request_connect );
    free( num_session_request_timeout );
    free( num_session_request_block );
    free( num_session_hangup );
    //free( num_segment_request_during_session );
}
/******************************************************************************************/
/**** FUNCTION: WLANStatCountClass::reset                                              ****/
/******************************************************************************************/
void WLANStatCountClass::reset()
{
    int traffic_type_idx = 0;

    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        if ( WLANNetworkClass::service_type == VoiceService ) {
            num_request[traffic_type_idx]                 = 0;
            num_request_block[traffic_type_idx]           = 0;
            num_request_connect[traffic_type_idx]         = 0;
            num_session_request_timeout[traffic_type_idx] = 0;
            num_hangup[traffic_type_idx]                  = 0;
            num_drop[traffic_type_idx]                    = 0;
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
            // For Data Service statistics
            num_segment_request_from_session[traffic_type_idx]    = 0;
            //num_segment_request_from_segment[traffic_type_idx]  = 0;
            num_segment_hangup[traffic_type_idx]                  = 0;
            num_session_request[traffic_type_idx]                 = 0;
            num_session_request_connect[traffic_type_idx]         = 0;
            num_session_request_timeout[traffic_type_idx]         = 0;
            num_session_request_block[traffic_type_idx]           = 0;
            num_session_hangup[traffic_type_idx]                  = 0;
            //num_segment_request_during_session[traffic_type_idx]  = 0;
        }
    }
}


/******************************************************************************************/
/**** FUNCTION: WLANStatCountClass::add_stat_count                                     ****/
/**** Add two WLANStatCountClass structrure.                                           ****/
/******************************************************************************************/
void WLANStatCountClass::add_stat_count(StatCountClass *s_a, StatCountClass *s_b)
{
    int traffic_type_idx = 0;

    WLANStatCountClass *a = (WLANStatCountClass *) s_a;
    WLANStatCountClass *b = (WLANStatCountClass *) s_b;

    for (traffic_type_idx=0; traffic_type_idx<num_traffic_type; traffic_type_idx++) {
        if ( WLANNetworkClass::service_type == VoiceService ) {
            num_request[traffic_type_idx]         = a->num_request[traffic_type_idx]
                                                  + b->num_request[traffic_type_idx];
            num_request_block[traffic_type_idx]   = a->num_request_block[traffic_type_idx]
                                                  + b->num_request_block[traffic_type_idx];
            num_request_connect[traffic_type_idx] = a->num_request_connect[traffic_type_idx]
                                                  + b->num_request_connect[traffic_type_idx];
            num_hangup[traffic_type_idx]          = a->num_hangup[traffic_type_idx]
                                                  + b->num_hangup[traffic_type_idx];
            num_drop[traffic_type_idx]            = a->num_drop[traffic_type_idx]
                                                  + b->num_drop[traffic_type_idx];
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
#if 0
            a->num_segment_request_during_session[traffic_type_idx] = a->num_segment_request_from_segment[traffic_type_idx]
                - a->num_session_request[traffic_type_idx];
            b->num_segment_request_during_session[traffic_type_idx] = b->num_segment_request_from_segment[traffic_type_idx]
                - b->num_session_request[traffic_type_idx];
#endif

            num_segment_request_from_session[traffic_type_idx]   = a->num_segment_request_from_session[traffic_type_idx]
                                                                 + b->num_segment_request_from_session[traffic_type_idx];
            num_segment_hangup[traffic_type_idx]                 = a->num_segment_hangup[traffic_type_idx]
                                                                 + b->num_segment_hangup[traffic_type_idx];
            num_session_request[traffic_type_idx]                = a->num_session_request[traffic_type_idx]
                                                                 + b->num_session_request[traffic_type_idx];
            num_session_request_connect[traffic_type_idx]        = a->num_session_request_connect[traffic_type_idx]
                                                                 + b->num_session_request_connect[traffic_type_idx];
            num_session_request_timeout[traffic_type_idx]        = a->num_session_request_timeout[traffic_type_idx]
                                                                 + b->num_session_request_timeout[traffic_type_idx];
            num_session_request_block[traffic_type_idx]          = a->num_session_request_block[traffic_type_idx]
                                                                 + b->num_session_request_block[traffic_type_idx];
            num_session_hangup[traffic_type_idx]                 = a->num_session_hangup[traffic_type_idx]
                                                                 + b->num_session_hangup[traffic_type_idx];
#if 0
            num_segment_request_during_session[traffic_type_idx] = a->num_segment_request_during_session[traffic_type_idx]
                                                                 + b->num_segment_request_during_session[traffic_type_idx];
#endif
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: print_stat_count                                                       ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
/**** format = 0: old format, one line per variable                                    ****/
/**** format = 1: one line per sector, tab delimited (headers)                         ****/
/**** format = 2: one line per sector, tab delimited (data)                            ****/
void WLANStatCountClass::print_stat_count(FILE *fp, char *idt, double duration, int format)
{
    int  tt_idx = 0;
    char *chptr, *errmsg, *name;

    errmsg = CVECTOR(MAX_LINE_SIZE);

    double request_rate = 0.0;           // Request rate in times/sec
    double block_rate   = 0.0;
    double connect_rate = 0.0;
    double hangup_rate  = 0.0;
    double drop_rate    = 0.0;


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


    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        name = traffic_type_list[tt_idx]->name();
        chptr = errmsg;

#if 0
        if (format == 0)
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQ          = %d\n", idt, name, num_request[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_TOTAL_NUM_REQ\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQ_BLOCK    = %d\n", idt, name, num_request_block[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_TOTAL_NUM_REQ_BLOCK\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request_block[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQ_CONNECT  = %d\n", idt, name, num_request_connect[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_TOTAL_NUM_REQ_CONNECT\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request_connect[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANGUP           = %d\n", idt, name, num_request_connect[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_TOTAL_NUM_HANGUP\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_hangup[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_TOTAL_DROP                 = %d\n", idt, name, num_request_connect[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_TOTAL_NUM_DROP\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_drop[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_NUM_REQ                = %d\n", idt, name, num_request[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_NUM_REQ\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_NUM_REQ_BLOCK          = %d\n", idt, name, num_request_block[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_NUM_REQ_BLOCK\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request_block[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_NUM_REQ_CONNECT        = %d\n", idt, name, num_request_connect[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_NUM_REQ_CONNECT\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_request_connect[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_NUM_HANGUP                 = %d\n", idt, name, num_hangup[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_NUM_HANGUP\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_hangup[tt_idx]);

        if (format == 0)
            chptr += sprintf(chptr, "%s%s_NUM_DROP                   = %d\n", idt, name, num_drop[tt_idx]);
        else if (format == 1) chptr += sprintf(chptr, "%s_NUM_DROP\t", name);
        else                  chptr += sprintf(chptr, "%d\t", num_drop[tt_idx]);

        PRMSG(fp, errmsg);
#endif

        if (format == 1) chptr += sprintf(chptr, "AP_IDX      ");

        if ( WLANNetworkClass::service_type == VoiceService ) {
            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_REQ                             = %d\n", idt, name, num_request[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_REQ  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_request[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_REQ_BLOCK                       = %d\n", idt, name, num_request_block[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_BLOCK  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_request_block[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_REQ_TIMEOUT         = %d\n", idt, name, num_session_request_timeout[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ_TIMEOUT  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_request_timeout[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_REQ_CONNECT                     = %d\n", idt, name, num_request_connect[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_CONNECT  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_request_connect[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_HANGUP                          = %d\n", idt, name, num_hangup[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_HANGUP  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_hangup[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_DROP                            = %d\n", idt, name, num_drop[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_DROP  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_drop[tt_idx]);
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
#if 0
            num_segment_request_during_session[tt_idx] = num_segment_request_from_segment[tt_idx]
                - num_session_request[tt_idx];
#endif

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SEGM_REQ(BY SEGMENT)     = %d\n", idt, name, 
                        num_segment_request_from_session[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SEGM_REQ(BY SEGMENT)  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_segment_request_from_session[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SEGM_HANGUP                  = %d\n", idt, name, num_segment_hangup[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SEGM_HANGUP  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_segment_hangup[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_REQ                 = %d\n", idt, name, num_session_request[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_request[tt_idx]);

#if 0
            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SEGM_REQ(BY SESSION)  = %d\n", idt, name, 
                        num_segment_request_during_session[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ(BY SESSION) ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_segment_request_during_session[tt_idx]);
#endif

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_REQ_CONNECT         = %d\n", idt, name, num_session_request_connect[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ_CONNECT  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_request_connect[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_REQ_TIMEOUT         = %d\n", idt, name, num_session_request_timeout[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ_TIMEOUT  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_request_timeout[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_REQ_BLOCK           = %d\n", idt, name, num_session_request_block[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_REQ_BLOCK  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_request_block[tt_idx]);

            if (format == 0)
                chptr += sprintf(chptr, "%s%s_NUM_SESS_HANGUP                  = %d\n", idt, name, num_session_hangup[tt_idx]);
            else if (format == 1) chptr += sprintf(chptr, "%s_NUM_SESS_HANGUP  ", name);
            else                  chptr += sprintf(chptr, "%d\t\t", num_session_hangup[tt_idx]);
        }

        if (format == 1) chptr += sprintf(chptr, "\n------------------------------------------------");
        if (format == 1) chptr += sprintf(chptr, "------------------------------------------------\n");

        PRMSG(fp, errmsg);
    }


    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        chptr = errmsg;

        if ( WLANNetworkClass::service_type == VoiceService ) {
            request_rate = (double) num_request[tt_idx]         / duration;
            block_rate   = (double) num_request_block[tt_idx]   / num_request[tt_idx];
            connect_rate = (double) num_request_connect[tt_idx] / num_request[tt_idx];
            hangup_rate  = (double) num_hangup[tt_idx]          / num_request[tt_idx];
            drop_rate    = (double) num_drop[tt_idx]            / num_request[tt_idx];

            if (format == 0) {
                if ( request_rate ) chptr += sprintf(chptr,  "%sREQ_RATE                  = %12.10f\n",   idt, request_rate );
                else chptr += sprintf(chptr,                 "%sREQ_RATE                  = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "REQ_RATE\t");
            else {
                if ( request_rate ) chptr += sprintf(chptr,  "%12.10f\t", request_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( block_rate ) chptr += sprintf(chptr,    "%sBLOCKING_RATE                 = %12.10f\n",   idt, block_rate );
                else chptr += sprintf(chptr,                 "%sBLOCKING_RATE                 = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "BLOCKING_RATE\t");
            else {
                if ( block_rate ) chptr += sprintf(chptr,  "%12.10f\t", block_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( connect_rate ) chptr += sprintf(chptr,  "%sCONNECT_RATE                  = %12.10f\n",   idt, connect_rate );
                else chptr += sprintf(chptr,                 "%sCONNECT_RATE                  = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "CONNECT_RATE\t");
            else {
                if ( connect_rate ) chptr += sprintf(chptr,  "%12.10f\t", connect_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

#if MC_DBG
            if (format == 0) {
                if ( hangup_rate ) chptr += sprintf(chptr,   "%sHANGUP_RATE                   = %12.10f\n",   idt, hangup_rate );
                else chptr += sprintf(chptr,                 "%sHANGUP_RATE                   = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "HANGUP_RATE\t");
            else {
                if ( hangup_rate ) chptr += sprintf(chptr,  "%12.10f\t", hangup_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }
#endif

            if (format == 0) {
                if ( drop_rate ) chptr += sprintf(chptr,     "%sDROP_RATE                     = %12.10f\n",   idt, drop_rate );
                else chptr += sprintf(chptr,                 "%sDROP_RATE                     = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "DROP_RATE  \t");
            else {
                if ( hangup_rate ) chptr += sprintf(chptr,  "%12.10f\t", drop_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }
        }
        else if ( WLANNetworkClass::service_type == DataService ) {
#if 0
            num_segment_request_during_session[tt_idx] = num_segment_request_from_segment[tt_idx]
                - num_session_request[tt_idx];
#endif

            segment_request_time_rate            = (double) num_segment_request_from_session[tt_idx]
                                                 / duration;
            session_request_time_rate            = (double) num_session_request[tt_idx]
                                                 / duration;
            session_request_rate                 = (double) num_session_request[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
            segment_hangup_rate                  = (double) num_segment_hangup[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
            session_hangup_rate                  = (double) num_session_hangup[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
#if 0
            segment_request_during_session_rate  = (double) num_segment_request_during_session[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
#endif
            session_request_connect_rate         = (double) num_session_request_connect[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
            session_request_timeout_rate         = (double) num_session_request_timeout[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];
            session_request_block_rate           = (double) num_session_request_block[tt_idx]
                                                 / num_segment_request_from_session[tt_idx];

            if (format == 0) {
                if ( segment_request_time_rate )
                     chptr += sprintf(chptr,  "%sSEGM_REQ_TIME_RATE          = %12.10f\n",   idt, segment_request_time_rate );
                else chptr += sprintf(chptr,  "%sSEGM_REQ_TIME_RATE          = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SEGM_REQ_TIME_RATE\t");
            else {
                if ( segment_request_time_rate )
                     chptr += sprintf(chptr,  "%12.10f\t", segment_request_time_rate );
                else chptr += sprintf(chptr,  "UNDEFINED\t");
            }

            if (format == 0) {
                if ( session_request_time_rate )
                     chptr += sprintf(chptr,  "%sSESS_REQ_TIME_RATE          = %12.10f\n",   idt, session_request_time_rate );
                else chptr += sprintf(chptr,  "%sSESS_REQ_TIME_RATE          = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SESS_REQ_TIME_RATE\t");
            else {
                if ( session_request_time_rate )
                     chptr += sprintf(chptr, "%12.10f\t", session_request_time_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( session_request_rate )
                     chptr += sprintf(chptr, "%sSESS_REQ_RATE                = %12.10f\n",   idt, session_request_rate );
                else chptr += sprintf(chptr, "%sSESS_REQ_RATE                = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SESS_REQ_RATE\t");
            else {
                if ( session_request_rate )
                     chptr += sprintf(chptr, "%12.10f\t", session_request_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( segment_hangup_rate )
                     chptr += sprintf(chptr, "%sSEGM_HANGUP_RATE                 = %12.10f\n",   idt, segment_hangup_rate );
                else chptr += sprintf(chptr, "%sSEGM_HANGUP_RATE                 = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SEGM_HANGUP_RATE \t");
            else {
                if ( segment_hangup_rate )
                     chptr += sprintf(chptr, "%12.10f\t", segment_hangup_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( session_hangup_rate )
                     chptr += sprintf(chptr, "%sSESS_HANGUP_RATE                 = %12.10f\n",   idt, session_hangup_rate );
                else chptr += sprintf(chptr, "%sSESS_HANGUP_RATE                 = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SESS_HANGUP_RATE \t");
            else {
                if ( session_hangup_rate )
                     chptr += sprintf(chptr, "%12.10f\t", session_hangup_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( segment_request_during_session_rate )
                     chptr += sprintf(chptr, "%sSEGM_REQ_DURING_SESS_RATE = %12.10f\n",
                             idt, segment_request_during_session_rate );
                else chptr += sprintf(chptr, "%sSEGM_REQ_DURING_SESS_RATE = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SEGM_REQ_DURING_SESS_RATE \t");
            else {
                if ( segment_request_during_session_rate )
                     chptr += sprintf(chptr, "%12.10f\t", segment_request_during_session_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( session_request_connect_rate )
                     chptr += sprintf(chptr, "%sSESS_REQ_CONNECT_RATE        = %12.10f\n",   idt, session_request_connect_rate );
                else chptr += sprintf(chptr, "%sSESS_REQ_CONNECT_RATE        = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SESS_REQ_CONNECT_RATE \t");
            else {
                if ( session_request_connect_rate )
                     chptr += sprintf(chptr, "%12.10f\t", session_request_connect_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }

            if (format == 0) {
                if ( session_request_block_rate )
                     chptr += sprintf(chptr, "%sSESS_REQ_BLOCK_RATE          = %12.10f\n",   idt, session_request_block_rate );
                else chptr += sprintf(chptr, "%sSESS_REQ_BLOCK_RATE          = UNDEFINED\n", idt);
            }
            else if (format == 1) chptr += sprintf(chptr, "SESS_REQ_BLOCK_RATE\t");
            else {
                if ( session_request_block_rate )
                     chptr += sprintf(chptr, "%12.10f\t", session_request_block_rate );
                else chptr += sprintf(chptr, "UNDEFINED\t");
            }
        }
    }

    free(errmsg);

    return;
}

/******************************************************************************************/
