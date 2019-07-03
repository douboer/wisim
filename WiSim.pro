######################################################################
# FILE: WiSim.pro
######################################################################

isEmpty( HAS_GUI ) {
    HAS_GUI = 1
}

isEmpty( HAS_ORACLE ) {
    HAS_ORACLE = 0
}

isEmpty( HAS_MONTE_CARLO ) {
    HAS_MONTE_CARLO = 1
}

isEmpty( HAS_CLUTTER ) {
    HAS_CLUTTER = 1
}

isEmpty( DEMO ) {
    DEMO = 0
}

isEmpty( TR_EMBED ) {
    TR_EMBED = 1
}

######################################################################

# use minGW/Linux to compile application
TEMPLATE = app

# use VC to compile application
#TEMPLATE = vcapp

QT += qt3support

contains(HAS_GUI, 1) {
    TARGET = WiSim
}
contains(HAS_GUI, 0) {
    TARGET = WiSim_no_gui
}

CORE_HEADERS = antenna.h                     \
               bin_io.h                      \
               binint.h                      \
               cconst.h                      \
               charstr.h                     \
               cdma2000.h                    \
               WiSim.h                     \
               clutter_data_analysis.h       \
               coverage.h                    \
               doubleintint.h                \
               global_defines.h              \
               global_fn.h                   \
               hot_color.h                   \
               intint.h                      \
               intintint.h                   \
               license.h                     \
               list.h                        \
               map_clutter.h                 \
               map_height.h                  \
               mesh.h                        \
               myusb.h                       \
               phs.h                         \
               polygon.h                     \
               posn_scan.h                   \
               pref.h                        \
               prop_model.h                  \
               randNumStr.h                  \
               randomc.h                     \
               road_test_data.h              \
               scramble.h                    \
               sparse_matrix.h               \
               spline.h                      \
               stdafx.h                      \
               strint.h                      \
               st_param.h                    \
               traffic_type.h                \
               utm_conversion.h              \
               wcdma.h                       \
               wlan.h

CORE_SOURCES = antenna.cpp                   \
               binint.cpp                    \
               bin_io.cpp                    \
               charstr.cpp                   \
               phs_cch_sync.cpp              \
               cdma2000.cpp                  \
               WiSim.cpp                   \
               check_road_test_data.cpp      \
               check_file.cpp                \
               clutter_data_analysis.cpp     \
               coverage.cpp                  \
               doubleintint.cpp              \
               gen_clutter.cpp               \
               geometry.cpp                  \
               hot_color.cpp                 \
               intdiv.cpp                    \
               intint.cpp                    \
               intintint.cpp                 \
               isortz.cpp                    \
               dsortz.cpp                    \
               global_fn.cpp                 \
               import_st.cpp                 \
               jacobiz.cpp                   \
               list.cpp                      \
               list_impl.cpp                 \
               main.cpp                      \
               map_clutter.cpp               \
               map_height.cpp                \
               map_layer.cpp                 \
               mersenne.cpp                  \
               mesh.cpp                      \
               my_svdcmpz.cpp                \
               phs.cpp                       \
               polygon.cpp                   \
               posn_scan.cpp                 \
               pref.cpp                      \
               prop_model.cpp                \
               randNumStr.cpp                \
               report_prop_model_param.cpp   \
               road_test_data.cpp            \
               sort2z.cpp                    \
               sparse_matrix.cpp             \
               spline.cpp                    \
               strint.cpp                    \
               st_param.cpp                  \
               subnet.cpp                    \
               traffic_type.cpp              \
               utm_conversion.cpp            \
               wcdma.cpp                     \
               wlan.cpp


HEADERS += $${CORE_HEADERS}
SOURCES += $${CORE_SOURCES}

contains(HAS_ORACLE, 1) {
ORACLE_HEADERS = database_fn.h               \
                 pref_dia_db.h
ORACLE_SOURCES = coverage_db.cpp             \
                 database.cpp                \
                 database_fn.cpp
HEADERS += $${ORACLE_HEADERS}
SOURCES += $${ORACLE_SOURCES}
}

contains(HAS_MONTE_CARLO, 1) {

MONTE_CARLO_HEADERS = phs_statistics.h       \
                      statistics.h           \
                      match_data.h           \
                      wlan_statistics.h

MONTE_CARLO_SOURCES = phs_assign_channel.cpp \
                      match_data.cpp         \
                      monte_carlo.cpp        \
                      phs_monte_carlo.cpp    \
                      phs_statistics.cpp     \
                      statistics.cpp         \
                      wlan_monte_carlo.cpp   \
                      wlan_statistics.cpp

HEADERS += $${MONTE_CARLO_HEADERS}

SOURCES += $${MONTE_CARLO_SOURCES}

}

contains(HAS_GUI, 1) {

GUI_HEADERS =  category_page_set.h           \
               category_wid.h                \
               WiSim_gui.h                 \
               clutter_sim_dia.h             \
               command_window.h              \
               convert_road_test_data_dialog.h \
               create_subnet_dia.h           \
               csdcaparam.h                  \
               cs_ll_err_option_dialog.h     \
               cs_lonlat_error_dialog.h      \
               cvg_analysis_page.h           \
               cvg_analysis_wizard.h         \
               cvg_info_level_wid.h          \
               cvg_info_wid.h                \
               cvg_level_wizard.h            \
               cvg_maindia.h                 \
               cvg_page_set.h                \
               cvg_part_disp.h               \
               cvg_part_wizard.h             \
               cvg_type_name_dia.h           \
               datachart.h                   \
               draw_polygon_dialog.h         \
               element.h                     \
               expo_prop_wizard.h            \
               filechooser.h                 \
               find_dia.h                    \
               gcall.h                       \
               gconst.h                      \
               generalparam.h                \
               helpdialog.h                  \
               icons.h                       \
               icons_test.h                  \
               image_registration.h          \
               import_st_data_dialog.h       \
               info_wid.h                    \
               info_window.h                 \
               logo_display.h                \
               logo.h                        \
               main_window.h                 \
               map_background.h              \
               map_layer.h                   \
               menu_structure.h              \
               mlistviewitem.h               \
               mod_clt_coef_dia.h            \
               num_sector_apply.h            \
               pixmap_item.h                 \
               pref_dia.h                    \
               printer.h                     \
               progress_slot.h               \
               prop_analysis_dia.h           \
               prop_mod_mgr_dia.h            \
               prop_mod_widget.h             \
#               prop_page_set.h               \
               prop_type_name_dia.h          \
               psdcaparam.h                  \
               qm_files.h                    \
               rd_prop_model.h               \
#               read_map_background_dialog.h \
               read_mif_dia.h                \
               reg_dia.h                     \
               reg_externalForm.h            \
               reg_form.h                    \
               reg_internalForm.h            \
               reg_lastForm.h                \
               report_dia.h                  \
               road_test_pair.h              \
               rtd_threshold_dialog.h        \
               sectorparamDia.h              \
               sectorparamPage.h             \
               sector_prop_table.h           \
#               seg_prop_wizard.h             \
               select_csid_format_dialog.h   \
               server.h                      \
               set_language.h                \
               set_shape_dialog.h            \
               set_strid_dia.h               \
               shift_dialog.h                \
               showhandover.h                \
               subnet_traffic_dia.h          \
               tooltip.h                     \
               unusedfreq.h                  \
               visibility_window.h           \
               wlan_generalparam.h

GUI_SOURCES =  addpolygon.cpp                \
               category_page_set.cpp         \
               category_wid.cpp              \
               cell_bitmap.cpp               \
               WiSim_gui.cpp               \
               clutter_sim_dia.cpp           \
               command_window.cpp            \
               convert_road_test_data_dialog.cpp \
               create_dialogs_lt.cpp         \
               create_dialogs_cg.cpp         \
               create_subnet_dia.cpp         \
               csdcaparam.cpp                \
               cs_ll_err_option_dialog.cpp   \
               cs_lonlat_error_dialog.cpp    \
               cvg_analysis_page.cpp         \
               cvg_analysis_wizard.cpp       \
               cvg_info_level_wid.cpp        \
               cvg_info_wid.cpp              \
               cvg_level_wizard.cpp          \
               cvg_maindia.cpp               \
               cvg_page_set.cpp              \
               cvg_part_disp.cpp             \
               cvg_part_wizard.cpp           \
               cvg_type_name_dia.cpp         \
               datachart.cpp                 \
               draw_polygon_dialog.cpp       \
               element.cpp                   \
               expo_prop_wizard.cpp          \
               filechooser.cpp               \
               find_dia.cpp                  \
               gcall.cpp                     \
               generalparam.cpp              \
               helpdialog.cpp                \
               icons.cpp                     \
               icons_test.cpp                \
               image_registration.cpp        \
               import_st_data_dialog.cpp     \
               info_wid.cpp                  \
               info_window.cpp               \
               logo_display.cpp              \
               main_window.cpp               \
               map_background.cpp            \
               menu_structure.cpp            \
               mlistviewitem.cpp             \
               mod_clt_coef_dia.cpp          \
               mouse.cpp                     \
               num_sector_apply.cpp          \
               pixmap_item.cpp               \
               plot_path_loss.cpp            \
               pref_dia.cpp                  \
               printer.cpp                   \
               print_gui_message.cpp         \
               progress_slot.cpp             \
               prop_analysis_dia.cpp         \
               prop_mod_mgr_dia.cpp          \
               prop_mod_widget.cpp           \
#               prop_page_set.cpp             \
               prop_type_name_dia.cpp        \
               psdcaparam.cpp                \
               rd_prop_model.cpp             \
#               read_map_background_dialog.cpp \
               read_mif_dia.cpp              \
               reg_dia.cpp                   \
               reg_externalForm.cpp          \
               reg_form.cpp                  \
               reg_internalForm.cpp          \
               reg_lastForm.cpp              \
               report_dia.cpp                \
               road_test_pair.cpp            \
               rtd_threshold_dialog.cpp      \
               sectorparamDia.cpp            \
               sectorparamPage.cpp           \
               sector_prop_table.cpp         \
#               seg_prop_wizard.cpp           \
               select_csid_format_dialog.cpp \
               server.cpp                    \
               set_language.cpp              \
               set_shape_dialog.cpp          \
               set_strid_dia.cpp             \
               shift_dialog.cpp              \
               showhandover.cpp              \
               subnet_traffic_dia.cpp        \
               tooltip.cpp                   \
               unusedfreq.cpp                \
               visibility_window.cpp         \
               wlan_generalparam.cpp

HEADERS += $${GUI_HEADERS}

SOURCES += $${GUI_SOURCES}

contains(HAS_MONTE_CARLO, 1) {

    MONTE_CARLO_HEADERS = run_simulation_dia.h

    MONTE_CARLO_SOURCES = run_simulation_dia.cpp

    HEADERS += $${MONTE_CARLO_HEADERS}

    SOURCES += $${MONTE_CARLO_SOURCES}
}

}

debug {
    DESTDIR=./debug
    LIBS += -L./debug
} else:release {
    DESTDIR=./release
    LIBS += -L./release
}
CONFIG += qt warn_on debug 

RC_FILE = WiSim.rc
# DEFINES += WISIM_RELEASE=\"081029\"
DEFINES += HAS_GUI=$${HAS_GUI}
DEFINES += HAS_ORACLE=$${HAS_ORACLE}
DEFINES += DEMO=$${DEMO}
DEFINES += TR_EMBED=$${TR_EMBED}
DEFINES += HAS_MONTE_CARLO=$${HAS_MONTE_CARLO}
DEFINES += HAS_CLUTTER=$${HAS_CLUTTER}

contains(HAS_GUI, 0) {
    # CONFIG -= qt
    win32:CONFIG += CONSOLE
}

contains(HAS_ORACLE, 1) {
    unix:INCLUDEPATH += $(ORACLE_HOME)/rdbms/demo
    unix:INCLUDEPATH += $(ORACLE_HOME)/rdbms/public
    unix:LIBS += -L$(ORACLE_HOME)/lib -L$(ORACLE_HOME)/rdbms/lib -locci -lclntsh

    win32:INCLUDEPATH += $(ORACLE_HOME)\oci\include
    win32:LIBS += $(ORACLE_HOME)\oci\lib\msvc\oci.lib
}


win32 {
    # QMAKE_CFLAGS += -EHsc
    # QMAKE_CXXFLAGS += -EHsc

    # use MingGW compile the application
    QMAKE_CFLAGS += -lwsock32
    QMAKE_CXXFLAGS += -lwsock32

    LIBS += C:\MinGW\lib\libwsock32.a
    INCLUDEPATH += "C:\MinGW\include"

    # use Visual C++ compile the application
    #LIBS += netapi32.lib
    #LIBS += Iphlpapi.lib
    #LIBS += wsock32.lib
    #QMAKE_LFLAGS += /LIBPATH:"C:\\Program Files\\Microsoft SDKs\\Windows\\v7.0A\\Lib"
    #QMAKE_LFLAGS += /LIBPATH:"C:\\Program Files\\Microsoft Visual Studio 10.0\\VC\\lib"
    #INCLUDEPATH += "C:\\Program Files\\Microsoft SDKs\\Windows\\v7.0A\\Include"
    #INCLUDEPATH += "C:\\Program Files\\Microsoft Visual Studio 10.0\\VC\\include"
}

TRANSLATIONS = WiSim_zh.ts
