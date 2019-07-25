/******************************************************************************************/
/**** FILE: printer.cpp                                                             *******/
/******************************************************************************************/

#include <qpainter.h>
#include <qprinter.h>
#include <q3canvas.h>
#include <q3vbox.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "wisim_gui.h"
#include "printer.h"

void myQCanvasView::drawContents( QPainter *p,int cx, int cy, int cw, int ch )  
{
     Q3CanvasView::drawContents( p, cx, cy, cw, ch);
}


myQCanvasView:: myQCanvasView(Q3Canvas *canvas,QWidget *parent, 
                  const char *name, Qt::WFlags f)
              : Q3CanvasView(canvas,parent,name,f)
{}

PrintRect::PrintRect()
          :QDialog()
{
     i = (Q3CanvasRectangle*)NULL;
     thiscanvas = NULL;
     w = 671.0; 
     h = 948.0;
     x = 0.0;
     y = 0.0;
     step = 1;
     size = 100.0;
     sizebox = (QSpinBox *) NULL;
     printer = (QPrinter *) NULL;
     thisview =(FigureEditor *) NULL;
}

PrintRect::~PrintRect()
{
    // if( i )  delete i;
    // delete sizebox;
}

void PrintRect::RepaintRect( )
{
     if(i)  delete i;
     i = new Q3CanvasRectangle( (int)x, (int)y, (int)w, (int)h, thiscanvas );
//     i->setPen( QPen(QColor( 2*8, 2*8, 2*8 ), 2 ) );
     i->setZ( 100 );
     i->show();
}

void PrintRect::RectMoveUP( )
{
     y -= step;
     RepaintRect();
}

void PrintRect::RectMoveDOWN( )
{
     y += step;
     RepaintRect();
}

void PrintRect::RectMoveLEFT( )
{
     x -= step;
     RepaintRect();
}

void PrintRect::RectMoveRIGHT( )
{
     x += step;
     RepaintRect();
}

void PrintRect::SetStep( int newstep )
{
     step = newstep;
}

void PrintRect::RectSizeChanged( int newsize )
{
     double oldsize = size;
     size = (double)newsize;
     x += w * ( oldsize - size ) / ( 2 * oldsize );
     y += h * ( oldsize - size ) / ( 2 * oldsize );
     w = w * size / oldsize;
     h = h * size / oldsize;
     RepaintRect();
}

void PrintRect::StartPrint(  )
{
      QPainter pp( printer );
      myQCanvasView canvasview( thisview->canvas() );
      QMatrix wm;
      double zoompercentw = 100.000000/size;
      double zoompercenth = 100.000000/size;
      wm.scale( zoompercentw, zoompercenth );
      wm.translate( -(int)x, -(int)y );
      canvasview.setWorldMatrix( wm );
      canvasview.drawContents( &pp, 0, 0, (int)(w*zoompercentw), (int)(h*zoompercenth) );
}

void PrintRect::DeleteRect()
{
      //printf("deleting rect......\n");
     if( i ) {
          delete i;
     }
printf("DeleteRect Finished\n");
}

void PrintRect::CalPosition()
{
//get the paper size from the former dialog and convert it to pixels on screen
     w = (double) paperwidth[ printer->pageSize() ] * 3.180;
     h = (double) paperheight[ printer->pageSize() ] * 3.180;

//let the original rect size to fit the canvas exactly
     if( printer->orientation() == 1 )  {   //if Landscape, swap width and height
          double temp = w;
          w = h;
          h = temp;
     }
     
     if (thisview->visibleWidth()/w < thisview->visibleHeight()/h ) {
          double size1 = 100*thisview->visibleWidth()/w;
          w = thisview->visibleWidth();
          h *= size1/100;
          size = size1;
     }

     else {
          double size1 = 100*thisview->visibleHeight()/h;
          h = thisview->visibleHeight();
          w *= size1/100;
          size = size1;
     }
     x = thisview->contentsX() + ( thisview->visibleWidth() - w ) / 2;
     y = thisview->contentsY() + ( thisview->visibleHeight() - h ) / 2;
     emit sizeChanged( (int)size );
}


void PrintRect::closeEvent(QCloseEvent * e )
{
     e->accept();
     DeleteRect();
}


void PrintDialog::closeEvent(QCloseEvent * e )
{
     e->accept();
     //printrect->DeleteRect();
}

void FigureEditor::regenPrintRect()
{  
     if(!printrect)  return;
     printrect->i = NULL;
     printrect->CalPosition();     
     printrect->RepaintRect();
}

void FigureEditor::print()
{
     if( !printer ) printer = new QPrinter;
     printer->setFullPage(TRUE);

//printer setup dialog
     if ( printer -> setup( this ) ) {
     printrect = new PrintRect();
     printrect->thisview = this;
     printrect->thiscanvas = this->canvas();
     printrect->printer = printer;
 //draw the initial rectangle of the area to be printed
     printrect -> CalPosition();     
     printrect -> RepaintRect();    
    
 //initiate a dialog to contain all the buttons  
     PrintDialog * pcp = new PrintDialog();
     pcp->printrect = printrect;
     pcp->setCaption( tr("Print Control Panel") );
     
//set the top level layout
     Q3VBoxLayout *toplayout = new Q3VBoxLayout( pcp, 20, 20, "toplayout" );

     QLabel title( tr("Select Print Area"), pcp );
     toplayout->addWidget( &title );

//setup the SpinBox for size control
     QSpinBox *sizespinbox = new QSpinBox( pcp );
     sizespinbox->setRange( 2, 600 );
     sizespinbox->setValue( (int)printrect->size );
     sizespinbox->setSuffix( "%" );
     QObject::connect( sizespinbox, SIGNAL( valueChanged( int ) ),
                       printrect, SLOT( RectSizeChanged( int ) ) );
     QObject::connect( printrect, SIGNAL( sizeChanged( int ) ),
                       sizespinbox,SLOT( setValue( int ) ) );
     QLabel sizetext( tr("Size"), pcp );
     printrect->sizebox = sizespinbox;     
//set layout of the size control area
     Q3HBoxLayout *sizelayout = new Q3HBoxLayout( toplayout );

     sizelayout->addWidget( &sizetext );
     sizelayout->addWidget( sizespinbox );

//set lower level layout     
     Q3HBoxLayout *modelayout = new Q3HBoxLayout( toplayout );
     Q3VBoxLayout *modelayoutl = new Q3VBoxLayout( modelayout );
     Q3GridLayout *modelayoutr = new Q3GridLayout( modelayout, 3, 3, -1, 0 );
   
     QLabel modetext(tr("Position"), pcp );
     QSpinBox *modestepbox = new QSpinBox( pcp );
     modestepbox->setRange( 1, 50 );
     modestepbox->setValue( 1 );
     modestepbox->setWrapping( TRUE );
     modestepbox->setSuffix( tr("pixel") );
     QObject::connect( modestepbox, SIGNAL( valueChanged( int ) ),
                       printrect, SLOT( SetStep( int ) ) );
     modelayoutl->addWidget( &modetext );
     
     Q3HBoxLayout *modestep = new Q3HBoxLayout( modelayoutl );
     
     QLabel modesteptext(tr("Step"), pcp );
     modestep->addWidget( &modesteptext );
     modestep->addWidget( modestepbox );

 //setup the rectangle-moving control button
      
     QPushButton up(    tr("UP"), pcp, 0 );
     QPushButton down(  tr("DOWN"), pcp, 0 );
     QPushButton left(  tr("LEFT"), pcp, 0 );
     QPushButton right( tr("RIGHT"), pcp, 0 );
     up.setFixedSize ( 50, 25 );
     down.setFixedSize ( 50, 25 );
     left.setFixedSize ( 50, 25 );
     right.setFixedSize ( 50, 25 );
     QObject::connect( &up, SIGNAL( clicked() ), 
                       printrect, SLOT( RectMoveUP( ) ) );
     QObject::connect( &down, SIGNAL( clicked() ), 
                       printrect, SLOT( RectMoveDOWN( ) ) );
     QObject::connect( &left, SIGNAL( clicked() ), 
                       printrect, SLOT( RectMoveLEFT( ) ) );
     QObject::connect( &right, SIGNAL( clicked() ), 
                       printrect, SLOT( RectMoveRIGHT( ) ) );
     modelayoutr->addWidget( &up, 0, 1 );
     modelayoutr->addWidget( &down, 2 ,1 );
     modelayoutr->addWidget( &left, 1, 0 );
     modelayoutr->addWidget( &right, 1, 2 );
     
//add two submit buttons to the panel
     Q3HBoxLayout *submitlayout = new Q3HBoxLayout( toplayout );
     QPushButton ok( tr("OK"), pcp, 0 );
     QObject::connect( &ok, SIGNAL( clicked() ),
                      pcp, SLOT( accept() ) );
     QPushButton cancel( tr("cancel"), pcp, 0 );
     QObject::connect( &cancel, SIGNAL( clicked() ),
                      pcp, SLOT( reject() ) );
     submitlayout->addWidget( &ok );
     submitlayout->addWidget( &cancel );
     ok.setFixedSize ( 100, 30 );
     cancel.setFixedSize ( 100, 30 );

     toplayout->activate();
     int result = pcp->exec();
     if( result == QDialog::Accepted ) {
          printrect->StartPrint();
          printrect->DeleteRect();
     }
     else printrect->DeleteRect();
     }
}

        


/******************************************************************************************/
