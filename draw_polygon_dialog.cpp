/******************************************************************************************/
/**** PROGRAM: draw_polygon_dialog.cpp                                                 ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <stdio.h>

#include <q3canvas.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <q3filedialog.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3CString>
#include <Q3VBoxLayout>

#include "wisim.h"
#include "phs.h"
#include "wisim_gui.h"
#include "draw_polygon_dialog.h"
#include "gconst.h"
#include "main_window.h"
#include "traffic_type.h"

#include "list.h"

#ifndef __linux__
#include <process.h>
#endif	

extern char *wisim_home;
extern MainWindowClass *main_window;

/******************************************************************************************/
/**** FUNCTION: DrawPolygonDialog::DrawPolygonDialog                                   ****/
/******************************************************************************************/
DrawPolygonDialog::DrawPolygonDialog(class FigureEditor *s_editor, QWidget* parent, int rtti_val)
    : QDialog(parent, 0, true)
{
    int traffic_type_idx;

    editor = s_editor;
    NetworkClass *np = editor->get_np();
    polygon_type = rtti_val;
    QString qstr;

    switch (polygon_type) {
        case GConst::subnetRTTI:
            setName(tr("Add Subnet"));
            setCaption(tr("Add Subnet"));

            traffic_comboBox = new QComboBox( FALSE, this, "traffic_comboBox" );
            for(traffic_type_idx=0; traffic_type_idx<=np->num_traffic_type-1; traffic_type_idx++) {
                traffic_comboBox->insertItem(np->traffic_type_list[traffic_type_idx]->name(), traffic_type_idx);
            }
            qstr = tr("Draw Polygon to Add Subnet");
            break;
        case GConst::systemBoundaryRTTI:
            setName(tr("Modify System Boundary"));
            setCaption(tr("Modify System Boundary"));

            prune_checkBox = new QCheckBox(tr("Prune Cells Outside System Boundary"), this, "prune_checkBox");
            qstr = tr("Draw Polygon to Modify System Boundary");
            break;
        case GConst::roadTestDataRTTI:
            setName(tr("Filter Road Test Data"));
            setCaption(tr("Filter Road Test Data"));
            qstr = tr("Draw Polygon to Filter Road Test Data");
            break;
        case GConst::polygonRegionRTTI:
            setName(tr("Draw Polygon Region"));
            setCaption(tr("Draw Polygon Region"));
            qstr = tr("Draw Polygon");
    }

    QLabel *text_label = new QLabel(qstr, this);

    QPushButton *draw_btn = new QPushButton( tr("&Draw"), this);
    draw_btn->setMaximumWidth(100);

    QPushButton *cancel_btn = new QPushButton( tr("&Cancel"), this);
    cancel_btn->setMaximumWidth(100);

    Q3VBoxLayout *vbox = new Q3VBoxLayout(this, 11, 6);
    vbox->addWidget(text_label);
    if (polygon_type == GConst::subnetRTTI) {
        vbox->addWidget(traffic_comboBox);
    } else if (polygon_type == GConst::systemBoundaryRTTI) {
        vbox->addWidget(prune_checkBox);
    }
    Q3HBoxLayout *hbox = new Q3HBoxLayout(vbox);
    hbox->addWidget(draw_btn);
    hbox->addWidget(cancel_btn);

    connect( draw_btn,   SIGNAL( clicked() ), this, SLOT( draw_btn_clicked()   ));
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ));

    exec();
}
/******************************************************************************************/
/**** FUNCTION: PrefDia::~PrefDia                                                      ****/
/******************************************************************************************/
DrawPolygonDialog::~DrawPolygonDialog()
{
}
/******************************************************************************************/
/**** FUNCTION: DrawPolygonDialog::draw_btn_clicked                                    ****/
/******************************************************************************************/
void DrawPolygonDialog::draw_btn_clicked()
{
    editor->setMouseMode(GConst::drawPolygonMode);
    connect(editor, SIGNAL( done_drawing_polygon(Q3CanvasPolygon*) ), this, SLOT( done_drawing(Q3CanvasPolygon*) ));

    hide();
}
/******************************************************************************************/
/**** FUNCTION: cancel_btn_clicked                                                     ****/
/******************************************************************************************/
void DrawPolygonDialog::cancel_btn_clicked()
{
    delete this;
}
/******************************************************************************************/
/**** FUNCTION: done_drawing                                                           ****/
/******************************************************************************************/
void DrawPolygonDialog::done_drawing(Q3CanvasPolygon* polygon)
{
    int i, grid_x, grid_y;
    NetworkClass *np = editor->get_np();
    char *chptr;
    QMessageBox *msgBox;
/*
#ifndef __linux__
    char *exefile;
#endif
*/
    int confirmed = 0;
    if (polygon) {
        QString s;
        s = "<h3>" + tr("WISIM") + "</h3>";
        s += "<ul>";
        s +=    "<li> " + tr("Use polygon as displayed?");
        s += "</ul>";

        QMessageBox* confirmBox = new QMessageBox( tr("Confirm Polygon"),
                s, QMessageBox::Question, 1 | QMessageBox::Default, 2, 0, 0, 0, TRUE );
                confirmBox->setButtonText( 1, tr("Ok") );
                confirmBox->setButtonText( 2, tr("Cancel") );
                confirmBox->show();

        if (confirmBox->exec() == 1) {
            confirmed = 1;
        }
    }

    if (confirmed) {
        chptr = np->line_buf;
        switch(polygon_type) {
            case GConst::subnetRTTI:
                chptr += sprintf(chptr, "add_polygon -type subnet -traffic_type %s -gptlist '",
                             np->traffic_type_list[traffic_comboBox->currentItem()]->get_strid());
                break;
            case GConst::systemBoundaryRTTI:
                chptr += sprintf(chptr, "add_polygon -type system_bdy -gptlist '");
                break;
            case GConst::roadTestDataRTTI:
                chptr += sprintf(chptr, "filter_rtd -gptlist '");
                break;
            case GConst::polygonRegionRTTI:
                chptr += sprintf(chptr, "select_sectors_polygon -f '%s%csector_list.txt' "
                                    "-ext_f '%s%cextended_sector_list.txt' "
                                    "-ext_bdy_f '%s%cextended_bdy.txt' -gptlist '",
                             wisim_home, FPATH_SEPARATOR, wisim_home, FPATH_SEPARATOR, wisim_home, FPATH_SEPARATOR);
                break;
            default:
                CORE_DUMP;
        }

        char *str                = (char*) malloc ( 20*sizeof(char) );
        char *gptstr             = (char*) malloc ( 5000*sizeof(char) );
        char *SubnetOfResearch   = (char*) malloc ( 5000*sizeof(char) );
        char *SubnetOfUnResearch = (char*) malloc ( 5000*sizeof(char) );
        double min_ext_dist = 1400.;
        double max_ext_dist = 1600.;
        ListClass<IntIntClass> *ii_list;
        ii_list = new ListClass<IntIntClass>(0);

        // clean the gptstr string
        sprintf(gptstr, " ");
        for (i=0; i<= (int) polygon->points().size()-1; i++) {
            editor->canvas_to_xy(grid_x,grid_y,polygon->points().point(i).x(),polygon->points().point(i).y());
            chptr += sprintf(chptr, "%d %d ", grid_x, grid_y);

            sprintf(str, "%d %d ", grid_x, grid_y);
            strcat ( gptstr, str );
        }

        //printf("gptstr %s\n ", gptstr);

        np->extract_intint_list(ii_list, gptstr);
        ((PHSNetworkClass *)np)->select_sectors_polygon(ii_list, min_ext_dist, max_ext_dist, SubnetOfResearch, SubnetOfUnResearch);

        free (str);
        free (gptstr);
        delete ii_list;

        //printf("SubnetOfResearch   %s\n ", SubnetOfResearch);
        //printf("SubnetOfUnResearch %s\n ", SubnetOfUnResearch);

        chptr += sprintf(chptr, "'");
        delete polygon;
        np->process_command(np->line_buf);
        
        if (polygon_type == GConst::systemBoundaryRTTI) {
            if (prune_checkBox->isChecked()) {
                chptr = np->line_buf;
                chptr += sprintf(chptr, "filter_system_bdy -cell");
                np->process_command(np->line_buf);
            }
        }
#ifndef __linux__
        if (polygon_type == GConst::polygonRegionRTTI) {
            QString s;
            FILE *fp;
            int msg_exec;

            int cont = 1;
            char *geofile   = (char *) NULL;
            // char *geofile_q = (char *) NULL;
            char *rtdfile   = (char *) NULL;

            if (cont) {
                QString fn = Q3FileDialog::getSaveFileName(QString::null,
                    tr("Geometry Files") + " (*.cgeo)" + ";;" + tr("All Files") + " (*)",
                    main_window,
                    tr("Create Geometry File Dialog"),
                    tr("Enter geometry file name to save"));

                if (!fn.isEmpty()) {
                    QRegExp rx = QRegExp("\\.cgeo$");
                    if (rx.search(fn) == -1) {
                        fn += ".cgeo";
                    }

                    Q3CString qcs(2*fn.length());
                    qcs = fn.local8Bit();

                    sprintf(np->line_buf, "%s", (const char *) qcs);
                    geofile = strdup(np->line_buf);
                } else {
                    cont = 0;
                }
            }

            if ( cont && (fp = fopen(geofile, "rb")) ) {
                fclose(fp);
                s = "<h3>WISIM</h3>";
                s += "<ul>";
                s +=    "<li> " + tr("The file ") + QString(geofile) + tr(" already exists.");
                s +=    "<li> " + tr("Do you want to overwrite this file?");
                s += "</ul>";
                msgBox = new QMessageBox( tr("Geometry file already exists"),
                    s, QMessageBox::Warning,  QMessageBox::Yes,  QMessageBox::No | QMessageBox::Default, 0, 0, 0, TRUE );
                msgBox->show();
                msg_exec = msgBox->exec();
                delete msgBox;
                if ( msg_exec == QMessageBox::Yes ) {
                    remove(geofile);
                    if ( (fp = fopen(geofile, "rb")) ) {
                        fclose(fp);
                        s = "<h3>WISIM</h3>";
                        s += "<ul>";
                        s +=    "<li> " + tr("Unable to overwrite the file ") + QString(geofile);
                        s += "</ul>";
                        msgBox = new QMessageBox( tr("Unable to overwrite file"),
                            s, QMessageBox::Warning, QMessageBox::Ok | QMessageBox::Default, 0, 0, 0, 0, TRUE );
                        msgBox->show();
                        msgBox->exec();
                        delete msgBox;
                        cont = 0;
                    }
                } else {
                    cont = 0;
                }
            }
            
            if (cont) {
                QString fn = Q3FileDialog::getOpenFileName(QString::null,
                    tr("Road Test Data Files") + " (*.crtd)" + ";;" + tr("All Files") + " (*)",
                    main_window,
                    tr("Create Geometry File Dialog"),
                    tr("Select road test data file to read"));

                if (!fn.isEmpty()) {
                    Q3CString qcs(2*fn.length());
                    qcs = fn.local8Bit();

                    sprintf(np->line_buf, "%s", (const char *) qcs);
                    rtdfile = strdup(np->line_buf);
                } else {
                    cont = 0;
                }
            }

            if (cont) {
                chptr = np->line_buf;
                chptr += sprintf(chptr, "read_road_test_data -f \'%s\'", rtdfile);
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "comp_prop_model -scope global -sectors all -useheight 1 -adjust_angles 0 -ignore_fs_check 1");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "comp_prop_model -scope individual -sectors all -useheight 0 -adjust_angles 1 -ignore_fs_check 1");
                np->process_command(np->line_buf);

                //Print #1, "comp_prop_model -scope individual -sectors all -useheight 0 -adjust_angles 0" '注释表示使用平均传播模型
                //Print #1, "date -s 'end create prop_model'"
                //Print #1, "comp_prop_error -f comp_prop_model.log"

                chptr = np->line_buf;
                chptr += sprintf(chptr, "set_csid_format -fmt %d", ((PHSNetworkClass *) np)->csid_format);;
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "set_color -all_pa");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "switch_mode -mode simulate");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "group -sectors %s", SubnetOfResearch);
                np->process_command(np->line_buf);

                //chptr = np->line_buf;
                //chptr += sprintf(chptr, "group -sectors %s", SubnetOfUnResearch);
                //np->process_command(np->line_buf);

                //Print #1, "date -s 'begin create subnet'"
                chptr = np->line_buf;
                chptr += sprintf(chptr, "create_subnets -scan_fractional_area 0.999 -init_sample_res 256 -dmax 1000 -threshold_db -83");
                np->process_command(np->line_buf);
                //Print #1, "date -s 'end create subnet'"

                chptr = np->line_buf;
                chptr += sprintf(chptr, "switch_mode -mode edit_geom");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "add_polygon -type system_bdy -traffic_type COMM -subnet_strid grp_0");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "filter_system_bdy -cell");
                np->process_command(np->line_buf);

                chptr = np->line_buf;
                chptr += sprintf(chptr, "display_geometry -f \'%s\'", geofile);
                np->process_command(np->line_buf);

                if (np->error_state) {
                    s = "<h3>WISIM</h3>";
                    s += "<ul>";
                    s +=    "<li> " + tr("ERROR: Unable to Create Geometry");
                    s += "</ul>";
                    msgBox = new QMessageBox( tr("Error creating geometry file"),
                            s, QMessageBox::Warning, QMessageBox::Ok | QMessageBox::Default, 0, 0, 0, 0, TRUE );
                    msgBox->show();
                    msgBox->exec();
                    delete msgBox;
                } else {        
                    s = "<h3>WISIM</h3>";
                    s += "<ul>";
                    s +=    "<li> " + tr("Geometry File Successfully Created");
                    s += "</ul>";
                    msgBox = new QMessageBox( tr("Geometry File Successfully Created"),
                            s, QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default, 0, 0, 0, 0, TRUE );
                    msgBox->show();
                    msgBox->exec();
                    delete msgBox;
                }
            }
            if (geofile)   { free(geofile); }
            if (rtdfile)   { free(rtdfile); }
        }
#endif
        free (SubnetOfResearch);
        free (SubnetOfUnResearch);
    } else {
        delete polygon;
    }
    delete this;
}
/******************************************************************************************/
