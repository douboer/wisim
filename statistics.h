/******************************************************************************************/
/**** FILE: statistics.h                                                               ****/
/**** Michael Mandell 2/12/04                                                          ****/
/******************************************************************************************/

#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>
#include <stdlib.h>

class TrafficTypeClass;

/******************************************************************************************/
/**** CLASS: StatClass                                                                 ****/
/******************************************************************************************/
class StatClass
{
public:

    int plot_num_comm;
    int plot_event;
    int plot_throughput;
    int plot_delay;
    int plot_jitter;
    int plot_pkt_loss_rate;

    FILE *fp_num_comm;
    FILE *fp_event;
    FILE *fp_throughput;
    FILE *fp_delay;
    FILE *fp_jitter;
    FILE *fp_pkt_loss_rate;

    int measure_crr;
    double min_crr, max_crr, *p_crr;
    double n_minus_1_over_max_minus_min_crr;
    int num_crr;

    double duration;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: StatCountClass                                                            ****/
/******************************************************************************************/
class StatCountClass
{
public:
    StatCountClass();
    virtual ~StatCountClass();
    virtual void reset();
    virtual void print_stat_count(FILE *fp, char *idt, double duration, int format);
    virtual void add_stat_count(StatCountClass *a, StatCountClass *b);
    friend class PHSNetworkClass;
    friend class NetworkClass;

protected:
    static int num_traffic_type;
    static TrafficTypeClass **traffic_type_list;
};
/******************************************************************************************/

#endif
