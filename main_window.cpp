/******************************************************************************************/
/**** PROGRAM: main_window.cpp                                                         ****/
/**** Michael Mandell 2/19/03                                                          ****/
/******************************************************************************************/

#include <qapplication.h>
#include <q3filedialog.h>
#include <qicon.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
//Added by qt3to4:
#include <Q3CString>
#include <QPixmap>
#include <QResizeEvent>
#include <QCloseEvent>

#ifndef __linux__
#include <process.h>
#endif

#include "cconst.h"
#include "wisim_gui.h"
#include "command_window.h"
#include "coverage.h"
#include "gcall.h"
#include "icons.h"
#include "info_window.h"
#include "list.h"
#include "logo.h"
#include "main_window.h"
#include "map_layer.h"
#include "pixmap_item.h"
#include "pref.h"
#include "road_test_data.h"
#include "sectorparamDia.h"
#include "select_csid_format_dialog.h"
#include "visibility_window.h"

extern QFont *fixed_width_font;
extern char *wisim_home;

/******************************************************************************************/
/**** FUNCTION: MainWindowClass::MainWindowClass                                       ****/
/******************************************************************************************/
MainWindowClass::MainWindowClass(NetworkClass *np_param, QWidget* parent, const char* name, Qt::WFlags f) :
    QMainWindow(parent,name,f)
{
    np = np_param;

    canvas = new Q3Canvas;
    canvas->setDoubleBuffering(TRUE);
    canvas->setAdvancePeriod(30);

    s1 = new QSplitter( Qt::Vertical,   this, "main" );
    s2 = new QSplitter( Qt::Horizontal, s1,   "main" );
    editor = new FigureEditor(canvas,np,s2);
    s3 = new QSplitter( Qt::Vertical,   s2,   "main" );
    //s3->hide();

    geometry_modified = 0;
    rtd_modified = 0;
    unsaved_sim_data = 0;

    // porting to QT4
    //setUsesBigPixmaps(true);

    msc = create_menu_structure();

    command_window = new CommandWindow(np, s1);

    connect(this, SIGNAL( gui_command_emitted(char*) ), command_window, SLOT( save_gui_command(char*) ));
    connect(command_window, SIGNAL( win_vis_changed(int) ), this, SLOT( vis_command_window(int) ));
    connect(command_window, SIGNAL( pop_signal() ),         this, SLOT( cmdPopOut() ));

    visibility_window = new VisibilityWindow(s3);

    editor->setVisibilityWindow(visibility_window);

    connect(visibility_window, SIGNAL( visibility_state_change(int) ), editor, SLOT( setVisibility(int) ));
    connect(visibility_window, SIGNAL( win_vis_changed(int) ), this, SLOT( vis_visibility_window(int) ));
    connect(visibility_window, SIGNAL( pop_signal() ),         this, SLOT( visPopOut() ));
    connect(visibility_window, SIGNAL( hide_signal() ),        this, SLOT( toggle_visibility_window() ));

    info_window = new InfoWindow(np, s3);
    connect(info_window, SIGNAL( win_vis_changed(int) ), this, SLOT( vis_info_window(int) ));
    connect(info_window, SIGNAL( pop_signal() ),         this, SLOT( infoPopOut() ));
    connect(info_window, SIGNAL( hide_signal() ),        this, SLOT( toggle_info_window() ));

    connect(editor, SIGNAL( selection_changed(ListClass<int> *) ), info_window, SLOT( update_selection_change(ListClass<int> *) ));

    s2->setCollapsible(editor,            0);
    s1->setCollapsible(s2,                0);
    s3->setCollapsible(visibility_window, 0);
    s3->setCollapsible(info_window,       0);

#if 0
    setDockEnabled(Qt::DockLeft,      FALSE);
    setDockEnabled(Qt::DockRight,     FALSE);
    setDockEnabled(Qt::DockBottom,    FALSE);
    setDockEnabled(Qt::DockMinimized, FALSE);
    setDockEnabled(Qt::DockTornOff,   FALSE);
    setDockEnabled(Qt::DockUnmanaged, FALSE);
#endif

    /**************************************************************************************/
    /**** BEGIN: Status Bar                                                            ****/
    /**************************************************************************************/
    msg = new QLabel( statusBar(), "message" );
    msg->setAlignment( Qt::AlignLeft );

    state_lbl = new QLabel( statusBar() );
    state_lbl->setAlignment( Qt::AlignRight );

    msg->setFont( *fixed_width_font );
    statusBar()->addWidget( msg, 4 );

    state_lbl->setFont( *fixed_width_font );
    statusBar()->addWidget( state_lbl, 0 );

    connect(editor, SIGNAL( message_text_changed(const QString&) ), msg, SLOT( setText(const QString&) ));
    /**************************************************************************************/
    /**** END: Status Bar                                                              ****/
    /**************************************************************************************/

    statusBar();

    gui_mode_changed();

    setCentralWidget(s1);
    s1->show();

    GCellClass::set_bitmaps(np->preferences->cell_size_idx);
    GCellClass::setCellPixmapList();
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::~MainWindowClass                                      ****/
/******************************************************************************************/
MainWindowClass::~MainWindowClass()
{
    GCellClass::clear_bitmaps();
    GCellClass::deleteCellPixmapList();
#if HAS_MONTE_CARLO
    GCallClass::deletePixmap();
#endif
    delete fileTools;
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::reset_visibility_window()                             ****/
/******************************************************************************************/
void MainWindowClass::reset_visibility_window()
{
    delete visibility_window;

    visibility_window = new VisibilityWindow(s3);
    s3->moveToFirst(visibility_window);
    s3->setCollapsible(visibility_window, 0);

    editor->setVisibilityWindow(visibility_window);

    connect(visibility_window, SIGNAL( visibility_state_change(int) ), editor, SLOT( setVisibility(int) ));
    connect(visibility_window, SIGNAL( win_vis_changed(int) ),         this, SLOT( vis_visibility_window(int) ));
    connect(visibility_window, SIGNAL( pop_signal() ),                 this, SLOT( visPopOut() ));

    toggle_visibility_window(GConst::visHide);
    view_menu_set_checked(GConst::visibilityWindow, false);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::toggle_command_window                                 ****/
/******************************************************************************************/
void MainWindowClass::toggle_command_window(int a)
{
    if ((a==GConst::visHide) || ( (a==GConst::visToggle) && (view_menu_is_checked(GConst::commandWindow)) )) {
        command_window->hide();
    } else {
        command_window->showNormal();
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::toggle_info_window                                    ****/
/******************************************************************************************/
void MainWindowClass::toggle_info_window(int a)
{
    if ((a==GConst::visHide) || ( (a==GConst::visToggle) && (view_menu_is_checked(GConst::infoWindow)) )) {
        info_window->hide();
        if ( (!visibility_window->parent()) || (!view_menu_is_checked(GConst::visibilityWindow)) ) {
            s3->hide();
        }
    } else {
        if (info_window->parent()) {
            s3->show();
        }
        info_window->showNormal();
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::help                                                  ****/
/******************************************************************************************/
void MainWindowClass::help()
{

    QPixmap ico;
    ico.load("wisim_icon.png");

    static QMessageBox* about = new QMessageBox( "WiSIM",
        "<h3>WiSIM</h3>"
        "<ul>"
        "<li> " + tr("Version") + " " + VERSION +
        "<li> " + tr("Build")   + " " + WISIM_RELEASE +
        "<li> " + tr("Developed by Chengan") +
        "<li> " + tr("douboer@gmail.com") +
        "<li> " + tr("Hangzhou, China") +
        "</ul>", QMessageBox::Information, 1, 0, 0, this, 0, TRUE );
    about->setCaption(tr("About WiSIM"));
    about->setButtonText( 1, tr("Dismiss") );
    //about->setIconPixmap( QPixmap((const char **) utsi_logo_small) );
    about->setIconPixmap(ico);
    about->setPaletteBackgroundColor(QColor(0xD2,0xCD,0xC8));
    about->show();
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::testPRMSG                                             ****/
/******************************************************************************************/
void MainWindowClass::testPRMSG()
{
    sprintf(np->msg, "TEST: a <b && c> d\n");
    PRMSG(stdout, np->msg);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::listCanvasItems                                       ****/
/******************************************************************************************/
void MainWindowClass::listCanvasItems()
{
    Q3CanvasItemList list = canvas->allItems();
    Q3CanvasItemList::Iterator it;

    int i=0;
    for (it = list.begin(); it != list.end(); it++) {
        printf("[%2d] Canvas Item RTTI = %d\n", i++, (*it)->rtti());
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_gui_exit                                      ****/
/******************************************************************************************/
void MainWindowClass::execute_gui_exit()
{
    int flag;

    flag = warn_unsaved_changes(GConst::applicationQuit);

    if (flag) {
        if (!(get_np()->error_state)) {
            strcpy(np->line_buf, "q");
            np->process_command(np->line_buf);
        } else {
            qApp->quit();
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_include                                       ****/
/******************************************************************************************/
void MainWindowClass::execute_include()
{
    QString fn = Q3FileDialog::getOpenFileName(
                    QString::null,
                    tr("WiSIM Script Files") + " (*.ccmd);;" + tr("All Files") + " (*)",
                    this,
                    tr("Include Command Script Dialog"),
                    tr("Choose WiSIM command script file to execute") );

    Q3CString qcs(2*fn.length());
    qcs = fn.local8Bit();

    if (!fn.isEmpty()) {
        sprintf(np->line_buf, "include -f \'%s\'", (const char *) qcs);
        np->process_command(np->line_buf);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::warn_unsaved_changes                                  ****/
/******************************************************************************************/
int MainWindowClass::warn_unsaved_changes(int action)
{
    int cont, map_layer_idx, ml_modified, exec_val;
    MapLayerClass *ml = (MapLayerClass *) NULL;
    QMessageBox* verifyQuitBox;
    QString s;

    ml_modified = 0;
    for (map_layer_idx=0; (map_layer_idx<=np->map_layer_list->getSize()-1)&&(!ml_modified); map_layer_idx++) {
        ml = (*np->map_layer_list)[map_layer_idx];
        if (ml->modified) {
            ml_modified = 1;
        }
    }

    if ((geometry_modified) || (rtd_modified) || (ml_modified)) {
#if 0
        s = "<h3>WISIM WARNING</h3>";
        s += "<ul>";
        if (geometry_modified) {
            s +=    tr("Geometry has been modified.")       + "  \n";
        }
        if (rtd_modified) {
            s +=    tr("Road test data has been modified.") + "  \n";
        }

        if (ml_modified) {
            for (map_layer_idx=0; map_layer_idx<=np->map_layer_list->getSize()-1; map_layer_idx++) {
                ml = (*np->map_layer_list)[map_layer_idx];
                if (ml->modified) {
                    s +=    tr("Map layer") + " " + ml->name + " " + tr("has been modified.") + "  \n";
                }
            }
        }
#endif

#if 0
        s +=    tr("Unsaved Changes will be lost.")  + "  \n";

        switch(action) {
            case GConst::applicationQuit:
                s +=    tr("Are you sure you want to quit?") + "\n";
                break;
            case GConst::geometryClose:
                s +=    tr("Are you sure you want to close the geometry?") + "\n";
                break;
            default: CORE_DUMP; break;
        }
        s += "</ul>";
#endif
    }

    cont = 1;

    if ( (geometry_modified) && (cont) ) {
        s = tr("Do you want to save the changes you made to the geometry?");

        verifyQuitBox = new QMessageBox( tr("WiSim"),
            s, QMessageBox::Warning, 1 | QMessageBox::Default, 2, 3, 0, 0, TRUE );
        verifyQuitBox->setButtonText( 1, tr("Yes") );
        verifyQuitBox->setButtonText( 2, tr("No" ) );
        verifyQuitBox->setButtonText( 3, tr("Cancel" ) );
        verifyQuitBox->show();

        exec_val = verifyQuitBox->exec();
        // printf("EXEC VAL = %d\n", exec_val);

        if (exec_val == 1) {
            execute_save_file(GConst::geometryFile);
            if (geometry_modified) {
                cont = 0;
            }
        } else if (exec_val == 3) {
            cont = 0;
        }

        delete verifyQuitBox;
    }

    if ( (rtd_modified) && (cont) ) {
        s = tr("Do you want to save the changes you made to the road test data?");

        verifyQuitBox = new QMessageBox( tr("WiSim"),
            s, QMessageBox::Warning, 1 | QMessageBox::Default, 2, 3, 0, 0, TRUE );
        verifyQuitBox->setButtonText( 1, tr("Yes") );
        verifyQuitBox->setButtonText( 2, tr("No" ) );
        verifyQuitBox->setButtonText( 3, tr("Cancel" ) );
        verifyQuitBox->show();

        exec_val = verifyQuitBox->exec();
        printf("EXEC VAL = %d\n", exec_val);

        if (exec_val == 1) {
            execute_save_file(GConst::roadTestDataFile);
            if (rtd_modified) {
                cont = 0;
            }
        } else if (exec_val == 3) {
            cont = 0;
        }

        delete verifyQuitBox;
    }

    if ( (ml_modified) && (cont) ) {
        for (map_layer_idx=0; (map_layer_idx<=np->map_layer_list->getSize()-1)&&(cont); map_layer_idx++) {
            ml = (*np->map_layer_list)[map_layer_idx];
            if (ml->modified) {
                s = tr("Do you want to save the changes you made to the map layer") + " " + QString(ml->name) + "?";

                verifyQuitBox = new QMessageBox( tr("WiSim"),
                    s, QMessageBox::Warning, 1 | QMessageBox::Default, 2, 3, 0, 0, TRUE );
                verifyQuitBox->setButtonText( 1, tr("Yes") );
                verifyQuitBox->setButtonText( 2, tr("No" ) );
                verifyQuitBox->setButtonText( 3, tr("Cancel" ) );
                verifyQuitBox->show();

                exec_val = verifyQuitBox->exec();
                printf("EXEC VAL = %d\n", exec_val);

                if (exec_val == 1) {
                    execute_save_file(GConst::mapLayerFile, map_layer_idx);
                    if (ml->modified) {
                        cont = 0;
                    }
                } else if (exec_val == 3) {
                    cont = 0;
                }

                delete verifyQuitBox;
            }
        }
    }

    return(cont);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::resizeEvent                                           ****/
/******************************************************************************************/
void MainWindowClass::resizeEvent(QResizeEvent *)
{
#if CDEBUG
    printf("Calling function MainWindowClass::resizeEvent()\n");
#endif
    editor->resizeCanvas();
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::vis_command_window                                    ****/
/******************************************************************************************/
void MainWindowClass::vis_command_window(int a)
{
    if (a==GConst::visShow) {
        view_menu_set_checked( GConst::commandWindow, true);
    } else {
        view_menu_set_checked( GConst::commandWindow, false);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::toggle_visibility_window                              ****/
/******************************************************************************************/
void MainWindowClass::toggle_visibility_window(int a)
{
    if ((a==GConst::visHide) || ( (a==GConst::visToggle) && (view_menu_is_checked(GConst::visibilityWindow)) )) {
        visibility_window->hide();
        if ( (!info_window->parent()) || (!view_menu_is_checked(GConst::infoWindow)) ) {
            s3->hide();
        }
    } else {
        if (visibility_window->parent()) {
            s3->show();
        }
        visibility_window->showNormal();
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::vis_visibility_window                                 ****/
/******************************************************************************************/
void MainWindowClass::vis_visibility_window(int a)
{
    if (a==GConst::visShow) {
        view_menu_set_checked( GConst::visibilityWindow, true);
    } else {
        view_menu_set_checked( GConst::visibilityWindow, false);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::vis_info_window                                       ****/
/******************************************************************************************/
void MainWindowClass::vis_info_window(int a)
{
    if (a==GConst::visShow) {
        view_menu_set_checked( GConst::infoWindow, true);
    } else {
        view_menu_set_checked( GConst::infoWindow, false);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::set_command_window_text                               ****/
/******************************************************************************************/
void MainWindowClass::set_command_window_prompt()
{
    command_window->cle_prompt->setText(np->prompt);
    command_window->clear_button->setEnabled((np->error_state ? TRUE : FALSE));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::closeEvent                                            ****/
/******************************************************************************************/
void MainWindowClass::closeEvent( QCloseEvent* ce )
{
    ce->ignore();
    execute_gui_exit();
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_read_file                                     ****/
/******************************************************************************************/
void MainWindowClass::execute_read_file(int type)
{
    QString filt;
    QString title;
    QString caption;
    char *cmd;

    switch(type) {
        case GConst::geometryFile:
            filt    = tr("Geometry Files") + " (*.cgeo)";
            title   = tr("Read Geometry File Dialog");
            caption = tr("Read Geometry");
            cmd     = strdup("read_geometry -f \'%s\'");
            break;
        case GConst::mapLayerFile:
            filt    = tr("Map Layer Files") + " (*.cmpl)";
            title   = tr("Read Map Layer File Dialog");
            caption = tr("Read Map Layer");
            cmd     = strdup("read_map_layer -f \'%s\'");
            break;
        case GConst::mapClutterFile:
            filt    = tr("Map Clutter Files") + " (*.cmpc)";
            title   = tr("Read Map Clutter File Dialog");
            caption = tr("Read Map Clutter");
            cmd     = strdup("read_map_clutter -f \'%s\'");
            break;
        case GConst::mapHeightFile:
            filt    = tr("Map Height Files") + " (*.cmph)";
            title   = tr("Read Map Height File Dialog");
            caption = tr("Read Map Height");
            cmd     = strdup("read_map_height -f \'%s\'");
            break;
        case GConst::cchRSSITableFile:
            filt    = tr("CCH RSSI Table Files") + " (*.crss)";
            title   = tr("Read CCH RSSI Table File Dialog");
            caption = tr("Read CCH RSSI");
            cmd     = strdup("read_cch_rssi_table -f \'%s\'");
            break;
        case GConst::roadTestDataFile:
            filt    = tr("Road Test Data Files") + " (*.crtd)";
            title   = tr("Read Road Test Data File Dialog");
            caption = tr("Read Road Test Data");
            cmd     = strdup("read_road_test_data -f \'%s\'");
            break;
        case GConst::coverageAnalysisFile:
            filt    = tr("Coverage Analysis Files") + " (*.ccvg)";
            title   = tr("Read Coverage Analysis File Dialog");
            caption = tr("Read Coverage Analysis");
            cmd     = strdup("read_coverage_analysis -f \'%s\'");
            break;
        case GConst::mapBackgroundTabFile:
            filt    = tr("Background Map Position Files") + " (*.TAB)";
            title   = tr("Read Background Map File Dialog");
            caption = tr("Read Background Map");
            cmd     = strdup("read_map_background -fposn \'%s\'");
            break;
        case GConst::imageRegistrationFile:
            filt    = tr("JPEG Files") + " (*.jpg)";
            title   = tr("Image Registration Dialog");
            caption = tr("Image Registration");
            cmd     = (char *) NULL;
            break;
        case GConst::clutterPropModelFile:
            filt    = tr("Clutter Propagation Model Files") + " (*.ccpm)";
            title   = tr("Import Clutter Propagation Model File Dialog");
            caption = tr("Import Clutter Propagation Model");
            cmd     = strdup("import_clutter_model -f \'%s\'");
            break;
        case GConst::handoverFile:
            filt    = tr("Handover Files") + " (*.txt)";
            title   = tr("Read Handover Data File Dialog");
            caption = tr("Read Handover Data");
            cmd     = strdup("handover -f \'%s\'");
            break;
        default: CORE_DUMP; break;
    }
    filt += ";;" + tr("All Files") + " (*)";

    QString fn = Q3FileDialog::getOpenFileName(QString::null, filt, this, title, caption);

    if (!fn.isEmpty()) {
        if (type == GConst::imageRegistrationFile) {
            editor->create_image_registration_dialog(fn);
        } else {
            Q3CString qcs(2*fn.length());
            qcs = fn.local8Bit();

            sprintf(np->line_buf, cmd, (const char *) qcs);
            np->process_command( np->line_buf);
        }
    }

    if (cmd) {
        free(cmd);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_save_file                                     ****/
/******************************************************************************************/
void MainWindowClass::execute_save_file(int type, int idx)
{
    QString start_with;
    QString filt;
    QString title;
    QString caption;
    QString extension;
    QRegExp rx;
    char *chptr = np->line_buf;

    switch(type) {
        case GConst::geometryFile:
            start_with = editor->geometry_filename;
            filt    = tr("Geometry Files") + " (*.cgeo)";
            title   = tr("Save Geometry File Dialog");
            caption = tr("Enter geometry file name to save");
            chptr   += sprintf(chptr, "display_geometry");
            extension = "cgeo";
            break;
        case GConst::mapClutterFile:
            start_with = QString::null;
            filt    = tr("Map Clutter Files") + " (*.cmpc)";
            title   = tr("Save Map Clutter File Dialog");
            caption = tr("Enter map clutter file name to save");
            chptr   += sprintf(chptr, "save_map_clutter");
            extension = "cmpc";
            break;
        case GConst::BMPFile:
            start_with = QString::null;
            filt    = tr("Bitmap Files") + " (*.bmp)";
            title   = tr("Print Bitmap File Dialog");
            caption = tr("Enter bitmap file name to save");
            chptr   += sprintf(chptr, "print_bmp");
            extension = "bmp";
            break;
        case GConst::JPGFile:
            start_with = QString::null;
            filt    = tr("Jpeg Files") + " (*.jpg)";
            title   = tr("Print Jpeg File Dialog");
            caption = tr("Enter jpeg file name to save");
            chptr   += sprintf(chptr, "print_jpg");
            extension = "jpg";
            break;
        case GConst::roadTestDataFile:
            start_with = QString::null;
            filt    = tr("Road Test Data Files") + " (*.crtd)";
            title   = tr("Save Road Test Data File Dialog");
            caption = tr("Enter road test data file name to save");
            chptr   += sprintf(chptr, "save_road_test_data");
            extension = "crtd";
            break;
        case GConst::mapLayerFile:
            start_with = QString::null;
            filt    = tr("Map Layer Files") + " (*.cmpl)";
            title   = tr("Save Map Layer File Dialog");
            caption = tr("Enter map layer file name to save");
            chptr   += sprintf(chptr, "save_map_layer -map_layer_idx %d", idx);
            extension = "cmpl";
            break;
        default: CORE_DUMP; break;
    }
    filt += ";;" + tr("All Files") + " (*)";

    QString fn = Q3FileDialog::getSaveFileName( start_with, filt, this, title, caption);

    if (!fn.isEmpty()) {
        rx = QRegExp("\\." + extension + "$");
        if (rx.search(fn) == -1) {
            fn += "." + extension;
        }

        Q3CString qcs(2*fn.length());
        qcs = fn.local8Bit();

        chptr += sprintf(chptr, " -f \'%s\'", (const char *) qcs);
        np->process_command(np->line_buf);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_read_geometry_db                              ****/
/******************************************************************************************/
void MainWindowClass::execute_read_geometry_db()
{
    sprintf(np->line_buf, "read_geometry_db");
    np->process_command( np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_save_geometry_db                              ****/
/******************************************************************************************/
void MainWindowClass::execute_save_geometry_db()
{
    sprintf(np->line_buf, "display_geometry_db");
    np->process_command( np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_close_geometry                                ****/
/******************************************************************************************/
void MainWindowClass::execute_close_geometry()
{
    int flag;

    if (np->system_bdy) {

        flag = warn_unsaved_changes(GConst::applicationQuit);

        if (flag) {
            sprintf(np->line_buf, "close_geometry");
            np->process_command( np->line_buf);
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_display_excel_geometry                        ****/
/******************************************************************************************/
void MainWindowClass::execute_display_excel_geometry()
{
#ifndef __linux__
#if (DEMO == 0)
    int i, csid_format_val;
    char *filename;
    QString s;
    FILE *fp;

    QString fn = Q3FileDialog::getOpenFileName(QString::null,
        tr("Excel Files") + " (*.xls);;" + tr("All Files") + " (*)",
        this,
        tr("Read Excel File Dialog"),
        tr("Choose Excel file to open"));

    if (!fn.isEmpty()) {
        Q3CString qcs(2*fn.length());
        qcs = fn.local8Bit();
        sprintf(np->line_buf, "%s", (const char *) qcs );
        for (i=0; i<=strlen(np->line_buf)-1; i++) {
            if (np->line_buf[i] == '/') { 
                np->line_buf[i] = '\\';
            }
        }
        editor->excel_file = strdup(np->line_buf);
        filename = CVECTOR(strlen(wisim_home) + 1 + strlen("read_excel_geo.exe"));
        sprintf(filename, "%s%cread_excel_geo.exe", wisim_home, FPATH_SEPARATOR);

        new SelectCSIDFormatDialog(&csid_format_val, this);

        spawnl(_P_WAIT,filename, "2", editor->excel_file, NULL);
        sprintf(filename, "%s%cread_excel_geo.txt", wisim_home, FPATH_SEPARATOR);
        if ( !(fp = fopen(filename, "rb")) ) {
            s = "<h3>WiSIM</h3>";
            s += "<ul>";
            s +=    "<li> " + tr("ERROR: Unable to Create Geometry");
            s += "</ul>";
            QMessageBox *msgBox = new QMessageBox( tr("Error creating geometry file"),
                s, QMessageBox::Warning, 1 | QMessageBox::Default, 0, 0, 0, 0, TRUE );
            msgBox->setButtonText( 1, tr("OK") );
            msgBox->exec();
        } else {
            fclose(fp);
            sprintf(np->line_buf, "read_geometry -f \'%s\'", filename);
            np->process_command( np->line_buf);

            sprintf(np->line_buf, "convert_utm");
            np->process_command( np->line_buf);

            sprintf(np->line_buf, "set_csid_format -fmt %d", csid_format_val);
            np->process_command( np->line_buf);

            sprintf(np->line_buf, "set_color -all_pa");
            np->process_command( np->line_buf);
        }

        free(filename);
    }
#endif
#endif
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_add_cell_at                                   ****/
/******************************************************************************************/
void MainWindowClass::execute_add_cell_at()
{
    sectorparamDia *sectorparamEdit;

    sectorparamEdit = new sectorparamDia(np, -1, this, 0, 0);
    sectorparamEdit->exec();
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_check_road_test_data                          ****/
/******************************************************************************************/
void MainWindowClass::execute_check_road_test_data()
{
    sprintf(np->line_buf, "check_road_test_data");
    np->process_command( np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::gui_mode_changed                                      ****/
/******************************************************************************************/
void MainWindowClass::gui_mode_changed()
{

#if DEMO
#define DEMO_OFF false
#else
#define DEMO_OFF true
#endif

    QString mode_text, qstr;

    menuBar()->setEnabled( np->error_state ? false : true );
    fileTools->setEnabled( np->error_state ? false : true );
    visibility_window->setEnabled( np->error_state ? false : true );
    info_window->setEnabled( np->error_state ? false : true );
    command_window->setEnabledButtons( np->error_state ? false : true );

    gui_mode_changed_msc();

    if (np->error_state) {
        mode_text = "           " + tr("ERROR");
    } else {
        if (np->mode == CConst::noGeomMode) {
            mode_text = "     " + tr("NO GEOMETRY");
        } else if (np->mode == CConst::editGeomMode) {
#if HAS_MONTE_CARLO
            editor->setVisibility(GConst::trafficRTTI);
#endif
            mode_text = "     " + tr("EDIT MODE");
        } else if (np->mode == CConst::simulateMode) {
            mode_text = " " + tr("SIMULATE MODE");
        } else {
            CORE_DUMP;
        }
    }
    state_lbl->setText(mode_text);

    if (DEMO == 0) {
        qstr = tr("WiSim");
    } else {
        qstr = tr("WiSim DEMO");
    }

    /*
     *  CG MOD
     *
    qstr += " - ";
    qstr += np->technology_str();
    if (np->mode != CConst::noGeomMode) {
        qstr += " - ";
        if (editor->geometry_filename.isNull()) {
            qstr += "[Unnamed]";
        } else {
            qstr += editor->geometry_filename;
        }
        if (geometry_modified) {
            qstr += "*";
        }
    }
    */


    setCaption(qstr);

#undef DEMO_OFF
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::execute_set_mode                                      ****/
/******************************************************************************************/
void MainWindowClass::execute_set_mode(int mode)
{
    int flag;

    if (mode == CConst::editGeomMode) {
        sprintf(np->line_buf, "switch_mode -mode edit_geom");
        if (unsaved_sim_data) {
            QString s;
            s = "<h3>WiSIM WARNING</h3>";
            s += "<ul>";
            s +=    tr("Unsaved simulation statistics will be lost.")    + "  \n";
            s +=    tr("Are you sure you want to switch to EDIT mode?")  + "\n";
            s += "</ul>";

            static QMessageBox* verifyCloseBox = new QMessageBox( tr("WiSim"),
                s, QMessageBox::Warning, 1, 2 | QMessageBox::Default, 0, 0, 0, TRUE );
                verifyCloseBox->setButtonText( 1, tr("Yes") );
                verifyCloseBox->setButtonText( 2, tr("No" ) );
                verifyCloseBox->show();

            if (verifyCloseBox->exec() == 1) {
                flag = 1;
            } else {
                flag = 0;
            }
        } else {
            flag = 1;
        }

    } else if (mode == CConst::simulateMode) {
        sprintf(np->line_buf, "switch_mode -mode simulate" );
        flag = 1;
    } else {
        CORE_DUMP;
    }

    if (flag) {
        np->process_command( np->line_buf);
    }
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::gui_command_entered                                   ****/
/******************************************************************************************/
void MainWindowClass::gui_command_entered(char *cmd)
{
#if CDEBUG
    printf("GUI COMMAND ENTERED: \"%s\"\n", cmd);
#endif
    emit gui_command_emitted(cmd);
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::visPopOut                                             ****/
/******************************************************************************************/
void MainWindowClass::visPopOut()
{
    toggle_visibility_window(GConst::visShow);
    qApp->processEvents();
#ifdef __linux__
    QPoint pt = frameGeometry().topLeft();
#else
    QPoint pt = geometry().topLeft();
#endif
    pt += s1->geometry().topLeft();
    pt += s2->geometry().topLeft();
    pt += s3->geometry().topLeft();
    pt += visibility_window->geometry().topLeft();

    qApp->processEvents();
    visibility_window->reparent((QWidget *) 0, pt);
    toggle_visibility_window(GConst::visShow);

    if ( !(s3->children()).empty() || (!view_menu_is_checked(GConst::infoWindow)) ) {
        s3->hide();
    }

    QIcon pop_in_iconset;
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    visibility_window->cancel_btn->hide();
    visibility_window->pop_btn->setIconSet( pop_in_iconset );
    disconnect(visibility_window, SIGNAL(pop_signal()), this, 0 );
    connect(visibility_window, SIGNAL( pop_signal() ),  this, SLOT( visPopIn() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::visPopIn                                              ****/
/******************************************************************************************/
void MainWindowClass::visPopIn()
{
    toggle_visibility_window(GConst::visShow);
    qApp->processEvents();
    visibility_window->reparent(s3, QPoint(0,0));
    s3->moveToFirst(visibility_window);
    s3->show();
    toggle_visibility_window(GConst::visShow);

    QIcon pop_out_iconset;
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    visibility_window->cancel_btn->show();
    visibility_window->pop_btn->setIconSet( pop_out_iconset );
    disconnect(visibility_window, SIGNAL(pop_signal()), this, 0 );
    connect(visibility_window, SIGNAL( pop_signal() ),  this, SLOT( visPopOut() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::cmdPopOut                                             ****/
/******************************************************************************************/
void MainWindowClass::cmdPopOut()
{
    toggle_command_window(GConst::visShow);
    qApp->processEvents();
#ifdef __linux__
    QPoint pt = frameGeometry().topLeft();
#else
    QPoint pt = geometry().topLeft();
#endif
    pt += s1->geometry().topLeft();
    pt += command_window->geometry().topLeft();

    command_window->reparent((QWidget *) 0, pt);
    toggle_command_window(GConst::visShow);

    QIcon pop_in_iconset;
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    command_window->cancel_btn->hide();
    command_window->pop_btn->setIconSet( pop_in_iconset );
    disconnect(command_window, SIGNAL(pop_signal()), this, 0 );
    connect(command_window, SIGNAL( pop_signal() ),  this, SLOT( cmdPopIn() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::cmdPopIn                                              ****/
/******************************************************************************************/
void MainWindowClass::cmdPopIn()
{
    toggle_command_window(GConst::visShow);
    qApp->processEvents();
    command_window->reparent(s1, QPoint(0,0));
    toggle_command_window(GConst::visShow);

    QIcon pop_out_iconset;
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    command_window->cancel_btn->show();
    command_window->pop_btn->setIconSet( pop_out_iconset );
    disconnect(command_window, SIGNAL(pop_signal()), this, 0 );
    connect(command_window, SIGNAL( pop_signal() ),  this, SLOT( cmdPopOut() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::infoPopOut                                             ****/
/******************************************************************************************/
void MainWindowClass::infoPopOut()
{
    toggle_info_window(GConst::visShow);
    qApp->processEvents();
#ifdef __linux__
    QPoint pt = frameGeometry().topLeft();
#else
    QPoint pt = geometry().topLeft();
#endif
    pt += s1->geometry().topLeft();
    pt += s2->geometry().topLeft();
    pt += s3->geometry().topLeft();
    pt += info_window->geometry().topLeft();

    info_window->reparent((QWidget *) 0, pt);
    toggle_info_window(GConst::visShow);

    if ( !(s3->children()).empty() || (!view_menu_is_checked(GConst::visibilityWindow)) ) {
        s3->hide();
    }

    QIcon pop_in_iconset;
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_in_iconset.setPixmap(QPixmap(XpmIcon::pop_in_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    info_window->cancel_btn->hide();
    info_window->pop_btn->setIconSet( pop_in_iconset );
    disconnect(info_window, SIGNAL(pop_signal()), this, 0 );
    connect(info_window, SIGNAL( pop_signal() ),  this, SLOT( infoPopIn() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::infoPopIn                                              ****/
/******************************************************************************************/
void MainWindowClass::infoPopIn()
{
    toggle_info_window(GConst::visShow);
    qApp->processEvents();
    info_window->reparent(s3, QPoint(0,0));
    s3->show();
    toggle_info_window(GConst::visShow);

    QIcon pop_out_iconset;
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    info_window->cancel_btn->show();
    info_window->pop_btn->setIconSet( pop_out_iconset );
    disconnect(info_window, SIGNAL(pop_signal()), this, 0 );
    connect(info_window, SIGNAL( pop_signal() ),  this, SLOT( infoPopOut() ));
}
/******************************************************************************************/
/**** FUNCTION: MainWindowClass::update_pwr_unit                                       ****/
/******************************************************************************************/
void MainWindowClass::update_pwr_unit()
{
    int cvg_idx;
    NetworkClass *np = get_np();

    printf("SET RTD PWR UNIT TO %d ---------------------------\n", np->preferences->pwr_unit);

    VisibilityList *vlist = visibility_window->visibility_list;

    vlist->update_rtd(0);

    for (cvg_idx=0; cvg_idx<=np->num_coverage_analysis-1; cvg_idx++) {
        vlist->update_cvg_analysis(cvg_idx);
    }

    return;
}
/******************************************************************************************/
