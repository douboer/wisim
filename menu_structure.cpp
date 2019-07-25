/******************************************************************************************/
/**** FILE: menu_structure.cpp                                                         ****/
/******************************************************************************************/

#include <qapplication.h>
#include <qcursor.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PopupMenu>

#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "icons.h"
#include "list.h"
#include "main_window.h"
#include "menu_structure.h"
#include "prop_model.h"
#include "sectorparamDia.h"
#include "showhandover.h"

MenuStructureClass *MainWindowClass::create_menu_structure()      { return(new MenuStructureClass(this, np)); }
void                MainWindowClass::gui_mode_changed_msc()       { msc->gui_mode_changed(this); }

bool MainWindowClass::view_menu_is_checked(int win_type)
{
    bool checked;

    switch(win_type) {
        case GConst::commandWindow:
            checked = msc->view_menu->isItemChecked(msc->command_window_ID);
            break;
        case GConst::visibilityWindow:
            checked = msc->view_menu->isItemChecked(msc->visibility_window_ID);
            break;
        case GConst::infoWindow:
            checked = msc->view_menu->isItemChecked(msc->info_window_ID);
            break;
        default:
            CORE_DUMP;
    }

    return(checked);
}

void MainWindowClass::view_menu_set_checked(int win_type, bool checked)
{
    int win_id;
    QToolButton *tb;

    switch(win_type) {
        case GConst::commandWindow:    win_id = msc->command_window_ID;    tb = msc->command_window_TB;    break;
        case GConst::visibilityWindow: win_id = msc->visibility_window_ID; tb = msc->visibility_window_TB; break;
        case GConst::infoWindow:       win_id = msc->info_window_ID;       tb = msc->info_window_TB;       break;
        default:
            CORE_DUMP;
    }

    msc->view_menu->setItemChecked(win_id, checked);
    tb->setOn(checked);
}
/******************************************************************************************/
/**** FUNCTION: MenuStructureClass::MenuStructureClass                                 ****/
/******************************************************************************************/
MenuStructureClass::MenuStructureClass(MainWindowClass *mw, NetworkClass *np_param)
{

    QMenuBar* menu = mw->menuBar();
    np = np_param;

    /**************************************************************************************/
    /**** Toolbar                                                                      ****/
    /**************************************************************************************/
    mw->fileTools = new QToolBar( mw, "file operations" );
    mw->fileTools->setLabel( "File Operations" );

    QIcon fileopen_iconset;
    fileopen_iconset.setPixmap(QPixmap(XpmIcon::fileopen_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    fileopen_iconset.setPixmap(QPixmap(XpmIcon::fileopen_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    read_geom_TB = new QToolButton( fileopen_iconset, tr("Read Geometry"), QString::null,
                           mw, SLOT(execute_read_file()), mw->fileTools, "read geometry" );

    QIcon fileclose_iconset;
    fileclose_iconset.setPixmap(QPixmap(XpmIcon::fileclose_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    fileclose_iconset.setPixmap(QPixmap(XpmIcon::fileclose_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    close_geom_TB = new QToolButton( fileclose_iconset, tr("Close Geometry"), QString::null,
                           mw, SLOT(execute_close_geometry()), mw->fileTools, "close geometry" );

    QIcon filesave_iconset;
    filesave_iconset.setPixmap(QPixmap(XpmIcon::filesave_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    filesave_iconset.setPixmap(QPixmap(XpmIcon::filesave_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    save_geom_TB = new QToolButton( filesave_iconset, tr("Save Geometry"), QString::null,
                           mw, SLOT(execute_save_file()), mw->fileTools, "save geometry" );

    QIcon fileprint_iconset;
    fileprint_iconset.setPixmap(QPixmap(XpmIcon::fileprint_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    fileprint_iconset.setPixmap(QPixmap(XpmIcon::fileprint_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    file_print_TB = new QToolButton( fileprint_iconset, tr("Print"), QString::null,
                           mw->editor, SLOT(print()), mw->fileTools, "print" );

    QIcon openhand_iconset;
    openhand_iconset.setPixmap(QPixmap(XpmIcon::openhand_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    openhand_iconset.setPixmap(QPixmap(XpmIcon::openhand_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    openhand_TB = new QToolButton( openhand_iconset, tr("Scroll"), QString::null,
                           mw->editor, SLOT(setScrollMode()), mw->fileTools, "Scroll" );

    QIcon zoomin_iconset;
    zoomin_iconset.setPixmap(QPixmap(XpmIcon::zoomin_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    zoomin_iconset.setPixmap(QPixmap(XpmIcon::zoomin_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    zoomin_TB = new QToolButton( zoomin_iconset, tr("Zoom in"), QString::null,
                           mw->editor, SLOT(setZoomMode()), mw->fileTools, "Zoom in" );

    QIcon zoomout_iconset;
    zoomout_iconset.setPixmap(QPixmap(XpmIcon::zoomout_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    zoomout_iconset.setPixmap(QPixmap(XpmIcon::zoomout_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    zoomout_TB = new QToolButton( zoomout_iconset, tr("Zoom out"), QString::null,
                           mw->editor, SLOT(zoomOut()), mw->fileTools, "Zoom out" );

    QIcon zoomToFit_iconset;
    zoomToFit_iconset.setPixmap(QPixmap(XpmIcon::zoomToFit_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    zoomToFit_iconset.setPixmap(QPixmap(XpmIcon::zoomToFit_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    zoomToFit_TB = new QToolButton( zoomToFit_iconset, tr("zoomToFit"), QString::null,
                           mw->editor, SLOT(zoomToFit()), mw->fileTools, "zoomToFit" );

    QIcon redeye_iconset;
    redeye_iconset.setPixmap(QPixmap(XpmIcon::redeye_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    redeye_iconset.setPixmap(QPixmap(XpmIcon::redeye_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    visibility_window_TB = new QToolButton( redeye_iconset, tr("Visibility Window"), QString::null,
                     mw, SLOT(toggle_visibility_window()), mw->fileTools, "Visibility Window" );
    visibility_window_TB->setToggleButton(true);

    QIcon info_win_iconset;
    info_win_iconset.setPixmap(QPixmap(XpmIcon::info_win_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::On);
    info_win_iconset.setPixmap(QPixmap(XpmIcon::info_win_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::On);
    info_window_TB = new QToolButton( info_win_iconset, tr("Info Window"), QString::null,
                     mw, SLOT(toggle_info_window()), mw->fileTools, "Info Window" );
    info_window_TB->setToggleButton(true);

    QIcon cmd_win_iconset;
    cmd_win_iconset.setPixmap(QPixmap(XpmIcon::cmd_win_icon_normal),   QIcon::Large, QIcon::Normal,   QIcon::Off);
    cmd_win_iconset.setPixmap(QPixmap(XpmIcon::cmd_win_icon_disabled), QIcon::Large, QIcon::Disabled, QIcon::Off);
    command_window_TB = new QToolButton( cmd_win_iconset, tr("Command Window"), QString::null,
                     mw, SLOT(toggle_command_window()), mw->fileTools, "Command Window" );
    command_window_TB->setToggleButton(true);

    //mw->moveDockWindow ( mw->fileTools, Qt::DockLeft );
    mw->addToolBar( Qt::LeftToolBarArea, mw->fileTools);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: File Menu                                                             ****/
    /**************************************************************************************/
    file_menu = new Q3PopupMenu( menu );
    read_geom_ID = file_menu->insertItem( fileopen_iconset, tr("&Read Geometry") + " ...", mw, SLOT(execute_read_file(int)),       Qt::CTRL+Qt::Key_R);
    file_menu->setItemParameter(read_geom_ID,  GConst::geometryFile);

    save_geom_ID = file_menu->insertItem( filesave_iconset, tr("&Save Geometry") + " ...", mw, SLOT(execute_save_file(int)),       Qt::CTRL+Qt::Key_S);
    file_menu->setItemParameter(save_geom_ID,  GConst::geometryFile);

    close_geom_ID = file_menu->insertItem(fileclose_iconset, tr("&Close Geometry"),            mw, SLOT(execute_close_geometry()));
    file_menu->insertSeparator();

#if HAS_ORACLE
    read_geom_db_ID = file_menu->insertItem(fileopen_iconset, tr("Read Geometry DB") + " ...", mw, SLOT(execute_read_geometry_db()));
    save_geom_db_ID = file_menu->insertItem(filesave_iconset, tr("Save Geometry DB") + " ...", mw, SLOT(execute_save_geometry_db()));
    file_menu->insertSeparator();
#endif

#ifndef __linux__
    display_excel_geometry_ID = file_menu->insertItem(tr("Import Excel Geometry") + " ...", mw, SLOT(execute_display_excel_geometry()),Qt::CTRL+Qt::Key_E);   //dos
    file_menu->insertSeparator();
#if (DEMO)
    file_menu->setItemEnabled(display_excel_geometry_ID, false);
#endif
#endif
    file_menu->insertItem(tr("&Include") + " ...",               mw, SLOT(execute_include()),             Qt::CTRL+Qt::Key_I);
    file_menu->insertSeparator();

#if 0
    read_cch_rssi_ID    = file_menu->insertItem(tr("Read CCH RSSI T&able") + " ...", mw, SLOT(execute_read_file(int)), Qt::CTRL+Qt::Key_A);
    file_menu->setItemParameter(read_cch_rssi_ID,  GConst::cchRSSITableFile);
#endif

    read_road_test_data_ID = file_menu->insertItem(tr("Read Road Test &Data") + " ...",   mw, SLOT(execute_read_file(int)), Qt::CTRL+Qt::Key_D);
    file_menu->setItemParameter(read_road_test_data_ID,  GConst::roadTestDataFile);

    save_road_test_data_ID = file_menu->insertItem(tr("Save Road Test Data") + " ...",   mw, SLOT(execute_save_file(int)));
    file_menu->setItemParameter(save_road_test_data_ID,  GConst::roadTestDataFile);

    convert_road_test_data_ID = file_menu->insertItem(tr("Convert Road Test Data") + " ...",   mw->editor, SLOT(create_convert_road_test_data_dialog()));

    check_road_test_data_ID = file_menu->insertItem(tr("Check Road Test Data") + " ...",   mw, SLOT(execute_check_road_test_data()));
    file_menu->insertSeparator();

    read_coverage_analysis_ID    = file_menu->insertItem(tr("Read Coverage Analysis") + " ...", mw, SLOT(execute_read_file(int)));
    file_menu->setItemParameter(read_coverage_analysis_ID,  GConst::coverageAnalysisFile);

    save_coverage_analysis_ID   = file_menu->insertItem(tr("Save Coverage Analysis") + " ...", mw->editor, SLOT(create_report_dialog(int)));
    file_menu->setItemParameter(save_coverage_analysis_ID, GConst::coverageAnalysisFile);

    file_menu->insertSeparator();
    file_print_ID = file_menu->insertItem(fileprint_iconset, tr("&Print") + " ...", mw->editor, SLOT(print()), Qt::CTRL+Qt::Key_P);

//    file_print_bmp_ID = file_menu->insertItem(tr("Print BMP") + " ...", mw, SLOT(execute_save_file(int)));
//    file_menu->setItemParameter(file_print_bmp_ID,  GConst::BMPFile);

//    file_print_jpg_ID = file_menu->insertItem(tr("Print JPG") + " ...", mw, SLOT(execute_save_file(int)));
//    file_menu->setItemParameter(file_print_jpg_ID,  GConst::JPGFile);

    file_menu->insertSeparator();
    file_menu->insertItem(tr("E&xit"), mw, SLOT( execute_gui_exit() ));

    menu->insertItem( tr("&File"), file_menu);
    /**************************************************************************************/
    /**** END: File Menu                                                               ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Edit Menu                                                             ****/
    /**************************************************************************************/
    edit_menu = new Q3PopupMenu( menu );

    find_ID = edit_menu->insertItem(tr("&Find")+"...",mw->editor, SLOT(create_find_dialog()),     Qt::CTRL+Qt::Key_F);
    edit_menu->insertSeparator();

    add_cell_ID = edit_menu->insertItem(tr("Add Cell"), mw->editor, SLOT(setMouseMode(int)));
    edit_menu->setItemParameter(add_cell_ID,  GConst::addCellMode);

    add_cell_at_ID = edit_menu->insertItem(tr("Add Cell at") + " ...", mw, SLOT(execute_add_cell_at()));
    edit_menu->insertSeparator();
#if HAS_MONTE_CARLO
    add_subnet_ID = edit_menu->insertItem(tr("Add Subnet"), mw->editor, SLOT(create_draw_polygon_dialog(int)));
    edit_menu->setItemParameter(add_subnet_ID, GConst::subnetRTTI);
    edit_menu->insertSeparator();
#endif

    edit_system_bdy_ID = edit_menu->insertItem(tr("Edit System Boundary"), mw->editor, SLOT(create_draw_polygon_dialog(int)));
    edit_menu->setItemParameter(edit_system_bdy_ID, GConst::systemBoundaryRTTI);

    edit_menu->insertSeparator();

    /* On windows, execute cre_geo.exe program */
    extract_simulation_region_ID = edit_menu->insertItem(tr("Extract Simulation Region"), mw->editor, SLOT(create_draw_polygon_dialog(int)));
    edit_menu->setItemParameter(extract_simulation_region_ID, GConst::polygonRegionRTTI);

    edit_menu->insertSeparator();
    shift_map_layer_ID = edit_menu->insertItem(tr("Shift Map Layer"), mw->editor, SLOT(create_shift_dialog(int)));
    edit_menu->setItemParameter(shift_map_layer_ID, GConst::mapLayerRTTI);
    shift_map_background_ID = edit_menu->insertItem(tr("Shift Map Background"), mw->editor, SLOT(create_shift_dialog(int)));
    edit_menu->setItemParameter(shift_map_background_ID, GConst::mapBackgroundRTTI);
    shift_road_test_data_ID = edit_menu->insertItem(tr("Shift Road Test Data"), mw->editor, SLOT(create_shift_dialog(int)));
    edit_menu->setItemParameter(shift_road_test_data_ID, GConst::roadTestDataRTTI);
    edit_menu->insertSeparator();
    edit_menu->insertItem(tr("&Preferences") + " ...", mw->editor, SLOT(create_pref_dialog()));

    menu->insertItem(tr("&Edit"), edit_menu);
    /**************************************************************************************/
    /**** END: Edit Menu                                                               ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: View Menu                                                             ****/
    /**************************************************************************************/
    view_menu = new Q3PopupMenu( menu );

    openhand_ID = view_menu->insertItem(openhand_iconset, tr("Scroll") + " ...",  mw->editor, SLOT(setMouseMode(int)));
    view_menu->setItemParameter(openhand_ID, GConst::scrollMode);

    zoomin_ID = view_menu->insertItem(zoomin_iconset, tr("Zoom &in") + " ...",                   mw->editor, SLOT(setMouseMode(int)));
    view_menu->setItemParameter(zoomin_ID, GConst::zoomMode);

    zoomout_ID = view_menu->insertItem(zoomout_iconset,  tr("Zoom &out"),    mw->editor, SLOT(zoomOut()));

    zoomToFit_ID = view_menu->insertItem(zoomToFit_iconset,  tr("Zoom to &Fit"),    mw->editor, SLOT(zoomToFit()));
    view_menu->insertSeparator();

    command_window_ID    = view_menu->insertItem(tr("Command Window"),    mw, SLOT(toggle_command_window()));
    visibility_window_ID = view_menu->insertItem(tr("Visibility Window"), mw, SLOT(toggle_visibility_window()));
    info_window_ID       = view_menu->insertItem(tr("Infomation Window"),       mw, SLOT(toggle_info_window()));

    view_menu->setItemChecked( command_window_ID,    false);
    view_menu->setItemChecked( visibility_window_ID, false);
    view_menu->setItemChecked( info_window_ID, false);
//    view_menu->insertItem(tr("Tools"), tools);

    menu->insertItem(tr("&View"), view_menu);
    /**************************************************************************************/
    /**** END: View Menu                                                               ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Traffic Menu                                                          ****/
    /**************************************************************************************/
//    traffic_menu = new QPopupMenu( menu );
//    subnet_traffic_dia_ID = traffic_menu->insertItem(tr("Subnet &Traffic"), mw->editor, SLOT( create_subnet_traffic_dialog() ), Qt::ALT+Qt::Key_T);

//    menu->insertItem(tr("&Traffic"), traffic_menu);
    /**************************************************************************************/
    /**** END: Traffic Menu                                                            ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Mode                                                                  ****/
    /**************************************************************************************/
    mode_menu = new Q3PopupMenu( menu );
    edit_mode_ID = mode_menu->insertItem(tr("&Edit"),       mw, SLOT(execute_set_mode(int)), Qt::Key_F2);
    mode_menu->setItemParameter(edit_mode_ID, CConst::editGeomMode);
    simulate_mode_ID = mode_menu->insertItem(tr("&Simulation"), mw, SLOT(execute_set_mode(int)), Qt::Key_F3);
    mode_menu->setItemParameter(simulate_mode_ID, CConst::simulateMode);

    menu->insertItem(tr("&Mode"), mode_menu);
    /**************************************************************************************/
    /**** END: Mode                                                                    ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Map Menu                                                      ****/
    /**************************************************************************************/
    map_menu = new Q3PopupMenu( menu );

    read_map_layer_ID   = map_menu->insertItem(tr("Read Map &Layer") + " ...",      mw, SLOT(execute_read_file(int)),      Qt::CTRL+Qt::Key_L);
    map_menu->setItemParameter(read_map_layer_ID,  GConst::mapLayerFile);

    save_map_layer_ID = map_menu->insertItem( filesave_iconset, tr("Save Map Layer") + " ...",  mw->editor, SLOT(create_report_dialog(int)));
    map_menu->setItemParameter(save_map_layer_ID,  GConst::mapLayerFile);
    map_menu->insertSeparator();

    read_mif_ID         = map_menu->insertItem(tr("Import MIF") + " ...",             mw->editor, SLOT(create_read_mif_dialog()),  Qt::CTRL+Qt::Key_M);
    map_menu->insertSeparator();
#if HAS_CLUTTER
// #if CDEBUG
    read_map_clutter_ID = map_menu->insertItem(tr("Read Map Clutter") + " ...",     mw, SLOT(execute_read_file(int)));
    map_menu->setItemParameter(read_map_clutter_ID,  GConst::mapClutterFile);

    save_map_clutter_ID = map_menu->insertItem(tr("Save Map Clutter") + " ...",     mw, SLOT(execute_save_file(int)));
    map_menu->setItemParameter(save_map_clutter_ID,  GConst::mapClutterFile);
    map_menu->insertSeparator();
// #endif
#endif
//    read_map_height_ID  = map_menu->insertItem(tr("Read Map Height") + " ...",      mw, SLOT(execute_read_file(int)));
//    map_menu->setItemParameter(read_map_height_ID,  GConst::mapHeightFile);
//    map_menu->insertSeparator();

    read_map_background_ID  = map_menu->insertItem(tr("Read Map Background") + " ...",     mw, SLOT(execute_read_file(int)));
    map_menu->setItemParameter(read_map_background_ID,  GConst::mapBackgroundTabFile);

    image_registration_ID  = map_menu->insertItem(tr("Image Registration") + " ...",     mw, SLOT(execute_read_file(int)));
    map_menu->setItemParameter(image_registration_ID,  GConst::imageRegistrationFile);

    menu->insertItem(tr("M&ap"),map_menu);
    /**************************************************************************************/
    /**** END: Map Menu                                                        ****/
    /**************************************************************************************/


    /**************************************************************************************/
    /**** BEGIN: Propagation Menu                                                      ****/
    /**************************************************************************************/
    propagation_menu = new Q3PopupMenu( menu );

    prop_model_mgr_ID = propagation_menu->insertItem(tr("Propagation Model Manager"), mw->editor, SLOT(create_prop_mgr_dialog()));

    propagation_menu->insertSeparator();
    prop_analysis_ID = propagation_menu->insertItem(tr("Compute Propagation Models"), mw->editor, SLOT(create_prop_analysis_dialog()));
    propagation_menu->insertSeparator();
    propagation_menu->insertSeparator();
#if HAS_CLUTTER
    import_clutter_model_ID = propagation_menu->insertItem(tr("Import Clutter Propagation Model"), mw, SLOT(execute_read_file(int)));
    propagation_menu->setItemParameter(import_clutter_model_ID,  GConst::clutterPropModelFile);
    export_clutter_model_ID = propagation_menu->insertItem(tr("Export Clutter Propagation Model"), mw->editor, SLOT(create_report_dialog(int)));
    propagation_menu->setItemParameter(export_clutter_model_ID,  GConst::clutterPropModelFile);
#endif
    menu->insertItem(tr("&Propagation"), propagation_menu);
    /**************************************************************************************/
    /**** END: Propagation Menu                                                        ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: subnet Menu                                                       ****/
    /**************************************************************************************/
    subnet_menu = new Q3PopupMenu( menu );

#if HAS_MONTE_CARLO
    subnet_traffic_dia_ID = subnet_menu->insertItem(tr("Subnet &Traffic"), mw->editor, SLOT( create_subnet_traffic_dialog() ), Qt::ALT+Qt::Key_T);
    subnet_menu->insertSeparator();
#endif

    create_subnet_dia_ID = subnet_menu->insertItem(tr("C&reate Subnets"), mw->editor, SLOT(create_create_subnet_dialog()),Qt::ALT+Qt::Key_R );
    delete_all_subnet_ID = subnet_menu->insertItem(tr("D&elete all Subnets"), this, SLOT(delete_all_subnets()));

    menu->insertItem(tr("&Subnet"), subnet_menu);
    /**************************************************************************************/
    /**** END: subnet Menu                                                         ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: monte_carlo Menu                                                       ****/
    /**************************************************************************************/
#if HAS_MONTE_CARLO
    monte_carlo_menu = new Q3PopupMenu( menu );
    set_param_ID = monte_carlo_menu->insertItem(tr("S&ystem Parameters"), mw->editor, SLOT(create_set_param_dialog()), Qt::ALT+Qt::Key_Y);

    monte_carlo_menu->insertSeparator();
    monte_carlo_ID  = monte_carlo_menu->insertItem(tr("&Monte-Carlo"), mw->editor, SLOT(create_run_simulation_dialog()), Qt::Key_F4);
    menu->insertItem(tr("&Monte_carlo"), monte_carlo_menu);

    monte_carlo_menu->insertSeparator();
    import_st_data_ID  = monte_carlo_menu->insertItem(tr("&Import ST Data"), mw->editor, SLOT(create_import_st_data_dialog()));
    delete_st_data_ID  = monte_carlo_menu->insertItem(tr("&Delete ST Data"), mw->editor, SLOT(delete_st_data()));

    monte_carlo_menu->insertSeparator();
    run_match_ID  = monte_carlo_menu->insertItem(tr("Run M&atch"), mw->editor, SLOT(run_match()));

#endif
    /**************************************************************************************/
    /**** END: monte_carlo Menu                                                         ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: coverage Menu                                                       ****/
    /**************************************************************************************/
    coverage_menu = new Q3PopupMenu( menu );
    coverage_ID     = coverage_menu->insertItem(tr("Coverage"), mw->editor, SLOT(create_cvg_mgr_dialog()), Qt::ALT+Qt::Key_C);
    menu->insertItem(tr("Coverage"), coverage_menu);
    /**************************************************************************************/
    /**** END: coverage Menu                                                         ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Report Menu                                                           ****/
    /**************************************************************************************/
    report_menu = new Q3PopupMenu( menu );

    report_coverage_ID   = report_menu->insertItem(tr("Coverage"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(report_coverage_ID, GConst::coverageReport);

    plot_coverage_ID     = report_menu->insertItem(tr("Coverage Plot"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(plot_coverage_ID, GConst::coveragePlot);

    report_subnets_ID   = report_menu->insertItem(tr("Subnets"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(report_subnets_ID, GConst::subnetReport);

    report_prop_error_ID   = report_menu->insertItem(tr("Propagation Error"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(report_prop_error_ID, GConst::propagationErrorReport);

    report_prop_param_ID   = report_menu->insertItem(tr("Propagation Parameters"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(report_prop_param_ID, GConst::propagationParamReport);
#if HAS_MONTE_CARLO
    statistics_ID = report_menu->insertItem(tr("Statistics"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(statistics_ID, GConst::statisticsReport);

    report_settings_ID   = report_menu->insertItem(tr("Settings"), mw->editor, SLOT(create_report_dialog(int)));
    report_menu->setItemParameter(report_settings_ID, GConst::settingsReport);
#endif

    menu->insertItem(tr("Reports"), report_menu);
    /**************************************************************************************/
    /**** END: Report Menu                                                             ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Handover Menu                                                             ****/
    /**************************************************************************************/
    handover_menu = new Q3PopupMenu( menu );
    read_handover_ID = handover_menu->insertItem( fileopen_iconset, tr("&Read Handover Data") + " ...", mw, SLOT(execute_read_file(int)));
    handover_menu->setItemParameter(read_handover_ID,  GConst::handoverFile);
    menu->insertItem(tr("&Handover"),handover_menu);
    /**************************************************************************************/
    /**** END: Handover Menu                                                               ****/
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Help Menu                                                             ****/
    /**************************************************************************************/
    help_menu = new Q3PopupMenu( menu );
    help_menu->insertItem(tr("&About WiSim"), mw, SLOT(help()), Qt::Key_F1);
    menu->insertItem(tr("&Help"),help_menu);
    /**************************************************************************************/
    /**** END: Help Menu                                                               ****/
    /**************************************************************************************/

//#if CDEBUG
    /**************************************************************************************/
    /**** BEGIN: Debug Menu                                                            ****/
    /**************************************************************************************/
    Q3PopupMenu *debug_menu = new Q3PopupMenu( menu );
    debug_menu->insertItem("&Erase canvas",     mw->editor, SLOT(clear()), Qt::CTRL+Qt::Key_E);
    debug_menu->insertItem("List canvas items", mw, SLOT(listCanvasItems()));
    debug_menu->insertItem("Update canvas",     mw->canvas, SLOT(update()));
    debug_menu->insertItem("Test Slot A",       mw->editor, SLOT(testSlotA()));
    debug_menu->insertItem("Test Slot B",       mw->editor, SLOT(testSlotB()));
    debug_menu->insertItem("Test Slot C",       mw->editor, SLOT(testSlotC()), Qt::Key_F10);
    debug_menu->insertItem("Test Slot D",       mw->editor, SLOT(testSlotD()), Qt::Key_F11);
    debug_menu->insertItem("&Canvas Update",    mw->canvas, SLOT(update()));
    debug_menu->insertSeparator();
    debug_menu->insertItem("Test PRMSG", mw, SLOT(testPRMSG()));
    debug_menu->insertSeparator();
    debug_menu->insertItem("Path Clutter", mw->editor, SLOT(measPath()));
    debug_menu->insertItem("Print Canvas Info", mw->editor, SLOT(printCanvasInfo()));
    debug_menu->insertItem("Print Clutter Info", mw->editor, SLOT(printClutterInfo()));
    debug_menu->insertItem("Scroll Up",    mw->editor, SLOT(scrollUp()),    Qt::CTRL+Qt::SHIFT+Qt::Key_K);
    debug_menu->insertItem("Scroll Down",  mw->editor, SLOT(scrollDown()),  Qt::CTRL+Qt::SHIFT+Qt::Key_J);
    debug_menu->insertItem("Scroll Left",  mw->editor, SLOT(scrollLeft()),  Qt::CTRL+Qt::SHIFT+Qt::Key_H);
    debug_menu->insertItem("Scroll Right", mw->editor, SLOT(scrollRight()), Qt::CTRL+Qt::SHIFT+Qt::Key_L);
    debug_menu->insertItem("Vis Popout",   mw,   SLOT(visPopOut()));
    debug_menu->insertItem("Vis Popin",    mw,   SLOT(visPopIn()));
    debug_menu->insertItem("Cmd Popout",   mw,   SLOT(cmdPopOut()));
    debug_menu->insertItem("Cmd Popin",    mw,   SLOT(cmdPopIn()));
    debug_menu->insertItem("Find ...",     mw->editor, SLOT(create_find_dialog()));
    debug_menu->insertItem("Import Excel Geometry", mw, SLOT(execute_display_excel_geometry()));

    int filt_rtd_ID = debug_menu->insertItem(tr("Filter Road Test Data"), mw->editor, SLOT(create_draw_polygon_dialog(int)));
    debug_menu->setItemParameter(filt_rtd_ID, GConst::roadTestDataRTTI);

    menu->insertItem("Debug", debug_menu);
    /**************************************************************************************/
    /**** END: Debug Menu                                                              ****/
    /**************************************************************************************/
//#endif

#if CGDEBUG
//---------------------------test menu----------------------------------------
    Q3PopupMenu *test_menu = new Q3PopupMenu( menu );
    test_menu->insertItem("Create Subnet", create_subnet_dia, SLOT(show() ));
    test_menu->insertItem("Subnet Traffic", subnet_traffic_dia, SLOT(display() ));
    test_menu->insertItem("Prop Mod Widget", prop_mod_insert_tabwidget, SLOT(display()));

    test_menu = new Q3PopupMenu( menu );
    test_menu->insertItem("Chose Prop Model", sector_prop_mod_choice_dia, SLOT(display()));
    // xxx test_menu->insertItem("Prop Mod Dia", prop_mod_dialog, SLOT(display()));

    menu->insertItem("&TEST", test_menu);
#endif
//---------------------------test menu----------------------------------------
};
/******************************************************************************************/
/**** FUNCTION: MenuStructureClass::~MenuStructureClass                                ****/
/******************************************************************************************/
MenuStructureClass::~MenuStructureClass()
{
}
/******************************************************************************************/
/**** FUNCTION: MenuStructureClass::delete_all_subnets                                 ****/
/******************************************************************************************/
void MenuStructureClass::delete_all_subnets()
{
    sprintf(np->line_buf, "delete_all_subnet");
    np->process_command(np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: bool_mode_en                                                           ****/
/******************************************************************************************/
inline bool mode_en(NetworkClass *np, const bool no_geom, const bool edit_geom, const bool simulate)
{
    bool retval = false;

    if (np->error_state) {
        retval = false;
    } else {
        switch(np->mode) {
            case CConst::noGeomMode:   retval = no_geom;   break;
            case CConst::editGeomMode: retval = edit_geom; break;
            case CConst::simulateMode: retval = simulate;  break;

            default: CORE_DUMP; break;
        }
    }

    return(retval);
}

/******************************************************************************************/
/**** FUNCTION: MenuStructureClass::gui_mode_changed                                   ****/
/******************************************************************************************/
void MenuStructureClass::gui_mode_changed(MainWindowClass *mw)
{
#if DEMO
#define DEMO_OFF false
#else
#define DEMO_OFF true
#endif

    NetworkClass *np = mw->np;

    if (!np->error_state) {
        bool smb = (np->map_background ? true : false);
        bool sml = (np->map_layer_list->getSize() ? true : false) && DEMO_OFF;
        bool smr = (np->road_test_data_list->getSize() ? true : false) && DEMO_OFF;
        bool smc = (np->num_coverage_analysis ? true : false) && DEMO_OFF;
        bool smm = (np->map_clutter ? true : false) && DEMO_OFF;
//                                                                            NO_GEOM   EDIT_GEOM SIMULATE
        file_menu->setItemEnabled( read_geom_ID,                  mode_en(np, true,     false,    false   ));
        file_menu->setItemEnabled( save_geom_ID,                  mode_en(np, false,    DEMO_OFF, DEMO_OFF));

      DB_FN(file_menu->setItemEnabled( read_geom_db_ID,           mode_en(np, true,     false,    false   )));
      DB_FN(file_menu->setItemEnabled( save_geom_db_ID,           mode_en(np, false,    DEMO_OFF, DEMO_OFF)));

#ifndef __linux__
        file_menu->setItemEnabled( create_geom_ID,                mode_en(np, DEMO_OFF, false,    false   ));
        file_menu->setItemEnabled( display_excel_geometry_ID,     mode_en(np, DEMO_OFF, false,    false   ));
#endif
        file_menu->setItemEnabled( close_geom_ID,                 mode_en(np, false,    true,     true    ));
        map_menu->setItemEnabled( read_map_layer_ID,              mode_en(np, true,     true,     false   ));
        map_menu->setItemEnabled( save_map_layer_ID,              mode_en(np, false,    true,     false   ));
        map_menu->setItemEnabled( read_mif_ID,                    mode_en(np, true,     true,     false   ));
#if HAS_CLUTTER
#if CDEBUG
        map_menu->setItemEnabled( read_map_clutter_ID,            mode_en(np, DEMO_OFF, DEMO_OFF, false   ));
        map_menu->setItemEnabled( save_map_clutter_ID,            mode_en(np, DEMO_OFF, smm,      false   ));
#endif
#endif
        // map_menu->setItemEnabled( read_map_height_ID,             mode_en(np, DEMO_OFF, DEMO_OFF, false   ));
        map_menu->setItemEnabled( read_map_background_ID,         mode_en(np, false,    true,     false   ));
        map_menu->setItemEnabled( image_registration_ID ,         mode_en(np, DEMO_OFF, DEMO_OFF, DEMO_OFF));
#if 0
        file_menu->setItemEnabled( read_cch_rssi_ID,              mode_en(np, false,    false,    DEMO_OFF));
#endif

        file_menu->setItemEnabled( read_road_test_data_ID,        mode_en(np, false,    DEMO_OFF, false   ));
        file_menu->setItemEnabled( save_road_test_data_ID,        mode_en(np, false,    smr,      smr     ));
        file_menu->setItemEnabled( convert_road_test_data_ID,     mode_en(np, DEMO_OFF, DEMO_OFF, DEMO_OFF));
        file_menu->setItemEnabled( check_road_test_data_ID,       mode_en(np, false,    smr,      smr     ));
        file_menu->setItemEnabled( read_coverage_analysis_ID,     mode_en(np, false,    DEMO_OFF, DEMO_OFF));
        file_menu->setItemEnabled( save_coverage_analysis_ID,     mode_en(np, false,    smc,      smc     ));
        file_menu->setItemEnabled( file_print_ID,                 mode_en(np, false,    DEMO_OFF, DEMO_OFF));
//        file_menu->setItemEnabled( file_print_bmp_ID,             mode_en(np, false,    DEMO_OFF, DEMO_OFF));
//        file_menu->setItemEnabled( file_print_jpg_ID,             mode_en(np, false,    DEMO_OFF, DEMO_OFF));

        edit_menu->setItemEnabled( find_ID,                       mode_en(np, false,    true,     true    ));
        edit_menu->setItemEnabled( add_cell_ID,                   mode_en(np, false,    true,     false   ));
        edit_menu->setItemEnabled( add_cell_at_ID,                mode_en(np, false,    true,     false   ));
#if HAS_MONTE_CARLO
        edit_menu->setItemEnabled( add_subnet_ID,                 mode_en(np, false,    true,     false   ));
#endif
        edit_menu->setItemEnabled( edit_system_bdy_ID,            mode_en(np, false,    true,     false   ));
        edit_menu->setItemEnabled( shift_map_layer_ID,            mode_en(np, false,    sml,      false   ));
        edit_menu->setItemEnabled( shift_map_background_ID,       mode_en(np, false,    smb,      false   ));
        edit_menu->setItemEnabled( shift_road_test_data_ID,       mode_en(np, false,    smr,      false   ));

        bool flag = ( ((mw->editor->excel_file) && (mw->geometry_modified != 1)) ? true : false );
        edit_menu->setItemEnabled( extract_simulation_region_ID,  mode_en(np, false,    flag,     false   ));

        propagation_menu->setItemEnabled( prop_model_mgr_ID,      mode_en(np, false,    true,     false   ));
        propagation_menu->setItemEnabled( prop_analysis_ID,       mode_en(np, false,    smr,      false   ));

        view_menu->setItemEnabled( openhand_ID,                  mode_en(np, false,    true,     true   ));
        view_menu->setItemEnabled( zoomin_ID,                    mode_en(np, false,    true,     true   ));
        view_menu->setItemEnabled( zoomout_ID,                   mode_en(np, false,    true,     true   ));
        view_menu->setItemEnabled( zoomToFit_ID,                 mode_en(np, false,    true,     true   ));
        view_menu->setItemEnabled( command_window_ID,            mode_en(np, true,    true,     true   ));
        view_menu->setItemEnabled( visibility_window_ID,         mode_en(np, false,    true,     true   ));
        view_menu->setItemEnabled( info_window_ID,        mode_en(np, false,    true,     true   ));

#if HAS_MONTE_CARLO
        monte_carlo_menu->setItemEnabled( monte_carlo_ID,          mode_en(np, false,    false,   true    ));
        monte_carlo_menu->setItemEnabled( run_match_ID,            mode_en(np, false,    false,   true    ));
        monte_carlo_menu->setItemEnabled( set_param_ID,            mode_en(np, false,    true,    false   ));
        monte_carlo_menu->setItemEnabled( import_st_data_ID,       mode_en(np, false,    true,    false   ));
        monte_carlo_menu->setItemEnabled( delete_st_data_ID,       mode_en(np, false,    true,    false   ));
#endif

        coverage_menu->setItemEnabled( coverage_ID,             mode_en(np, false,    false,    true    ));
        subnet_menu->setItemEnabled( create_subnet_dia_ID,    mode_en(np, false,    false,    DEMO_OFF));
        subnet_menu->setItemEnabled( delete_all_subnet_ID,    mode_en(np,false,    true,  true));
#if HAS_MONTE_CARLO
        subnet_menu->setItemEnabled( set_param_ID,                 mode_en(np, false,    false,   true    ));
        subnet_menu->setItemEnabled( subnet_traffic_dia_ID,        mode_en(np, false,    true,    false   ));
#endif
        if (np->num_coverage_analysis) {
        report_menu->setItemEnabled( report_coverage_ID,          mode_en(np, false,    true,     true    ));
        report_menu->setItemEnabled( plot_coverage_ID,            mode_en(np, false,    true,     true    ));
        } else {
        report_menu->setItemEnabled( report_coverage_ID,          mode_en(np, false,    false,    false   ));
        report_menu->setItemEnabled( plot_coverage_ID,            mode_en(np, false,    false,    false   ));
        }
        if (np->num_traffic_type) {
        report_menu->setItemEnabled( report_subnets_ID,           mode_en(np, false,    true,     true    ));
        } else {
        report_menu->setItemEnabled( report_subnets_ID,           mode_en(np, false,    false,    false   ));
        }
        report_menu->setItemEnabled( report_prop_error_ID,        mode_en(np, false,    smr,      true    ));
        report_menu->setItemEnabled( report_prop_param_ID,        mode_en(np, false,    true,     true    ));
#if HAS_MONTE_CARLO
        report_menu->setItemEnabled( statistics_ID,               mode_en(np, false,    false,    true    ));
        report_menu->setItemEnabled( report_settings_ID,          mode_en(np, false,    true,     true    ));
#endif
        mode_menu->setItemEnabled( edit_mode_ID,                  mode_en(np, false,    true,     true    ));
        mode_menu->setItemEnabled( simulate_mode_ID,              mode_en(np, false,    true,     true    ));
        mode_menu->setItemChecked( edit_mode_ID,                  mode_en(np, false,    true,     false   ));
        mode_menu->setItemChecked( simulate_mode_ID,              mode_en(np, false,    false,    true    ));

        handover_menu->setItemEnabled( read_handover_ID,          mode_en(np, false,    true,     false   ));

        read_geom_TB->setEnabled(  mode_en(np, true,     false,    false   ));
        close_geom_TB->setEnabled(  mode_en(np, false,   true,     true   ));
        save_geom_TB->setEnabled(  mode_en(np, false,    DEMO_OFF, DEMO_OFF));
        file_print_TB->setEnabled( mode_en(np, false,    DEMO_OFF, DEMO_OFF));
        command_window_TB->setEnabled(  mode_en(np, true,     true,    true   ));
        visibility_window_TB->setEnabled(  mode_en(np, false,     true,    true   ));
        info_window_TB->setEnabled(  mode_en(np, false,     true,    true   ));
        openhand_TB->setEnabled(  mode_en(np, false,     true,    true   ));
        zoomin_TB->setEnabled(  mode_en(np, false,     true,    true   ));
        zoomout_TB->setEnabled(  mode_en(np, false,     true,    true   ));
        zoomToFit_TB->setEnabled(  mode_en(np, false,     true,    true   ));
    }

#undef DEMO_OFF
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::mouseMenu                                                ****/
/******************************************************************************************/
void FigureEditor::mouseMenu(int found, Q3CanvasItemList *canvas_item_list)
{
    int cell_idx, pm_idx;
    int menu_id;
    QString cell_str;
    sectorparamDia *sectorparamEdit;
    Q3PopupMenu* contextMenu;
    int contextMenu_ID1, contextMenu_ID2, contextMenu_ID3, contextMenu_ID4;
    int contextMenu_ID5, contextMenu_ID6, contextMenu_ID7, contextMenu_ID8;
    Q3CanvasItemList::Iterator it;

    bool smr = (np->road_test_data_list->getSize() ? true : false);

    contextMenu = new Q3PopupMenu( this );
    contextMenu->insertSeparator();
    contextMenu->insertSeparator();
    menu_id = contextMenu->insertItem( tr("Scroll"),        this, SLOT(setMouseMode(int)) );
    contextMenu->setItemParameter(menu_id, GConst::scrollMode);
    menu_id = contextMenu->insertItem( tr("Zoom"),          this, SLOT(setMouseMode(int)) );
    contextMenu->setItemParameter(menu_id, GConst::zoomMode);
    contextMenu->insertItem( tr("Zoom In"),       this, SLOT(zoomIn()) );
    contextMenu->insertItem( tr("Zoom Out"),      this, SLOT(zoomOut()) );
    contextMenu->insertItem( tr("Zoom to Fit"),   this, SLOT(zoomToFit()) );
    contextMenu->insertSeparator();

    // 2007-3-27 MOD
    contextMenu->insertItem( tr("Show Handover"),   this, SLOT(showHandover()) );
    contextMenu->insertItem( tr("Toggle Noise"), this, SLOT(toggleNoise()) );
    contextMenu->insertSeparator();

    if ( select_cell_list->getSize() ) {
        contextMenu_ID1 = contextMenu->insertItem( tr("Set Color") + " ...",    this, SLOT(setGroupColor()));
        contextMenu_ID2 = contextMenu->insertItem( tr("Set Shape") + " ...",    this, SLOT(create_set_group_shape_dialog()));
        contextMenu_ID3 = contextMenu->insertItem( tr("Delete"),                this, SLOT(deleteGroup()));
        contextMenu_ID4 = contextMenu->insertItem( tr("Coverage") + " ...",     this, SLOT(create_cvg_mgr_dialog()));
        contextMenu_ID5 = contextMenu->insertItem( tr("Toggle Road Test Data"), this, SLOT(toggleRoadTestDataGroup()));

        contextMenu->setItemEnabled(contextMenu_ID1, true);
        contextMenu->setItemEnabled(contextMenu_ID2, true);
        contextMenu->setItemEnabled(contextMenu_ID5, smr);

        if (np->mode == CConst::editGeomMode) {
            contextMenu->setItemEnabled(contextMenu_ID3, true);
            contextMenu->setItemEnabled(contextMenu_ID4, false);
            contextMenu->setItemEnabled(contextMenu_ID6, false);
        } else if (np->mode == CConst::simulateMode) {
            contextMenu->setItemEnabled(contextMenu_ID3, false);
            contextMenu->setItemEnabled(contextMenu_ID4, true);
            contextMenu->setItemEnabled(contextMenu_ID6, true);
        }
    } else if ( found ) {
        if (found > 1) {
            Q3PopupMenu* selectCellMenu = new Q3PopupMenu( this, "Select Cell" );
            for (it=canvas_item_list->begin(); it!=canvas_item_list->end(); ++it) {
                if ( (*it)->rtti() == GConst::cellRTTI ) {
                    selectCell = (GCellClass *) *it;
                    cell_idx = selectCell->getCellIdx();
                    cell_str = QString("Cell %1").arg(cell_idx);
                    menu_id = selectCellMenu->insertItem( cell_str, this, SLOT(setSelectCell(int)));
                    selectCellMenu->setItemParameter(menu_id, (int) selectCell);
                }
            }
            selectCell = (GCellClass *) NULL;
            selectCellMenu->exec( QCursor::pos() );
            delete selectCellMenu;
        }

        if (selectCell) {

            cell_idx = selectCell->getCellIdx();

            /**********************************************************/
            /**** Used for Power Meter                             ****/
            /**********************************************************/
            selectSector  = np->cell_list[cell_idx]->sector_list[0];
            select_posn_x = np->idx_to_x(np->cell_list[cell_idx]->posn_x);
            select_posn_y = np->idx_to_y(np->cell_list[cell_idx]->posn_y);
            /**********************************************************/

            set_message_text();

            sectorparamEdit = new sectorparamDia(np, cell_idx, this, 0);

            contextMenu_ID1 = contextMenu->insertItem( tr("Copy"),   this, SLOT(setMouseMode(int)));
            contextMenu->setItemParameter(contextMenu_ID1, GConst::copyCellMode);
            contextMenu_ID2 = contextMenu->insertItem( tr("Move"),   this, SLOT(setMouseMode(int)));
            contextMenu->setItemParameter(contextMenu_ID2, GConst::moveCellMode);
            contextMenu_ID3 = contextMenu->insertItem( tr("Delete"), this, SLOT(deleteCell()));
            contextMenu_ID4 = contextMenu->insertSeparator();
            contextMenu_ID5 = contextMenu->insertItem(
                (np->mode == CConst::editGeomMode ? tr("Edit Parameters") : tr("View Parameters")) + " ...",
                sectorparamEdit, SLOT(display()));
            contextMenu_ID6 = contextMenu->insertItem( tr("Power Meter"), this, SLOT(setMouseMode(int)));
            contextMenu->setItemParameter(contextMenu_ID6, GConst::powerMeterMode);
            contextMenu_ID7 = contextMenu->insertItem( tr("Path Loss Plot"), this, SLOT(plotPathLoss()));
            contextMenu_ID8 = contextMenu->insertItem( tr("Toggle Road Test Data"), this, SLOT(toggleRoadTestData()));
            //contextMenu->exec( QCursor::pos() );
            contextMenu->insertSeparator();
            contextMenu->insertSeparator();

            contextMenu->setItemEnabled(contextMenu_ID8, smr);
            if (np->mode == CConst::editGeomMode) {
                contextMenu->setItemEnabled(contextMenu_ID1, true);
                contextMenu->setItemEnabled(contextMenu_ID2, true);
                contextMenu->setItemEnabled(contextMenu_ID3, true);
                contextMenu->setItemEnabled(contextMenu_ID4, true);
                contextMenu->setItemEnabled(contextMenu_ID5, true);
            } else if (np->mode == CConst::simulateMode) {
                contextMenu->setItemEnabled(contextMenu_ID1, false);
                contextMenu->setItemEnabled(contextMenu_ID2, false);
                contextMenu->setItemEnabled(contextMenu_ID3, false);
                contextMenu->setItemEnabled(contextMenu_ID4, false);
                contextMenu->setItemEnabled(contextMenu_ID5, true);
            }

            if (selectSector->prop_model == -1) {
                contextMenu->setItemEnabled(contextMenu_ID6, false);
            } else {
                contextMenu->setItemEnabled(contextMenu_ID6, true);
            }

            contextMenu->setItemEnabled(contextMenu_ID7, true);
        }
    } else if ( (!found) ) {
//        menu_id = contextMenu->insertItem( tr("Track Cells"), this, SLOT(setMouseMode(int)) );
//        contextMenu->setItemParameter(menu_id, GConst::trackCellMode);
        menu_id = contextMenu->insertItem( tr("Track Subnets"), this, SLOT(setMouseMode(int)) );
        contextMenu->setItemParameter(menu_id, GConst::trackSubnetMode);
        menu_id = contextMenu->insertItem( tr("Track Map Layer Polygons"), this, SLOT(setMouseMode(int)) );
        contextMenu->setItemParameter(menu_id, GConst::trackMapLayerPolygonMode);

        menu_id = contextMenu->insertItem( tr("Track Road Test Data"), this, SLOT(setMouseMode(int)) );
        contextMenu->setItemParameter(menu_id, GConst::trackRoadTestDataMode);
        contextMenu->setItemEnabled(menu_id, smr);

        menu_id = contextMenu->insertItem( tr("Track Signal Level"), this, SLOT(setMouseMode(int)) );
        contextMenu->setItemParameter(menu_id, GConst::trackSignalLevelMode);

#if HAS_CLUTTER
        if (np->map_clutter) {
            menu_id = contextMenu->insertItem( tr("Track Clutter"), this, SLOT(setMouseMode(int)) );
            contextMenu->setItemParameter(menu_id, GConst::trackClutterMode);
        }

        found = 0;
        Q3PopupMenu *trackClutterModelMenu = new Q3PopupMenu( this );
        for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
             if (np->prop_model_list[pm_idx]->is_clutter_model()) {
                 found = 1;
                 menu_id = trackClutterModelMenu->insertItem( np->prop_model_list[pm_idx]->get_strid(), this, SLOT(setTrackClutterPropModelMode(int)));
                 trackClutterModelMenu->setItemParameter(menu_id, pm_idx);
             }
        }
        menu_id = contextMenu->insertItem( tr("Track Clutter Propagation"), trackClutterModelMenu);
        contextMenu->setItemEnabled(menu_id, (found ? true : false));

        /**********************************************************
        * Modify clutter coeffecient
        **********************************************************/
        found = 0;
        Q3PopupMenu *modCltCoef = new Q3PopupMenu( this );
        menu_id = contextMenu->insertItem( tr("Modify Clutter Coeff"), modCltCoef);
        for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
             if (np->prop_model_list[pm_idx]->is_clutter_model()) {
                 found = 1;
                 menu_id = modCltCoef->insertItem( np->prop_model_list[pm_idx]->get_strid(), this, SLOT(setModClutterCoef(int)));
                 modCltCoef->setItemParameter(menu_id, pm_idx);
             }
        }
        contextMenu->setItemEnabled(menu_id, (found ? true : false));
        /*********************************************************/
#endif
        contextMenu->insertSeparator();

        menu_id = contextMenu->insertItem( tr("Ruler"), this, SLOT(setMouseMode(int)) );
        contextMenu->setItemParameter(menu_id, GConst::rulerStartMode);
        contextMenu->insertSeparator();

        if (np->mode == CConst::editGeomMode) {
#if HAS_MONTE_CARLO
            menu_id = contextMenu->insertItem( tr("System Parameters"), this, SLOT(create_set_param_dialog()) );
            contextMenu->insertSeparator();
#endif
            Q3PopupMenu *addSubMenu = new Q3PopupMenu( this );
            menu_id = addSubMenu->insertItem( tr("Add Cell"),   this, SLOT(setMouseMode(int)));
            addSubMenu->setItemParameter(menu_id,  GConst::addCellMode);
#if HAS_MONTE_CARLO
            menu_id = addSubMenu->insertItem( tr("Add Subnet"), this, SLOT(create_draw_polygon_dialog(int)));
            addSubMenu->setItemParameter(menu_id, GConst::subnetRTTI);
#endif
            menu_id = addSubMenu->insertItem( tr("Add Road Test Data"),   this, SLOT(setMouseMode(int)));
            addSubMenu->setItemParameter(menu_id,  GConst::addRoadTestDataMode);
            contextMenu->insertItem(tr("Add"), addSubMenu);
            contextMenu->insertSeparator();
        } else if (np->mode == CConst::simulateMode) {
#if HAS_MONTE_CARLO
            contextMenu->insertItem( tr("Monte-Carlo"), this, SLOT(create_run_simulation_dialog()));
#endif
            contextMenu->insertItem( tr("Coverage"),   this, SLOT(create_cvg_mgr_dialog()));
        }
    }
    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
}

void FigureEditor::showHandover()
{
    showHandoverDia* hoDialog = new showHandoverDia(np);
    hoDialog->show();
}

void FigureEditor::run_match()
{
    sprintf(np->line_buf, "run_match");
    np->process_command(np->line_buf);
}

void FigureEditor::delete_st_data()
{
    sprintf(np->line_buf, "delete_st_data");
    np->process_command(np->line_buf);
}

/******************************************************************************************/
