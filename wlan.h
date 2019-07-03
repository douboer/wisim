/******************************************************************************************/
/**** FILE: wlan.h                                                                     ****/
/**** Michael Mandell 1/28/04                                                          ****/
/******************************************************************************************/

#ifndef WLAN_H
#define WLAN_H

#include  <math.h>
#include  <vector>

#include "WiSim.h"
#include "traffic_type.h"
#include "prop_model.h"

// These parameters may be defined in constructure function as default.
#define SIFS            10               // SIFS length in bits
#define DIFS            50               // SIFS length in bits
#define LS              20               // SLOT length in bits
#define IPH             160              // IP header   in bits
#define PAYLOAD         8000             // payload length in bits
#define PH              192              // PHY header length in bits
#define MH              208              // MAC header length in bits
#define L_RTS           160+PH           // RTS length in bits
#define L_CTS           112+PH           // CTS length in bits
#define L_ACK           112+PH           // ACK length in bits
#define P_DELAY         1                // Propagation delay in us
#define W               32               // The initial contention size

#define MC_DBG          0

class WLANNetworkClass;
class WLANTrafficTypeClass;
class WLANSessionClass;
class WLANExpoPM;

enum ServiceType {
    VoiceService,
    DataService
};

/******************************************************************************************/
/**** CLASS: WLANSectorClass                                                           ****/
/******************************************************************************************/
class WLANSectorClass : public SectorClass
{
public:
    WLANSectorClass(CellClass *cell);
    ~WLANSectorClass();
    // void assign_channel_generic(PHSNetworkClass *np, EventClass *, int *channel_list, int algorithm);
    // void assign_channel_melco(PHSNetworkClass *np, EventClass *event, int *channel_list);
    // void set_unused_freq(int n, int *freq_list);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    SectorClass *duplicate(int copy_csid);
    void set_default_parameters();
    void copy_sector_values(SectorClass *new_sector);
    /**************************************************************************************/

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    static double bit_rate;

    int chan_idx;

};

/******************************************************************************************/
/**** CLASS: WLANCellClass                                                             ****/
/******************************************************************************************/
class WLANCellClass : public CellClass
{
public:
    WLANCellClass(int num_sector = 0);
    ~WLANCellClass();
    CellClass *duplicate(const int x, const int y, int copy_csid);

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    char *view_name(int cell_idx, int cell_name_pref);
    /**************************************************************************************/
};

/******************************************************************************************/
/**** CLASS: WLANNetworkClass                                                          ****/
/******************************************************************************************/
class WLANNetworkClass : public NetworkClass
{
public:
    WLANNetworkClass();
    ~WLANNetworkClass();

    friend class NetworkClass;

    void read_cch_rssi_table(char *filename);
    void write_cch_rssi_table(char *filename);
    void reset_melco_params(int option);
    void run_system_sync();

    double comp_sir_call( int cell_idx, int call_px, int call_py ); 
    double comp_int_call( int cell_idx, int call_px, int call_py ); 

    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    const int technology();
    // void read_geometry_db(char *WiSim_home);
    void print_geometry(char *filename);
    // void print_geometry_db();
    void process_mode_change();

    void set_default_parameters();
    void display_settings(FILE *fp);

    void   comp_sir_aps( double* sir_list, double radius, double r_threshold );
    double sir_ap( WLANSectorClass* m_ap, double radius, double r_threshold );

    inline WLANExpoPM* get_pm() { return wpm; }
    
#if HAS_MONTE_CARLO
    void gen_event(EventClass *event);
    void update_network(EventClass *);
    void reset_base_stations(int option);
    StatCountClass * create_call_stat_count();
    void print_call_statistics(char *filename, char *sector_list_str, int format);
#endif

    static ServiceType service_type;
    int    limit_retry;                     // The limit times of retry
    int    max_backoff_stage;               // maximum backoff stage
    int    use_rts_cts;

    //double ac_num_user;         // Access control number of users
    /**************************************************************************************/
    void read_geometry_1_0(FILE *fp, char *antenna_filepath, char *line, char *filename, int linenum);

protected:
    /**************************************************************************************/
    /**** Virtual Functions                                                            ****/
    /**************************************************************************************/
    int check_parameters();
    /**************************************************************************************/

    double   lambda_segment_user         ( WLANSessionClass* call );
    double   lambda_segment_net          ();

    // first algorithm of lambda drop (mean duration to next drop)
    int      prop_pkt_loss               ( WLANSectorClass* ap, double* ppkt_loss, double err = 1E-10 );
    double*  prop_drop                   ( WLANSectorClass* ap, double* p_drop_list );
    double   drop_mean_duration_ap       ( WLANSectorClass* ap, double* ap_lambda_traffic_type_total, int options = 0 );

    // second algorithm of drop call
    double   drop_mean_duration_user     ( WLANSessionClass* call );

    double   tau_equations_no_inter      ( int num_user, double& p_cond_coll, double& tau );
    double   tau_iteration_no_inter      ( int num_user, double& p_cond_coll, double& tau, double err = 1E-10 );
    double   tau_with_inter              ( double err = 1E-10 );
    void     get_inter_matrix            ( int** mx_a, double* tau_user_no_inter );

    double   bino_p                      ( int m, int n, double p );
    int      find_krange                 ( int k );
    double   prop_tran_ahead             ( double p_cond_coll, int k );
    double   frozen_stage                ( int num_user, double p_cond_coll, int b );
    double   l_backoff                   ( int b );
    double   l_frozen                    ( int num_user,double p_cond_coll,int b );

    // Compute the access delay for a traffic type user under an AP.
    // ad_seg_type_f is access delay when transmitting a segment, this value devites from the backoff length with Window/2;
    //               so, the value is fixed, it is not a fact condition, but it can be used to do access control
    //               ( ad_seg_type_f[i] <==> number of some type of users under an AP )
    // ad_seg_type_r is access delay when transmitting a segment, this value choosed from the backoff window with random style;
    //               so, it can simulate the fact condition of wlan.
    double comp_mean_delay_a            ( WLANSectorClass* ap, double& ad_seg_type_r );
    double throughput_computation       ( WLANSectorClass* ap, double& normal_coll, double err );
    double comp_mean_delay_b            ( WLANSectorClass* ap );
    double comp_jitter                  ( WLANSectorClass* ap );
    double comp_mean_delay_a            ( int num_user, double& ad_seg_type_r, double  p_cond_coll );
    double comp_jitter                  ( int num_user, double  p_cond_coll, double tau );

    double phy_mac_convert();

private:
    static int current_event_idx;

    double phy_mac_conv;

    // The mechanism of an call block ( LOW RSSI, HIGH INT, LOW SIR );
    int mechanism_block;

    /* May be need to move to WLANTrafficTypeClass, so it's a list value in an AP which hold
     * more than one type of traffic;
     **************************************************************************************/
    // double loss_pkt;              // Call drop if loss_pkt outof num_pkt packet loss.
    // double num_pkt;               // Call drop if loss_pkt outof num_pkt packet loss.
    // Threshold of packet loss ratio that lead to call drop ( = loss_pkt/num_pkt ).
    // double thr_pkt_loss_ratio;
    // double thr_pkt_loss_ratio

    /* Threshold of call block
     * Initial value ?
     **************************************************************************************/
    double rssi_threshold_call_block_db;
    double sir_threshold_call_block_db;
    double int_threshold_call_block_db;

    int connect_call_request(EventClass *event, int cell_idx, int sector_idx);

    // int num_traffic_class;            // Number of traffic type class.
    double vs_rto;                       // Ratio of Voice Service in WLAN Network

    // Access control parameters.
    int    ac_num_user;                  // Access control number of users
    double ac_activate;                  // Access control activate threshold
    double ac_deactivate;                // Access control deactivate threshold

    WLANExpoPM* wpm;

    /* get all tau values when number of users less than number of access control users
     * 1) tau under no interference is only correlated to number of users under an AP
     * 2) we can quickly find tau value under no interference condition by users number
     *    and speed up simulating process
     * 3) length of vector = ac_num_user
     */
    std::vector<double> tau_nu_no_inter;

    /* devite tau under interference with tau_nu_no_inter and interference matrix(by get_inter_matrix())
     * length of list = number of users in network
     */
    // double* tau_user_inter;
    // double* p_cond_coll_user_inter;
    std::vector<double> tau_user_inter; // tau of users with interference
    std::vector<double> p_cond_coll_user_inter; // condition collision probability of users with interference
};


/******************************************************************************************/
/**** CLASS: WLANTrafficTypeClass                                                      ****/
/******************************************************************************************/
class WLANTrafficTypeClass : public TrafficTypeClass
{
public:
    WLANTrafficTypeClass(char *s = (char *) NULL);
    ~WLANTrafficTypeClass();

    int type;

    int duration_dist;
    int bit_per_pkt;

    double user_data_rate;               // Rate of voice codec in Kbps 
    // double ap_data_rate;              // Procecss rate of AP( MAC layer) in Kbps 
    double basic_rate ;                  // Basic rate
    double phy_rate   ;                  // Physical rate
    double hd_rate    ;                  // Header rate

    double mean_session_duration;        // Mean time duration of a session
    double mean_drop_duration;           // Mean time duration to drop

    /* second type of algorithm for drop call duration
     * packet_loss_threshold: packet loss rate benchmark for drop call(default value 5%).
     * duration_dropcall_threshold: average drop call duration when the packet loss rate
     *       equals to packet_loss_threshold
     *       (default value duration_dropcall_threshold = duration_call);
     * max_consecut_drop_pkt: the maximum number for consecutive invalid packet before drop call
     */
    double packet_loss_thr;
    double duration_dropcall_thr;
    int max_consecut_drop_pkt;

    // double bit_rate;
    int drop_total_pkt;                  // Call drop if drop_error_pkt outof drop_total_pkt packet loss 
    int drop_error_pkt;                  // Call drop if drop_error_pkt outof drop_total_pkt packet loss 

    // variables for data service
    // Maybe we should seperate it to two classes, VoiceTrafficType and DataTrafficType 
    double session_req_rate;             // Rate of session request
    int    mean_num_seg_per_session;     // Mean number of data segments per session
    double mean_session;                 // Mean duration of a session
    double mean_segment;                 // Mean duration of an segment of sessions
    double mean_read_time;               // Mean duration of reading time
    double mean_seg_read_total;          // Mean duration of segment include reading time
    int    min_seg_size;                 // Minimum data segment size
    int    max_seg_size;                 // Maximum data segment size
    double down_up_ratio;                // downlink/uplink ratio
    double dur_compress;                 // the voice duration for each compressing
};

/******************************************************************************************/
/**** CLASS: WLANSessionClass                                                          ****/
/******************************************************************************************/
class WLANSessionClass : public CallClass
{
public:
    WLANSessionClass();
    ~WLANSessionClass();
    double receive_pwr(WLANNetworkClass *np, int x, int y);

    /* get the value of clear channel assesment (CCA)
     * if interference signal receieved from other terminal(can be calculated by function receive_pwr()
     * in fact, maybe more than one * interference terminal, here we only count one of them) is larger than
     * the cca value the terminal will be disactive.
     */
    static double get_cca();

    /* Counter of segment left in a session.
     * Initial value of this variable equals to the segment number of a session.
     * If the value decretes to zero the correspanding session HandUp(session finished).
     */
    int  seg_cnt_left;
    
    /* The flag of first segment of a session.
     * If first_seg_flag is true, starting the correspanding session ( i.e. Session Request).
     * Generate the num_segment, and let seg_cnt_left = num_segment - 1;
     *
     * Substituted it with Session Request random process
     * ( SessionRequestEvent <==> RequestEvent ).
     */
    // bool first_seg_flag;

    // Number of segment included in the session.
    int  num_segment;

    static double tx_pwr_mw;
    double tx_body_gain;  // Include cable loss and connector loss
    double tx_ant_gain;
    double rx_body_gain;
    double rx_ant_gain;
};


class WLANExpoPM
{
public:
    WLANExpoPM();
    double prop_power_loss(NetworkClass *np, WLANSessionClass* user, int delta_x, int delta_y);

private:
    // pl = -10*a*log10(d) - c
    double a;
    double c;
};

/******************************************************************************************/
#endif
