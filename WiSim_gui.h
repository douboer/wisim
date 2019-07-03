#ifndef WISIM_GUI_H
#define WISIM_GUI_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <q3canvas.h>
//Added by qt3to4:
#include <Q3PointArray>
#include <QPixmap>
#include <QMouseEvent>
#include "datachart.h"

class QGb18030Codec;
class Q3CanvasItemList;

class CellClass;
class DynamicTip;
class GCellClass;
class LineClass;
class PixmapItem;
class PolygonClass;
class PositionClass;
class PrefClass;
class PrintDialog;
class PrintRect;
class RulerClass;
class SectorClass;
class VisibilityCheckItem;
class VisibilityList;
class VisibilityWindow;

template<class T> class ListClass;

/******************************************************************************************/
/**** CLASS: FigureEditor                                                              ****/
/******************************************************************************************/
class FigureEditor : public Q3CanvasView
{
    Q_OBJECT

    friend class FindDialog;

public:
    FigureEditor(Q3Canvas *, class NetworkClass *, QWidget* parent=0, const char* name=0, Qt::WFlags f=0);
    ~FigureEditor();
    void clear(int rtti_val);
    double getZoom();
    void setZoom(double z);
    NetworkClass *get_np();
    void setVisibilityWindow(VisibilityWindow *vw);
    void xy_to_canvas(int&, int&, int, int);
    void xy_to_canvas(int&, int&, double, double);
    void canvas_to_xy(int&, int&, int, int);
    void canvas_to_xy(double&, double&, int, int);
    void comp_effective_polygon_segment(QVector<QPoint> &pa, const int n, const int *x, const int *y,
                                          const int min_x, const int max_x,
                                          const int min_y, const int max_y);
    void drawPolygonPainter(PolygonClass *p, QPainter &painter,
                                      const int min_x, const int max_x, const int min_y, const int max_y);
    void drawLinePainter(LineClass *p, QPainter &painter,
                                      const int min_x, const int max_x, const int min_y, const int max_y);
    void scrollCanvas(int x, int y);
    void plotCoverage(int cvg_idx_0, int cvg_idx_1 = -1);
    void clear_select_list();

    int c_pmXL, c_pmYL, c_pmXH, c_pmYH;
    // QGb18030Codec *gbc;

    PixmapItem *bgr_pm;

    QColor systemBoundaryOutlineColor;
    Qt::BrushStyle systemBoundaryBrushStyle;

    QColor mapLayerPolygonOutlineColor;
    Qt::BrushStyle mapLayerPolygonBrushStyle;

    Qt::BrushStyle coverageBrushStyle;

    void addPolygon(int mouseAction, QMouseEvent* e);
    void cellPolyChose(int mouseAction, QMouseEvent* e );
    QRect tip( const QPoint& p, QString& s);
    void set_message_text();
    char *excel_file;

    QString geometry_filename;

    ListClass<int> *select_cell_list;
    
    int if_show_handset_anomalies;
    int if_show_handset_noise;
    int if_show_3ho;
    int ho_show_index;
    int max_index, min_index;
    int start_index, end_index;

    std::vector <int> vec_anomaly_x;
    std::vector <int> vec_anomaly_y;
    std::vector <int> vec_source_cs;
    std::vector <int> vec_dest_cs;
    std::vector <int> vec_anomaly_index;
    
    std::vector <int> vec_noise_begin_x;
    std::vector <int> vec_noise_begin_y;
    std::vector <int> vec_noise_end_x;
    std::vector <int> vec_noise_end_y;
    std::vector <int> vec_noise_index;

    std::vector <int> vec_cs_new_x;
    std::vector <int> vec_cs_new_y;
    std::vector <int> vec_cs_index;
    std::vector <int> vec_dis;

    int num_noise;	
    int num_cs_new;	
    int num_anomalies;

    xyGraph* datagraph;
    xyGraph* pdg;

public slots:
    void clear();
    void regenCanvas(int cx, int cy);
    void resizeCanvas();
    void zoomToFit();
    void zoomByFactor(double zf);
    void zoomIn();
    void zoomOut();
    void setZoomMode();
    void mouseMenu(int found, Q3CanvasItemList *canvas_item_list);

    void toggleNoise();

    void run_match();
    void delete_st_data();

    void setSelectCell(int c);

    void setGroupColor();
    void deleteGroup();

    void setMouseMode(int mode = -1);
    void setScrollMode();

    void measPath();
    void printCanvasInfo();
    void printClutterInfo();
    void setVisibility(int rtti_val);
    void plotPathLoss();
    void toggleRoadTestData();
    void toggleRoadTestDataGroup();
    void deleteCell();
    void groupSectors();
    void print();
    void setTrackClutterPropModelMode(int pm_idx);
    void setModClutterCoef(int pm_idx);

    void create_prop_analysis_dialog();
    void create_run_simulation_dialog();
    void create_create_subnet_dialog();
    void create_subnet_traffic_dialog();
    void create_set_param_dialog();
    void create_report_dialog(int type);
    void create_cvg_mgr_dialog();
    void create_pref_dialog();
    void create_draw_polygon_dialog(int type);
    // xxxxxxxxx void create_read_map_background_dialog();
    void create_image_registration_dialog(QString imagefile);
    void create_prop_mgr_dialog();
    void create_read_mif_dialog();
    void create_shift_dialog(int);
    void create_set_rtd_threshold_dialog();
    void create_set_group_shape_dialog();
    void create_convert_road_test_data_dialog();
    void create_find_dialog();
    void create_import_st_data_dialog();

    void trackRoadTestData();
    void trackClutter();
    void trackClutterPropModel();
    void trackCell();
    void trackMapLayerPolygon();
    void trackSignalLevel();
    void rulerInfo();

    void showHandover();
    void modCltCoeff();

    void get_handset_anomalies(char* filename);
    void show_handset_anomalies();
    void show_handset_noise();
    
    void show_cs_new_location();
		
    // Test Slots
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void testSlotA();
    void testSlotB();
    void testSlotC();
    void testSlotD();

protected:
    virtual void contentsMousePressEvent(QMouseEvent*);
    virtual void contentsMouseMoveEvent(QMouseEvent*);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent*);

signals:
    void message_text_changed(const QString&);
    void done_drawing_polygon(Q3CanvasPolygon* polygon);
    void selection_changed(ListClass<int> *select_cell_list);

private:
    void regenPrintRect();
    NetworkClass *np;
    GCellClass* selectCell;
    QPoint moving_start;
    Q3CanvasRectangle *selectRegion;
    RulerClass *ruler;
    double zoom, margin;
    int mouseMode;
    int scrollX, scrollY;
    int select_pm_idx;
    SectorClass *selectSector;
    double select_posn_x, select_posn_y;

    int meas_path_x, meas_path_y;

    DynamicTip *t;
    VisibilityWindow *visibility_window;
    QPrinter *printer;

    PrintRect *printrect;

    PrintDialog *pcp;

    PositionClass *mousePosition;

    Q3CanvasLine *power_meter_line;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: GCellClass                                                                ****/
/******************************************************************************************/
class GCellClass : public Q3CanvasRectangle
{
public:
    GCellClass(FigureEditor *editor, int cell_idx, CellClass *cell);
    ~GCellClass();
    int rtti() const;
    int getCellIdx();
    static void setCellPixmapList();
    static QPixmap *getCellPixmap(int i);
    static void deleteCellPixmapList();
    static void setCellSize(int size_idx);
    static int size, size_selected;
    static QString view_label(NetworkClass *np, int cell_name_pref);

    static int num_sizes;
    static int size_list[];
    static QBitmap **bm_list;
    static QBitmap **selected_bm_list;
    static void set_bitmaps(int size_idx);
    static void clear_bitmaps();

protected:
    void drawShape( QPainter & );

private:
    static QPixmap **pixmap_list;
    static QPixmap **selected_pixmap_list;
    int cell_idx, selected;
    CellClass *cell;
};
/******************************************************************************************/
/**** CLASS: CellText                                                                  ****/
/******************************************************************************************/
class CellText : public Q3CanvasText
{
public:
    CellText(FigureEditor *editor, int cell_idx, char *vec_vis);
    ~CellText();
    int rtti() const;
    int getCellIdx();

private:
    int cell_idx;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: CallConnection                                                            ****/
/******************************************************************************************/
class CallConnection : public Q3CanvasLine
{
public:
    CallConnection(Q3Canvas *canvas, QPoint pa, QPoint pb);
    ~CallConnection();
    int rtti() const;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: RulerPtClass                                                              ****/
/******************************************************************************************/
class RulerPtClass : public Q3CanvasRectangle
{
public:
    RulerPtClass(Q3Canvas *canvas, int x, int y);
    ~RulerPtClass();
    int rtti() const;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: RulerLineClass                                                            ****/
/******************************************************************************************/
class RulerLineClass : public Q3CanvasLine
{
public:
    RulerLineClass(Q3Canvas *canvas, int xa, int ya, int xb, int yb);
    ~RulerLineClass();
    int rtti() const;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: RulerClass                                                                ****/
/******************************************************************************************/
class RulerClass
{
public:
    RulerClass(int xa, int ya, int xb, int yb);
    ~RulerClass();
    RulerPtClass *pta;
    RulerPtClass *ptb;
    RulerLineClass *line;
    int xa, ya, xb, yb;

    void clear();
    void draw(FigureEditor *editor, Q3Canvas *canvas);
    void setPt(int idx, int x, int y);
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: PositionClass                                                             ****/
/******************************************************************************************/
class PositionClass
{
public:
    PositionClass();
    ~PositionClass();
    void setPosition(NetworkClass *np, FigureEditor *editor, QMouseEvent *e);
    friend class FigureEditor;

private:
    int    pixel_x, pixel_y;
    int    grid_x,  grid_y;
    double posn_x,  posn_y;
    double lon_deg, lat_deg;
};
/******************************************************************************************/

int qstringcmp(QString k1, QString k2);
//void create_regerror_dialog(int registration, PrefClass *preferences, unsigned char *reg_info, int ris);
void set_language(int language);

#endif
