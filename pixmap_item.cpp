/******************************************************************************************/
/**** FILE: pixmap_item.cpp                                                            ****/
/**** Michael Mandell 2/6/04                                                           ****/
/******************************************************************************************/

#include <string.h>
#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3PointArray>
// #include <qgb18030codec.h>

#include "antenna.h"
#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "clutter_data_analysis.h"
#include "coverage.h"
#include "gconst.h"
#include "list.h"
#include "map_background.h"
#include "map_clutter.h"
#include "map_height.h"
#include "map_layer.h"
#include "pixmap_item.h"
#include "pref.h"
#include "road_test_data.h"

/******************************************************************************************/
/**** FUNCTION: PixmapItem::PixmapItem                                                 ****/
/******************************************************************************************/
PixmapItem::PixmapItem(FigureEditor *editor) : Q3CanvasRectangle(editor->canvas())
{
    setPixmapGeom(editor);
    setZ(GConst::backgroundZ);
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::rtti                                                       ****/
/******************************************************************************************/
int PixmapItem::rtti() const {return GConst::backgroundPixmapRTTI;}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::fill                                                       ****/
/******************************************************************************************/
void PixmapItem::fill(const QColor & color)
{
    pixmap.fill(color);
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::setPixmapGeom                                              ****/
/******************************************************************************************/
void PixmapItem::setPixmapGeom(FigureEditor *editor)
{
    int canvas_w = editor->canvas()->width();
    int canvas_h = editor->canvas()->height();

#if CDEBUG
    printf("canvas()->width()  = %d\n", canvas_w);
    printf("canvas()->height() = %d\n", canvas_h);
#endif

    setSize(canvas_w, canvas_h);
    move(0, 0);

    pixmap.resize(width(),height());
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawMapClutter                                             ****/
/******************************************************************************************/
void PixmapItem::drawMapClutter(FigureEditor *editor, char *vec_vis)
{
    QImage pmImage(width(),height(),32);

    double zoom   = editor->getZoom();
    NetworkClass *np = editor->get_np();
    MapClutterClass *mc = np->map_clutter;

    int m, img_i, img_j, color;
    for (int i=0; i<=width()-1; i++) {
        img_i = (int) floor( (editor->c_pmXL + i)/(zoom*mc->map_sim_res_ratio)
                        - (double) (mc->offset_x - 0.5) / mc->map_sim_res_ratio );
        for (int j=0; j<=height()-1; j++) {
            img_j = - (int) ceil((editor->c_pmYL + j)/(zoom*mc->map_sim_res_ratio)
                        + (double) (mc->offset_y - 0.5) / mc->map_sim_res_ratio );

            m = mc->get_clutter_type(img_i, img_j);
            if (m==-1) {
                color = 0xFFFFFF;
//             } else if (m == 255) { // xxxxxxx ??
//                 color = 0;
            } else if (vec_vis[m]) {
                color = mc->color[m];
            } else {
                color = 0xFFFFFF;
            }
            pmImage.setPixel(i,j,color);
        }
    }
#if 1
    QPainter painter(&pixmap);
    // Do not exist in QT 4
    // painter.setRasterOp(Qt::NotXorROP);
    painter.drawImage(0, 0, pmImage);
#else
    pixmap.convertFromImage(pmImage, 0);
#endif
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawClutterPropModel                                       ****/
/******************************************************************************************/
void PixmapItem::drawClutterPropModel(FigureEditor *editor, char *vec_vis)
{
    int clutter_type, img_i, img_j, color, pm_idx;
    double min_neg, max_pos;
    QImage pmImage(width(),height(),32);

    double zoom   = editor->getZoom();
    NetworkClass *np = editor->get_np();
    GenericClutterPropModelClass *pm;

    for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
        if (np->prop_model_list[pm_idx]->is_clutter_model()) {
            pm = (GenericClutterPropModelClass *) np->prop_model_list[pm_idx];
            pm->get_min_max_color(min_neg, max_pos);
            for (int i=0; i<=width()-1; i++) {
                img_i = (int) floor( (editor->c_pmXL + i)/(zoom*pm->clutter_sim_res_ratio)
                                - (double) (pm->offset_x - 0.5) / pm->clutter_sim_res_ratio );
                for (int j=0; j<=height()-1; j++) {
                    img_j = - (int) ceil((editor->c_pmYL + j)/(zoom*pm->clutter_sim_res_ratio)
                                + (double) (pm->offset_y - 0.5) / pm->clutter_sim_res_ratio );

                    if ( (img_i >= 0) && (img_i <= pm->npts_x-1) && (img_j >= 0) && (img_j <= pm->npts_y-1) ) {
                        clutter_type = img_j*pm->npts_x + img_i;
                    } else {
                        clutter_type = -1;
                    }
                    if (clutter_type==-1) {
                        color = 0xFFFFFF;
                    } else {
                        color = pm->get_color(clutter_type, min_neg, max_pos);
                    }
                    pmImage.setPixel(i,j,color);
                }
            }
            QPainter painter(&pixmap);
            // Do not exist in QT 4
            // painter.setRasterOp(Qt::NotXorROP);
            painter.drawImage(0, 0, pmImage);
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawResolutionGrid                                         ****/
/******************************************************************************************/
void PixmapItem::drawResolutionGrid(FigureEditor *editor)
{
    int posn_x, posn_y, x_canvas, y_canvas;

    double zoom = editor->getZoom();

    if (zoom > 10) {
        QPainter painter(&pixmap);
        painter.setBrush(Qt::black);
        painter.setPen(Qt::black);
        // Do not exist in QT 4
        //painter.setRasterOp(Qt::AndROP);

        int min_x, max_x, min_y, max_y;
        editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
        editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);
        max_x++;
        max_y++;

        for (posn_x=min_x; posn_x<=max_x; posn_x++) {
            for (posn_y=min_y; posn_y<=max_y; posn_y++) {
                editor->xy_to_canvas(x_canvas, y_canvas, posn_x, posn_y);
                painter.drawEllipse(x_canvas-2, y_canvas-2, 5, 5);
            }
        }
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawMapHeight                                              ****/
/******************************************************************************************/
void PixmapItem::drawMapHeight(FigureEditor *editor)
{
    QImage pmImage(width(),height(),32);

    double zoom   = editor->getZoom();
    NetworkClass *np = editor->get_np();
    MapHeightClass *mh = np->map_height;

    int bps = np->map_height->bytes_per_sample;
    int m, img_i, img_j, val;
    QRgb color;
    for (int i=0; i<=width()-1; i++) {
        img_i = (int) ( (editor->c_pmXL + i)/(zoom*mh->map_sim_res_ratio)
                        - (double) mh->offset_x / mh->map_sim_res_ratio );
        for (int j=0; j<=height()-1; j++) {
            img_j = mh->npts_y-1 + (int) ((editor->c_pmYL + j)/(zoom*mh->map_sim_res_ratio)
                        + (double) mh->offset_y / mh->map_sim_res_ratio );
            if (    (img_i < 0) || (img_i > mh->npts_x-1)
                 || (img_j < 0) || (img_j > mh->npts_y-1) ) {
                color = 0xFFFFFF;
            } else {
                val = 0;
                int index = img_j * mh->npts_x + img_i;
                for (int bi=0; bi<=bps-1; bi++) {
                    m = (int) np->map_height->data[index*bps+bi];
                    val = (val<<8)|m;
                }
                if (val == ((((1<<(8*(bps-1)))-1)<<8)|255)) {
                    color = qRgb(0,0,255);  /* use blue if no data available */
                } else {
                    m = (unsigned int) floor(val * 255.0 / (np->map_height->hsize-1));
                    color = qRgb(m,m,m);
                }
            }
            pmImage.setPixel(i,j,color);
        }
    }

    pixmap.convertFromImage(pmImage, 0);
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawMapBackground                                          ****/
/******************************************************************************************/
void PixmapItem::drawMapBackground(FigureEditor *editor)
{
    QImage pmImage(width(),height(),32);

    double zoom   = editor->getZoom();
    NetworkClass *np = editor->get_np();
    MapBackgroundClass *mb = np->map_background;

    double map_dist_sq = (mb->xmax-mb->xmin)*(mb->xmax-mb->xmin) + (mb->ymax-mb->ymin)*(mb->ymax-mb->ymin);
    double img_dist_sq = mb->mapImage->width()*mb->mapImage->width() + mb->mapImage->height()*mb->mapImage->height();
    double map_sim_res_ratio = sqrt(map_dist_sq/img_dist_sq); 

    int offset_x = (int) ( -mb->xmin / map_sim_res_ratio );
    int offset_y = (int) ( mb->mapImage->height()-1 + mb->ymin/map_sim_res_ratio );

    int img_i, img_j;
    QRgb color;
    for (int i=0; i<=width()-1; i++) {
        img_i = offset_x + (int) ((editor->c_pmXL + i)/(zoom*map_sim_res_ratio));
        for (int j=0; j<=height()-1; j++) {
            img_j = offset_y + (int) ((editor->c_pmYL + j)/(zoom*map_sim_res_ratio));
            if (    (img_i < 0) || (img_i > mb->mapImage->width()-1)
                 || (img_j < 0) || (img_j > mb->mapImage->height()-1) ) {
                color = 0xFFFFFF;
            } else {
                color = mb->mapImage->pixel(img_i, img_j);
            }
            pmImage.setPixel(i,j,color);
            // printf("PIXEL (%d,%d) set to mapdata[%d][%d] height = %d color %d,%d,%d\n",
            //    i, j, img_i, img_j, val, (color>>16)&255,(color>>8)&255,color&255);
        }
    }

    pixmap.convertFromImage(pmImage, 0);
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawSystemBoundary                                         ****/
/******************************************************************************************/
void PixmapItem::drawSystemBoundary(FigureEditor *editor)
{
    QPainter painter(&pixmap);
    QColor penColor = editor->systemBoundaryOutlineColor;
    painter.setPen(penColor);

    // Do not exist in QT 4
    //painter.setRasterOp(Qt::NotXorROP);
    NetworkClass *np = editor->get_np();

    painter.setBrush( QBrush(NetworkClass::system_bdy_color, editor->systemBoundaryBrushStyle) );

    /* Outer Bounding Box */
    /* Pt outside RECT (  min_x,   max_x,   min_y, max_y  ) not on canvas */
    /* Pt inside  RECT (min_x+1, max_x-1, min_y+1, max_y-1)     on canvas */
    int min_x, max_x, min_y, max_y;

#if 0
    editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
    editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);
#endif
    /*  all draw items use c_pm..  - Chengan 20190627
     *  canvas()->width()  = c_pmXH - c_pmXL
     *  canvas()->height() = c_pmYH - c_pmYL
     */
    editor->canvas_to_xy(min_x, min_y, 0, (editor->c_pmYH - editor->c_pmYL)-1);
    editor->canvas_to_xy(max_x, max_y, (editor->c_pmXH - editor->c_pmXL)-1, 0);
    max_x++;
    max_y++;

    editor->drawPolygonPainter(np->system_bdy, painter, min_x, max_x, min_y, max_y);
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawSubnet                                                 ****/
/******************************************************************************************/
void PixmapItem::drawSubnet(FigureEditor *editor, char **vec_vis)
{
    int tt_idx, subnet_idx;
    PolygonClass *p;

    QPainter painter(&pixmap);
    QColor penColor = Qt::black;
    painter.setPen(penColor);
    // Do not exist in QT 4
    //painter.setRasterOp(Qt::NotXorROP);

    NetworkClass *np = editor->get_np();

    /* Outer Bounding Box */
    /* Pt outside RECT (  min_x,   max_x,   min_y, max_y  ) not on canvas */
    /* Pt inside  RECT (min_x+1, max_x-1, min_y+1, max_y-1)     on canvas */
    int min_x, max_x, min_y, max_y;
    editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
    editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);
    max_x++;
    max_y++;

    for (tt_idx=0; tt_idx<=np->num_traffic_type-1; tt_idx++) {
        for (subnet_idx=0; subnet_idx<=np->num_subnet[tt_idx]-1; subnet_idx++) {
            if (vec_vis[tt_idx][subnet_idx]) {
                painter.setBrush( QBrush(np->subnet_list[tt_idx][subnet_idx]->color) );
                p = np->subnet_list[tt_idx][subnet_idx]->p;
                editor->drawPolygonPainter(p, painter, min_x, max_x, min_y, max_y);
            }
        }
    }
}

/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawCoverage                                               ****/
/******************************************************************************************/
void PixmapItem::drawCoverage(FigureEditor *editor, char **vec_vis)
{
    int i, color_idx, p_idx, cvg_idx, cell_idx;
    int canvas_x, canvas_y, flag;
    CellClass *cell;
    PolygonClass *p;

    QPainter painter(&pixmap);
    QPainterPath path;
    QColor penColor = Qt::black;
    painter.setPen(penColor);
    // Do not exist in QT 4
    //painter.setRasterOp(Qt::NotXorROP);

    NetworkClass *np = editor->get_np();
    CoverageClass *cvg;

    /* Outer Bounding Box */
    /* Pt outside RECT (  min_x,   max_x,   min_y, max_y  ) not on canvas */
    /* Pt inside  RECT (min_x+1, max_x-1, min_y+1, max_y-1)     on canvas */
    int min_x, max_x, min_y, max_y;
    editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
    editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);
    max_x++;
    max_y++;

    QColor color;
    for (cvg_idx=0; cvg_idx<=np->num_coverage_analysis-1; cvg_idx++) {
        cvg = np->coverage_list[cvg_idx];
        flag = 0;

        // if run_coverage command do not execute, WiSim will crash
        if (cvg->polygon_list == 0)
            continue;

        // draw coverage polygons
        for (p_idx=0; p_idx<=cvg->scan_list->getSize()-1; p_idx++) {
            p = cvg->polygon_list[p_idx];
            if (p) {
                if (vec_vis[cvg_idx][p_idx]) {
                    color = cvg->color_list[p_idx];

                    //set transparency (0~1) 
                    color.setAlphaF(0.8);

                    painter.setBrush( QBrush(color, editor->coverageBrushStyle) );
                    editor->drawPolygonPainter(p, painter, min_x, max_x, min_y, max_y);
                    flag = 1;
                }
            }
        }

        // mark the simulaiton cells selected with solid black triangle
        if (cvg->clipped_region && flag) {
            for (i=0; i<=cvg->cell_list->getSize()-1; i++) {
                cell_idx = (*cvg->cell_list)[i];
                cell = np->cell_list[cell_idx];
                editor->xy_to_canvas(canvas_x, canvas_y, cell->posn_x, cell->posn_y);

                painter.setBrush( QBrush(Qt::black, Qt::SolidPattern) );
                // porting to QT4
                path.moveTo(canvas_x, canvas_y - 20);
                path.lineTo( (int) floor(canvas_x-20*sqrt(3.0)/2), (int) floor(canvas_y + 20*0.5) );
                path.lineTo( (int) floor(canvas_x+20*sqrt(3.0)/2), (int) floor(canvas_y + 20*0.5) );
                path.lineTo(canvas_x, canvas_y - 20);
                painter.drawPath(path);
            }
        }
    }
}

/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawDirAntenna                                             ****/
/******************************************************************************************/
void PixmapItem::drawDirAntenna(FigureEditor *editor)
{
    int cell_idx, sector_idx, canvas_x, canvas_y, is_omni;
    CellClass *cell;
    SectorClass *sector;
    NetworkClass *np = editor->get_np();
    Q3PointArray pa(3);

    QPainter painter(&pixmap);
    QPainterPath path;
    QColor penColor = AntennaClass::color;
    painter.setPen(penColor);

    double cos_phi = cos(PI/6.0);
    double sin_phi = sin(PI/6.0);
    double arrow_len = 30.0;
    double tip_len   = 5.0;

    for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            is_omni = np->antenna_type_list[sector->antenna_type]->get_is_omni();

            editor->xy_to_canvas(canvas_x, canvas_y, cell->posn_x, cell->posn_y);
            if (is_omni) {
                painter.setBrush(Qt::NoBrush);
                //painter.drawEllipse(canvas_x-10, canvas_y-10, 20, 20);
                painter.drawEllipse(canvas_x-25, canvas_y-25, 50, 50);
            } else {
                // - Chengan 20190626
                // draw pie
                // different sector with different color
                painter.setRenderHint(QPainter::Antialiasing, true);
                int radius   = 50;
                int spanAng  = 16*60;
                int startAng = 16*sector->antenna_angle_rad*180/PI - spanAng/2.;
                QRectF rect(canvas_x-radius, canvas_y-radius, radius << 1, radius << 1);
                painter.setPen(Qt::darkRed);
                painter.setBrush(QBrush(Qt::gray, Qt::Dense6Pattern));
                painter.drawPie(rect, startAng, spanAng);

                // draw arrow
                painter.setPen(Qt::green);
                painter.setBrush( QBrush(AntennaClass::color, Qt::SolidPattern) );
                double cos_theta = cos(sector->antenna_angle_rad);
                double sin_theta = sin(sector->antenna_angle_rad);
                path.moveTo(canvas_x, canvas_y);
                canvas_x += (int) floor( arrow_len*cos_theta + 0.5);
                canvas_y += (int) floor(-arrow_len*sin_theta + 0.5);
                path.lineTo(canvas_x, canvas_y);
                painter.drawPath(path);
                pa[0] = QPoint(canvas_x, canvas_y);
                canvas_x = pa[0].x() + (int) floor( -tip_len*cos_phi*cos_theta - tip_len*sin_phi*sin_theta  + 0.5);
                canvas_y = pa[0].y() - (int) floor( -tip_len*cos_phi*sin_theta + tip_len*sin_phi*cos_theta  + 0.5);
                pa[1] = QPoint(canvas_x, canvas_y);
                canvas_x = pa[0].x() + (int) floor( -tip_len*cos_phi*cos_theta + tip_len*sin_phi*sin_theta  + 0.5);
                canvas_y = pa[0].y() - (int) floor( -tip_len*cos_phi*sin_theta - tip_len*sin_phi*cos_theta  + 0.5);
                pa[2] = QPoint(canvas_x, canvas_y);
                painter.drawPolygon(pa);
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawMapLayer                                               ****/
/******************************************************************************************/
void PixmapItem::drawMapLayer(FigureEditor *editor, char *vec_vis)
{
    int map_layer_idx, polygon_idx, line_idx, text_idx;
    int x_canvas, y_canvas;
    MapLayerClass *map_layer;
    PolygonClass *p;
    LineClass *line;
    QString s;

    QPainter painter(&pixmap);
    QColor penColor = Qt::black;
    painter.setPen(penColor);
    painter.setFont( QFont("Arial",14,QFont::Bold) );

    NetworkClass *np = editor->get_np();

    /* Outer Bounding Box */
    /* Pt outside RECT (  min_x,   max_x,   min_y, max_y  ) not on canvas */
    /* Pt inside  RECT (min_x+1, max_x-1, min_y+1, max_y-1)     on canvas */
    int min_x, max_x, min_y, max_y;
    editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
    editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);
    max_x++;
    max_y++;

    for (map_layer_idx=0; map_layer_idx<=np->map_layer_list->getSize()-1; map_layer_idx++) {
        map_layer = (*(np->map_layer_list))[map_layer_idx];
        if (vec_vis[map_layer_idx]) {
            painter.setBrush( QBrush(map_layer->color) );

            for (polygon_idx=0; polygon_idx<=map_layer->num_polygon-1; polygon_idx++) {

                p = map_layer->polygon_list[polygon_idx];

                editor->drawPolygonPainter(p, painter, min_x, max_x, min_y, max_y);
            }

            painter.setPen( QPen(QBrush(map_layer->color), 1, Qt::SolidLine));
            for (line_idx=0; line_idx<=map_layer->num_line-1; line_idx++) {

                line = map_layer->line_list[line_idx];

                editor->drawLinePainter(line, painter, min_x, max_x, min_y, max_y);
            }
            painter.setPen(penColor);
            for (text_idx=0; text_idx<=map_layer->num_text-1; text_idx++) {
                if (    (map_layer->posn_x[text_idx] >= min_x)
                     && (map_layer->posn_x[text_idx] <= max_x)
                     && (map_layer->posn_y[text_idx] >= min_y)
                     && (map_layer->posn_y[text_idx] <= max_y) ) {
                    // s = editor->gbc->toUnicode(map_layer->text[text_idx], strlen(map_layer->text[text_idx]));
                    s = QString::fromLocal8Bit(map_layer->text[text_idx]);
                    editor->xy_to_canvas(x_canvas, y_canvas,
                                             map_layer->posn_x[text_idx],
                                             map_layer->posn_y[text_idx]);
                    painter.drawText(x_canvas, y_canvas, s);
                }
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawRoadTestData                                             ****/
/******************************************************************************************/
void PixmapItem::drawRoadTestData(FigureEditor *editor, int vec_vis_size, char *vec_vis, int *vec_cell_idx, int *vec_sector_idx, char *vec_vis_rtd_level)
{
    int cell_idx, sector_idx, rtd_idx;
    int x_canvas, y_canvas;
    int is_visible, scan_type, min_i, max_i, new_i;
    double pwr_db;

    SectorClass *sector;
    RoadTestPtClass *rtp;

    QPainter painter(&pixmap);
    QColor penColor = Qt::black;
    painter.setPen(penColor);

    NetworkClass *np = editor->get_np();
    int is_by_cell = (np->preferences->rtd_view_pref==CConst::RTDbyCell ? 1 : 0);

    int min_x, max_x, min_y, max_y;
    editor->canvas_to_xy(min_x, min_y, 0, editor->canvas()->height()-1);
    editor->canvas_to_xy(max_x, max_y, editor->canvas()->width()-1, 0);

    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        sector = np->cell_list[cell_idx]->sector_list[sector_idx];
        if (sector->vis_rtd) {
            if (is_by_cell) {
                painter.setBrush( QBrush(sector->road_test_pt_color) );
                painter.setPen(   sector->road_test_pt_color );
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
                is_visible = vec_vis_rtd_level[scan_type];
                if (is_visible) {
                    painter.setBrush( QBrush(RoadTestPtClass::color_list[scan_type]));
                    painter.setPen(   RoadTestPtClass::color_list[scan_type]);
                }
            }
            if (is_visible) {
                editor->xy_to_canvas(x_canvas, y_canvas, rtp->posn_x, rtp->posn_y);
                painter.drawEllipse(x_canvas-2, y_canvas-2, 5, 5);
#if (CDEBUG && 0)
                int c_x_canvas, c_y_canvas;
                editor->xy_to_canvas(c_x_canvas, c_y_canvas,
                    np->cell_list[cell_idx]->posn_x, np->cell_list[cell_idx]->posn_y);
                painter.drawLine(c_x_canvas, c_y_canvas, x_canvas, y_canvas);
#endif
            }
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: CellClass::view_label                                                  ****/
/******************************************************************************************/
QString PixmapItem::view_label(int rtti_val, NetworkClass *np, int pref_val)
{
    QString s;

    switch(rtti_val) {
        case GConst::roadTestDataRTTI:
            s = qApp->translate("PixmapItem", "Road Test Data") + " ";
            switch(pref_val) {
                case CConst::RTDbyCell:
                    s += "(" + qApp->translate("PixmapItem", "Cell") + ")";
                    break;
                case CConst::RTDbyLevel:
                    s += "(" + qApp->translate("PixmapItem", "Signal Level") + ")";
                    break;
                default:
                    CORE_DUMP; break;
            }   
            break;
        default:
            CORE_DUMP; break;
    }

    return(s);
}
/******************************************************************************************/
