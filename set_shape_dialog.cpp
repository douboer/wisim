
/******************************************************************************************
**** PROGRAM: set_shape_dialog.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <q3iconview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qglobal.h>
#include <qpainter.h>

// for StyledButton
#include <qvariant.h>
#include <qcolordialog.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <q3dragobject.h>
#include <qstyle.h>

#include <Q3Button>

#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <Q3VBoxLayout>
#include <QDragEnterEvent>

#include "wisim.h"
#include "wisim_gui.h"
#include "set_shape_dialog.h"
#include "list.h"

/******************************************************************************************/
/**** SetGroupShapeDialog::SetGroupShapeDialog                                         ****/
/******************************************************************************************/
SetGroupShapeDialog::SetGroupShapeDialog(FigureEditor* editor_param, QWidget* parent,
                                         const char* name, bool modal, Qt::WFlags f )
  :QDialog(parent, name, modal, f) 
   // : QDialog( parent, 0, true)
{
    // set the dialog properties
    setName( "SetGroupShapeDialog Dialog" );
    setCaption( tr("Select Shape") );
    resize(200, 150);
    
    //init the bm_idx and the real bitmap pos.in the list.
    bm_idx = -1;
    iBeginPos = 2;   
    // get the data required    
    editor = editor_param;
    np = editor->get_np();
    iOptionsNum = CellClass::num_bm;
    opMapList = GCellClass::bm_list;
  
#if defined(QT_CHECK_NULL)
    Q_CHECK_PTR( opMapList );
      if ( opMapList == 0 )
          qWarning( "f: Null pointer not allowed" );
#endif

    oVBoxLayout = new Q3VBoxLayout(this, 11, 6, "setGroupShape");
 
    // -----------------------------------
    // add StyledButtons 
    // -----------------------------------
    Q3HBoxLayout *buttonsLayout1= new Q3HBoxLayout( 0, 0, 6 );

    // create the rectangle Bitmap and circular Bitmap
    QPixmap oDefaultMapOne1(GCellClass::size, GCellClass::size);
    oDefaultMapOne1.fill(Qt::color0);
    QPainter painter1( &oDefaultMapOne1);
    painter1.setPen( Qt::NoPen );
    painter1.setBrush( Qt::black);
    painter1.drawRect(0, 0, GCellClass::size, GCellClass::size);

    // Circular Bitmap
    QPixmap oDefaultMapTwo2(GCellClass::size, GCellClass::size);
    oDefaultMapTwo2.fill(paletteBackgroundColor ());
    QPainter painter2( &oDefaultMapTwo2);
    painter2.setPen( Qt::NoPen );
    painter2.setBrush( Qt::red);
    painter2.drawEllipse(0, 0, GCellClass::size, GCellClass::size);

    // create the self-defined buttons
    StyledButton* buttonPixmap1 = new StyledButton( this, "buttonPixmap1" );
    buttonPixmap1->setMinimumSize( QSize( GCellClass::size+8, GCellClass::size+8));
    buttonPixmap1->setMaximumSize( QSize( GCellClass::size+8, GCellClass::size+8));
    buttonPixmap1->setEditor( StyledButton::PixmapEditor );
    buttonPixmap1->setProperty( "pixmap", oDefaultMapOne1);
    buttonPixmap1->index  = 0;
    buttonsLayout1->addWidget( buttonPixmap1 );

    StyledButton* buttonPixmap2 = new StyledButton( this, "buttonPixmap2" );
    buttonPixmap2->setMinimumSize(QSize( GCellClass::size+8, GCellClass::size+8));
    buttonPixmap2->setMaximumSize(QSize( GCellClass::size+8, GCellClass::size+8));
    buttonPixmap2->setEditor( StyledButton::PixmapEditor );
    buttonPixmap2->setProperty( "pixmap", oDefaultMapTwo2);
    buttonPixmap2->index = 1;
    buttonsLayout1->addWidget( buttonPixmap2 );      
 
    connect( buttonPixmap1, SIGNAL( chooseIndex(int) ),
       this, SLOT( getChoice(int) ) );  
    connect( buttonPixmap2, SIGNAL( chooseIndex(int) ),
       this, SLOT( getChoice(int) ) );  

    StyledButton* buttonPixmap3;   

    for (int i = iBeginPos; i < iOptionsNum; ++i) {
      buttonPixmap3= new StyledButton( this, "buttonPixmap3" );
      buttonPixmap3->setMinimumSize( QSize(GCellClass::size+8, GCellClass::size+8));
      buttonPixmap3->setMaximumSize( QSize(GCellClass::size+8, GCellClass::size+8));
      buttonPixmap3->setEditor( StyledButton::PixmapEditor );
      buttonPixmap3->setProperty( "pixmap", QPixmap(*(opMapList[i])));
      buttonPixmap3->index = i;
      buttonsLayout1->addWidget( buttonPixmap3 ); 
      connect( buttonPixmap3, SIGNAL( chooseIndex(int) ),
               this, SLOT( getChoice(int) ) );  
    }

    oVBoxLayout->addLayout( buttonsLayout1);


    // add PushButtons
    Q3HBoxLayout *buttonsLayout = new Q3HBoxLayout( 0, 0, 6 );
    QSpacerItem *spacer = new QSpacerItem( 0, 0,
                            QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonsLayout->addItem( spacer );
    oYesButton = new QPushButton( tr("OK"), this );
    oYesButton->setDefault( TRUE );
    oYesButton->setEnabled(false);
    buttonsLayout->addWidget( oYesButton );

    oCancelButton = new QPushButton( tr("Cancel"), this );
    oCancelButton->setFocus();
    buttonsLayout->addWidget( oCancelButton );
    oVBoxLayout->addLayout( buttonsLayout);
    
    connect( oYesButton, SIGNAL( clicked() ), 
             this, SLOT( changeShape() ) );
    connect( oCancelButton, SIGNAL( clicked() ), 
             this, SLOT( cancel() ) );

    
    this->exec();
}
/******************************************************************************************/
/**** SetGroupShapeDialog::~SetGroupShapeDialog                                        ****/
/******************************************************************************************/
SetGroupShapeDialog::~SetGroupShapeDialog()
{

}
/******************************************************************************************/

/****************************************************/
/*  the slots functions                            **/
/****************************************************/


/*******************************************/
/* slot for styleButton's signal           */
/*******************************************/
void SetGroupShapeDialog::getChoice(int i) {
  bm_idx = i;
  oYesButton->setEnabled(true);
}


/********************************************/
/*  slot for cancle pushbutton signal       */
/********************************************/

void SetGroupShapeDialog::cancel() {
  delete this;
}

/*********************************************/
/* Ok Button response                        */
/* Input:                                    */
/*        bm_idx is set                      */
/*        selecct_cell_number & list         */
/* Method: call the set cell shape command   */
/*********************************************/
void SetGroupShapeDialog::changeShape() {

  char *chptr;
  // this->accept();

  if ( bm_idx >= 0) {
    chptr = np->line_buf;
    chptr += sprintf(chptr, "set_bitmap -bm_idx %d -cell_idx \'", bm_idx);
    for (int i=0; i<=editor->select_cell_list->getSize()-1; i++) {
      chptr += sprintf(chptr, "%d%c", (*editor->select_cell_list)[i], ((i==editor->select_cell_list->getSize()-1)?'\'':' '));
    }    
    np->process_command(np->line_buf);

    delete this;
  }
  
}

// -----------------------------------------------
// for StyledButton class
// -----------------------------------------------
StyledButton::StyledButton(QWidget* parent, const char* name)
  : Q3Button( parent, name ), pix( 0 ), spix( 0 ), edit( ColorEditor ), s( 0), mousePressed( FALSE )
{
    setMinimumSize( minimumSizeHint() );
    setAcceptDrops( TRUE );
    setFocusPolicy(Qt::StrongFocus);
    connect( this, SIGNAL(clicked()), SLOT(onEditor()));
}

StyledButton::StyledButton( const QBrush& b, QWidget* parent, const char* name, Qt::WFlags f )
    : Q3Button( parent, name, f ), spix( 0 ), s( 0 )
{
    col = b.color();
    pix = b.pixmap();
    setMinimumSize( minimumSizeHint() );
}

StyledButton::~StyledButton()
{
    if ( pix ) {
        delete pix;
        pix = 0;
    }
    if ( spix ) {
        delete spix;
        spix = 0;
    }
}

void StyledButton::setEditor( EditorType e )
{
    if ( edit == e )
        return;

    edit = e;
    update();
}

StyledButton::EditorType StyledButton::editor() const
{
    return edit;
}

void StyledButton::setColor( const QColor& c )
{
    col = c;
    update();
}

void StyledButton::setPixmap( const QPixmap & pm )
{
    if ( !pm.isNull() ) {
        delete pix;
        pix = new QPixmap( pm );
    } else {
        delete pix;
        pix = 0;
    }
    scalePixmap();
}

QColor StyledButton::color() const
{
    return col;
}

QPixmap* StyledButton::pixmap() const
{
    return pix;
}

bool StyledButton::scale() const
{
    return s;
}

void StyledButton::setScale( bool on )
{
    if ( s == on )
        return;

    s = on;
    scalePixmap();
}

QSize StyledButton::sizeHint() const
{
    return QSize( 50, 25 );
}

QSize StyledButton::minimumSizeHint() const
{
    return QSize( 50, 25 );
}

void StyledButton::scalePixmap()
{
    delete spix;

    if ( pix ) {
      spix = new QPixmap( 6*width()/8, 6*height()/8 );
      QImage img = pix->convertToImage();
      spix->convertFromImage( s? img.smoothScale( 6*width()/8, 6 *height()/8) : img );
    } else {
        spix = 0;
    }

    update();
}

void StyledButton::resizeEvent( QResizeEvent* e )
{
    scalePixmap();
    Q3Button::resizeEvent( e );
}

void StyledButton::drawButton( QPainter *paint )
{
    // porting to QT4
    //style().drawPrimitive(QStyle::PE_ButtonBevel, paint, rect(), colorGroup(), isDown() ? QStyle::State_Sunken : QStyle::State_None);
    drawButtonLabel(paint);

    /*
    if (hasFocus())
        style().drawPrimitive(QStyle::PE_FocusRect, paint,
                              rect(),
                              colorGroup(), QStyle::State_None);
     */
}

void StyledButton::drawButtonLabel( QPainter *paint )
{
    paint->setPen( Qt::NoPen );
 
    if(!isEnabled())
      paint->setBrush( QBrush( colorGroup().button() ) );
    else if ( edit == PixmapEditor  && spix ) {
      QBrush brush( Qt::black, *pix) ;
      paint->setBrush(brush);
      paint->setBrushOrigin( width()/2 - pix->width()/2 , height()/2 - pix->height()/2 );   

    } else {
      //qDebug(" else is in");
       paint->setBrush( QBrush( Qt::red ) ); 
    }

    paint->drawRect( width()/ 2 - pix->width()/2  , height()/2 - pix->height()/2 , pix->width(), pix->height());
}

void StyledButton::onEditor()
{
    qDebug("onEditor is called");
    switch (edit) {
    case ColorEditor: {
        qDebug("ColorEditor is called");
        QColor c = QColorDialog::getColor( palette().active().background(), this );
        if ( c.isValid() ) {
            setColor( c );
            emit changed();
        }
    } break;
    case PixmapEditor: {
      emit chooseIndex(index);

    } break;
    default:
        break;
    }
}

void StyledButton::mousePressEvent(QMouseEvent* e)
{
    Q3Button::mousePressEvent(e);
    mousePressed = TRUE;
    pressPos = e->pos();
}

void StyledButton::mouseMoveEvent(QMouseEvent* e)
{
    Q3Button::mouseMoveEvent( e );
#ifndef QT_NO_DRAGANDDROP
    if ( !mousePressed )
        return;
    if ( ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
        if ( edit == ColorEditor ) {
            Q3ColorDrag *drg = new Q3ColorDrag( col, this );
            QPixmap pix( 25, 25 );
            pix.fill( col );
            QPainter p( &pix );
            p.drawRect( 0, 0, pix.width(), pix.height() );
            p.end();
            drg->setPixmap( pix );
            mousePressed = FALSE;
            drg->dragCopy();
        }
        else if ( edit == PixmapEditor && pix && !pix->isNull() ) {
            QImage img = pix->convertToImage();
            Q3ImageDrag *drg = new Q3ImageDrag( img, this );
            if(spix)
                drg->setPixmap( *spix );
            mousePressed = FALSE;
            drg->dragCopy();
        }
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void StyledButton::dragEnterEvent( QDragEnterEvent *e )
{
    setFocus();
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) )
        e->accept();
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

void StyledButton::dragLeaveEvent( QDragLeaveEvent * )
{
    if ( hasFocus() )
        parentWidget()->setFocus();
}

void StyledButton::dragMoveEvent( QDragMoveEvent *e )
{
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) )
        e->accept();
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

void StyledButton::dropEvent( QDropEvent *e )
{
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) ) {
        QColor color;
        Q3ColorDrag::decode( e, color );
        setColor(color);
        emit changed();
        e->accept();
    }
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) ) {
        QImage img;
        Q3ImageDrag::decode( e, img );
        QPixmap pm;
        pm.convertFromImage(img);
        setPixmap(pm);
        emit changed();
        e->accept();
    } else {
        e->ignore();
    }
}

#endif // QT_NO_DRAGANDDROP
