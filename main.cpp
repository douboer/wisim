#include <string.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <QString>
#include <QSettings>
#include <QEventLoop>
#include <QDebug>

//Added by qt3to4:
#include <QTranslator>
#include <QPixmap>
#include <QDesktopWidget>

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <asm/processor.h>
#include <netinet/in.h>
#else
#include <io.h>
#include <fcntl.h>

//#include <stdafx.h>
//#include <atlbase.h>
//#include <IPHlpApi.h>
#include <winsock.h>
//#include <direct.h>
//#include <process.h>
//#include <windows.h>
#endif

#include "set_language.h"
#include "antenna.h"
#include "binint.h"
#include "bin_io.h"
#include "cdma2000.h"
#include "WiSim.h"
#include "clutter_data_analysis.h"
#include "cconst.h"
#include "coverage.h"
#include "hot_color.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "map_height.h"
#include "map_layer.h"
#include "phs.h"
#include "polygon.h"
#include "pref.h"
#include "prop_model.h"
#include "randomc.h"
#include "road_test_data.h"
#include "statistics.h"
#include "utm_conversion.h"
#include "wcdma.h"
#include "wlan.h"

#if HAS_GUI
#include <qsplitter.h>
#include <qeventloop.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <q3filedialog.h>
#include <q3header.h>
#include <qcursor.h>

#include "WiSim_gui.h"
#include "gcall.h"
#include "logo_display.h"
#include "main_window.h"
#include "map_background.h"
#include "pixmap_item.h"
#include "progress_slot.h"
#include "run_simulation_dia.h"
#include "visibility_window.h"
#include "reg_dia.h"

#ifndef __linux__
#include <windows.h>
//extern "C"
//{
//#include <basetyps.h>
//#include <windowsx.h>
//#include <initguid.h>
//}

#endif

#include <iostream>

/*
#include "resource.h"
extern "C"
{
#if HAS_DDK
#include "myusb.h"
#endif
}
*/

extern QFont *application_font;
extern QFont *fixed_width_font;

MainWindowClass *main_window;
int use_gui = 1;
int show_logo = 1;
int show_lic_update = 1;
int hide_main = 0;
#endif

void get_mac_info(unsigned char *reg_info);
void get_hd_srnum(unsigned char *reg_info);
int proc_lic_file(char *path, char *filename, unsigned char *reg_info, int ris, int force);
int get_lic_n(unsigned char *reg_info);
void set_lic_n(int lic_n, unsigned char *reg_info);
void gaussian(TRandomMersenne *rg, double &g1, double &g2);

#if HAS_GUI
int install_license_file(char *install_file, char *WiSim_home, unsigned char *reg_info, int ris);
#endif
void make_reg_data(unsigned char *reg_info, int ris, char *str);

#ifndef __linux__
// Show dos cmd window to redirect information
void InitConsoleWindow();

QString read_registration_table(char *path, char *varname);
void write_registration_table(char *path, char *varname, char *value);
#endif

int get_net_date(int &year, int &month, int &day);

char *WiSim_home = (char *) NULL;
char *usb_sn = (char *) NULL;
char *initfile = (char *) NULL;
char *geomfile = (char *) NULL;
char *logfile  = (char *) NULL;
int technology = CConst::PHS;
char *WiSim_pref_file = (char *) NULL;

#include "license.h"
#include "scramble.h"

#ifndef __linux__
//*****************************************************************************
// D E F I N E S
//*****************************************************************************

// window control defines
//
#define SIZEBAR             0
#define WINDOWSCALEFACTOR   15

#endif

int main( int argc, char ** argv )
{
    NetworkClass *np;
    int cont;
    int proper_installation = 1;

#ifndef __linux__
    // Show dos cmd window to redirect information
    InitConsoleWindow();
#endif

#if CDEBUG
    time_t td;
#endif

#if (DEMO == 0)
    int registered;
#endif

    const static char *opt_msg[] = {
    "***********************************************************************************************************************",
    "* Options:                                                                                                            *",
    "*     read_geometry -f filename [-force_fmt format]                 Read cell geometry file                           *",
#if HAS_ORACLE
    "*     read_geometry_db                                              Read cell geometry from database                  *",
#endif
    "*     display_geometry [-f filename]                                Display geometry data or write to a file          *",
#if HAS_ORACLE
    "*     display_geometry_db                                           Write cell geometry to database                   *",
#endif
    "*     define_geometry -coord_sys s -resolution r                    Define a new, empty, geometry with specified      *",
    "*                 -system_bdy 'x0 y0 ... xn-1 yn-1'                 coord system, resolution, and system bdy.         *",
    "*     print_bmp      [-f filename]                                  Print current screen to BMP file                  *",
    "*     print_jpg      [-f filename]                                  Print current screen to JPG file                  *",
    "*     close_geometry                                                Close current geometry                            *",
    "*     include -f filename                                           Execute commands in file                          *",
    "*     switch_mode -mode [edit_geom | simulate]                      Switch mode to edit_geometry or simulate          *",
    "*     redefine_system_bdy -threshold_db val                         Redefine system boundary based on signal level    *",
    "*           -scan_fractional_area f -init_sample_res n                                                                *",
    "*     export_clutter_model -pm_idx 0 -f filename                    Export spec clutter prop model to file            *",
    "*     import_clutter_model -f filename                              Import clutter prop model from the specified file *",
//  "*     report_sector -sectors 'c0_s0 c1_s1 c2_s2 ...'   xxxxxxxxxx                                                     *",
    "*     set_csid_format -fmt value                                    Set the CSID format (0 - 19NP, 1 - 16NP)          *",
    "*     display_csid_format                                           display the format of csid                        *",
    "*     import_st_data  -fmt [melco|sanyo] -fcsc cscfile -f filename  Import ST data.                                   *",
    "*     delete_st_data                                                Delete all ST data.                               *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     read_cch_rssi_table -f filename                               Read CCH RSSI table                               *",
    "*     read_road_test_data -f filename [-force_fmt format]           Read road test data file                          *",
    "*     shift_road_test_data -x xval -y yval                          Shift road test data                              *",
    "*     check_road_test_data                                          Check road test data                              *",
    "*     save_road_test_data -f filename                               Save road test data to a file                     *",
    "*     add_rtd_pt  -posn_x x -posn_y y [-sdev_db val]                Add a road test data point at the specified posn  *",
//Do not use in Chile    "*     filter_rtd -gptlist 'x0 y0 ... xn-1 yn-1'                     Delete all road test data outside spec polygon    *",
    "*     write_cch_rssi_table -f filename                              Write CCH RSSI table                              *",
    "*     comp_prop_model -scope [global | individual]                  Compute propagation model using road test data.   *",
    "*                     -sectors 'c0_s0 c1_s1 c2_s2 ...'                                                                *",
    "*                     -useheight [ 0 | 1 ] -adjust_angles [0 | 1]                                                     *",
    "*                     -min_dist val                                 Ignore rtd closer than min_dist.                  *",
    "*                     -ignore_fs_check [0 | 1]                      Whether or not to ignore free-space check         *",
    "*     comp_prop_error [-f filename]                                 Compute RMS error between prop model and road test*",
    "*     set_unassigned_prop_model [-no_rtd]                           Set prop_model for all unassigned sectors.        *",
    "*     convert_utm -utm_a a -utm_e e [-utm_zone n]                   Convert coordinate system to UTM.                 *",
    "*                 [-utm_north [ 0 | 1 ]] [-r resolution]                                                              *",
    "*     flip_lon                                                      Horizontally flip design replacing lon by -lon    *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     create_coverage_analysis -name name                           Create new coverage analysis                      *",
    "*         -type [layer | level | sir_layer | paging_area]                                                             *",
    "*     set_coverage_analysis -name name -cell 'c0 c1 ... cn-1'       Set list of cells and max dist for coverage       *",
    "*                                      -dmax val                    analysis                                          *",
    "*     set_coverage_analysis -name name -num_scan_type val           Set num scan types (colors) for coverage analysis *",
    "*     set_coverage_analysis -name name -threshold_db val            Set threshold for layer analysis                  *",
    "*     set_coverage_analysis -name name -thr_idx j -threshold_db val Set threshold j for level analysis                *",
    "*     set_coverage_analysis -name name -scan_fractional_area f      Set scan_fractional_area for coverage analysis    *",
    "*     set_coverage_analysis -name name -init_sample_res n           Set init_sample_res for coverage analysis         *",
    "*     set_coverage_analysis -name name -use_gpm [0 | 1]             Whether or not to use global prop models          *",
    "*     set_coverage_analysis -name name -strid new_name              Rename coverage analysis                          *",
    "*     run_coverage -name name                                       Run coverage simulation                           *",
    "*     write_coverage_analysis -name name -f filenname               Write coverage analysis results into file         *",
    "*     read_coverage_analysis -f filenname                           Read  coverage analysis results from file         *",
#if HAS_ORACLE
    "*     write_coverage_analysis_db -name name                         Write coverage analysis results to database       *",
    "*     read_coverage_analysis_db                                     Read coverage analysis results from database      *",
#endif
    "*     report_coverage_analysis -name name [-f filenname]            Report coverage analysis data                     *",
    "*     plot_coverage_analysis -name name [-name2 name2]              Plot coverage analysis data.  Compare two         *",
    "*                                                                   coverage analyses if name2 is specified.          *",
    "*     delete_coverage_analysis -name name                           Delete coverage analysis                          *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     read_map_layer -f filename -fmt [ generic | mif ]             Read map layer file                               *",
    "*                    [-name name] [-filter [0 | 1]]                                                                   *",
    "*     shift_map_layer -map_layer_idx i -x xval -y yval              Shift map_layer                                   *",
    "*     save_map_layer -map_layer_idx i [-f filename]                 Save map layer file                               *",
    "*     read_map_clutter -f filename [-force]                         Read map clutter file                             *",
    "*     save_map_clutter -f filename                                  Save map clutter file                             *",
    "*     read_map_height -f filename                                   Read map height file                              *",
    "*     read_map_background -f filename -fposn position_file          Read map background file                          *",
    "*     shift_map_background -x xval -y yval                          Shift map_background                              *",
    "*     close_map_background                                          Close map background file                         *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     refine_clutter_model -pm_idx i -n val                         Split resolution of spec clutter prop model       *",
    "*     shift_clutter_model -pm_idx i -x xval -y yval                 Shift spec clutter prop model                     *",
    "*     create_clutter_map -map_sim_res_ratio i [-num_clutter_type n] Create new clutter map                            *",
    "*     set_clutter_type -clutter_type_idx i                          Set clutter type for points in polygon            *",
    "*         -gptlist 'x0 y0 ... xn-1 yn-1'                                                                              *",
    "*     set_init_clutter_type                                         Assign initial clutter type to all points in grid *",
    "*     shift_clutter_map -x xval -y yval                             Shift clutter map                                 *",
    "*     set_clutter_coeff -pm_idx i -clutter_idx i -val val           Set the coeffecient for a clutter index           *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set -frequency value                                          Set the RF frequency (Hz)                         *",
#if HAS_MONTE_CARLO
    "*     set -num_freq value                                           Set the number of frequencies in the system       *",
    "*     set -num_slot value                                           Set the number of TCH time slots                  *",
    "*     set -num_cntl_chan_slot value                                 Set the number of CCH time slots                  *",
    "*     set -cntl_chan_freq value                                     Set the CCH frequency                             *",
//  "*     set -cs_dca_alg [sir | int | int_sir | sir_int | melco]       Use the specified DCA algorithm at CS             *",
    "*     set -cs_dca_alg [sir | int | int_sir | sir_int ]              Use the specified DCA algorithm at CS             *",
    "*     set -ps_dca_alg [sir | int | int_sir]                         Use the specified DCA algorithm at PS             *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set -sir_threshold_call_request_cs_db value                   Set the SIR threshold at a CS for a call request  *",
    "*                                                                   to be connected (DB) for SIR DCA alg.             *",
    "*     set -sir_threshold_call_request_ps_db value                   Set the SIR threshold at a PS for a call request  *",
    "*                                                                   to be connected (DB) for SIR DCA alg.             *",
    "*     set -sir_threshold_call_drop_cs_db value                      Set the SIR threshold at a CS for a call in       *",
    "*                                                                   progress to be dropped (DB) for SIR DCA alg.      *",
    "*     set -sir_threshold_call_drop_ps_db value                      Set the SIR threshold at a PS for a call in       *",
    "*                                                                   progress to be dropped (DB) for SIR DCA alg.      *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set -int_threshold_call_request_cs_db value                   Set the INT threshold at a CS for a call request  *",
    "*                                                                   to be connected (DB) for INT DCA alg.             *",
    "*     set -int_threshold_call_request_ps_db value                   Set the INT threshold at a PS for a call request  *",
    "*                                                                   to be connected (DB) for INT DCA alg.             *",
    "*     set -int_threshold_call_drop_cs_db value                      Set the INT threshold at a CS for a call in       *",
    "*                                                                   progress to be dropped (DB) for INT DCA alg.      *",
    "*     set -int_threshold_call_drop_ps_db value                      Set the INT threshold at a PS for a call in       *",
    "*                                                                   progress to be dropped (DB) for INT DCA alg.      *",
    "*---------------------------------------------------------------------------------------------------------------------*",
#if 0
    "*     set -cch_allocation_threshold_db value                        Set max RSSI level for CCH allocation             *",
    "*     set -sync_level_allocation_threshold_db value                 Set min RSSI level to allow sync level assignment *",
#endif
    "*     set -ac_hide_thr value                                        Set number of avail phys chan for which access    *",
    "*                                                                   control will hide a CS.                           *",
    "*     set -ac_use_thr value                                         Set number of avail phys chan for which access    *",
    "*                                                                   control will use a CS.                            *",
    "*     set -ac_hide_timer value                                      Set timer duration after which CS is hidden       *",
    "*     set -ac_use_timer value                                       Set timer duration after which CS is used         *",
    "*     set -memoryless_ps value                                      Whether or not PS attempts return to orig CS      *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     WLAN PARAMETERS:                                                                                                *",
    "*     set -limit_retry value                                        Limit the number of retries                       *",
    "*     set -max_backoff_stage value                                  Maximum backoff stage                             *",
    "*     set -use_rts_cts [0 | 1]                                      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                 *",
    "*     set -ac_num_user value                                        Set the access control number of users            *",
    "*     set -ac_activate value                                        Set the access control activate threshold         *",
#endif
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set_traffic -traffic_type t -duration_dist [expo | unif]      Set the prob dist of call duration                *",
    "*     set_traffic -traffic_type t -mean_time value                  Set the average call duration for expo dist (Sec) *",
    "*     set_traffic -traffic_type t -min_time value                   Set the minimun call duration for unif dist (Sec) *",
    "*     set_traffic -traffic_type t -max_time value                   Set the maximum call duration for unif dist (Sec) *",
    "*     set_traffic -traffic_type t -num_attempt_request  value       Set the num of attempts to assign a chan request  *",
    "*     set_traffic -traffic_type t -num_attempt_handover value       Set the num of attempts to assign a chan handover *",
    "*     set_traffic -traffic_type t -ps_meas_best_channel [0 | 1]     Set whether or not PS meas best channel           *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     WLAN PARAMETERS:                                                                                                *",
    "*     set_traffic -traffic_type t -bit_per_pkt value                Set the num of bits per packet                    *",
    "*     set_traffic -traffic_type t -user_data_rate value             Set the rate of voice codec in Kbps               *",
    "*     set_traffic -traffic_type t -basic_rate value                 Set the basic Rate                                *",
    "*     set_traffic -traffic_type t -phy_rate value                   Set the physical rate                             *",
    "*     set_traffic -traffic_type t -hd_rate value                    Set the header rate                               *",
    "*     set_traffic -traffic_type t -mean_session_duration value      Set the mean time duration to drop                *",
    "*     set_traffic -traffic_type t -drop_total_pkt value             xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                 *",
    "*     set_traffic -traffic_type t -drop_error_pkt value             xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                 *",
    "*     set_traffic -traffic_type t -mean_segment value               Set the mean duration of an segment of sessions   *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set_total_traffic -traffic_type t -arrival_rate value         Set the total arrival rate for all subnets of     *",
    "*                                                                   specified traffic type                            *",
    "*---------------------------------------------------------------------------------------------------------------------*",
//    "*     create_prop_model -name name -type [expo | terrain | segment  Create new propagation model                      *",
//    "*         clutter_simp | clutter_full | clutter_sym_full                                                              *",
//    "*         clutter_wt_expo]                                                                                            *",
    "*     create_prop_model -name name -type [expo | segment]           Create new propagation model                      *",
    "*     delete_prop_model [-pm_idx i | -name name] [force]            Delete propagation model                          *",
    "*     delete_all_unused_prop_model                                  Delete all prop models that are not being used    *",
    "*     delete_all_prop_model                                         Delete all prop models                            *",
    "*     set_prop_model [-pm_idx i | -name name]                       Set the propagation model type                    *",
    "*                    -type [expo | terrain | segment]                                                                 *",
    "*     set_prop_model [-pm_idx i | -name name] -exponent value       Set the propagation exponent for expo prop_model  *",
    "*     set_prop_model [-pm_idx i | -name name] -coefficient value    Set the propagation coeff    for expo prop_model  *",
    "*     set_prop_model [-pm_idx i | -name name] -val_y value          Set val_y  for terrain prop_model                 *",
    "*     set_prop_model [-pm_idx i | -name name] -val_py value         Set val_py for terrain prop_model                 *",
    "*     set_prop_model [-pm_idx i | -name name] -val_s1 value         Set val_s1 for terrain prop_model                 *",
    "*     set_prop_model [-pm_idx i | -name name] -val_s2 value         Set val_s2 for terrain prop_model                 *",
    "*     set_prop_model [-pm_idx i | -name name] -useheight [0 | 1]    Set useheight   for segment, clutter  prop_models *",
    "*     set_prop_model [-pm_idx i | -name name] -start_slope value    Set start_slope for segment prop_model            *",
    "*     set_prop_model [-pm_idx i | -name name] -final_slope value    Set final_slope for segment prop_model            *",
    "*     set_prop_model [-pm_idx i | -name name] -num_pts val          Set number of points for segment prop_model       *",
    "*     set_prop_model [-pm_idx i | -name name] -pt_idx k -x x -y y   Set point k for segment prop_model                *",
    "*     set_prop_model [-pm_idx i | -name name] -num_clutter_type val Set num of clutter types for segment prop_model   *",
    "*     set_prop_model [-pm_idx i | -name name] -c_idx j -c_coeff val Set clutter coeff for segment prop_model          *",
    "*     report_prop_model -name name [-f filenname]                   Report parameters of specified prop model         *",
    "*     report_prop_model_param -param [inflexion | slope | all]                                                        *",
    "*                             -model [all | used] [-f file]         Report sorted list for prop models                *",
    "*     report_clutter_model [-pm_idx i | -name name]                  Report clutter prop model params                  *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set_cell -cell 'c0 c1 ... cn-1' -num_sector m                 Set num sectors for specified cells               *",
    "*     set_bitmap -cell_idx 'c0 c1 ... cn-1' -bm_idx i               Set bitmap index to be used for specified cells   *",
    "*     [-sector c_s | sector_csid_hex h | sector_gs_csc_cs g]        SECTOR_SPEC used for set_sector command           *",
    "*     set_sector SECTOR_SPEC -csid_hex csid                         Set hexadecimal CSID for specified sector         *",
    "*     set_sector SECTOR_SPEC -antenna_type value                    Set antenna_type for specified sector             *",
    "*     set_sector SECTOR_SPEC -antenna_height value                  Set antenna_height sectors for specified sector   *",
    "*     set_sector SECTOR_SPEC -antenna_angle_deg value               Set antenna_angle_deg for specified sector        *",
    "*     set_sector SECTOR_SPEC -tx_pwr value                          Set tx_pwr for specified sector                   *",
    "*     set_sector SECTOR_SPEC -prop_model pm_idx                     Set propagation model for specified sector        *",
    "*     set_sector SECTOR_SPEC -num_physical_tx value                 Set num_physical_tx for specified sector          *",
    "*     set_sector SECTOR_SPEC -cntl_chan_slot value                  Set cntl_chan_slot for specified sector           *",
    "*     set_sector SECTOR_SPEC -has_access_control value              Set has_access_control for specified sector       *",
    "*     set_sector SECTOR_SPEC -chan_idx                              Set chan_idx for specified sector (WLAN)          *",
//  "*     set_sector SECTOR_SPEC -sync_level value                      Set sync_level for specified sector               *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set_color -cell color -cell_idx 'c0 c1 ... cn-1'              Set color for specified cells                     *",
    "*     set_color -all_pa                                             Color all cells by PA                             *",
    "*     set_color -cell_text color                                    Set cell text color                               *",
    "*     set_color -traffic -color -traffic_type t                     Set color for traffic type                        *",
    "*     set_color -system_bdy color                                   Set system boundary color                         *",
    "*     set_color -subnet color -traffic_type t -subnet_idx j         Set subnet color for specified index              *",
    "*     set_color -coverage color -cvg_idx i -scan_idx j              Set coverage analysis color for specified indices *",
    "*     set_color -map_layer color -map_layer_idx i                   Set map layer color for specified layer           *",
//    "*     set_color -map_clutter color -clutter_idx i                   Set map clutter color for specified clutter type  *",
    "*     set_color -road_test_data color -sector i                     Set road test data color for specified sector     *",
    "*     set_color -road_test_data color -level i                      Set road test data color for level between thr's  *",
    "*     set_color -antenna color                                      Set antenna color                                 *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     set_subnet -traffic_type t -subnet strid -arrival_rate t      Set traffic in a subnet                           *",
    "*     set_unused_freq -sector c_s -freq_list 'f0 f1 ... fn-1'       Set unused frequencies for a sector               *",
    "*     set_sector_meas_ctr -sector c_s -gw_csc_cs -traffic_type t               Set Measured Carried Traffic Rate for a sector    *",
    "*                         -meas_ctr t                                                                                 *",
    "*     set_road_test_data -thr_list 't0 t1 ... tn-1'                 Set thresholds for road test data                 *",
    "*     set_csid_byte_length -length n                                Set CSID byte length                              *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     add_cell  -posn_x x -posn_y y -num_sector n [-lon_lat]        Add a cell at the specificied location            *",
    "*     copy_cell  -cell i -posn_x x -posn_y y                        Copy specified cell and place at new location     *",
    "*     move_cell -cell_idx i -posn_x x -posn_y y                     Move the specified cell to posn (x, y)            *",
    "*     delete_cell -cell_idx 'c0 c1 ... cn-1'                        Delete the specified cells                        *",
    "*     add_polygon -type t -traffic_type t                           Add polygon, valid types are                      *",
    "*         -subnet_idx j -subnet_strid str                           (subnet, system_bdy)                              *",
    "*         -gptlist 'x0 y0 ... xn-1 yn-1'                                                                              *",
    "*     filter_system_bdy [-cell] [-map_layer_idx i]                  Filter (delete) objects outside system boundary   *",
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     expand_cch_rssi_table -threshold_db val                       Expand CCH RSSI table down to specified threshold *",
    "*     set_sync_level -level n -sectors 'c0_s0 c1_s1 c2_s2 ...'      Assign the specified sync level to the specified  *",
    "*                                                                   sectors.  0=Master, 1=Slave1, etc.  (-1 to clear) *",
    "*     print_sync_state [-f file]                                    Print state of CCH and sync level for each sector *",
    "*     run_system_sync                                               Run system sync algorithm assigning CCH and sync  *",
    "*                                                                   levels to each sector in the system               *",
    "*     group  -sectors 'c0_s0 c1_s1 c2_s2 ...' -name name            Group the specified sectors for subnet generation *",
    "*     group  [-by_paging_area | -by_csc]                            Create groups by paging area or by CSC            *",
    "*     delete_all_groups                                             Delete all groups                                 *",
    "*     create_subnets -scan_fractional_area f -init_sample_res n     Create subnets                                    *",
    "*         -exclude_ml 'm0 m1 ...' -threshold_db val -dmax d                                                           *",
    "*     delete_subnet -traffic_type t -subnet s                       Delete subnet named s of specified traffic_type   *",
    "*     delete_all_subnet [-traffic_type t]                           Delete all subnets of specified traffic_type, if  *",
    "*                                                                   traffic type not spec del for all traffic types.  *",
    "*     report_subnets -traffic_type t [-f filenname]                 Report subnet data for traffic type t             *",
    "*         [-contained_cells] [-by_segment]                                                                            *",
    "*     display_settings [-f file]                                    Display parameter settings                        *",
    "*     check_antenna_gain -antenna a -f file -numpts n -orient [H|V] Plot antenna gain for specified antenna           *",
#if HAS_MONTE_CARLO
    "*     run_event -num_event n                                        Advance simulation n events                       *",
    "*     run_simulation [-num_event n | -time t]                       Advance simulation n events or for time t         *",
    "*     adjust_offered_traffic -num_init_events i -num_run_events j   Adjust offered traffic to make carried traffic    *",
    "*        -f filename                                                match field measurements.                         *",
    "*     print_call_status [-f file]                                   Print info for all calls in the network           *",
    "*     plot_num_comm -action [start | stop] -f file                  Plot num_comm vs time                             *",
    "*     plot_event -action [start | stop] -f file                     Plot events info                                  *",
    "*     plot_throughput -action [start | stop] -f file                Plot throughput data                              *",
    "*     plot_delay -action [start | stop] -f file                     Plot delay data                                   *",
    "*     measure_crr -action [start | stop]                            Start or stop measuring CRR statistics            *",
    "*                 [-min_crr val -max_crr val -num_crr val]                                                            *",
    "*     plot_crr_cdf -f file                                          Plot CDF of CRR statistics                        *",
    "*     reset_call_statistics                                         Reset counters used for statistical measurements  *",
    "*     print_call_statistics [-f file] [-sectors 'c0_s0 c1_s1 ...']  Print call statistics                             *",
    "*     reset_system                                                  Clears simulation state removing users & stats    *",
    "*     reseed_rangen -seed val                                       Re-seed random number generator                   *",
    "*     run_match -f file [-check check_point_file]                   Run iterative match algorithm                     *",
#endif
    "*---------------------------------------------------------------------------------------------------------------------*",
    "*     date -s str                                                   Echo string str and print current date and time   *",
    "*     h                                                             Show this message screen                          *",
    "*     q                                                             Quit                                              *",
    "***********************************************************************************************************************",
    0};

    set_options(argc, argv);

#if HAS_GUI
    QApplication *app = (QApplication *) NULL;
    if (use_gui) app = new QApplication(argc,argv);

    // CG
    //app->setWindowIcon(QIcon("wisim_icon32.ico"));
#endif

    if (!logfile) {
        logfile = strdup("WiSim.log");
    }

    QString homestr = get_environment_variable("WISIM_HOME");
	if ( homestr.isEmpty() || homestr.isNull() )
	{
        QMessageBox::critical(0, QObject::tr("Start WiSim"),
             QObject::tr("WISIM_HOME variable not setting! Reinstall the application."));

        return -1;
	}
    WiSim_home = CVECTOR(strlen(homestr.toAscii().data()));
    sprintf(WiSim_home, "%s", homestr.toAscii().data());

    if (WiSim_home) {
        if (strlen(WiSim_home)) {
            if (    (WiSim_home[strlen(WiSim_home)-1] == '/' )
                 || (WiSim_home[strlen(WiSim_home)-1] == '\\') ) {
                 WiSim_home[strlen(WiSim_home)-1] = (char) NULL;
            }
        }

        WiSim_pref_file = CVECTOR( strlen(WiSim_home) + strlen("/WiSim_pref_.txt") + strlen(TECHNOLOGY_NAME(technology)));
        sprintf(WiSim_pref_file, "%s%c%s%s%s",
            WiSim_home, FPATH_SEPARATOR, "WiSim_pref_", TECHNOLOGY_NAME(technology), ".txt");
    } else {
        proper_installation = 0;
    }

#if (DEMO == 0)
    const int ris = 55;
    unsigned char reg_info[ris];
#   ifdef __linux__
    reg_info[0] = LIC_LINUX;
    get_mac_info(reg_info+29);     //  6 bytes
    get_hd_srnum(reg_info+35);     // 20 bytes
#   else 
    reg_info[0] = LIC_WINDOWS;


#   endif
#endif

    switch(technology) {
        case CConst::PHS:      np = new PHSNetworkClass();   break;
        case CConst::WCDMA:    np = new WCDMANetworkClass(); break;
        case CConst::CDMA2000: np = new CDMA2000NetworkClass(); break;
        case CConst::WLAN:     np = new WLANNetworkClass();  break;
        default:
            CORE_DUMP;
            break;
    }
    np->input_stream = stdin;
    np->opt_msg = opt_msg;

    np->fl = fopen(logfile, "wb");
    free(logfile);

    if (proper_installation) {
        np->preferences->readFile(WiSim_pref_file);
    }

    GUI_FN(VisibilityWindow    :: setNetworkStruct(np));
    GUI_FN(VisibilityList      :: setNetworkStruct(np));
    GUI_FN(VisibilityItem      :: setNetworkStruct(np));
    GUI_FN(VisibilityCheckItem :: setNetworkStruct(np));

#if HAS_MONTE_CARLO
    GUI_FN(GCallClass          :: setNetworkStruct(np));
#endif

#if HAS_GUI
    //QApplication *app = (QApplication *) NULL;
    if (use_gui) {
        //app = new QApplication(argc,argv);
        set_language(np->preferences->language);

        // translation file for application strings
        if (np->preferences->language == CConst::zh) {
            QTranslator *translator = new QTranslator(0);
#if TR_EMBED
            translator->load( WiSim_zh_qm_data, WiSim_zh_qm_len );
#else
            translator->load( QString( "WiSim_zh.qm" ), "." );
#endif
            app->installTranslator( translator );
#   ifdef __linux__
            application_font = new QFont("ZYSong18030", 10, QFont::Normal);
            fixed_width_font = new QFont("ZYSong18030", 10, QFont::Normal);
#   else
            application_font = new QFont(QString::fromLocal8Bit("ËÎÌו"), 10, QFont::Normal);
            fixed_width_font = new QFont(QString::fromLocal8Bit("ËÎÌו"), 10, QFont::Normal);
#   endif
        } else {
#   ifdef __linux__
            application_font = new QFont("TimesNewRoman", 10, QFont::Normal);
            fixed_width_font = new QFont("Monospace", 10, QFont::Normal);
#   else
            application_font = new QFont("Times New Roman", 10, QFont::Normal);
            fixed_width_font = new QFont("Fixedsys", 10, QFont::Normal);
#   endif
        }

        app->setFont(*application_font);
    }
#endif

#if (DEMO == 0)
    registered = LIC_VALID;
#endif

#if HAS_GUI
    if (use_gui) {
        QString s;
        if (!proper_installation) {

            s = "<h3>WiSIM</h3>";
            s += "<ul>";
            s +=    "<li> " + qApp->translate("QMessageBox", "WiSim Version: ") + WISIM_RELEASE;
            s +=    "<li> " + qApp->translate("QMessageBox", "ERROR: WiSIM has not been properly installed");
            s += "</ul>";

            static QMessageBox* improperInstBox = new QMessageBox( qApp->translate("QMessageBox", "Improper Installation ERROR"),
                s, QMessageBox::Critical, 1 | QMessageBox::Default, 0, 0, 0, 0, TRUE );
                improperInstBox->setButtonText( 1, qApp->translate("QMessageBox", "OK") );
            improperInstBox->show();

            improperInstBox->exec();
            cont = 0;
#if (DEMO == 0)
        } else if (registered != LIC_VALID) {

            char *path;
            usb_sn = read_registration_table("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR\\Enum", "0").toAscii().data();
            path = read_registration_table("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Desktop").toAscii().data();

            RegistrationErrorDialog *regiDia = new RegistrationErrorDialog(registered, np->preferences, reg_info, ris);
            regiDia->set_usb_sn(usb_sn);
            regiDia->set_path(path);
            regiDia->launchDia();
            cont = 0;

#endif
        } else {

            main_window = new MainWindowClass(np);
#ifdef __linux__
            main_window->resize(main_window->sizeHint());
#else
            main_window->showMaximized();
#endif
            qApp->setMainWidget(main_window);

            main_window->setMinimumSize(300, 400);
            if (hide_main) {
                main_window->hide();
            } else if ( QApplication::desktop()->width() > main_window->width() + 10
                && QApplication::desktop()->height() > main_window->height() +30 ) {
                main_window->show();
            } else {
                main_window->showMaximized();
            }
            main_window->editor->zoomToFit();

            if (show_logo) {
                new LogoDisplay(main_window, 0, Qt::WStyle_NoBorder);
            }

            sprintf(np->line_buf, "WiSIM CODE RELEASE VERSION: \"%s\"\n", WISIM_RELEASE);
            PRMSG(stdout, np->line_buf);

            if (geomfile) {
                sprintf(np->line_buf, "read_geometry -f '%s'", geomfile);
                cont = np->process_command(np->line_buf);
                free(geomfile);
            } else {
                cont = 1;
            }

            if (initfile) {
                sprintf(np->line_buf, "include -f '%s'", initfile);
                cont = np->process_command(np->line_buf);
                free(initfile);
            } else {
                cont = 1;
            }

#ifndef __linux__
            if ( (!geomfile) && (!initfile) ) {
                char *my_doc_dir;
                my_doc_dir = read_registration_table("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Personal").toAscii().data();
                if (my_doc_dir == NULL) {
                    chdir(my_doc_dir);
                    free(my_doc_dir);
                }
            }
#endif
        }

        if (cont) {
            QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
            qApp->exec();
        } else {
            qApp->quit();
        }

        set_language(CConst::none);

    } else {
#endif

        if (!proper_installation) {
            printf("ERROR: WiSIM has not been properly installed\n");
#if (DEMO == 0)
        } else if (registered != LIC_VALID) {
            printf("ERROR: This copy of WiSIM is not properly registered\n");
#endif
        } else {
            sprintf(np->line_buf, "h");
            np->process_command(np->line_buf);

            if (geomfile) {
                sprintf(np->line_buf, "read_geometry -f %s", geomfile);
                cont = np->process_command(np->line_buf);
                free(geomfile);
            } else {
                cont = 1;
            }

            if (initfile) {
                sprintf(np->line_buf, "include -f %s", initfile);
                cont = np->process_command(np->line_buf);
                free(initfile);
            } else {
                cont = 1;
            }

            while (cont) {
                if (!get_next_command(np->input_stream, np->line_buf)) {
                    printf("\n WARNING: Unexpected EOF");
                    exit(0);
                }
                cont = np->process_command(np->line_buf);
            }
        }
#if HAS_GUI
    }
#endif

#if CDEBUG
    time(&td);
    printf("TIMESTAMP: %s\n", ctime(&td));
#endif

    if (WiSim_home) { free(WiSim_home); }
    free(WiSim_pref_file);

    return(0);
}
/******************************************************************************************/
int get_next_command(FILE *input_stream, char *line)
{
    int n, retval;

#if 0
    if (input_stream == stdin) {
        printf("\nWiSim> ");
        fflush(stdout);
    }
#endif

    if (fgetline(input_stream, line)) {
        n = strlen(line);
        line[n-1] = (char) NULL;  /* if term_char is '\n' change to NULL */
        retval = 1;
    } else {
        retval = 0;
    }

    return(retval);
}
/******************************************************************************************/
/****  need to zoomToFit after read/import geometry/map/coverage...                    ****/
/****   - Chegnan                                                                      ****/
/******************************************************************************************/
int NetworkClass::process_command(char *line)
{
    int i, n, numpts, orient, allocated;
    int start, cell_idx, thr_idx, subnet_idx, scan_idx, segment_idx, pt_idx, pm_idx, p_idx, num_sector;
    int grp_idx = -1;
    int sector_idx = -1;
    int map_layer_idx = -1;
    int map_fmt = -1;
    int ant_idx;
    int traffic_type_idx, tt_idx;
    int reached_eof;
    int grid_x, grid_y;
    int dx = 0;
    int dy = 0;
    int found;
    int map_sim_res_ratio;
    int num_clutter_type, clutter_type_idx;
    int add_geometry = 0;
    int cvg_idx;
    int changed_bit_cell;
    int prev_num_level;
    int scope, useheight, adjust_angles;
    int bm_idx;
    int ignore_fs_check;
    double subnet_dmax;
    double area = 0.0;
    double scan_fractional_area = 0.0;
    double min_val, max_val;
    double posn_x, posn_y;
    double pwr, pwr_db, r, lon, lat;
    double threshold_db, traffic;
    double clutter_slope, clutter_intercept;
    double sdev_db;
    double min_dist;
    int utm_z, utm_n, simOpt;
    double utm_a, utm_e;
    char *command, *cmd_optlist[50];
    const char **opt_ptr;
    char *chptr;
    time_t timeVal;

    ListClass<int> *int_list;
    ListClass<double> *dbl_list;
    ListClass<IntIntClass> *ii_list;
    CellClass *cell = (CellClass *) NULL;
    TrafficTypeClass *traffic_type = (TrafficTypeClass *) NULL;
    SectorClass *sector = (SectorClass *) NULL;
    AntennaClass *antenna = (AntennaClass *) NULL;
    CoverageClass *cvg = (CoverageClass *) NULL;
    MapLayerClass *ml = (MapLayerClass *) NULL;
    ExpoPropModelClass *expo_pm = (ExpoPropModelClass *) NULL;
    TerrainPropModelClass *trn_pm = (TerrainPropModelClass *) NULL;
    SegmentPropModelClass *seg_pm = (SegmentPropModelClass *) NULL;
    ClutterPropModelClass *clt_pm = (ClutterPropModelClass *) NULL;
    ClutterPropModelFullClass *cltf_pm = (ClutterPropModelFullClass *) NULL;
    ClutterSymFullPropModelClass *cltsf_pm = (ClutterSymFullPropModelClass *) NULL;
    ClutterWtExpoPropModelClass *cltwe_pm = (ClutterWtExpoPropModelClass *) NULL;
    ClutterWtExpoSlopePropModelClass *cltwes_pm = (ClutterWtExpoSlopePropModelClass *) NULL;
    RoadTestPtClass *rtp = (RoadTestPtClass *) NULL;
    PolygonClass *poly;
    FILE *prev_fstream, *fp;
    int cont = 1;
    ListClass<char *> *namelist;
    static char save_line[MAX_LINE_SIZE];
    static const char *prop_model_type_str[] = { "expo", "terrain", "segment", "clutter_simp", "clutter_full", "clutter_sym_full",
                                                "clutter_wt_expo", "clutter_wt_expo_slope", 0};
    static const char *prop_model_param_str[] = { "inflexion", "slope", "all", 0};
    static const char *prop_model_str[]       = { "all", "used", 0};
    static const char *scope_str[] = { "global", "individual", 0};
    static const char *clutter_type_str[] = { "simple", "full", "sym_full", "wt_expo", "wt_expo_slope", "global", "expo_linear", 0};
    static const char *action_str[] = { "stop", "start", 0};
    static const char *cs_type_str[] = { "melco", "sanyo", 0};

    /**************************************************************************************/
    /**** Valid modes:                                                                 ****/
    /****    3-bit integer:   X X X                                                    ****/
    /****                     | | |                                                    ****/
    /****                     | | |__ CConst::noGeomMode   valid (n)                   ****/
    /****                     | |____ CConst::editGeomMode valid (e)                   ****/
    /****                     |______ CConst::simulateMode valid (s)                   ****/
    /**************************************************************************************/
    const int  all_mode_valid = 7;
    const int    n_mode_valid = 1;
    const int    e_mode_valid = 2;
    const int    s_mode_valid = 4;
    const int   ne_mode_valid = n_mode_valid | e_mode_valid;
    // const int   ns_mode_valid = n_mode_valid | s_mode_valid;
    const int   es_mode_valid = e_mode_valid | s_mode_valid;
    /**************************************************************************************/

#if HAS_GUI
    Q3ListViewItem *qlve;
    SubnetClass *subnet;
    MapClutterClass *mc            = (MapClutterClass *) NULL;
    MapHeightClass *mh             = (MapHeightClass *) NULL;
    VisibilityList *vlist          = (VisibilityList *) NULL;
    VisibilityItem *vi             = (VisibilityItem *) NULL;
    VisibilityCheckItem *vci       = (VisibilityCheckItem *) NULL;
    VisibilityItem *cellViewItem   = (VisibilityItem *) NULL;
    VisibilityItem *subnetViewItem = (VisibilityItem *) NULL;
    VisibilityWindow *vw;
    QString s;
#endif

#if CDEBUG
    int rtd_idx;
#endif

    int cmdfound = true;

    int_list = new ListClass<int>(0);
    dbl_list = new ListClass<double>(0);
    ii_list = new ListClass<IntIntClass>(0);

    n = strlen(line);

    if (error_state) {
        if (strcmp(line, "clear_error_state")==0) {
            error_state = 0;
        }
    } else {
        //
        // Not show the command input on command window
        // - 10/21/2008 CG
        //GUI_FN(sprintf(msg, "%s%s", prompt, line));
        //GUI_FN(PRMSG(stdout, msg));
        //

#if CDEBUG
        printf ("LINE = \"%s\"", line);
        printf ("n = %d\n", n);
#endif

        command = strtok(line, CHDELIM);

#if CDEBUG
        printf ("command = \"%s\"\n", command);
#endif

        if (!(   (command == (char *) NULL)
              || (command[0] == '#')
              || (strcmp(command, "include") == 0) )) {
            if ((int) strlen(command) < n-1) {
                sprintf(save_line, "%s %s", command, line + strlen(command) + 1);
            } else {
                sprintf(save_line, "%s", command);
            }
            fprintf(fl, "%s\n", save_line);
            fflush(fl);
            GUI_FN(main_window->gui_command_entered(save_line));
        }

#if HAS_GUI
        if (input_stream == stdin) {
            GUI_FN(main_window->setEnabled(false));
            GUI_FN(QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) ));
        }
#endif

        char *command_opt;
        if ( (command) && (command[0] != '#') ) {
            command_opt = strtok(NULL, "");
        } else {
            command_opt = (char *) NULL;
        }

    //liutao
        int rec_command = 0;
#if HAS_GUI    
        char str_num[4];
        int start_i, end_i;

        if ( command != (char *) NULL && strcmp(command,"goto") == 0) {
            parse_command(command_opt, "s e", cmd_optlist);
            require_param(command, "s", cmd_optlist[0]);
            //require_param(command, "e", cmd_optlist[1]);
            
            check_param_value(start_i, command, "s", cmd_optlist[0], 0, 5000, 0);
            check_param_value(end_i, command, "e", cmd_optlist[1], 0, 5000, start_i);
            
            check_valid_mode(command, all_mode_valid);
            
            int min_index = main_window->editor->min_index;
            int max_index = main_window->editor->max_index;
            
            if(!error_state) {
                //main_window->editor->if_show_3ho = 1;
                if (start_i==1000) {
                    main_window->editor->if_show_handset_anomalies = 0;
                    main_window->editor->resizeCanvas();
                }
                else {
                    if(start_i<min_index)
                        start_i = min_index;
                    if(start_i>max_index)
                        start_i = max_index;
                    if(end_i<min_index)
                        end_i = min_index;
                    if(end_i>max_index)
                        end_i = max_index;
                    if(start_i>end_i)
                        end_i = start_i;
        
                    main_window->editor->if_show_handset_anomalies = 1;
                    main_window->editor->start_index = start_i;
                    main_window->editor->end_index = end_i;
                    main_window->editor->show_handset_anomalies();
                    main_window->editor->resizeCanvas();
                }
            }   
            rec_command = 1;
        }
#endif
    //end. liutao

        if (command == (char *) NULL) {
            /* Ignore blank lines */
        } else if (command[0] == '#') {
            /* Ignore comments beginning with '#' */
        } else if (strcmp(command, "read_geometry") == 0) {
            parse_command(command_opt, "f force_fmt", cmd_optlist);
            check_valid_mode(command, n_mode_valid);
            require_param(command, "f", cmd_optlist[0]);

            if (!error_state) {
                read_geometry(cmd_optlist[0], WiSim_home, cmd_optlist[1]);
                if (error_state) {
                    close_geometry();
                }
            }

            if (!error_state) {
                mode = CConst::editGeomMode;
                process_mode_change();
                set_current_dir_from_file(cmd_optlist[0]);
            }


#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->geometry_filename = QString::fromLocal8Bit(cmd_optlist[0]);

                vlist = main_window->visibility_window->visibility_list;

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                new VisibilityCheckItem( vlist, GConst::antennaRTTI, 0, qApp->translate("VisibilityWindow", "Antenna"), AntennaClass::color );

                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, GCellClass::view_label(this, preferences->vw_cell_name_pref), -2);
                }
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    new VisibilityCheckItem( cellViewItem, GConst::cellRTTI, cell_idx, cell->view_name(cell_idx, preferences->vw_cell_name_pref), cell->color);
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem * subnetViewItem =
                    new VisibilityItem( vlist,     GConst::subnetRTTI, 0, qApp->translate("VisibilityWindow", "Subnets"));
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    vi = new VisibilityItem( subnetViewItem,     GConst::subnetRTTI, traffic_type_idx, traffic_type->name());
                    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                        subnet = subnet_list[traffic_type_idx][subnet_idx];
                        new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
                    }
                }

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                main_window->visibility_window->visibility_list->update_cpm();
                main_window->editor->setVisibility(GConst::clutterPropModelRTTI);

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "Propagation Model"));
                if (technology() == CConst::PHS) {
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "CSID")           );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Paging Area")    );
                }
                if (!main_window->view_menu_is_checked(GConst::visibilityWindow)) {
                    main_window->visibility_window->setGeometry(200, 200, (int) floor(vlist->header()->sectionPos(vlist->header()->mapToIndex(1))*0.7), 400);
                }
                main_window->toggle_visibility_window(GConst::visShow);
                main_window->toggle_info_window(GConst::visShow);
                qApp->processEvents();
                main_window->editor->zoomToFit();
                cellViewItem->toggleChildren();
            }
#endif

#if HAS_ORACLE
        } else if (strcmp(command, "read_geometry_db") == 0) {
            check_valid_mode(command, n_mode_valid);
            if (!error_state) {
                read_geometry_db(WiSim_home);
            }
            if (!error_state) {
                mode = CConst::editGeomMode;
                process_mode_change();
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->geometry_filename = QString();
                vlist = main_window->visibility_window->visibility_list;

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );

                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                }
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    new VisibilityCheckItem( cellViewItem, GConst::cellRTTI, cell_idx, cell->view_name(cell_idx, preferences->vw_cell_name_pref), cell->color);
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem * subnetViewItem =
                    new VisibilityItem( vlist,     GConst::subnetRTTI, 0, qApp->translate("VisibilityWindow", "Subnets"));

                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    vi = new VisibilityItem( subnetViewItem,     GConst::subnetRTTI, traffic_type_idx, traffic_type->name());
                    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                        subnet = subnet_list[traffic_type_idx][subnet_idx];
                        new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
                    }
                }

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));

                main_window->toggle_visibility_window(GConst::visShow);
                qApp->processEvents();
                main_window->editor->zoomToFit();
                cellViewItem->toggleChildren();
            }
#endif
#endif
        } else if (strcmp(command, "close_geometry") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            if (!error_state) {
                close_geometry();
            }
            if (!error_state) {
                mode = CConst::noGeomMode;
                process_mode_change();
                GUI_FN(main_window->geometry_modified = 0);
                GUI_FN( if(main_window->editor->excel_file) {
                    free(main_window->editor->excel_file);
                    main_window->editor->excel_file=(char*)NULL; //liutao
                });
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->geometry_filename = QString("");
                main_window->reset_visibility_window();
                main_window->toggle_info_window(GConst::visHide);
                main_window->editor->zoomToFit();
            }
#endif
        } else if (strcmp(command, "read_road_test_data") == 0) {
            parse_command(command_opt, "f force_fmt", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f", cmd_optlist[0]);

            if (!error_state) {
                read_road_test_data(cmd_optlist[0], cmd_optlist[1]);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->visibility_window->visibility_list->update_rtd(1);
                main_window->visibility_window->resize();
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "shift_road_test_data") == 0) {
            parse_command(command_opt, "x y", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            check_param_value(posn_x, command, "x", cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y, command, "y", cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            if (!error_state) {
                shift_road_test_data(posn_x, posn_y);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->rtd_modified = 1;
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "check_road_test_data") == 0) {
#if HAS_GUI
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            if (!error_state) {
                check_road_test_data();
            }
#endif
        } else if (strcmp(command, "filter_rtd") == 0) {
            parse_command(command_opt, "gptlist", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "gptlist", cmd_optlist[0]);
            extract_intint_list(ii_list, cmd_optlist[0]);

            if (!error_state) {
                filter_rtd(ii_list);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) && n ) {
                main_window->rtd_modified = 1;
                main_window->visibility_window->visibility_list->update_rtd(1);
                main_window->visibility_window->resize();
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "filt_road_test_data") == 0) {
            double angle_deg, width_deg;
            parse_command(command_opt, "sectors angle_deg width_deg", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "sectors", cmd_optlist[0]);
            check_param_value(angle_deg, command, "angle_deg", cmd_optlist[1], -90.0, 360.0,   0.0);
            check_param_value(width_deg, command, "width_deg", cmd_optlist[2],   0.0, 360.0, 360.0);

            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                    extract_sector_list(int_list, cmd_optlist[0], CConst::CellIdxRef);
                } else {
                    int_list->reset();
                }
            }

            if (!error_state) {
                filt_road_test_data(int_list, angle_deg, width_deg);
                bit_cell = -1;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->rtd_modified = 1;
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "save_road_test_data") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!error_state) {
                save_road_test_data(cmd_optlist[0]);
            }
            if (!error_state) {
                GUI_FN(main_window->rtd_modified = 0);
            }
        } else if (strcmp(command, "add_rtd_pt") == 0) {
            parse_command(command_opt, "posn_x posn_y sdev_db", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "posn_x", cmd_optlist[0]);
            require_param(command, "posn_y", cmd_optlist[1]);
            check_param_value(posn_x, command, "posn_x", cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y, command, "posn_y", cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            check_param_value(sdev_db, command, "sdev_db", cmd_optlist[2], 0.0, 1.0e100, 0.0);

            if (!error_state) {
                check_grid_val(posn_x, resolution, system_startx, &grid_x);
                check_grid_val(posn_y, resolution, system_starty, &grid_y);

                n = 0;
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        sector = cell->sector_list[sector_idx];
                        pwr = sector->tx_pwr*sector->comp_prop(this, grid_x, grid_y);
                        if (pwr > add_rtd_pt_threshold) {
                            pwr_db = 10*log(pwr)/log(10.0);
                            if (sdev_db != 0.0) {
                                double g1, g2;
                                gaussian(rg, g1, g2);
                                pwr_db += sdev_db*g1;
                            }
                            road_test_data_list->append( RoadTestPtClass(grid_x, grid_y, pwr_db, cell_idx, sector_idx) );
                            RoadTestPtClass::sort_type = CConst::byPwrSort;
                            road_test_data_list->sort();
                            GUI_FN(main_window->rtd_modified = 1);
                            n++;
                        }
                    }
                }
                printf("NUMBER OF ROAD TEST POINTS ADDED: %d\n", n);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) && n ) {
                main_window->visibility_window->visibility_list->update_rtd(1);
                main_window->visibility_window->resize();
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "write_cch_rssi_table") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!error_state) {
                ((PHSNetworkClass *) this)->write_cch_rssi_table(cmd_optlist[0]);
            }
        } else if (strcmp(command, "comp_prop_model") == 0) {
            parse_command(command_opt, "scope useheight sectors adjust_angles new- min_dist ignore_fs_check", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            check_param_value(scope, command, "scope", cmd_optlist[0], scope_str, 0);
            check_param_value(useheight, command, "useheight", cmd_optlist[1], 0, 1, 0);
            require_param(command, "sectors", cmd_optlist[2]);
            check_param_value(adjust_angles, command, "adjust_angles", cmd_optlist[3], 0, 1, 0);
            check_param_value(min_dist, command, "min_dist", cmd_optlist[5], resolution, 100.0, 15.0);
            check_param_value(ignore_fs_check, command, "ignore_fs_check", cmd_optlist[6], 0, 1, 0);

            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                    extract_sector_list(int_list, cmd_optlist[2], CConst::CellIdxRef);
                } else {
                    n = 0;
                }
            }

            if (!error_state) {

                if (scope == 0) {
                    if (cmd_optlist[4]) {
                        comp_prop_model_segment_angle(int_list->getSize(), &((*int_list)[0]), useheight, 0, 1, ignore_fs_check);
                    } else {
                        comp_prop_model(int_list->getSize(), &((*int_list)[0]), useheight, 0, log(min_dist)/log(10.0), 1, ignore_fs_check);
                    }
                } else {
                    GUI_FN(prog_bar = new ProgressSlot(main_window, "Progress Bar", qApp->translate("ProgressSlot", "Computing Propagation Models") + " ..."));
                    GUI_FN(prog_bar->set_prog_percent(0));
                    for (i=0; (i<=int_list->getSize()-1)&&(!error_state); i++) {
                        cell_idx   = (*int_list)[i] & ((1<<bit_cell)-1);
                        sector_idx = (*int_list)[i] >> bit_cell;
    //                    if (cell_list[cell_idx]->sector_list[sector_idx]->num_road_test_pt < 10) {
    //                        sprintf(msg, "WARNING: CELL %d SECTOR %d has num_road_test_pt = %d < 10."
    //                                         "  Too few points to compute propagation model.\n",
    //                                         cell_idx, sector_idx, cell_list[cell_idx]->sector_list[sector_idx]->num_road_test_pt);
    //                        PRMSG(stdout, msg); warning_state = 1;
    //                    } else {
                            if (cmd_optlist[4]) {
                                comp_prop_model_segment_angle(1, &((*int_list)[i]), useheight, adjust_angles, 0, ignore_fs_check);
                            } else {
                                comp_prop_model(1, &((*int_list)[i]), useheight, adjust_angles, log(min_dist)/log(10.0), 0, ignore_fs_check);
                            }
    //                    }
                        GUI_FN(prog_bar->set_prog_percent((int) 100.0 * (i+1) / n));
                    }
                    GUI_FN(delete prog_bar);
                    GUI_FN(prog_bar = (ProgressSlot *) NULL);
                }

                bit_cell = -1;

                GUI_FN(main_window->geometry_modified = 1);
            }
        } else if (strcmp(command, "comp_prop_error") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            if (!error_state) {
                if ( num_cell && (mode == CConst::editGeomMode) ) {
                    BITWIDTH(bit_cell, num_cell-1);
                }
                comp_prop_error(cmd_optlist[0]);
                if ( mode == CConst::editGeomMode ) {
                    bit_cell = -1;
                }
            }
        } else if (strcmp(command, "set_unassigned_prop_model") == 0) {
            check_valid_mode(command, e_mode_valid);
            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                }
                set_unassigned_prop_model();
                GUI_FN(main_window->geometry_modified = 1);
                bit_cell = -1;
            }
        } else if (strcmp(command, "convert_utm") == 0) {
            parse_command(command_opt, "utm_a utm_e utm_zone utm_north r", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            check_param_value(utm_a, command, "utm_a", cmd_optlist[0], 1000.0, 1.0e100, 6380725.0);
            check_param_value(utm_e, command, "utm_e", cmd_optlist[1],    0.0,     1.0, 0.006681000);

            /* Value of r = 0.0 uses automatic selection of resolution */
            check_param_value(r,     command, "r",     cmd_optlist[4],    0.0, 1.0e100, 0.0);
            if (cmd_optlist[2] && cmd_optlist[3]) {
                check_param_value(utm_z, command, "utm_zone",  cmd_optlist[2], 0, 60, 40);
                check_param_value(utm_n, command, "utm_north", cmd_optlist[3], 0,  1,  1);
            } else if ((!cmd_optlist[2]) && (!cmd_optlist[3])) {
                if (coordinate_system == CConst::CoordLONLAT) {
                    lon = idx_to_x(system_bdy->bdy_pt_x[0][0]);
                    lat = idx_to_y(system_bdy->bdy_pt_y[0][0]);
                    getUTMZone(utm_z, utm_n, lon, lat);
                } else {
                    sprintf(msg, "ERROR: No zone specification for convert_utm\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            } else {
                sprintf(msg, "ERROR: Incomplete zone specification for convert_utm\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                convert_utm(r, utm_a, utm_e, utm_z, utm_n);
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->zoomToFit();
            }
#endif
        } else if (strcmp(command, "flip_lon") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if (!error_state) {
                flip_lon();
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->zoomToFit();
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "adjust_offered_traffic") == 0) {
            parse_command(command_opt, "num_init_events num_run_events f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            check_param_value(i, command, "num_init_events", cmd_optlist[0], 0, 1<<30, 1000);
            check_param_value(n, command, "num_run_events",  cmd_optlist[1], 0, 1<<30, 10000);
            if (!error_state) {
                ((PHSNetworkClass *) this)->adjust_offered_traffic(i, n, cmd_optlist[2]);
            }
#endif
        } else if (strcmp(command, "create_coverage_analysis") == 0) {
            parse_command(command_opt, "name type", cmd_optlist);
            check_valid_mode(command, s_mode_valid);

            require_param(command, "type", cmd_optlist[1]);
            static const char *coverage_type_str[] = { "layer", "level", "sir_layer", "paging_area", 0};
            check_param_value(n, command, "type", cmd_optlist[1], coverage_type_str, 0);

            require_param(command, "name", cmd_optlist[0]);
            if ( (!error_state) && (get_cvg_idx(cmd_optlist[0], 0) != -1) ) {
                sprintf(msg, "ERROR: Coverage analysis name %s already defined\n", cmd_optlist[0]);
                PRMSG(stdout, msg); error_state = 1;
            }

            if (!error_state) {
                switch (n) {
                    case 0:   n = CConst::layerCoverage;      break;
                    case 1:   n = CConst::levelCoverage;      break;
                    case 2:   n = CConst::sirLayerCoverage;   break;
                    case 3:   n = CConst::pagingAreaCoverage; break;
                    default:  CORE_DUMP;                      break;
                }
                num_coverage_analysis++;
                coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));
                coverage_list[num_coverage_analysis-1] = new CoverageClass(this, cmd_optlist[0], n);
            }
        } else if (strcmp(command, "set_coverage_analysis") == 0) {
            parse_command(command_opt, "name num_scan_type thr_idx threshold_db init_sample_res scan_fractional_area cell dmax use_gpm strid", cmd_optlist);
            check_valid_mode(command, s_mode_valid);

            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }

            if (!error_state) {
                cvg = coverage_list[cvg_idx];
                if (cmd_optlist[1]) {
                    if (cvg->type == CConst::pagingAreaCoverage) {
                        sprintf(msg, "ERROR: num_scan_type cannot be set for paging area analysis\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                    if (!error_state) {
                        check_param_value(n, command, "num_scan_type", cmd_optlist[1], 2, 256, 0);
                    }
                    if (!error_state) {
                        int prev_num_scan_type = cvg->scan_list->getSize();
                        for (i=cvg->scan_list->getSize(); i<n; i++) {
                            cvg->scan_list->append(i);
                        }
                        for (i=cvg->scan_list->getSize()-1; i>=n; i--) {
                            cvg->scan_list->del_elem(i);
                        }
                        cvg->color_list = (int *) realloc((void *) cvg->color_list, cvg->scan_list->getSize()*sizeof(int));
                        if (cvg->type == CConst::levelCoverage) {
                            cvg->level_list = (double *) realloc((void *) cvg->level_list, (cvg->scan_list->getSize()-1)*sizeof(double));
                        }
                        for (i=prev_num_scan_type; i<=cvg->scan_list->getSize()-1; i++) {
                            cvg->color_list[i] = hot_color->get_color(i, cvg->scan_list->getSize());
                            if ( (cvg->type == CConst::levelCoverage) && (i >= 1) ) {
                                cvg->level_list[i-1] = 0.0;
                            }
                        }
                    }
                }
                if (cmd_optlist[3]) {
                    double threshold;
                    check_param_value(threshold_db, command, "threshold_db", cmd_optlist[3], -200.0, 200.0, 0.0);
                    threshold = exp(threshold_db*log(10.0)/10.0);
                    if (    (cvg->type == CConst::layerCoverage)
                         || (cvg->type == CConst::sirLayerCoverage)
                         || (cvg->type == CConst::pagingAreaCoverage) ) {
                        cvg->threshold = threshold;
                        cvg->has_threshold = 1;
                    } else if (cvg->type == CConst::levelCoverage) {
                        require_param(command, "thr_idx", cmd_optlist[2]);
                        check_param_value(thr_idx, command, "thr_idx", cmd_optlist[2], 0, cvg->scan_list->getSize()-2, 0);
                        if (!error_state) {
                            cvg->level_list[thr_idx] = threshold;
                        }
                    }
                }
                if (cmd_optlist[4]) {
                    check_param_value(n, command, "init_sample_res", cmd_optlist[4], 1, 2048, 16);
                    if ( (!error_state) && (n & (n-1)) ) {
                        sprintf(msg, "ERROR: command %s parameter -%s set to invalid value %d\nValue must be a power of two\n",
                            command, "init_sample_res", n);
                        PRMSG(stdout, msg);
                        error_state = 1;
                    } else {
                        cvg->init_sample_res = n;
                    }
                }
                if (cmd_optlist[5]) {
                    check_param_value(cvg->scan_fractional_area, command, "scan_fractional_area", cmd_optlist[5], 0.0, 1.0, 0.995);
                }
                if (cmd_optlist[6]) {
                    require_param(command, "dmax", cmd_optlist[7]);
                    cvg->clipped_region = 1;
                    check_param_value(cvg->dmax, command, "dmax", cmd_optlist[7], resolution, 1.0e10, 1000);
                    if (cvg->cell_list) {
                        delete cvg->cell_list;
                        cvg->cell_list = (ListClass<int> *) NULL;
                    }
                    cvg->cell_list = new ListClass<int>(n);
                    extract_cell_list(cvg->cell_list, cmd_optlist[6]);
                }
                if (cmd_optlist[8]) {
                    check_param_value(cvg->use_gpm, command, "use_gpm", cmd_optlist[8], 0, 1, 0);
                }
                if (cmd_optlist[9]) {
                    for (i=0; (i<=num_coverage_analysis-1)&&(!error_state); i++) {
                        if (i != cvg_idx) {
                            if (strcmp(coverage_list[i]->strid, cmd_optlist[9])==0) {
                                sprintf(msg, "ERROR: coverage analysis \"%s\" already defined\n", cmd_optlist[9]);
                                PRMSG(stdout, msg); error_state = 1;
                            }
                        }
                    }
                    if (!error_state) {
                        free(cvg->strid);
                        cvg->strid = strdup(cmd_optlist[9]);
                    }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->visibility_window->visibility_list->update_cvg_analysis(cvg_idx);
            }
#endif
                }
            }
        } else if (strcmp(command, "run_coverage") == 0) {
            parse_command(command_opt, "name", cmd_optlist);
            check_valid_mode(command, s_mode_valid);

            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }

            if (!error_state) {
                cvg = coverage_list[cvg_idx];
                scan_has_threshold = cvg->has_threshold;
                if (scan_has_threshold) {
                    scan_threshold     = cvg->threshold;
                }
                run_coverage(cvg_idx);
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->visibility_window->visibility_list->update_cvg_analysis(cvg_idx);
            }
#endif
        } else if (strcmp(command, "read_coverage_analysis") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f", cmd_optlist[0]);

            if (!error_state) {
                namelist = new ListClass<char *>(0);
                for (cvg_idx=0; cvg_idx<=num_coverage_analysis-1; cvg_idx++) {
                    namelist->append(coverage_list[cvg_idx]->strid);
                }
                num_coverage_analysis++;
                coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));
                coverage_list[num_coverage_analysis-1] = new CoverageClass();
                coverage_list[num_coverage_analysis-1]->read_coverage(this, cmd_optlist[0], (char *) NULL);
                uniquify_str(coverage_list[num_coverage_analysis-1]->strid, namelist);
                delete namelist;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->visibility_window->visibility_list->update_cvg_analysis(num_coverage_analysis-1);
            }
#endif
        } else if (strcmp(command, "write_coverage_analysis") == 0) {
            parse_command(command_opt, "name f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }
            require_param(command, "f", cmd_optlist[1]);

            if (!error_state) {
                cvg = coverage_list[cvg_idx];
                cvg->write_coverage(this, cmd_optlist[1]);
            }

#if HAS_ORACLE
        } else if (strcmp(command, "write_coverage_analysis_db") == 0) {
            parse_command(command_opt, "name", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }

            if (!error_state) {
                cvg = coverage_list[cvg_idx];
                cvg->write_coverage_db(this);
            }
        } else if (strcmp(command, "read_coverage_analysis_db") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, es_mode_valid);

            if (!error_state) {
                num_coverage_analysis++;
                coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));
                coverage_list[num_coverage_analysis-1] = new CoverageClass();
                coverage_list[num_coverage_analysis-1]->read_coverage_db(this);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->visibility_window->visibility_list->update_cvg_analysis(num_coverage_analysis-1);
            }
#endif
#endif
        } else if (strcmp(command, "report_coverage_analysis") == 0) {
            parse_command(command_opt, "name f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }

            if (!error_state) {
                cvg = coverage_list[cvg_idx];
                cvg->report_coverage(this, cmd_optlist[1], -1);
            }
        } else if (strcmp(command, "plot_coverage_analysis") == 0) {
            int cvg_idx_0, cvg_idx_1;

            parse_command(command_opt, "name name2", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx_0 = get_cvg_idx(cmd_optlist[0], 1); }

            cvg_idx_1 = -1;
            if (cmd_optlist[1]) {
                if (!error_state) { cvg_idx_1 = get_cvg_idx(cmd_optlist[1], 1); }
            }

#if HAS_GUI
            if (!error_state) {
                main_window->editor->plotCoverage(cvg_idx_0, cvg_idx_1);
            }
#endif
        } else if (strcmp(command, "delete_coverage_analysis") == 0) {
            parse_command(command_opt, "name", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);
            if (!error_state) { cvg_idx = get_cvg_idx(cmd_optlist[0], 1); }
            if (!error_state) { cvg     = coverage_list[cvg_idx]; }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::coverageTopRTTI, 0);
                vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::coverageRTTI, cvg_idx);
                for (scan_idx=0; scan_idx<=cvg->scan_list->getSize()-1; scan_idx++) {
                    vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::coverageRTTI, scan_idx);
                    vci->setOn(false);
                    delete vci;
                }
                delete vi;
                free(main_window->visibility_window->vec_vis_coverage[cvg_idx]);
                if (cvg_idx < num_coverage_analysis-1) {
                    main_window->visibility_window->vec_vis_coverage[cvg_idx] = main_window->visibility_window->vec_vis_coverage[num_coverage_analysis-1];
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::coverageTopRTTI, 0);
                    vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::coverageRTTI, num_coverage_analysis-1);
                    vi->setIndex(cvg_idx);
                }
                main_window->visibility_window->vec_vis_coverage = (char **) realloc((void *) main_window->visibility_window->vec_vis_coverage, (num_coverage_analysis-1)*sizeof(char));
                if (num_coverage_analysis == 1) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::coverageTopRTTI, 0);
                    delete vi;
                }
            }
#endif
            if (!error_state) {

                delete coverage_list[cvg_idx];
                if (cvg_idx < num_coverage_analysis-1) {
                    coverage_list[cvg_idx] = coverage_list[num_coverage_analysis-1];
                }
                num_coverage_analysis--;
                coverage_list = (CoverageClass **) realloc((void *) coverage_list, num_coverage_analysis*sizeof(CoverageClass *));
            }
        } else if (strcmp(command, "display_geometry") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            if (!error_state) {
                print_geometry(cmd_optlist[0]);
            }
#if HAS_GUI
            if ((use_gui) && (!error_state) && (cmd_optlist[0])) {
                main_window->editor->geometry_filename = QString::fromLocal8Bit(cmd_optlist[0]);
                if ( (main_window->geometry_modified) && (main_window->editor->excel_file) ) {
                    free(main_window->editor->excel_file);
                    main_window->editor->excel_file=(char*)NULL;//liutao
                }
                main_window->geometry_modified = 0;
            }
#endif

#if HAS_ORACLE
        } else if (strcmp(command, "display_geometry_db") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            if (!error_state) {
                print_geometry_db();
            }
#endif

        } else if (strcmp(command, "define_geometry") == 0) {
            parse_command(command_opt, "coord_sys resolution system_bdy", cmd_optlist);
            check_valid_mode(command, n_mode_valid);
            require_param(command, "coord_sys",  cmd_optlist[0]);
            require_param(command, "resolution", cmd_optlist[1]);
            require_param(command, "system_bdy", cmd_optlist[2]);
            check_param_value(resolution, command, "resolution", cmd_optlist[1], 1.0e-6, 100.0, 1.0);

            if (!error_state) {
                define_geometry(cmd_optlist[0], cmd_optlist[2]);
                if (error_state) {
                    close_geometry();
                }
            }
            if (!error_state) {
                mode = CConst::editGeomMode;
                process_mode_change();
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->geometry_filename = QString();
                vlist = main_window->visibility_window->visibility_list;

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );

                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                }
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    new VisibilityCheckItem( cellViewItem, GConst::cellRTTI, cell_idx, cell->view_name(cell_idx, preferences->vw_cell_name_pref), cell->color);
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem * subnetViewItem =
                    new VisibilityItem( vlist,     GConst::subnetRTTI, 0, qApp->translate("VisibilityWindow", "Subnets"));
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    vi = new VisibilityItem( subnetViewItem,     GConst::subnetRTTI, traffic_type_idx, traffic_type->name());
                    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                        subnet = subnet_list[traffic_type_idx][subnet_idx];
                        new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
                    }
                }

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));

                main_window->visibility_window->setGeometry(200, 200, (int) floor(vlist->header()->sectionPos(vlist->header()->mapToIndex(1))*0.7), 400);
                main_window->toggle_visibility_window(GConst::visShow);
                qApp->processEvents();
                main_window->editor->zoomToFit();
                cellViewItem->toggleChildren();
            }
#endif

        } else if (strcmp(command, "print_bmp") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            require_param(command, "f", cmd_optlist[0]);
            check_valid_mode(command, es_mode_valid);

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                QPixmap pixmap = QPixmap(main_window->editor->canvas()->width(),main_window->editor->canvas()->height());
                QPainter pp(&pixmap);
                main_window->editor->canvas()->drawArea(QRect(0, 0, pixmap.width(),pixmap.height()),&pp,FALSE);

                QImage image1 = pixmap.convertToImage();
                QImage image2 = image1.copy(main_window->editor->contentsX(), main_window->editor->contentsY(), main_window->editor->visibleWidth(), main_window->editor->visibleHeight());
                image2.save( QString::fromLocal8Bit(cmd_optlist[0]), "BMP");
            }
#endif

        } else if (strcmp(command, "print_jpg") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            require_param(command, "f", cmd_optlist[0]);
            check_valid_mode(command, es_mode_valid);

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                QPixmap pixmap = QPixmap(main_window->editor->canvas()->width(),main_window->editor->canvas()->height());
                QPainter pp(&pixmap);
                main_window->editor->canvas()->drawArea(QRect(0, 0, pixmap.width(),pixmap.height()),&pp,FALSE);

                QImage image1 = pixmap.convertToImage();
                QImage image2 = image1.copy(main_window->editor->contentsX(), main_window->editor->contentsY(), main_window->editor->visibleWidth(), main_window->editor->visibleHeight());
                image2.save( QString::fromLocal8Bit(cmd_optlist[0]), "JPEG");
            }
#endif
        } else if (strcmp(command, "read_cch_rssi_table") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!error_state) {
                ((PHSNetworkClass *) this)->read_cch_rssi_table(cmd_optlist[0]);
            }
        } else if (strcmp(command, "read_map_layer") == 0) {
            int filter;
            parse_command(command_opt, "f fmt name filter force-", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            static const char *map_fmt_str[] = { "generic", "mif", 0};
            check_param_value(map_fmt, command, "fmt", cmd_optlist[1], map_fmt_str, 0);
            check_param_value(filter, command, "filter", cmd_optlist[3], 0, 1, 1);
            if (!error_state) {
                add_geometry = (system_bdy ? 0 : 1);
                ml = new MapLayerClass();
                switch (map_fmt) {
                    case 0:
                        ml->read_map_layer(this, cmd_optlist[0], (cmd_optlist[4] ? 1 : 0));
                        break;
                    case 1:
                        ml->read_map_layer_mif(this, cmd_optlist[0], cmd_optlist[2], filter);
                        // ml->color = default_color_list[map_layer_list->getSize() % num_default_color];
                        ml->color = hot_color->get_color(map_layer_list->getSize());
                        break;
                    default:
                        CORE_DUMP;
                        break;
                }
                if (error_state) {
                    delete ml;
                }
            }
            if (!error_state) {
                map_layer_list->append(ml);
            }
            if ((!error_state)&&(add_geometry)) {
                mode = CConst::editGeomMode;
                process_mode_change();
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {

                vlist = main_window->visibility_window->visibility_list;
                map_layer_idx = map_layer_list->getSize() - 1;

                if (add_geometry) {
                    main_window->editor->geometry_filename = QString();

                    new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                    cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                    if (!cellViewItem) {
                        cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                    }

#if HAS_MONTE_CARLO
                    VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                    if (!trafficViewItem) {
                        trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                    }
                    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                        traffic_type = traffic_type_list[traffic_type_idx];
                        new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                    }
#endif

                    VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                    if (!cellTextViewItem) {
                        cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                    }

                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                    new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));
                }

                VisibilityItem *mapLayerViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::mapLayerRTTI, 0);
                if (!mapLayerViewItem) {
                    mapLayerViewItem = new VisibilityItem(  vlist, GConst::mapLayerRTTI, 0, qApp->translate("VisibilityWindow", "Map Layer"));
                }

                vci = new VisibilityCheckItem( mapLayerViewItem, GConst::mapLayerRTTI, map_layer_idx, (*map_layer_list)[map_layer_idx]->name, (*map_layer_list)[map_layer_idx]->color);

                if (add_geometry) {
                    main_window->toggle_visibility_window(GConst::visShow);
                    qApp->processEvents();
                    main_window->editor->zoomToFit();
                }

                vci->setOn(true);
            }
#endif
        } else if (strcmp(command, "save_map_layer") == 0) {
            parse_command(command_opt, "map_layer_idx f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "map_layer_idx", cmd_optlist[0]);
            check_param_value(map_layer_idx, command, "map_layer_idx", cmd_optlist[0], 0, map_layer_list->getSize()-1, 0);
            if (!error_state) {
                (*map_layer_list)[map_layer_idx]->save(this, cmd_optlist[1]);
            }
            if (!error_state) {
                (*map_layer_list)[map_layer_idx]->modified = 0;
            }
        } else if (strcmp(command, "read_map_clutter") == 0) {
            parse_command(command_opt, "f force-", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            allocated = 0;
            if (map_clutter) {
                sprintf(msg, "ERROR: map clutter already defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            add_geometry = (system_bdy ? 0 : 1);
            if (!error_state) {
                map_clutter = new MapClutterClass();
                allocated = 1;
                map_clutter->read_map_clutter(this, cmd_optlist[0], (cmd_optlist[1] ? 1 : 0));
            }
            if ((!error_state)&&(add_geometry)) {
                mode = CConst::editGeomMode;
                process_mode_change();
            } else if (error_state && allocated) {
                delete map_clutter;
                map_clutter = (MapClutterClass *) NULL;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                mc = map_clutter;

                if (add_geometry) {
                main_window->editor->geometry_filename = QString();

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));
                }

                VisibilityItem *mapClutterViewItem = new VisibilityItem(  vlist, GConst::mapClutterRTTI, 0, qApp->translate("VisibilityWindow", "Map Clutter"));
                for (clutter_type_idx=0; clutter_type_idx<=map_clutter->num_clutter_type-1; clutter_type_idx++) {
                    new VisibilityCheckItem( mapClutterViewItem, GConst::mapClutterRTTI, clutter_type_idx, mc->description[clutter_type_idx], mc->color[clutter_type_idx]);
                }

                if (add_geometry) {
                    main_window->toggle_visibility_window(GConst::visShow);
                    qApp->processEvents();
                    main_window->editor->zoomToFit();
                }

                mapClutterViewItem->toggleChildren();
            }
#endif
        } else if (strcmp(command, "save_map_clutter") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!map_clutter) {
                sprintf(msg, "ERROR: no map clutter defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                map_clutter->save_map_clutter(this, cmd_optlist[0]);
            }
        } else if (strcmp(command, "read_map_height") == 0) {
            parse_command(command_opt, "f force-", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            allocated = 0;
            if (map_height) {
                sprintf(msg, "ERROR: map height already defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            add_geometry = (system_bdy ? 0 : 1);
            if (!error_state) {
                map_height = new MapHeightClass();
                allocated = 1;
                map_height->read_map_height(this, cmd_optlist[0], (cmd_optlist[1] ? 1 : 0));
            }
            if ((!error_state)&&(add_geometry)) {
                mode = CConst::editGeomMode;
                process_mode_change();
            } else if (error_state && allocated) {
                delete map_height;
                map_height = (MapHeightClass *) NULL;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                mh = map_height;

                if (add_geometry) {
                main_window->editor->geometry_filename = QString();
                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                new VisibilityCheckItem( vlist, GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"), 0); // xxxxxxxxxxxxxx
                }

                vci = new VisibilityCheckItem( vlist, GConst::mapHeightRTTI, 0, qApp->translate("VisibilityWindow", "Map Height"), /*xxxxxxx*/ 0 );

                if (add_geometry) {
                    main_window->toggle_visibility_window(GConst::visShow);
                    qApp->processEvents();
                    main_window->editor->zoomToFit();
                }
                vci->setOn(true);
            }
#endif
        } else if (strcmp(command, "read_map_background") == 0) {
            parse_command(command_opt, "f fposn", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            // require_param(command, "f", cmd_optlist[0]);
            require_param(command, "fposn", cmd_optlist[1]);

#if HAS_GUI
            if (map_background) {
                sprintf(msg, "ERROR: map background already defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }

            if ( (use_gui) && (!error_state) ) {
                add_geometry = (system_bdy ? 0 : 1);
                map_background = new MapBackgroundClass();
                map_background->read_map_background(this, cmd_optlist[0], cmd_optlist[1]);
                if (error_state) {
                    delete map_background;
                    map_background = (MapBackgroundClass *) NULL;
                }
            }
            if ((!error_state)&&(add_geometry)) {
                switch_mode(CConst::editGeomMode);
            }
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                if (add_geometry) {
                main_window->editor->geometry_filename = QString();

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));
                }

                vci = new VisibilityCheckItem( vlist, GConst::mapBackgroundRTTI, 0, qApp->translate("VisibilityWindow", "Map Background"), /*xxxxxxx*/ 0 );

                if (add_geometry) {
                    main_window->toggle_visibility_window(GConst::visShow);
                    qApp->processEvents();
                    main_window->editor->zoomToFit();
                }

                vci->setOn(true);

            }
#endif
        } else if (strcmp(command, "shift_map_background") == 0) {
            parse_command(command_opt, "x y", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            check_param_value(posn_x, command, "x", cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y, command, "y", cmd_optlist[1], -1.0e100, 1.0e100, 0.0);

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                if (!map_background) {
                    sprintf(msg, "ERROR: current design does not contain map background\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if ( (use_gui) && (!error_state) ) {
                check_grid_val(posn_x, resolution, 0, &dx);
                check_grid_val(posn_y, resolution, 0, &dy);
                map_background->shift(dx, dy);
            }
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapBackgroundRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "close_map_background") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, es_mode_valid);

#if HAS_GUI
            if (!map_background) {
                sprintf(msg, "ERROR: no map background defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }

            if ( (use_gui) && (!error_state) ) {
                delete map_background;
                map_background = (MapBackgroundClass *) NULL;
            }
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                vci = (VisibilityCheckItem *) VisibilityList::findItem(vlist, GConst::mapBackgroundRTTI, 0);

                if (vci) {
                    vci->setOn(false);
                    delete vci;
                }
            }
#endif
        } else if (strcmp(command, "shift_map_layer") == 0) {
            parse_command(command_opt, "x y map_layer_idx", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "map_layer_idx", cmd_optlist[2]);
            check_param_value(posn_x, command, "x", cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y, command, "y", cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            check_param_value(map_layer_idx, command, "map_layer_idx", cmd_optlist[2], 0, map_layer_list->getSize()-1, 0);

            if (!error_state) {
                check_grid_val(posn_x, resolution, 0, &dx);
                check_grid_val(posn_y, resolution, 0, &dy);
                (*map_layer_list)[map_layer_idx]->shift(dx, dy);
                (*map_layer_list)[map_layer_idx]->modified = 1;
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapBackgroundRTTI);
                main_window->canvas->update();
            }
#endif
#if HAS_MONTE_CARLO
        // run_event command replaced by run_simulation command, left for backward compatability with old scripts.
        // Should be removed eventually.
        } else if (strcmp(command, "run_event") == 0) {
            parse_command(command_opt, "num_event", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            check_param_value(n, command, "num_event", cmd_optlist[0], 1, 1000000000, 1);
            if (!error_state) {
                run_simulation(1, n, -1.0);
                GUI_FN(main_window->unsaved_sim_data = 1);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::trafficRTTI);
            }
#endif
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "run_simulation") == 0) {
            double time_duration;
            parse_command(command_opt, "num_event time", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            check_param_value(n, command, "num_event", cmd_optlist[0], 1, 1000000000, 1);
            check_param_value(time_duration, command, "time",      cmd_optlist[1], 0.0, 1.0e15, 0.0);

            if (cmd_optlist[0] && (!cmd_optlist[1])) {
                simOpt = 1;
            } else if ((!cmd_optlist[0]) && (cmd_optlist[1])) {
                simOpt = 0;
            } else {
                if (cmd_optlist[0]) {
                    sprintf(msg, "ERROR: Can't specify both num_event and time for run_simulation command\n");
                } else {
                    sprintf(msg, "ERROR: Must specify either num_event or time for run_simulation command\n");
                }
                PRMSG(stdout, msg); error_state = 1;
            }

            if (!error_state) {
                run_simulation(simOpt, n, time_duration);
                GUI_FN(main_window->unsaved_sim_data = 1);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::trafficRTTI);
            }
#endif
#endif
        } else if (strcmp(command, "set") == 0) {
            check_valid_mode(command, e_mode_valid);

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// BEGINNING of code generated by gen_set.pl for set command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            parse_command(command_opt,
                "frequency "
                "cs_dca_alg "
                "ps_dca_alg "
                "memoryless_ps "
                "ac_hide_thr "
                "ac_use_thr "
                "ac_hide_timer "
                "ac_use_timer "
                "num_freq "
                "num_slot "
                "num_cntl_chan_slot "
                "cntl_chan_freq "
                "sir_threshold_call_request_cs_db "
                "sir_threshold_call_request_ps_db "
                "sir_threshold_call_drop_cs_db "
                "sir_threshold_call_drop_ps_db "
                "int_threshold_call_request_cs_db "
                "int_threshold_call_request_ps_db "
                "int_threshold_call_drop_cs_db "
                "int_threshold_call_drop_ps_db "
                "cch_allocation_threshold_db "
                "sync_level_allocation_threshold_db "
                "limit_retry "
                "max_backoff_stage "
                "use_rts_cts "
                "ac_num_user "
                "ac_activate ",                      cmd_optlist);

            if (cmd_optlist[0]) {
                check_param_value(frequency, command, "frequency", cmd_optlist[0], 1.0, 1.0e100, 1.910e9);
                if (!error_state) {
                    sprintf(msg, "frequency set to %15.10e\n", frequency);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[1]) {
                check_technology("cs_dca_alg", CConst::PHS);
                static const char *cs_dca_alg_str[] = { "sir", "int", "int_sir", "sir_int", "melco", 0};
                check_param_value(n, command, "cs_dca_alg", cmd_optlist[1], cs_dca_alg_str, 0);
                if (!error_state) {
                    switch (n) {
                        case 0:   ((PHSNetworkClass *) this)->cs_dca_alg = CConst::SIRDCA; break;
                        case 1:   ((PHSNetworkClass *) this)->cs_dca_alg = CConst::IntDCA; break;
                        case 2:   ((PHSNetworkClass *) this)->cs_dca_alg = CConst::IntSIRDCA; break;
                        case 3:   ((PHSNetworkClass *) this)->cs_dca_alg = CConst::SIRIntDCA; break;
                        case 4:   ((PHSNetworkClass *) this)->cs_dca_alg = CConst::MelcoDCA; break;
                        default:   CORE_DUMP;                    break;
                    }
                    sprintf(msg, "cs_dca_alg set to %s\n",
                        (((PHSNetworkClass *) this)->cs_dca_alg == CConst::SIRDCA) ? "sir" : 
                        (((PHSNetworkClass *) this)->cs_dca_alg == CConst::IntDCA) ? "int" : 
                        (((PHSNetworkClass *) this)->cs_dca_alg == CConst::IntSIRDCA) ? "int_sir" : 
                        (((PHSNetworkClass *) this)->cs_dca_alg == CConst::SIRIntDCA) ? "sir_int" : 
                        "melco");
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[2]) {
                check_technology("ps_dca_alg", CConst::PHS);
                static const char *ps_dca_alg_str[] = { "sir", "int", "int_sir", 0};
                check_param_value(n, command, "ps_dca_alg", cmd_optlist[2], ps_dca_alg_str, 0);
                if (!error_state) {
                    switch (n) {
                        case 0:   ((PHSNetworkClass *) this)->ps_dca_alg = CConst::SIRDCA; break;
                        case 1:   ((PHSNetworkClass *) this)->ps_dca_alg = CConst::IntDCA; break;
                        case 2:   ((PHSNetworkClass *) this)->ps_dca_alg = CConst::IntSIRDCA; break;
                        default:   CORE_DUMP;                    break;
                    }
                    sprintf(msg, "ps_dca_alg set to %s\n",
                        (((PHSNetworkClass *) this)->ps_dca_alg == CConst::SIRDCA) ? "sir" : 
                        (((PHSNetworkClass *) this)->ps_dca_alg == CConst::IntDCA) ? "int" : 
                        "int_sir");
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[3]) {
                check_technology("memoryless_ps", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->memoryless_ps, command, "memoryless_ps", cmd_optlist[3], 0, 1, 0);
                if (!error_state) {
                    sprintf(msg, "memoryless_ps set to %d\n", ((PHSNetworkClass *) this)->memoryless_ps);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[4]) {
                check_technology("ac_hide_thr", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->ac_hide_thr, command, "ac_hide_thr", cmd_optlist[4], 0, 1000, 0);
                if (!error_state) {
                    sprintf(msg, "ac_hide_thr set to %d\n", ((PHSNetworkClass *) this)->ac_hide_thr);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[5]) {
                check_technology("ac_use_thr", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->ac_use_thr, command, "ac_use_thr", cmd_optlist[5], 0, 1000, 0);
                if (!error_state) {
                    sprintf(msg, "ac_use_thr set to %d\n", ((PHSNetworkClass *) this)->ac_use_thr);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[6]) {
                check_technology("ac_hide_timer", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->ac_hide_timer, command, "ac_hide_timer", cmd_optlist[6], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    sprintf(msg, "ac_hide_timer set to %15.10e\n", ((PHSNetworkClass *) this)->ac_hide_timer);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[7]) {
                check_technology("ac_use_timer", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->ac_use_timer, command, "ac_use_timer", cmd_optlist[7], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    sprintf(msg, "ac_use_timer set to %15.10e\n", ((PHSNetworkClass *) this)->ac_use_timer);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[8]) {
                check_technology("num_freq", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->num_freq, command, "num_freq", cmd_optlist[8], 2, 1000, 77);
                if (!error_state) {
                    sprintf(msg, "num_freq set to %d\n", ((PHSNetworkClass *) this)->num_freq);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[9]) {
                check_technology("num_slot", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->num_slot, command, "num_slot", cmd_optlist[9], 1, 1000, 4);
                if (!error_state) {
                    sprintf(msg, "num_slot set to %d\n", ((PHSNetworkClass *) this)->num_slot);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[10]) {
                check_technology("num_cntl_chan_slot", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->num_cntl_chan_slot, command, "num_cntl_chan_slot", cmd_optlist[10], 1, 1000, 80);
                if (!error_state) {
                    sprintf(msg, "num_cntl_chan_slot set to %d\n", ((PHSNetworkClass *) this)->num_cntl_chan_slot);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[11]) {
                check_technology("cntl_chan_freq", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->cntl_chan_freq, command, "cntl_chan_freq", cmd_optlist[11], 0, 1000, 25);
                if (!error_state) {
                    sprintf(msg, "cntl_chan_freq set to %d\n", ((PHSNetworkClass *) this)->cntl_chan_freq);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[12]) {
                check_technology("sir_threshold_call_request_cs", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->sir_threshold_call_request_cs_db, command, "sir_threshold_call_request_cs_db", cmd_optlist[12], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->sir_threshold_call_request_cs = exp(((PHSNetworkClass *) this)->sir_threshold_call_request_cs_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "sir_threshold_call_request_cs_db set to %6.4f\n", ((PHSNetworkClass *) this)->sir_threshold_call_request_cs_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[13]) {
                check_technology("sir_threshold_call_request_ps", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->sir_threshold_call_request_ps_db, command, "sir_threshold_call_request_ps_db", cmd_optlist[13], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->sir_threshold_call_request_ps = exp(((PHSNetworkClass *) this)->sir_threshold_call_request_ps_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "sir_threshold_call_request_ps_db set to %6.4f\n", ((PHSNetworkClass *) this)->sir_threshold_call_request_ps_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[14]) {
                check_technology("sir_threshold_call_drop_cs", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->sir_threshold_call_drop_cs_db, command, "sir_threshold_call_drop_cs_db", cmd_optlist[14], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->sir_threshold_call_drop_cs = exp(((PHSNetworkClass *) this)->sir_threshold_call_drop_cs_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "sir_threshold_call_drop_cs_db set to %6.4f\n", ((PHSNetworkClass *) this)->sir_threshold_call_drop_cs_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[15]) {
                check_technology("sir_threshold_call_drop_ps", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->sir_threshold_call_drop_ps_db, command, "sir_threshold_call_drop_ps_db", cmd_optlist[15], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->sir_threshold_call_drop_ps = exp(((PHSNetworkClass *) this)->sir_threshold_call_drop_ps_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "sir_threshold_call_drop_ps_db set to %6.4f\n", ((PHSNetworkClass *) this)->sir_threshold_call_drop_ps_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[16]) {
                check_technology("int_threshold_call_request_cs", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->int_threshold_call_request_cs_db, command, "int_threshold_call_request_cs_db", cmd_optlist[16], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->int_threshold_call_request_cs = exp(((PHSNetworkClass *) this)->int_threshold_call_request_cs_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "int_threshold_call_request_cs_db set to %6.4f\n", ((PHSNetworkClass *) this)->int_threshold_call_request_cs_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[17]) {
                check_technology("int_threshold_call_request_ps", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->int_threshold_call_request_ps_db, command, "int_threshold_call_request_ps_db", cmd_optlist[17], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->int_threshold_call_request_ps = exp(((PHSNetworkClass *) this)->int_threshold_call_request_ps_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "int_threshold_call_request_ps_db set to %6.4f\n", ((PHSNetworkClass *) this)->int_threshold_call_request_ps_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[18]) {
                check_technology("int_threshold_call_drop_cs", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->int_threshold_call_drop_cs_db, command, "int_threshold_call_drop_cs_db", cmd_optlist[18], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->int_threshold_call_drop_cs = exp(((PHSNetworkClass *) this)->int_threshold_call_drop_cs_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "int_threshold_call_drop_cs_db set to %6.4f\n", ((PHSNetworkClass *) this)->int_threshold_call_drop_cs_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[19]) {
                check_technology("int_threshold_call_drop_ps", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->int_threshold_call_drop_ps_db, command, "int_threshold_call_drop_ps_db", cmd_optlist[19], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->int_threshold_call_drop_ps = exp(((PHSNetworkClass *) this)->int_threshold_call_drop_ps_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "int_threshold_call_drop_ps_db set to %6.4f\n", ((PHSNetworkClass *) this)->int_threshold_call_drop_ps_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[20]) {
                check_technology("cch_allocation_threshold", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->cch_allocation_threshold_db, command, "cch_allocation_threshold_db", cmd_optlist[20], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->cch_allocation_threshold = exp(((PHSNetworkClass *) this)->cch_allocation_threshold_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "cch_allocation_threshold_db set to %6.4f\n", ((PHSNetworkClass *) this)->cch_allocation_threshold_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[21]) {
                check_technology("sync_level_allocation_threshold", CConst::PHS);
                check_param_value(((PHSNetworkClass *) this)->sync_level_allocation_threshold_db, command, "sync_level_allocation_threshold_db", cmd_optlist[21], -200.0, 100.0, 0.0);
                ((PHSNetworkClass *) this)->sync_level_allocation_threshold = exp(((PHSNetworkClass *) this)->sync_level_allocation_threshold_db*log(10.0)/10.0);
                if (!error_state) {
                    sprintf(msg, "sync_level_allocation_threshold_db set to %6.4f\n", ((PHSNetworkClass *) this)->sync_level_allocation_threshold_db);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[22]) {
                check_technology("limit_retry", CConst::WLAN);
                check_param_value(((WLANNetworkClass *) this)->limit_retry, command, "limit_retry", cmd_optlist[22], 0, 1000, 1);
                if (!error_state) {
                    sprintf(msg, "limit_retry set to %d\n", ((WLANNetworkClass *) this)->limit_retry);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[23]) {
                check_technology("max_backoff_stage", CConst::WLAN);
                check_param_value(((WLANNetworkClass *) this)->max_backoff_stage, command, "max_backoff_stage", cmd_optlist[23], 0, 1000, 6);
                if (!error_state) {
                    sprintf(msg, "max_backoff_stage set to %d\n", ((WLANNetworkClass *) this)->max_backoff_stage);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[24]) {
                check_technology("use_rts_cts", CConst::WLAN);
                check_param_value(((WLANNetworkClass *) this)->use_rts_cts, command, "use_rts_cts", cmd_optlist[24], 0, 1, 0);
                if (!error_state) {
                    sprintf(msg, "use_rts_cts set to %d\n", ((WLANNetworkClass *) this)->use_rts_cts);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[25]) {
                check_technology("ac_num_user", CConst::WLAN);
                check_param_value(((WLANNetworkClass *) this)->ac_num_user, command, "ac_num_user", cmd_optlist[25], 0, 1000000, 60);
                if (!error_state) {
                    sprintf(msg, "ac_num_user set to %d\n", ((WLANNetworkClass *) this)->ac_num_user);
                    PRMSG(stdout, msg);
                }
            }
            if (cmd_optlist[26]) {
                check_technology("ac_activate", CConst::WLAN);
                check_param_value(((WLANNetworkClass *) this)->ac_activate, command, "ac_activate", cmd_optlist[26], 0.001, 1.0e9, 6000.0);
                if (!error_state) {
                    sprintf(msg, "ac_activate set to %15.10e\n", ((WLANNetworkClass *) this)->ac_activate);
                    PRMSG(stdout, msg);
                }
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::cellTextRTTI);
                main_window->canvas->update();
                main_window->geometry_modified = 1;
            }
#endif

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// END of code generated by gen_set.pl for set command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

        } else if (strcmp(command, "create_prop_model") == 0) {
            parse_command(command_opt, "name type", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            check_param_value(n, command, "type", cmd_optlist[1], prop_model_type_str, 0);

            require_param(command, "name", cmd_optlist[0]);

            if (!error_state) {
                num_prop_model++;
                prop_model_list = (PropModelClass **) realloc( (void *) prop_model_list, num_prop_model*sizeof(PropModelClass *));

                switch (n) {
                    case 0:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ExpoPropModelClass(cmd_optlist[0]);                        break;
                    case 1:   prop_model_list[num_prop_model-1] = (PropModelClass *) new TerrainPropModelClass(map_clutter, cmd_optlist[0]);        break;
                    case 2:   prop_model_list[num_prop_model-1] = (PropModelClass *) new SegmentPropModelClass(map_clutter, cmd_optlist[0]);        break;
                    case 3:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ClutterPropModelClass(cmd_optlist[0]);            break;
                    case 4:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ClutterPropModelFullClass(cmd_optlist[0]);        break;
                    case 5:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ClutterSymFullPropModelClass(cmd_optlist[0]);     break;
                    case 6:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ClutterWtExpoPropModelClass(cmd_optlist[0]);      break;
                    case 7:   prop_model_list[num_prop_model-1] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(cmd_optlist[0]); break;
                    default:   CORE_DUMP;                    break;
                }
                GUI_FN(main_window->geometry_modified = 1);
            }
        } else if (strcmp(command, "delete_prop_model") == 0) {
            parse_command(command_opt, "name pm_idx force-", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if ( (!error_state) && (!cmd_optlist[0]) && (!cmd_optlist[1]) ) {
                sprintf(msg, "ERROR: delete_prop_model requires specification of pm_idx or name\n");
                PRMSG(stdout, msg);
                error_state = 1;
            } else if (cmd_optlist[0]) {
                pm_idx = get_pm_idx(cmd_optlist[0], 1);
                int_list->reset();
                int_list->append(pm_idx);
            } else {
                extract_pm_list(int_list, cmd_optlist[1]);
            }

            if ( (!error_state) ) {
                for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        sector = cell->sector_list[sector_idx];
                        if (int_list->contains(sector->prop_model)) {
                            if (cmd_optlist[2]) {
                                sprintf( msg, "WARNING: CELL %d SECTOR %d prop_model set to UNASSIGNED\n", cell_idx, sector_idx);
                                sector->prop_model = -1;
                            } else {
                                sprintf( msg, "ERROR: CELL %d SECTOR %d is currently using prop_model %d\n",
                                    cell_idx, sector_idx, pm_idx);
                                PRMSG(stdout, msg); error_state = 1;
                            }
                        }
                    }
                }
            }

            if ( (!error_state) ) {
                delete_prop_model(int_list);
                GUI_FN(main_window->geometry_modified = 1);
            }
        } else if (strcmp(command, "delete_all_unused_prop_model") == 0) {
            check_valid_mode(command, e_mode_valid);
            delete_all_unused_prop_model();
            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "delete_all_prop_model") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if ( (!error_state) ) {
                for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        sector = cell->sector_list[sector_idx];
                        sector->prop_model = -1;
                    }
                }

                int_list->reset();
                for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
                    int_list->append(pm_idx);
                }
                delete_prop_model(int_list);

                GUI_FN(main_window->geometry_modified = 1);
            }
        } else if (strcmp(command, "set_prop_model") == 0) {
            parse_command(command_opt,
                /*  0 */ "pm_idx "
                /*  1 */ "type "
                /*  2 */ "exponent "
                /*  3 */ "coefficient "
                /*  4 */ "val_y "
                /*  5 */ "val_py "
                /*  6 */ "val_s1 "
                /*  7 */ "val_s2 "
                /*  8 */ "useheight "
                /*  9 */ "start_slope "
                /* 10 */ "final_slope "
                /* 11 */ "num_pts "
                /* 12 */ "pt_idx "
                /* 13 */ "x "
                /* 14 */ "y "
                /* 15 */ "num_clutter_type "
                /* 16 */ "c_idx "
                /* 17 */ "c_coeff "
                /* 18 */ "slope "
                /* 19 */ "intercept "
                /* 20 */ "r0 "
                /* 21 */ "name "
                /* 22 */ "coeff_logh "
                /* 23 */ "coeff_loghd ", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if ( (!error_state) && (!cmd_optlist[0]) && (!cmd_optlist[21]) ) {
                sprintf(msg, "ERROR: set_prop_model requires specification of pm_idx or name\n");
                PRMSG(stdout, msg);
                error_state = 1;
            } else if (cmd_optlist[21]) {
                pm_idx = get_pm_idx(cmd_optlist[21], 1);
            } else {
                check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[0],         0, num_prop_model-1, 0);
            }

            if ( (!error_state) && (cmd_optlist[1]) ) {
                check_param_value(n, command, "type", cmd_optlist[1], prop_model_type_str, 0);
                if (!error_state) {
                    PropModelClass *tmp_pm = prop_model_list[pm_idx];
                    switch (n) {
                        case 0:   prop_model_list[pm_idx] = (PropModelClass *) new ExpoPropModelClass(tmp_pm->get_strid());                        break;
                        case 1:   prop_model_list[pm_idx] = (PropModelClass *) new TerrainPropModelClass(map_clutter, tmp_pm->get_strid());        break;
                        case 2:   prop_model_list[pm_idx] = (PropModelClass *) new SegmentPropModelClass(map_clutter, tmp_pm->get_strid());        break;
                        case 3:   prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelClass(tmp_pm->get_strid());            break;
                        case 4:   prop_model_list[pm_idx] = (PropModelClass *) new ClutterPropModelFullClass(tmp_pm->get_strid());        break;
                        case 5:   prop_model_list[pm_idx] = (PropModelClass *) new ClutterSymFullPropModelClass(tmp_pm->get_strid());     break;
                        case 6:   prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoPropModelClass(tmp_pm->get_strid());      break;
                        case 7:   prop_model_list[pm_idx] = (PropModelClass *) new ClutterWtExpoSlopePropModelClass(tmp_pm->get_strid()); break;
                        default:  CORE_DUMP;                                                                                              break;
                    }
                    delete tmp_pm;
                }
            }

            if ( (!error_state) && (cmd_optlist[2]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropExpo) {
                    sprintf(msg, "ERROR: exponent can only be set on propagation model of type EXPONENTIAL\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    expo_pm = (ExpoPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(expo_pm->exponent, command, "exponent", cmd_optlist[2], 0.0, 100.0, 2.0);
                }
                if (!error_state) {
                    sprintf(msg, "exponent set to %15.10e\n", expo_pm->exponent);
                    PRMSG(stdout, msg);
                }
            }

            if ( (!error_state) && (cmd_optlist[3]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropExpo) {
                    sprintf(msg, "ERROR: coefficient can only be set on propagation model of type EXPONENTIAL\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    expo_pm = (ExpoPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(expo_pm->coefficient, command, "coefficient", cmd_optlist[3], 0.0, 1.0e100, 1.0);
                }
                if (!error_state) {
                    sprintf(msg, "coefficient set to %15.10e\n", expo_pm->coefficient);
                    PRMSG(stdout, msg);
                }
            }

            if ( (!error_state) && (cmd_optlist[4]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropTerrain) {
                    sprintf(msg, "ERROR: val_y can only be set on propagation model of type TERRAIN\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    trn_pm = (TerrainPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(trn_pm->val_y, command, "val_y", cmd_optlist[4], -200.0, 200.0, 2.0);
                }
                if (!error_state) {
                    sprintf(msg, "val_y set to %15.10e\n", trn_pm->val_y);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[5]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropTerrain) {
                    sprintf(msg, "ERROR: val_py can only be set on propagation model of type TERRAIN\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    trn_pm = (TerrainPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(trn_pm->val_py, command, "val_py", cmd_optlist[5], -1.0e100, 1.0e100, 1.0);
                }
                if (!error_state) {
                    sprintf(msg, "val_py set to %15.10e\n", trn_pm->val_py);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[6]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropTerrain) {
                    sprintf(msg, "ERROR: val_s1 can only be set on propagation model of type TERRAIN\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    trn_pm = (TerrainPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(trn_pm->val_s1, command, "val_s1", cmd_optlist[6], -1.0e100, 1.0e100, 1.0);
                }
                if (!error_state) {
                    sprintf(msg, "val_s1 set to %15.10e\n", trn_pm->val_s1);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[7]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropTerrain) {
                    sprintf(msg, "ERROR: val_s2 can only be set on propagation model of type TERRAIN\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    trn_pm = (TerrainPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(trn_pm->val_s2, command, "val_s2", cmd_optlist[7], -1.0e100, 1.0e100, 1.0);
                }
                if (!error_state) {
                    sprintf(msg, "val_s2 set to %15.10e\n", trn_pm->val_s2);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[8]) ) {
                switch (prop_model_list[pm_idx]->type()) {
                    case CConst::PropSegment:
                        seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                        n = seg_pm->useheight;
                        check_param_value(seg_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            if (n != seg_pm->useheight) {
                                if (!seg_pm->useheight) {
                                    for (i=0; i<=seg_pm->num_clutter_type-1; i++) {
                                        seg_pm->vec_k[i] = seg_pm->vec_k[i+2];
                                    }
                                }
                                seg_pm->vec_k = (double *) realloc((void *) seg_pm->vec_k, (seg_pm->num_clutter_type+2*seg_pm->useheight)*sizeof(double));
                                if (seg_pm->useheight) {
                                    for (i=seg_pm->num_clutter_type-1; i>=0; i--) {
                                        seg_pm->vec_k[i+2] = seg_pm->vec_k[i];
                                    }
                                    seg_pm->vec_k[0] = 0.0;
                                    seg_pm->vec_k[1] = 0.0;
                                }
                            }
                            sprintf(msg, "useheight set to %d\n", seg_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterSimp:
                        clt_pm = (ClutterPropModelClass *) prop_model_list[pm_idx];
                        n = clt_pm->num_clutter_type+clt_pm->useheight;
                        check_param_value(clt_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            clt_pm->mvec_x = (double *) realloc((void *) clt_pm->mvec_x, (clt_pm->num_clutter_type+clt_pm->useheight)*sizeof(double));
                            for (i=n; i<=clt_pm->num_clutter_type+clt_pm->useheight-1; i++) {
                                clt_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "useheight set to %d\n", clt_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterFull:
                        cltf_pm = (ClutterPropModelFullClass *) prop_model_list[pm_idx];
                        n = 2*cltf_pm->num_clutter_type + cltf_pm->useheight;
                        check_param_value(cltf_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            cltf_pm->mvec_x = (double *) realloc((void *) cltf_pm->mvec_x, (2*cltf_pm->num_clutter_type+cltf_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltf_pm->num_clutter_type+cltf_pm->useheight-1; i++) {
                                cltf_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "useheight set to %d\n", cltf_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterSymFull:
                        cltsf_pm = (ClutterSymFullPropModelClass *) prop_model_list[pm_idx];
                        n = 2*cltsf_pm->num_clutter_type + cltsf_pm->useheight;
                        check_param_value(cltsf_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            cltsf_pm->mvec_x = (double *) realloc((void *) cltsf_pm->mvec_x, (2*cltsf_pm->num_clutter_type+cltsf_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltsf_pm->num_clutter_type+cltsf_pm->useheight-1; i++) {
                                cltsf_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "useheight set to %d\n", cltsf_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterWtExpo:
                        cltwe_pm = (ClutterWtExpoPropModelClass *) prop_model_list[pm_idx];
                        n = 2*cltwe_pm->num_clutter_type + cltwe_pm->useheight;
                        check_param_value(cltwe_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            cltwe_pm->mvec_x = (double *) realloc((void *) cltwe_pm->mvec_x, (2*cltwe_pm->num_clutter_type+cltwe_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltwe_pm->num_clutter_type+cltwe_pm->useheight-1; i++) {
                                cltwe_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "useheight set to %d\n", cltwe_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterWtExpoSlope:
                        cltwes_pm = (ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx];
                        n = cltwes_pm->num_clutter_type + cltwes_pm->useheight;
                        check_param_value(cltwes_pm->useheight, command, "useheight", cmd_optlist[8], 0, 1, 0);
                        if (!error_state) {
                            cltwes_pm->mvec_x = (double *) realloc((void *) cltwes_pm->mvec_x, (cltwes_pm->num_clutter_type+cltwe_pm->useheight)*sizeof(double));
                            for (i=n; i<=cltwes_pm->num_clutter_type+cltwe_pm->useheight-1; i++) {
                                cltwe_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "useheight set to %d\n", cltwes_pm->useheight);
                            PRMSG(stdout, msg);
                        }
                        break;
                    default:
                        sprintf(msg, "ERROR: useheight can only be set on propagation models of type SEGMENT, CLUTTER, CLUTTERFULL, of CLUTTERWTEXPO\n");
                        PRMSG(stdout, msg);
                        error_state = 1;
                        break;
                }
            }
            if ( (!error_state) && (cmd_optlist[9]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: start_slope can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(seg_pm->start_slope, command, "start_slope", cmd_optlist[9], -200.0, 200.0, -2.0);
                }
                if (!error_state) {
                    sprintf(msg, "start_slope set to %15.10e\n", seg_pm->start_slope);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[10]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: final_slope can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(seg_pm->final_slope, command, "final_slope", cmd_optlist[10], -200.0, 200.0, -2.0);
                }
                if (!error_state) {
                    sprintf(msg, "final_slope set to %15.10e\n", seg_pm->final_slope);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[11]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: num_pts can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    n = seg_pm->num_inflexion;
                    check_param_value(seg_pm->num_inflexion, command, "num_pts", cmd_optlist[11], 1, 100, 1);
                }
                if (!error_state) {
                    seg_pm->x = (double *) realloc((void *) seg_pm->x, seg_pm->num_inflexion*sizeof(double));
                    seg_pm->y = (double *) realloc((void *) seg_pm->y, seg_pm->num_inflexion*sizeof(double));
                    for (i=n; i<=seg_pm->num_inflexion-1; i++) {
                        seg_pm->x[i] = seg_pm->x[n-1];
                        seg_pm->y[i] = seg_pm->y[n-1];
                    }
                    sprintf(msg, "num_pts set to %d\n", seg_pm->num_inflexion);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[13] || cmd_optlist[14]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: x, y can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    require_param(command, "pt_idx", cmd_optlist[12]);
                    check_param_value(n, command, "pt_idx", cmd_optlist[12], 0, seg_pm->num_inflexion-1, 0);
                    if ( (!error_state) && (cmd_optlist[13]) ) {
                        check_param_value(seg_pm->x[n], command, "x", cmd_optlist[13], -200.0, 200.0, 0.0);
                        if (!error_state) {
                            sprintf(msg, "x[%d] set to %15.10e\n", n, seg_pm->x[n]);
                            PRMSG(stdout, msg);
                        }
                    }
                    if ( (!error_state) && (cmd_optlist[14]) ) {
                        check_param_value(seg_pm->y[n], command, "y", cmd_optlist[14], -500, 500, 0.0);
                        if (!error_state) {
                            sprintf(msg, "y[%d] set to %15.10e\n", n, seg_pm->y[n]);
                            PRMSG(stdout, msg);
                        }
                    }
                }
            }
            if ( (!error_state) && (cmd_optlist[15]) ) {
                switch (prop_model_list[pm_idx]->type()) {
                    case CConst::PropSegment:
                        seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                        n = seg_pm->num_clutter_type;
                        check_param_value(seg_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 100, 0);
                        if (!error_state) {
                            seg_pm->vec_k = (double *) realloc((void *) seg_pm->vec_k, (seg_pm->num_clutter_type+2*seg_pm->useheight)*sizeof(double));
                            for (i=n; i<=seg_pm->num_clutter_type-1; i++) {
                                seg_pm->vec_k[2*seg_pm->useheight+i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", seg_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterSimp:
                        clt_pm = (ClutterPropModelClass *) prop_model_list[pm_idx];
                        n = clt_pm->num_clutter_type+clt_pm->useheight;
                        check_param_value(clt_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 1000000, 0);
                        if (!error_state) {
                            clt_pm->mvec_x = (double *) realloc((void *) clt_pm->mvec_x, (clt_pm->num_clutter_type+clt_pm->useheight)*sizeof(double));
                            for (i=n; i<=clt_pm->num_clutter_type+clt_pm->useheight-1; i++) {
                                clt_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", clt_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterFull:
                        cltf_pm = (ClutterPropModelFullClass *) prop_model_list[pm_idx];
                        n = 2*cltf_pm->num_clutter_type + cltf_pm->useheight;
                        check_param_value(cltf_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 1000000, 0);
                        if (!error_state) {
                            cltf_pm->mvec_x = (double *) realloc((void *) cltf_pm->mvec_x, (2*cltf_pm->num_clutter_type+cltf_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltf_pm->num_clutter_type+cltf_pm->useheight-1; i++) {
                                cltf_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", cltf_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterSymFull:
                        cltsf_pm = (ClutterSymFullPropModelClass *) prop_model_list[pm_idx];
                        n = 2*cltsf_pm->num_clutter_type + cltsf_pm->useheight;
                        check_param_value(cltsf_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 1000000, 0);
                        if (!error_state) {
                            cltsf_pm->mvec_x = (double *) realloc((void *) cltsf_pm->mvec_x, (2*cltsf_pm->num_clutter_type+cltsf_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltsf_pm->num_clutter_type+cltsf_pm->useheight-1; i++) {
                                cltsf_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", cltsf_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterWtExpo:
                        cltwe_pm = (ClutterWtExpoPropModelClass *) prop_model_list[pm_idx];
                        n = 2*cltwe_pm->num_clutter_type + cltwe_pm->useheight;
                        check_param_value(cltwe_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 1000000, 0);
                        if (!error_state) {
                            cltwe_pm->mvec_x = (double *) realloc((void *) cltwe_pm->mvec_x, (2*cltwe_pm->num_clutter_type+cltwe_pm->useheight)*sizeof(double));
                            for (i=n; i<=2*cltwe_pm->num_clutter_type+cltwe_pm->useheight-1; i++) {
                                cltwe_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", cltwe_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    case CConst::PropClutterWtExpoSlope:
                        cltwes_pm = (ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx];
                        n = cltwes_pm->num_clutter_type + cltwes_pm->useheight;
                        check_param_value(cltwes_pm->num_clutter_type, command, "num_clutter_type", cmd_optlist[15], 0, 1000000, 0);
                        if (!error_state) {
                            cltwes_pm->mvec_x = (double *) realloc((void *) cltwes_pm->mvec_x, (cltwes_pm->num_clutter_type+cltwes_pm->useheight)*sizeof(double));
                            for (i=n; i<=cltwes_pm->num_clutter_type+cltwes_pm->useheight-1; i++) {
                                cltwes_pm->mvec_x[i] = 0.0;
                            }
                            sprintf(msg, "num_clutter_type set to %d\n", cltwes_pm->num_clutter_type);
                            PRMSG(stdout, msg);
                        }
                        break;
                    default:
                        sprintf(msg, "ERROR: num_clutter_type can only be set for propagation model of type SEGMENT, CLUTTER, CLUTTERFULL, of CLUTTERSYMFULL\n");
                        PRMSG(stdout, msg);
                        error_state = 1;
                        break;
                }
            }
            if ( (!error_state) && (cmd_optlist[17]) ) {
                require_param(command, "c_idx", cmd_optlist[16]);

                switch (prop_model_list[pm_idx]->type()) {
                    case CConst::PropSegment:
                        seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, seg_pm->num_clutter_type-1, 0);
                        if (!error_state) {
                            check_param_value(seg_pm->vec_k[2*seg_pm->useheight+n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    case CConst::PropClutterSimp:
                        clt_pm = (ClutterPropModelClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, clt_pm->num_clutter_type+clt_pm->useheight-1, 0);
                        if (!error_state) {
                            check_param_value(clt_pm->mvec_x[n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    case CConst::PropClutterFull:
                        cltf_pm = (ClutterPropModelFullClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, 2*cltf_pm->num_clutter_type+cltf_pm->useheight-1, 0);
                        if (!error_state) {
                            check_param_value(cltf_pm->mvec_x[n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    case CConst::PropClutterSymFull:
                        cltsf_pm = (ClutterSymFullPropModelClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, 2*cltsf_pm->num_clutter_type+cltsf_pm->useheight-1, 0);
                        if (!error_state) {
                            check_param_value(cltsf_pm->mvec_x[n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    case CConst::PropClutterWtExpo:
                        cltwe_pm = (ClutterWtExpoPropModelClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, 2*cltwe_pm->num_clutter_type+cltwe_pm->useheight-1, 0);
                        if (!error_state) {
                            check_param_value(cltwe_pm->mvec_x[n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    case CConst::PropClutterWtExpoSlope:
                        cltwes_pm = (ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx];
                        check_param_value(n, command, "c_idx", cmd_optlist[16], 0, cltwes_pm->num_clutter_type+cltwes_pm->useheight-1, 0);
                        if (!error_state) {
                            check_param_value(cltwes_pm->mvec_x[n], command, "c_coeff", cmd_optlist[17], -200.0, 200.0, 0.0);
                        }
                        break;
                    default:
                        sprintf(msg, "ERROR: c_coeff can only be set for propagation models of type SEGMENT, CLUTTER, CLUTTERFULL, CLUTTERSYMFULL, CLUTTERWTEXPO\n");
                        PRMSG(stdout, msg);
                        error_state = 1;
                        break;
                }
            }
            if ( (!error_state) && (cmd_optlist[18]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropClutterSimp) {
                    sprintf(msg, "ERROR: slope can only be set on propagation model of type CLUTTER\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    clt_pm = (ClutterPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(clt_pm->k, command, "slope", cmd_optlist[18], -1.0e10, 1.0e10, 0.0);
                }
                if (!error_state) {
                    sprintf(msg, "slope set to %15.10e\n", clt_pm->k);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[19]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropClutterSimp) {
                    sprintf(msg, "ERROR: intercept can only be set on propagation model of type CLUTTER\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    clt_pm = (ClutterPropModelClass *) prop_model_list[pm_idx];
                    check_param_value(clt_pm->b, command, "intercept", cmd_optlist[19], -1.0e10, 1.0e10, 0.0);
                }
                if (!error_state) {
                    sprintf(msg, "intercept set to %15.10e\n", clt_pm->b);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[20]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropClutterWtExpoSlope) {
                    sprintf(msg, "ERROR: r0 can only be set on propagation model of type CLUTTERWTEXPOSLOPE\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    cltwes_pm = (ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx];
                    check_param_value(cltwes_pm->r0, command, "intercept", cmd_optlist[20], 1.0e-6, 1.0e10, 316.0);
                }
                if (!error_state) {
                    sprintf(msg, "r0 set to %15.10e\n", cltwes_pm->r0);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[22]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: coeff_logh can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    if (!seg_pm->useheight) {
                        sprintf(msg, "ERROR: coeff_logh can't be set when useheight = 0\n");
                        PRMSG(stdout, msg);
                        error_state = 1;
                    } else {
                        check_param_value(seg_pm->vec_k[0], command, "coeff_logh", cmd_optlist[22], 1.0e-6, 1.0e10, 0.0);
                    }
                }
                if (!error_state) {
                    sprintf(msg, "coeff_logh set to %15.10e\n", seg_pm->vec_k[0]);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[23]) ) {
                if (prop_model_list[pm_idx]->type() != CConst::PropSegment) {
                    sprintf(msg, "ERROR: coeff_loghd can only be set on propagation model of type SEGMENT\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    seg_pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                    if (!seg_pm->useheight) {
                        sprintf(msg, "ERROR: coeff_loghd can't be set when useheight = 0\n");
                        PRMSG(stdout, msg);
                        error_state = 1;
                    } else {
                        check_param_value(seg_pm->vec_k[1], command, "coeff_loghd", cmd_optlist[23], 1.0e-6, 1.0e10, 0.0);
                    }
                }
                if (!error_state) {
                    sprintf(msg, "coeff_loghd set to %15.10e\n", seg_pm->vec_k[0]);
                    PRMSG(stdout, msg);
                }
            }

            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "report_prop_model") == 0) {
            parse_command(command_opt, "name f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "name", cmd_optlist[0]);

            if (!error_state) { pm_idx = get_pm_idx(cmd_optlist[0], 1); }

            fp = (FILE *) NULL;
            if (!error_state) {
                if ( (cmd_optlist[1] == NULL) || (strcmp(cmd_optlist[1], "") == 0) ) {
                    fp = stdout;
                } else if ( !(fp = fopen(cmd_optlist[1], "w")) ) {
                    sprintf(msg, "ERROR: Unable to write to file %s\n", cmd_optlist[1]);
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                prop_model_list[pm_idx]->report(this, fp);
                if ( fp != stdout ) {
                    fclose(fp);
                }
            }
        } else if (strcmp(command, "report_prop_model_param") == 0) {
            parse_command(command_opt, "param model f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "param", cmd_optlist[0]);
            check_param_value(n, command, "param", cmd_optlist[0], prop_model_param_str, 0);
            check_param_value(i, command, "model", cmd_optlist[1], prop_model_str, 0);

            if (!error_state) {
                report_prop_model_param(n, i, cmd_optlist[2]);
            }
        } else if (strcmp(command, "report_clutter_model") == 0) {
            parse_command(command_opt, "pm_idx name f map_layer_idx p_idx", cmd_optlist);
            check_valid_mode(command, es_mode_valid);

            if ( (!error_state) && (!cmd_optlist[0]) && (!cmd_optlist[1]) ) {
                sprintf(msg, "ERROR: set_prop_model requires specification of pm_idx or name\n");
                PRMSG(stdout, msg);
                error_state = 1;
            } else if (cmd_optlist[1]) {
                pm_idx = get_pm_idx(cmd_optlist[1], 1);
            } else {
                check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[0],         0, num_prop_model-1, 0);
            }

            poly = (PolygonClass *) NULL;
            if ((!error_state) && (cmd_optlist[3])) {
                check_param_value(map_layer_idx, command, "map_layer_idx", cmd_optlist[3], 0, map_layer_list->getSize()-1, 0);

                require_param(command, "p_idx", cmd_optlist[4]);
                check_param_value(p_idx, command, "p_idx", cmd_optlist[4], 0, (*map_layer_list)[map_layer_idx]->num_polygon-1, 0);
                if (!error_state) {
                    poly = (*map_layer_list)[map_layer_idx]->polygon_list[p_idx];
                }
            }

            fp = (FILE *) NULL;
            if (!error_state) {
                if (!prop_model_list[pm_idx]->is_clutter_model()) {
                    sprintf(msg, "ERROR: Propagation model \"%s\" is not a clutter model\n", prop_model_list[pm_idx]->get_strid());
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                if ( (cmd_optlist[2] == NULL) || (strcmp(cmd_optlist[2], "") == 0) ) {
                    fp = stdout;
                } else if ( !(fp = fopen(cmd_optlist[2], "w")) ) {
                    sprintf(msg, "ERROR: Unable to write to file %s\n", cmd_optlist[2]);
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                ((GenericClutterPropModelClass *) prop_model_list[pm_idx])->report_clutter(this, fp, poly);
                if ( fp != stdout ) {
                    fclose(fp);
                }
            }
        } else if (strcmp(command, "set_cell") == 0) {
            parse_command(command_opt, "cell num_sector", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "cell", cmd_optlist[0]);
            extract_cell_list(int_list, cmd_optlist[0]);
            
            if ( (!error_state) && (cmd_optlist[1]) ) {
                check_param_value(num_sector, command, "num_sector", cmd_optlist[1], 1, 10, 1);
                if (!error_state) {
                    for (i=0; i<=int_list->getSize()-1; i++) {
                        cell_idx = (*int_list)[i];
                        cell = cell_list[cell_idx];

                        if (cell->num_sector != num_sector) {
                            for (sector_idx=num_sector; sector_idx<=cell->num_sector-1; sector_idx++) {
                                delete cell->sector_list[sector_idx];
                            }
                            cell->sector_list = (SectorClass **) realloc((void *) cell->sector_list, num_sector*sizeof(SectorClass *));
                            for (sector_idx=cell->num_sector; sector_idx<=num_sector-1; sector_idx++) {
                                cell->sector_list[sector_idx] = cell->sector_list[0]->duplicate(0);
                                cell->sector_list[sector_idx]->parent_cell = cell;
                            }
                            cell->num_sector = num_sector;
                            GUI_FN(main_window->geometry_modified = 1);
                        }

                        sprintf(msg, "Cell %d num_sector set to %d\n", cell_idx, cell->num_sector);
                        PRMSG(stdout, msg);
                    }
                }
            }
        } else if (strcmp(command, "set_sector") == 0) {
            check_valid_mode(command, e_mode_valid);
            if (num_cell) {
                BITWIDTH(bit_cell, num_cell-1);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// BEGINNING of code generated by gen_set.pl for set_sector command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            parse_command(command_opt, "sector sector_csid_hex sector_gw_csc_cs "
                "antenna_type "
                "antenna_height "
                "antenna_angle_deg "
                "tx_pwr "
                "num_physical_tx "
                "cntl_chan_slot "
                "has_access_control "
                "sync_level "
                "csid_hex "
                "gw_csc_cs "
                "prop_model "
                "chan_idx ",                      cmd_optlist);

            if ( (cmd_optlist[0] ? 1 : 0) + (cmd_optlist[1] ? 1 : 0) + (cmd_optlist[2] ? 1 : 0) != 1 ) {
                sprintf(msg, "ERROR: Invalid sector specification, must specify \"sector\", \"sector_csid_hex\", or \"sector_gw_csc_cs\"\n");
                PRMSG(stdout, msg); error_state = 1;
            }

            if (!error_state) {
                if (cmd_optlist[0]) {
                    extract_sector_list(int_list, cmd_optlist[0], CConst::CellIdxRef);
                } else if (cmd_optlist[1]) {
                    extract_sector_list(int_list, cmd_optlist[1], CConst::CellHexCSIDRef);
                } else if (cmd_optlist[2]) {
                    extract_sector_list(int_list, cmd_optlist[2], CConst::CellCSNumberRef);
                }
            }

            if (!error_state) {
                if (int_list->getSize() != 1) {
                    sprintf(msg, "ERROR: Invalid sector specification\n");
                    PRMSG(stdout, msg); error_state = 1;
                } else {
                    cell_idx   = (*int_list)[0] & ((1<<bit_cell)-1);
                    sector_idx = (*int_list)[0] >> bit_cell;
                    sector = cell_list[cell_idx]->sector_list[sector_idx];
                }
            }

            if ( (!error_state) && (cmd_optlist[3]) ) {
                check_param_value(sector->antenna_type, command, "antenna_type", cmd_optlist[3], 0, num_antenna_type-1, 0);
                if (!error_state) {
                    sprintf(msg, "antenna_type set to %d\n", sector->antenna_type);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[4]) ) {
                check_param_value(sector->antenna_height, command, "antenna_height", cmd_optlist[4], 0.0, 100.0, 32.0);
                if (!error_state) {
                    sprintf(msg, "antenna_height set to %15.10e\n", sector->antenna_height);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[5]) ) {
                double antenna_angle_deg;
                check_param_value(antenna_angle_deg, command, "antenna_angle_deg", cmd_optlist[5], -180.0, 360.0, 0.0);
                if (!error_state) {
                    sector->antenna_angle_rad = antenna_angle_deg * PI / 180.0;
                    sprintf(msg, "antenna_angle set to %15.10f (deg) = %15.10f (rad)\n",
                        antenna_angle_deg, sector->antenna_angle_rad);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[6]) ) {
                check_param_value(sector->tx_pwr, command, "tx_pwr", cmd_optlist[6], 0.0, 1.0e5, 1000.0);
                if (!error_state) {
                    sprintf(msg, "tx_pwr set to %15.10e\n", sector->tx_pwr);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[7]) ) {
                check_technology("num_physical_tx", CConst::PHS);
                check_param_value(((PHSSectorClass *) sector)->num_physical_tx, command, "num_physical_tx", cmd_optlist[7], 0, 100, 4);
                if (!error_state) {
                    sprintf(msg, "num_physical_tx set to %d\n", ((PHSSectorClass *) sector)->num_physical_tx);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[8]) ) {
                check_technology("cntl_chan_slot", CConst::PHS);
                check_param_value(((PHSSectorClass *) sector)->cntl_chan_slot, command, "cntl_chan_slot", cmd_optlist[8], 0, 1000, 0);
                if (!error_state) {
                    sprintf(msg, "cntl_chan_slot set to %d\n", ((PHSSectorClass *) sector)->cntl_chan_slot);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[9]) ) {
                check_technology("has_access_control", CConst::PHS);
                check_param_value(((PHSSectorClass *) sector)->has_access_control, command, "has_access_control", cmd_optlist[9], 0, 1, 0);
                if (!error_state) {
                    sprintf(msg, "has_access_control set to %d\n", ((PHSSectorClass *) sector)->has_access_control);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[10]) ) {
                check_technology("sync_level", CConst::PHS);
                check_param_value(((PHSSectorClass *) sector)->sync_level, command, "sync_level", cmd_optlist[10], 0, max_sync_level, 0);
                if (!error_state) {
                    sprintf(msg, "sync_level set to %d\n", ((PHSSectorClass *) sector)->sync_level);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[11]) ) {
                check_technology("csid_hex", CConst::PHS);
                if ( strlen(cmd_optlist[11]) != 2*PHSSectorClass::csid_byte_length ) {
                    sprintf(msg, "ERROR: invalid set_sector command\n"
                                     "CSID: %s has improper length\n", cmd_optlist[11]);
                    PRMSG(stdout, msg);
                    error_state = 1;
                } else {
                    if (!((PHSSectorClass *) sector)->csid_hex) {
                        ((PHSSectorClass *) sector)->csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
                    }
                    if (!hexstr_to_hex(((PHSSectorClass *) sector)->csid_hex, cmd_optlist[11], PHSSectorClass::csid_byte_length)) {
                        error_state = 1;
                    }
                }
            }
            if ( (!error_state) && (cmd_optlist[12]) ) {
                check_technology("gw_csc_cs", CConst::PHS);
                check_param_value(((PHSSectorClass *) sector)->gw_csc_cs, command, "gw_csc_cs", cmd_optlist[12], 0, 999999, 0);
                if (!error_state) {
                    sprintf(msg, "gw_csc_cs set to %d\n", ((PHSSectorClass *) sector)->gw_csc_cs);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[13]) ) {
                check_param_value(sector->prop_model, command, "prop_model", cmd_optlist[13], -1, num_prop_model-1, 0);
                if (!error_state) {
                    sprintf(msg, "prop_model set to %d\n", sector->prop_model);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[14]) ) {
                check_technology("chan_idx", CConst::WLAN);
                check_param_value(((WLANSectorClass *) sector)->chan_idx, command, "chan_idx", cmd_optlist[14], 1, num_freq, 0);
                if (!error_state) {
                    sprintf(msg, "chan_idx set to %d\n", ((WLANSectorClass *) sector)->chan_idx);
                    PRMSG(stdout, msg);
                }
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::cellTextRTTI);
                main_window->editor->setVisibility(GConst::antennaRTTI);
                main_window->canvas->update();

                vlist = main_window->visibility_window->visibility_list;
                vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::cellRTTI, cell_idx);
                cell = cell_list[cell_idx];
                vci->setText(0, cell->view_name(cell_idx, preferences->vw_cell_name_pref));
                vlist->sort();
                main_window->geometry_modified = 1;
            }
#endif

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// END of code generated by gen_set.pl for set_sector command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            bit_cell = -1;
        } else if (strcmp(command, "set_traffic") == 0) {
            check_valid_mode(command, e_mode_valid);

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// BEGINNING of code generated by gen_set.pl for set_traffic command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            parse_command(command_opt, "traffic_type "
                "duration_dist "
                "mean_time "
                "min_time "
                "max_time "
                "num_attempt_request "
                "num_attempt_handover "
                "ps_meas_best_channel "
                "bit_per_pkt "
                "user_data_rate "
                "basic_rate "
                "phy_rate "
                "hd_rate "
                "mean_session_duration "
                "drop_total_pkt "
                "drop_error_pkt "
                "mean_segment "
                "dur_compress "
                "packet_loss_thr "
                "duration_dropcall_thr "
                "max_consecut_drop_pkt ",                      cmd_optlist);

            require_param(command, "traffic_type", cmd_optlist[0]);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
            }
            if (!error_state) {
                traffic_type = traffic_type_list[traffic_type_idx];
            }

            if ( (!error_state) && (cmd_optlist[1]) ) {
                static const char *duration_dist_str[] = { "expo", "unif", 0};
                check_param_value(n, command, "duration_dist", cmd_optlist[1], duration_dist_str, 0);
                if (!error_state) {
                    switch (n) {
                        case 0:   traffic_type->duration_dist = CConst::ExpoDist; break;
                        case 1:   traffic_type->duration_dist = CConst::UnifDist; break;
                        default:   CORE_DUMP;                    break;
                    }
                    sprintf(msg, "duration_dist set to %s\n",
                        (traffic_type->duration_dist == CConst::ExpoDist) ? "expo" : 
                        "unif");
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[2]) ) {
                check_param_value(traffic_type->mean_time, command, "mean_time", cmd_optlist[2], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    sprintf(msg, "mean_time set to %15.10e\n", traffic_type->mean_time);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[3]) ) {
                check_param_value(traffic_type->min_time, command, "min_time", cmd_optlist[3], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    sprintf(msg, "min_time set to %15.10e\n", traffic_type->min_time);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[4]) ) {
                check_param_value(traffic_type->max_time, command, "max_time", cmd_optlist[4], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    sprintf(msg, "max_time set to %15.10e\n", traffic_type->max_time);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[5]) ) {
                check_technology("num_attempt_request", CConst::PHS);
                check_param_value(((PHSTrafficTypeClass *) traffic_type)->num_attempt_request, command, "num_attempt_request", cmd_optlist[5], 1, 10, 4);
                if (!error_state) {
                    sprintf(msg, "num_attempt_request set to %d\n", ((PHSTrafficTypeClass *) traffic_type)->num_attempt_request);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[6]) ) {
                check_technology("num_attempt_handover", CConst::PHS);
                check_param_value(((PHSTrafficTypeClass *) traffic_type)->num_attempt_handover, command, "num_attempt_handover", cmd_optlist[6], 0, 10, 4);
                if (!error_state) {
                    sprintf(msg, "num_attempt_handover set to %d\n", ((PHSTrafficTypeClass *) traffic_type)->num_attempt_handover);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[7]) ) {
                check_technology("ps_meas_best_channel", CConst::PHS);
                check_param_value(((PHSTrafficTypeClass *) traffic_type)->ps_meas_best_channel, command, "ps_meas_best_channel", cmd_optlist[7], 0, 1, 0);
                if (!error_state) {
                    sprintf(msg, "ps_meas_best_channel set to %d\n", ((PHSTrafficTypeClass *) traffic_type)->ps_meas_best_channel);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[8]) ) {
                check_technology("bit_per_pkt", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->bit_per_pkt, command, "bit_per_pkt", cmd_optlist[8], 1, 1000000, 2364);
                if (!error_state) {
                    sprintf(msg, "bit_per_pkt set to %d\n", ((WLANTrafficTypeClass *) traffic_type)->bit_per_pkt);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[9]) ) {
                check_technology("user_data_rate", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->user_data_rate, command, "user_data_rate", cmd_optlist[9], 1.0, 1.0e9, 1000.0);
                if (!error_state) {
                    sprintf(msg, "user_data_rate set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->user_data_rate);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[10]) ) {
                check_technology("basic_rate", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->basic_rate, command, "basic_rate", cmd_optlist[10], 1.0, 1.0e9, 64.0);
                if (!error_state) {
                    sprintf(msg, "basic_rate set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->basic_rate);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[11]) ) {
                check_technology("phy_rate", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->phy_rate, command, "phy_rate", cmd_optlist[11], 1.0, 1.0e9, 11000.0);
                if (!error_state) {
                    sprintf(msg, "phy_rate set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->phy_rate);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[12]) ) {
                check_technology("hd_rate", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->hd_rate, command, "hd_rate", cmd_optlist[12], 1.0, 1.0e9, 2000.0);
                if (!error_state) {
                    sprintf(msg, "hd_rate set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->hd_rate);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[13]) ) {
                check_technology("mean_session_duration", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->mean_session_duration, command, "mean_session_duration", cmd_optlist[13], 0.001, 1.0e9, 50.0);
                if (!error_state) {
                    sprintf(msg, "mean_session_duration set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->mean_session_duration);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[14]) ) {
                check_technology("drop_total_pkt", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->drop_total_pkt, command, "drop_total_pkt", cmd_optlist[14], 0, 1000000, 128);
                if (!error_state) {
                    sprintf(msg, "drop_total_pkt set to %d\n", ((WLANTrafficTypeClass *) traffic_type)->drop_total_pkt);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[15]) ) {
                check_technology("drop_error_pkt", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->drop_error_pkt, command, "drop_error_pkt", cmd_optlist[15], 0, 1000000, 5);
                if (!error_state) {
                    sprintf(msg, "drop_error_pkt set to %d\n", ((WLANTrafficTypeClass *) traffic_type)->drop_error_pkt);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[16]) ) {
                check_technology("mean_segment", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->mean_segment, command, "mean_segment", cmd_optlist[16], 0, 1.0e9, 20);
                if (!error_state) {
                    sprintf(msg, "mean_segment set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->mean_segment);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[17]) ) {
                check_technology("dur_compress", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->dur_compress, command, "dur_compress", cmd_optlist[17], 0.0, 1.0e9, 15.0);
                if (!error_state) {
                    sprintf(msg, "dur_compress set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->dur_compress);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[18]) ) {
                check_technology("packet_loss_thr", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->packet_loss_thr, command, "packet_loss_thr", cmd_optlist[18], 0.0, 1.0, 0.05);
                if (!error_state) {
                    sprintf(msg, "packet_loss_thr set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->packet_loss_thr);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[19]) ) {
                check_technology("duration_dropcall_thr", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->duration_dropcall_thr, command, "duration_dropcall_thr", cmd_optlist[19], 0.0, 1.0e9, 100.0);
                if (!error_state) {
                    sprintf(msg, "duration_dropcall_thr set to %15.10e\n", ((WLANTrafficTypeClass *) traffic_type)->duration_dropcall_thr);
                    PRMSG(stdout, msg);
                }
            }
            if ( (!error_state) && (cmd_optlist[20]) ) {
                check_technology("max_consecut_drop_pkt", CConst::WLAN);
                check_param_value(((WLANTrafficTypeClass *) traffic_type)->max_consecut_drop_pkt, command, "max_consecut_drop_pkt", cmd_optlist[20], 0, 100, 5);
                if (!error_state) {
                    sprintf(msg, "max_consecut_drop_pkt set to %d\n", ((WLANTrafficTypeClass *) traffic_type)->max_consecut_drop_pkt);
                    PRMSG(stdout, msg);
                }
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////// END of code generated by gen_set.pl for set_traffic command
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "set_total_traffic") == 0) {
            check_valid_mode(command, e_mode_valid);

            parse_command(command_opt, "traffic_type arrival_rate", cmd_optlist);

            require_param(command, "traffic_type", cmd_optlist[0]);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
            }

            if ( (!error_state) && (cmd_optlist[1]) ) {
                check_param_value(traffic, command, "arrival_rate", cmd_optlist[1], 0.0, 1.0e100, 1.0);
                if (!error_state) {
                    set_total_arrival_rate(traffic_type_idx, traffic);
                }
                if (!error_state) {
                    sprintf(msg, "Total arrival rate for traffic type \"%s\" set to %15.10e\n", traffic_type_list[traffic_type_idx]->name(), traffic);
                    PRMSG(stdout, msg);
                }
            }

            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "set_bitmap") == 0) {
            check_valid_mode(command, es_mode_valid);
            parse_command(command_opt, "cell_idx bm_idx", cmd_optlist);
            require_param(command, "cell", cmd_optlist[0]);
            extract_cell_list(int_list, cmd_optlist[0]);
            
            if ( (!error_state) && (cmd_optlist[1]) ) {
                check_param_value(bm_idx, command, "bm_idx", cmd_optlist[1], 0, CellClass::num_bm-1, 0);
                if (!error_state) {
                    for (i=0; i<=int_list->getSize()-1; i++) {
                        cell_idx = (*int_list)[i];
                        cell_list[cell_idx]->bm_idx = bm_idx;
                    }
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    main_window->editor->setVisibility(GConst::cellRTTI);
                    main_window->canvas->update();
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
        } else if (strcmp(command, "set_color") == 0) {
            int color;
            GUI_FN(vlist = main_window->visibility_window->visibility_list);
            check_valid_mode(command, es_mode_valid);
            parse_command(command_opt,
            /*  0 */    "cell "
            /*  1 */    "cell_idx "
            /*  2 */    "cell_text "
            /*  3 */    "traffic "
            /*  4 */    "traffic_type "
            /*  5 */    "system_bdy "
            /*  6 */    "subnet "
            /*  7 */    "subnet_idx "
            /*  8 */    "coverage "
            /*  9 */    "cvg_idx "
            /* 10 */    "scan_idx "
            /* 11 */    "map_layer "
            /* 12 */    "map_layer_idx "
            /* 13 */    "map_clutter "
            /* 14 */    "map_clutter_idx "
            /* 15 */    "road_test_data "
            /* 16 */    "sector "
            /* 17 */    "level "
            /* 18 */    "antenna "
            /* 19 */    "all_pa- "
            /* 20 */    "gw_csc_cs"
                , cmd_optlist);

            if ( (!error_state) && (cmd_optlist[0]) ) {
                check_param_value(color, command, "cell", cmd_optlist[0], 0, 0xFFFFFF, 0);

                if ( (!cmd_optlist[1]) && (!cmd_optlist[20]) ) {
                    sprintf(msg, "ERROR: Either cell_idx or gw_csc_cs must be specified\n");
                    PRMSG(stdout, msg); error_state = 1;
                } else if (cmd_optlist[1]) {
                    check_param_value(color,            command, "cell",     cmd_optlist[0], 0, 0xFFFFFF, 0);
                    extract_cell_list(int_list, cmd_optlist[1]);
                } else {
                    extract_int_list(int_list, cmd_optlist[20]);
                    for (i=0; i<=int_list->getSize()-1; i++) {
                        uid_to_sector((*int_list)[i], cell_idx, sector_idx);
                        if (cell_idx == -1) {
                            sprintf(msg, "WARNING: GW_CSC_CS = %.6d not found\n", (*int_list)[i]);
                            PRMSG(stdout, msg); warning_state = 1;
                        }
                        (*int_list)[i] = cell_idx;
                    }
                }

                if (!error_state) {
                    for (i=0; i<=int_list->getSize()-1; i++) {
                        cell_idx = (*int_list)[i];
                        if (cell_idx != -1) {
                            cell_list[cell_idx]->color = color;
                        }
                    }
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist,  GConst::cellRTTI, 0);
                    if (vi) {
                        for (i=0; i<=int_list->getSize()-1; i++) {
                            cell_idx = (*int_list)[i];
                            if (cell_idx != -1) {
                                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::cellRTTI, cell_idx);
                                if (vci) {
                                    vci->setItemColor(color);
                                }
                            }
                        }
                    }
                    main_window->editor->setVisibility(GConst::cellRTTI);
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[2]) ) {
                check_param_value(color, command, "cell_text", cmd_optlist[2], 0, 0xFFFFFF, 0);
                if (!error_state) {
                    CellClass::text_color = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                    if (qlve) {
                        ((VisibilityItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::cellTextRTTI);
                    }
                }

#endif
            }
            if ( (!error_state) && (cmd_optlist[3]) ) {
                require_param(command, "traffic_type", cmd_optlist[4]);
                if (!error_state) {
                    traffic_type_idx = get_traffic_type_idx(cmd_optlist[4], 1);
                }
                check_param_value(color,            command, "traffic",          cmd_optlist[3], 0, 0xFFFFFF, 0);

                if (!error_state) {
                    traffic_type_list[traffic_type_idx]->color = color;
                }
#if (HAS_GUI && HAS_MONTE_CARLO)
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::trafficRTTI, traffic_type_idx);
                    }
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        GCallClass::deletePixmap();
                        GCallClass::setPixmap();
                        main_window->editor->setVisibility(GConst::trafficRTTI);
                    }
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[5]) ) {
                check_param_value(color, command, "system_bdy", cmd_optlist[5], 0, 0xFFFFFF, 0);
                if (!error_state) {
                    NetworkClass::system_bdy_color = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::systemBoundaryRTTI, 0);
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::systemBoundaryRTTI);
                    }
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[6]) ) {
                check_param_value(color, command, "subnet", cmd_optlist[6], 0, 0xFFFFFF, 0);
                require_param(command, "traffic_type", cmd_optlist[4]);
                if (!error_state) {
                    traffic_type_idx = get_traffic_type_idx(cmd_optlist[4], 1);
                }
                if (!error_state) {
                    require_param(command, "subnet_idx", cmd_optlist[7]);
                    check_param_value(subnet_idx, command, "subnet_idx", cmd_optlist[7], 0, num_subnet[traffic_type_idx]-1, 0);
                }
                if (!error_state) {
                    subnet_list[traffic_type_idx][subnet_idx]->color = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::subnetRTTI, traffic_type_idx);
                    }
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::subnetRTTI, subnet_idx);
                    }
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::subnetRTTI);
                    }
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[8]) ) {
                check_param_value(color, command, "coverage", cmd_optlist[8], 0, 0xFFFFFF, 0);
                require_param(command, "cvg_idx", cmd_optlist[9]);
                check_param_value(cvg_idx, command, "cvg_idx", cmd_optlist[9], 0, num_coverage_analysis-1, 0);
                if (!error_state) {
                    require_param(command, "scan_idx", cmd_optlist[10]);
                    check_param_value(scan_idx, command, "scan_idx", cmd_optlist[10], 0, coverage_list[cvg_idx]->scan_list->getSize()-1, 0);
                }
                if (!error_state) {
                    coverage_list[cvg_idx]->color_list[scan_idx] = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::coverageTopRTTI, 0);
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::coverageRTTI, cvg_idx);
                    }
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::coverageRTTI, scan_idx);
                    }
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::coverageRTTI);
                    }
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[11]) ) {
                check_param_value(color, command, "map_layer", cmd_optlist[11], 0, 0xFFFFFF, 0);
                require_param(command, "map_layer_idx", cmd_optlist[12]);
                check_param_value(map_layer_idx, command, "map_layer_idx", cmd_optlist[12], 0, map_layer_list->getSize()-1, 0);
                if (!error_state) {
                    (*map_layer_list)[map_layer_idx]->color = color;
                    (*map_layer_list)[map_layer_idx]->modified = 1;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::mapLayerRTTI, 0);
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::mapLayerRTTI, map_layer_idx);
                    }
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::mapLayerRTTI);
                    }
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[13]) ) {
                int map_clutter_idx;
                if (map_clutter) {
                    check_param_value(color, command, "map_clutter", cmd_optlist[13], 0, 0xFFFFFF, 0);
                    require_param(command, "map_clutter_idx", cmd_optlist[14]);
                    check_param_value(map_clutter_idx, command, "map_clutter_idx", cmd_optlist[14], 0, map_clutter->num_clutter_type-1, 0);
                } else {
                    sprintf(msg, "ERROR: Map Clutter Not Defined\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
                if (!error_state) {
                    map_clutter->color[map_clutter_idx] = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::mapClutterRTTI, 0);
                    if (qlve) {
                        qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::mapClutterRTTI, map_clutter_idx);
                    }
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::mapClutterRTTI);
                    }
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[15]) ) {
                check_param_value(color, command, "road_test_data", cmd_optlist[15], 0, 0xFFFFFF, 0);
                if (cmd_optlist[16]) {
                    if (!error_state) {
                        if (num_cell) {
                            BITWIDTH(bit_cell, num_cell-1);
                        }
                        extract_sector_list(int_list, cmd_optlist[16], CConst::CellIdxRef);
                    }
                    if (!error_state) {
                        if (int_list->getSize() != 1) {
                            sprintf(msg, "ERROR: Invalid sector specification\n");
                            PRMSG(stdout, msg); error_state = 1;
                        } else {
                            cell_idx   = (*int_list)[0] & ((1<<bit_cell)-1);
                            sector_idx = (*int_list)[0] >> bit_cell;
                            sector = cell_list[cell_idx]->sector_list[sector_idx];
                        }
                        bit_cell = -1;
                    }

                    if (!error_state) {
                        sector->road_test_pt_color = color;
                    }
#if HAS_GUI
                    if ( (use_gui) && (!error_state) ) {
                        int idx = 0;
                        found = 0;
                        vw = main_window->visibility_window;
                        while ((idx <= vw->num_road_test_data_set) && (!found)) {
                            if (   (vw->vec_rtd_cell_idx[idx]   == cell_idx)
                                && (vw->vec_rtd_sector_idx[idx] == sector_idx) ) {
                                found = 1;
                            } else {
                                idx++;
                            }
                        }
                        qlve = VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);
                        if ((found) && (qlve)) {
                            qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::roadTestDataRTTI, 0);
                            if (qlve) {
                                qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::roadTestDataRTTI, idx);
                            }
                            if (qlve) {
                                ((VisibilityCheckItem *) qlve)->setItemColor(color);
                                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                            }
                        }
                    }
#endif
                } else if (cmd_optlist[17]) {
                    check_param_value(i, command, "level", cmd_optlist[17], 0, RoadTestPtClass::num_level, 0);
                    if (!error_state) {
                        RoadTestPtClass::color_list[i] = color;
                    }
#if HAS_GUI
                    if ( (use_gui) && (!error_state) ) {
                        qlve = VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);
                        if (qlve) {
                            qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::roadTestDataRTTI, 1);
                        }
                        if (qlve) {
                            qlve = VisibilityList::findItem((VisibilityItem *) qlve, GConst::roadTestDataRTTI, i);
                        }
                        if (qlve) {
                            ((VisibilityCheckItem *) qlve)->setItemColor(color);
                            main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                        }
                    }
#endif
                } else {
                    sprintf(msg, "ERROR: must specify either sector or level\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }

            if ( (!error_state) && (cmd_optlist[18]) ) {
                check_param_value(color, command, "antenna", cmd_optlist[18], 0, 0xFFFFFF, 0);
                if (!error_state) {
                    AntennaClass::color = color;
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    qlve = VisibilityList::findItem(vlist, GConst::antennaRTTI, 0);
                    if (qlve) {
                        ((VisibilityCheckItem *) qlve)->setItemColor(color);
                        main_window->editor->setVisibility(GConst::antennaRTTI);
                    }
                }
#endif
            }
            if ( (!error_state) && (cmd_optlist[19]) ) {
                if (!error_state) {
                    ((PHSNetworkClass *) this)->color_cells_by_pa();
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist,  GConst::cellRTTI, 0);
                    if (vi) {
                        for (i=0; i<=int_list->getSize()-1; i++) {
                            cell_idx = (*int_list)[i];
                            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::cellRTTI, cell_idx);
                            if (vci) {
                                vci->setItemColor(color);
                            }
                        }
                    }
                    main_window->editor->setVisibility(GConst::cellRTTI);
                }
                if (!main_window->geometry_modified) {
                    main_window->geometry_modified = 2;
                }
#endif
            }
        } else if (strcmp(command, "set_subnet") == 0) {
            parse_command(command_opt, "traffic_type subnet arrival_rate strid", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "traffic_type",     cmd_optlist[0]);
            require_param(command, "subnet",           cmd_optlist[1]);
            check_param_value(traffic, command, "arrival_rate", cmd_optlist[2], 0.0, 1.0e10, 0.0);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
            }
            if (!error_state) {
                subnet_idx = get_subnet_idx(cmd_optlist[1], traffic_type_idx, 1);
            }

            if ((!error_state)&&(cmd_optlist[2])) {
                subnet_list[traffic_type_idx][subnet_idx]->arrival_rate = traffic;
                GUI_FN(main_window->geometry_modified = 1);
            }
            if ((!error_state)&&(cmd_optlist[3])) {
                for (i=0; (i<=num_subnet[traffic_type_idx]-1)&&(!error_state); i++) {
                    if ((i != subnet_idx) && (subnet_list[traffic_type_idx][i]->strid)
                                   && (strcmp(subnet_list[traffic_type_idx][i]->strid,cmd_optlist[3])==0)) {
                        sprintf(msg, "ERROR: Subnet already exists with name \"%s\"\n", cmd_optlist[3]);
                        PRMSG(stdout, msg); error_state = 1;
                    }
                }
                if (!error_state) {
                    free(subnet_list[traffic_type_idx][subnet_idx]->strid);
                    subnet_list[traffic_type_idx][subnet_idx]->strid = strdup(cmd_optlist[3]);
#if HAS_GUI
                    if ( (use_gui) && (!error_state) ) {
                        vlist = main_window->visibility_window->visibility_list;
                        vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                        vi = (VisibilityItem *) VisibilityList::findItem(vi,  GConst::subnetRTTI, traffic_type_idx);
                        vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::subnetRTTI, subnet_idx);
                        vci->setText(0, subnet_list[traffic_type_idx][subnet_idx]->strid);
                        vlist->sort();
                        main_window->geometry_modified = 1;
                    }
#endif
                }
            }
        } else if (strcmp(command, "set_unused_freq") == 0) {
            parse_command(command_opt, "sector freq_list", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "sector", cmd_optlist[0]);
            require_param(command, "freq_list", cmd_optlist[1]);
            changed_bit_cell = 0;
            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                    changed_bit_cell = 1;
                }
                extract_sector_list(int_list, cmd_optlist[0], CConst::CellIdxRef);
            }
            if (!error_state) {
                if (int_list->getSize() != 1) {
                    sprintf(msg, "ERROR: Invalid sector specification\n");
                    PRMSG(stdout, msg); error_state = 1;
                } else {
                    cell_idx   = (*int_list)[0] & ((1<<bit_cell)-1);
                    sector_idx = (*int_list)[0] >> bit_cell;
                    sector = cell_list[cell_idx]->sector_list[sector_idx];
                }
            }
            if (!error_state) {
                extract_int_list(int_list, cmd_optlist[1]);
            }
            if (!error_state) {
                ((PHSSectorClass *) sector)->set_unused_freq(int_list);
            }
            if (changed_bit_cell) {
                bit_cell = -1;
            }
        } else if (strcmp(command, "set_sector_meas_ctr") == 0) {
            parse_command(command_opt, "sector traffic_type meas_ctr gw_csc_cs", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "traffic_type",     cmd_optlist[1]);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[1], 1);
            }
            require_param(command, "meas_ctr",         cmd_optlist[2]);
            check_param_value(traffic, command, "meas_ctr", cmd_optlist[2], 0.0, 1.0e10, 0.0);
            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                }
                if ( (!cmd_optlist[0]) && (!cmd_optlist[3]) ) {
                    sprintf(msg, "ERROR: Either sector or gw_csc_cs must be specified\n");
                    PRMSG(stdout, msg); error_state = 1;
                } else if (cmd_optlist[0]) {
                    extract_sector_list(int_list, cmd_optlist[0], CConst::CellIdxRef);
                } else {
                    extract_int_list(int_list, cmd_optlist[3]);
                    for (i=int_list->getSize()-1; i>=0; i--) {
                        uid_to_sector((*int_list)[i], cell_idx, sector_idx);
                        if (cell_idx == -1) {
                            sprintf(msg, "WARNING: GW_CSC_CS = %.6d not found\n", (*int_list)[i]);
                            PRMSG(stdout, msg); warning_state = 1;
                            int_list->del_elem_idx(i);
                        } else {
                            (*int_list)[i] = (sector_idx << bit_cell) | cell_idx;
                        }
                    }
                }
            }

            if (!error_state) {
                found = 0;
                for (i=0; (i<=SectorClass::num_traffic-1)&&(!found); i++) {
                    if (SectorClass::traffic_type_idx_list[i] == traffic_type_idx) {
                        tt_idx = i;
                        found = 1;
                    }
                }
                if (!found) {
                    sprintf(msg, "ERROR: Unable to set sector traffic\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }

            if (!error_state) {
                for (i=0; i<=int_list->getSize()-1; i++) {
                    cell_idx   = (*int_list)[i] & ((1<<bit_cell)-1);
                    sector_idx = (*int_list)[i] >> bit_cell;
                    sector = cell_list[cell_idx]->sector_list[sector_idx];
                    sector->meas_ctr_list[tt_idx] = traffic;
                }
            }
            bit_cell = -1;
            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "set_road_test_data") == 0) {
            parse_command(command_opt, "thr_list", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "thr_list", cmd_optlist[0]);
            if (!error_state) {
                extract_double_list(dbl_list, cmd_optlist[0]);
            }
            if (!error_state) {
                if (dbl_list->getSize() == 0) {
                    sprintf(msg, "ERROR: No Threshold Levels Specified\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                prev_num_level = RoadTestPtClass::num_level;
                RoadTestPtClass::num_level = dbl_list->getSize();
                RoadTestPtClass::level_list = (double *) realloc( (void *) RoadTestPtClass::level_list, RoadTestPtClass::num_level*sizeof(double) );
                RoadTestPtClass::color_list = (int    *) realloc( (void *) RoadTestPtClass::color_list, (RoadTestPtClass::num_level+1)*sizeof(int) );
                for (i=0; i<=RoadTestPtClass::num_level-1; i++) {
                    RoadTestPtClass::level_list[i] = (*dbl_list)[i];
                }
                if (RoadTestPtClass::num_level != prev_num_level) {
                    for (i=0; i<=RoadTestPtClass::num_level; i++) {
                        RoadTestPtClass::color_list[i] = hot_color->get_color(i, RoadTestPtClass::num_level+1);
                    }
                }
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);
                if (vi) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vi, GConst::roadTestDataRTTI, 1);
                }
                if (vi) {
                    for (i=RoadTestPtClass::num_level+1; i<=prev_num_level; i++) {
                        vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::roadTestDataRTTI, i);
                        delete vci;
                    }
                    for (i=prev_num_level+1; i<=RoadTestPtClass::num_level; i++) {
                        new VisibilityCheckItem( vi, GConst::roadTestDataRTTI, i, "", RoadTestPtClass::color_list[i]);
                    }
                }

                main_window->visibility_window->visibility_list->update_rtd(0);
            }
#endif
        } else if (strcmp(command, "set_csid_byte_length") == 0) {
            parse_command(command_opt, "length", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            require_param(command, "length", cmd_optlist[0]);
            check_param_value(n,     command, "length",     cmd_optlist[0], 1, 50, 10);
            if (!error_state) {
                if ( (mode == CConst::editGeomMode) && (num_cell != 0) ) {
                    sprintf(msg, "ERROR: set_csid_byte_length can only be run in NO_GEOM mode or in EDIT mode when there are no cells in the system\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                PHSSectorClass::csid_byte_length = n;
                sprintf(msg, "CSID byte length set to %d (%d HEX digits)\n", n, 2*n);
                PRMSG(stdout, msg);
            }
        } else if (strcmp(command, "add_cell") == 0) {
            parse_command(command_opt, "posn_x posn_y num_sector lon_lat-", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "posn_x", cmd_optlist[0]);
            require_param(command, "posn_y", cmd_optlist[1]);
            if (cmd_optlist[3]) {
                check_param_value(lon,     command, "posn_x",     cmd_optlist[0], -360.0, 360.0, 0.0);
                check_param_value(lat,     command, "posn_y",     cmd_optlist[1], -180.0, 180.0, 0.0);
                if (coordinate_system == CConst::CoordUTM) {
                    LLtoUTM( lon, lat, posn_x,  posn_y, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);
                } else {
                    sprintf(msg, "ERROR: Coordinate system has no translation to LON/LAT\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            } else {
                check_param_value(posn_x,     command, "posn_x",     cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
                check_param_value(posn_y,     command, "posn_y",     cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            }
            check_param_value(num_sector, command, "num_sector", cmd_optlist[2], 1, 6, 1);

            if (!error_state) {
                num_cell++;
                cell_list = (CellClass **) realloc( (void *) cell_list, num_cell*sizeof(CellClass *));
                switch(technology()) {
                    case CConst::PHS:      cell_list[num_cell-1] = (CellClass *) new PHSCellClass(num_sector);      break;
                    case CConst::WCDMA:    cell_list[num_cell-1] = (CellClass *) new WCDMACellClass(num_sector);    break;
                    case CConst::CDMA2000: cell_list[num_cell-1] = (CellClass *) new CDMA2000CellClass(num_sector); break;
                    case CConst::WLAN:     cell_list[num_cell-1] = (CellClass *) new WLANCellClass(num_sector);     break;
                    default: CORE_DUMP; break;
                }
                cell = cell_list[num_cell-1];
                check_grid_val(posn_x, resolution, system_startx, &cell->posn_x);
                check_grid_val(posn_y, resolution, system_starty, &cell->posn_y);

                if (!system_bdy->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    sprintf(msg, "WARNING: Cell %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                        num_cell-1, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
                    PRMSG(stdout, msg); warning_state = 1;
                }
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                new GCellClass(main_window->editor, num_cell-1, cell_list[num_cell-1]);
                main_window->editor->setVisibility(GConst::cellTextRTTI);

                vlist = main_window->visibility_window->visibility_list;
                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                cell = cell_list[num_cell-1];
                vci = new VisibilityCheckItem( cellViewItem, GConst::cellRTTI, num_cell-1, cell->view_name(num_cell-1, preferences->vw_cell_name_pref), cell->color);
                vci->setOn(true);

                main_window->canvas->update();
                main_window->geometry_modified = 1;
            }
#endif

        } else if (strcmp(command, "copy_cell") == 0) {
            parse_command(command_opt, "cell posn_x posn_y", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "cell",   cmd_optlist[0]);
            require_param(command, "posn_x", cmd_optlist[1]);
            require_param(command, "posn_y", cmd_optlist[2]);

            check_param_value(cell_idx,   command, "cell",       cmd_optlist[0],   0, num_cell-1, 0);
            check_param_value(posn_x,     command, "posn_x",     cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y,     command, "posn_y",     cmd_optlist[2], -1.0e100, 1.0e100, 0.0);

            if (!error_state) {
                cell = cell_list[cell_idx]->duplicate(0, 0, 0);
                num_cell++;
                cell_list = (CellClass **) realloc( (void *) cell_list, num_cell*sizeof(CellClass *));
                cell_list[num_cell-1] = cell;
                check_grid_val(posn_x, resolution, system_startx, &cell->posn_x);
                check_grid_val(posn_y, resolution, system_starty, &cell->posn_y);
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                new GCellClass(main_window->editor, num_cell-1, cell_list[num_cell-1]);
                main_window->editor->setVisibility(GConst::cellTextRTTI);

                vlist = main_window->visibility_window->visibility_list;
                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                cell = cell_list[num_cell-1];
                vci = new VisibilityCheckItem( cellViewItem, GConst::cellRTTI, num_cell-1, cell->view_name(num_cell-1, preferences->vw_cell_name_pref), cell->color);
                vci->setOn(true);

                main_window->canvas->update();
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "add_polygon") == 0) {
            int type_idx;
            parse_command(command_opt, "type traffic_type gptlist subnet_idx subnet_strid", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            static const char *type_str[] = { "subnet", "system_bdy", 0};
            require_param(command, "type",    cmd_optlist[0]);
            check_param_value(type_idx, command, "type", cmd_optlist[0], type_str, 0);
            if (type_idx == 0) {
                require_param(command, "gptlist", cmd_optlist[2]);
                require_param(command, "traffic_type", cmd_optlist[1]);
                if (!error_state) {
                    traffic_type_idx = get_traffic_type_idx(cmd_optlist[1], 1);
                }
            } else {
                if ( cmd_optlist[1] && !cmd_optlist[2] &&  (cmd_optlist[3] || cmd_optlist[4]) ) {
                    traffic_type_idx = get_traffic_type_idx(cmd_optlist[1], 1);
                    if (!error_state) {
                        if (!cmd_optlist[3]) {
                            subnet_idx = get_subnet_idx(cmd_optlist[4], traffic_type_idx, 1);
                        } else {
                            check_param_value(subnet_idx, command, "subnet_idx", cmd_optlist[3], 0, num_subnet[traffic_type_idx]-1, 0);
                        }
                    }
                } else if ( !cmd_optlist[1] &&  cmd_optlist[2] && !cmd_optlist[3] && !cmd_optlist[4]) {
                    // do nothing
                } else {
                    sprintf(msg, "ERROR: To redefine system boundary, must specify either gptlist or traffic_type_idx and subnet_idx\n");
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (cmd_optlist[2]) {
                extract_intint_list(ii_list, cmd_optlist[2]);
            } else if (!error_state) {
                poly = subnet_list[traffic_type_idx][subnet_idx]->p;
                max_val = PolygonClass::comp_bdy_area(poly->num_bdy_pt[0], poly->bdy_pt_x[0], poly->bdy_pt_y[0]);
                i = 0;
                for (segment_idx=1; segment_idx<=poly->num_segment-1; segment_idx++) {
                    area = PolygonClass::comp_bdy_area(poly->num_bdy_pt[segment_idx], poly->bdy_pt_x[segment_idx], poly->bdy_pt_y[segment_idx]);
                    if ( area > max_val) {
                        max_val = area;
                        i = segment_idx;
                    }
                }
                for (pt_idx=0; pt_idx<=poly->num_bdy_pt[i]-1; pt_idx++) {
                    ii_list->append(IntIntClass(poly->bdy_pt_x[i][pt_idx], poly->bdy_pt_y[i][pt_idx]));
                }

#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    vlist = main_window->visibility_window->visibility_list;

                    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                        vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                        vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
                        for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
                            delete vci;
                        }
                        free(main_window->visibility_window->vec_vis_subnet[traffic_type_idx]);
                        main_window->visibility_window->vec_vis_subnet[traffic_type_idx] = (char *) NULL;
                    }
                }
#endif
                if (!error_state) {
                    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                        for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                            delete subnet_list[traffic_type_idx][subnet_idx];
                        }
                        if (num_subnet[traffic_type_idx]) {
                            free(subnet_list[traffic_type_idx]);
                        }
                        num_subnet[traffic_type_idx] = 0;
                    }
                }
            }

            if (!error_state) {
                dx = system_startx;
                dy = system_starty;
                add_polygon(cmd_optlist[0], traffic_type_idx, ii_list);
                dx = system_startx - dx;
                dy = system_starty - dy;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                if (type_idx == 0) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                    vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
                    subnet = subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1];
                    vci = new VisibilityCheckItem( vi, GConst::subnetRTTI, num_subnet[traffic_type_idx]-1, subnet->strid, subnet->color);
                    main_window->editor->setVisibility(GConst::subnetRTTI);
                    vci->setOn(TRUE);
                } else if (type_idx == 1) {
                    main_window->editor->scrollCanvas(dx, dy);
                } else {
                    CORE_DUMP;
                }
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "filter_system_bdy") == 0) {
            parse_command(command_opt, "cell- map_layer_idx", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if ((!error_state) && cmd_optlist[0]) {
                int_list->reset();
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    if (!system_bdy->in_bdy_area(cell_list[cell_idx]->posn_x, cell_list[cell_idx]->posn_y)) {
                        int_list->append(cell_idx);
                    }
                }
                delete_cell(int_list);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::cellRTTI);
                main_window->editor->setVisibility(GConst::cellTextRTTI);
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
                main_window->editor->scrollCanvas(0, 0);
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "move_cell") == 0) {
            parse_command(command_opt, "cell_idx posn_x posn_y", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "cell_idx", cmd_optlist[0]);
            require_param(command, "posn_x",   cmd_optlist[1]);
            require_param(command, "posn_y",   cmd_optlist[2]);
            check_param_value(cell_idx,   command, "cell_idx",   cmd_optlist[0],         0, num_cell-1, 0);
            check_param_value(posn_x,     command, "posn_x",     cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y,     command, "posn_y",     cmd_optlist[2], -1.0e100, 1.0e100, 0.0);

            if (!error_state) {
                cell = cell_list[cell_idx];
                check_grid_val(posn_x, resolution, system_startx, &cell->posn_x);
                check_grid_val(posn_y, resolution, system_starty, &cell->posn_y);
                if (!system_bdy->in_bdy_area(cell->posn_x, cell->posn_y)) {
                    sprintf(msg, "WARNING: Cell %d (%12.7f, %12.7f) is not contained in the system boundary area\n",
                        cell_idx, idx_to_x(cell->posn_x), idx_to_y(cell->posn_y));
                    PRMSG(stdout, msg); warning_state = 1;
                }
            }

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::cellRTTI);
                main_window->editor->setVisibility(GConst::cellTextRTTI);
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "delete_cell") == 0) {
            parse_command(command_opt, "cell_idx", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "cell_idx", cmd_optlist[0]);
            if (!error_state) {
                extract_cell_list(int_list, cmd_optlist[0]);
            }

            delete_cell(int_list);

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::cellRTTI);
                main_window->editor->setVisibility(GConst::cellTextRTTI);
                main_window->editor->setVisibility(GConst::roadTestDataRTTI);
                main_window->canvas->update();
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "display_settings") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, all_mode_valid);
            fp = (FILE *) NULL;
            if (!error_state) {
                if (cmd_optlist[0] == NULL) {
                    fp = stdout;
                } else if ( !(fp = fopen(cmd_optlist[0], "w")) ) {
                    sprintf(msg, "ERROR: Unable to write to file %s\n", cmd_optlist[0]);
                    PRMSG(stdout, msg); error_state = 1;
                }
            }

            if (!error_state) {
                display_settings(fp);
            }

            if ( (fp) && (fp != stdout) ) {
                fclose(fp);
            }
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "print_call_status") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            fp = (FILE *) NULL;
            if (!error_state) {
                if (cmd_optlist[0] == NULL) {
                    fp = stdout;
                } else if ( !(fp = fopen(cmd_optlist[0], "w")) ) {
                    sprintf(msg, "ERROR: Unable to write to file %s\n", cmd_optlist[0]);
                    PRMSG(stdout, msg); error_state = 1;
                }
            }
            if (!error_state) {
                print_call_status(fp);
            }
            if ( (fp) && (fp != stdout) ) {
                fclose(fp);
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "plot_num_comm") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);
            if (start) {
                require_param(command, "f", cmd_optlist[1]);
            }

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_num_comm == 0) {
                        if ( (stat->fp_num_comm = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_num_comm = 1;
                            // GUI_FN(main_window->run_simulation_dia->num_comm_sim_dia->plot_num_comm_stat_on());
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_num_comm) {
                        stat->plot_num_comm = 0;
                        fprintf(stat->fp_num_comm, "\n");
                        fclose(stat->fp_num_comm);
                        // GUI_FN(main_window->run_simulation_dia->num_comm_sim_dia->plot_num_comm_stat_off());
                    }
                }
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "plot_event") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[1]);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_event == 0) {
                        if ( (stat->fp_event = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_event = 1;
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_event) {
                        stat->plot_event = 0;
                        fclose(stat->fp_event);
                    }
                }
            }
        } else if (strcmp(command, "plot_throughput") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[1]);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_throughput == 0) {
                        if ( (stat->fp_throughput = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_throughput = 1;
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_throughput) {
                        stat->plot_throughput = 0;
                        fclose(stat->fp_throughput);
                    }
                }
            }
        } else if (strcmp(command, "plot_delay") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[1]);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_delay == 0) {
                        if ( (stat->fp_delay = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_delay = 1;
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_delay) {
                        stat->plot_delay = 0;
                        fclose(stat->fp_delay);
                    }
                }
            }
        } else if (strcmp(command, "plot_jitter") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[1]);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_jitter == 0) {
                        if ( (stat->fp_jitter = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_jitter = 1;
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_jitter) {
                        stat->plot_jitter = 0;
                        fclose(stat->fp_jitter);
                    }
                }
            }
        } else if (strcmp(command, "plot_pkt_loss_rate") == 0) {
            parse_command(command_opt, "action f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[1]);
            check_param_value(start, command, "action", cmd_optlist[0], action_str, 1);

            if (!error_state) {
                if (start == 1) {
                    if (stat->plot_pkt_loss_rate == 0) {
                        if ( (stat->fp_pkt_loss_rate = fopen(cmd_optlist[1], "w")) ) {
                            stat->plot_pkt_loss_rate = 1;
                        } else {
                            sprintf(msg, "ERROR writing to file %s\n", cmd_optlist[1]);
                            PRMSG(stdout, msg); error_state = 1;
                        }
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->plot_pkt_loss_rate) {
                        stat->plot_pkt_loss_rate = 0;
                        fclose(stat->fp_pkt_loss_rate);
                    }
                }
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "measure_crr") == 0) {
            parse_command(command_opt, "action min_crr max_crr num_crr", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            check_param_value(start,   command, "action",  cmd_optlist[0], action_str, 1);
            check_param_value(min_val, command, "min_crr", cmd_optlist[1], 0.0, 100.0, 1.0);
            check_param_value(max_val, command, "max_crr", cmd_optlist[2], 0.0, 100.0, 1.0);
            check_param_value(n,       command, "num_crr", cmd_optlist[3],   2, 10000, 100);

            if (!error_state) {
                if (start == 1) {
                    if ( (!stat->measure_crr) && (min_val < max_val) && (n > 0) ) {
                        stat->measure_crr = 1;
                        stat->min_crr = min_val;
                        stat->max_crr = max_val;
                        stat->num_crr = n;
                        stat->p_crr = DVECTOR(n);
                        for (i=0; i<=n-1; i++) {
                            stat->p_crr[i] = 0.0;
                        }
                        stat->n_minus_1_over_max_minus_min_crr = (n-1)/(max_val-min_val);
                        // GUI_FN(main_window->run_simulation_dia->crr_sim_dia->plot_crr_stat_on());
                    } else {
                        sprintf(msg, "ERROR: Illegal command\n");
                        PRMSG(stdout, msg); error_state = 1;
                    }
                } else {
                    if (stat->measure_crr) {
                        stat->measure_crr = 0;
                        free(stat->p_crr);
                        // GUI_FN(main_window->run_simulation_dia->crr_sim_dia->plot_crr_stat_off());
                    }
                }
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "plot_crr_cdf") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!error_state) {
                plot_crr_cdf(cmd_optlist[0]);
            }
#endif
        } else if (strcmp(command, "create_clutter_map") == 0) {
            parse_command(command_opt, "map_sim_res_ratio num_clutter_type", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "map_sim_res_ratio", cmd_optlist[0]);
            check_param_value(map_sim_res_ratio, command, "map_sim_res_ratio", cmd_optlist[0],  0, 10000,        1);
            check_param_value(num_clutter_type,  command, "num_clutter_type",  cmd_optlist[1], -1, 2000000000,  -1);
            if (map_clutter) {
                sprintf(msg, "ERROR: map clutter already defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                map_clutter = new MapClutterClass();
                dx = system_startx;
                dy = system_starty;
                map_clutter->create_clutter_map(this, map_sim_res_ratio, num_clutter_type);
                dx = system_startx - dx;
                dy = system_starty - dy;

            }
            if (error_state) {
                delete map_clutter;
                map_clutter = (MapClutterClass *) NULL;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                mc = map_clutter;

                VisibilityItem *mapClutterViewItem = new VisibilityItem(  vlist, GConst::mapClutterRTTI, 0, qApp->translate("VisibilityWindow", "Map Clutter"));
                for (clutter_type_idx=0; clutter_type_idx<=map_clutter->num_clutter_type-1; clutter_type_idx++) {
                    new VisibilityCheckItem( mapClutterViewItem, GConst::mapClutterRTTI, clutter_type_idx, mc->description[clutter_type_idx], mc->color[clutter_type_idx]);
                }
                main_window->editor->scrollCanvas(dx, dy);
            }
#endif
        } else if (strcmp(command, "set_clutter_type") == 0) {
            parse_command(command_opt, "clutter_type_idx gptlist", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "clutter_type_idx", cmd_optlist[1]);
            require_param(command, "gptlist", cmd_optlist[1]);
            if (!map_clutter) {
                sprintf(msg, "ERROR: no map clutter defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                check_param_value(clutter_type_idx,    command, "clutter_type_idx", cmd_optlist[0], 0, map_clutter->num_clutter_type-1, 0);
            }
            extract_intint_list(ii_list, cmd_optlist[1]);
            if (!error_state) {
                map_clutter->set_clutter_type(clutter_type_idx, ii_list);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapClutterRTTI);
            }
#endif
        } else if (strcmp(command, "set_init_clutter_type") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            if (!map_clutter) {
                sprintf(msg, "ERROR: no map clutter defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                map_clutter->set_init_clutter_type();
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapClutterRTTI);
            }
#endif
        } else if (strcmp(command, "shift_clutter_map") == 0) {
            parse_command(command_opt, "x y", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            check_param_value(posn_x, command, "x", cmd_optlist[0], -1.0e100, 1.0e100, 0.0);
            check_param_value(posn_y, command, "y", cmd_optlist[1], -1.0e100, 1.0e100, 0.0);
            if (!error_state) {
                check_grid_val(posn_x, resolution, 0, &dx);
                check_grid_val(posn_y, resolution, 0, &dy);
                map_clutter->offset_x += dx;
                map_clutter->offset_y += dy;
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapClutterRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "refine_clutter_model") == 0) {
            parse_command(command_opt, "n pm_idx", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            check_param_value(n, command, "n", cmd_optlist[0], 1, 10, 1);
            check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[1],         0, num_prop_model-1, 0);

            BITWIDTH(bit_cell, num_cell-1);
            extract_sector_list(int_list, "all", CConst::CellIdxRef);

            if (!error_state) {
                if (prop_model_list[pm_idx]->is_clutter_model()) {
                    ((GenericClutterPropModelClass *) prop_model_list[pm_idx])->refine_clutter(this, n, int_list);
                } else {
                    sprintf(msg, "ERROR: Specified propagation model is not a clutter model\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                }
            }

            bit_cell = -1;

#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->geometry_modified = 1;
                main_window->editor->setVisibility(GConst::clutterPropModelRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "shift_clutter_model") == 0) {
            parse_command(command_opt, "x y pm_idx", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            check_param_value(posn_x, command, "x",      cmd_optlist[0], -1.0e100,          1.0e100, 0.0);
            check_param_value(posn_y, command, "y",      cmd_optlist[1], -1.0e100,          1.0e100, 0.0);
            check_param_value(pm_idx, command, "pm_idx", cmd_optlist[2],        0, num_prop_model-1,   0);
            if (!error_state) {
                check_grid_val(posn_x, resolution, 0, &dx);
                check_grid_val(posn_y, resolution, 0, &dy);
                if (prop_model_list[pm_idx]->is_clutter_model()) {
                    ((GenericClutterPropModelClass *) prop_model_list[pm_idx])->offset_x += dx;
                    ((GenericClutterPropModelClass *) prop_model_list[pm_idx])->offset_y += dy;
                } else {
                    sprintf(msg, "ERROR: Specified propagation model is not a clutter model\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                }
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::mapClutterRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "export_clutter_model") == 0) {
            parse_command(command_opt, "pm_idx f", cmd_optlist);
            check_param_value(pm_idx, command, "pm_idx", cmd_optlist[0],        0, num_prop_model-1,   0);
            require_param(command, "f", cmd_optlist[1]);
            if (!error_state) {
                if (prop_model_list[pm_idx]->is_clutter_model()) {
                    ((GenericClutterPropModelClass *) prop_model_list[pm_idx])->save(this, cmd_optlist[1]);
                } else {
                    sprintf(msg, "ERROR: Specified propagation model is not a clutter model\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                }
            }
        } else if (strcmp(command, "import_clutter_model") == 0) {
            parse_command(command_opt, "f force-", cmd_optlist);
            check_valid_mode(command, ne_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            add_geometry = (system_bdy ? 0 : 1);
            if (!error_state) {
                pm_idx = read_clutter_model(cmd_optlist[0], (cmd_optlist[1] ? 1 : 0));
            }
            if ((!error_state)&&(add_geometry)) {
                mode = CConst::editGeomMode;
                process_mode_change();
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                if (add_geometry) {
                main_window->editor->geometry_filename = QString();

                new VisibilityCheckItem( vlist, GConst::systemBoundaryRTTI, 0, qApp->translate("VisibilityWindow", "System Boundary"), NetworkClass::system_bdy_color );
                cellViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
                if (!cellViewItem) {
                    cellViewItem = new VisibilityItem( vlist,     GConst::cellRTTI, 0, qApp->translate("VisibilityWindow", "Cell"));
                }

#if HAS_MONTE_CARLO
                VisibilityItem *trafficViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::trafficRTTI, 0);
                if (!trafficViewItem) {
                    trafficViewItem = new VisibilityItem( vlist,     GConst::trafficRTTI, 0, qApp->translate("VisibilityWindow", "Traffic"));
                }
                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    traffic_type = traffic_type_list[traffic_type_idx];
                    new VisibilityCheckItem( trafficViewItem, GConst::trafficRTTI, traffic_type_idx, traffic_type->name(), traffic_type->color);
                }
#endif

                VisibilityItem *cellTextViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellTextRTTI, 0);
                if (!cellTextViewItem) {
                    cellTextViewItem = new VisibilityItem(  vlist, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Text"), CellClass::text_color);
                }

                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 0, qApp->translate("VisibilityWindow", "Cell Index")     );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 1, qApp->translate("VisibilityWindow", "CSID")           );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 2, qApp->translate("VisibilityWindow", "GW_CSC_CS")      );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 3, qApp->translate("VisibilityWindow", "Antenna Height") );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 4, qApp->translate("VisibilityWindow", "Paging Area")    );
                new VisibilityCheckItem( cellTextViewItem, GConst::cellTextRTTI, 5, qApp->translate("VisibilityWindow", "Propagation Model"));

                if (!main_window->view_menu_is_checked(GConst::visibilityWindow)) {
                    main_window->visibility_window->setGeometry(200, 200, (int) floor(vlist->header()->sectionPos(vlist->header()->mapToIndex(1))*0.7), 400);
                }
                main_window->toggle_visibility_window(GConst::visShow);
                main_window->toggle_info_window(GConst::visShow);
                qApp->processEvents();
                main_window->editor->zoomToFit();
                }


                main_window->visibility_window->visibility_list->update_cpm();
                vi  = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::clutterPropModelRTTI, 0);
                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi, GConst::clutterPropModelRTTI, pm_idx);
                vci->setOn(true);
                main_window->visibility_window->resize();
                main_window->editor->setVisibility(GConst::clutterPropModelRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "set_clutter_coeff") == 0) {
            int clutter_idx;
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {

                parse_command(command_opt, "pm_idx clutter_idx val", cmd_optlist);
                check_valid_mode(command, e_mode_valid);
                check_param_value(pm_idx, command, "pm_idx", cmd_optlist[0], 0, num_prop_model-1, 0);

                GenericClutterPropModelClass *pm =  (GenericClutterPropModelClass *) prop_model_list[pm_idx];
                if (pm->is_clutter_model()) {
                    pm->offset_x += dx;
                    pm->offset_y += dy;
                } else {
                    sprintf(msg, "ERROR: Specified propagation model is not a clutter model\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                }
                check_param_value(clutter_idx, command, "map_clutter_idx", cmd_optlist[1], 0, pm->num_clutter_type-1, 0);

                if (pm->type() == CConst::PropClutterFull) {
                    pm->mvec_x[2*clutter_idx + pm->useheight];
                    check_param_value(pm->mvec_x[2*clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                } else if (pm->type() == CConst::PropClutterSymFull) {
                    check_param_value(pm->mvec_x[2*clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                } else if (pm->type() == CConst::PropClutterWtExpo) {
                    check_param_value(pm->mvec_x[2*clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                } else if (pm->type() == CConst::PropClutterWtExpoSlope) {
                    check_param_value(pm->mvec_x[clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                } else if (pm->type() == CConst::PropClutterExpoLinear) {
                    check_param_value(pm->mvec_x[clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                } else if (pm->type() == CConst::PropClutterGlobal) {
                    check_param_value(pm->mvec_x[clutter_idx + pm->useheight], command, "coeffecient_value",
                            cmd_optlist[2], -1.0e100, 1.0e100, 0.0);
                }

                //main_window->editor->setVisibility(GConst::mapClutterRTTI);
                //main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "set_csid_format") == 0) {
            parse_command(command_opt, "fmt", cmd_optlist);
            int prev_csid_format = ((PHSNetworkClass *) this)->csid_format;

            check_param_value(((PHSNetworkClass *) this)->csid_format, command, "csid_format", cmd_optlist[0], 0, ((PHSNetworkClass*)this)->num_csid_format-1, 0);
            if (!error_state) {

                // not show this info in command windows
                // - 8/22/2008 CG
                sprintf(msg, "CSID Format set to %s\n",
                        (((PHSNetworkClass *) this)->csid_format==CConst::CSID19NP)?"19 bits PA type":"16 bits PA type");
                PRMSG(stdout, msg);

                // Don't set geometry_modified flag
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) && (((PHSNetworkClass *) this)->csid_format != prev_csid_format) ) {
                for (cvg_idx=0; cvg_idx<=num_coverage_analysis-1; cvg_idx++) {
                    if (coverage_list[cvg_idx]->type == CConst::pagingAreaCoverage) {
                        main_window->visibility_window->visibility_list->update_cvg_analysis(cvg_idx);
                    }
                }
            }
#endif
        } else if (strcmp(command, "import_st_data") == 0) {
            parse_command(command_opt, "fmt fcsc f p", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "fcsc", cmd_optlist[1]);
            require_param(command, "f", cmd_optlist[2]);
            check_param_value(n, command, "fmt", cmd_optlist[0], cs_type_str, 0);
            int period;
            check_param_value(period, command, "p", cmd_optlist[3], 1, 24, 22);

            if (!error_state) {
                import_st_data(n, cmd_optlist[1], cmd_optlist[2], period);
            }
            GUI_FN(main_window->geometry_modified = 1);
        } else if (strcmp(command, "delete_st_data") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            if (!error_state) {
                delete_st_data();
            }
            GUI_FN(main_window->geometry_modified = 1);
#if 0 // xxxxxxxxxxxx DELETE
        } else if (strcmp(command, "comp_map_clutter") == 0) {
            parse_command(command_opt, "map_sim_res_ratio num_clutter_type", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            require_param(command, "map_sim_res_ratio", cmd_optlist[0]);
            require_param(command, "num_clutter_type",  cmd_optlist[1]);
            check_param_value(map_sim_res_ratio, command, "map_sim_res_ratio", cmd_optlist[0], 0, 10000, 1);
            check_param_value(num_clutter_type,  command, "num_clutter_type",  cmd_optlist[1], 0, 254,   1);
            if (map_clutter) {
                sprintf(msg, "ERROR: map clutter already defined\n");
                PRMSG(stdout, msg); error_state = 1;
            }
            if (!error_state) {
                map_clutter = new MapClutterClass();
                map_clutter->comp_map_clutter(this, map_sim_res_ratio, num_clutter_type);
            }
#if 0 // xxxxxxxxxxxx HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;
                mc = map_clutter;

                VisibilityItem *mapClutterViewItem = new VisibilityItem(  vlist, GConst::mapClutterRTTI, 0, qApp->translate("VisibilityWindow", "Map Clutter"));
                for (clutter_type_idx=0; clutter_type_idx<=map_clutter->num_clutter_type-1; clutter_type_idx++) {
                    new VisibilityCheckItem( mapClutterViewItem, GConst::mapClutterRTTI, clutter_type_idx, mc->description[clutter_type_idx], mc->color[clutter_type_idx]);
                }
            }
#endif
#endif

        } else if (strcmp(command, "check_antenna_gain") == 0) {
            parse_command(command_opt, "f antenna numpts orient", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "f",      cmd_optlist[0]);

            static const char *orient_str[] = { "H", "V", 0};
            check_param_value(ant_idx,    command, "antenna", cmd_optlist[1], 0, num_antenna_type-1, 0);
            check_param_value(numpts,     command, "numpts",  cmd_optlist[2], 3, 10000, 1000);
            check_param_value(orient,     command, "orient",  cmd_optlist[3], orient_str, 0);

            if (!error_state) {
                antenna = antenna_type_list[ant_idx];
                if (!antenna->checkGain(cmd_optlist[0], orient, numpts)) {
                    error_state = 1;
                }
            }
        } else if (strcmp(command, "expand_cch_rssi_table") == 0) {
            parse_command(command_opt, "threshold_db", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "threshold_db", cmd_optlist[0]);

            check_param_value(threshold_db, command, "threshold_db", cmd_optlist[0], -200.0, 200.0, 0.0);

            if (!error_state) {
                ((PHSNetworkClass *)this)->expand_cch_rssi_table(threshold_db);
            }
        } else if (strcmp(command, "set_sync_level") == 0) {
            parse_command(command_opt, "level sectors", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "sectors", cmd_optlist[1]);

            int sync_level;
            check_param_value(sync_level, command, "level", cmd_optlist[0], -1, 100, -1);

            if (num_cell) {
                BITWIDTH(bit_cell, num_cell-1);
            }

            if (!error_state) {
                extract_sector_list(int_list, cmd_optlist[1], CConst::CellIdxRef);
            }
            if (!error_state) {
                for (i=0; i<=int_list->getSize()-1; i++) {
                    cell_idx   = (*int_list)[i] & ((1<<bit_cell)-1);
                    sector_idx = (*int_list)[i] >> bit_cell;
                    sector = cell_list[cell_idx]->sector_list[sector_idx];
                    ((PHSSectorClass *) sector)->sync_level = sync_level;
                }
            }
            bit_cell = -1;
        } else if (strcmp(command, "print_sync_state") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            if (!error_state) {
                ((PHSNetworkClass *)this)->print_sync_state(cmd_optlist[0]);
            }
        } else if (strcmp(command, "run_system_sync") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            if (!error_state) {
                ((PHSNetworkClass *)this)->run_system_sync();
            }
        } else if (strcmp(command, "group") == 0) {
            parse_command(command_opt, "sectors name gw_csc_cs by_paging_area- by_csc-", cmd_optlist);
            check_valid_mode(command, s_mode_valid);

            if ( (!cmd_optlist[0]) && (!cmd_optlist[2]) && (!cmd_optlist[3]) && (!cmd_optlist[4]) ) {
                sprintf(msg, "ERROR: Either sectors, gw_csc_cs, by_paging_area, or by_csc must be specified\n");
                PRMSG(stdout, msg); error_state = 1;
            }

            if (!error_state) {
                if ((cmd_optlist[3]) || (cmd_optlist[4])) {
                    ((PHSNetworkClass *) this)->group_by_csid_field( (cmd_optlist[3] ? CConst::CSIDPagingArea : CConst::CSIDCSC));
                } else {
                    process_group_sector_cmd(cmd_optlist[0], cmd_optlist[2], cmd_optlist[1]);
                }
            }
        } else if (strcmp(command, "delete_all_groups") == 0) {
            for (grp_idx=0; grp_idx<=sector_group_list->getSize()-1; grp_idx++) {
                delete (ListClass<int> *) ((*sector_group_list)[grp_idx]);
            }
            sector_group_list->reset();
        } else if (strcmp(command, "select_sectors_polygon") == 0) {
            double min_ext_dist, max_ext_dist;
            parse_command(command_opt, "gptlist min_ext_dist max_ext_dist f ext_f ext_bdy_f", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "gptlist", cmd_optlist[0]);
            check_param_value(min_ext_dist, command, "min_ext_dist", cmd_optlist[1], 0.0,          1.0e100, 1400.0);
            check_param_value(max_ext_dist, command, "max_ext_dist", cmd_optlist[2], min_ext_dist, 1.0e100, 1600.0);
            extract_intint_list(ii_list, cmd_optlist[0]);

            if (!error_state) {
                ((PHSNetworkClass *)this)->select_sectors_polygon(ii_list, min_ext_dist, max_ext_dist, cmd_optlist[3], cmd_optlist[4], cmd_optlist[5]);
            }
        } else if (strcmp(command, "switch_mode") == 0) {
            parse_command(command_opt, "mode", cmd_optlist);
            require_param(command, "mode", cmd_optlist[0]);
            if (cmd_optlist[0]) {
                static const char *mode_str[] = { "edit_geom", "simulate", 0};
                check_param_value(n, command, "mode", cmd_optlist[0], mode_str, 0);
                switch_mode((n==0) ? CConst::editGeomMode : CConst::simulateMode);
                GUI_FN(main_window->unsaved_sim_data = 0);
            }
        } else if (strcmp(command, "create_subnets") == 0) {
            int init_sample_res;
            int num_max;
            double scan_fractional_area;

            parse_command(command_opt, "scan_fractional_area init_sample_res exclude_ml num_max threshold_db dmax closest-", cmd_optlist);
            check_valid_mode(command, s_mode_valid);

            check_param_value(scan_fractional_area, command, "scan_fractional_area", cmd_optlist[0], 0.0, 1.0, 0.9);
            check_param_value(init_sample_res,      command, "init_sample_res",      cmd_optlist[1], 1, 2048, 16);
            check_param_value(num_max,              command, "num_max",              cmd_optlist[3], 1, num_cell, 1);
            check_param_value(threshold_db,         command, "threshold_db",         cmd_optlist[4], -200.0, 200.0, 0.0);
            check_param_value(subnet_dmax,          command, "dmax",                 cmd_optlist[5], 0.0,    1.0e6, 0.0);

            if ( (!error_state) && (init_sample_res & (init_sample_res-1)) ) {
                sprintf(msg, "ERROR: command %s parameter -%s set to invalid value %d\nValue must be a power of two\n",
                    command, "init_sample_res", init_sample_res);
                PRMSG(stdout, msg);
                error_state = 1;
            }

            if ( (!error_state) && (cmd_optlist[2]) ) {
                extract_int_list(int_list, cmd_optlist[2]);
                for (i=0; (i<=int_list->getSize()-1)&&(!error_state); i++) {
                    if ( ((*int_list)[i] < 0) || ((*int_list)[i] > map_layer_list->getSize()-1) ) {
                        sprintf(msg, "ERROR: command %s parameter -%s contains invalid value %d\nMap layer does not exist\n",
                            command, "exclude_ml", (*int_list)[i]);
                        PRMSG(stdout, msg);
                        error_state = 1;
                    }
                }
            } else {
                int_list->reset();
            }

            if (!error_state) {
                create_subnets(scan_fractional_area, init_sample_res, int_list, num_max, (cmd_optlist[4] ? 1 : 0), threshold_db, subnet_dmax, (cmd_optlist[6] ? 1 : 0));
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                    vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
                    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                        subnet = subnet_list[traffic_type_idx][subnet_idx];
                        new VisibilityCheckItem( vi, GConst::subnetRTTI, subnet_idx, subnet->strid, subnet->color);
                    }
                }
                main_window->geometry_modified = 1;
            }
#endif
        } else if (strcmp(command, "delete_subnet") == 0) {
            parse_command(command_opt, "traffic_type subnet", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "traffic_type", cmd_optlist[0]);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
            }
            require_param(command, "subnet", cmd_optlist[1]);
            if (!error_state) {
                subnet_idx = get_subnet_idx(cmd_optlist[1], traffic_type_idx, 1);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
                vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, traffic_type_idx);
                vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, subnet_idx);
                delete vci;
                if (subnet_idx != num_subnet[traffic_type_idx]-1) {
                    vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::subnetRTTI, num_subnet[traffic_type_idx]-1);
                    vci->setIndex(subnet_idx);
                    // vci->setText(0, subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1]->strid);
                    main_window->visibility_window->vec_vis_subnet[traffic_type_idx][subnet_idx]
                        = main_window->visibility_window->vec_vis_subnet[traffic_type_idx][num_subnet[traffic_type_idx]-1];
                }
                vlist->sort();
                main_window->visibility_window->vec_vis_subnet[traffic_type_idx] = (char *) realloc((void *) main_window->visibility_window->vec_vis_subnet[traffic_type_idx], (num_subnet[traffic_type_idx]-1)*sizeof(char));
                main_window->geometry_modified = 1;
            }
#endif
            if (!error_state) {
                delete subnet_list[traffic_type_idx][subnet_idx];
                if (subnet_idx != num_subnet[traffic_type_idx]-1) {
                    subnet_list[traffic_type_idx][subnet_idx] = subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1];
                }
                subnet_list[traffic_type_idx][num_subnet[traffic_type_idx]-1] = (SubnetClass *) NULL;
                num_subnet[traffic_type_idx]--;
                subnet_list[traffic_type_idx] = (SubnetClass **) realloc( (void *) subnet_list[traffic_type_idx], num_subnet[traffic_type_idx]*sizeof(SubnetClass *));
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::subnetRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "delete_all_subnet") == 0) {
            parse_command(command_opt, "traffic_type", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            if (!error_state) {
                if (cmd_optlist[0]) {
                    traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
                    int_list->append(traffic_type_idx);
                } else {
                    for (traffic_type_idx=0; traffic_type_idx<=num_traffic_type-1; traffic_type_idx++) {
                        int_list->append(traffic_type_idx);
                    }
                }
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                vlist = main_window->visibility_window->visibility_list;

                subnetViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);

                for (i=0; i<=int_list->getSize()-1; i++) {
                    traffic_type_idx = (*int_list)[i];
                    vi = (VisibilityItem *) VisibilityList::findItem(subnetViewItem,    GConst::subnetRTTI, traffic_type_idx);

                    vci = (VisibilityCheckItem *) vi->firstChild();
                    while(vci) {
                        delete vci;
                        vci = (VisibilityCheckItem *) vi->firstChild();
                    }
                    main_window->visibility_window->vec_vis_subnet[traffic_type_idx] = (char *) realloc((void *) main_window->visibility_window->vec_vis_subnet[traffic_type_idx], 0);
                }
                main_window->geometry_modified = 1;
            }
#endif
            if (!error_state) {
                for (i=0; i<=int_list->getSize()-1; i++) {
                    traffic_type_idx = (*int_list)[i];
                    for (subnet_idx=0; subnet_idx<=num_subnet[traffic_type_idx]-1; subnet_idx++) {
                        delete subnet_list[traffic_type_idx][subnet_idx];
                    }
                    subnet_list[traffic_type_idx] = (SubnetClass **) realloc( (void *) subnet_list[traffic_type_idx], 0);
                    num_subnet[traffic_type_idx] = 0;
                    subnet_list[traffic_type_idx] = (SubnetClass **) realloc( (void *) subnet_list[traffic_type_idx], num_subnet[traffic_type_idx]*sizeof(SubnetClass *));
                }
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->editor->setVisibility(GConst::subnetRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "report_subnets") == 0) {
            parse_command(command_opt, "traffic_type contained_cells- traffic_cs_density- f by_segment-", cmd_optlist);
            check_valid_mode(command, es_mode_valid);
            require_param(command, "traffic_type", cmd_optlist[0]);
            if (!error_state) {
                traffic_type_idx = get_traffic_type_idx(cmd_optlist[0], 1);
            }
            if (!error_state) {
                report_subnets(traffic_type_idx, (cmd_optlist[1] ? 1 : 0), (cmd_optlist[2] ? 1 : 0), cmd_optlist[3],
                               (cmd_optlist[4] ? 1 : 0) );
            }
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "reset_call_statistics") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            if (!error_state) {
                reset_call_statistics(0);
                GUI_FN(main_window->unsaved_sim_data = 0);
            }
#endif
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "print_call_statistics") == 0) {
            parse_command(command_opt, "f sectors fmt", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            static const char *call_stat_fmt_str[] = { "block", "tab", 0};
            check_param_value(n, command, "fmt", cmd_optlist[2], call_stat_fmt_str, 1);

            if (!error_state) {
                print_call_statistics(cmd_optlist[0], cmd_optlist[1], n);
            }
            if ((!error_state)&&(cmd_optlist[0])&&(!cmd_optlist[1])) {
                GUI_FN(main_window->unsaved_sim_data = 0);
            }
#endif
        } else if (strcmp(command, "redefine_system_bdy") == 0) {
            parse_command(command_opt, "threshold_db scan_fractional_area init_sample_res", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            require_param(command, "level", cmd_optlist[0]);
            check_param_value(threshold_db, command, "threshold_db", cmd_optlist[0], -200.0, 200.0, 0.0);
            check_param_value(scan_fractional_area, command, "scan_fractional_area", cmd_optlist[1], 0.0, 1.0, 0.995);
            check_param_value(n, command, "init_sample_res", cmd_optlist[2], 1, 2048, 16);
            if ( (!error_state) && (n & (n-1)) ) {
                sprintf(msg, "ERROR: command %s parameter -%s set to invalid value %d\nValue must be a power of two\n",
                    command, "init_sample_res", n);
                PRMSG(stdout, msg);
                error_state = 1;
            }
            if (!error_state) {
                redefine_system_bdy(threshold_db, scan_fractional_area, n);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) ) {
                main_window->geometry_modified = 1;
                main_window->editor->setVisibility(GConst::systemBoundaryRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "include") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, all_mode_valid);
            require_param(command, "f", cmd_optlist[0]);
            if (!error_state) {
                prev_fstream = input_stream;

                if ( (input_stream = fopen(cmd_optlist[0], "rb")) ) {
                    sprintf(msg, "Reading from file %s\n", cmd_optlist[0]);
                    set_current_dir_from_file(cmd_optlist[0]);
                    reached_eof = 0;
                } else {
                    sprintf(msg, "ERROR: Unable to open file \"%s\"\n", cmd_optlist[0]);
                    reached_eof = 1;
                }
                PRMSG(stdout, msg);

                while(cont && (!reached_eof)) {
                    if (get_next_command(input_stream, line_buf)) {
                        cont = process_command(line_buf);
                    } else {
                        reached_eof = 1;
                    }
                }
                if (input_stream) {
                    fclose(input_stream);
                } else {
                    error_state = 1;
                }
                input_stream = prev_fstream;
            }
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "reset_system") == 0) {
            parse_command(command_opt, "", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            if (!error_state) {
                reset_system();
            }
#endif
        } else if (strcmp(command, "gen_clutter") == 0) {
            parse_command(command_opt, "scope useheight sectors type slope intercept map_sim_res_ratio", cmd_optlist);
            check_valid_mode(command, e_mode_valid);

            check_param_value(scope, command, "scope", cmd_optlist[0], scope_str, 0);
            check_param_value(useheight, command, "useheight", cmd_optlist[1], 0, 1, 0);
            require_param(command, "sectors", cmd_optlist[2]);
            check_param_value(i, command, "type", cmd_optlist[3], clutter_type_str, 0);
            check_param_value(clutter_slope, command, "slope", cmd_optlist[4], -1.0e100, 1.0e100, -2.0);
            check_param_value(clutter_intercept, command, "intercept", cmd_optlist[5], -1.0e100, 1.0e100, -80.0);
            require_param(command, "map_sim_res_ratio", cmd_optlist[6]);
            check_param_value(map_sim_res_ratio, command, "map_sim_res_ratio", cmd_optlist[6],  1, 10000,        1);

            if (!error_state) {
                if (num_cell) {
                    BITWIDTH(bit_cell, num_cell-1);
                    extract_sector_list(int_list, cmd_optlist[2], CConst::CellIdxRef);
                } else {
                    n = 0;
                }
            }

            if (!error_state) {
                i = ( (i == 0) ? CConst::PropClutterSimp        :
                      (i == 1) ? CConst::PropClutterFull        :
                      (i == 2) ? CConst::PropClutterSymFull     :
                      (i == 3) ? CConst::PropClutterWtExpo      :
                      (i == 4) ? CConst::PropClutterWtExpoSlope :
                      (i == 5) ? CConst::PropClutterGlobal      :
                      (i == 6) ? CConst::PropClutterExpoLinear  : -1 );
                gen_clutter(int_list, useheight, scope, i, clutter_slope, clutter_intercept, map_sim_res_ratio);

                bit_cell = -1;

                GUI_FN(main_window->geometry_modified = 1);
            }
#if HAS_GUI
            if ( (use_gui) && (!error_state) && n ) {
                main_window->visibility_window->visibility_list->update_cpm();
                main_window->visibility_window->resize();
                main_window->editor->setVisibility(GConst::clutterPropModelRTTI);
                main_window->canvas->update();
            }
#endif
        } else if (strcmp(command, "check_file") == 0) {
            parse_command(command_opt, "f", cmd_optlist);
            check_valid_mode(command, all_mode_valid);
            if (!error_state) {
                check_file(cmd_optlist[0]);
            }
        } else if (strcmp(command, "version") == 0) {
            parse_command(command_opt, "a-", cmd_optlist);
            chptr = msg;
            chptr += sprintf(chptr, "WiSIM CODE RELEASE VERSION: \"%s\"\n", WISIM_RELEASE);
            if (cmd_optlist[0]) {
                chptr += sprintf(chptr, "HAS_GUI:         %d\n", HAS_GUI);
                chptr += sprintf(chptr, "HAS_ORACLE:      %d\n", HAS_ORACLE);
                chptr += sprintf(chptr, "HAS_MONTE_CARLO: %d\n", HAS_MONTE_CARLO);
                chptr += sprintf(chptr, "HAS_CLUTTER:     %d\n", HAS_CLUTTER);
                chptr += sprintf(chptr, "DEMO:            %d\n", DEMO);
                chptr += sprintf(chptr, "TR_EMBED:        %d\n", TR_EMBED);
                chptr += sprintf(chptr, "CDEBUG:          %d\n", CDEBUG);
            }
            PRMSG(stdout, msg);
        } else if (strcmp(command, "reseed_rangen") == 0) {
            parse_command(command_opt, "seed", cmd_optlist);
            check_valid_mode(command, e_mode_valid);
            check_param_value(seed, command, "seed", cmd_optlist[0], 0, MAX_SEED, 0);
            if (!error_state) {
                sprintf(msg, "seed set to %d\n", seed);
                PRMSG(stdout, msg);
                rg->RandomInit(seed);
            }
        } else if (strcmp(command, "date") == 0) {
            parse_command(command_opt, "s", cmd_optlist);
            time(&timeVal);
            if (cmd_optlist[0]) {
                sprintf(msg, "%s %s\n", cmd_optlist[0], ctime(&timeVal));
            } else {
                sprintf(msg, "%s\n", ctime(&timeVal));
            }
            PRMSG(stdout, msg);
#if HAS_MONTE_CARLO
        } else if (strcmp(command, "run_match") == 0) {
                int it,rt;
            parse_command(command_opt, "f it rt force_fmt check", cmd_optlist);
            check_valid_mode(command, s_mode_valid);
            //require_param(command, "f", cmd_optlist[0]);
            //require_param(command, "it", cmd_optlist[1]);
            //require_param(command, "rt", cmd_optlist[2]);
            check_param_value(it, command, "it", cmd_optlist[1], 1, 36000, 3600);
            check_param_value(rt, command, "rt", cmd_optlist[2], 1, 36000, 3600);
            //PRMSG(stdout, cmd_optlist[1]);      
            //PRMSG(stdout, cmd_optlist[2]);      									          

            if (!error_state) {
                //read_match_data(cmd_optlist[0], cmd_optlist[3], cmd_optlist[4]);
                set_match_data(it,rt);
            }
            if (!error_state) {
                //run_match();
            }
#endif        
        } else if (strcmp(command, "h") == 0) {
            check_valid_mode(command, all_mode_valid);
            opt_ptr = opt_msg;
            while (*opt_ptr) {
                sprintf(msg, "%s\n", *opt_ptr++);
                PRMSG(stdout, msg);
            }
        } else if (strcmp(command, "q") == 0) {
            check_valid_mode(command, all_mode_valid);
            cont = 0;
        }
        // use cmdfound flag
        // to avoid the error: "fatal error C1061: compiler limit : blocks nested too deeply"
        else
            cmdfound = false;

        if (!cmdfound)
        {
            //liutao
#if HAS_GUI
            if (strcmp(command,"handover") == 0) {
                parse_command(command_opt, "f all- ", cmd_optlist);

                check_valid_mode(command, all_mode_valid);

                if (!error_state) {      
                    main_window->editor->if_show_handset_anomalies = 1;
                    main_window->editor->get_handset_anomalies(cmd_optlist[0]);
                    main_window->editor->if_show_handset_noise = 1;	
                    main_window->editor->show_handset_noise();
                }
            } else if (strcmp(command,"next") == 0) {
                check_valid_mode(command, all_mode_valid);
            
                if (!error_state) {
                    if (main_window->editor->if_show_handset_anomalies ==1 
                    && main_window->editor->ho_show_index<main_window->editor->num_anomalies-1) {
                        main_window->editor->ho_show_index++;
                        main_window->editor->show_handset_anomalies();
                    }
                }
                else if(main_window->editor->ho_show_index>=main_window->editor->num_anomalies-1) {
                    sprintf(msg, "All handover points has been shown");
                    PRMSG(stdout, msg);
                }
            } else if (strcmp(command,"toggle_noise") == 0) {
                check_valid_mode(command, all_mode_valid);
                
                if (!error_state) {
                    main_window->editor->if_show_handset_noise = !main_window->editor->if_show_handset_noise;	
                    main_window->editor->resizeCanvas();
                }
            }
#endif
        //end liutao

#if (CDEBUG)
        /**************************************************************************************/
        /**** Debugging commands                                                           ****/
        /**************************************************************************************/
#if HAS_GUI
            else if (strcmp(command, "lc") == 0) {
                Q3CanvasItemList list = main_window->canvas->allItems();
                Q3CanvasItemList::Iterator it;

                printf("Canvas Item List:\n");
                for (it = list.begin(); it != list.end(); it++) {
                    printf("RTTI = %d\n", (*it)->rtti());
                }
                printf("\n");
            }
            else if (strcmp(command, "set_zoom") == 0) {
                parse_command(command_opt, "z", cmd_optlist);
                if (cmd_optlist[0]) {
                    double z = atof(cmd_optlist[0]);
                    main_window->editor->setZoom(z);
                }
            }
#endif
            else if (strcmp(command, "cr") == 0) {
                parse_command(command_opt, "pm_idx iter", cmd_optlist);
                check_valid_mode(command, e_mode_valid);
                check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[0],         0, num_prop_model-1, 0);
                check_param_value(i,        command, "iter",     cmd_optlist[1],         0,                1, 0);

                BITWIDTH(bit_cell, num_cell-1);
                extract_sector_list(int_list, "all", CConst::CellIdxRef);

                if (!error_state) {
                    if (prop_model_list[pm_idx]->type() == CConst::PropClutterFull) {
                        ((ClutterPropModelFullClass *) prop_model_list[pm_idx])->clt_regulation(this);
                    } else if (prop_model_list[pm_idx]->type() == CConst::PropClutterSymFull) {
                        ((ClutterSymFullPropModelClass *) prop_model_list[pm_idx])->clt_regulation(this);
                    } else if (prop_model_list[pm_idx]->type() == CConst::PropClutterWtExpo) {
                        ((ClutterWtExpoPropModelClass *) prop_model_list[pm_idx])->clt_regulation(this, int_list, i);
                    } else if (prop_model_list[pm_idx]->type() == CConst::PropClutterWtExpoSlope) {
                        ((ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx])->clt_regulation(this, int_list, i);
                    }
                }

                bit_cell = -1;
            }
            else if (strcmp(command, "cs") == 0) {
                parse_command(command_opt, "pm_idx", cmd_optlist);
                check_valid_mode(command, e_mode_valid);
                check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[0],         0, num_prop_model-1, 0);

                BITWIDTH(bit_cell, num_cell-1);

                if (!error_state) {
                    map_clutter->split(this);
                    if (prop_model_list[pm_idx]->type() == CConst::PropClutterWtExpoSlope) {
                        ((ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx])->split_clutter(this);
                    }
                }

                bit_cell = -1;
            }
            else if (strcmp(command, "cf") == 0) {
                parse_command(command_opt, "pm_idx", cmd_optlist);
                check_valid_mode(command, e_mode_valid);
                check_param_value(pm_idx,   command, "pm_idx",   cmd_optlist[0],         0, num_prop_model-1, 0);

                BITWIDTH(bit_cell, num_cell-1);
                extract_sector_list(int_list, "all", CConst::CellIdxRef);

                if (!error_state) {
                    if (prop_model_list[pm_idx]->type() == CConst::PropClutterWtExpoSlope) {
                        ((ClutterWtExpoSlopePropModelClass *) prop_model_list[pm_idx])->clt_fill(this, int_list);
                    }
                }

                bit_cell = -1;
            }
            else if (strcmp(command, "pra") == 0) {
                parse_command(command_opt, "cell_idx", cmd_optlist);
                check_param_value(cell_idx, command, "cell_idx", cmd_optlist[0], 0, num_cell-1, 0);
                for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
                    rtp = &( (*road_test_data_list)[rtd_idx] );
                    if (rtp->cell_idx == cell_idx) {
                        double delta_x = (rtp->posn_x - cell_list[cell_idx]->posn_x) * resolution;
                        double delta_y = (rtp->posn_y - cell_list[cell_idx]->posn_y) * resolution;
                        double rsq = delta_x*delta_x + delta_y*delta_y;
                        double d = 0.5*log(rsq)/log(10.0);
                        double phi = 180.0*atan2(delta_y, delta_x)/PI;
                        printf("%15.10e %15.10e\n", phi, d);
                    }
                }
                printf("\n");
            }
            else if (strcmp(command, "mm") == 0) {
                /* move map */
                int mx, my;
                char *str1;
                PolygonClass *p;
                LineClass    *pline;
                int polygon_idx, line_idx;
                str1 = strtok(command_opt, CHDELIM); mx = atoi(str1);
                str1 = strtok(NULL, CHDELIM);    my = atoi(str1);
                for (map_layer_idx=0; map_layer_idx<=map_layer_list->getSize()-1; map_layer_idx++) {
                    ml = (*map_layer_list)[map_layer_idx];
                    for (polygon_idx=0; polygon_idx<=ml->num_polygon-1; polygon_idx++) {
                        p = ml->polygon_list[polygon_idx];
                        for (segment_idx=0; segment_idx<=p->num_segment-1; segment_idx++) {
                            for (pt_idx=0; pt_idx<=p->num_bdy_pt[segment_idx]; pt_idx++) {
                                p->bdy_pt_x[segment_idx][pt_idx] += mx;
                                p->bdy_pt_y[segment_idx][pt_idx] += my;
                            }
                        }
                    }
                    for (line_idx=0; line_idx<=ml->num_line-1; line_idx++) {
                        pline = ml->line_list[line_idx];
                        for (pt_idx=0; pt_idx<=pline->num_pt-1; pt_idx++) {
                            pline->pt_x[pt_idx] += mx;
                            pline->pt_y[pt_idx] += my;
                        }
                    }
                }
#if HAS_GUI
                if ( (use_gui) && (!error_state) ) {
                    main_window->editor->setVisibility(GConst::mapLayerRTTI);
                    main_window->canvas->update();
                }
#endif
            }
            else if (strcmp(command, "mp") == 0) {
                int x0, y0, x1, y1;
                char *str1;
                str1 = strtok(NULL, CHDELIM); x0 = atoi(str1);
                str1 = strtok(NULL, CHDELIM); y0 = atoi(str1);
                str1 = strtok(NULL, CHDELIM); x1 = atoi(str1);
                str1 = strtok(NULL, CHDELIM); y1 = atoi(str1);
                double *dist_vector = (double *) malloc( map_clutter->num_clutter_type * sizeof(double));

                printf("Calling get_path_clutter: (%d, %d) --> (%d, %d)\n", x0, y0, x1, y1);
                get_path_clutter(x0, y0, x1, y1, dist_vector);

                for (int clutter_idx=0; clutter_idx<=map_clutter->num_clutter_type-1; clutter_idx++) {
                    printf("Clutter type %2d Dist = %15.10f\n", clutter_idx, dist_vector[clutter_idx]);
                }
                free(dist_vector);
            } else if (strcmp(command, "drtd") == 0) { /* Dump road test data */
                parse_command(command_opt, "f", cmd_optlist);

#if 1
                double *dist_vector;
                char filename[100];
                FILE *fp;
                if (map_clutter) {
                    dist_vector = (double *) malloc( map_clutter->num_clutter_type * sizeof(double));
                } else {
                    dist_vector = (double *) NULL;
                }
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        SectorClass *sector = cell->sector_list[sector_idx];
                        if (cmd_optlist[0]) {
                            sprintf(filename, "/tmp/rtd/%s_%d_%d", cmd_optlist[0], cell_idx, sector_idx);
                            fp = fopen(filename, "w");
                        } else {
                            fp = stdout;
                        }
                        printf("# %d %d\n", cell_idx, sector_idx);
                        for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
                            rtp = &( (*road_test_data_list)[rtd_idx] );
                            if ( (rtp->cell_idx == cell_idx) && (rtp->sector_idx == sector_idx) ) {
                                if (map_clutter) {
                                    get_path_clutter(cell->posn_x, cell->posn_y, rtp->posn_x, rtp->posn_y, dist_vector);
                                }
                                double dx = (double) rtp->posn_x - cell->posn_x;
                                double dy = (double) rtp->posn_y - cell->posn_y;
                                double dz = - sector->antenna_height;
                                double logd = 0.5*log( dx*dx + dy*dy + dz*dz) / log(10.0);
                                AntennaClass *antenna = antenna_type_list[sector->antenna_type];
                                double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
                                double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
                                fprintf(fp, "%15.10f    %15.10f", logd, rtp->pwr_db - gain_db - tx_pwr_db);
                                MapClutterClass *mc = map_clutter;
                                if (mc) {
                                    for (i=0; i<=mc->num_clutter_type-1; i++) { 
                                        fprintf(fp, "    %15.10f", dist_vector[i]);
                                    }
                                }
                            fprintf(fp, "\n");
                            }
                        }
                        fprintf(fp, "\n");
                        if (fp != stdout) {
                            fclose(fp);
                        }
                    }
                }
                free(dist_vector);
            } else if (strcmp(command, "ppm") == 0) { /* Print propagation model data */
                for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
                    cell = cell_list[cell_idx];
                    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                        SectorClass *sector = cell->sector_list[sector_idx];
                        if (sector->prop_model != -1) {
                            TerrainPropModelClass *tpm = (TerrainPropModelClass *) prop_model_list[sector->prop_model];
                            printf("%8.5f %8.5f %8.5f %8.5f %8.5f\n", sector->antenna_height, exp(tpm->val_y*log(10.0)), tpm->val_py, tpm->val_s1, tpm->val_s2);
                        }
                    }
                }
            } else if (strcmp(command, "drssi") == 0) { /* Dump road test data */
                // dump_rssi_data(np);
#endif
            }
            /**************************************************************************************/
#endif
            else {
                if (!rec_command) {
                    sprintf(msg, "Unrecognized command: %s\n", command);
                    PRMSG(stdout, msg);
                    error_state = 1;
                }
            }
        }
    }

    if (cont) {
        if (error_state) {
            strcpy(prompt, "ERROR> ");
            GUI_FN(main_window->toggle_command_window(GConst::visShow));
        } else {
            strcpy(prompt, "WiSim> ");
        }

        if (warning_state) {
            warning_state = 0;
            if (!error_state) {
                GUI_FN(main_window->toggle_command_window(GConst::visShow));
            }
        }

#if HAS_GUI
        if (use_gui) {
            main_window->set_command_window_prompt();
            main_window->gui_mode_changed();
            if (input_stream == stdin) {
                QApplication::restoreOverrideCursor();
                GUI_FN(main_window->setEnabled(true));
            }

            main_window->editor->clear_select_list();
        }
#endif
    }

#if HAS_GUI
    if (!use_gui) {
#endif
        if ((cont) && (input_stream == stdin)) {
            printf(prompt);
            fflush(stdout);
        }
#if HAS_GUI
    }
#endif

    delete int_list;
    delete dbl_list;
    delete ii_list;

    if ((!cont) && (input_stream == stdin)) {
        if (mode != CConst::noGeomMode) {
            close_geometry();
        }
        GUI_FN(delete main_window);
        delete this;
        GUI_FN(qApp->quit());
    }

    return(cont);
}
/******************************************************************************************/
void NetworkClass::require_param(char *command, const char *param_name, char *param_value)
{
    if ((!error_state)&&(!param_value)) {
        sprintf(msg, "ERROR: command %s requires that parameter -%s must be specified\n",
            command, param_name);
        PRMSG(stdout, msg);

        error_state = 1;
    }

    return;
}
/******************************************************************************************/
void NetworkClass::check_valid_mode(char *command, int valid_modes)
{
    int mask;
    char *chptr;

    switch (mode) {
        case CConst::noGeomMode:   mask = 1; break;
        case CConst::editGeomMode: mask = 2; break;
        case CConst::simulateMode: mask = 4; break;
        default: mask = -1;       CORE_DUMP; break;
    }

    if ( !(valid_modes & mask) ) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR: command %s can not be run in Mode %s\n",
            command,
            (mode==CConst::editGeomMode ? "EDIT_GEOM" :
             mode==CConst::simulateMode ? "SIMULATE"  : "NO_GEOMETRY") );
        PRMSG(stdout, msg);

        error_state = 1;
    }

    return;
}
/******************************************************************************************/
void NetworkClass::check_param_value(int& param_int_value, char *command, const char *param_name, char *param_value,
    const int min_value, const int max_value, const int default_value)
{
    int ival;
    char *chptr;


    if ( (!error_state) && (param_value) ) {
        ival = atoi(param_value);
        if ( (ival < min_value) || (ival > max_value) ) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: command %s parameter -%s set to invalid value %d\n",
                command, param_name, ival);
            chptr += sprintf(chptr, "Value must be between %d and %d\n", min_value, max_value);
            PRMSG(stdout, msg);

            error_state = 1;
        } else {
            param_int_value = ival;
        }
    } else {
        param_int_value = default_value;
    }

    return;
}
/******************************************************************************************/
void NetworkClass::check_param_value(double& param_double_value, char *command, const char *param_name, char *param_value,
    const double min_value, const double max_value, const double default_value)
{
    double dval;
    char *chptr;

    if ( (!error_state) && (param_value) ) {
        dval = atof(param_value);
        if ( (dval < min_value) || (dval > max_value) ) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: command %s parameter -%s set to invalid value %g\n",
                command, param_name, dval);
            chptr += sprintf(chptr, "Value must be between %g and %g\n", min_value, max_value);
            PRMSG(stdout, msg);

            error_state = 1;
        } else {
            param_double_value = dval;
        }
    } else {
        param_double_value = default_value;
    }

    return;
}
/******************************************************************************************/
void NetworkClass::check_param_value(int& param_int_value, char *command, const char *param_name, char *param_value,
    const char **valid_value_list, const int default_value)
{
    int i, found;
    char *chptr;

    param_int_value = default_value;

    if ( (!error_state) && (param_value) ) {
        i = 0;
        found = 0;
        while ((!found) && (valid_value_list[i])) {
            if (strcmp(valid_value_list[i], param_value)==0) {
                found = 1;
                param_int_value = i;
            }
            i++;
        }
        if (!found) {
            chptr = msg;
            chptr += sprintf(chptr, "ERROR: command %s parameter -%s set to invalid value \"%s\"\n",
                command, param_name, param_value);
            chptr += sprintf(chptr, "Valid values are:");
            i = 0;
            while (valid_value_list[i]) {
                chptr += sprintf(chptr, " \"%s\"", valid_value_list[i++]);
            }
            chptr += sprintf(chptr, "\n");

            PRMSG(stdout, msg);
            error_state = 1;
        }
    }

    return;
}
/******************************************************************************************/
void NetworkClass::check_technology(const char *param_name, int valid_tech)
{
    int ival;
    char *chptr;

    if ( (!error_state) && (technology() != valid_tech) ) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR: parameter -%s can only be set for techology %s, current technology is %s\n",
            param_name, TECHNOLOGY_NAME(valid_tech), TECHNOLOGY_NAME(technology()));
        PRMSG(stdout, msg);
        error_state = 1;
    }

    return;
}
/******************************************************************************************/
void proc_xy_list(int &n, double *x, double *y, char *str, int &allocation_size, int &error_state)
{
    n = 0;
    if (!error_state) {
        char *chptr = strtok(str, CHDELIM);
        while ( (chptr) && (!error_state) ) {
            if (n/2 >= allocation_size) {
                allocation_size += 1000;
                x = (double *) realloc((void *) x, (allocation_size)*sizeof(double));
                y = (double *) realloc((void *) y, (allocation_size)*sizeof(double));
            }
            if ( (n&1) == 0) {
                x[n/2] = atof(chptr);
            } else {
                y[n/2] = atof(chptr);
            }
            n++;
            chptr = strtok(NULL, CHDELIM);
        }

        if (n&1) {
            error_state=1;
        }
    }

    n = n/2;

    return;
}
/******************************************************************************************/
/***  FUNCTION: gaussian                                                                ***/
/***  INPUTS:                                                                           ***/
/***  OUTPUTS: p1, p2                                                                   ***/
/***  Returns pointers to 2 indep. mean 0 var 1 gaussian r.v.s                          ***/
void gaussian(TRandomMersenne *rg, double &g1, double &g2)
{
    double x, y, u, r;

    do {
        x = 2*rg->Random() - 1.0;
        y = 2*rg->Random() - 1.0;

        u = x*x + y*y;
    } while ( u >= 1.0);

    r = sqrt(-2.0*log(u));
    u = sqrt(u);
    g1 = r*x/u;
    g2 = r*y/u;
}
/******************************************************************************************/


#if HAS_GUI
/******************************************************************************************/
/**** FUNCTION: install_license_file                                                   ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS: Returns 0 on success, 1 on error                                        ****/
int install_license_file(char *install_file, char *WiSim_home, unsigned char *reg_info, int ris)
{
    QString s, errmsg = "";
    QMessageBox* msg_box;
    int c;
    int error = 0;
    char *license_file;
    FILE *file_r, *file_w;

    if ( !(file_r = fopen(install_file, "rb")) ) {
        errmsg += qApp->translate("QMessageBox", "Cannot read from file") + ": " + QString::fromLocal8Bit(install_file);
        error = 1;
    }

    if (!error) {
        int r = proc_lic_file((char *) NULL, install_file, reg_info, ris, 1);

        if (r == LIC_FILE_NOT_FOUND) {
            errmsg += qApp->translate("QMessageBox", "Unable to Open File") + " " + QString::fromLocal8Bit(install_file) + ".";
            error = 1;
        } else if (r == LIC_INVALID) {
            errmsg += qApp->translate("QMessageBox", "File") + " " + QString::fromLocal8Bit(install_file) + " ";
            errmsg += qApp->translate("QMessageBox", "is not a valid license file.");
            error = 1;
        } else if (r == LIC_EXPIRED) {
            errmsg += qApp->translate("QMessageBox", "License File") + " " + QString::fromLocal8Bit(install_file) + " ";
            errmsg += qApp->translate("QMessageBox", "has expired.");
            error = 1;
        } else if (r == LIC_NO_NET_CONN) {
            errmsg += qApp->translate("QMessageBox", "No Network Connection Available.  Unable to Install File") + " ";
            errmsg += QString::fromLocal8Bit(install_file) + ".";
            error = 1;
        }

    }

    if (!error) {
        license_file = CVECTOR( strlen(WiSim_home) + strlen("/license.dat") );
        sprintf(license_file, "%s%c%s", WiSim_home, FPATH_SEPARATOR, "license.dat");
        if ( !(file_w = fopen(license_file, "wb")) ) {
            errmsg += qApp->translate("QMessageBox", "Cannot write to file:") + " " + QString::fromLocal8Bit(license_file);
            error = 1;
        }
    }

    if (!error) {
        while ((c=fgetc(file_r)) != EOF) {
            fprintf(file_w, "%c", c);
        }
        fclose(file_r);
        fclose(file_w);

        s = "<h3>License File Successfully Installed</h3>";
        s += "<ul>";
        s +=    "<li> " + qApp->translate("QMessageBox", "License File Successfully Installed") + ".";
        s +=    "<li> " + qApp->translate("QMessageBox", "Please restart WiSim to use the newly installed license file") + ".";
        s += "</ul>";
    } else {
        s = "<h3>License File Installation Error</h3>";
        s += "<ul>";
        s +=    "<li> " + qApp->translate("QMessageBox", "ERROR") + ": " + errmsg;
        s += "</ul>";
    }

    msg_box = new QMessageBox( qApp->translate("QMessageBox", "WiSim"),
        s, (error ? QMessageBox::Critical : QMessageBox::Information), 1 | QMessageBox::Default, 0, 0, 0, 0, TRUE );
    msg_box->setButtonText( 1, qApp->translate("QMessageBox", "OK") );
    msg_box->show();
    msg_box->exec();
    delete msg_box;

    return(error);
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: parse_command                                                          ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void NetworkClass::parse_command(char *command_ptr, const char *kwstrptr, char **cmd_optlist)
{
    int n, i, k, state, found, *cmd_kwhasopt;
    int quoted_str = -1;
    int kw_idx     = -1;
    char **cmd_kwlist, *chptr, nch, *kwstr, *msgptr;

    cmd_kwlist   = (char **) malloc(100*sizeof(char *));
    cmd_kwhasopt = IVECTOR(100);
    kwstr = strdup(kwstrptr);

    n = -1;
    do {
        n++;
        chptr = (n==0 ? kwstr : (char *) NULL);
        cmd_kwlist[n] = strtok(chptr, CHDELIM);
    } while(cmd_kwlist[n]);

    for (i=0; i<=n-1; i++) {
        k = strlen(cmd_kwlist[i]);
        if (cmd_kwlist[i][k-1] == '-') {
            cmd_kwlist[i][k-1] = (char) NULL;
            cmd_kwhasopt[i] = 0;
        } else {
            cmd_kwhasopt[i] = 1;
        }
        cmd_optlist[i] = (char *) NULL;
    }

    chptr = command_ptr;
    state = 0;

    if (chptr) {
        while(*chptr) {

            switch(state) {
                case 0:
                    if (*chptr == ' ') {
                        chptr++;
                    } else if (*chptr == '-') {
                        found = 0;
                        for (i=0; (i<=n-1)&&(!found); i++) {
                            if (strncmp(chptr+1, cmd_kwlist[i], strlen(cmd_kwlist[i]))==0) {
                                nch = *(chptr + 1 + strlen(cmd_kwlist[i]));
                                if ( (nch == (char) NULL) || (nch == ' ') ) {
                                    found = 1;
                                    kw_idx = i;
                                    chptr += 1 + strlen(cmd_kwlist[i]);
                                    state = (cmd_kwhasopt[i] ? 1 : 0);
                                    if (!cmd_kwhasopt[i]) {
                                        cmd_optlist[i] = chptr;
                                        if (*chptr) {
                                            *chptr = (char) NULL;
                                            chptr++;
                                        }
                                    }
                                }
                            }
                        }
                        if (!found) {
                            msgptr = msg;
                            msgptr += sprintf(msgptr, "ERROR: Unable to parse command\n");
                            msgptr += sprintf(msgptr, "Option not found: %s\n", chptr);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                    } else {
                        msgptr = msg;
                        msgptr += sprintf(msgptr, "ERROR: Unable to parse command\n");
                        msgptr += sprintf(msgptr, "Expecting '-' followed by keyword: %s\n", chptr);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    break;
                case 1:
                    if (*chptr == ' ') {
                        chptr++;
                    } else {
                        if (*chptr == '\'') {
                            quoted_str = 1;
                            chptr++;
                        } else {
                            quoted_str = 0;
                        }
                        cmd_optlist[kw_idx] = chptr;
                        state = 2;
                    }
                    break;
                case 2:
                    if ( (quoted_str) && (*chptr == '\'') ) {
                        *chptr = (char) NULL;
                        state = 0;
                    } else if ( (!quoted_str) && (*chptr == ' ') ) {
                        *chptr = (char) NULL;
                        state = 0;
                    }
                    chptr++;
                    break;
                default:
                    sprintf(msg, "unable to parse command\n");
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if ( (state == 1) || ((state == 2) && (quoted_str)) ) {
        sprintf(msg, "unable to parse command\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    free(kwstr);
    free(cmd_kwlist);
    free(cmd_kwhasopt);

    return;
}
/******************************************************************************************/
/*** Set all variables defined by command line options.                              ******/
void set_options(int argc, char **argv)
{
    const static char  *help_msg[] = {
#if HAS_GUI
    " -gui         --            Use GUI.",
    " -no_gui      --            No GUI.",
#endif
    " -init        cmdfile       Specify command file to execute",
    " -f           geofile       Specify geometry file to read",
    " -log         logfile       Specify name of logfile (default WiSim.log)",
    " -technology  [phs | wcdma | cdma2000 | wlan] Select technology",
    " -h           --            Show this help message.",
    0};
    static char  *usage[] = {
            " [ -option value] [ -h ]",
    0};
    char *name = *argv;
    const char **p = help_msg;
    char **u = usage;
    while ( --argc > 0 ) {
        argv++;

             if (strcmp(*argv,"-v")          ==0)
             {   printf("WiSim version %s\n", WISIM_RELEASE);
                 printf("CG, Hangzhou, China\n");
                 exit(1); }
#if HAS_GUI
        else if (strcmp(*argv,"-gui")           ==0) { use_gui = 1;                            }
        else if (strcmp(*argv,"-no_gui")        ==0) { use_gui = 0;                            }
        else if (strcmp(*argv,"-no_logo")       ==0) { show_logo = 0;                          }
        else if (strcmp(*argv,"-no_lic_update") ==0) { show_lic_update = 0;                    }
        else if (strcmp(*argv,"-hide")          ==0) { hide_main = 1;                          }
#endif
        else if (strcmp(*argv,"-init")          ==0) { initfile  = strdup(*++argv);    argc--; }
        else if (strcmp(*argv,"-f")             ==0) { geomfile  = strdup(*++argv);    argc--; }
        else if (strcmp(*argv,"-log")           ==0) { logfile   = strdup(*++argv);    argc--; }
        else if (strcmp(*argv,"-technology")    ==0) {
            argv++;
            if (strcmp(*argv, "phs")==0) {
                technology = CConst::PHS;
            } else if (strcmp(*argv, "wcdma")==0) {
                technology = CConst::WCDMA;
            } else if (strcmp(*argv, "cdma2000")==0) {
                technology = CConst::CDMA2000;
            } else if (strcmp(*argv, "wlan")==0) {
                technology = CConst::WLAN;
            } else {
                fprintf(stderr, "\n\n%s Invalid Technology: %s \n", name, *argv);
                exit(1);
            }
            argc--;
        }
        else if (strcmp(*argv,"-h")==0)
        {   fprintf(stdout, "\n\n");
            fprintf(stdout, "usage:\n%s", name);
            while (*u) fprintf(stdout, "%s\n", *u++);
            fprintf(stdout, "\n");
            while (*p) fprintf(stdout, "%s\n", *p++);
#if CDEBUG
            system("pause");
#endif
            exit(1); }
        else
        {   fprintf(stderr, "\n\n%s Invalid Option: %s \n", name, *argv);
            fprintf(stderr, "\n\n");
            fprintf(stderr, "usage:\n%s", name);
            while (*u) fprintf(stderr, "%s\n", *u++);
            fprintf(stderr, "\n");
#if CDEBUG
            system("pause");
#endif
            exit(1); }
    }

    return;
}
/******************************************************************************************/
inline
void scramble(BinIntClass &out, BinIntClass &in)
{
    int i, n_msb;
    BinIntClass d(LIC_NUM_BYTES/BYTES_PER_SEG);
    BinIntClass n(LIC_NUM_BYTES/BYTES_PER_SEG);
    BinIntClass **f;

    f = (BinIntClass **) malloc(BITS_PER_SEG*sizeof(BinIntClass *));
    for (i=0; i<=BITS_PER_SEG-1; i++) {
        f[i] = new BinIntClass(LIC_NUM_BYTES/BYTES_PER_SEG);
    }

#include "scramble.h"

    n.comp_f(n_msb, f);
    out.exp_mod(in, d, n, n_msb, f);

    for (i=0; i<=BITS_PER_SEG-1; i++) {
        delete f[i];
    }
    free(f);
}

/******************************************************************************************/
/**** PROCESS LICENSE FILE                                                             ****/
/**** Return codes: LIC_VALID          = Valid registration                            ****/
/****               LIC_FILE_NOT_FOUND = License file not found (unable to read)       ****/
/****               LIC_INVALID        = License file invalid                          ****/
/****               LIC_NO_NET_CONN    = Unable to obtain network time                 ****/
/****               LIC_USB_INVALID    = the usb key is not invalid                    ****/
/******************************************************************************************/
int proc_lic_file(char *path, char *filename, unsigned char *reg_info, int ris, int force)
{
    int access_network, use_net_time, require_usb;
    BinIntClass c(LIC_NUM_BYTES/BYTES_PER_SEG);
    FILE *fp;
    BinIntClass x(LIC_NUM_BYTES/BYTES_PER_SEG);
    unsigned char ptr[LIC_NUM_BYTES];
    time_t start_time, end_time, current_time;

    char path_filename[500];

    if (path) {
        sprintf(path_filename, "%s/%s", path, filename);
    } else {
        sprintf(path_filename, "%s", filename);
    }
    fp = fopen(path_filename, "rb");
    if (!fp) {
        return(LIC_FILE_NOT_FOUND);
    }
 
    if (fread(ptr, sizeof(unsigned char), LIC_NUM_BYTES, fp) != LIC_NUM_BYTES) {
        fclose(fp);
        return(LIC_INVALID);
    }
    fclose(fp);
    c.setval(ptr);

    scramble(x, c);

    x.writebytestr(ptr);

#if USE_MAC_INFO
    if (memcmp(ptr+LIC_BYTE_OFFSET, reg_info, ris) != 0) {
        return(LIC_INVALID);
    }
#else
    if (memcmp(ptr+LIC_BYTE_OFFSET, reg_info, ris-26) != 0) {
        return(LIC_INVALID);
    }

    if (memcmp(ptr+LIC_BYTE_OFFSET+ris-20, reg_info+ris-20, 20) != 0) {
        return(LIC_INVALID);
    }
#endif

    memcpy(&start_time, ptr+LIC_BYTE_OFFSET+ris, sizeof(unsigned long));
    start_time = (int) ntohl((unsigned long) start_time);

    memcpy(&end_time, ptr+LIC_BYTE_OFFSET+ris+sizeof(unsigned long), sizeof(unsigned long));
    end_time = (int) ntohl((unsigned long) end_time);

    if (ptr[LIC_BYTE_OFFSET+ris+2*sizeof(unsigned long)] == LIC_NET_TIME) {
        use_net_time = 1;
    } else if (ptr[LIC_BYTE_OFFSET+ris+2*sizeof(unsigned long)] == LIC_LOCAL_TIME) {
        use_net_time = 0;
    } else {
        return(LIC_INVALID);
    }

    if (ptr[LIC_BYTE_OFFSET+ris+2*sizeof(unsigned long)+1] == LIC_REQUIRE_USB) {
        require_usb = 1;
    } else if (ptr[LIC_BYTE_OFFSET+ris+2*sizeof(unsigned long)+1] == LIC_DO_NOT_REQUIRE_USB) {
        require_usb = 0;
    } else {
        return(LIC_INVALID);
    }

    if (end_time <= start_time) {
        return(LIC_INVALID);
    }

    int flag = 0;
    if (use_net_time) {
        int lic_n;
        if (force) {
            lic_n = -1;
        } else {
            lic_n = get_lic_n(reg_info);
        }

        if (lic_n > 0) {
#if HAS_GUI
            if (show_lic_update && (lic_n <= LIC_UPDATE_N)) {
                QString s;
                s = "<h3>WISIM</h3>";
                s += "<ul>";
                s +=    QString("You currently can run WISIM %1 times without accessing a network connection.").arg(lic_n)    + "  \n";
                s +=    QString("Do you want to update the license counter now?")  + "  \n";
                s += "</ul>";

                static QMessageBox* verifyQuitBox = new QMessageBox( "WiSim",
                    s, (lic_n < 5 ? QMessageBox::Warning : QMessageBox::Information),
                    1, 2 | QMessageBox::Default, 0, 0, 0, TRUE );
                    verifyQuitBox->setButtonText(1, "Yes");
                    verifyQuitBox->setButtonText(2, "No" );
                    verifyQuitBox->show();

                if (verifyQuitBox->exec() == 1) {
                    access_network = 1;
                } else {
                    access_network = 0;
                }
            } else {
                access_network = 0;
            }
#else
            access_network = 0;
#endif
        } else {
            access_network = 1;
        }

        if (access_network == 0) {
            lic_n--;
            set_lic_n(lic_n, reg_info);
            time(&current_time);
        } else {
#if HAS_GUI
            static QMessageBox* AccessingNetworkBox = new QMessageBox( "WiSim",
                "Accessing Network ...", QMessageBox::Information,
                0, 0, 0, 0, 0, true);
            // qApp->eventLoop()->processEvents(QEventLoop::AllEvents, 1);
            AccessingNetworkBox->show();
            qApp->processEvents();
#endif

            int year, month, day;
            if (get_net_date(year, month, day)) {
                tm t;
                t.tm_sec = 0;            /* seconds */
                t.tm_min = 0;            /* minutes */
                t.tm_hour = 0;           /* hours */
                t.tm_mday = day;         /* day of the month */
                t.tm_mon  = month-1;     /* month */
                t.tm_year = year - 1900; /* year */
                t.tm_isdst = 0;          /* daylight saving time */
                current_time  = mktime(&t);
                flag = 1;
            } else {
                return(LIC_NO_NET_CONN);
            }

#if HAS_GUI
            AccessingNetworkBox->hide();
            delete AccessingNetworkBox;
#endif
        }
    } else {
        time(&current_time);
    }
    if ( (current_time < start_time) || (current_time > end_time) ) {
        return(LIC_EXPIRED);
    } else if (flag) {
        int num_days = (end_time - current_time) / (24*60*60) + 1;
        int lic_n = num_days * 6;
        if (lic_n > LIC_N) { lic_n = LIC_N; }
        set_lic_n(lic_n, reg_info);
    }

    return(LIC_VALID);
}

#ifdef __linux__
/******************************************************************************************/
/**** GET_ENVIRONMENT_VARIABLE: Linux Version                                          ****/
/******************************************************************************************/
QString get_environment_variable(char *varname)
{
    return QString(getenv(varname));
}
/******************************************************************************************/
#else
/******************************************************************************************/
/**** GET_ENVIRONMENT_VARIABLE: Windows Version                                        ****/
/**** Reads directly from registration table.                                          ****/
/******************************************************************************************/
QString get_environment_variable(char *varname)
{
    return read_registration_table("HKEY_CURRENT_USER\\Environment", varname);
}

/******************************************************************************************/
QString read_registration_table(char *path, char *varname)
{
    QString str;

    QSettings settings( str.fromAscii(path), QSettings::NativeFormat);

    return settings.value(str.fromAscii(varname)).toString();
}

void write_registration_table(char *path, char *varname, char *value)
{
    QSettings settings( QString(path), QSettings::NativeFormat);

    // check whether or not the varname is existed
    //qDebug() << settings.value(QString(varname)).toString();
    //if (settings.value(QString(varname)).isEmpty()) return;

    settings.setValue(QString(varname), QString(value));
}
#endif

#ifdef __linux__
/******************************************************************************************/
/**** GET MAC INFO: LINUX Version                                                      ****/
/******************************************************************************************/
void get_mac_info(unsigned char *reg_info)
{
    int s;
    struct ifreq buffer;

    s = socket(PF_INET, SOCK_DGRAM, 0);

    memset(&buffer, 0x00, sizeof(buffer));

    strcpy(buffer.ifr_name, "eth0");

    ioctl(s, SIOCGIFHWADDR, &buffer);

    close(s);

    // hex_to_hexstr(reg_info, (unsigned char *)buffer.ifr_hwaddr.sa_data, 6);
    memcpy( (void *) reg_info, (void *)buffer.ifr_hwaddr.sa_data, 6);
    // printf("MAC_INFO = %s\n",reg_info);

/*
    for( s = 0; s < 6; s++ ) {
        printf("%.2X ", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
    }
    printf("\n");
*/

    return;
}
/******************************************************************************************/
#else

/******************************************************************************************/
/**** GET MAC INFO: Windows Version                                                    ****/
/******************************************************************************************/
void get_mac_info(unsigned char *reg_info)
{
// CG deleted
}
/******************************************************************************************/

#endif

#ifdef __linux__
/******************************************************************************************/
/**** GET HD SrNum: Linux Version                                                    ****/
/******************************************************************************************/
void get_hd_srnum(unsigned char *reg_info)
{
    memset(reg_info, 0x00, 20);
}
#else

// CG deleted all

#endif

/******************************************************************************************/
/**** FUNCTION: gen_reg_file                                                           ****/
/******************************************************************************************/
int gen_reg_file(unsigned char *reg_info, int ris, char *name, char *email, char *company, char *&reg_file_rel, char *&reg_file_full)
{
    int i, retval;
    unsigned char ptr[LIC_NUM_BYTES];
    FILE *fp;
    BinIntClass x(LIC_NUM_BYTES/BYTES_PER_SEG);
    BinIntClass y(LIC_NUM_BYTES/BYTES_PER_SEG);

    int seed = ((int) time((time_t *) NULL)) & ((1<<25)-1);
    TRandomMersenne *rg = new TRandomMersenne(seed);

    memcpy( (void *) ptr, (void *) reg_info, ris);
    for (i=ris; i<=LIC_NUM_BYTES-1; i++) {
        ptr[i] = (i<=LIC_MS_BYTE_N-1 ? (unsigned char) floor(rg->Random()*256) : 0);
    }
    x.setval(ptr);

#if 0
    printf("RAW DATA:\n");
    x.printhex();
#endif

    scramble(y, x);

#if 0
    printf("ENCRYPTED DATA:\n");
    y.printhex();
#endif

    for (i=0; i <= (int) strlen(name)-1; i++) {
        if (name[i] == ' ') { name[i] = '_'; }
    }

#if 0
    printf("Name = %s\n", (const char *) name);
    printf("Email = %s\n", (const char *) email);
#endif

    reg_file_rel = CVECTOR(strlen(name) + strlen(".creg"));
    sprintf(reg_file_rel, "%s.creg", name);

#ifdef __linux__
    reg_file_full = strdup(reg_file_rel);
#else
    char *path;
    path = read_registration_table("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Desktop").toAscii().data();
    if (path == NULL) {
        reg_file_full = CVECTOR(strlen(path) + 1 + strlen(reg_file_rel));
        sprintf(reg_file_full, "%s\\%s.creg", path, name);
    } else {
        reg_file_full = strdup(reg_file_rel);
    }
#endif

    if ( (fp = fopen(reg_file_full, "wb")) ) {
        y.writebytestr(ptr);
        fwrite(ptr, 1, LIC_NUM_BYTES, fp);
        write_fs_str(fp, name);
        write_fs_str(fp, company);
        write_fs_str(fp, email);
        write_fs_str(fp, WISIM_RELEASE);
        fclose(fp);
        retval = 1;
    } else {
        retval = 0;
    }

    delete rg;
    return(retval);
}
/******************************************************************************************/
/**** GET_ENVIRONMENT_VARIABLE: get_net_date                                           ****/
/**** Get network date.                                                                ****/
/******************************************************************************************/
int get_net_date(int &year, int &month, int &day)
{
#define GENLOG 1

    int cont = 1;
#ifdef __linux__
    char *date_file = CVECTOR(strlen(WiSim_home) + 2 + strlen("IE6.log"));
    sprintf(date_file, "%s%cIE6.log", WiSim_home, FPATH_SEPARATOR);
    char *exefile = CVECTOR(strlen(WiSim_home) + 2 + strlen("mdi.pl"));
    sprintf(exefile, "%s%cmdi.pl", WiSim_home, FPATH_SEPARATOR);
#else
    char *date_file = strdup("C:\\IE6.log");
    char *exefile = CVECTOR(strlen(WiSim_home) + 2 + strlen("cs.dat"));
    sprintf(exefile, "%s%ccs.dat", WiSim_home, FPATH_SEPARATOR);
#endif

#if GENLOG
    char *logfile = CVECTOR(strlen(WiSim_home) + 2 + strlen("debug.log"));
    sprintf(logfile, "%s%cdebug.log", WiSim_home, FPATH_SEPARATOR);
#endif

    unsigned long uval;
    char *str = (char *) NULL;
    char *chptr, testch;
    FILE *fp = (FILE *) NULL;

#if GENLOG
    FILE *fl = (FILE *) NULL;
    fl = fopen(logfile, "wb");
#endif

    remove(date_file);
    if ( (fp = fopen(date_file, "rb")) ) {
        fclose(fp);
        fp = (FILE *) NULL;
        cont = 0;
#if GENLOG
        fprintf(fl, "ERROR DELETING DATE FILE\n");
#endif
    }

    if (cont) {
#ifdef __linux__
        system(exefile);
#else
//        _spawnl(_P_WAIT, exefile, " ", NULL);

        PROCESS_INFORMATION pi;
        STARTUPINFO si;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

 
        /**********************************************************************************/
        /**** Convert exefile to wide char                                             ****/
        /**********************************************************************************/
        int n = MultiByteToWideChar(
            CP_ACP,   // code page
            NULL,     // character-type options
            exefile,  // string to map
            -1,       // number of bytes in string
            NULL,     // wide-character buffer
            0         // size of buffer
        );

        LPWSTR wexefile = (LPWSTR) malloc(2*(n+1));

        MultiByteToWideChar(
            CP_ACP,   // code page
            NULL,     // character-type options
            exefile,  // string to map
            -1,       // number of bytes in string
            wexefile, // wide-character buffer
            n+1       // size of buffer
        );
        /**********************************************************************************/

        if(CreateProcess(wexefile, NULL, NULL, NULL, false, 0, NULL, NULL,&si, &pi)) {
            if(pi.hProcess)
                WaitForSingleObject(pi.hProcess, 120000);//two minutes

            DWORD exitcode;

            if(pi.hProcess) {
                GetExitCodeProcess( pi.hProcess, &exitcode );
                if( exitcode == STILL_ACTIVE ) {
                    TerminateProcess(pi.hProcess,9);
                }
                CloseHandle( pi.hProcess );
                CloseHandle( pi.hThread );
            }
        }

#endif

        str = (char *) malloc(3*sizeof(unsigned long));

        if ( !(fp = fopen(date_file, "rb")) ) {
#if GENLOG
            fprintf(fl, "ERROR OPENING DATE FILE\n");
#endif
            cont = 0;
        }
    }

    if ( (cont) && (fread(str, 1, 3*sizeof(unsigned long), fp) != 3*sizeof(unsigned long)) ) {
#if GENLOG
        fprintf(fl, "ERROR READ 1 DATE FILE\n");
#endif
        cont = 0;
    }
    if ( (cont) && (fread(&testch, 1, 1, fp) != 0) ) {
#if GENLOG
        fprintf(fl, "ERROR READ 2 DATE FILE\n");
#endif
        cont = 0;
    }
    if (fp) {
        fclose(fp);
    }

    remove(date_file);

    if (cont) {
        chptr = str;

        memcpy(&uval, chptr, sizeof(unsigned long));
        year = (int) ntohl(uval);
        chptr += sizeof(unsigned long);

        memcpy(&uval, chptr, sizeof(unsigned long));
        month = (int) ntohl(uval);
        chptr += sizeof(unsigned long);

        memcpy(&uval, chptr, sizeof(unsigned long));
        day = (int) ntohl(uval);
        chptr += sizeof(unsigned long);

#if GENLOG
        fprintf(fl, "YEAR = %d\n", year);
        fprintf(fl, "MONTH = %d\n", month);
        fprintf(fl, "DAY = %d\n", day);
#endif
    }

    if (str) {
        free(str);
    }

    if ( (cont) && ( (year < 2000) || (year > 2010) || (month < 1) || (month > 12) || (day < 1) || (day > 31) ) ) {
#if GENLOG
        fprintf(fl, "ERROR DATE FIELD\n");
#endif
        cont = 0;
    }

#if GENLOG
    fclose(fl);
#endif
    free(date_file);
    free(exefile);
    return(cont);

#undef GENLOG
}
/******************************************************************************************/
/**** FUNCTION: get_lic_n                                                              ****/
/**** Get lic_n = Num times can run without getting network time                       ****/
/******************************************************************************************/
int get_lic_n(unsigned char *reg_info)
{
    int i, n_msb, lic_n;
    BinIntClass d(44/BYTES_PER_SEG);
    BinIntClass n(44/BYTES_PER_SEG);
    BinIntClass c(44/BYTES_PER_SEG);
    BinIntClass x(44/BYTES_PER_SEG);
    BinIntClass **f;
    unsigned char lic_n_data[44], ptr[44];
    char *ascii_ptr;

    f = (BinIntClass **) malloc(BITS_PER_SEG*sizeof(BinIntClass *));
    for (i=0; i<=BITS_PER_SEG-1; i++) {
        f[i] = new BinIntClass(44/BYTES_PER_SEG);
    }

    d.setval((unsigned char *) "\xf7\xf8\x79\x56\xeb\x28\x56\x1d\x34\x82"
                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00");

    n.setval((unsigned char *) "\x95\x79\xf3\x3c\x1d\x1b\x98\xf2\x04\x4c"
                               "\x1e\x40\xf3\xff\xb5\x27\x22\x53\xe6\xf1"
                               "\xca\xa8\x01\xeb\xfd\xa9\x7e\x49\x06\x85"
                               "\xab\x42\x52\x97\x95\xad\x1d\x2c\x63\x56"
                               "\x00\x00\x00\x00");


//    d.setval((unsigned char *) "\xc1\x91\xb8\xc6\x8d\xf4\x1b\x00\x00\x00"
//                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//                               "\x00\x00");

//    n.setval((unsigned char *) "\xc9\xcb\xed\x3f\x49\xe6\xc9\x29\x35\xf3"
//                               "\x28\xf1\xa6\x86\x4a\x32\x44\x8c\x04\xd2"
//                               "\xdf\xa9\x12\xf1\x5d\x9b\x0d\x00\x00\x00"
//                               "\x00\x00");

#ifndef __linux__
    ascii_ptr = read_registration_table("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion", "MsData").toAscii().data();
#else
    ascii_ptr = CVECTOR(88);
    FILE *fp;
    if ( !(fp = fopen("/tmp/lic_n.dat", "rb")) ) {
        printf("get_lic_n(): Can't read from file\n");
        return(-1);
    }

    if (fread(ascii_ptr, sizeof(unsigned char), 88, fp) != 88) {
        fclose(fp);
        printf("get_lic_n(): File size != 82 bytes\n");
        return(-1);
    }
    fclose(fp);
#endif

    for (i=0; i<=43; i++) {
        lic_n_data[i] = ((ascii_ptr[2*i]-'A') << 4) | (ascii_ptr[2*i+1]-'A');
    }
    free(ascii_ptr);

    c.setval(lic_n_data);

    n.comp_f(n_msb, f);

#if 0
    printf("\n\n");
    printf("C: "); c.printhex();
    printf("D: "); d.printhex();
    printf("N: "); n.printhex();
#endif

    x.exp_mod(c, d, n, n_msb, f);

//  printf("X: "); x.printhex();

    x.writebytestr(ptr);

    if (memcmp(ptr, reg_info, 28) != 0) {
        printf("get_lic_n(): File INVALID\n");
        return(-1);
    }

    memcpy(&lic_n, ptr+34, sizeof(unsigned long));
    lic_n = (int) ntohl((unsigned long) lic_n);

    for (i=0; i<=BITS_PER_SEG-1; i++) {
        delete(f[i]);
    }
    free(f);

    printf("READ LIC_N VAL = %d\n", lic_n);
    return(lic_n);
}
/******************************************************************************************/
/**** FUNCTION: set_lic_n                                                              ****/
/**** Set lic_n                                                                        ****/
/******************************************************************************************/
void set_lic_n(int lic_n, unsigned char *reg_info)
{

    int i, n_msb, n_lic_n;
    BinIntClass e(44/BYTES_PER_SEG);
    BinIntClass n(44/BYTES_PER_SEG);
    BinIntClass c(44/BYTES_PER_SEG);
    BinIntClass m(44/BYTES_PER_SEG);
    BinIntClass **f;
    unsigned char ptr[45];
    char ascii_ptr[89];

    printf("set_lic_n(): Setting value to %d\n", lic_n);

    f = (BinIntClass **) malloc(BITS_PER_SEG*sizeof(BinIntClass *));
    for (i=0; i<=BITS_PER_SEG-1; i++) {
        f[i] = new BinIntClass(44/BYTES_PER_SEG);
    }

    e.setval((unsigned char *) "\x47\x73\xb4\xa9\xbf\x22\x86\xf4\x31\xa2"
                               "\x46\xa6\x4b\x0e\x5b\x75\xa6\x98\x88\xfe"
                               "\xfb\xea\xdf\xde\xf3\x72\x23\xcb\x59\x3d"
                               "\xbf\xd7\x83\x49\x35\x49\xdc\x15\xc5\x23"
                               "\x00\x00\x00\x00");

    n.setval((unsigned char *) "\x95\x79\xf3\x3c\x1d\x1b\x98\xf2\x04\x4c"
                               "\x1e\x40\xf3\xff\xb5\x27\x22\x53\xe6\xf1"
                               "\xca\xa8\x01\xeb\xfd\xa9\x7e\x49\x06\x85"
                               "\xab\x42\x52\x97\x95\xad\x1d\x2c\x63\x56"
                               "\x00\x00\x00\x00");


//    e.setval((unsigned char *) "\xd9\x5f\xbd\xf7\xa3\x66\xb7\x94\x4f\xb2"
//                               "\xa7\x41\xeb\xc5\xf7\x82\x86\x01\xb9\xa3"
//                               "\x3a\x42\xf8\xd6\x8d\x73\x06\x00\x00\x00"
//                               "\x00\x00");

//    n.setval((unsigned char *) "\xc9\xcb\xed\x3f\x49\xe6\xc9\x29\x35\xf3"
//                               "\x28\xf1\xa6\x86\x4a\x32\x44\x8c\x04\xd2"
//                               "\xdf\xa9\x12\xf1\x5d\x9b\x0d\x00\x00\x00"
//                               "\x00\x00");

    memcpy(ptr, reg_info, 28);

    int seed = ((int) time((time_t *) NULL)) & ((1<<25)-1);
    TRandomMersenne *rg = new TRandomMersenne(seed);
    for (i=28; i<=33; i++) {
        ptr[i] = (unsigned char) floor(rg->Random()*256);
    }
    delete rg;

    n_lic_n = htonl((unsigned long) lic_n);
    memcpy(ptr+34, (const void *) &n_lic_n, sizeof(unsigned long));

    for (i=38; i<=43; i++) {
        ptr[i] = (unsigned char) 0;
    }

    m.setval(ptr);

    n.comp_f(n_msb, f);

#if 0
    printf("M: "); m.printhex();
    printf("E: "); e.printhex();
    printf("N: "); n.printhex();
#endif

    c.exp_mod(m, e, n, n_msb, f);

//  printf("C: "); c.printhex();

    c.writebytestr(ptr);

    for (i=0; i<=43; i++) {
        ascii_ptr[2*i  ] = 'A' + ( (ptr[i] >> 4) & 0x0F );
        ascii_ptr[2*i+1] = 'A' + ( (ptr[i]     ) & 0x0F );
    }
    ascii_ptr[88] = (char) NULL;

#ifndef __linux__
    write_registration_table("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion", "MsData", ascii_ptr);
#else
    FILE *fp;
    if (!(fp = fopen("/tmp/lic_n.dat", "wb"))) {
        return;
    }
    fwrite(ascii_ptr, 1, 88, fp);
    fclose(fp);
#endif

    for (i=0; i<=BITS_PER_SEG-1; i++) {
        delete(f[i]);
    }
    free(f);

    return;
}
/******************************************************************************************/

// Show dos cmd window to redirect information
#ifndef __linux__
void InitConsoleWindow()
{
    int hCrt;
    FILE *hf;
    AllocConsole();
    hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT );
    hf = _fdopen( hCrt, "w" );
    *stdout = *hf;
    setvbuf( stdout, NULL, _IONBF, 0 );

    printf(" ------------ Launch Console Window ------------ \n\n");
}
#endif

