/******************************************************************************************/
/**** FILE: pixmap_item.h                                                              ****/
/**** Michael Mandell 2/24/04                                                          ****/
/******************************************************************************************/

#ifndef PIXMAP_ITEM_H
#define PIXMAP_ITEM_H

#include <stdlib.h>
#include <q3canvas.h>
//Added by qt3to4:
#include <QPixmap>

class FigureEditor;
class NetworkClass;

/******************************************************************************************/
/**** CLASS: PixmapItem                                                                ****/
/******************************************************************************************/
class PixmapItem : public Q3CanvasRectangle
{
public:
    PixmapItem(FigureEditor *editor);
    int rtti() const;
    void execute_update() { update(); };
    void drawMapClutter(FigureEditor *editor, char *vec_vis);
    void drawMapHeight(FigureEditor *editor);
    void drawMapBackground(FigureEditor *editor);
    void drawSystemBoundary(FigureEditor *editor);
    void drawDirAntenna(FigureEditor *editor);
    void drawSubnet(FigureEditor *editor, char **vec_vis);
    void drawMapLayer(FigureEditor *editor, char *vec_vis);
    void drawCoverage(FigureEditor *editor, char **vec_vis);
    void drawClutterPropModel(FigureEditor *editor, char *vec_vis);
    void drawRoadTestData(FigureEditor *editor, int vec_vis_size, char *vec_vis, int *vec_cell_idx, int *vec_sector_idx, char *vec_vis_rtd_level);
    void fill(const QColor &);
    static QString view_label(int rtti_val, NetworkClass *np, int cell_name_pref);

    void drawResolutionGrid(FigureEditor *editor);

protected:
    void drawShape( QPainter & );
    void setPixmapGeom(FigureEditor *editor);
    QPixmap pixmap;
};
/******************************************************************************************/

#endif
