/**************************************************************************************************
 FILE: image_registration.h 
**************************************************************************************************/
#ifndef IMAGE_REGISTRATION_H 
#define IMAGE_REGISTRATION_H

#include <qdialog.h>
#include <qwidget.h>
#include <qvariant.h>
#include <qdialog.h>
#include <q3canvas.h>
#include <qbitmap.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QPixmap>
#include <QLabel>
#include <QMouseEvent>
#include <Q3PtrList>
#include <Q3VBoxLayout>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QPushButton;
class FileChooser;
class QToolButton;
class Q3CanvasRectangle;
class Q3CanvasLine;
class Q3Table;
class Q3Frame;

class FigureEditor;
class NetworkClass;
class AddPoint; 
class ImageRegistration; 
class ProjectSetting ; 

#define MAP_DEBUG 0 

/**************************************************************************************************
   Class CrossItem and Functions.
**************************************************************************************************/
class CrossItem : public Q3CanvasItem
{
public:
    CrossItem ( QPixmap pmp, Q3Canvas * canvas );
    QRect boundingRect() const { return pixmap.rect(); };
    bool collidesWith(const Q3CanvasItem*) const  { return 0; };
    bool collidesWith(const Q3CanvasSprite*, const Q3CanvasPolygonalItem*,
        const Q3CanvasRectangle*, const Q3CanvasEllipse*, const Q3CanvasText*) const  { return 0; } 
    void draw( QPainter& );

    double z_px, z_py;
    QString text;
private:
    QPixmap pixmap;
    QPixmap bitmap;
};

/**************************************************************************************************
 Class CanvasView
**************************************************************************************************/
class CanvasView: public Q3CanvasView {
    Q_OBJECT
public:
    CanvasView(Q3Canvas*, Q3Frame* parent=0);
    ~CanvasView();
    void zoomMap(double scaleX, double scaleY);

    double factor;
    int    pt_idx;
    bool   mousepressed;

    int    cvs_width;
    int    cvs_height;

    AddPoint* add_pt;

    enum MouseStat {
        AddPt,
        ZoomIn,
        ZoomOut,
        ZoomRect
    };
    MouseStat MS;

    QPixmap pm;
    int px, py;
    double zoom_px, zoom_py;        //Pixel of original image.
    Q3PtrList <CrossItem> cross_list;

public slots:
    void contentsMousePressEvent( QMouseEvent* );
    void contentsMouseMoveEvent( QMouseEvent* );
    void contentsMouseReleaseEvent( QMouseEvent* );
    void replotAll( double factor );
    
private:
    int mrect_w, mrect_h;        //Rectangle width of mouse move.
    int rx, ry;
    int cx, cy;

};

class Frame : public Q3Frame {
    Q_OBJECT
public:
    Frame(Q3Canvas*, QDialog* parent=0);
    ~Frame();
    QImage* mapImage;
    QString digi_map_filename;

    Q3Canvas*    delcanvas;
    CanvasView* cvs_view;

#if MAP_DEBUG
    void view_map();
#endif

public slots:
    void init();
    void addDigitMap();
private:
    Q3Canvas*  canvas;
};

/**************************************************************************************************
 Class AddPoint 
**************************************************************************************************/
class AddPoint : public QDialog
{
    Q_OBJECT

public:
    AddPoint( QWidget* parent = 0, const char* name = 0 );
    ~AddPoint();

    QLabel*     pt_idx_lbl;
    QLineEdit*  pt_idx_lineEdit;
    QLabel*     imgx_lbl;
    QLineEdit*  imgx_lineEdit;
    QLabel*     imgy_lbl;
    QLineEdit*  imgy_lineEdit;
    QLabel*     mapx_lbl;
    QLineEdit*  mapx_lineEdit;
    QLabel*     mapy_lbl;
    QLineEdit*  mapy_lineEdit;

    QLabel* deg_lbl1;
    QLabel* deg_lbl2;

    QPushButton* ok_btn;
    QPushButton* cancel_btn;

    static int pt_idx;
    int        imgx;
    int        imgy;
    double     mapx;
    double     mapy;
    
protected:
    Q3GridLayout* AddPointLayout;
    QSpacerItem* spacer4;
    Q3GridLayout* layout1;
    Q3HBoxLayout* layout2;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;

protected slots:
    virtual void languageChange();

public slots:
    bool ok_button_clicked();
    void cancel_button_clicked();

};


/**************************************************************************************************
 Class ImageRegistration 
**************************************************************************************************/
enum CoorType { LlType, UtmType };  
class ImageRegistration : public QDialog
{
    Q_OBJECT

public:
    ImageRegistration( QString imagefile, QWidget* parent = 0, const char* name = 0);
    ~ImageRegistration();

    QPushButton* ok_btn;
    QPushButton* cancel_btn;
    QPushButton* proj_btn;
    Frame*       image_canvas;
    QLabel*      textLabel;
    QToolButton* zoomin_toolButton;
    QToolButton* zoomout_toolButton;
    QToolButton* zoomrect_toolButton; 
    Q3Table*      pts_table;
    QPushButton* goto_btn;
    QPushButton* del_btn;

    ProjectSetting* proj_set;

    Q3Canvas*  canvas;
    QString jpg_path;
    QString tab_path;

    CoorType CTyp;
protected:
    Q3GridLayout* ImageRegistrationLayout;

    Q3HBoxLayout* layout1;
    Q3GridLayout* layout2;
    Q3GridLayout* layout3;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;
    QSpacerItem* spacer6;
    QSpacerItem* spacer7;
    QSpacerItem* spacer8;
    QSpacerItem* spacer9;
    QSpacerItem* spacer10;
    QSpacerItem* spacer11;

public slots:
    virtual void languageChange();
    void         add_pt_ok_clicked();
    void         set_coor( CoorType& );

private slots:
    void ok_button_clicked();
    void cancel_button_clicked();
    void proj_button_clicked();
    void goto_button_clicked();
    void del_button_clicked();
    void zoomin_button_clicked();
    void zoomout_button_clicked();
    void zoomrect_button_clicked();
};

class QRadioButton;
class Q3ButtonGroup;
class ProjectSetting : public QDialog
{
    Q_OBJECT

public:
    ProjectSetting( QWidget* parent = 0, const char* name = 0 );
    ~ProjectSetting();

    Q3ButtonGroup* cood_buttonGroup;
    QRadioButton* ll_radioButton;
    QRadioButton* utm_radioButton;
    QPushButton*  ok_btn;

    CoorType CTyp;
protected:
    Q3GridLayout* ProjectSettingLayout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    Q3GridLayout* cood_buttonGroupLayout;

signals:

protected slots:
    virtual void languageChange();
    void         ok_button_clicked();
    void         cood_buttongrp_clicked( int );

};

#endif
/******************************************************************************************/
