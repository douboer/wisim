/******************************************************************************************/
/**** FILE: phs.h                                                                      ****/
/**** Michael Mandell 1/28/04                                                          ****/
/******************************************************************************************/

#ifndef PHS_H
#define PHS_H

#include "WiSim.h"
#include "traffic_type.h"

class PHSNetworkClass;
class STParamClass;
/******************************************************************************************/
/**** CLASS: PHSSectorClass                                                            ****/
/******************************************************************************************/
class PHSSectorClass : public SectorClass
{
public:
    PHSSectorClass(CellClass *cell);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    ~PHSSectorClass();
    SectorClass *duplicate(int copy_csid);
    void set_default_parameters();
    int get_ival(int param_type);
    void copy_sector_values(SectorClass *new_sector);
    double st_comp_arrival_rate(int traffic_idx);
    static int getSTParamIdx(int format, int stIdx);
    static int getMaxSTIdx(int format);
    /**************************************************************************************/

#if HAS_MONTE_CARLO
    void assign_channel_generic(PHSNetworkClass *np, EventClass *, int ps_best_channel, int *channel_list, int algorithm);
    void assign_channel_melco(PHSNetworkClass *np, EventClass *event, int *channel_list);
#endif

    void set_unused_freq(ListClass<int> *freq_list);
    int extract_csid_data(int field, int csid_format);
    int checkSTData(char *msg);

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    int gw_csc_cs;
    unsigned char *csid_hex;
    static unsigned int csid_byte_length;
    int cntl_chan_slot;
    int num_unused_freq, *unused_freq;
    int has_access_control;
    int mlc_priority_grp;
    int sync_level; /* 0: Master, 1: Slave 1, 2: Slave 2, ... */
    int *st_data;

    static int st_data_period;
    static ListClass<STParamClass *> *st_param_list;
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    int num_physical_tx;     /* number of physical transmitters in this sector        */
    int cntl_chan_eff_tch_slot; /* TCH slot that cntl_chan_slot interferes with, ie cntl_chan_slot % num_slot */
    int active;
    /**********************************************************************************/
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: PHSCellClass                                                              ****/
/******************************************************************************************/
class PHSCellClass : public CellClass
{
public:
    PHSCellClass(int num_sector = 0);
    ~PHSCellClass();
    CellClass *duplicate(const int x, const int y, int copy_csid);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    char *view_name(int cell_idx, int cell_name_pref);
    /**************************************************************************************/
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: PHSNetworkClass                                                           ****/
/******************************************************************************************/
class PHSNetworkClass : public NetworkClass
{
public:
    PHSNetworkClass();
    ~PHSNetworkClass();

    void read_cch_rssi_table(char *filename);
    void write_cch_rssi_table(char *filename);
    void reset_melco_params(int option);
    int get_ps_best_channel(EventClass *event);
    void color_cells_by_pa();

    void run_system_sync();
    void print_sync_state(char *filename);
    void comp_sync_order(int *sector_list, int num_sector);
    void expand_cch_rssi_table(double threshold_db);
    void group_by_csid_field(int field);

    void select_sectors_polygon(ListClass<IntIntClass> *ii_list, double min_ext_dist, double max_ext_dist, char *filename,  char *extended_filename, char *extended_bdy_filename);
    void select_sectors_polygon(ListClass<IntIntClass> *ii_list, double min_ext_dist, double max_ext_dist, char *small_poly, char *ext_poly);

#if HAS_MONTE_CARLO
    void adjust_offered_traffic(int num_init_events, int num_run_events, char *filename);
#endif

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    const int technology();
    void read_geometry_db(char *WiSim_home);
    void print_geometry(char *filename);
    void print_geometry_db();
    void process_mode_change();

    void set_default_parameters();
    void display_settings(FILE *fp);

    void uid_to_sector(char *uid, int &cell_idx, int &sector_idx);
    void uid_to_sector(int   uid, int &cell_idx, int &sector_idx);
    int sector_to_uid(char *uid, int cell_idx, int sector_idx);
    int Sum_st(int st_idx);
    char * pa_to_str(int pa);

#if HAS_MONTE_CARLO
    void gen_event(EventClass *event);
    void update_network(EventClass *);
    void reset_base_stations(int option);
    StatCountClass * create_call_stat_count();
    void print_call_statistics(char *filename, char *sector_list_str, int format);
#endif
    /**************************************************************************************/

    int num_slot;            /* total number of time slots  avail in the network          */
    int cntl_chan_freq;      /* CCH frequency                                             */
    int num_cntl_chan_slot;  /* total number of CCH time slots (multiple of num_slot)     */
    int bit_slot;

    int ac_hide_thr, ac_use_thr;
    double ac_hide_timer, ac_use_timer;

    static int num_csid_format;     /* Number of CSID formats                         */
    static int csid_format;         /* CSID FORMAT                                    */

    int cs_dca_alg, ps_dca_alg;

    /**************************************************************************************/
    /**** dca_alg = SIR/INT_SIR                                                        ****/
    /**************************************************************************************/
    double sir_threshold_call_request_cs_db, sir_threshold_call_request_cs;
    double sir_threshold_call_request_ps_db, sir_threshold_call_request_ps;
    double sir_threshold_call_drop_cs_db,    sir_threshold_call_drop_cs;
    double sir_threshold_call_drop_ps_db,    sir_threshold_call_drop_ps;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** dca_alg = INT/INT_SIR                                                        ****/
    /**************************************************************************************/
    double int_threshold_call_request_cs_db, int_threshold_call_request_cs;
    double int_threshold_call_request_ps_db, int_threshold_call_request_ps;
    double int_threshold_call_drop_cs_db,    int_threshold_call_drop_cs;
    double int_threshold_call_drop_ps_db,    int_threshold_call_drop_ps;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** dca_alg = MELCO                                                              ****/
    /**************************************************************************************/
    int mlc_ng, *mlc_grpsz, **mlc_cmt, *mlc_priority_grp;
    /**************************************************************************************/

    void read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);
    void read_geometry_1_1(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);
    void read_geometry_1_2(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);
    void read_geometry_1_3(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);
    void read_geometry_1_4(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);
    void read_geometry_1_5(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);

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
/**** CLASS: PHSTrafficTypeClass                                                       ****/
/******************************************************************************************/
class PHSTrafficTypeClass : public TrafficTypeClass
{
public:
    PHSTrafficTypeClass(char *s = (char *) NULL);
    ~PHSTrafficTypeClass();
    int get_num_attempt(int type);
    int check(NetworkClass *np);
    friend class NetworkClass;
    friend class PHSNetworkClass;

private:
    int num_attempt_request;
    int num_attempt_handover;  // TCH-switching
    int ps_meas_best_channel;
};
/******************************************************************************************/

#endif
