/******************************************************************************************/
/**** FILE: phs_statistics.h                                                           ****/
/******************************************************************************************/

#ifndef PHS_STATISTICS_H
#define PHS_STATISTICS_H

#include <stdio.h>
#include <stdlib.h>

#include "statistics.h"

class TrafficTypeClass;

/******************************************************************************************/
/**** CLASS: PHSStatCountClass                                                         ****/
/******************************************************************************************/
class PHSStatCountClass : public StatCountClass
{
public:
    PHSStatCountClass();
    ~PHSStatCountClass();
    void reset();
    void print_stat_count(FILE *fp, char *idt, double duration, int format);
    void add_stat_count(StatCountClass *a, StatCountClass *b);
    friend class PHSNetworkClass;
    friend class NetworkClass;

private:
    int **num_request, **num_request_block, **num_request_assign, **num_request_blocked_phys_chan;
    int **num_request_blocked_sir_cs, **num_request_blocked_sir_ps;
    int **num_handover,   **num_handover_block,   **num_handover_assign,   **num_handover_blocked_phys_chan;
    int **num_handover_blocked_sir_cs,   **num_handover_blocked_sir_ps;
    int *num_initreq, *num_rereq;
    int *num_drop, *num_mult_handover_drop;
};
/******************************************************************************************/

#endif
