/******************************************************************************************/
/**** FILE: statistics.cpp                                                             ****/
/**** Michael Mandell 2/12/04                                                          ****/
/******************************************************************************************/

#include <string.h>
#include "cconst.h"
#include "phs_statistics.h"
#include "traffic_type.h"
#include "WiSim.h"
#include "phs.h"

/******************************************************************************************/
/**** FUNCTION: PHSStatCountClass::PHSStatCountClass                                   ****/
/******************************************************************************************/
PHSStatCountClass::PHSStatCountClass()
{
    int traffic_type_idx, num_attempt;

    num_request                   = (int **) malloc(num_traffic_type * sizeof(int *));
    num_request_block             = (int **) malloc(num_traffic_type * sizeof(int *));
    num_request_assign            = (int **) malloc(num_traffic_type * sizeof(int *));
    num_request_blocked_phys_chan = (int **) malloc(num_traffic_type * sizeof(int *));
    num_request_blocked_sir_cs    = (int **) malloc(num_traffic_type * sizeof(int *));
    num_request_blocked_sir_ps    = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover                  = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover_block             = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover_assign            = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover_blocked_phys_chan = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover_blocked_sir_cs    = (int **) malloc(num_traffic_type * sizeof(int *));
    num_handover_blocked_sir_ps    = (int **) malloc(num_traffic_type * sizeof(int *));
    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
        num_request[traffic_type_idx]                   = IVECTOR(num_attempt);
        num_request_block[traffic_type_idx]             = IVECTOR(num_attempt);
        num_request_assign[traffic_type_idx]            = IVECTOR(num_attempt);
        num_request_blocked_phys_chan[traffic_type_idx] = IVECTOR(num_attempt);
        num_request_blocked_sir_cs[traffic_type_idx]    = IVECTOR(num_attempt);
        num_request_blocked_sir_ps[traffic_type_idx]    = IVECTOR(num_attempt);

        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        num_handover[traffic_type_idx]                   = IVECTOR(num_attempt);
        num_handover_block[traffic_type_idx]             = IVECTOR(num_attempt);
        num_handover_assign[traffic_type_idx]            = IVECTOR(num_attempt);
        num_handover_blocked_phys_chan[traffic_type_idx] = IVECTOR(num_attempt);
        num_handover_blocked_sir_cs[traffic_type_idx]    = IVECTOR(num_attempt);
        num_handover_blocked_sir_ps[traffic_type_idx]    = IVECTOR(num_attempt);
    }

    num_initreq           = IVECTOR(num_traffic_type);
    num_rereq             = IVECTOR(num_traffic_type);
    num_drop              = IVECTOR(num_traffic_type);
    num_mult_handover_drop = IVECTOR(num_traffic_type);

    reset();
}
/******************************************************************************************/
/**** FUNCTION: PHSStatCountClass::~PHSStatCountClass                                  ****/
/******************************************************************************************/
PHSStatCountClass::~PHSStatCountClass()
{
    int traffic_type_idx, num_attempt;

    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);

        if (num_attempt) {
            free(num_request[traffic_type_idx]);
            free(num_request_block[traffic_type_idx]);
            free(num_request_assign[traffic_type_idx]);
            free(num_request_blocked_phys_chan[traffic_type_idx]);
            free(num_request_blocked_sir_cs[traffic_type_idx]);
            free(num_request_blocked_sir_ps[traffic_type_idx]);
        }

        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        if (num_attempt) {
            free(num_handover[traffic_type_idx]);
            free(num_handover_block[traffic_type_idx]);
            free(num_handover_assign[traffic_type_idx]);
            free(num_handover_blocked_phys_chan[traffic_type_idx]);
            free(num_handover_blocked_sir_cs[traffic_type_idx]);
            free(num_handover_blocked_sir_ps[traffic_type_idx]);
        }
    }
    free(num_request);
    free(num_request_block);
    free(num_request_assign);
    free(num_request_blocked_phys_chan);
    free(num_request_blocked_sir_cs);
    free(num_request_blocked_sir_ps);
    free(num_handover);
    free(num_handover_block);
    free(num_handover_assign);
    free(num_handover_blocked_phys_chan);
    free(num_handover_blocked_sir_cs);
    free(num_handover_blocked_sir_ps);

    free(num_initreq);
    free(num_rereq);
    free(num_drop);
    free(num_mult_handover_drop);
}
/******************************************************************************************/
/**** FUNCTION: PHSStatCountClass::reset                                               ****/
/******************************************************************************************/
void PHSStatCountClass::reset()
{
    int i, traffic_type_idx, num_attempt;

    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
        for (i=0; i<=num_attempt-1; i++) {
            num_request[traffic_type_idx][i]                   = 0;
            num_request_block[traffic_type_idx][i]             = 0;
            num_request_assign[traffic_type_idx][i]            = 0;
            num_request_blocked_phys_chan[traffic_type_idx][i] = 0;
            num_request_blocked_sir_cs[traffic_type_idx][i]    = 0;
            num_request_blocked_sir_ps[traffic_type_idx][i]    = 0;
        }
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        for (i=0; i<=num_attempt-1; i++) {
            num_handover[traffic_type_idx][i]                   = 0;
            num_handover_block[traffic_type_idx][i]             = 0;
            num_handover_assign[traffic_type_idx][i]            = 0;
            num_handover_blocked_phys_chan[traffic_type_idx][i] = 0;
            num_handover_blocked_sir_cs[traffic_type_idx][i]    = 0;
            num_handover_blocked_sir_ps[traffic_type_idx][i]    = 0;
        }

        num_initreq[traffic_type_idx]           = 0;
        num_rereq[traffic_type_idx]             = 0;
        num_drop[traffic_type_idx]              = 0;
        num_mult_handover_drop[traffic_type_idx] = 0;
    }
}
/******************************************************************************************/
/**** FUNCTION: PHSStatCountClass::add_stat_count                                      ****/
/**** Add two PHSStatCountClass structrure.                                            ****/
/******************************************************************************************/
void PHSStatCountClass::add_stat_count(StatCountClass *s_a, StatCountClass *s_b)
{
    int i, traffic_type_idx, num_attempt;

    PHSStatCountClass *a = (PHSStatCountClass *) s_a;
    PHSStatCountClass *b = (PHSStatCountClass *) s_b;

    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent);
        for (i=0; i<=num_attempt-1; i++) {
            num_request[traffic_type_idx][i]                   = a->num_request[traffic_type_idx][i]
                                                               + b->num_request[traffic_type_idx][i];
            num_request_block[traffic_type_idx][i]             = a->num_request_block[traffic_type_idx][i]
                                                               + b->num_request_block[traffic_type_idx][i];
            num_request_assign[traffic_type_idx][i]            = a->num_request_assign[traffic_type_idx][i]
                                                               + b->num_request_assign[traffic_type_idx][i];
            num_request_blocked_phys_chan[traffic_type_idx][i] = a->num_request_blocked_phys_chan[traffic_type_idx][i]
                                                               + b->num_request_blocked_phys_chan[traffic_type_idx][i];
            num_request_blocked_sir_cs[traffic_type_idx][i]    = a->num_request_blocked_sir_cs[traffic_type_idx][i]
                                                               + b->num_request_blocked_sir_cs[traffic_type_idx][i];
            num_request_blocked_sir_ps[traffic_type_idx][i]    = a->num_request_blocked_sir_ps[traffic_type_idx][i]
                                                               + b->num_request_blocked_sir_ps[traffic_type_idx][i];
        }
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent);
        for (i=0; i<=num_attempt-1; i++) {
            num_handover[traffic_type_idx][i]                   = a->num_handover[traffic_type_idx][i]
                                                                + b->num_handover[traffic_type_idx][i];
            num_handover_block[traffic_type_idx][i]             = a->num_handover_block[traffic_type_idx][i]
                                                                + b->num_handover_block[traffic_type_idx][i];
            num_handover_assign[traffic_type_idx][i]            = a->num_handover_assign[traffic_type_idx][i]
                                                                + b->num_handover_assign[traffic_type_idx][i];
            num_handover_blocked_phys_chan[traffic_type_idx][i] = a->num_handover_blocked_phys_chan[traffic_type_idx][i]
                                                                + b->num_handover_blocked_phys_chan[traffic_type_idx][i];
            num_handover_blocked_sir_cs[traffic_type_idx][i]    = a->num_handover_blocked_sir_cs[traffic_type_idx][i]
                                                                + b->num_handover_blocked_sir_cs[traffic_type_idx][i];
            num_handover_blocked_sir_ps[traffic_type_idx][i]    = a->num_handover_blocked_sir_ps[traffic_type_idx][i]
                                                                + b->num_handover_blocked_sir_ps[traffic_type_idx][i];
        }

        num_initreq[traffic_type_idx]           = a->num_initreq[traffic_type_idx]
                                                + b->num_initreq[traffic_type_idx];
        num_rereq[traffic_type_idx]             = a->num_rereq[traffic_type_idx]
                                                + b->num_rereq[traffic_type_idx];
        a->num_drop[traffic_type_idx]           = a->num_drop[traffic_type_idx]
                                                + b->num_drop[traffic_type_idx];
        num_mult_handover_drop[traffic_type_idx] = a->num_mult_handover_drop[traffic_type_idx]
                                                + b->num_mult_handover_drop[traffic_type_idx];
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
void PHSStatCountClass::print_stat_count(FILE *fp, char *idt, double duration, int format)
{
    int attempt, num_attempt, tt_idx;
    char *chptr, *errmsg, *name;

    int print_data = ( ((format == 0) || (format == 2)) ? 1 : 0 );

    errmsg = CVECTOR(MAX_LINE_SIZE);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////// BEGINNING of code generated by gen_print_stat.pl
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    int *total_num_request                             = IVECTOR(num_traffic_type);
    int *total_num_request_block                       = IVECTOR(num_traffic_type);
    int *total_num_request_assign                      = IVECTOR(num_traffic_type);
    int *total_num_request_blocked_phys_chan           = IVECTOR(num_traffic_type);
    int *total_num_request_blocked_sir_cs              = IVECTOR(num_traffic_type);
    int *total_num_request_blocked_sir_ps              = IVECTOR(num_traffic_type);
    int *total_num_handover                            = IVECTOR(num_traffic_type);
    int *total_num_handover_block                      = IVECTOR(num_traffic_type);
    int *total_num_handover_assign                     = IVECTOR(num_traffic_type);
    int *total_num_handover_blocked_phys_chan          = IVECTOR(num_traffic_type);
    int *total_num_handover_blocked_sir_cs             = IVECTOR(num_traffic_type);
    int *total_num_handover_blocked_sir_ps             = IVECTOR(num_traffic_type);
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        total_num_request[tt_idx]                     = 0;
        total_num_request_block[tt_idx]               = 0;
        total_num_request_assign[tt_idx]              = 0;
        total_num_request_blocked_phys_chan[tt_idx]   = 0;
        total_num_request_blocked_sir_cs[tt_idx]      = 0;
        total_num_request_blocked_sir_ps[tt_idx]      = 0;
        total_num_handover[tt_idx]                    = 0;
        total_num_handover_block[tt_idx]              = 0;
        total_num_handover_assign[tt_idx]             = 0;
        total_num_handover_blocked_phys_chan[tt_idx]  = 0;
        total_num_handover_blocked_sir_cs[tt_idx]     = 0;
        total_num_handover_blocked_sir_ps[tt_idx]     = 0;
    }
    int phys_blocking_rate_num                        = 0;
    int phys_blocking_rate_den                        = 0;
    int cs_blocking_rate_num                          = 0;
    int cs_blocking_rate_den                          = 0;
    int rereq_rate_num                                = 0;
    int rereq_rate_den                                = 0;
    int request_rate_num                              = 0;
    for (tt_idx=0; tt_idx<=num_traffic_type-1; tt_idx++) {
        name = traffic_type_list[tt_idx]->name();
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[tt_idx])->get_num_attempt(CConst::RequestEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            chptr = errmsg;
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_%d                             = %d\n", idt, name, attempt, num_request[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_BLOCK_%d                       = %d\n", idt, name, attempt, num_request_block[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_BLOCK_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request_block[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_ASSIGN_%d                      = %d\n", idt, name, attempt, num_request_assign[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_ASSIGN_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request_assign[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_BLOCKED_PHYS_CHAN_%d           = %d\n", idt, name, attempt, num_request_blocked_phys_chan[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_BLOCKED_PHYS_CHAN_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request_blocked_phys_chan[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_BLOCKED_SIR_CS_%d              = %d\n", idt, name, attempt, num_request_blocked_sir_cs[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_BLOCKED_SIR_CS_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request_blocked_sir_cs[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_REQUEST_BLOCKED_SIR_PS_%d              = %d\n", idt, name, attempt, num_request_blocked_sir_ps[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_REQUEST_BLOCKED_SIR_PS_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_request_blocked_sir_ps[tt_idx][attempt]);
            }
            PRMSG(fp, errmsg);

            if (print_data) {
                total_num_request[tt_idx]                     += num_request[tt_idx][attempt];
                total_num_request_block[tt_idx]               += num_request_block[tt_idx][attempt];
                total_num_request_assign[tt_idx]              += num_request_assign[tt_idx][attempt];
                total_num_request_blocked_phys_chan[tt_idx]   += num_request_blocked_phys_chan[tt_idx][attempt];
                total_num_request_blocked_sir_cs[tt_idx]      += num_request_blocked_sir_cs[tt_idx][attempt];
                total_num_request_blocked_sir_ps[tt_idx]      += num_request_blocked_sir_ps[tt_idx][attempt];
                phys_blocking_rate_num                        += num_request_blocked_phys_chan[tt_idx][attempt];
                phys_blocking_rate_den                        += num_request[tt_idx][attempt];
                cs_blocking_rate_num                          += num_request_blocked_sir_cs[tt_idx][attempt];
                cs_blocking_rate_den                          += num_request[tt_idx][attempt];
                rereq_rate_den                                += num_request[tt_idx][attempt];
                request_rate_num                              += num_request[tt_idx][attempt];
            }
        }

        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[tt_idx])->get_num_attempt(CConst::HandoverEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            chptr = errmsg;
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_%d                            = %d\n", idt, name, attempt, num_handover[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_BLOCK_%d                      = %d\n", idt, name, attempt, num_handover_block[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_BLOCK_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover_block[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_ASSIGN_%d                     = %d\n", idt, name, attempt, num_handover_assign[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_ASSIGN_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover_assign[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_BLOCKED_PHYS_CHAN_%d          = %d\n", idt, name, attempt, num_handover_blocked_phys_chan[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_BLOCKED_PHYS_CHAN_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover_blocked_phys_chan[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_BLOCKED_SIR_CS_%d             = %d\n", idt, name, attempt, num_handover_blocked_sir_cs[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_BLOCKED_SIR_CS_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover_blocked_sir_cs[tt_idx][attempt]);
            }
            if (format == 0) {
                chptr += sprintf(chptr, "%s%s_NUM_HANDOVER_BLOCKED_SIR_PS_%d             = %d\n", idt, name, attempt, num_handover_blocked_sir_ps[tt_idx][attempt]);
            } else if (format == 1) {
                chptr += sprintf(chptr, "%s_NUM_HANDOVER_BLOCKED_SIR_PS_%d\t", name, attempt);
            } else {
                chptr += sprintf(chptr, "%d\t", num_handover_blocked_sir_ps[tt_idx][attempt]);
            }
            PRMSG(fp, errmsg);

            if (print_data) {
                total_num_handover[tt_idx]                    += num_handover[tt_idx][attempt];
                total_num_handover_block[tt_idx]              += num_handover_block[tt_idx][attempt];
                total_num_handover_assign[tt_idx]             += num_handover_assign[tt_idx][attempt];
                total_num_handover_blocked_phys_chan[tt_idx]  += num_handover_blocked_phys_chan[tt_idx][attempt];
                total_num_handover_blocked_sir_cs[tt_idx]     += num_handover_blocked_sir_cs[tt_idx][attempt];
                total_num_handover_blocked_sir_ps[tt_idx]     += num_handover_blocked_sir_ps[tt_idx][attempt];
                phys_blocking_rate_num                        += num_handover_blocked_phys_chan[tt_idx][attempt];
                phys_blocking_rate_den                        += num_handover[tt_idx][attempt];
                cs_blocking_rate_num                          += num_handover_blocked_sir_cs[tt_idx][attempt];
                cs_blocking_rate_den                          += num_handover[tt_idx][attempt];
                rereq_rate_den                                += num_handover[tt_idx][attempt];
                request_rate_num                              += num_handover[tt_idx][attempt];
            }
        }

        if (print_data) {
            rereq_rate_num                                += num_rereq[tt_idx];
        }

        chptr = errmsg;
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST                          = %d\n", idt, name, total_num_request[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST_BLOCK                    = %d\n", idt, name, total_num_request_block[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST_BLOCK\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request_block[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST_ASSIGN                   = %d\n", idt, name, total_num_request_assign[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST_ASSIGN\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request_assign[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST_BLOCKED_PHYS_CHAN        = %d\n", idt, name, total_num_request_blocked_phys_chan[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST_BLOCKED_PHYS_CHAN\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request_blocked_phys_chan[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST_BLOCKED_SIR_CS           = %d\n", idt, name, total_num_request_blocked_sir_cs[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST_BLOCKED_SIR_CS\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request_blocked_sir_cs[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_REQUEST_BLOCKED_SIR_PS           = %d\n", idt, name, total_num_request_blocked_sir_ps[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_REQUEST_BLOCKED_SIR_PS\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_request_blocked_sir_ps[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER                         = %d\n", idt, name, total_num_handover[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER_BLOCK                   = %d\n", idt, name, total_num_handover_block[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER_BLOCK\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover_block[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER_ASSIGN                  = %d\n", idt, name, total_num_handover_assign[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER_ASSIGN\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover_assign[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER_BLOCKED_PHYS_CHAN       = %d\n", idt, name, total_num_handover_blocked_phys_chan[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER_BLOCKED_PHYS_CHAN\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover_blocked_phys_chan[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER_BLOCKED_SIR_CS          = %d\n", idt, name, total_num_handover_blocked_sir_cs[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER_BLOCKED_SIR_CS\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover_blocked_sir_cs[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_TOTAL_NUM_HANDOVER_BLOCKED_SIR_PS          = %d\n", idt, name, total_num_handover_blocked_sir_ps[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_TOTAL_NUM_HANDOVER_BLOCKED_SIR_PS\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", total_num_handover_blocked_sir_ps[tt_idx]);
        }

        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_NUM_INITREQ                                = %d\n", idt, name, num_initreq[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_NUM_INITREQ\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", num_initreq[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_NUM_REREQ                                  = %d\n", idt, name, num_rereq[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_NUM_REREQ\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", num_rereq[tt_idx]);
        }
        if (format == 0) {
            chptr += sprintf(chptr, "%s%s_NUM_DROP                                   = %d\n", idt, name, num_drop[tt_idx]);
        } else if (format == 1) {
            chptr += sprintf(chptr, "%s_NUM_DROP\t", name);
        } else {
            chptr += sprintf(chptr, "%d\t", num_drop[tt_idx]);
        }
        PRMSG(fp, errmsg);
    }

    chptr = errmsg;
    if (format == 0) {
        if (phys_blocking_rate_den) {
            chptr += sprintf(chptr, "%sPHYS_BLOCKING_RATE                            = %12.10f\n", idt, (double) phys_blocking_rate_num / phys_blocking_rate_den);
        } else {
            chptr += sprintf(chptr, "%sPHYS_BLOCKING_RATE                            = UNDEFINED\n", idt);
        }
    } else if (format == 1) {
        chptr += sprintf(chptr, "PHYS_BLOCKING_RATE\t");
    } else {
        if (phys_blocking_rate_den) {
            chptr += sprintf(chptr, "%12.10f\t", (double) phys_blocking_rate_num / phys_blocking_rate_den);
        } else {
            chptr += sprintf(chptr, "UNDEFINED\t");
        }
    }
    if (format == 0) {
        if (cs_blocking_rate_den) {
            chptr += sprintf(chptr, "%sCS_BLOCKING_RATE                              = %12.10f\n", idt, (double) cs_blocking_rate_num / cs_blocking_rate_den);
        } else {
            chptr += sprintf(chptr, "%sCS_BLOCKING_RATE                              = UNDEFINED\n", idt);
        }
    } else if (format == 1) {
        chptr += sprintf(chptr, "CS_BLOCKING_RATE\t");
    } else {
        if (cs_blocking_rate_den) {
            chptr += sprintf(chptr, "%12.10f\t", (double) cs_blocking_rate_num / cs_blocking_rate_den);
        } else {
            chptr += sprintf(chptr, "UNDEFINED\t");
        }
    }
    if (format == 0) {
        if (rereq_rate_den) {
            chptr += sprintf(chptr, "%sREREQ_RATE                                    = %12.10f\n", idt, (double) rereq_rate_num / rereq_rate_den);
        } else {
            chptr += sprintf(chptr, "%sREREQ_RATE                                    = UNDEFINED\n", idt);
        }
    } else if (format == 1) {
        chptr += sprintf(chptr, "REREQ_RATE\t");
    } else {
        if (rereq_rate_den) {
            chptr += sprintf(chptr, "%12.10f\t", (double) rereq_rate_num / rereq_rate_den);
        } else {
            chptr += sprintf(chptr, "UNDEFINED\t");
        }
    }
    if (format == 0) {
        if (duration > 0.0) {
            chptr += sprintf(chptr, "%sREQUEST_RATE                                  = %12.10f\n", idt, (double) request_rate_num / duration);
        } else {
            chptr += sprintf(chptr, "%sREQUEST_RATE                                  = UNDEFINED\n", idt);
        }
    } else if (format == 1) {
        chptr += sprintf(chptr, "REQUEST_RATE\t");
    } else {
        if (duration > 0.0) {
            chptr += sprintf(chptr, "%12.10f\t", (double) request_rate_num / duration);
        } else {
            chptr += sprintf(chptr, "UNDEFINED\t");
        }
    }
    if ((format == 1)||(format == 2)) {
        chptr += sprintf(chptr, "\n");
    }
    PRMSG(fp, errmsg);

    free(total_num_request);
    free(total_num_request_block);
    free(total_num_request_assign);
    free(total_num_request_blocked_phys_chan);
    free(total_num_request_blocked_sir_cs);
    free(total_num_request_blocked_sir_ps);
    free(total_num_handover);
    free(total_num_handover_block);
    free(total_num_handover_assign);
    free(total_num_handover_blocked_phys_chan);
    free(total_num_handover_blocked_sir_cs);
    free(total_num_handover_blocked_sir_ps);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////// END of code generated by gen_print_stat.pl
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    free(errmsg);

    return;
}
/******************************************************************************************/

