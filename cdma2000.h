/******************************************************************************************/
/**** FILE: wcdma.h                                                                    ****/
/**** Michael Mandell 1/28/04                                                          ****/
/******************************************************************************************/

#ifndef CDMA2000_H
#define CDMA2000_H

#include "wisim.h"
#include "traffic_type.h"

class CDMA2000NetworkClass;
/******************************************************************************************/
/**** CLASS: CDMA2000SectorClass                                                       ****/
/******************************************************************************************/
class CDMA2000SectorClass : public SectorClass
{
public:
    CDMA2000SectorClass(CellClass *cell);
    ~CDMA2000SectorClass();
    // void assign_channel_generic(PHSNetworkClass *np, EventClass *, int *channel_list, int algorithm);
    // void assign_channel_melco(PHSNetworkClass *np, EventClass *event, int *channel_list);
    // void set_unused_freq(int n, int *freq_list);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    SectorClass *duplicate(int copy_csid);
    void set_default_parameters();
    /**************************************************************************************/

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    // int cs_number;
    // unsigned char *csid_hex;
    // static unsigned int csid_byte_length;
    // int cntl_chan_slot;
    // int num_unused_freq, *unused_freq;
    // int has_access_control;
    // int mlc_priority_grp;
    // int sync_level; /* 0: Master, 1: Slave 1, 2: Slave 2, ... */
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    // int num_physical_tx;     /* number of physical transmitters in this sector        */
    // int cntl_chan_eff_tch_slot; /* TCH slot that cntl_chan_slot interferes with, ie cntl_chan_slot % num_slot */
    // int active;
    /**********************************************************************************/
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CDMA2000CellClass                                                         ****/
/******************************************************************************************/
class CDMA2000CellClass : public CellClass
{
public:
    CDMA2000CellClass(int num_sector = 0);
    ~CDMA2000CellClass();
    CellClass *duplicate(const int x, const int y, int copy_csid);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    char *view_name(int cell_idx, int cell_name_pref);
    /**************************************************************************************/
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CDMA2000NetworkClass                                                      ****/
/******************************************************************************************/
class CDMA2000NetworkClass : public NetworkClass
{
public:
    CDMA2000NetworkClass();
    ~CDMA2000NetworkClass();

    void read_cch_rssi_table(char *filename);
    void write_cch_rssi_table(char *filename);
    void reset_melco_params(int option);
    void run_system_sync();

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    const int technology();
    // void update_network(EventClass *);
    // void reset_base_stations(int option);
    // void read_geometry_db(char *wisim_home);
    void print_geometry(char *filename);
    // void print_geometry_db();
    void process_mode_change();

    void set_default_parameters();
    void display_settings(FILE *fp);
    /**************************************************************************************/

    // int num_freq;            /* total number of frequencies avail in the network          */
    // int num_slot;            /* total number of time slots  avail in the network          */
    // int cntl_chan_freq;      /* CCH frequency                                             */
    // int num_cntl_chan_slot;  /* total number of CCH time slots (multiple of num_slot)     */
    // int bit_slot;

    // int ac_hide_thr, ac_use_thr;
    // double ac_hide_timer, ac_use_timer;

    // int cs_dca_alg, ps_dca_alg;

    /**************************************************************************************/
    /**** dca_alg = SIR/INT_SIR                                                        ****/
    /**************************************************************************************/
    // double sir_threshold_call_request_cs_db, sir_threshold_call_request_cs;
    // double sir_threshold_call_request_ps_db, sir_threshold_call_request_ps;
    // double sir_threshold_call_drop_cs_db,    sir_threshold_call_drop_cs;
    // double sir_threshold_call_drop_ps_db,    sir_threshold_call_drop_ps;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** dca_alg = INT/INT_SIR                                                        ****/
    /**************************************************************************************/
    // double int_threshold_call_request_cs_db, int_threshold_call_request_cs;
    // double int_threshold_call_request_ps_db, int_threshold_call_request_ps;
    // double int_threshold_call_drop_cs_db,    int_threshold_call_drop_cs;
    // double int_threshold_call_drop_ps_db,    int_threshold_call_drop_ps;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** dca_alg = MELCO                                                              ****/
    /**************************************************************************************/
    // int mlc_ng, *mlc_grpsz, **mlc_cmt, *mlc_priority_grp;
    /**************************************************************************************/

    void read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);

protected:
    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    int check_parameters();
    /**************************************************************************************/

private:
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CDMA2000TrafficTypeClass                                                  ****/
/******************************************************************************************/
class CDMA2000TrafficTypeClass : public TrafficTypeClass
{
public:
    CDMA2000TrafficTypeClass(char *s = (char *) NULL);
    ~CDMA2000TrafficTypeClass();
};
/******************************************************************************************/

#endif
