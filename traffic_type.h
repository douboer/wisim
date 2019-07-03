/******************************************************************************************/
/**** FILE: traffic_type.h                                                             ****/
/**** Michael Mandell 2/6/04                                                           ****/
/******************************************************************************************/

#ifndef TRAFFIC_TYPE_H
#define TRAFFIC_TYPE_H

#include <stdlib.h>

class EventClass;
class NetworkClass;
class PHSNetworkClass;
class SubnetTraffic;
/******************************************************************************************/
/**** CLASS: TrafficTypeClass                                                          ****/
/******************************************************************************************/
class TrafficTypeClass
{
public:
    TrafficTypeClass(char *s = (char *) NULL);
    virtual ~TrafficTypeClass();
    char *name();
    int get_color();
    char *get_strid();
    double get_mean_time();
    double get_min_time();
    double get_max_time();
    virtual int check(NetworkClass *np);
    friend class NetworkClass;
    friend class PHSNetworkClass;
    friend class WCDMANetworkClass;
    friend class CDMA2000NetworkClass;
    friend class WLANNetworkClass;
    friend class TrafficDialog;
    friend class SubnetTraffic;

private:
    char *strid;
    int color;
    int duration_dist;

    double mean_time;     // Should be renamed to hangup_mean_time, which is a base data member.
    double min_time;      // PHS only, should be moved to PhsTrafficClass.
    double max_time;      // PHS only, should be moved to PhsTrafficClass.
};
/******************************************************************************************/

#endif
