/******************************************************************************************/
/**** FILE: wlan_statistics.h                                                           ****/
/******************************************************************************************/

#ifndef WLAN_STATISTICS_H
#define WLAN_STATISTICS_H

#include <stdio.h>
#include <stdlib.h>

#include "statistics.h"

class TrafficTypeClass;

/******************************************************************************************/
/**** CLASS: StatCountClass                                                            ****/
/******************************************************************************************/
class WLANStatCountClass : public StatCountClass
{
public:
    WLANStatCountClass();
    ~WLANStatCountClass();
    void reset();
    void print_stat_count(FILE *fp, char *idt, double duration, int format);
    void add_stat_count(StatCountClass *a, StatCountClass *b);

    friend class WLANNetworkClass;
    friend class NetworkClass;

private:
    int* num_request;
    int* num_request_block;
    int* num_request_connect;
    int* num_hangup;
    int* num_drop;

    // Variables used to keep data service simulation statistics results.
    int* num_segment_request_from_segment;          // Number of segment request event
    int* num_segment_request_from_session;          // Number of segment request event
    int* num_segment_request_during_session;        // Number of segment request event when the correspanding session is connection.
    int* num_segment_hangup;                        // Number of segment hangup event
    int* num_session_request;                       // Number of session request
    int* num_session_request_connect;               // Number of successful session request 
    int* num_session_request_block;                 // Number of blocked session request
    int* num_session_hangup;                        // Number of session hangup, i.e. end of a session
    int* num_session_request_timeout;               // Number of session request time out
};
/******************************************************************************************/

#endif
