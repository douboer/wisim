/******************************************************************************************/
/**** FILE: visibility_window.cpp                                                      ****/
/******************************************************************************************/

#include <qapplication.h>
#include <qcolordialog.h>
#include <qcursor.h>
#include <qevent.h>
#include <q3frame.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3popupmenu.h>
#include <qpushbutton.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QPixmap>
#include <QHideEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <Q3VBoxLayout>
#include <QShowEvent>
#include <QPolygon>

#include "antenna.h"
#include "cconst.h"
#include "WiSim_gui.h"
#include "WiSim.h"
#include "coverage.h"
#include "gcall.h"
#include "hot_color.h"
#include "icons.h"
#include "list.h"
#include "main_window.h"
#include "map_clutter.h"
#include "map_layer.h"
#include "phs.h"
#include "pixmap_item.h"
#include "pref.h"
#include "prop_model.h"
#include "road_test_data.h"
#include "set_strid_dia.h"
#include "traffic_type.h"
#include "visibility_window.h"
#include "set_language.h"

extern QFont *application_font;
extern QFont *fixed_width_font;

extern MainWindowClass *main_window;

/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::VisibilityWindow                                     ****/
/******************************************************************************************/
VisibilityWindow::VisibilityWindow(QWidget* parent, const char* name)
    : Q3Frame(parent, name)
{
    visibility_list = new VisibilityList( this );
    setCaption(tr("Visibility"));

    vis_system_boundary    = 0x00;
    vis_antenna            = 0x00;
    vis_cell               = 0;
    vis_cell_text          = 0x00;
    vis_map_height         = 0x00;
    vis_map_clutter        = 0;
    vis_clutter_prop_model = 0;
    vis_map_background     = 0x00;
    vis_map_layer          = 0x00;
    vis_coverage           = 0x00;
    vis_road_test_data     = 0;
    vis_subnet             = 0x00;
    vis_rtd_level          = 0x00;

#if HAS_MONTE_CARLO
    vis_traffic            = 0x00;
    vec_vis_traffic        = (char  *) NULL;
#endif

    vec_vis_cell           = (char  *) NULL;
    vec_vis_subnet         = (char **) NULL;
    vec_vis_map_clutter    = (char  *) NULL;
    vec_vis_clutter_prop_model  = (char  *) NULL;
    vec_vis_map_layer      = (char  *) NULL;
    vec_vis_coverage       = (char **) NULL;

    vec_vis_road_test_data = (char  *) NULL;

    vec_vis_rtd_level      = (char  *) NULL;
    num_road_test_data_set = 0;
    num_rtd_level          = 0;
    vec_rtd_cell_idx       = (int  *) NULL;
    vec_rtd_sector_idx     = (int  *) NULL;

    vec_vis_cell_text      = (char *) NULL;

    VisibilityList::setVisibilityWindow(this);
    VisibilityItem::setVisibilityWindow(this);
    VisibilityCheckItem::setVisibilityWindow(this);

    QIcon pop_out_iconset;
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    QIcon win_close_iconset;
    win_close_iconset.setPixmap(QPixmap(XpmIcon::win_close_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    win_close_iconset.setPixmap(QPixmap(XpmIcon::win_close_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    header = new QWidget(this);
    pop_btn    = new QPushButton(QString::null, header);
    cancel_btn = new QPushButton(QString::null, header);
    pop_btn->setIconSet( pop_out_iconset );
    cancel_btn->setIconSet( win_close_iconset );
    pop_btn->setFixedSize(QSize(16,16));
    cancel_btn->setFixedSize(QSize(16,16));

    Q3VBoxLayout *vbox = new Q3VBoxLayout( this );
    header->setPaletteBackgroundColor(QColor(105,137,188));
    vbox->addWidget(header);
    Q3HBoxLayout *hbox = new Q3HBoxLayout( header );
    vbox->addWidget(visibility_list);

    QLabel *win_lbl = new QLabel( tr("Visibility Window"), header);
    win_lbl->setPaletteForegroundColor(QColor(255,255,255));

    win_lbl->setFont(*application_font);
    hbox->addWidget(win_lbl, 0, Qt::AlignLeft);
    hbox->addStretch(100);
    hbox->setMargin(2);
    hbox->addWidget(pop_btn,         0, Qt::AlignRight);
    hbox->addSpacing(2);
    hbox->addWidget(cancel_btn,      0, Qt::AlignRight);

    connect(pop_btn,    SIGNAL(released()), SIGNAL(pop_signal()));
    connect(cancel_btn, SIGNAL(released()), SIGNAL(hide_signal()));

    setGeometry(800, 20, 400, 400);
    setFrameStyle(Q3Frame::Sunken|Q3Frame::Box);
    setFont(*fixed_width_font);

    hide();
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::~VisibilityWindow                                    ****/
/******************************************************************************************/
VisibilityWindow::~VisibilityWindow()
{
    int cvg_idx;
    VisibilityList *vlist = visibility_list;
    VisibilityItem *vi, *child;

    free(vec_vis_cell           );
    free(vec_vis_cell_text      );
    free(vec_vis_map_clutter    );
    free(vec_vis_clutter_prop_model );
    free(vec_vis_map_layer      );
    free(vec_vis_road_test_data );
    free(vec_vis_rtd_level      );
#if HAS_MONTE_CARLO
    free(vec_vis_traffic        );
#endif

    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::subnetRTTI, 0);
    if (vi) {
        child = (VisibilityItem *) vi->firstChild();
        while(child) {
            free(vec_vis_subnet[child->index]);
            child = (VisibilityItem *) child -> nextSibling();
        }
    }
    free(vec_vis_subnet);

    for (cvg_idx=0; cvg_idx<=np->num_coverage_analysis-1; cvg_idx++) {
        free(vec_vis_coverage[cvg_idx]);
    }
    free(vec_vis_coverage);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::visibility_changed                                   ****/
/******************************************************************************************/
void VisibilityWindow::visibility_changed(int rtti_val)
{
    // printf("Visibility Window: RTTI = %d\n", vis_item->rtti_val);
    emit visibility_state_change( rtti_val );
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::delete_cell                                          ****/
/******************************************************************************************/
void VisibilityWindow::delete_cell(int cell_idx)
{
    int sector_idx, rtd_idx;
    VisibilityItem *vi;
    VisibilityCheckItem *vci;
    VisibilityList *vlist = visibility_list;

    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
    vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::cellRTTI, cell_idx);
    delete vci;

    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);
    if (vi) {
        vi = (VisibilityItem *) VisibilityList::findItem(vi,    GConst::roadTestDataRTTI, 0);
        for (sector_idx=0; sector_idx<=np->cell_list[cell_idx]->num_sector-1; sector_idx++) {
            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::roadTestDataRTTI, cell_idx, sector_idx);
            if (vci) {
                rtd_idx = vci->getIndex();
                delete vci;
                if (rtd_idx != num_road_test_data_set-1) {
                    vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::roadTestDataRTTI, num_road_test_data_set-1);
                    vci->setIndex(rtd_idx);
                    vec_vis_road_test_data[rtd_idx] = vec_vis_road_test_data[num_road_test_data_set-1];
                    vec_rtd_cell_idx[rtd_idx]       = vec_rtd_cell_idx[num_road_test_data_set-1];
                    vec_rtd_sector_idx[rtd_idx]     = vec_rtd_sector_idx[num_road_test_data_set-1];
                }
                num_road_test_data_set--;
                vec_vis_road_test_data = (char *) realloc((void *) vec_vis_road_test_data, (num_road_test_data_set)*sizeof(char));
                vec_rtd_cell_idx       = (int  *) realloc((void *) vec_rtd_cell_idx,       (num_road_test_data_set)*sizeof(int ));
                vec_rtd_sector_idx     = (int  *) realloc((void *) vec_rtd_sector_idx,     (num_road_test_data_set)*sizeof(int ));
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::change_cell_idx                                      ****/
/******************************************************************************************/
void VisibilityWindow::change_cell_idx(int old_idx, int new_idx)
{
    int sector_idx, rtd_idx;
    VisibilityItem *vi;
    VisibilityCheckItem *vci;
    VisibilityList *vlist = visibility_list;

    if (new_idx != old_idx) {
        vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::cellRTTI, 0);
        vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::cellRTTI, old_idx);
        vci->setIndex(new_idx);
        vci->setText(0, np->cell_list[old_idx]->view_name(new_idx, np->preferences->vw_cell_name_pref));
        vec_vis_cell[new_idx] = vec_vis_cell[old_idx];
        // vi->sortChildItems(1, TRUE);  Does not seem to be necessary

        // Sorting extremely slow when deleting multiple cells, if necessary first delete all cells
        // then perform sort only once.
    }

    vi = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);
    if (vi) {
        vi = (VisibilityItem *) VisibilityList::findItem(vi, GConst::roadTestDataRTTI, 0);
        for (sector_idx=0; sector_idx<=np->cell_list[old_idx]->num_sector-1; sector_idx++) {
            vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::roadTestDataRTTI, old_idx, sector_idx);
            if (vci) {
                rtd_idx = vci->getIndex();
                vec_rtd_cell_idx[rtd_idx] = new_idx;
                sprintf(np->msg, "%d_%d", new_idx, sector_idx);
                vci->setText(0, np->msg);
            }
        }
        // vi->sortChildItems(1, TRUE);  Does not seem to be necessary
    }
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::resizeEvent                                            ****/
/******************************************************************************************/
void VisibilityWindow::resizeEvent(QResizeEvent *e)
{
    Q3Frame::resizeEvent(e);
    resize();
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::resize                                               ****/
/******************************************************************************************/
void VisibilityWindow::resize()
{
    int new_width;

    visibility_list->triggerUpdate();
    qApp->processEvents();

#if 0
    printf("Visibility Window Resized\n");
    printf("WIDTH = %d, HEIGHT = %d\n", width(), height());

    printf("COL 0: OLD WIDTH: %d, NEW WIDTH: %d POSN 1: %d SB %d\n",
        visibility_list->columnWidth(0),
        width() - visibility_list->columnWidth(1),
        visibility_list->header()->sectionPos(visibility_list->header()->mapToIndex( 1 )),
        (visibility_list->verticalScrollBar()->isShown() ? 1 : 0)
    );
#endif

    new_width = width() - visibility_list->columnWidth(1);
    if (visibility_list->verticalScrollBar()->isShown()) {
        new_width -= visibility_list->verticalScrollBar()->width();
    }
    visibility_list->setColumnWidth(0, new_width);
    visibility_list->triggerUpdate();
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::VisibilityList                                         ****/
/******************************************************************************************/
VisibilityList::VisibilityList(QWidget *parent, const char *name) : Q3ListView(parent, name)
{
    header()->setClickEnabled( FALSE );
    setRootIsDecorated( TRUE );
    setSorting ( 0, TRUE );
    setSelectionMode( Q3ListView::Extended);
    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    addColumn( "" /* qApp->translate("VisibilityWindow", "Visibility") */ );
    addColumn( "" /* qApp->translate("VisibilityWindow", "Color") */, 20 );
    header()->hide();
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::findItem                                               ****/
/******************************************************************************************/
Q3ListViewItem *VisibilityList::findItem(VisibilityList *parent, int rtti_val, int index)
{
    int qlve_rtti_val = -1;
    int qlve_index    = -1;
    Q3ListViewItem *found = (Q3ListViewItem *) NULL;

    Q3ListViewItem *qlve = parent->firstChild();
    while ((qlve)&&(!found)) {

        if (qlve->rtti() == GConst::visibilityItemRTTI) {
            qlve_rtti_val = ((VisibilityItem *) qlve)->rtti_val;
            qlve_index    = ((VisibilityItem *) qlve)->index;
        } else if (qlve->rtti() == GConst::visibilityCheckItemRTTI) {
            qlve_rtti_val = ((VisibilityCheckItem *) qlve)->rtti_val;
            qlve_index    = ((VisibilityCheckItem *) qlve)->index;
        } else {
            CORE_DUMP;
        }

        if ( (qlve_rtti_val == rtti_val) && (qlve_index == index) ) {
            found = qlve;
        }

        qlve = qlve -> nextSibling();
    }

    return(found);
}
Q3ListViewItem *VisibilityList::findItem(VisibilityItem *parent, int rtti_val, int index)
{
    int qlve_rtti_val = -1;
    int qlve_index    = -1;
    Q3ListViewItem *found = (Q3ListViewItem *) NULL;

    Q3ListViewItem *qlve = parent->firstChild();
    while ((qlve)&&(!found)) {

        if (qlve->rtti() == GConst::visibilityItemRTTI) {
            qlve_rtti_val = ((VisibilityItem *) qlve)->rtti_val;
            qlve_index    = ((VisibilityItem *) qlve)->index;
        } else if (qlve->rtti() == GConst::visibilityCheckItemRTTI) {
            qlve_rtti_val = ((VisibilityCheckItem *) qlve)->rtti_val;
            qlve_index    = ((VisibilityCheckItem *) qlve)->index;
        } else {
            CORE_DUMP;
        }

        if ( (qlve_rtti_val == rtti_val) && (qlve_index == index) ) {
            found = qlve;
        }

        qlve = qlve -> nextSibling();
    }

    return(found);
}
Q3ListViewItem *VisibilityList::findItem(VisibilityItem *parent, int rtti_val, int cell_idx, int sector_idx)
{
    Q3ListViewItem *qlve;
    int rtd_idx, index;
    VisibilityWindow *vw = visibility_window;

    if (rtti_val != GConst::roadTestDataRTTI) {
        CORE_DUMP;
    }

    int found = 0;
    for (rtd_idx=0; (rtd_idx<=vw->num_road_test_data_set-1)&&(!found); rtd_idx++) {
        if ( (vw->vec_rtd_cell_idx[rtd_idx] == cell_idx) && (vw->vec_rtd_sector_idx[rtd_idx] == sector_idx) ) {
            found = 1;
            index = rtd_idx;
        }
    }

    if (found) {
        qlve = findItem(parent, rtti_val, index);
    } else {
        qlve = (Q3ListViewItem *) NULL;
    }

    return(qlve);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::update_rtd                                             ****/
/******************************************************************************************/
void VisibilityList::update_rtd(int create)
{
    int i, rtd_idx, cell_idx, sector_idx, scan_idx, n, flag;
    int nd, num_digit, modified_bit_cell;
    double level;
    double pwr_offset = np->preferences->pwr_offset;
    QString s;
    SectorClass *sector;
    VisibilityCheckItem *vci;
    RoadTestPtClass *rtp = (RoadTestPtClass *) NULL;
    ListClass<int> *scan_list = new ListClass<int>(np->num_cell);

    if (np->bit_cell == -1) {
        BITWIDTH(np->bit_cell, np->num_cell-1);
        modified_bit_cell = 1;
    } else {
        modified_bit_cell = 0;
    }

    VisibilityItem *roadTestDataViewItem = (VisibilityItem *) VisibilityList::findItem(this, GConst::roadTestDataRTTI, 0);
    if (roadTestDataViewItem) {
        delete roadTestDataViewItem;
    }
    if (np->road_test_data_list->getSize()) {
        roadTestDataViewItem = new VisibilityItem(  this, GConst::roadTestDataRTTI, 0,
                   PixmapItem::view_label(GConst::roadTestDataRTTI, np, np->preferences->rtd_view_pref), -2);
    } else {
        return;
    }
    
    s = GCellClass::view_label(np, np->preferences->vw_cell_name_pref);
    VisibilityItem *roadTestDataCellViewItem = (VisibilityItem *) VisibilityList::findItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 0);
    if (!roadTestDataCellViewItem) {
        roadTestDataCellViewItem = new VisibilityItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 0, s);
        flag = 1;
    } else {
        roadTestDataCellViewItem->setText(0, s);
        flag = 0;
    }

    if (1) {
        for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
            rtp = &( (*np->road_test_data_list)[rtd_idx] );
            cell_idx = rtp->cell_idx;
            sector_idx = rtp->sector_idx;
            scan_idx = (sector_idx << np->bit_cell) | cell_idx;
            scan_list->ins_elem(scan_idx, 0);
        }
        for (i=0; i<=scan_list->getSize()-1; i++) {
            scan_idx = (*scan_list)[i];
            cell_idx   = scan_idx & ((1<<np->bit_cell)-1);
            sector_idx = scan_idx >> np->bit_cell;
            s = np->cell_list[cell_idx]->view_name(cell_idx, np->preferences->vw_cell_name_pref);
            vci = (VisibilityCheckItem *) findItem(roadTestDataCellViewItem, GConst::roadTestDataRTTI, cell_idx, sector_idx);
            if ( (!vci) ) {
                sector = np->cell_list[cell_idx]->sector_list[sector_idx];
                n = visibility_window->num_road_test_data_set;
                // sector->road_test_pt_color = np->default_color_list[n % np->num_default_color];
                sector->road_test_pt_color = np->hot_color->get_color(n, scan_list->getSize());

                if (np->preferences->rtd_view_pref == CConst::RTDbyCell) {
                    vci = new VisibilityCheckItem( roadTestDataCellViewItem, GConst::roadTestDataRTTI, n, s, sector->road_test_pt_color, cell_idx, sector_idx);
                } else {
                    vci = new VisibilityCheckItem( roadTestDataCellViewItem, GConst::roadTestDataRTTI, n, s, -1, cell_idx, sector_idx);
                }
                vci->setItemID(cell_idx, sector_idx);
            } else {
                vci->setText(0, s);
            }
        }
    }

    if (np->preferences->rtd_view_pref == CConst::RTDbyLevel) {
        VisibilityItem *roadTestDataLevelViewItem = (VisibilityItem *) VisibilityList::findItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 1);
        s = tr("Level") + " (" + QString(np->preferences->pwr_str_short) + ")";
        if (!roadTestDataLevelViewItem) {
            roadTestDataLevelViewItem = new VisibilityItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 1, s, -2);
            create = 1;
        } else {
            roadTestDataLevelViewItem->setText(0, s);
            create = 0;
        }

        num_digit = 1;
        for (i=0; i<=RoadTestPtClass::num_level-1; i++) {
            level = RoadTestPtClass::level_list[0]-pwr_offset;
            if (fabs(level) < 1.0) {
                nd = 1;
            } else {
                nd = (int) floor( log(fabs(level))/log(10.0) + 1.0);
            }
            if (level < 0.0) { nd++; }
            if (nd > num_digit) { num_digit = nd; }
        }

        for (i=0; i<=RoadTestPtClass::num_level; i++) {
            if (i == 0) {
                s = QString("Below %1").arg(RoadTestPtClass::level_list[0]-pwr_offset, num_digit+2, 'f', 1);
            } else if (i == RoadTestPtClass::num_level) {
                s = QString("Above %1").arg(RoadTestPtClass::level_list[RoadTestPtClass::num_level-1]-pwr_offset, num_digit+2, 'f', 1);
            } else {
                s = QString("%1 to %2").arg(RoadTestPtClass::level_list[i-1]-pwr_offset, num_digit+2, 'f', 1)
                                       .arg(RoadTestPtClass::level_list[i  ]-pwr_offset, num_digit+2, 'f', 1);
            }
            if (create) {
                new VisibilityCheckItem( roadTestDataLevelViewItem, GConst::roadTestDataRTTI, i, s, RoadTestPtClass::color_list[i]);
            } else {
                vci = (VisibilityCheckItem *) findItem(roadTestDataLevelViewItem, GConst::roadTestDataRTTI, i);
                vci->setText(0, s);
            }
        }
        if (create) {
            roadTestDataLevelViewItem->toggleChildren();
        }
    }
    if (flag) {
        roadTestDataCellViewItem->toggleChildren();
    }

    if (modified_bit_cell) {
        np->bit_cell = -1;
    }

    delete scan_list;

    return;
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::update_cpm                                             ****/
/******************************************************************************************/
void VisibilityList::update_cpm()
{
    int pm_idx;
    PropModelClass *pm;
    VisibilityCheckItem *vci;

    VisibilityItem *clutterPropModelViewItem = (VisibilityItem *) VisibilityList::findItem(this, GConst::clutterPropModelRTTI, 0);

    for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
        pm = np->prop_model_list[pm_idx];
        if (pm->is_clutter_model()) {
            if (!clutterPropModelViewItem) {
                clutterPropModelViewItem = new VisibilityItem(  this, GConst::clutterPropModelRTTI, 0, tr("Clutter Prop Models"), -1);
            }
            vci = (VisibilityCheckItem *) findItem(clutterPropModelViewItem, GConst::clutterPropModelRTTI, pm_idx);
            if ( (!vci) ) {
                vci = new VisibilityCheckItem( clutterPropModelViewItem, GConst::clutterPropModelRTTI, pm_idx, pm->get_strid(), -1);
            }
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: VisibilityList::update_cvg_analysis                                    ****/
/******************************************************************************************/
void VisibilityList::update_cvg_analysis(int cvg_idx)
{
    int scan_type_idx, create;
    char *label = CVECTOR(50);
    QString s;
    VisibilityCheckItem *vci;
    CoverageClass *cvg = np->coverage_list[cvg_idx];
    double pwr_offset = np->preferences->pwr_offset;

    VisibilityItem *coverageAnalysisItem = (VisibilityItem *) findItem(this, GConst::coverageTopRTTI, 0);
    if (!coverageAnalysisItem) {
        coverageAnalysisItem = new VisibilityItem(  this, GConst::coverageTopRTTI, 0, tr("Coverage"));
    }

    s = QString(cvg->strid);
    if (cvg->type == CConst::levelCoverage) {
        s += " (" + QString(np->preferences->pwr_str_short) + ")";
    }
    VisibilityItem *cvgViewItem = (VisibilityItem *)  findItem(coverageAnalysisItem, GConst::coverageRTTI, cvg_idx);
    if (!cvgViewItem) {
        create = 1;
        cvgViewItem = new VisibilityItem(  coverageAnalysisItem, GConst::coverageRTTI, cvg_idx, s);
    } else {
        cvgViewItem->setText(0, s);
        create = 0;
    }

    cvg->set_num_digit(pwr_offset);

    for (scan_type_idx=0; scan_type_idx<=cvg->scan_list->getSize()-1; scan_type_idx++) {
        cvg->scan_label(np, label, pwr_offset, scan_type_idx);
        if (create) {
            new VisibilityCheckItem( cvgViewItem, GConst::coverageRTTI, scan_type_idx, label, cvg->color_list[scan_type_idx]);
        } else {
            vci = (VisibilityCheckItem *) VisibilityList::findItem(cvgViewItem, GConst::coverageRTTI, scan_type_idx);
            vci->setText(0, label);
        }
    }
    if (create) {
        cvgViewItem->toggleChildren();
    }

    free(label);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityItem::VisibilityItem                                         ****/
/******************************************************************************************/
VisibilityItem::VisibilityItem(Q3ListView *parent, const int m_rtti_val, const int m_index, const QString & text, int m_color)
    : Q3ListViewItem(parent, text), rtti_val(m_rtti_val), index(m_index), color(m_color)
{
    allocateItemVisibility();
    setSelectable(false);
    if (color == -2) {
        setText(1, qApp->translate("VisibilityItem", "options"));
    }
}

VisibilityItem::VisibilityItem(Q3ListViewItem *parent, const int m_rtti_val, const int m_index, const QString & text, int m_color)
    : Q3ListViewItem(parent, text), rtti_val(m_rtti_val), index(m_index), color(m_color)
{
    allocateItemVisibility();
    setSelectable(false);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityItem::rtti                                                   ****/
/******************************************************************************************/
int VisibilityItem::rtti() const {return GConst::visibilityItemRTTI;}
/******************************************************************************************/
void VisibilityItem::allocateItemVisibility()
{
    VisibilityWindow *vw = visibility_window;

    switch(rtti_val) {
        case GConst::coverageRTTI:
            vw->vec_vis_coverage = (char **) realloc( (void *) vw->vec_vis_coverage, (index+1)*sizeof(char *));
            vw->vec_vis_coverage[index] = (char *)  NULL;
            break;
        case GConst::subnetRTTI:
            vw->vec_vis_subnet = (char **) realloc( (void *) vw->vec_vis_subnet, (index+1)*sizeof(char *));
            vw->vec_vis_subnet[index] = (char *)  NULL;
            break;
    }
}
/******************************************************************************************/
/**** FUNCTION: VisibilityCheckItem::VisibilityCheckItem                               ****/
/******************************************************************************************/
VisibilityCheckItem::VisibilityCheckItem(Q3ListView *parent, const int m_rtti_val, const int m_index,
                                         const QString & text, int m_color, int cell_idx, int sector_idx )
    : Q3CheckListItem(parent, text, Q3CheckListItem::CheckBox), rtti_val(m_rtti_val), index(m_index), color(m_color)
{
    setItemVisibility(1, cell_idx, sector_idx);
    setRenameEnabled(0, false);
}

VisibilityCheckItem::VisibilityCheckItem(Q3ListViewItem *parent, const int m_rtti_val, const int m_index,
                                         const QString & text, int m_color, int cell_idx, int sector_idx )
    : Q3CheckListItem(parent, text, Q3CheckListItem::CheckBox), rtti_val(m_rtti_val), index(m_index), color(m_color)
{
    setItemVisibility(1, cell_idx, sector_idx);
    setRenameEnabled(0, false);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityCheckItem::rtti                                              ****/
/******************************************************************************************/
int VisibilityCheckItem::rtti() const {return GConst::visibilityCheckItemRTTI;}
/******************************************************************************************/
void VisibilityItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    if ( (column == 0) || (color == -1) ) {
        Q3ListViewItem::paintCell( p, cg, column, width, align );
    } else if ( (column == 1) && (color == -2) ) {
        // QListViewItem::paintCell( p, cg, column, width, align );
        // p->setPen(Qt::black);
        // p->drawRect(0,2,40,height()-4);

        p->setPen(QColor(0xFFFFFF));
        p->setBrush(QColor(0xFFFFFF));
        p->drawRect(0,0,width, height());

        p->setPen(Qt::black);
        p->setBrush(Qt::lightGray);
        p->drawRect(0,2,height()-4,height()-4);

        QPolygon a(3);
        a.setPoint(0, 2, 4);
        a.setPoint(1, height()-7, 4);
        a.setPoint(2, (height()-5)/2, height()-5);
        p->setBrush(Qt::black);
        p->drawPolygon(a);
    } else {
        p->setPen(QColor(0xFFFFFF));
        p->setBrush(QColor(0xFFFFFF));
        p->drawRect(0,0,width, height());
        p->setPen(Qt::black);
        p->setBrush(QColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF));
        // p->drawRect(0,2,40,height()-4);
        p->drawRect(0,2,height()-4,height()-4);
    }
}
/******************************************************************************************/
void VisibilityCheckItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
#if 0
    QColorGroup _cg( cg );
    QColor cb, ct;

    if (column == 1) {
        cb = _cg.base();
        ct = _cg.text();
        _cg.setColor( QColorGroup::Base, QColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF) );
        _cg.setColor( QColorGroup::Text,       QColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF) );
    }

    Q3CheckListItem::paintCell( p, _cg, column, width, align );

    if (column == 1) {
        _cg.setColor( QColorGroup::Base, cb );
        _cg.setColor( QColorGroup::Text, ct );
    }
#endif

    if ( (column == 0) || (color == -1) ) {
        Q3CheckListItem::paintCell( p, cg, column, width, align );
    } else {
        p->setPen(QColor(0xFFFFFF));
        p->setBrush(QColor(0xFFFFFF));
        p->drawRect(0,0,width, height());
        p->setPen(Qt::black);
        p->setBrush(QColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF));
        // p->drawRect(0,2,40,height()-4);
        p->drawRect(0,2,height()-4,height()-4);
    }
}
/******************************************************************************************/
void VisibilityItem::setItemColor(int c_color)
{
    color = c_color;
    repaint();
}
/******************************************************************************************/
void VisibilityCheckItem::setItemColor(int c_color)
{
#if 0
    QPixmap pm(30, 10, QPixmap::MemoryOptim);
    pm.fill(QColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF));

    QPainter painter(&pm);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 30, 10);

    setPixmap(1, pm);
#else
    color = c_color;
    repaint();
#endif
}
/******************************************************************************************/
void VisibilityCheckItem::setItemVisibility(int init, int cell_idx, int sector_idx)
{
    int p_idx;

    VisibilityWindow *vw = visibility_window;

    switch(rtti_val) {
        case GConst::systemBoundaryRTTI:
            vw->vis_system_boundary = (isOn() ? 0xFF : 0x00); break;
        case GConst::antennaRTTI:
            vw->vis_antenna         = (isOn() ? 0xFF : 0x00); break;

        case GConst::cellRTTI:
            if (init) {
                vw->vec_vis_cell = (char *) realloc((void *) vw->vec_vis_cell, (index+1)*sizeof(char));
            } else {
                vw->vis_cell += (isOn() ? 1 : -1);
            }
            vw->vec_vis_cell[index] = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::cellTextRTTI:
            if (init) {
                vw->vec_vis_cell_text = (char *) realloc((void *) vw->vec_vis_cell_text, (index+1)*sizeof(char));
            } else {
                vw->vis_cell_text += (isOn() ? 1 : -1);
            }
            vw->vec_vis_cell_text[index] = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::subnetRTTI:
            p_idx = ((VisibilityItem *) parent())->index;
            if (init) {
                vw->vec_vis_subnet[p_idx] = (char *) realloc((void *) vw->vec_vis_subnet[p_idx], (index+1)*sizeof(char));
            } else {
                vw->vis_subnet += (isOn() ? 1 : -1);
            }
            vw->vec_vis_subnet[p_idx][index] = (isOn() ? 0xFF : 0x00);
            break;

#if HAS_MONTE_CARLO
        case GConst::trafficRTTI:
            if (init) {
                vw->vec_vis_traffic = (char *) realloc((void *) vw->vec_vis_traffic, (index+1)*sizeof(char));
            } else {
                vw->vis_traffic += (isOn() ? 1 : -1);
            }
            vw->vec_vis_traffic[index] = (isOn() ? 0xFF : 0x00);
            break;
#endif

        case GConst::mapHeightRTTI:
            vw->vis_map_height = (isOn() ? 0xFF : 0x00); break;

        case GConst::mapClutterRTTI:
            if (init) {
                vw->vec_vis_map_clutter = (char *) realloc((void *) vw->vec_vis_map_clutter, (index+1)*sizeof(char));
            } else {
                vw->vis_map_clutter += (isOn() ? 1 : -1);
            }
            vw->vec_vis_map_clutter[index] = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::clutterPropModelRTTI:
            if (init) {
                vw->vec_vis_clutter_prop_model = (char *) realloc((void *) vw->vec_vis_clutter_prop_model, (index+1)*sizeof(char));
            } else {
                vw->vis_clutter_prop_model += (isOn() ? 1 : -1);
            }
            vw->vec_vis_clutter_prop_model[index] = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::mapBackgroundRTTI:
            vw->vis_map_background = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::mapLayerRTTI:
            if (init) {
                vw->vec_vis_map_layer = (char *) realloc((void *) vw->vec_vis_map_layer, (index+1)*sizeof(char));
            } else {
                vw->vis_map_layer += (isOn() ? 1 : -1);
            }
            vw->vec_vis_map_layer[index] = (isOn() ? 0xFF : 0x00);
            break;
        case GConst::coverageRTTI:
            p_idx = ((VisibilityItem *) parent())->index;
            if (init) {
                vw->vec_vis_coverage[p_idx] = (char *) realloc((void *) vw->vec_vis_coverage[p_idx], (index+1)*sizeof(char));
            } else {
                vw->vis_coverage += (isOn() ? 1 : -1);
            }
            vw->vec_vis_coverage[p_idx][index] = (isOn() ? 0xFF : 0x00);
            break;

        case GConst::roadTestDataRTTI:
            p_idx = ((VisibilityItem *) parent())->index;

            if (init) {
                if (p_idx == 0) {
                    vw->vec_vis_road_test_data = (char *) realloc((void *) vw->vec_vis_road_test_data, (index+1)*sizeof(char));
                    vw->vec_rtd_cell_idx       = (int  *) realloc((void *) vw->vec_rtd_cell_idx,       (index+1)*sizeof(int ));
                    vw->vec_rtd_sector_idx     = (int  *) realloc((void *) vw->vec_rtd_sector_idx,     (index+1)*sizeof(int ));
                    vw->num_road_test_data_set = index+1;
                    setItemID(cell_idx, sector_idx);
                } else if (p_idx == 1) {
                    vw->vec_vis_rtd_level      = (char *) realloc((void *) vw->vec_vis_rtd_level,      (index+1)*sizeof(char));
                    vw->num_rtd_level          = index+1;
                } else {
                    CORE_DUMP;
                }
            } else {
                if (p_idx == 0) {
                    vw->vis_road_test_data += (isOn() ? 1 : -1);
                } else if (p_idx == 1) {
                    vw->vis_rtd_level  += (isOn() ? 1 : -1);
                } else {
                    CORE_DUMP;
                }
            }
            if (p_idx == 0) {
                cell_idx   = vw->vec_rtd_cell_idx[index];
                sector_idx = vw->vec_rtd_sector_idx[index];
                np->cell_list[cell_idx]->sector_list[sector_idx]->vis_rtd = (isOn() ? 0xFF : 0x00);
                // vw->vec_vis_road_test_data[index] = (isOn() ? 0xFF : 0x00);
            } else if (p_idx == 1) {
                vw->vec_vis_rtd_level[index] = (isOn() ? 0xFF : 0x00);
            }
            break;
    }
}
/******************************************************************************************/
void VisibilityCheckItem::setItemID(int cell_idx, int sector_idx)
{
    VisibilityWindow *vw = visibility_window;

    vw->vec_rtd_cell_idx[index]   = cell_idx;
    vw->vec_rtd_sector_idx[index] = sector_idx;
}
/******************************************************************************************/
/**** FUNCTION: VisibilityCheckItem::stateChange                                       ****/
/******************************************************************************************/
void VisibilityCheckItem::stateChange(bool)
{
#if CDEBUG && 0
    printf("Visibility Item: %s state changed RTTI = %d\n", text(0).latin1(), rtti_val);
#endif

    setItemVisibility(0);
    visibility_window->visibility_changed(rtti_val);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityCheckItem::compare                                           ****/
/******************************************************************************************/
int VisibilityCheckItem::compare(Q3ListViewItem *qlvi, int col, bool ascending ) const
{
    int use_index, retval;

    use_index = 0;
    if ( (rtti_val == GConst::coverageRTTI) && ( ((VisibilityCheckItem *)qlvi)->rtti_val == GConst::coverageRTTI) ) {
        use_index = 1;
    } else if ( (rtti_val == GConst::roadTestDataRTTI) && ( ((VisibilityCheckItem *)qlvi)->rtti_val == GConst::roadTestDataRTTI) ) {
        if (((VisibilityItem *) qlvi->parent())->index == 1) {
            use_index = 1;
        }
    }

    if ( use_index ) {
        if (index < ((VisibilityCheckItem *)qlvi)->index) {
            retval = -1;
        } else if (index > ((VisibilityCheckItem *)qlvi)->index) {
            retval = 1;
        } else {
            retval = 0;
        }
    } else if ( (rtti_val == GConst::cellRTTI) && ( np->preferences->vw_cell_name_pref == CConst::CellHexCSIDRef) ) {
        QString k1 = key(col, ascending);
        QString k2 = qlvi->key(col, ascending);
        retval = qstrcmp(k1, k2);
    } else {
        QString k1 = key(col, ascending);
        QString k2 = qlvi->key(col, ascending);
        retval = qstringcmp(k1, k2);
    }

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::setVisibility                                            ****/
/******************************************************************************************/
void FigureEditor::setVisibility(int rtti_val)
{
    int cell_idx, traffic_type_idx;
    VisibilityWindow *vw = visibility_window;

    if (   (rtti_val == GConst::systemBoundaryRTTI)
        || (rtti_val == GConst::antennaRTTI)
        || (rtti_val == GConst::subnetRTTI)
        || (rtti_val == GConst::mapHeightRTTI)
        || (rtti_val == GConst::roadTestDataRTTI)
        || (rtti_val == GConst::mapClutterRTTI)
        || (rtti_val == GConst::mapLayerRTTI)
        || (rtti_val == GConst::mapBackgroundRTTI)
        || (rtti_val == GConst::coverageRTTI)
        || (rtti_val == GConst::clutterPropModelRTTI) ) {

        /**********************************************************************************/
        /**** Draw Background                                                          ****/
        /**********************************************************************************/
        bgr_pm->fill(Qt::white);
#if CDEBUG
        bgr_pm->drawResolutionGrid(this);
#endif

        if (vw->vis_map_height) {
            bgr_pm->drawMapHeight(this);
        }
        if (vw->vis_map_background) {
            bgr_pm->drawMapBackground(this);
        }
        if (vw ->vis_map_clutter) {
            bgr_pm->drawMapClutter(this, vw->vec_vis_map_clutter);
        }

        if (vw ->vis_clutter_prop_model) {
            bgr_pm->drawClutterPropModel(this, vw->vec_vis_clutter_prop_model);
        }

        if ( vw->vis_map_layer ) {
            bgr_pm->drawMapLayer(this, vw->vec_vis_map_layer);
        }
        if ( vw->vis_system_boundary ) {
            bgr_pm->drawSystemBoundary(this);
        }

        if (vw->vis_subnet) {
            bgr_pm->drawSubnet(this, vw->vec_vis_subnet);
        }

        if ( vw->vis_coverage ) {
            bgr_pm->drawCoverage(this, vw->vec_vis_coverage);
        }
        if ( vw->vis_road_test_data ) {
            bgr_pm->drawRoadTestData(this, vw->num_road_test_data_set, vw->vec_vis_road_test_data, vw->vec_rtd_cell_idx, vw->vec_rtd_sector_idx,
                                     vw->vec_vis_rtd_level);

        }
        if ( vw->vis_antenna) {
            bgr_pm->drawDirAntenna(this);
        }
        bgr_pm->show();
        bgr_pm->execute_update();
        /**********************************************************************************/
    } else if (   (rtti_val == GConst::cellTextRTTI) ) {
        /**********************************************************************************/
        /**** Draw Cell Text                                                           ****/
        /**********************************************************************************/
        clear(GConst::cellTextRTTI);
        if ( vw->vis_cell_text ) {
            for (int cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
                new CellText(this, cell_idx, vw->vec_vis_cell_text);
            }
        }
        /**********************************************************************************/
    } else {
        switch(rtti_val) {
            case GConst::cellRTTI:
                clear(GConst::cellRTTI);
                if ( vw->vis_cell ) {
                    for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
                        if (vw->vec_vis_cell[cell_idx]) {
                            new GCellClass(this, cell_idx, np->cell_list[cell_idx]);
                        }
                    }
                }
                break;
#if HAS_MONTE_CARLO
            case GConst::trafficRTTI:
                clear(GConst::trafficRTTI);
                if ( vw->vis_traffic && np->master_call_list ) {
                    for (int master_idx=0; master_idx<=np->master_call_list->getSize()-1; master_idx++) {
                        traffic_type_idx = ((CallClass *) (*(np->master_call_list))[master_idx])->traffic_type_idx;
                        if (vw->vec_vis_traffic[traffic_type_idx]) {
                            new GCallClass(this, master_idx, traffic_type_idx);
                        }
                    }
                }
                break;
#endif
            default:
                CORE_DUMP;
                break;
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: VisibilityList::contentsMousePressEvent                                ****/
/******************************************************************************************/
void VisibilityList::contentsMousePressEvent(QMouseEvent* e)
{
#if CDEBUG
    printf("Visibility List mouse pressed\n");

    Q3ListViewItemIterator it( this, Q3ListViewItemIterator::Selected);
    while ( it.current() ) {
        printf("SELECTED: \"%s\"\n", it.current()->text(0).latin1() );
        it++;
    }
#endif

    int px, py, menu_id, rtti_val;
    Q3PopupMenu* selectParamMenu = (Q3PopupMenu *) NULL;

    int col_num = -1;

    contentsToViewport(e->pos().x(), e->pos().y(), px, py);
    Q3ListViewItem *qlve = itemAt(QPoint(px, py));
    viewportToContents ( px, py, px, py );

    if (qlve) {
        if (    ( px <= header()->sectionPos(header()->mapToIndex( 1 )) )
             && ( px >= header()->sectionPos(header()->mapToIndex(0))
                        + treeStepSize() * ( qlve->depth() + (rootIsDecorated() ? 1 : 0)
                                            + (qlve->rtti()==GConst::visibilityCheckItemRTTI ? 1 : 0) )
                        + itemMargin()) ) {
            col_num = 0;
        } else if (    (px > header()->sectionPos(header()->mapToIndex( 1 )))
                    && (px < header()->sectionPos(header()->mapToIndex( 1 )) + qlve->height()-4) ) {
            col_num = 1;
        }
    }

    if (    ((e->button() == Qt::RightButton) && (col_num == 0))
         || ((e->button() == Qt::LeftButton)  && (col_num == 1)) ) {

        VisibilityWindow *vw = ((VisibilityItem *) qlve)->visibility_window;

        if (qlve->rtti() == GConst::visibilityItemRTTI) {
            rtti_val = ((VisibilityItem *) qlve)->rtti_val;
        } else {
            rtti_val = ((VisibilityCheckItem *) qlve)->rtti_val;
        }

        if ( (qlve->rtti() == GConst::visibilityItemRTTI) && (rtti_val == GConst::cellRTTI) ) {
            selectParamMenu = new Q3PopupMenu( this, "Select Parameter Menu" );

            menu_id = selectParamMenu->insertItem( tr("CSID"), vw, SLOT(setCellNamePref(int)));
            selectParamMenu->setItemParameter(menu_id, CConst::CellHexCSIDRef);
            if (np->preferences->vw_cell_name_pref == CConst::CellHexCSIDRef) {
                selectParamMenu->setItemChecked(menu_id, true);
            }

            menu_id = selectParamMenu->insertItem( tr("GW_CSC_CS"), vw, SLOT(setCellNamePref(int)));
            selectParamMenu->setItemParameter(menu_id, CConst::CellCSNumberRef);
            if (np->preferences->vw_cell_name_pref == CConst::CellCSNumberRef) {
                selectParamMenu->setItemChecked(menu_id, true);
            }

            menu_id = selectParamMenu->insertItem( tr("Cell Index"), vw, SLOT(setCellNamePref(int)));
            selectParamMenu->setItemParameter(menu_id, CConst::CellIdxRef);
            if (np->preferences->vw_cell_name_pref == CConst::CellIdxRef) {
                selectParamMenu->setItemChecked(menu_id, true);
            }

            selectParamMenu->exec( QCursor::pos() );
            delete selectParamMenu;
        } else if (    (qlve->rtti()   == GConst::visibilityItemRTTI) && (rtti_val == GConst::roadTestDataRTTI)
                    && (qlve->parent() == (Q3ListViewItem *) NULL) ) {

            selectParamMenu = new Q3PopupMenu( this, "Select Parameter Menu" );

            menu_id = selectParamMenu->insertItem( tr("Cell"), vw, SLOT(setRTDPref(int)));
            selectParamMenu->setItemParameter(menu_id, CConst::RTDbyCell);
            if (np->preferences->rtd_view_pref == CConst::RTDbyCell) {
                selectParamMenu->setItemChecked(menu_id, true);
            }

            menu_id = selectParamMenu->insertItem( tr("Signal Level"), vw, SLOT(setRTDPref(int)));
            selectParamMenu->setItemParameter(menu_id, CConst::RTDbyLevel);
            if (np->preferences->rtd_view_pref == CConst::RTDbyLevel) {
                selectParamMenu->setItemChecked(menu_id, true);
            }

            selectParamMenu->exec( QCursor::pos() );
            delete selectParamMenu;
        } else if (    (qlve->rtti()   == GConst::visibilityItemRTTI) && (rtti_val == GConst::roadTestDataRTTI)
                    && (qlve->parent()) && (qlve->parent()->parent() == (Q3ListViewItem *) NULL)
                    && (((VisibilityItem *) qlve)->index == 1) ) {
            selectParamMenu = new Q3PopupMenu( this, "Select Parameter Menu" );
            menu_id = selectParamMenu->insertItem( tr("Specify Thresholds"), main_window->editor, SLOT(create_set_rtd_threshold_dialog()));
            selectParamMenu->exec( QCursor::pos() );
            delete selectParamMenu;

        } else if (   (    (e->button() == Qt::RightButton)
                        && (qlve->rtti() == GConst::visibilityCheckItemRTTI)
                        && (rtti_val == GConst::subnetRTTI)
                      )
                   || (    (e->button() == Qt::RightButton)
                        && (qlve->rtti() == GConst::visibilityItemRTTI)
                        && (rtti_val == GConst::coverageRTTI)
                      )
                  ) {
            if (qlve->rtti() == GConst::visibilityCheckItemRTTI) {
                vw->select_index = ((VisibilityCheckItem *) qlve)->index;
            } else {
                vw->select_index = ((VisibilityItem *) qlve)->index;
            }
            vw->select_p_idx = ((VisibilityItem *) qlve->parent())->index;
            vw->select_rtti  = rtti_val;
            selectParamMenu = new Q3PopupMenu( this, "Select Parameter Menu" );
            menu_id = selectParamMenu->insertItem( "Rename", vw, SLOT(setStrid()));
            selectParamMenu->exec( QCursor::pos() );
            delete selectParamMenu;
        }
    } else {
        Q3ListView::contentsMousePressEvent(e);
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: VisibilityList::contentsMouseReleaseEvent                              ****/
/******************************************************************************************/
void VisibilityList::contentsMouseReleaseEvent(QMouseEvent*)
{
    visibility_window->resize();
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: FigureEditor::contentsMouseDoubleClickEvent                            ****/
/******************************************************************************************/
void VisibilityList::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
#if CDEBUG
    printf("Visibility List double-clicked\n");
#endif

    int px, py;

#if 1
printf("MOUSE EVENT: %4d %4d\n", e->pos().x(), e->pos().y());
#endif

    contentsToViewport(e->pos().x(), e->pos().y(), px, py);

#if 1
printf("VIEWPORT:    %4d %4d\n", px, py);
#endif

    Q3ListViewItem *qlve = itemAt(QPoint(px, py));
    viewportToContents ( px, py, px, py );

#if 1
printf("CONTENTS:    %4d %4d\n", px, py);

printf("header()->mapToIndex(1):    %4d\n", header()->mapToIndex(1) );
printf("header()->sectionPos(header()->mapToIndex( 1 )):    %4d\n", header()->sectionPos(header()->mapToIndex( 1 )) );

printf("header()->mapToIndex(0):    %4d\n", header()->mapToIndex(0) );
printf("header()->sectionPos(header()->mapToIndex( 0 )):    %4d\n", header()->sectionPos(header()->mapToIndex( 0 )) );

printf("itemMargin():      %4d\n", itemMargin());
printf("treeStepSize():    %4d\n", treeStepSize());
printf("rootIsDecorated(): %4d\n", rootIsDecorated());

if (qlve) {
    printf("qlve->depth():     %4d\n", qlve->depth());

    printf("Start Posn:     %4d\n",
        header()->sectionPos(header()->mapToIndex(0))
        + treeStepSize() * ( qlve->depth() + (rootIsDecorated() ? 1 : 0) + (qlve->rtti() == GConst::visibilityCheckItemRTTI ? 1 : 0) )
        + itemMargin());
}
#endif

    if (qlve) {
        if (    (qlve->rtti() == GConst::visibilityItemRTTI)
             && (px < header()->sectionPos(header()->mapToIndex( 1 ))) ) {
            ((VisibilityItem *) qlve)->toggleChildren();
        } else if ( (px >= header()->sectionPos(header()->mapToIndex( 1 ))) &&
                    (    ((qlve->rtti() == GConst::visibilityCheckItemRTTI) && (((VisibilityCheckItem *) qlve)->color >= 0))
                      || ((qlve->rtti() == GConst::visibilityItemRTTI)      && (((VisibilityItem *)      qlve)->color >= 0))
                    )
                  ) {

            int rtti_val, index;
            VisibilityWindow *vw;
            if (qlve->rtti() == GConst::visibilityCheckItemRTTI) {
                rtti_val = ((VisibilityCheckItem *) qlve)->rtti_val;
                vw       = ((VisibilityCheckItem *) qlve)->visibility_window;
                index    = ((VisibilityCheckItem *) qlve)->index;
            } else {
                rtti_val = ((VisibilityItem *) qlve)->rtti_val;
                vw       = ((VisibilityItem *) qlve)->visibility_window;
                index    = ((VisibilityItem *) qlve)->index;
            }

            char *chptr = np->line_buf;
            int init_color, p_idx;
            int cell_idx, sector_idx;

            switch(rtti_val) {
                case GConst::cellRTTI:
                    chptr += sprintf(chptr,"set_color -cell_idx %d -cell ", index);
                    init_color = np->cell_list[index]->color;
                    break;
                case GConst::cellTextRTTI:
                    chptr += sprintf(chptr,"set_color -cell_text ");
                    init_color = CellClass::text_color;
                    break;
#if HAS_MONTE_CARLO
                case GConst::trafficRTTI:
                    chptr += sprintf(chptr,"set_color -traffic_type %s -traffic ", np->traffic_type_list[index]->get_strid());
                    init_color = np->traffic_type_list[index]->get_color();
                    break;
#endif
                case GConst::systemBoundaryRTTI:
                    chptr += sprintf(chptr,"set_color -system_bdy ");
                    init_color = NetworkClass::system_bdy_color;
                    break;
                case GConst::antennaRTTI:
                    chptr += sprintf(chptr,"set_color -antenna ");
                    init_color = AntennaClass::color;
                    break;
                case GConst::subnetRTTI:
                    p_idx = ((VisibilityItem *) (qlve->parent()))->index;
                    chptr += sprintf(chptr,"set_color -subnet_idx %d -traffic_type %s -subnet ", index, np->traffic_type_list[p_idx]->get_strid());
                    init_color = np->subnet_list[p_idx][index]->color;
                    break;
                case GConst::coverageRTTI:
                    p_idx = ((VisibilityItem *) (qlve->parent()))->index;
                    chptr += sprintf(chptr,"set_color -cvg_idx %d -scan_idx %d -coverage ", p_idx, index);
                    init_color = np->coverage_list[p_idx]->color_list[index];
                    break;
                case GConst::mapLayerRTTI:
                    chptr += sprintf(chptr,"set_color -map_layer_idx %d -map_layer ", index);
                    init_color = (*(np->map_layer_list))[index]->color;
                    break;
                case GConst::mapClutterRTTI:
                    chptr += sprintf(chptr,"set_color -map_clutter_idx %d -map_clutter ", index);
                    init_color = np->map_clutter->color[index];
                    break;
                case GConst::roadTestDataRTTI:
                    p_idx = ((VisibilityItem *) (qlve->parent()))->index;
                    if (p_idx == 0) {
                        cell_idx   = vw->vec_rtd_cell_idx[index];
                        sector_idx = vw->vec_rtd_sector_idx[index];
                        chptr += sprintf(chptr,"set_color -sector %d_%d -road_test_data ", cell_idx, sector_idx);
                        init_color = np->cell_list[cell_idx]->sector_list[sector_idx]->road_test_pt_color;
                    } else {
                        chptr += sprintf(chptr,"set_color -level %d -road_test_data ", index);
                        init_color = RoadTestPtClass::color_list[index];
                    }
                    break;
                default:
#if CDEBUG
                    CORE_DUMP;
#else
                    init_color = 0xFFFFFF;
#endif
                    break;
            }

            QColor qcolor = QColorDialog::getColor(init_color);
            if (qcolor.isValid()) {
                int color = (qcolor.rgb())&(0xFFFFFF);
                sprintf(chptr, "%d", color);
                np->process_command(np->line_buf);
            }
#if CDEBUG
        } else if ( px >= header()->sectionPos(header()->mapToIndex(0))
                            + treeStepSize() * ( qlve->depth() + (rootIsDecorated() ? 1 : 0)
                                                + (qlve->rtti()==GConst::visibilityCheckItemRTTI ? 1 : 0) )
                            + itemMargin()) {
            qlve->startRename(0);
#endif

        }
    }
}
/******************************************************************************************/
/**** FUNCTION: VisibilityItem::toggleChildren                                         ****/
/******************************************************************************************/
void VisibilityItem::toggleChildren()
{
    int found_off = 0;
    Q3ListViewItem *first_child = (Q3ListViewItem *) NULL;
    Q3ListViewItem *child = firstChild();

    while(child && !found_off) {
        if (child->rtti() == GConst::visibilityCheckItemRTTI) {
            if ( ((VisibilityCheckItem *) child)->isOn() == false ) {
                 found_off = 1;
            }
        }
        child = child->nextSibling();
    }
    child = firstChild();
    int flag = 0;
    while(child) {
        if (child->rtti() == GConst::visibilityCheckItemRTTI) {
            if (    ( ( found_off) && (((VisibilityCheckItem *) child)->isOn() == false) )
                 || ( (!found_off) && (((VisibilityCheckItem *) child)->isOn() == true ) ) ) {
                if (!flag) {
                    visibility_window->blockSignals(true);
                    first_child = child;
                    flag = 1;
                } else {
                    ((VisibilityCheckItem *) child)->setOn((found_off ? true : false));
                }
            }
        }
        child = child->nextSibling();
    }
    if (flag) {
        visibility_window->blockSignals(false);
        ((VisibilityCheckItem *) first_child)->setOn((found_off ? true : false));
    }
}


#if 0
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::hideEvent                                            ****/
/******************************************************************************************/
void VisibilityWindow::hideEvent(QHideEvent* e)
{
    printf("Visibility Window Hidden: type = %d\n", e->type());
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::showEvent                                            ****/
/******************************************************************************************/
void VisibilityWindow::showEvent(QShowEvent* e)
{
    printf("Visibility Window Hidden: type = %d\n", e->type());
    //emit win_vis_changed(GConst::visShow);
}
#endif
// rewrite with this function  - CG 190628
void VisibilityWindow::mouseReleaseEvent ( QMouseEvent* e)
{
    printf("Visibility Window Hidden: type = %d\n", e->type());
    if (!isHidden())
        emit win_vis_changed(GConst::visHide);
    else
        emit win_vis_changed(GConst::visShow);
}

/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::setCellNamePref                                      ****/
/******************************************************************************************/
void VisibilityWindow::setCellNamePref(int vw_cell_name_pref)
{
    int cell_idx;
    CellClass *cell;
    VisibilityItem *vi;
    VisibilityCheckItem *vci;

    np->preferences->vw_cell_name_pref = vw_cell_name_pref;

    vi = (VisibilityItem *) VisibilityList::findItem(visibility_list,  GConst::cellRTTI, 0);
    vi->setText(0, GCellClass::view_label(np, vw_cell_name_pref));
    for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
        vci = (VisibilityCheckItem *) VisibilityList::findItem(vi,    GConst::cellRTTI, cell_idx);
        cell = np->cell_list[cell_idx];
        vci->setText(0, cell->view_name(cell_idx, vw_cell_name_pref));
    }
    visibility_list->update_rtd(0);
    visibility_list->sort();
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::setRTDPref                                           ****/
/******************************************************************************************/
void VisibilityWindow::setRTDPref(int rtd_view_pref)
{
    int color, rtd_idx;
    VisibilityItem *vi, *rtd_vi;
    VisibilityCheckItem *vci;
    QString s;

    np->preferences->rtd_view_pref = rtd_view_pref;

    rtd_vi = (VisibilityItem *) VisibilityList::findItem(visibility_list,  GConst::roadTestDataRTTI, 0);
    rtd_vi->setText(0, PixmapItem::view_label(GConst::roadTestDataRTTI, np, rtd_view_pref));

    vi     = (VisibilityItem *) VisibilityList::findItem(rtd_vi,  GConst::roadTestDataRTTI, 0);
    for (rtd_idx=0; rtd_idx<=num_road_test_data_set-1; rtd_idx++) {
        vci = (VisibilityCheckItem *) visibility_list->findItem(vi, GConst::roadTestDataRTTI, rtd_idx);
        if (rtd_view_pref == CConst::RTDbyCell) {
            color = np->cell_list[vec_rtd_cell_idx[rtd_idx]]->sector_list[vec_rtd_sector_idx[rtd_idx]]->road_test_pt_color;
        } else {
            color = -1;
        }
        vci->setItemColor(color);
    }

    vi     = (VisibilityItem *) VisibilityList::findItem(rtd_vi,  GConst::roadTestDataRTTI, 1);
    if (vi && (rtd_view_pref == CConst::RTDbyCell)) {
        delete vi;
    } else if (!vi && (rtd_view_pref == CConst::RTDbyLevel)) {
        visibility_list->update_rtd(1);
    }
    visibility_list->sort();

    main_window->editor->setVisibility(GConst::roadTestDataRTTI);
}
/******************************************************************************************/
/**** FUNCTION: VisibilityWindow::setStrid                                             ****/
/******************************************************************************************/
void VisibilityWindow::setStrid()
{
    new SetStridDiaClass(select_rtti, select_index, select_p_idx, np, this);
}
/******************************************************************************************/
