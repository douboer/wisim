#ifndef MENU_STRUCTURE_H
#define MENU_STRUCTURE_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class QToolButton;
class Q3PopupMenu;
class QMenuBar;
class MainWindowClass;
class NetworkClass;

/******************************************************************************************/
/**** CLASS: MenuStructureClass                                                        ****/
/******************************************************************************************/
class MenuStructureClass : public QObject
{
    Q_OBJECT

public:
    MenuStructureClass(MainWindowClass *main_window, NetworkClass *np_param);
    ~MenuStructureClass();
    void gui_mode_changed(MainWindowClass *mw);
    friend class MainWindowClass;

private:
    NetworkClass *np;
    //--------------file menu item---------------//
    Q3PopupMenu* file_menu;
    
    int read_geom_ID;
    int save_geom_ID;
    int display_excel_geometry_ID;
    int read_geom_db_ID;
    int save_geom_db_ID;
    int create_geom_ID;
    int close_geom_ID;

    int read_cch_rssi_ID;
    int read_road_test_data_ID;
    int save_road_test_data_ID;
    int convert_road_test_data_ID;
    int check_road_test_data_ID;
    int save_coverage_analysis_ID;
    int read_coverage_analysis_ID;
    int file_print_ID;
//    int file_print_bmp_ID;
//    int file_print_jpg_ID;

    QToolButton *read_geom_TB;
    QToolButton *close_geom_TB;
    QToolButton *save_geom_TB;
    QToolButton *file_print_TB;

    //--------------edit menu item---------------//
    Q3PopupMenu* edit_menu;

    int find_ID;
    int add_cell_ID;
    int add_cell_at_ID;
    int add_subnet_ID;
    int edit_system_bdy_ID;
    int shift_map_layer_ID;
    int shift_map_background_ID;
    int shift_road_test_data_ID;
    int extract_simulation_region_ID;
    
    //------------- view menu item---------------//
    Q3PopupMenu* view_menu;

    int command_window_ID;
    int visibility_window_ID;
    int info_window_ID;
    int openhand_ID;
    int zoomin_ID;
    int zoomout_ID;
    int zoomToFit_ID;
    QToolButton *command_window_TB;
    QToolButton *visibility_window_TB;
    QToolButton *info_window_TB;
    QToolButton *openhand_TB;
    QToolButton *zoomin_TB;
    QToolButton *zoomout_TB;
    QToolButton *zoomToFit_TB;    

    //------------- traffic menu item---------------//
  //  QPopupMenu* traffic_menu;
 //   int subnet_traffic_dia_ID;

    //--------------mode menu item---------------//
    Q3PopupMenu* mode_menu;	
    
    int edit_mode_ID;
    int simulate_mode_ID;

    //--------------map menu item---------------//
    Q3PopupMenu* map_menu;
    int read_map_layer_ID;
    int save_map_layer_ID;
    int read_mif_ID;
    int read_map_clutter_ID;
    int save_map_clutter_ID;
    // int read_map_height_ID;
    int read_map_background_ID;
    int image_registration_ID;

    //--------------Propagation menu item---------//
    Q3PopupMenu* propagation_menu;
    int prop_model_mgr_ID;
    int prop_analysis_ID;
    int import_clutter_model_ID;
    int export_clutter_model_ID;
    
    //--------------subnet menu item---------//
    Q3PopupMenu* subnet_menu;
    int subnet_traffic_dia_ID;
    int create_subnet_dia_ID;
    int delete_all_subnet_ID;

    //--------------monte_carlo menu item---------//
    Q3PopupMenu* monte_carlo_menu;
    int set_param_ID;
    int monte_carlo_ID;
    int run_match_ID;
    int import_st_data_ID;
    int delete_st_data_ID;
        
    //--------------coverage menu item---------//
    Q3PopupMenu* coverge_menu;
    int coverage_ID;

    //--------------report menu item---------//
    Q3PopupMenu* report_menu;
    int report_coverage_ID;
    int plot_coverage_ID;
    int report_subnets_ID;
    int report_prop_error_ID;
    int report_prop_param_ID;
    int report_settings_ID;
    int statistics_ID;

    //-------------- coverage menu item---------//
    Q3PopupMenu* coverage_menu;

    //--------------Handover menu item---------------//
    Q3PopupMenu* handover_menu;
    int read_handover_ID;    

    //--------------help menu item---------------//
    Q3PopupMenu* help_menu;    

    void reset_visibility_window();

private slots:
    void delete_all_subnets();
};
/******************************************************************************************/

#endif
