/******************************************************************************************/
/**** FILE: mouse.cpp                                                                  ****/
/******************************************************************************************/
/**** Mouse related gui functions                                                      ****/
/******************************************************************************************/
#include <qcursor.h>
#include <qmenubar.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <QMouseEvent>
#include <QToolBar>


#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "clutter_data_analysis.h"
#include "doubleintint.h"
#include "icons.h"
#include "list.h"
#include "main_window.h"
#include "map_clutter.h"
#include "map_height.h"
#include "map_layer.h"
#include "phs.h"
#include "polygon.h"
#include "pref.h"
#include "prop_model.h"
#include "road_test_data.h"
#include "sectorparamDia.h"
#include "utm_conversion.h"
#include "visibility_window.h"
#include "info_window.h"
#include "showhandover.h"
#include "mod_clt_coef_dia.h"
#include "command_window.h"

extern MainWindowClass *main_window;

/******************************************************************************************/
/**** FUNCTION: FigureEditor::contentsMousePressEvent                                  ****/
/******************************************************************************************/
#define MDBG 1

#if MDBG
static QPoint first_p;
static QPoint second_p;
static Q3CanvasRectangle* chose_rect;
#endif

void FigureEditor::contentsMousePressEvent(QMouseEvent* e)
{
    QString cell_str;
    mousePosition->setPosition(np, this, e);

    if (!np->error_state) {

#if MDBG
        first_p = e->pos();
        chose_rect = new Q3CanvasRectangle(first_p.x(), first_p.y(), 1, 1, canvas());
        printf("Mouse Mode : %d\n", mouseMode);
#endif
        switch(mouseMode) {
            case GConst::zoomMode:
                if (e->button() == Qt::LeftButton) {
                    selectRegion = new Q3CanvasRectangle(canvas());
                    selectRegion->setX(mousePosition->pixel_x);
                    selectRegion->setY(mousePosition->pixel_y);
                    selectRegion->setSize(10,10);
                    selectRegion->setZ(GConst::selectRegionZ);
                    selectRegion->show();
                } else {
                    setMouseMode(GConst::selectMode);
                    if (selectRegion) {
                        delete selectRegion;
                        selectRegion = (Q3CanvasRectangle *) NULL;
                    }
                }
                break;
            case GConst::scrollMode:
                if (e->button() == Qt::LeftButton) {
                    scrollX = mousePosition->pixel_x;
                    scrollY = mousePosition->pixel_y;
                } else {
                    setMouseMode(GConst::selectMode);
                    if (selectRegion) {
                        delete selectRegion;
                        selectRegion = (Q3CanvasRectangle *) NULL;
                    }
                }
                break;
            case GConst::measPathMode:
                meas_path_x = mousePosition->grid_x;
                meas_path_y = mousePosition->grid_y;
                printf("Path Starting Point (%d, %d)\n", meas_path_x, meas_path_y);
                break;
            case GConst::rulerStartMode:
                if (ruler) {
                    delete ruler;
                    ruler = (RulerClass *) NULL;
                }
                if (e->button() == Qt::LeftButton) {
                    ruler = new RulerClass(mousePosition->grid_x, mousePosition->grid_y,
                                           mousePosition->grid_x, mousePosition->grid_y);
                    setMouseMode(GConst::rulerMeasMode);
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::rulerMeasMode:
                if (e->button() == Qt::LeftButton) {
                    rulerInfo();
                    setMouseMode(GConst::rulerStartMode);
                } else {
                    setMouseMode(GConst::selectMode);
                    if (ruler) {
                        delete ruler;
                        ruler = (RulerClass *) NULL;
                    }
                }
                break;
            case GConst::copyCellMode:
                if (e->button() == Qt::LeftButton) {
                    sprintf(np->line_buf, "copy_cell -cell %d -posn_x %9.7f -posn_y %9.7f",
                            selectCell->getCellIdx(), mousePosition->posn_x, mousePosition->posn_y);
                    np->process_command( np->line_buf);
                }
                selectCell = (GCellClass *) NULL;
                setMouseMode(GConst::selectMode);
                break;

            case GConst::drawPolygonMode:
                addPolygon(GConst::mousePress, e);
                break;
            case GConst::trackCellMode:
                if (e->button() == Qt::LeftButton) {
                    trackCell();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::trackSubnetMode:
                setMouseMode(GConst::selectMode);
                break;
            case GConst::trackMapLayerPolygonMode:
                if (e->button() == Qt::LeftButton) {
                    trackMapLayerPolygon();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::powerMeterMode:
                setMouseMode(GConst::selectMode);
                set_message_text();
                if ( power_meter_line ) {
                    delete power_meter_line;
                    power_meter_line = (Q3CanvasLine *) NULL;
                }
                break;
            case GConst::trackRoadTestDataMode:
                if (e->button() == Qt::LeftButton) {
                    trackRoadTestData();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::addRoadTestDataMode:
                if (e->button() == Qt::LeftButton) {
                    sprintf(np->line_buf, "add_rtd_pt -posn_x %9.7f -posn_y %9.7f",
                        mousePosition->posn_x, mousePosition->posn_y);
                    np->process_command( np->line_buf);
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::trackClutterMode:
                if (e->button() == Qt::LeftButton) {
                    trackClutter();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::trackClutterPropModelMode:
                if (e->button() == Qt::LeftButton) {
                    trackClutterPropModel();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::ModClutterCoeff:
                if (e->button() == Qt::LeftButton) {
                    modCltCoeff();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::trackSignalLevelMode:
                if (e->button() == Qt::LeftButton) {
                    trackSignalLevel();
                } else {
                    setMouseMode(GConst::selectMode);
                }
                break;
            case GConst::selectMode:
                Q3CanvasItemList l=canvas()->collisions(QPoint(mousePosition->pixel_x, mousePosition->pixel_y));
                Q3CanvasItemList::Iterator it;

                int found = 0;
                for (it=l.begin(); it!=l.end(); ++it) {
                    if ( (*it)->rtti() == GConst::cellRTTI ) {
                        selectCell = (GCellClass *) *it;
                        found++;
                    }
                }

                if ( (e->button() == Qt::LeftButton) ) {
                    selectRegion = new Q3CanvasRectangle(canvas());
                    selectRegion->setX(mousePosition->pixel_x);
                    selectRegion->setY(mousePosition->pixel_y);
                    selectRegion->setSize(0,0);
                    selectRegion->setZ(GConst::selectRegionZ);
                    selectRegion->show();
                } else if ( (e->button() == Qt::RightButton) )
                    // this function is defined in menu_structure.cpp
                    mouseMenu(found, &l);
                break;
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::contentsMouseMoveEvent                                   ****/
/******************************************************************************************/
void FigureEditor::contentsMouseMoveEvent(QMouseEvent* e)
{
    int s_x, s_y;
    mousePosition->setPosition(np, this, e);

#if MDBG
    second_p = e->pos();
#endif

    if (   ((mouseMode == GConst::zoomMode)   && selectRegion)
        || ((mouseMode == GConst::selectMode) && selectRegion) ) {
        s_x = mousePosition->pixel_x-((int) selectRegion->x());
        s_y = mousePosition->pixel_y-((int) selectRegion->y());

        if ((s_x == 0) || (s_x == 1)) {
            s_x = 2;
        } else if (s_x == -1) {
            s_x = -2;
        }

        if ((s_y == 0) || (s_y == 1)) {
            s_y = 2;
        } else if (s_y == -1) {
            s_y = -2;
        }
        selectRegion->setSize(s_x, s_y);
        selectRegion->show();
        canvas()->update();
    } else if ( (mouseMode == GConst::scrollMode) && (e->state() == Qt::LeftButton) ) {
        scrollBy(scrollX-mousePosition->pixel_x, scrollY-mousePosition->pixel_y);
        if (    (contentsX() <= 0)
             || (contentsY() <= 0)
             || (contentsX() + visibleWidth() - 1 >= canvas()->width()-1)
             || (contentsY() + visibleHeight() - 1 >= canvas()->height()-1)  ) {

            clear();
            int center_x = (int) floor(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0);
            int center_y = (int) floor(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0);
            printf("Emitting Regenerate Canvas signal: center_x = %d center_y = %d\n", center_x, center_y);
            int prev_pmXL = c_pmXL;
            int prev_pmYL = c_pmYL;
            regenCanvas(center_x,center_y);
            scrollX += prev_pmXL - c_pmXL;
            scrollY += prev_pmYL - c_pmYL;
        }
    } else if ( mouseMode == GConst::drawPolygonMode ) {
        addPolygon(GConst::mouseMove, e);
    } else if ( mouseMode == GConst::moveCellMode ) {
        // printf("Moving to position %d %d\n", p.x(), p.y());
        int offset_x = 0;
        int offset_y = 0;
        if (selectCell->rtti() == GConst::cellRTTI) {
            mousePosition->pixel_x = (int) floor(floor(mousePosition->pixel_x/zoom + 0.5) * zoom);
            mousePosition->pixel_y = (int) floor(floor(mousePosition->pixel_y/zoom + 0.5) * zoom);
            offset_x = selectCell->width()/2;
            offset_y = selectCell->height()/2;
        }
        selectCell->move(mousePosition->pixel_x - offset_x, mousePosition->pixel_y - offset_y);
        canvas()->update();
    } else if ( mouseMode == GConst::powerMeterMode) {
        int startPx = 0;
        int startPy = 0;
        int endPx = 0;
        int endPy = 0;
        xy_to_canvas(startPx, startPy, selectSector->parent_cell->posn_x, selectSector->parent_cell->posn_y);

        endPx = mousePosition->pixel_x;
        endPy = mousePosition->pixel_y;

        if ( power_meter_line==NULL ) {
            power_meter_line = new Q3CanvasLine(canvas());
        }
        power_meter_line->setPoints( startPx, startPy, endPx, endPy );
        power_meter_line->setZ(GConst::selectRegionZ);
        power_meter_line->show();
    } else if ( mouseMode == GConst::rulerMeasMode) {
// xxxxxxxxx
//        if ( ruler==NULL ) {
//            ruler = new RulerClass(mousePosition->grid_x, mousePosition->grid_y,
//                                   mousePosition->grid_x, mousePosition->grid_y);
//        }

        ruler->clear();
        ruler->setPt( 1, mousePosition->grid_x, mousePosition->grid_y );
        ruler->draw(this, canvas());
    }

#if MDBG
    else if ( (mouseMode == GConst::selectMode) && (e->state() == Qt::LeftButton) ) {
        int chose_width = second_p.x()-first_p.x();
        int chose_height = second_p.y()-first_p.y();
        chose_rect->setSize(chose_width, chose_height);
        chose_rect->setBrush( QColor( Qt::lightGray ) );
        chose_rect->show();

        canvas()->update();
    }
#endif

    if (!np->error_state) {
        set_message_text();
    }

}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::contentsMouseReleaseEvent                                ****/
/******************************************************************************************/
void FigureEditor::contentsMouseReleaseEvent(QMouseEvent* e)
{
    int clutter_idx;
    int center_x, center_y;
    int add_select, use_region;
    double zx, zy, zf, *dist_vector;
    Q3CanvasItemList canvas_item_list;
    Q3CanvasItemList::Iterator it;

    mousePosition->setPosition(np, this, e);

    switch(mouseMode) {
        case GConst::zoomMode:
            zx = (visibleWidth()  - 1.0) / (abs(selectRegion->width())-1);
            zy = (visibleHeight() - 1.0) / (abs(selectRegion->height())-1);
            zf = ((zx < zy) ? zx : zy);

            center_x = (int) floor(zf*(c_pmXL + selectRegion->x() + (selectRegion->width()  - 1)/2.0));
            center_y = (int) floor(zf*(c_pmYL + selectRegion->y() + (selectRegion->height() - 1)/2.0));

            // setMouseMode(GConst::selectMode);
            delete selectRegion;
            selectRegion = (Q3CanvasRectangle *) NULL;

            zoom *= zf;
            regenCanvas(center_x,center_y);
            break;
        case GConst::scrollMode:
            center_x = (int) floor(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0);
            center_y = (int) floor(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0);
            regenCanvas(center_x,center_y);
            break;
        case GConst::measPathMode:
            dist_vector = (double *) malloc( np->map_clutter->num_clutter_type * sizeof(double));
            printf("Path End Point (%d, %d)\n", mousePosition->grid_x, mousePosition->grid_y);

            printf("Calling get_path_clutter: (%d, %d) --> (%d, %d)\n", meas_path_x, meas_path_y, mousePosition->grid_x, mousePosition->grid_y);
            np->get_path_clutter(meas_path_x, meas_path_y, mousePosition->grid_x, mousePosition->grid_y, dist_vector);

            for (clutter_idx=0; clutter_idx<=np->map_clutter->num_clutter_type-1; clutter_idx++) {
                printf("Clutter type %2d Dist = %15.10f (%s)\n", clutter_idx, dist_vector[clutter_idx], np->map_clutter->description[clutter_idx]);
            }
            setMouseMode(GConst::selectMode);
            free(dist_vector);
            break;
        case GConst::drawPolygonMode:
            addPolygon(GConst::mouseRelease, e);
            break;
        case GConst::addCellMode:
            if (e->button() == Qt::LeftButton) {
                sprintf(np->line_buf, "add_cell -posn_x %9.7f -posn_y %9.7f -num_sector 1",
                    mousePosition->posn_x, mousePosition->posn_y);
                np->process_command( np->line_buf);
            }

            setMouseMode(GConst::selectMode);
            break;
        case GConst::moveCellMode:
            sprintf(np->line_buf, "move_cell -cell_idx %d -posn_x %9.7f -posn_y %9.7f",
                    selectCell->getCellIdx(), mousePosition->posn_x, mousePosition->posn_y);
                np->process_command( np->line_buf);
            selectCell = (GCellClass *) NULL;
            setMouseMode(GConst::selectMode);
            break;
        // 2007-03-27 MOD
        /**
        case GConst::toggleNoiseMode:
            sprintf(np->line_buf, "toggle_noise");
            np->process_command( np->line_buf);
            setMouseMode(GConst::selectMode);
            break;
         */
        case GConst::selectMode:
            if (e->button() == Qt::LeftButton) {
                if (e->state() & Qt::ShiftModifier) {
                    add_select = 1;
                } else {
                    add_select = 0;
                }
                if ( selectRegion && (selectRegion->width() || selectRegion->height())) {
                    use_region = 1;
                } else {
                    use_region = 0;
                }
                if ( use_region ) {
                    canvas_item_list = canvas()->collisions(selectRegion->rect());
                } else {
                    canvas_item_list = canvas()->collisions(QPoint(mousePosition->pixel_x, mousePosition->pixel_y));
                }

                int found = 0;
                for (it=canvas_item_list.begin(); it!=canvas_item_list.end(); ++it) {
                    if ( (*it)->rtti() == GConst::cellRTTI ) {
                        selectCell = (GCellClass *) *it;
                        found++;
                    }
                }
                if (!add_select) {
                    select_cell_list->reset();
                }
                if (found) {
                    for (it=canvas_item_list.begin(); it!=canvas_item_list.end(); ++it) {
                        if ( (*it)->rtti() == GConst::cellRTTI ) {
                            selectCell = (GCellClass *) *it;
                            if ( (add_select) && (!use_region) ) {
                                select_cell_list->toggle_elem(selectCell->getCellIdx());
                            } else {
                                select_cell_list->ins_elem(selectCell->getCellIdx(), 0);
                            }
                        }
                    }
                }

                setVisibility(GConst::cellRTTI);

                if ( selectRegion ) {
                    delete selectRegion;
                    selectRegion = (Q3CanvasRectangle *) NULL;
                }

                emit selection_changed(select_cell_list);
            }
            break;
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::contentsMouseDoubleClickEvent                            ****/
/******************************************************************************************/
void FigureEditor::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
    switch(mouseMode) {
        case GConst::drawPolygonMode:
            addPolygon(GConst::mouseDoubleClick, e);
            break;
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::setMouseMode                                             ****/
/******************************************************************************************/
void FigureEditor::setMouseMode(int mode)
{
printf("setMouseMode: mode = %d\n", mode); fflush(stdout);
    if (mode != -1) {
        mouseMode = mode;
    }

    switch (mouseMode) {
        case GConst::selectMode:                 setCursor( Qt::ArrowCursor );                     break;
        case GConst::toggleNoiseMode:            setCursor( Qt::ArrowCursor );                     break;
        case GConst::zoomMode:                   setCursor(QCursor(XpmIcon::zoomin_icon_normal));  break;
        case GConst::addCellMode:                setCursor( Qt::CrossCursor );                     break;
        case GConst::measPathMode:               setCursor( Qt::CrossCursor );                     break;
        case GConst::scrollMode:                 setCursor(QCursor(XpmIcon::openhand_icon_normal)); break;
        case GConst::drawPolygonMode:            setCursor( Qt::CrossCursor );                     break;
        case GConst::moveCellMode:               setCursor( Qt::ArrowCursor );                     break;
        case GConst::copyCellMode:               setCursor( Qt::CrossCursor );                     break;
        case GConst::trackCellMode:              setCursor( Qt::CrossCursor );                     break;
        case GConst::trackSubnetMode:            setCursor( Qt::CrossCursor );                     break;
        case GConst::trackMapLayerPolygonMode:   setCursor( Qt::CrossCursor );                     break;
        case GConst::trackRoadTestDataMode:      setCursor( Qt::CrossCursor );                     break;
        case GConst::trackSignalLevelMode:       setCursor( Qt::CrossCursor );                     break;
        case GConst::trackClutterMode:           setCursor( Qt::CrossCursor );                     break;
        case GConst::trackClutterPropModelMode:  setCursor( Qt::CrossCursor );                     break;
        case GConst::ModClutterCoeff:            setCursor( Qt::CrossCursor );                     break;
        case GConst::rulerStartMode:             setCursor( QCursor(XpmIcon::x_icon));             break;
        case GConst::rulerMeasMode:              setCursor( QCursor(XpmIcon::x_icon));             break;
        case GConst::addRoadTestDataMode:        setCursor( Qt::CrossCursor );                     break;

        case GConst::powerMeterMode:
             setCursor( Qt::CrossCursor );
             power_meter_line = (Q3CanvasLine *) NULL;                               break;
        default:
            CORE_DUMP;
            break;
    }

    if (    (mouseMode == GConst::selectMode)
         || (mouseMode == GConst::rulerMeasMode) ) {
        emit selection_changed(select_cell_list);
    }

    bool enable = (mouseMode == GConst::selectMode) ? true : false;

    bool vis_enable;
    switch (mouseMode) {
        case GConst::selectMode:
        case GConst::zoomMode:
        case GConst::scrollMode:
        case GConst::trackCellMode:
        case GConst::trackSubnetMode:
        case GConst::trackMapLayerPolygonMode:
        case GConst::trackRoadTestDataMode:
        case GConst::trackSignalLevelMode:
        case GConst::trackClutterMode:
        case GConst::trackClutterPropModelMode:
            vis_enable = true;
            break;
        default:
            vis_enable = false;
            break;
    }

    main_window->menuBar()->setEnabled( enable );
    main_window->fileTools->setEnabled( enable );
    main_window->visibility_window->setEnabled( vis_enable );
    // main_window->info_window->setEnabled( enable );
    main_window->command_window->setEnabledButtons( enable );
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::mouseMenu                                                ****/
/******************************************************************************************/
void FigureEditor::setTrackClutterPropModelMode(int pm_idx)
{
    select_pm_idx = pm_idx;
    setMouseMode(GConst::trackClutterPropModelMode);
}

/*
 * - 4/10/2008 CG
 */
void FigureEditor::setModClutterCoef(int pm_idx)
{
    select_pm_idx = pm_idx;
    setMouseMode(GConst::ModClutterCoeff);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackClutter                                             ****/
/******************************************************************************************/
void FigureEditor::trackClutter()
{
    int map_i, map_j, clutter_idx;
    int pm_idx;
    PropModelClass *pm;
    QString qstr;

    InfoWindow *iw = main_window->info_window;

    map_i = DIV(mousePosition->grid_x-np->map_clutter->offset_x, np->map_clutter->map_sim_res_ratio);
    map_j = DIV(mousePosition->grid_y-np->map_clutter->offset_y, np->map_clutter->map_sim_res_ratio);
    clutter_idx = np->map_clutter->get_clutter_type(map_i, map_j);
    if (clutter_idx != -1) {
        qstr += QString("Clutter Type: ") + QString(np->map_clutter->description[clutter_idx]) + "\n";
        qstr += QString("Clutter Map Coordinates: %1 %2\n").arg(map_i).arg(map_j);
        qstr += QString("Clutter Index: %1\n").arg(clutter_idx);

        for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
            pm = np->prop_model_list[pm_idx];
            if (pm->type() == CConst::PropClutterFull) {
                qstr += QString("PROP_MODEL_%1:").arg(pm_idx);
                if (pm->get_strid()) {
                    qstr += " \""+ QString(pm->get_strid()) + "\"";
                }
                qstr += "\n";
                if ( ((ClutterPropModelFullClass *) pm)->useheight ) {
                    qstr += QString("Antenna Height Coefficient: %1\n").arg(((ClutterPropModelFullClass *) pm)->mvec_x[0]);
                }
                qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterPropModelFullClass *) pm)->mvec_x[2*clutter_idx + ((ClutterPropModelFullClass *) pm)->useheight]);
                qstr += QString("Clutter Constant: %1\n").arg(((ClutterPropModelFullClass *) pm)->mvec_x[2*clutter_idx + ((ClutterPropModelFullClass *) pm)->useheight + 1]);
            } else if (pm->type() == CConst::PropClutterSymFull) {
                qstr += QString("PROP_MODEL_%1:").arg(pm_idx);
                if (pm->get_strid()) {
                    qstr += " \"" + QString(pm->get_strid()) + "\"";
                }
                qstr += "\n";
                if ( ((ClutterSymFullPropModelClass *) pm)->useheight ) {
                    qstr += QString("Antenna Height Coefficient: %1\n").arg(((ClutterSymFullPropModelClass *) pm)->mvec_x[0]);
                }
                qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterSymFullPropModelClass *) pm)->mvec_x[2*clutter_idx + ((ClutterSymFullPropModelClass *) pm)->useheight]);
                qstr += QString("Clutter Constant: %1\n").arg(((ClutterSymFullPropModelClass *) pm)->mvec_x[2*clutter_idx + ((ClutterSymFullPropModelClass *) pm)->useheight + 1]);
            } else if (pm->type() == CConst::PropClutterWtExpo) {
                qstr += QString("PROP_MODEL_%1:").arg(pm_idx);
                if (pm->get_strid()) {
                    qstr += " \"" + QString(pm->get_strid()) + "\"";
                }
                qstr += "\n";
                if ( ((ClutterWtExpoPropModelClass *) pm)->useheight ) {
                    qstr += QString("Antenna Height Coefficient: %1\n").arg(((ClutterWtExpoPropModelClass *) pm)->mvec_x[0]);
                }
                qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterWtExpoPropModelClass *) pm)->mvec_x[2*clutter_idx + ((ClutterWtExpoPropModelClass *) pm)->useheight]);
                qstr += QString("Clutter Constant: %1\n").arg(((ClutterWtExpoPropModelClass *) pm)->mvec_x[2*clutter_idx + ((ClutterWtExpoPropModelClass *) pm)->useheight + 1]);
            } else if (pm->type() == CConst::PropClutterWtExpoSlope) {
                qstr += QString("PROP_MODEL_%1:").arg(pm_idx);
                if (pm->get_strid()) {
                    qstr += " \"" + QString(pm->get_strid()) + "\"";
                }
                qstr += "\n";
                if ( ((ClutterWtExpoSlopePropModelClass *) pm)->useheight ) {
                    qstr += QString("Antenna Height Coefficient: %1\n").arg(((ClutterWtExpoSlopePropModelClass *) pm)->mvec_x[0]);
                }
                qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterWtExpoSlopePropModelClass *) pm)->mvec_x[clutter_idx + ((ClutterWtExpoSlopePropModelClass *) pm)->useheight]);
            } else if (pm->type() == CConst::PropClutterExpoLinear) {
                qstr += QString("PROP_MODEL_%1:").arg(pm_idx);
                if (pm->get_strid()) {
                    qstr += " \"" + QString(pm->get_strid()) + "\"";
                }
                qstr += "\n";
                if ( ((ClutterExpoLinearPropModelClass *) pm)->useheight ) {
                    qstr += QString("Antenna Height Coefficient: %1\n").arg(((ClutterExpoLinearPropModelClass *) pm)->mvec_x[0]);
                }
                qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterExpoLinearPropModelClass *) pm)->mvec_x[clutter_idx + ((ClutterExpoLinearPropModelClass *) pm)->useheight]);
            }
        }
    }

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackClutterPropModel                                    ****/
/******************************************************************************************/
void FigureEditor::trackClutterPropModel()
{
    int map_i, map_j, clutter_idx;
    QString qstr;
    GenericClutterPropModelClass *pm = (GenericClutterPropModelClass *) np->prop_model_list[select_pm_idx];

    InfoWindow *iw = main_window->info_window;

    map_i = DIV(mousePosition->grid_x-pm->offset_x, pm->clutter_sim_res_ratio);
    map_j = DIV(mousePosition->grid_y-pm->offset_y, pm->clutter_sim_res_ratio);

    if ( (map_i >= 0) && (map_i <= pm->npts_x-1) && (map_j >= 0) && (map_j <= pm->npts_y-1) ) {
        clutter_idx = map_j*pm->npts_x + map_i;
    } else {
        clutter_idx = -1;
    }

    if (clutter_idx != -1) {
        qstr += QString("PROP_MODEL_%1:").arg(select_pm_idx);
        if (pm->get_strid()) {
            qstr += " \"" + QString(pm->get_strid()) + "\"";
        }
        qstr += "\n";

        qstr += QString("Clutter Map Coordinates: %1 %2\n").arg(map_i).arg(map_j);
        qstr += QString("Clutter Index: %1\n").arg(clutter_idx);
        qstr += QString("NPTS_X: %1\n").arg(pm->npts_x);
        qstr += QString("NPTS_Y: %1\n").arg(pm->npts_y);

#if MDBG
        qstr += QString("OFFSET_X: %1\n").arg(pm->offset_x);
        qstr += QString("OFFSET_Y: %1\n").arg(pm->offset_y);
#endif

        qstr += QString("CLUTTER_SIM_RES_RATIO: %1\n").arg(pm->clutter_sim_res_ratio);

        if (pm->type() == CConst::PropClutterFull) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("Clutter Coefficient: %1\n").arg(pm->mvec_x[2*clutter_idx + pm->useheight]);
            qstr += QString("Clutter Constant: %1\n").arg(pm->mvec_x[2*clutter_idx + pm->useheight + 1]);
        } else if (pm->type() == CConst::PropClutterSymFull) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("Clutter Coefficient: %1\n").arg(((ClutterSymFullPropModelClass *) pm)->mvec_x[2*clutter_idx + pm->useheight]);
            qstr += QString("Clutter Constant: %1\n").arg(((ClutterSymFullPropModelClass *) pm)->mvec_x[2*clutter_idx + pm->useheight + 1]);
        } else if (pm->type() == CConst::PropClutterWtExpo) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("Clutter Coefficient: %1\n").arg(pm->mvec_x[2*clutter_idx + pm->useheight]);
            qstr += QString("Clutter Constant: %1\n").arg(pm->mvec_x[2*clutter_idx + pm->useheight + 1]);
        } else if (pm->type() == CConst::PropClutterWtExpoSlope) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("R0: %1\n").arg(((ClutterWtExpoSlopePropModelClass *) pm)->r0);
            qstr += QString("Clutter Coefficient: %1\n").arg(pm->mvec_x[clutter_idx + pm->useheight]);
        } else if (pm->type() == CConst::PropClutterExpoLinear) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("R0: %1\n").arg(((ClutterExpoLinearPropModelClass *) pm)->r0);
            qstr += QString("Clutter Coefficient: %1\n").arg(pm->mvec_x[clutter_idx + pm->useheight]);
        } else if (pm->type() == CConst::PropClutterGlobal) {
            if ( pm->useheight ) {
                qstr += QString("Antenna Height Coefficient: %1\n").arg(pm->mvec_x[0]);
            }
            qstr += QString("Clutter Coefficient: %1\n").arg(pm->mvec_x[clutter_idx + pm->useheight]);
        }
    }

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}

/*
 *  - 4/10/2008 CG
 */
void FigureEditor::modCltCoeff()
{
    int map_i, map_j, clutter_idx;
    QString qstr;
    GenericClutterPropModelClass *pm = (GenericClutterPropModelClass *) np->prop_model_list[select_pm_idx];

    modCltCoefDia* modCltDia = new modCltCoefDia(np);

    map_i = DIV(mousePosition->grid_x-pm->offset_x, pm->clutter_sim_res_ratio);
    map_j = DIV(mousePosition->grid_y-pm->offset_y, pm->clutter_sim_res_ratio);

    if ( (map_i >= 0) && (map_i <= pm->npts_x-1) && (map_j >= 0) && (map_j <= pm->npts_y-1) ) {
        clutter_idx = map_j*pm->npts_x + map_i;
    } else {
        clutter_idx = -1;
    }

    if (clutter_idx != -1) {
        modCltDia->set_pm_idx(select_pm_idx);
        modCltDia->clt_coodx_lineEdit->setText(QString("%1").arg(map_i));
        modCltDia->clt_coody_lineEdit->setText(QString("%1").arg(map_j));
        modCltDia->clt_idx_lineEdit->setText(QString("%1").arg(clutter_idx));
        modCltDia->clt_size_lineEdit->setText(QString("%1").arg(pm->clutter_sim_res_ratio));

        if (pm->type() == CConst::PropClutterFull) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[2*clutter_idx + pm->useheight]));
        } else if (pm->type() == CConst::PropClutterSymFull) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[2*clutter_idx + pm->useheight]));
        } else if (pm->type() == CConst::PropClutterWtExpo) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[2*clutter_idx + pm->useheight]));
        } else if (pm->type() == CConst::PropClutterWtExpoSlope) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[clutter_idx + pm->useheight]));
        } else if (pm->type() == CConst::PropClutterExpoLinear) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[clutter_idx + pm->useheight]));
        } else if (pm->type() == CConst::PropClutterGlobal) {
            modCltDia->clt_coef_lineEdit->setText(QString("%1").arg(pm->mvec_x[clutter_idx + pm->useheight]));
        }
    }

    modCltDia->show();
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackRoadTestData                                        ****/
/******************************************************************************************/
void FigureEditor::trackRoadTestData()
{
    int grid_x_min, grid_x_max, grid_y_min, grid_y_max;
    int cell_idx, sector_idx, rtd_idx;
    int i, scan_type, min_i, max_i, new_i;
    int is_visible, found, num_digit;
    double dbl_x_min, dbl_x_max, dbl_y_min, dbl_y_max;
    double pwr_db;
    SectorClass *sector;
    RoadTestPtClass *rtp;
    QString qstr;

    canvas_to_xy(dbl_x_min, dbl_y_min, mousePosition->pixel_x-3, mousePosition->pixel_y+3);
    canvas_to_xy(dbl_x_max, dbl_y_max, mousePosition->pixel_x+3, mousePosition->pixel_y-3);
    grid_x_min = (int) floor(dbl_x_min + 1.0);
    grid_x_max = (int) floor(dbl_x_max);
    grid_y_min = (int) floor(dbl_y_min + 1.0);
    grid_y_max = (int) floor(dbl_y_max);

    int is_by_cell = (np->preferences->rtd_view_pref==CConst::RTDbyCell ? 1 : 0);
    VisibilityWindow *vw = main_window->visibility_window;
    InfoWindow *iw = main_window->info_window;

    ListClass<DoubleIntIntClass> *rtd_list = new ListClass<DoubleIntIntClass>(30);

    if (np->num_cell >= 2) {
        num_digit = (int) ceil( log((double) np->num_cell-1) / log(10.0) );
    } else {
        num_digit = 1;
    }

    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        sector = np->cell_list[cell_idx]->sector_list[sector_idx];

        if (is_by_cell) {
            is_visible = 1;
        } else {
            pwr_db = rtp->pwr_db;

            if (pwr_db < RoadTestPtClass::level_list[0]) {
                scan_type = 0;
            } else if (pwr_db >= RoadTestPtClass::level_list[RoadTestPtClass::num_level-1]) {
                scan_type = RoadTestPtClass::num_level;
            } else {
                min_i = 0;
                max_i = RoadTestPtClass::num_level-1;
                while (max_i > min_i+1) {
                    new_i = (min_i + max_i) / 2;
                    if (pwr_db >= RoadTestPtClass::level_list[new_i]) {
                        min_i = new_i;
                    } else {
                        max_i = new_i;
                    }
                }
                scan_type = max_i;
            }
            is_visible = vw->vec_vis_rtd_level[scan_type];
        }
        if (is_visible) {
            if (    (grid_x_min <= rtp->posn_x) && (rtp->posn_x <= grid_x_max)
                 && (grid_y_min <= rtp->posn_y) && (rtp->posn_y <= grid_y_max) ) {
                rtd_list->ins_elem(DoubleIntIntClass(rtp->pwr_db, cell_idx, sector_idx), 0);
            }
        }
    }

    if (rtd_list->getSize()) {
        char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
        char *gwcsccsstr = CVECTOR(6);
        found = 1;
        rtd_list->sort();

        qstr = tr("Road Test Data") + ": " + QString(np->preferences->pwr_str_short) + "\n";
        for (i=rtd_list->getSize()-1; i>=0; i--) {
            cell_idx = (*rtd_list)[i].getInt(0);
            sector_idx = (*rtd_list)[i].getInt(1);
            double pwr_db = (*rtd_list)[i].getDouble();

            sector = np->cell_list[cell_idx]->sector_list[sector_idx];
            hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
            sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);

            qstr += QString("(%1) %2 ").arg(rtd_list->getSize()-i,num_digit).arg(cell_idx, num_digit) + QString(hexstr) + " "
                               + QString(gwcsccsstr)
                               + QString(" %1 ").arg(pwr_db - np->preferences->pwr_offset, 7, 'f', 3);

            if (i) {
                qstr += "\n";
            }
        }
        free(hexstr);
        free(gwcsccsstr);
    }
    
    delete rtd_list;

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackCell                                                ****/
/******************************************************************************************/
void FigureEditor::trackCell()
{
    int cell_idx, sector_idx, tti_idx;
    int found, i;
    int menu_id;
    QString cell_str, qstr;
    CellClass *cell;
    PHSSectorClass *sector;

    InfoWindow *iw = main_window->info_window;

    Q3CanvasItemList l=canvas()->collisions(QPoint(mousePosition->pixel_x, mousePosition->pixel_y));
    Q3CanvasItemList::Iterator it;

    found = 0;
    for (it=l.begin(); it!=l.end(); ++it) {
        if ( (*it)->rtti() == GConst::cellRTTI ) {
            selectCell = (GCellClass *) *it;
            found++;
        }
    }

    if (found > 1) {
        Q3PopupMenu* selectCellMenu = new Q3PopupMenu( this, "Select Cell" );
        for (it=l.begin(); it!=l.end(); ++it) {
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
        cell = np->cell_list[cell_idx];

        qstr = QString();
        qstr += QString().sprintf("CELL_%d:", cell_idx);
        if (cell->strid) {
            qstr += QString().sprintf(" \"%s\"", cell->strid);
        }
        qstr += "\n";
        qstr += QString().sprintf("    POSN: %9.7f %9.7f\n", np->idx_to_x(cell->posn_x), np->idx_to_y(cell->posn_y));
        qstr += QString().sprintf("    BM_IDX: %d\n", cell->bm_idx);
        qstr += QString().sprintf("    COLOR: %d\n", cell->color);

        if (np->technology() == CConst::PHS) {
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                qstr += QString().sprintf("        COMMENT: %s\n", (sector->comment ? sector->comment : ""));
                qstr += QString().sprintf("        CSID:");
                if (sector->csid_hex) {
                    qstr += " ";
                    for (unsigned int byte_num = 0; byte_num<=PHSSectorClass::csid_byte_length-1; byte_num++) {
                        qstr += QString().sprintf("%.2X", sector->csid_hex[byte_num]);
                    }
                }
                qstr += "\n";
                qstr += QString().sprintf("        CS_NUMBER: %d\n", sector->gw_csc_cs);

                for (tti_idx=0; tti_idx<=SectorClass::num_traffic-1; tti_idx++) {
                    qstr += QString().sprintf("        MEAS_CTR_%d: %.7f\n",   tti_idx, sector->meas_ctr_list[tti_idx]);
                }
                qstr += QString().sprintf("        ANGLE_DEG: %.7f\n",      sector->antenna_angle_rad*180.0/PI);
                qstr += QString().sprintf("        ANTENNA_TYPE: %d\n",       sector->antenna_type);
                qstr += QString().sprintf("        ANTENNA_HEIGHT: %.7f\n", sector->antenna_height);
                qstr += QString().sprintf("        PROP_MODEL: %d\n",         sector->prop_model);
                qstr += QString().sprintf("        TX_POWER: %.7f\n",       sector->tx_pwr);
                qstr += QString().sprintf("        NUM_PHYSICAL_TX: %d\n",    sector->num_physical_tx);
                qstr += QString().sprintf("        HAS_ACCESS_CONTROL: %d\n", sector->has_access_control);
                qstr += QString().sprintf("        CNTL_CHAN_SLOT: %d\n",     sector->cntl_chan_slot);
                qstr += QString().sprintf("        NUM_UNUSED_FREQ: %d\n",    sector->num_unused_freq);
                for (i=0; i<=sector->num_unused_freq-1; i++) {
                    qstr += QString().sprintf("            UNUSED_FREQ_%d: %d\n", i, sector->unused_freq[i]);
                }
            }
        }

        iw->setText(qstr);
        main_window->toggle_info_window(GConst::visShow);
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackSignalLevel                                         ****/
/******************************************************************************************/
void FigureEditor::trackSignalLevel()
{
    int cell_idx, sector_idx;
    int i, found, use, num_digit;
    double pwr, pwr_db;
    CellClass *cell;
    SectorClass *sector;
    QString qstr;

    char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
    char *gwcsccsstr = CVECTOR(6);
    InfoWindow *iw = main_window->info_window;

    ListClass<DoubleIntIntClass> *pt_list = new ListClass<DoubleIntIntClass>(30);

    if (np->num_cell >= 2) {
        num_digit = (int) ceil( log((double) np->num_cell-1) / log(10.0) );
    } else {
        num_digit = 1;
    }

    for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];

            use = 1;
            if (sector->prop_model == -1) {
                use = 0;
            } else {
                pwr = sector->tx_pwr*sector->comp_prop(np, mousePosition->grid_x, mousePosition->grid_y);
                if (pwr > 0.0) {
                    pwr_db = 10*log(pwr)/log(10.0);
                } else {
                    use = 0;
                }
            }

            if (use) {
                pt_list->ins_elem(DoubleIntIntClass(pwr_db, cell_idx, sector_idx), 0);
            }
        }
    }

    if (pt_list->getSize()) {
        found = 1;
        pt_list->sort();

        qstr = tr("Signal Level") + ": " + QString(np->preferences->pwr_str_short) + "\n";
        for (i=pt_list->getSize()-1; i>=0; i--) {
            cell_idx = (*pt_list)[i].getInt(0);
            sector_idx = (*pt_list)[i].getInt(1);
            double pwr_db = (*pt_list)[i].getDouble();

            sector = np->cell_list[cell_idx]->sector_list[sector_idx];
            hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
            sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);

            qstr += QString("(%1) %2 ").arg(pt_list->getSize()-i,num_digit).arg(cell_idx, num_digit) + QString(hexstr) + " "
                               + QString(gwcsccsstr)
                               + QString(" %1 ").arg(pwr_db - np->preferences->pwr_offset, 7, 'f', 3);

            if (i) {
                qstr += "\n";
            }
        }
    }

    free(hexstr);
    free(gwcsccsstr);
    delete pt_list;

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::trackMapLayerPolygon                                     ****/
/******************************************************************************************/
void FigureEditor::trackMapLayerPolygon()
{
    int grid_x, grid_y;
    int map_layer_idx, p_idx, found;
    MapLayerClass *ml = (MapLayerClass *) NULL;
    PolygonClass *p;
    QString qstr;

    InfoWindow *iw = main_window->info_window;

    canvas_to_xy(grid_x, grid_y, mousePosition->pixel_x, mousePosition->pixel_y);

    found = 0;
    for (map_layer_idx=0; map_layer_idx<=np->map_layer_list->getSize()-1; map_layer_idx++) {
        ml = (*np->map_layer_list)[map_layer_idx];
        for (p_idx=0; p_idx<=ml->num_polygon-1; p_idx++) {
            p = ml->polygon_list[p_idx];
            if (p->in_bdy_area(grid_x, grid_y)) {
                if (!found) {
                    found = 1;
                } else {
                    qstr += "\n";
                }
                qstr += "MAP LAYER: " + QString(ml->name) + "\n";
                qstr += QString("MAP LAYER INDEX: ")   + QString("%1\n").arg(map_layer_idx);
                qstr += QString("POLYGON   INDEX: ")   + QString("%1\n").arg(p_idx);
            }
        }
    }

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::rulerInfo                                                ****/
/******************************************************************************************/
void FigureEditor::rulerInfo()
{
    double posn_x0, posn_y0, posn_x1, posn_y1, dx, dy;
    QString qstr;

    InfoWindow *iw = main_window->info_window;

    posn_x0 = np->idx_to_x(ruler->xa);
    posn_y0 = np->idx_to_x(ruler->ya);
    posn_x1 = np->idx_to_x(ruler->xb);
    posn_y1 = np->idx_to_x(ruler->yb);
    dx = (ruler->xb - ruler->xa)*np->resolution;
    dy = (ruler->yb - ruler->ya)*np->resolution;

    qstr = tr("Ruler") + "\n";
    qstr += QString(tr("Start")+" "+tr("Point")) + QString(": %1, %2\n").arg(posn_x0, 10, 'f', 1).arg(posn_y0, 10, 'f', 1);
    qstr += QString(tr("End")+"   "+tr("Point")) + QString(": %1, %2\n").arg(posn_x1, 10, 'f', 1).arg(posn_y1, 10, 'f', 1);
    qstr += QString(tr("Delta")+ "      ") + QString(": %1, %2\n").arg(dx, 10, 'f', 1).arg(dy, 10, 'f', 1);
    qstr += QString(tr("Distance")) + QString(": %1\n").arg(sqrt(dx*dx+dy*dy), 10, 'f', 1);

    iw->setText(qstr);

    main_window->toggle_info_window(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::set_message_text                                         ****/
/******************************************************************************************/
void FigureEditor::set_message_text()
{
    double dis, pwr, pwr_db, dx, dy;

    QString str  = "";

#if 1
    str += QString("PX: %1").arg(mousePosition->pixel_x, 5);
    str += QString(" PY: %1").arg(mousePosition->pixel_y, 5);
    str += QString(" GX: %1").arg(mousePosition->grid_x, 6);
    str += QString(" GY: %1").arg(mousePosition->grid_y, 6);
#endif

    if (np->mode != CConst::noGeomMode) {
        str += QString("    X: %1").arg(mousePosition->posn_x, 10, 'f', 1);
        str += QString("    Y: %1").arg(mousePosition->posn_y, 10, 'f', 1);

        if (np->coordinate_system == CConst::CoordUTM) {
            str += "    " + tr("Lon") + QString(": %1").arg(mousePosition->lon_deg, 10, 'f', 6);
            str += "    " + tr("Lat") + QString(": %1").arg(mousePosition->lat_deg, 10, 'f', 6);
        }
    }

    if (mouseMode == GConst::powerMeterMode) {
        dis = sqrt( (mousePosition->posn_x-select_posn_x)*(mousePosition->posn_x-select_posn_x) +
                    (mousePosition->posn_y-select_posn_y)*(mousePosition->posn_y-select_posn_y) );
        str += "\n    " + tr("Distance") + QString(": %1").arg(dis, 10, 'f', 6);

        pwr = selectSector->tx_pwr*selectSector->comp_prop(np, mousePosition->grid_x, mousePosition->grid_y);
        if (pwr > 0.0) {
            pwr_db = 10*log(pwr)/log(10.0) - np->preferences->pwr_offset;
            str += "    " + tr("Pow") + QString(": %1").arg(pwr_db, 10, 'f', 6) + " " + QString(np->preferences->pwr_str_short);
        } else {
            str += "    " + tr("Pow") + QString(": -INF");
        }
    } else if (mouseMode == GConst::rulerStartMode) {
        str += "\n    " + tr("Select Point to begin ruler measurement");
    } else if (mouseMode == GConst::rulerMeasMode) {
        dx  = (ruler->xb - ruler->xa)*np->resolution;
        dy  = (ruler->yb - ruler->ya)*np->resolution;
        dis = sqrt( dx*dx + dy*dy );
        str += "\n   " + tr("dX") + QString(": %1").arg(dx, 10, 'f', 1);
        str +=   "   " + tr("dY") + QString(": %1").arg(dy, 10, 'f', 1);
        str +=   "    " + tr("DISTANCE") + QString(": %1").arg(dis, 10, 'f', 6);

#if 0
    } else {
        if (np->map_height) {
            map_i = (mousePosition->grid_x-np->map_height->offset_x)/np->map_height->map_sim_res_ratio;
            map_j = (mousePosition->grid_y-np->map_height->offset_y)/np->map_height->map_sim_res_ratio;
            if ( (0<=map_i) && (map_i<=np->map_height->npts_x-1) && (0<=map_j) && (map_j<=np->map_height->npts_y-1) ) {
                height_idx = np->map_height->npts_x*(np->map_height->npts_y-1-map_j) + map_i;
                val = 0;
                for (int bi=0; bi<=np->map_height->bytes_per_sample-1; bi++) {
                    m = (int) np->map_height->data[height_idx*np->map_height->bytes_per_sample+bi];
                    val = (val<<8)|m;
                }
                if (val == ((((1<<(8*(np->map_height->bytes_per_sample-1)))-1)<<8)|255)) {
                    str += "    " + tr("Height") + ":  NONE";
                } else {
                    str += "    " + tr("Height") + QString(":  %1").arg((np->map_height->hstarti+val)*np->map_height->h_resolution, 10, 'f', 6);
                }
            } else {
                str += "    " + tr("Height") + ":  NONE";
            }
        }
#endif

    }

    emit message_text_changed(str);
}
/******************************************************************************************/
