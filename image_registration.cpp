/**************************************************************************************************
 FILE: image_registration.cpp  
**************************************************************************************************/
#include <iostream>

#include <qapplication.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qlineedit.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <q3filedialog.h>
#include <qfile.h>
#include <qtoolbutton.h>
#include <q3table.h>
#include <qpainter.h>
#include <qimage.h>
#include <qlocale.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <QPixmap>
#include <Q3Frame>
#include <QMouseEvent>
#include <fstream>
#include <iomanip>
#include <qregexp.h>

#include "cconst.h"
#include "icons.h"
#include "WiSim.h"
#include "WiSim_gui.h"
#include "image_registration.h"
#include "filechooser.h"

/**************************************************************************************************
   Class ImageItem and Functions.
**************************************************************************************************/
class ImageItem: public Q3CanvasRectangle
{
public:
    ImageItem( QImage img, Q3Canvas *canvas );
    void drawShape( QPainter & );
private:
    QImage image;
};

ImageItem::ImageItem( QImage img, Q3Canvas *canvas )
    : Q3CanvasRectangle( canvas )
{
    image = img;
    setSize( image.width(), image.height() );
}

void ImageItem::drawShape( QPainter& p )
{
    p.drawImage( int(x()), int(y()), image, 0, 0, -1, -1, Qt::OrderedAlphaDither );
}

CrossItem::CrossItem( QPixmap pmp, Q3Canvas* canvas ) : Q3CanvasItem( canvas )
{ pixmap = pmp;  }
void CrossItem::draw( QPainter& p )
{    
    QMatrix m( 1,0,0,1, 0, 0 );
    QBrush brush(Qt::red);
    p.setWorldMatrix( m );
    p.setBrush( brush );
    p.setPen( Qt::red );
    p.drawPixmap(int(x()), int(y()), pixmap, 0, 0, -1, -1); 
    p.drawText(int(x())+pixmap.width()+5, int(y())+pixmap.height(), text, -1); 
}
static CrossItem* cross_item;
/**************************************************************************************************
   Functions of class CanvasView.
**************************************************************************************************/
int AddPoint::pt_idx = 0;
CanvasView::CanvasView(
    Q3Canvas* c, Q3Frame* parent) :
    Q3CanvasView(c,parent)
{
    factor = 1;
    AddPoint::pt_idx = 0;
    MS = AddPt; 
    setUpdatesEnabled( false );
    viewport()->setMouseTracking ( true );
    cross_list.setAutoDelete ( true );
    cross_list.clear();
}

void CanvasView::zoomMap(double scaleX, double scaleY)
{
    QMatrix m = worldMatrix();
    m.scale( scaleX, scaleY );
    setWorldMatrix( m );

#if MAP_DEBUG
    std::cout << " zoomMap m11 = "  << m.m11() << std::endl;
    std::cout << " zoomMap m22 = "  << m.m22() << std::endl;
    std::cout << " zoomMap m12 = "  << m.m12() << std::endl;
    std::cout << " zoomMap m21 = "  << m.m21() << std::endl;
    std::cout << " zoomMap cvs_width = "  << width() << std::endl;
    std::cout << " zoomMap cvs_height = " << height() << std::endl;
#endif

    replotAll( m.m11() );
}

static Q3CanvasEllipse* ellipse[500];
static Q3CanvasText*    text[500];
void CanvasView::replotAll( double factor )
{
    uint rout;
    for ( rout=0; rout < cross_list.count() && cross_item; rout++ ) {
        double x = cross_list.at(rout)->z_px;
        double y = cross_list.at(rout)->z_py;
        cross_list.at(rout)->move( (int)(x*factor), (int)(y*factor));
        cross_list.at(rout)->show();
    }
}

CanvasView::~CanvasView()
{
    Q3CanvasItemList l = canvas()->allItems();
    Q3CanvasItemList::Iterator it = l.begin();
    for (it=l.begin(); it != l.end(); ++it) {
        if ( *it )
            delete *it;
    }
    if ( cross_list.count() != 0 )
        cross_list.clear();
}
/**************************************************************************************************
  Mouse Event
 **************************************************************************************************/
#if MAP_DEBUG
static const char *cross_pm[]={
"13 13 2 1", ". c None", "# c #ff0000",
"......#......", "......#......", "......#......",
"......#......", "......#......", ".....###.....",
"#############", ".....###.....", "......#......",
"......#......", "......#......", "......#......", "......#......"};
#endif
static Q3CanvasRectangle* select_rect;
void CanvasView::contentsMousePressEvent( QMouseEvent* e )
{
    mousepressed = true;

    QPoint pt = e->pos();
    px = pt.x();
    py = pt.y();
    zoom_px = ((double)pt.x() / factor);
    zoom_py = ((double)pt.y() / factor);

    if ( e->button() == Qt::LeftButton && MS == CanvasView::ZoomRect ) {
        std::cout << " mouse press e->button() == LeftButton && MS == CanvasView::ZoomRect " << std::endl;
        select_rect = new Q3CanvasRectangle( (int)floor(zoom_px+0.5), (int)floor(zoom_py+0.5), 10, 10,  canvas() );
        select_rect->setZ( 20 );
        select_rect->show();
    } 
#if 0
    else {
        if ( select_rect ) {
            delete select_rect;
            select_rect = (Q3CanvasRectangle *) NULL;
        }
    }
#endif
}

void CanvasView::contentsMouseMoveEvent( QMouseEvent* e )
{
    QPoint pt = e->pos();
    mrect_w = pt.x() - px;
    mrect_h = pt.y() - py;

    double zoom_mrect_w = ceil((double) mrect_w/factor);
    double zoom_mrect_h = ceil((double) mrect_h/factor);

#if MAP_DEBUG
    std::cout << "=======================" << std::endl;
    std::cout << " mouse move x = " << pt.x() << std::endl;
    std::cout << " mouse move y = " << pt.y() << std::endl;
    std::cout << " mouse move zoom_x = " << pt.x()/factor << std::endl;
    std::cout << " mouse move zoom_y = " << pt.y()/factor << std::endl;
    std::cout << "=======================\n" << std::endl;
#endif

    if ( mousepressed && MS == CanvasView::ZoomRect ) {
        select_rect->setSize( (int) zoom_mrect_w+1, (int) zoom_mrect_h+1 );
        select_rect->show();
        canvas()->update();
    }
}

void CanvasView::contentsMouseReleaseEvent( QMouseEvent* e )
{
    mousepressed = false;
    QPoint pt = e->pos();
    rx = pt.x();
    ry = pt.y();
    mrect_w = rx - px;
    mrect_h = ry - py;

    double xf = (double) cvs_width/mrect_w;
    double yf = (double) cvs_height/mrect_h;

    if ( e->button() == Qt::LeftButton && MS == CanvasView::ZoomRect ) 
    {
        std::cout << " mouse release e->button() == LeftButton && MS == CanvasView::ZoomRect " << std::endl;
        if ( xf <= yf ) { 
            factor *= xf; 
            zoomMap(xf, xf);
            cx = (int)(( (double)px + (double)mrect_w/2 )*xf);
            cy = (int)(( (double)py + (double)mrect_h/2 )*xf);
        } else {
            factor *= yf; 
            zoomMap(yf, yf);
            cx = (int)(( (double)px + (double)mrect_w/2 )*yf);
            cy = (int)(( (double)py + (double)mrect_h/2 )*yf);
        }
        center ( cx, cy );

        delete select_rect;
        select_rect = (Q3CanvasRectangle *) NULL;
        //canvas()->update();
    }

    std::cout << "=======================" << std::endl;
    std::cout << " cvs_width = " << cvs_width << std::endl;
    std::cout << " cvs_height = " << cvs_height << std::endl;
    std::cout << " mrect_w = " << mrect_w << std::endl;
    std::cout << " mrect_h = " << mrect_h << std::endl;
    std::cout << " xf = " << xf << std::endl;
    std::cout << " yf = " << yf << std::endl;
    std::cout << " factor = " << factor << std::endl;
#if !MAP_DEBUG
    // debug output
    std::cout << " press X = " << px << std::endl;
    std::cout << " press Y = " << py << std::endl;
    std::cout << " Release X = " << rx << std::endl;
    std::cout << " Release Y = " << ry << std::endl;
#endif
    std::cout << "=======================\n" << std::endl;

    if ( MS == CanvasView::AddPt && e->button() == Qt::LeftButton ) {
        // Draw cross sign
        // Create AddPoint object and initial some variant.
        add_pt = new AddPoint( this, "Add a Point");
        // initial
        AddPoint::pt_idx ++;
        add_pt->imgx   = (int)floor(zoom_px+0.5);
        add_pt->imgy   = (int)floor(zoom_py+0.5);
        add_pt->mapx   = 0.0;
        add_pt->mapy   = 0.0;

        QString pstr;
        pstr.sprintf("PT %d", AddPoint::pt_idx);
        add_pt->pt_idx_lineEdit->setText( pstr ); 
        pstr.sprintf("%d", (int)floor(zoom_px+0.5));
        add_pt->imgx_lineEdit->setText( pstr ); 
        pstr.sprintf("%d", (int)floor(zoom_py+0.5));
        add_pt->imgy_lineEdit->setText( pstr ); 
        add_pt->mapx_lineEdit->setText( "0.000000" ); 
        add_pt->mapy_lineEdit->setText( "0.000000" ); 

        Frame* obj1 = (Frame*) (this->parent());
        ImageRegistration* obj2 = (ImageRegistration*) (obj1->parent());
        connect( add_pt->ok_btn, SIGNAL(clicked()), obj2, SLOT(add_pt_ok_clicked()));

        if ( obj2->CTyp == UtmType ) {
            add_pt->mapx_lbl->setText( tr( "UTM X" ) );
            add_pt->mapy_lbl->setText( tr( "UTM Y" ) );
            add_pt->deg_lbl2->setText( tr( "m" ) );
            add_pt->deg_lbl1->setText( tr( "m" ) );
        }

        add_pt->show();
    }
    MS = CanvasView::AddPt;
    viewport()->setCursor( QCursor( Qt::CrossCursor ) );
}
/*************************************************************************************************/

/**************************************************************************************************
   Functions.
**************************************************************************************************/
Frame::Frame(Q3Canvas* c, QDialog* parent) :
    Q3Frame(parent),
    canvas(c)
{
    cvs_view = new CanvasView(canvas, this);
    cvs_view->resize(size());
}

void Frame::init()
{
    mapImage = 0;
    addDigitMap();
}

Frame::~Frame()
{
    delete mapImage;
    mapImage = 0;
}

void Frame::addDigitMap()
{
    mapImage = new QImage;

    if (!mapImage->load( digi_map_filename )) {
        // std::cout << "cannot open the file: " << digi_map_filename << std::endl;
        QMessageBox::warning( this, "Warning",
            tr(" Can not open the file %1. \n   Plesse check the name.!    ").arg( digi_map_filename ),
            QMessageBox::Warning,
            0
        );
        return;
    } else { }
    
    Q3CanvasPolygonalItem* poly_item = new ImageItem(QImage( *mapImage ), canvas);
    poly_item->setZ( 10 );
    poly_item->show();
}
/*************************************************************************************************/


/***************************************************************************************************
 FUNCTION: AddPoint::AddPoint( QWidget* parent, const char* name ) 
****************************************************************************************************/
AddPoint::AddPoint( QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    if ( !name )
    setName( "AddPoint" );
    imgx   = 0;
    imgy   = 0;
    mapx   = 0.0;
    mapy   = 0.0;

    AddPointLayout = new Q3GridLayout( this, 1, 1, 11, 6, "AddPointLayout"); 

    layout1 = new Q3GridLayout( 0, 1, 1, 8, 16, "layout1");
    pt_idx_lineEdit = new QLineEdit( this, "pt_idx_lineEdit" );
    layout1->addWidget( pt_idx_lineEdit, 0, 1 );
    imgx_lbl = new QLabel( this, "imgx_lbl" );
    layout1->addWidget( imgx_lbl, 3, 0 );
    imgy_lineEdit = new QLineEdit( this, "imgy_lineEdit" );
    layout1->addWidget( imgy_lineEdit, 4, 1 );
    mapx_lbl = new QLabel( this, "mapx_lbl" );
    layout1->addWidget( mapx_lbl, 1, 0 );
    pt_idx_lbl = new QLabel( this, "pt_idx_lbl" );
    layout1->addWidget( pt_idx_lbl, 0, 0 );
    imgx_lineEdit = new QLineEdit( this, "imgx_lineEdit" );
    layout1->addWidget( imgx_lineEdit, 3, 1 );
    imgy_lbl = new QLabel( this, "imgy_lbl" );
    layout1->addWidget( imgy_lbl, 4, 0 );
    deg_lbl2 = new QLabel( this, "deg_lbl2" );
    layout1->addWidget( deg_lbl2, 2, 2 );
    mapx_lineEdit = new QLineEdit( this, "mapx_lineEdit" );
    layout1->addWidget( mapx_lineEdit, 1, 1 );
    deg_lbl1 = new QLabel( this, "deg_lbl1" );
    layout1->addWidget( deg_lbl1, 1, 2 );
    mapy_lbl = new QLabel( this, "mapy_lbl" );
    layout1->addWidget( mapy_lbl, 2, 0 );
    mapy_lineEdit = new QLineEdit( this, "mapy_lineEdit" );
    layout1->addWidget( mapy_lineEdit, 2, 1 );
    AddPointLayout->addLayout( layout1, 0, 0 );

    spacer4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    AddPointLayout->addItem( spacer4, 1, 0 );

    layout2 = new Q3HBoxLayout( 0, 0, 6, "layout2"); 
    spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer1 );
    ok_btn = new QPushButton( this, "ok_btn" );
    layout2->addWidget( ok_btn );
    spacer2 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer2 );
    cancel_btn = new QPushButton( this, "cancel_btn" );
    layout2->addWidget( cancel_btn );
    spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer3 );
    AddPointLayout->addLayout( layout2, 2, 0 );

    languageChange();
    resize( QSize(261, 250).expandedTo(minimumSizeHint()) );

    // connect signels and slots
    //connect( ok_btn, SIGNAL(clicked()), this, SLOT(ok_button_clicked()));
    connect( cancel_btn, SIGNAL(clicked()), this, SLOT(cancel_button_clicked()));

    // tab order
    setTabOrder( pt_idx_lineEdit, mapx_lineEdit );
    setTabOrder( mapx_lineEdit, mapy_lineEdit );
    setTabOrder( mapy_lineEdit, imgx_lineEdit );
    setTabOrder( imgx_lineEdit, imgy_lineEdit );
    setTabOrder( imgy_lineEdit, ok_btn );
    setTabOrder( ok_btn, cancel_btn );
}


/***************************************************************************************************
 FUNCTION: AddPoint::~AddPoint()
****************************************************************************************************/
AddPoint::~AddPoint()
{
}


/***************************************************************************************************
 FUNCTION: AddPoint::languageChange()
****************************************************************************************************/
void AddPoint::languageChange()
{
    setCaption( tr( "Add Point" ) );
    imgx_lbl->setText( tr( "Pixel X" ) );
    imgy_lbl->setText( tr( "Pixel Y" ) );
    mapx_lbl->setText( tr( "Longitude" ) );
    mapy_lbl->setText( tr( "Latitude" ) );
    deg_lbl2->setText( tr( "degree" ) );
    deg_lbl1->setText( tr( "degree" ) );
    pt_idx_lbl->setText( tr( "Point Index" ) );
    ok_btn->setText( tr( "Ok" ) );
    cancel_btn->setText( tr( "Cancel" ) );
}

/***************************************************************************************************
 FUNCTION: AddPoint::ok_button_clicked()
 ss_item->move( obj1->px, obj1->py );
****************************************************************************************************/
bool AddPoint::ok_button_clicked()
{
    QString pstr;
    QLocale::setDefault(QLocale::C);

    bool mapx_flg, mapy_flg, imgx_flg, imgy_flg;

    mapx = mapx_lineEdit->text().toDouble(&mapx_flg);
    mapy = mapy_lineEdit->text().toDouble(&mapy_flg);
    imgx = imgx_lineEdit->text().toInt(&imgx_flg);
    imgy = imgy_lineEdit->text().toInt(&imgy_flg);

    if ( !mapx_flg || !mapy_flg ) {
        QMessageBox::warning( this, "Warning",
                              QString( tr("Map coordination is not invalid !")),
                              QMessageBox::Warning,
                              0
                            );

        return false;
    }
    if ( !imgx_flg || !imgy_flg ) {
        QMessageBox::warning( this, "Warning",
                              QString( tr("Image Pixel values is not invalid !")),
                              QMessageBox::Warning,
                              0
                            );
        return false;
    }

    CanvasView* obj1 = (CanvasView*) (this->parent());
    pstr.sprintf("PT %d", AddPoint::pt_idx);
#if MAP_DEBUG
    obj1->pm = QPixmap( cross_pm );
    cross_item = new CrossItem( obj1->pm, obj1->canvas() );
    cross_item->setZ( 30 );
    cross_item->text = pstr; 
    cross_item->z_px = obj1->zoom_px;
    cross_item->z_py = obj1->zoom_py;
    cross_item->move( obj1->px, obj1->py );
    cross_item->show();
    obj1->canvas()->update();
    obj1->cross_list.append( cross_item );
#else
    QBrush brush( Qt::red );
    int idx = AddPoint::pt_idx;

    ellipse[idx-1] = new Q3CanvasEllipse( 10, 10, obj1->canvas() );
    ellipse[idx-1]->setBrush( brush );
    ellipse[idx-1]->setSize( 2, 2 );
    ellipse[idx-1]->move( imgx, imgy );
    ellipse[idx-1]->setZ( 25 );
    ellipse[idx-1]->show();

    QFont ft( "Times", 20, QFont::Light);
    text[idx-1] = new Q3CanvasText( obj1->canvas() );
    text[idx-1]->move( imgx-7, imgy-13);
    text[idx-1]->setColor( Qt::red );
    text[idx-1]->setFont( ft );
    text[idx-1]->setText( "*" );
    text[idx-1]->setZ( 20 );
    text[idx-1]->show();
#endif

    return true;
}


/***************************************************************************************************
 FUNCTION: AddPoint::cancel_button_clicked()
****************************************************************************************************/
void AddPoint::cancel_button_clicked()
{
    AddPoint::pt_idx -= 1;
    delete this;
}

static const char *zoom_icon[]={
"20 20 3 1", ". c None", "# c #000000", "a c #848484",
"....................", "..............#.....", "..#########...#.....",
"..###.........#.....", "..####........#.....", "..#.###...#########.",
"..#..###......#.....", "..#...###.....#.....", "..#....###....#.....",
"..#.....###...#.....", "..#......###........", "..........###.......",
"...........###......", "............###.....", ".............###....",
"..............###...", "...............###..", "................#a..",
"....................", "...................."};
/***************************************************************************************************
 FUNCTION: ImageRegistration::ImageRegistration( QWidget* parent, const char* name ) 
****************************************************************************************************/
ImageRegistration::ImageRegistration( QString imagefile, QWidget* parent, const char* name)
    : QDialog( parent, name, true )
{
    if ( !name )
    setName( "ImageRegistration" );
    CTyp = LlType;

    ImageRegistrationLayout = new Q3GridLayout( this, 1, 1, 11, 16, "ImageRegistrationLayout"); 

    layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 
    spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer1 );
    ok_btn = new QPushButton( this, "ok_btn" );
    layout1->addWidget( ok_btn );
    spacer2 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer2 );
    cancel_btn = new QPushButton( this, "cancel_btn" );
    layout1->addWidget( cancel_btn );
    spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer3 );
    proj_btn = new QPushButton( this, "proj_btn" );
    layout1->addWidget( proj_btn );
    spacer11 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer11 );
    ImageRegistrationLayout->addLayout( layout1, 2, 0 );

    layout3 = new Q3GridLayout( 0, 1, 1, 0, 6, "layout3"); 
    spacer8 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout3->addItem( spacer8, 3, 2 );

    canvas = new Q3Canvas(400,400);
    image_canvas = new Frame( canvas, this );
    image_canvas->setFixedSize( 400, 200 );
    image_canvas->setFrameShape( Q3Frame::StyledPanel );
    image_canvas->setFrameShadow( Q3Frame::Raised );
    image_canvas->cvs_view->resize( 400, 200 );
    image_canvas->cvs_view->viewport()->setCursor( QCursor( Qt::CrossCursor ) );
    layout3->addMultiCellWidget( image_canvas, 1, 7, 0, 1 );
    spacer9 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout3->addItem( spacer9, 5, 2 );

    textLabel = new QLabel( this, "textLabel" );
    textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, textLabel->sizePolicy().hasHeightForWidth() ) );
    layout3->addWidget( textLabel, 0, 0 );
    spacer6 = new QSpacerItem( 243, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addMultiCell( spacer6, 0, 0, 1, 2 );

    zoomrect_toolButton = new QToolButton( this, "zoomrect_toolButton" );
    zoomrect_toolButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, zoomrect_toolButton->sizePolicy().hasHeightForWidth() ) );
    zoomrect_toolButton->setMinimumSize( QSize( 25, 0 ) );
    layout3->addWidget( zoomrect_toolButton, 6, 2 );
    //zoomrect_toolButton->setText( tr("Z") );
    zoomrect_toolButton->setIconSet( QIcon( zoom_icon ) );

    QIcon zoomin_iconset;
    zoomin_iconset.setPixmap(QPixmap(XpmIcon::zoomin_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    zoomin_iconset.setPixmap(QPixmap(XpmIcon::zoomin_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);
    zoomin_toolButton = new QToolButton( this, "zoomin_toolButton" );
    zoomin_toolButton->setIconSet( zoomin_iconset );
    zoomin_toolButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, zoomin_toolButton->sizePolicy().hasHeightForWidth() ) );
    zoomin_toolButton->setMinimumSize( QSize( 25, 0 ) );
    layout3->addWidget( zoomin_toolButton, 2, 2 );

    QIcon zoomout_iconset;
    zoomout_iconset.setPixmap(QPixmap(XpmIcon::zoomout_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    zoomout_iconset.setPixmap(QPixmap(XpmIcon::zoomout_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);
    zoomout_toolButton = new QToolButton( this, "zoomout_toolButton" );
    zoomout_toolButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, zoomout_toolButton->sizePolicy().hasHeightForWidth() ) );
    zoomout_toolButton->setIconSet( zoomout_iconset );
    zoomout_toolButton->setMinimumSize( QSize( 25, 0 ) );
    layout3->addWidget( zoomout_toolButton, 4, 2 );

    spacer7 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout3->addItem( spacer7, 1, 2 );

    spacer10 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout3->addItem( spacer10, 7, 2 );
    ImageRegistrationLayout->addLayout( layout3, 1, 0 );

    layout2 = new Q3GridLayout( 0, 1, 1, 0, 6, "layout2"); 

    pts_table = new Q3Table( this, "pts_table" );
    pts_table->setFixedSize( 400, 127 );
    pts_table->setReadOnly( false );
    pts_table->setPaletteBackgroundColor( this->paletteBackgroundColor());
    pts_table->setNumCols( pts_table->numCols() + 1 );
    pts_table->horizontalHeader()->setLabel( pts_table->numCols() - 1, qApp->translate("ImageRegistration", "Point Index" ) );
    pts_table->setNumCols( pts_table->numCols() + 1 );
    pts_table->horizontalHeader()->setLabel( pts_table->numCols() - 1, qApp->translate("ImageRegistration", "X-Coordinate" ) );
    pts_table->setNumCols( pts_table->numCols() + 1 );
    pts_table->horizontalHeader()->setLabel( pts_table->numCols() - 1, qApp->translate("ImageRegistration", "Y-Coordinate" ) );
    pts_table->setNumCols( pts_table->numCols() + 1 );
    pts_table->horizontalHeader()->setLabel( pts_table->numCols() - 1, qApp->translate("ImageRegistration", "X-Pixel" ) );
    pts_table->setNumCols( pts_table->numCols() + 1 );
    pts_table->horizontalHeader()->setLabel( pts_table->numCols() - 1, qApp->translate("ImageRegistration", "Y-Pixel" ) );
    pts_table->setHScrollBarMode( Q3ScrollView::AlwaysOff );
    pts_table->verticalHeader()->hide();
    pts_table->setLeftMargin( 0 );
    pts_table->verticalHeader()->setPaletteBackgroundColor(Qt::gray);
    pts_table->setNumCols( 5 );
    pts_table->setSorting  ( true );
    pts_table->setColumnWidth( 0, 40 );
    pts_table->setColumnWidth( 1, 110 );
    pts_table->setColumnWidth( 2, 110 );
    pts_table->setColumnWidth( 3, 68 );
    pts_table->setColumnWidth( 4, 68 );
    pts_table->setSelectionMode( Q3Table::SingleRow );
    layout2->addMultiCellWidget( pts_table, 0, 3, 0, 0 );

    spacer4 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed );
    layout2->addItem( spacer4, 0, 1 );

    goto_btn = new QPushButton( this, "goto_btn" );
    goto_btn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, goto_btn->sizePolicy().hasHeightForWidth() ) );
    goto_btn->setFixedWidth( 60 );
    layout2->addWidget( goto_btn, 1, 1 );

    del_btn = new QPushButton( this, "del_btn" );
    del_btn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, del_btn->sizePolicy().hasHeightForWidth() ) );
    del_btn->setFixedWidth( 60 );
    layout2->addWidget( del_btn, 2, 1 );

    spacer5 = new QSpacerItem( 16, 50, QSizePolicy::Minimum, QSizePolicy::Fixed );
    layout2->addItem( spacer5, 3, 1 );

    ImageRegistrationLayout->addLayout( layout2, 0, 0 );
    
    /*
     *  Create object and initial.                                             */
    jpg_path = imagefile;

    image_canvas->digi_map_filename = jpg_path;
    tab_path = jpg_path;

    tab_path.replace( ".jpg",  ".TAB", false );
    tab_path.replace( ".jpeg", ".TAB", false );

#if MAP_DEBUG
    std::cout << "JPEG file path : " << jpg_path << std::endl;
    std::cout << "TAB file path  : "  << tab_path << std::endl;
#endif
    image_canvas->init();

    /*
     * resize canvas to fit the size of adding image                           */
    QImage* img = image_canvas->mapImage;
    int img_width  = img->width();
    int img_height = img->height();
    int cvs_width  = image_canvas->contentsRect().width() - 20;
    int cvs_height = image_canvas->contentsRect().height() - 20;
    image_canvas->cvs_view->cvs_width = cvs_width;
    image_canvas->cvs_view->cvs_height = cvs_height;
    
    double xf = (double) cvs_width/img_width ;
    double yf = (double) cvs_height/img_height; 
    canvas->resize( img_width, img_height );
    if ( xf > 1 || yf > 1 ) {
        if ( xf > yf ) {
            image_canvas->cvs_view->factor *= xf; 
            image_canvas->cvs_view->zoomMap(xf, xf);
        } else {
            image_canvas->cvs_view->factor *= yf; 
            image_canvas->cvs_view->zoomMap(yf, yf);
        }
    } else {
        image_canvas->cvs_view->center( img_width/2, img_height/2 );
    }

    languageChange();
    resize( QSize(484, 401).expandedTo(minimumSizeHint()) );
    setFixedSize(450, 400);

    // connect signals and slots
    connect( ok_btn,              SIGNAL(clicked()), this, SLOT(ok_button_clicked()));
    connect( cancel_btn,          SIGNAL(clicked()), this, SLOT(cancel_button_clicked()));
    connect( proj_btn,            SIGNAL(clicked()), this, SLOT(proj_button_clicked()));
    connect( goto_btn,            SIGNAL(clicked()), this, SLOT(goto_button_clicked()));
    connect( del_btn,             SIGNAL(clicked()), this, SLOT(del_button_clicked()));
    connect( zoomin_toolButton,   SIGNAL(clicked()), this, SLOT(zoomin_button_clicked()));
    connect( zoomout_toolButton,  SIGNAL(clicked()), this, SLOT(zoomout_button_clicked()));
    connect( zoomrect_toolButton, SIGNAL(clicked()), this, SLOT(zoomrect_button_clicked()));

    exec();
}


/***************************************************************************************************
 FUNCTION: ImageRegistration::~ImageRegistration()
****************************************************************************************************/
ImageRegistration::~ImageRegistration()
{
}


/***************************************************************************************************
 FUNCTION: ImageRegistration::languageChange()
****************************************************************************************************/
void ImageRegistration::languageChange()
{
    setCaption( tr( "Image Registration" ) );

    ok_btn->setText( tr( "&Ok" ) );
    ok_btn->setDisabled( true );
    cancel_btn->setText( tr( "&Cancel" ) );
    proj_btn->setText( tr( "&Projection" ) );
    textLabel->setText( tr( "Click Image to Add Point" ) );
    goto_btn->setText( tr( "&Goto" ) );
    del_btn->setText( tr( "&Delete" ) );
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::ok_button_clicked()
****************************************************************************************************/
void ImageRegistration::ok_button_clicked()
{
    QString jpg_file_name = NULL;
    char *separ = CVECTOR(1);
    sprintf(separ, "%c", '/');
    char buff[100]; 
    int count = 0;
    sprintf( buff, "%s", jpg_path.latin1() );
    strtok( buff, separ );
    while ( strtok( NULL, separ ) ) count++ ;

#ifdef __linux__
    jpg_file_name = jpg_path.section( separ, count, count, QString::SectionSkipEmpty );
#else 
    jpg_file_name = jpg_path.section( separ, count, count, QString::SectionSkipEmpty );
#endif
    free ( separ );

    /*
     *  Write the registration's information to *.Tab.                       */ 
#if 0
     /-----------------------------------------------------\
     |   !table                                            |
     |   !version 300                                      |
     |   !charset WindowsSimpChinese                       |
     |                                                     |
     |   Definition Table                                  |
     |     File "zhanjiang.jpg"                            |
     |     Type "RASTER"                                   |
     |     (110.41148,21.21508) (2669,2993) Label "Pt 1",  |
     |     (110.4027,21.19692) (2353,3695) Label "Pt 2",   |
     |     CoordSys "UTM"( or "LL" )                       |
     |     Units "meter" ( or "degree" )                   |
     /-----------------------------------------------------/
#endif


   //Check whether there is a same name file exsiting.
    if ( QFile::exists( tab_path ) && QMessageBox::question(
                this,
                tr("Overwrite File?"),
                tr("A file called \"%1\" already exists.\n"
                    "Do you want to overwrite it?")
                .arg( tab_path ),
                tr("&Yes"), tr("&No"),
                QString::null, 0, 1 ) ) {  /* "No" button clicked, do nothing!!*/  }
    else {
        std::ofstream tab_file;
        tab_file.open( tab_path.local8Bit() );
        if ( !tab_file ) {
            std::cout << "can not open file : " << tab_path.toStdString() << std::endl;
            exit(1);
        }

        tab_file << "!Table\n" << "!Created by WiSim\n\n";
#if MAP_DEBUG
        // For mapinfo system.
        tab_file << "!table\n!version 300\n!charset WindowsSimpChinese\n\n";
#endif
        //tab_file << "Definition Table\n  File \"" <<  jpg_file_name.local8Bit().data()  << "\"\n  Type \"RASTER\"\n"; 
        tab_file << "Definition Table\n  File \"" <<  jpg_file_name.toStdString()  << "\"\n  Type \"RASTER\"\n"; 

        int rout = 0;
        Q3Table* tb = pts_table;
        int pt_idx = AddPoint::pt_idx;
        tab_file.setf( std::ios_base::fixed );
        for ( rout=0; rout<pt_idx; rout++ ) {
        tab_file << "  (" << std::setprecision(6) << tb->text(rout, 1).toDouble() << "," 
                 << std::setprecision(6) << tb->text(rout, 2).toDouble() << ") "
                 << "("   << tb->text(rout, 3).toInt() << "," << tb->text(rout, 4).toInt() << ") "
                 << "Label " << "\"" << tb->text(rout, 0).local8Bit().data() << "\",\n";
        }
        // For mapinfo system.
        //tab_file << "  CoordSys Earth Projection 1, 0\n" << "  Units \"degree\"";
        if ( CTyp == LlType ) tab_file << "  CoordSys \"LL\"\n"  << "  Units \"degree\"";
        else                  tab_file << "  CoordSys \"UTM\"\n" << "  Units \"meter\"";
        tab_file.close();

        delete this;
    }
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::cancel_button_clicked()
****************************************************************************************************/
void ImageRegistration::cancel_button_clicked()
{
    delete this;
}

void ImageRegistration::proj_button_clicked()
{
    proj_set = new ProjectSetting(this);
}

void ImageRegistration::set_coor( CoorType& type )
{
    CTyp = type;
}


/***************************************************************************************************
 FUNCTION: ImageRegistration::del_button_clicked()()
****************************************************************************************************/
void ImageRegistration::del_button_clicked()
{
    if (AddPoint::pt_idx > 0)
    {
        AddPoint::pt_idx --;

        int idx = AddPoint::pt_idx;
        pts_table->removeRow(idx);

        delete ellipse[idx];
        delete text[idx];

        image_canvas->cvs_view->canvas()->update();
    }
    else
    {
        QMessageBox::warning( this, "Warning",
                              QString( tr("No point exsiting !")),
                              QMessageBox::Warning,
                              0
                            );
    }
}


/***************************************************************************************************
 FUNCTION: ImageRegistration::goto_button_clicked()()
****************************************************************************************************/
void ImageRegistration::goto_button_clicked()
{
    //std::cout << " Current Row is " << pts_table->currentRow() << std::endl;

    int cx, cy;
    int row = pts_table->currentRow();
    cx = pts_table->text( row, 3 ).toInt();
    cy = pts_table->text( row, 4 ).toInt();

#if MAP_DEBUG
    std::cout << " goto cx " << cx << std::endl;
    std::cout << " goto cy " << cy << std::endl;
#endif

    double factor = image_canvas->cvs_view->factor;
    image_canvas->cvs_view->center( (int)(cx * factor), (int)(cy * factor) ) ;
    image_canvas->cvs_view->update();
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::zoomin_button_clicked()
****************************************************************************************************/
void ImageRegistration::zoomin_button_clicked()
{
    //image_canvas->cvs_view->MS = CanvasView::ZoomIn;
    image_canvas->cvs_view->MS = CanvasView::AddPt;
    image_canvas->cvs_view->zoomMap(1.5, 1.5);
    image_canvas->cvs_view->factor *= 1.5;

    image_canvas->setCursor( QCursor( Qt::CrossCursor ) );
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::zoomout_button_clicked()
****************************************************************************************************/
void ImageRegistration::zoomout_button_clicked()
{
    //image_canvas->cvs_view->MS = CanvasView::ZoomOut;
    image_canvas->cvs_view->MS = CanvasView::AddPt;

    QImage* img = image_canvas->mapImage;
    double factor = image_canvas->cvs_view->factor;
    int img_width  = (int) (img->width() * factor);
    int img_height = (int) (img->height()* factor);
    int cvs_width  = image_canvas->cvs_view->viewport()->width() - 20;
    int cvs_height = image_canvas->cvs_view->viewport()->height() - 20;
    double xf = (double) img_width/cvs_width ;
    double yf = (double) img_height/cvs_height; 

#if MAP_DEBUG
    std::cout << "=======================" << std::endl;
    std::cout << "xf = " << xf << std::endl;
    std::cout << "yf = " << yf << std::endl;
    std::cout << "img_width = " << img_width << std::endl;
    std::cout << "img_height = " << img_height << std::endl;
    std::cout << "cvs_width = " << cvs_width << std::endl;
    std::cout << "cvs_height = " << cvs_height << std::endl;
    std::cout << "=======================\n" << std::endl;
#endif

    if ( xf > 1.4 && yf > 1.4 ) {
        image_canvas->cvs_view->zoomMap(0.7, 0.7);
        image_canvas->cvs_view->factor *= 0.7;
    } else { }

    image_canvas->cvs_view->viewport()->setCursor( QCursor( Qt::CrossCursor ) );
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::zoomrect_button_clicked()
****************************************************************************************************/
void ImageRegistration::zoomrect_button_clicked()
{
    QPixmap pm( zoom_icon );
    image_canvas->cvs_view->MS = CanvasView::ZoomRect;
    image_canvas->cvs_view->viewport()->setCursor( QCursor( pm, 0, 0 ) );
}

/***************************************************************************************************
 FUNCTION: ImageRegistration::zoomout_button_clicked()
****************************************************************************************************/
void ImageRegistration::add_pt_ok_clicked()
{
    std::cout << "Call function ImageRegistration::add_pt_ok_clicked()\n";

    AddPoint* add_pt = image_canvas->cvs_view->add_pt;
    if ( !add_pt->ok_button_clicked() )
    {
        delete add_pt;
        AddPoint::pt_idx -= 1;

        return;
    }

    int idx = AddPoint::pt_idx;
    if ( idx >= 2 ) {
        ok_btn->setDisabled( false );
    }

    QString str;
    double mapx, mapy;
    int    imgx, imgy;
    mapx   = add_pt->mapx_lineEdit->text().toDouble();
    mapy   = add_pt->mapy_lineEdit->text().toDouble();
    imgx   = add_pt->imgx_lineEdit->text().toInt();
    imgy   = add_pt->imgy_lineEdit->text().toInt();

    pts_table->insertRows(idx-1);
    pts_table->setText( idx-1, 0, str.sprintf("Pt %d", idx ) );
    pts_table->setText( idx-1, 1, str.sprintf("%12.6f", mapx) );
    pts_table->setText( idx-1, 2, str.sprintf("%12.6f", mapy) );
    pts_table->setText( idx-1, 3, str.sprintf("%d", imgx) );
    pts_table->setText( idx-1, 4, str.sprintf("%d", imgy) );

    delete add_pt;
}


/***************************************************************************************************
 FUNCTION: ImageRegistration::zoomout_button_clicked()
****************************************************************************************************/
ProjectSetting::ProjectSetting( QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    if ( !name )
    setName( "ProjectSetting" );
    CTyp = LlType;
    
    ProjectSettingLayout = new Q3GridLayout( this, 1, 1, 11, 6, "ProjectSettingLayout"); 

    cood_buttonGroup = new Q3ButtonGroup( this, "cood_buttonGroup" );
    cood_buttonGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, cood_buttonGroup->sizePolicy().hasHeightForWidth() ) );
    cood_buttonGroup->setColumnLayout(0, Qt::Vertical );
    cood_buttonGroup->layout()->setSpacing( 15 );
    cood_buttonGroup->layout()->setMargin( 22 );
    cood_buttonGroupLayout = new Q3GridLayout( cood_buttonGroup->layout() );
    cood_buttonGroupLayout->setAlignment( Qt::AlignTop );

    ll_radioButton = new QRadioButton( cood_buttonGroup, "ll_radioButton" );
    cood_buttonGroupLayout->addWidget( ll_radioButton, 0, 0 );

    utm_radioButton = new QRadioButton( cood_buttonGroup, "utm_radioButton" );
    cood_buttonGroupLayout->addWidget( utm_radioButton, 1, 0 );

    ProjectSettingLayout->addMultiCellWidget( cood_buttonGroup, 0, 0, 0, 2 );
    ok_btn = new QPushButton( this, "ok_btn" );
    ok_btn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, ok_btn->sizePolicy().hasHeightForWidth() ) );
    ok_btn->setMaximumSize( QSize( 120, 32767 ) );
    ProjectSettingLayout->addWidget( ok_btn, 1, 1 );
    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ProjectSettingLayout->addItem( spacer1, 1, 0 );
    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ProjectSettingLayout->addItem( spacer2, 1, 2 );

    languageChange();

    resize( QSize(388, 162).expandedTo(minimumSizeHint()) );

    connect( ok_btn,           SIGNAL( clicked() ),    this, SLOT( ok_button_clicked() ) );
    connect( cood_buttonGroup, SIGNAL( clicked(int) ), this, SLOT( cood_buttongrp_clicked( int ) ) ); 

    exec();
}
ProjectSetting::~ProjectSetting()
{ }
void ProjectSetting::languageChange()
{
    setCaption( tr( "ProjectSetting" ) );
    cood_buttonGroup->setTitle( tr( "Choose Coordination System" ) );
    ll_radioButton->setText( tr( "Lontitude" ) + " / " + tr( "Latitude" ) );
    utm_radioButton->setText( tr( "UTM" ) );
    if (((ImageRegistration*)parent())->CTyp == LlType) 
        ll_radioButton->setChecked( true );
    else if (((ImageRegistration*)parent())->CTyp == UtmType)
        utm_radioButton->setChecked( true );
    ok_btn->setText( tr( "&OK" ) );
    ok_btn->setAccel( QKeySequence( tr( "Alt+O" ) ) );
}
void ProjectSetting::ok_button_clicked()
{ 
    delete this;
}
void ProjectSetting::cood_buttongrp_clicked( int idx )
{
    switch ( idx ) {
        case 0: 
            CTyp = LlType;
            break;
        case 1:
            CTyp = UtmType;
            break;
    }
    ((ImageRegistration*)parent())->set_coor( CTyp ); 
}

#undef MAP_DEBUG
/*************************************************************************************************/
