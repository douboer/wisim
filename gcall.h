/******************************************************************************************/
/**** FILE: gcall.h                                                                    ****/
/******************************************************************************************/

#ifndef GCALL_H
#define GCALL_H

#include <q3canvas.h>
//Added by qt3to4:
#include <QPixmap>

class CallConnection;
class FigureEditor;
class NetworkClass;
class QPixmap;

/******************************************************************************************/
/**** CLASS: GCallClass                                                                ****/
/******************************************************************************************/
class GCallClass : public Q3CanvasRectangle
{
public:
    GCallClass(FigureEditor *editor, int p_master_idx, int p_traffic_type_idx);
    ~GCallClass();
    int rtti() const;
    CallConnection *connection;
    static void setNetworkStruct( NetworkClass *p_np);
    static void setPixmap();
    static void deletePixmap();

private:
    int master_idx;
    int traffic_type_idx;
    static NetworkClass *np;
    static QPixmap **pixmap;
    void drawShape( QPainter & );
};
/******************************************************************************************/


#endif
