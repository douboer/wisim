/******************************************************************************************/
/**** FILE: tooltip.cpp                                                                ****/
/******************************************************************************************/

#include <qapplication.h>
#include <qpainter.h>

#include <stdlib.h>

#include "wisim_gui.h"
#include "wisim.h"
#include "doubleintint.h"
#include "list.h"
#include "main_window.h"
#include "map_clutter.h"
#include "phs.h"
#include "polygon.h"
#include "pref.h"
#include "road_test_data.h"
#include "tooltip.h"
#include "traffic_type.h"
#include "visibility_window.h"

extern QFont *fixed_width_font;

// porting to QT4
/*
DynamicTip::DynamicTip( QWidget * parent )
    : QToolTip( parent )
{
    // no explicit initialization needed
    setFont(*fixed_width_font);
}

void DynamicTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget()->inherits( "FigureEditor" ) ) {
        return;
    }

    QString s;
    QRect r( ((FigureEditor *)parentWidget())->tip(pos, s) );
    if ( !r.isValid() ) {
        return;
    }

    showText ( const QPoint & pos, const QString & text, QWidget * w, const QRect & rect )

    tip( r, s );
}
*/

QRect FigureEditor::tip( const QPoint& p, QString& s )
{
    int grid_x, grid_y, map_i, map_j, subnet_idx, clutter_idx;
    int num_digit;
    QString tmp;
    QPoint q = viewportToContents(p);
    Q3CanvasItemList list = canvas()->collisions(q);
    Q3CanvasItemList::Iterator it;
    NetworkClass *np = get_np();

    if (np->num_cell >= 2) {
        num_digit = (int) ceil( log((double) np->num_cell-1) / log(10.0) );
    } else {
        num_digit = 1;
    }

    int found = 0, tt_idx;
    s = "";
    for (it = list.begin(); it != list.end(); it++) {
        if ( (*it)->rtti() == GConst::cellRTTI ) {
            int cell_idx = ((GCellClass *)(*it))->getCellIdx();
            if (!found) {
                found = 1;
            } else {
                s += "\n";
            }
            s += QString("cell_%1").arg(cell_idx);
            s += QString().sprintf(": %.6d", ((PHSSectorClass *) np->cell_list[cell_idx]->sector_list[0])->gw_csc_cs);
        }
    }

    if (mouseMode == GConst::trackSubnetMode) {
        canvas_to_xy(grid_x, grid_y, q.x(), q.y());
        SubnetClass *subnet;

        for (tt_idx=0; tt_idx<=np->num_traffic_type-1; tt_idx++) {
            for (subnet_idx=0; subnet_idx<=np->num_subnet[tt_idx]-1; subnet_idx++) {
                subnet = np->subnet_list[tt_idx][subnet_idx];
                if (subnet->p->in_bdy_area(grid_x, grid_y)) {
                    if (!found) {
                        found = 1;
                    } else {
                        s += "\n";
                    }
                    s += np->traffic_type_list[tt_idx]->name();
                    s += ":";
                    s += subnet->strid;
                }
            }
        }
    }  else if (mouseMode == GConst::trackClutterMode) {
        canvas_to_xy(grid_x, grid_y, q.x(), q.y());
        map_i = DIV(grid_x-np->map_clutter->offset_x, np->map_clutter->map_sim_res_ratio);
        map_j = DIV(grid_y-np->map_clutter->offset_y, np->map_clutter->map_sim_res_ratio);
        if (!found) {
            found = 1;
        } else {
            s += "\n";
        }
        clutter_idx = np->map_clutter->get_clutter_type(map_i, map_j);
        if (clutter_idx == -1) {
            s += tr("Clutter") + ": NONE";
        } else {
            s += tr("Clutter") + ": " + QString(np->map_clutter->description[clutter_idx]);
        }
    }

    if (found) {
        return QRect( p.x(), p.y(), 1, 1);
    } else {
        return QRect( 0, 0, -1, -1);
    }
}

