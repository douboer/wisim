#include <qwidget.h>
#include <q3mainwindow.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qcolor.h>
#include <qdialog.h>
#include <q3popupmenu.h>
#include <qprinter.h>
#include <q3canvas.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listview.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <q3tabdialog.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>

#ifndef DATACHART_H
#define DATACHART_H

enum CHARTTYPE { POINTCHART, LINECHART, BARCHART, PIECHART, PIECHART2 };
enum VALUETYPE { ONEDIM, TWODIM };

class dc_dataset{

public:
     int id;
     QString *desc;
     double *data_x;
     double *data_y;
     char **idx;
     int data_length;
     int linesize;
     QColor data_color;
     QColor* pie_data_color;
     double scale_rate;
     double offset_x;
     double offset_y;
     bool visi;
     bool showpoints;
//     struct dc_dataset * next;
};


class dc_settings{

public:
     int id;
     QString *desc;
     bool visi;
     int linesize;
     QColor color;
     double scale_rate;
     double offset_x;
     double offset_y;
     bool showpoints;
};


class dc_settings_general{
public:
    QString title;
    QColor title_color;
    double minx, maxx, miny, maxy;
};


class xyGraph : public QWidget {

     Q_OBJECT

public:

/**********************************************************************************/
/*******  the following methods are written to be invoked by an application  ******/
/**********************************************************************************/

//xyGraph construction fuction
     xyGraph();

//reloaded xyGraph construction function for display a pointchart( 2D data, like(x,y) )
//minx: min x coordinate value
//maxx: max x coordinate value
//miny, maxy: min(max) y coordinate value
//title: title to be shown at the top of the graph
//charttypename: POINTCHART, BARCHART, PIECHART
                 
     xyGraph( double minx, double maxx, double miny, double maxy, 
              QString title = " ", CHARTTYPE charttypename = POINTCHART, QWidget * parent1 = 0 );

//reloaded xyGraph construction function for display a barchart( 1D data )
//maxy: max y(vertical) coordinate value
     xyGraph( double maxy,
              QString title = " ", CHARTTYPE charttypename = BARCHART, QWidget * parent1 = 0 );

//reloaded xyGraph construction function for display a none-coordinate graph, such as PIECHART.
//when it is used to display a coorinate graph, u have to use fuction setCoord() to sepecify the coordinate after the construction fuction
     xyGraph( QString title, CHARTTYPE charttypename, QWidget * parent1 = 0 );

//destruction function
     ~xyGraph();

//add a data set: (x0,y0),(x1,y1),(x2,y2)... in the graph 
//datasetx: pointer to the x value list: x[0],x[1],x[2]...x[num_data-1]
//datasety: pointer to the y value list: y[0],y[1],y[2]...y[num_data-1]
//num_data: number of the point( a point is in a format like (x[i],y[i]) )
//lineSize: size of the line joins each two points( point(x[i],y[i]) and point(x[i+1],y[i+1]) )
//            the default value is 0, which means do not join any points with lines
//Color: the color used to display this data set in the graph
     void addXY( double * datasetx, double * datasety, int num_data, int lineSize = 0, QColor Color = Qt::white );
 
//reloaded function for adding a 1D data set: y0,y1,y2.....
//index: a pointer to an array of string which contains the comments of y[i]
//       thus, each element in the data set is like ( index[i], y[i] )
//datasety: pointer to the value list: y[0],y[1],y[2]...y[num_data-1]
     void addXY( char ** index, double * datasety, int num_data, QColor Color = Qt::white );

//reloaded function used to adding a 1D data set without a comment for each element
     void addXY( double * datasety, int num_data, QColor Color = Qt::white );

//remove a data set
//dataset: index of the data set in all data sets
     void removeXY( int dataset );

//set coordinate value
//return true if succeed, otherwise false
     bool setCoord( double minx, double maxx, double miny, double maxy );
 
//set graph title
//color: the color which the title displayed in
     void setTitle( QString title, QColor color );

//reloaded function for setting the title of a PIECHART2 chart
     void setTitle( QString title1, QString title2, QColor color );

//set whether to show the grid or not
//flag: true for show and false for not
     void setShowgrid( bool flag );

//set size of a point in a POINTCHART
     void setPointSize( int size );

//repaint the graph
//u can invoke this function whenever u find the need to refresh it
     void repaintChart( );

//set coordinate description
     void setCoordDesc( QString desc_x = "X", QString desc_y = "Y" );

 //set type of reference on the right of the graph, 0 for normal and 1 for display prop-model
     void setReference( int reference_type );

//set the color of data i in every datasets to datacolor in a PIECHART or PIECHART2 
//before callling this function, the program use default color for each data automatically
     void setPieDataColor( QColor* datacolor ); 

//the following 2 functions are not available yet
     void setExpXY( int expx, int expy );
 
     void setDataStep( double dstepx, double dstepy );


/**********************************************************************************/

private slots:
     void PointChart();
     void LineChart();
     void BarChart();
     void PieChart();
     void About();
     void ExportBmp();
     void ExportJpg();
     void Export( QString );
     void Print();
     void Settings();
     void setPieSet( int );
     void onSelectionChanged( Q3ListViewItem* );
     void onBtnColorClicked( );
     void onBtnTitleColorClicked( );
     void onLinesizeChanged( int );
     void applySettings( );
     void addFromFile( );
     void removeDataset( );
     void viewData( );
     void View_saveData( );
     void zoomIn();
     void zoomOut();
     void scroll();
     void restore();
     void close();
 
private:
     void init();
     void initMenu( CHARTTYPE );
     void paintEvent( QPaintEvent* );
     void drawCoordinate( );
     void drawChartIndex( );
 //    void drawIntCoord( );
     void drawLine( int );
     void drawTitle( QString );
     void drawGrid( );
     void drawPoints( int );
     void drawPointsValue( int, int );
//     void drawzoomGraph();
     void drawLineChart( int );
     void drawBarChart( int );
     void drawOnePie( int, int, int );
     void drawPieChart( int );
     void drawBarValue( int );
     void zoomGraph();

     void mousePressEvent( QMouseEvent* );
     void mouseReleaseEvent( QMouseEvent* );
     void mouseMoveEvent( QMouseEvent* );
     
     void loadSettings( int );
     void saveSettings( int );
     void repaintPageDataset( int );
     void disableWidgets( );
     void enableWidgets( );
     
     void checkBarPieData( double*, int );
     bool IsMouseeventInregion();
     bool IsPointInregion( int, int );
     void resetPainter( );
     int modifiedX( int );
     int modifiedY( int );
     double calStep( double, double, int );
     void dc_FloatToStr( double, char*, int );
     double nearestValue( double, double, int );
     QColor chooseColor( int );
     QColor chooseColorMore( int );    
     double getRateXFrame();
     double getRateYFrame();
     int getMaxStrLen( char**, int );
     void drawDelta( int, int );
     void setDouble( QString &, double );
 
 //public class members, also called "properties"
 public:
     int max_x, max_y; //max pixel number of the coordinate
     int size_x, size_y; //max size of this dialog
     int title_height; //title height
     int menu_height; //menu height
     int lmargin, rmargin, tmargin, bmargin, tmargin_all; //margin between edges of the widget and the coordniate

 //private class members
 private:
     double dminx, dminy, dmaxx, dmaxy;
     double dminx_old, dminy_old, dmaxx_old, dmaxy_old;
     double dminx_ori, dminy_ori, dmaxx_ori, dmaxy_ori;
     QString title, title1, title2;
     int title_type;
     int pie_color_type;
     QColor title_color;
     int pointsize;
     QString desc_x, desc_y;
     double zoomrate;
     QPixmap *pixmap;
          
     dc_settings_general* settings_general;
     dc_dataset** setlist;
     dc_settings** settings_setlist; 
         
     int num_data_set;
     double ratex;
     double ratey;
     QPrinter *printer;

     int xmousepress, ymousepress;
     int xmouserelease, ymouserelease;
     int xmousecurrent, ymousecurrent;
     int regionstartx, regionstarty;
     int scrollstartx, scrollstarty, scrollendx, scrollendy;
     bool scrollbegin;
     int scrollx_old, scrolly_old;
     int reference_type;

     double regionw, regionh;
     double valuestartx, valueendx, valuestarty, valueendy;
     bool mousepressed;
     QMenuBar *menubar;
     Q3PopupMenu *file;
     Q3PopupMenu *view;
     Q3PopupMenu *options;
     Q3PopupMenu *help;
     int gid, pid, lid, pointcid, linecid, barcid, piecid, scrollid;
     int pie_set_num;
     QPainter *painter;
     QRect *regionrect;
     CHARTTYPE charttype;
     VALUETYPE valuetype;
     bool showgrid;
     bool viewscroll;
     bool intCoord;
     bool cursor_inregion;
     int exp_x, exp_y;
     double grid_size_x, grid_size_y;
     double num_grid_x, num_grid_y;
     double dstepx, dstepy;
     int dexpx, dexpy;
     QString datatext;
     
     QRadioButton* Btn_Grids_Yes;
     QRadioButton* Btn_Grids_No;
     QRadioButton* Btn_Coord_Yes;
     QRadioButton* Btn_Coord_No;
     QLineEdit* LE_minX;
     QLineEdit* LE_maxX;
     QLineEdit* LE_minY;
     QLineEdit* LE_maxY;
     QLineEdit* LE_Title;
     
     Q3TabDialog* Dlg_Settings;
     QWidget* Page_General;
     QWidget* Page_Dataset;
     Q3ListView* LV_Dataset;
     Q3ListViewItem* olditem;
     QLineEdit* LE_Desc;
     QPushButton* Btn_Add;
     QPushButton* Btn_Remove;
     QPushButton* Btn_Data;
     QRadioButton* Btn_Visi_Yes;
     QRadioButton* Btn_Visi_No;
     QPushButton* Btn_Color;
     QPushButton* Btn_Title_Color;
     QSpinBox* SBox_Linesize;
     QCheckBox* CB_showPoints;
     QSpinBox* SBox_Scale;
     QLineEdit* LE_Trans_X;
     QLineEdit* LE_Trans_Y;
     QCheckBox* CB_Grid;
     QCheckBox* CB_Coord;
     QCheckBox* CB_Index;     
     Q3TextEdit* textEditer;
};
#endif
