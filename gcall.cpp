/******************************************************************************************/
/**** PROGRAM: gcall.cpp                                                               ****/
/******************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>

#include "wisim.h"
#include "wisim_gui.h"
#include "gcall.h"
#include "list.h"
#include "gconst.h"
#include "traffic_type.h"

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
QPixmap         ** GCallClass          :: pixmap                 = (QPixmap         **) NULL;
NetworkClass     * GCallClass          :: np                     = (NetworkClass     *) NULL;
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: GCallClass::GCallClass                                                 ****/
/******************************************************************************************/
GCallClass::GCallClass(FigureEditor *editor, int p_master_idx, int p_traffic_type_idx) : Q3CanvasRectangle( editor->canvas() )
{
    int cell_idx, call_x, call_y, cell_x, cell_y;
    CellClass   *cell;
    CallClass   *call;

    master_idx = p_master_idx;
    traffic_type_idx = p_traffic_type_idx;
    call = (CallClass *) (*(np->master_call_list))[master_idx];
    cell_idx = call->cell_idx;
    cell = np->cell_list[cell_idx];

    editor->xy_to_canvas(call_x, call_y, call->posn_x, call->posn_y);
    editor->xy_to_canvas(cell_x, cell_y, cell->posn_x, cell->posn_y);

    setSize(GConst::callSize,GConst::callSize);
    move(call_x-width()/2, call_y-height()/2);
    setZ(9);
    show();

    connection = new CallConnection(editor->canvas(), QPoint(call_x, call_y), QPoint(cell_x, cell_y));
    connection->setPen( QPen(QColor(100,100,100), 1) );
    connection->setZ(9);
    connection->show();
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::~GCallClassCall                                            ****/
/******************************************************************************************/
GCallClass::~GCallClass()
{
    delete connection;
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::rtti                                                       ****/
/******************************************************************************************/
int GCallClass::rtti() const { return GConst::trafficRTTI; }
/******************************************************************************************/
/**** FUNCTION: GCallClass::setNetworkStruct                                           ****/
/******************************************************************************************/
void GCallClass::setNetworkStruct( NetworkClass *p_np) { np = p_np; };
/******************************************************************************************/
/**** FUNCTION: GCallClass::setPixmap                                                  ****/
/******************************************************************************************/
void GCallClass::setPixmap()
{
    int tti;

    pixmap = (QPixmap **) malloc(np->num_traffic_type*sizeof(QPixmap *));
    for (tti=0; tti<=np->num_traffic_type-1; tti++) {
        pixmap[tti] = new QPixmap(GConst::callSize,GConst::callSize);
        pixmap[tti]->fill(QColor(np->traffic_type_list[tti]->get_color()));

        QPainter painter(pixmap[tti]);
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(0, 0, GConst::callSize, GConst::callSize);
    }
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::drawShape                                                  ****/
/******************************************************************************************/
void GCallClass::drawShape(QPainter &p)
{
    p.drawPixmap((int) x(), (int) y(), *(pixmap[traffic_type_idx]));
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::deletePixmap                                               ****/
/******************************************************************************************/
void GCallClass::deletePixmap()
{
    int tti;

    if (pixmap) {
        for (tti=0; tti<=np->num_traffic_type-1; tti++) {
            delete pixmap[tti];
        }
        free(pixmap);
    }

    pixmap = (QPixmap **) NULL;
}
/******************************************************************************************/



