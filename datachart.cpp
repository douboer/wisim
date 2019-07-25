#include <qpainter.h>
#include <qwidget.h>
#include <q3mainwindow.h>
#include <qpixmap.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QPaintEvent>
#include <stdlib.h>
#include <math.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qdialog.h>
#include <qcolor.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <q3tabdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listview.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcolordialog.h>
#include <q3filedialog.h>
#include <qfile.h>
#include <q3textedit.h>
#include <q3vbox.h>
#include <qstyle.h>
#include <qwindowsstyle.h>
#include <qcursor.h>
#include <qaction.h>
#include <q3dockarea.h>
#include "wisim.h"
#include "datachart.h"

#define DEBUG_DATACHART 0

void xyGraph::init()
{
#if DEBUG_DATACHART
     printf("init()\n");

#endif
     max_x = 500;
     max_y = 500;
     title_height = 10;
     menu_height = 60;
     lmargin = 50;
     rmargin = 250;
     tmargin = 50;
     bmargin = 50;
     tmargin_all = tmargin + menu_height + title_height;
     size_x = max_x + lmargin + rmargin;
     size_y = max_y + bmargin + tmargin_all;

     setBackgroundColor( Qt::white );
     //setPalette( QPalette( white ) );
     resize( size_x, size_y );
     num_data_set = 0;     
     dminx_old = 0;
     dmaxx_old = 100;
     dminy_old = 0;
     dmaxy_old = 100;
     dminx = 0;
     dminy = 0;
     dmaxx = 100;
     dmaxy = 100;     
     dexpx = 0;
     dexpy = 0;
     
     zoomrate = 1.0;
     pie_set_num = 0;
     title = "MY GRAPH";
     title_color = QColor( Qt::magenta );
     title1 = "1";
     title2 = "2";
     title_type = 0;
     pie_color_type = 0; //use default color
     desc_x = "X";
     desc_y = "Y";
     pointsize = 4;
     ratex = (double) ( max_x/(dmaxx-dminx) );
     ratey = (double) ( max_y/(dmaxy-dminy) );
     xmousepress = 0;
     ymousepress = 0;
     xmouserelease = 0;
     ymouserelease = 0;
     mousepressed = false;
     charttype = POINTCHART;
     valuetype = TWODIM;
     reference_type = 0;
     showgrid = true;
     viewscroll = false;
     printer = NULL;
     intCoord = true;
     cursor_inregion = false;
     
     pixmap = new QPixmap( 1024, 768 );
     painter = new QPainter( pixmap );
     pixmap->fill();

}


void xyGraph::initMenu( CHARTTYPE charttypename )
{
    menubar = new QMenuBar( this );
    file = new Q3PopupMenu( this );
    help = new Q3PopupMenu( this );

    Q3PopupMenu* exportMenu = new Q3PopupMenu( this );
    exportMenu->insertItem( "BMP", this, SLOT(ExportBmp()) );
    exportMenu->insertItem( "JPEG", this, SLOT(ExportJpg()) );
    file->setCaption( "File" );
    file->insertItem( "Export Image as ", exportMenu );
    file->insertItem( "Print...", this, SLOT(Print()) );
    file->insertSeparator();
    file->insertItem( "Quit", this, SLOT(close()) );
    help->setCaption( "Help" );
    help->insertItem( "About DataChart", this, SLOT(About()) );

    if( charttypename == POINTCHART ) {
        options = new Q3PopupMenu( this );
        view = new Q3PopupMenu( this );

        options->setCaption( "Options" );
        pointcid = options->insertItem( "Point Chart", this, SLOT(PointChart()) );
        linecid  = options->insertItem( "Line  Chart", this, SLOT(LineChart()) );
        barcid   = options->insertItem( "Bar   Chart", this, SLOT(BarChart()) );
        piecid   = options->insertItem( "Pie   Chart", this, SLOT(PieChart()) );
        options->insertSeparator();
        options->insertItem("Settings...", this, SLOT(Settings()) );

        view->insertItem( "Zoom In", this, SLOT(zoomIn()) );
//     zoomInAction->addTo( view );
        view->insertItem( "Zoom Out", this, SLOT(zoomOut()) );
        scrollid = view->insertItem( "Scroll", this, SLOT(scroll()) );
        view->insertItem( "Restore", this, SLOT(restore()) );
        view->setItemChecked( scrollid, false );

        options->setItemEnabled( pointcid, false );
        options->setItemEnabled( linecid, false );
        options->setItemEnabled( barcid, false );
        options->setItemEnabled( piecid, false );
     
        menubar->insertItem( "&File", file, Qt::CTRL+Qt::Key_F );
        menubar->insertItem( "&View", view, Qt::CTRL+Qt::Key_V );
        menubar->insertItem( "&Options", options, Qt::CTRL+Qt::Key_O );
        menubar->insertItem( "&Help", help, Qt::CTRL+Qt::Key_H );

        menubar->setSeparator( QMenuBar::InWindowsStyle );

        setMouseTracking( true ); 
    }

    if( charttypename == PIECHART || charttypename == PIECHART2 ) {
        menubar->insertItem( "&File", file, Qt::CTRL+Qt::Key_F );
        menubar->insertItem( "&Help", help, Qt::CTRL+Qt::Key_H );
        menubar->setSeparator( QMenuBar::InWindowsStyle );
    }
}


xyGraph::xyGraph()
{
#if DEBUG_DATACHART
     printf("xyGraph() no parameters\n");

#endif
     init();
}


xyGraph::xyGraph(double x1, double x2, double y1, double y2, QString s, CHARTTYPE charttypename, QWidget * parent1 ):
QWidget(parent1)
{    
#if DEBUG_DATACHART
     printf("xyGraph()1\n");
     char msg[100];
     sprintf(msg,"xyGraph()1\n");
     PRMSG(stdout, msg);

#endif
     init();
     initMenu( charttypename );

     if( x1>=x2 || y1>=y2 ){
          QMessageBox::warning( this, "Error", "Incorrect coordinate value!\nUse default coordinate...");
          x2 = x1+10;
          y2 = y2+10;
     }
     if( s.length()>50 ) {
          QMessageBox::warning( this, "Warning", "Title too long!\nPress OK to continue...");
     }

     charttype = charttypename;

     options->setItemEnabled( pointcid, true );

     if( charttype != POINTCHART ) {
          QMessageBox::warning( this, "Warning",
                "Only a POINTCHART can set minx and miny, \nthese two values will be ignored here!\nPress OK to continue...");
     } 
     switch (charttype) {
     case POINTCHART : 
        options->setItemVisible( pointcid, false );
        options->setItemVisible( linecid, false );
        options->setItemVisible( barcid, false );
        options->setItemVisible( piecid, false );
        break;
     case LINECHART  : options->setItemChecked( linecid, true ); break;
     case BARCHART   : options->setItemChecked( barcid, true ); break;
     default         : options->setItemChecked( piecid, true );
     }
     
     dmaxx_old = x2;
     dmaxy_old = y2;

     intCoord = true;
          
     if( charttype == POINTCHART ) {
          dminx_old = x1;
          dminy_old = y1;
          dminx_ori = x1;
          dminy_ori = y1;
          dmaxx_ori = x2;
          dmaxy_ori = y2;
          dstepx = calStep( dminx_old, dmaxx_old, 1 );     
          dmaxx = nearestValue( dmaxx_old, dstepx, 1 );
          dminx = nearestValue( dminx_old, dstepx, -1 );
          dstepy = calStep( dminy_old, dmaxy_old, 2 );    
          dmaxy = nearestValue( dmaxy_old, dstepy, 1 );
          dminy = nearestValue( dminy_old, dstepy, -1 );
     }
     else {
          dmaxx = dmaxx_old;
          dmaxy = dmaxy_old;
     }
     
     if( intCoord ) {
          num_grid_x = (double)((dmaxx-dminx)/dstepx);
          num_grid_y = (double)((dmaxy-dminy)/dstepy);
          grid_size_x = (double)(max_x/num_grid_x);
          grid_size_y = (double)(max_y/num_grid_y);
     }
     else {
          num_grid_x = 20;
          num_grid_y = 20;
          grid_size_x = 25;
          grid_size_y = 25;
     }
     
#if DEBUG_DATACHART
     printf("----------- intiate information ------------\n");
     printf("dstepx: %f\tdstepy: %f\n", dstepx, dstepy );
     printf("dminx: %f\tdmaxx: %f\n", dminx, dmaxx );
     printf("dminy: %f\tdmaxy: %f\n", dminy, dmaxy );
     
     printf("num_grid_x: %f\t", num_grid_x );
     printf("grid_size_x: %f\n", grid_size_x );     
     printf("num_grid_y: %f\t", num_grid_y );
     printf("grid_size_y: %f\n", grid_size_y );

     printf("-----------------------------------------\n");
#endif

     ratex = (double) (max_x/(dmaxx-dminx));
     ratey = (double) (max_y/(dmaxy-dminy));
     title = s;
     setCaption( s );
}


xyGraph::xyGraph( double y, QString s, CHARTTYPE charttypename, QWidget * parent1 ):
QWidget(parent1)
{
#if DEBUG_DATACHART
     printf("xyGraph()2\n");

#endif
     init();
     initMenu( charttypename );

     charttype = charttypename;
     
     if( charttype == POINTCHART ) {
          QMessageBox::warning( this, "Error", "Please specify xmin, xmax, ymin, ymax .\nPress OK to abort..." );
          exit(0);
     }
     
     if( s.length()>50 ) {
          QMessageBox::warning( this, "Warning", "Title too long!\nPress OK to continue...");
     }
          
     options->setItemEnabled( barcid, true );
     options->setItemEnabled( piecid, true );
     options->setItemEnabled( linecid, true );
     if( charttype == LINECHART )
          options->setItemChecked( linecid, true );
     if( charttype == BARCHART )
          options->setItemChecked( barcid, true );
     if( charttype == PIECHART )
          options->setItemChecked( piecid, true );
//          dmaxx = x;
     dmaxy_old = y;

     intCoord = true;
              
     dstepy = calStep( dminy_old, dmaxy_old, 2 );  

     dmaxy = nearestValue( dmaxy_old, dstepy, 1 );
    
     if( intCoord ) {
          num_grid_y = (double)((dmaxy-dminy)/dstepy);
          grid_size_y = (double)(max_y/num_grid_y);
     }
     else {
          num_grid_x = 20;
          num_grid_y = 20;
          grid_size_x = 25;
          grid_size_y = 25;
     }
     
#if DEBUG_DATACHART
     printf("----------- intiate information ------------\n");
     printf("dstepx: %f\tdstepy: %f\n", dstepx, dstepy );
     printf("dminx: %f\tdmaxx: %f\n", dminx, dmaxx );
     printf("dminy: %f\tdmaxy: %f\n", dminy, dmaxy );
     
     printf("num_grid_x: %f\t", num_grid_x );
     printf("grid_size_x: %f\n", grid_size_x );     
     printf("num_grid_y: %f\t", num_grid_y );
     printf("grid_size_y: %f\n", grid_size_y );

     printf("-----------------------------------------\n");
#endif
     
     ratex = (double) (max_x/dmaxx);
     ratey = (double) (max_y/dmaxy);
     title =s;
     setCaption( s );
}


xyGraph::xyGraph( QString s, CHARTTYPE charttypename, QWidget * parent1 ):
QWidget(parent1)
{
#if DEBUG_DATACHART
     printf("xyGraph()3\n");
     char msg[100];
     sprintf(msg,"xyGraph()3\n");
     PRMSG(stdout, msg);

#endif
     init();
     initMenu( charttypename );

     max_x = 300;
     max_y = 300;
     title_height = 10;
     menu_height = 60;
     lmargin = 50;
     rmargin = 50;
     tmargin = 20;
     bmargin = 170;
     tmargin_all = tmargin + menu_height + title_height;
     size_x = max_x + lmargin + rmargin;
     size_y = max_y + bmargin + tmargin_all;

     resize( size_x, size_y );

     if( charttypename != PIECHART && charttypename!= PIECHART2 ) {
          QMessageBox::warning( this, "Error", "Only a PIECHART can be drawn without coordinate\n"
               "Press OK to abort..." );
          exit(0);
     }
     if( s.length()>50 )
          QMessageBox::warning( this, "Warning", "Title too long!\nPress OK to continue...");

     charttype = charttypename;

     title = s;
     setCaption( s );
}


xyGraph::~xyGraph()
{
#if DEBUG_DATACHART

     char msg[100];
     sprintf(msg, "~xyGraph()\n");
     PRMSG(stdout, msg);
#endif

     if ( painter )  delete painter;
     if ( pixmap  )  delete pixmap;
     if ( menubar ) delete menubar;
     if ( printer ) delete printer;    
     if ( setlist ) free(setlist); 
     if ( settings_setlist ) free(settings_setlist);
}


void xyGraph::close()
{
#if DEBUG_DATACHART

     char msg[100];
     sprintf(msg, "close()\n");
     PRMSG(stdout, msg);
#endif
     QWidget::close();
    // ~xyGraph();
}


/****************  public functions  *****************/


bool xyGraph::setCoord( double x1, double x2, double y1, double y2 )
{
     if( x1>=x2 || y1>=y2 )
          return false;

     dminx_old = x1;
     dminy_old = y1;     
     dmaxx_old = x2;
     dmaxy_old = y2;
     
     if( intCoord ) {     
          dstepx = calStep( dminx_old, dmaxx_old, 1 );      
          dstepy = calStep( dminy_old, dmaxy_old, 2 );       

          dmaxx = nearestValue( dmaxx_old, dstepx, 1 );
          dmaxy = nearestValue( dmaxy_old, dstepy, 1 );
          dminx = nearestValue( dminx_old, dstepx, -1 );
          dminy = nearestValue( dminy_old, dstepy, -1 );
     
          num_grid_x = (double)((dmaxx-dminx)/dstepx);
          num_grid_y = (double)((dmaxy-dminy)/dstepy);
          grid_size_x = (double)(max_x/num_grid_x);
          grid_size_y = (double)(max_y/num_grid_y);
     }
     else {
          dminx = x1;
          dminy = y1;
          dmaxx = x2;
          dmaxy = y2;
          num_grid_x = 10;
          num_grid_y = 10;
          grid_size_x = 50;
          grid_size_y = 50;
          
     }
     
#if DEBUG_DATACHART
     printf("----------- coordinate information ------------\n");
     printf("dstepx: %f\tdstepy: %f\n", dstepx, dstepy );
     printf("dminx: %f\tdmaxx: %f\n", dminx, dmaxx );
     printf("dminy: %f\tdmaxy: %f\n", dminy, dmaxy );
     
     printf("num_grid_x: %f\t", num_grid_x );
     printf("grid_size_x: %f\n", grid_size_x );     
     printf("num_grid_y: %f\t", num_grid_y );
     printf("grid_size_y: %f\n", grid_size_y );

     printf("-----------------------------------------\n");
#endif
     
     ratex = (double) (max_x/(dmaxx-dminx));
     ratey = (double) (max_y/(dmaxy-dminy));
     return true;
}


void xyGraph::setTitle( QString s, QColor color )
{
     title_type =0;
     title = s;
     title_color = color;
     setCaption( s );
}


void xyGraph::setTitle( QString s1, QString s2, QColor color )
{
    if( charttype == PIECHART2 ) {
        title_type = 1;
        title1 = s1;
        title2 = s2;
        title_color = color;
        setCaption( "Coverage Report" );
    }
    else 
        setTitle( s1+s2, color );
}


void xyGraph::setPointSize( int size )
{
    pointsize = size;
}


void xyGraph::addXY( double *x, double *y, int n, int lineSize, QColor Color )
{
#if DEBUG_DATACHART
     printf("addXY1()\n");

#endif

     if( charttype != POINTCHART ) {
          QMessageBox::warning( this, "Warning",
          "A BARCHART, PIECHART or LINECHART needs only one set of numbers.\n"
          "The first set will be converted as indexes!\nPress OK to continue...(Now exit)");
          exit(0);
     }    
     num_data_set++;
     if( num_data_set == 1 ) 
          setlist = (dc_dataset**)malloc( sizeof( dc_dataset *) );
     else  setlist = (dc_dataset**)realloc( setlist, num_data_set*sizeof( dc_dataset *) );
     
     setlist[num_data_set-1] = new dc_dataset();
     setlist[num_data_set-1]->id = num_data_set-1;
     setlist[num_data_set-1]->desc = new QString("");
     setlist[num_data_set-1]->scale_rate = 1.0;
     setlist[num_data_set-1]->offset_x = 0.0;
     setlist[num_data_set-1]->offset_y = 0.0;     
     setlist[num_data_set-1]->visi = true;
     setlist[num_data_set-1]->linesize = lineSize;
     setlist[num_data_set-1]->showpoints = false;
     setlist[num_data_set-1]->data_color = Color;
     if ( Color == Qt::white )
          setlist[num_data_set-1]->data_color = chooseColor( num_data_set-1 ); 
     setlist[num_data_set-1]->data_length = n;
     setlist[num_data_set-1]->data_x = x;
     setlist[num_data_set-1]->data_y = y;   
     
     setlist[num_data_set-1]->idx=(char **)malloc(5*n);

     int i;
     
     for( i=0; i< n; i++ ) {
          setlist[num_data_set-1]->idx[i]=(char *)malloc(5);      
          sprintf( setlist[ num_data_set-1 ]->idx[i], "%d", i );
     }       
}


void xyGraph::addXY( char **x, double *y, int n, QColor Color )
{
#if DEBUG_DATACHART
     printf("addXY2()\n");

#endif

     if( charttype == POINTCHART ) {
          QMessageBox::warning( this, "Error", 
          "A POINTCHART must have two sets of numbers.\nPress OK to abort...");
          exit(0);
     }   
     num_data_set++;

     if( charttype == PIECHART || charttype == PIECHART2 ) {
          bmargin = n*15+60;
         size_y = max_y + bmargin + tmargin_all;
         if( charttype == PIECHART2 && num_data_set>=2 ) {
            int w1, w2, w3, w4, w5, w6, w7, w8, w_total;
            w1 = 45;  //color
            w2 = (int)(6.5*getMaxStrLen(setlist[0]->idx, setlist[0]->data_length ))+5; //signal index
            w3 = 70; //area
            w4 = 80; //percentage
            w5 = 70;
            w6 = 80;
            w7 = 70;
            w8 = 80;
            w_total = w1+w2+w3+w4+w5+w6+w7+w8;
            int mmargin = 50;
            size_x = lmargin+rmargin+2*max_x+mmargin;
            if( w_total >= size_x-20 ) {
                lmargin += (w_total+20-size_x)/2;
                rmargin += (w_total+20-size_x)/2;
                size_x = lmargin+rmargin+max_x;
                resize( size_x, size_y );
            }
         }
         resize( size_x, size_y );
     }

     if( num_data_set == 1 ) 
          setlist = (dc_dataset**)malloc( sizeof( dc_dataset *) );
     else  setlist = (dc_dataset**)realloc( setlist, num_data_set*sizeof( dc_dataset *) );
     
     setlist[num_data_set-1] = new dc_dataset();
     setlist[num_data_set-1]->data_length = n;
     setlist[num_data_set-1]->data_color = Color;
     if ( Color == Qt::white )
          setlist[num_data_set-1]->data_color = chooseColor( num_data_set-1 ); 
     setlist[ num_data_set-1 ]->idx = x;
     for( int i=0; i<= num_data_set-1; i++ )
         if( y[i]<=0.0000001 && y[i]>=-0.000001 )
               y[i]=0.0;
     setlist[ num_data_set-1 ]->data_y =y;
}
     

void xyGraph::addXY( double *y, int n, QColor Color )
{
#if DEBUG_DATACHART
     printf("addXY3()\n");

#endif

     int i = 0;
     if( charttype == POINTCHART ) {
          QMessageBox::warning( this, "Error", 
          "A POINTCHART must have two sets of numbers.\nPress OK to abort...");
          exit(0);
     }
     checkBarPieData( y, n );   
     num_data_set++;
     if( num_data_set == 1 ) 
          setlist = (dc_dataset**)malloc( sizeof( dc_dataset *) );
     else  setlist = (dc_dataset**)realloc( setlist, num_data_set*sizeof( dc_dataset *) );
     
     setlist[num_data_set-1] = new dc_dataset();
     setlist[num_data_set-1]->id = num_data_set-1;
     setlist[num_data_set-1]->desc = new QString("");
     setlist[num_data_set-1]->scale_rate = 1.0;
     setlist[num_data_set-1]->offset_x = 0.0;
     setlist[num_data_set-1]->offset_y = 0.0;     
     setlist[num_data_set-1]->visi = true;
     setlist[num_data_set-1]->data_length = n;
     setlist[num_data_set-1]->data_color = Color;
     if ( Color == Qt::white )
          setlist[num_data_set-1]->data_color = chooseColor( num_data_set-1 ); 

     setlist[num_data_set-1]->idx=(char **)malloc(5*n);

     for( i=0; i< n; i++ ) {
          setlist[num_data_set-1]->idx[i]=(char *)malloc(5);      
          sprintf( setlist[ num_data_set-1 ]->idx[i], "%d", i );
     }
     setlist[ num_data_set-1 ]->data_y =y;
}


void xyGraph::removeXY( int dataset )
{
#if DEBUG_DATACHART
     printf("removeXY()\n");

#endif

     num_data_set--;
     for( int i=dataset; i<=num_data_set-1; i++ ) {
          setlist[i]->id = setlist[i+1]->id;
          setlist[i]->desc = setlist[i+1]->desc;
          setlist[i]->scale_rate = setlist[i+1]->scale_rate;
          setlist[i]->offset_x = setlist[i+1]->offset_x;
          setlist[i]->offset_y = setlist[i+1]->offset_y;
          setlist[i]->visi = setlist[i+1]->visi;
          setlist[i]->data_length = setlist[i+1]->data_length;
          setlist[i]->data_color = setlist[i+1]->data_color;
          setlist[i]->linesize = setlist[i+1]->linesize;
          setlist[i]->showpoints = setlist[i+1]->showpoints;
          setlist[i]->data_x = setlist[i+1]->data_x;
          setlist[i]->data_y = setlist[i+1]->data_y;
     }
     if( num_data_set>=1 )
          setlist = (dc_dataset**)realloc( setlist, num_data_set*sizeof( dc_dataset *) );
}


void xyGraph::repaintChart()
{
#if DEBUG_DATACHART
     printf("repaintChart()\n");

#endif
     int dataset;
     painter->end();
     painter->begin( pixmap );
     resetPainter();
     painter->scale( getRateXFrame(), getRateYFrame() );


                  //painter->translate( 0, 70 );

     //painter->eraseRect( 0, -(menu_height+title_height), size_x, size_y + (menu_height+title_height) );
     painter->eraseRect( 0, 0, size_x, size_y );
     //painter->scale( width() / size_x, height() / ( size_y + (menu_height+title_height) ) );

     if( charttype == POINTCHART ) {
          if( showgrid )
               drawGrid( );
          for( dataset=0; dataset<num_data_set;dataset++) {
               if( !setlist[dataset]->visi ) continue;
               if( setlist[dataset]->linesize>0 )
                    drawLine( dataset );
          }
     }
     /*erase those lines out of region*/
     if (charttype == POINTCHART ) {
          painter->eraseRect( 0, 0, size_x, tmargin_all );
          painter->eraseRect( 0, 0, lmargin, size_y );
          painter->eraseRect( size_x-rmargin, 0, size_x, size_y );
          painter->eraseRect( 0, size_y-bmargin, size_x, size_y );
     }
     drawTitle( title );
     
     if( charttype == LINECHART ) {
          for( dataset=0; dataset<num_data_set;dataset++) {
               if( !setlist[dataset]->visi ) continue;
               drawLineChart( dataset );
          }
          drawChartIndex( );
     }
     if( charttype == BARCHART ) {
          for( dataset=0; dataset<num_data_set;dataset++) {
               if( !setlist[dataset]->visi ) continue;
               drawBarChart( dataset );
          }
          for( dataset=0; dataset<num_data_set;dataset++) {
               if( !setlist[dataset]->visi ) continue;
               drawBarValue( dataset );              
          }
          drawChartIndex( );
     }
     if( charttype == PIECHART ) {
          drawPieChart( 1 );
     }
     if( charttype == PIECHART2 ) {
          drawPieChart( 2 );
     }
     if ( charttype == POINTCHART ) {
          drawCoordinate( );
          for( dataset=0; dataset<num_data_set;dataset++) {
               if( !setlist[dataset]->visi ) continue;
               if( setlist[dataset]->linesize==0 || setlist[dataset]->showpoints )
                    drawPoints( dataset );
          }
          drawChartIndex( );
     }
     painter->end();
     painter->begin( this );

     painter->drawPixmap( 0, 0, *pixmap );
//     painter->end();
}


void xyGraph::setShowgrid( bool flag )
{
    if( charttype == POINTCHART ) 
     showgrid = flag;
}


void xyGraph::setCoordDesc( QString x, QString y )
{
    if( charttype == POINTCHART ) {
        desc_x = x;
        desc_y = y;
    }
}


void xyGraph::setReference( int type )
{
    if( charttype == POINTCHART )
        reference_type = type;
}


void xyGraph::setPieDataColor( QColor *datacolor )
{
    if( charttype == PIECHART || charttype == PIECHART2 ) {
        pie_color_type = 1;
        for( int dataset=0; dataset<num_data_set; dataset++ ) {
            setlist[dataset]->pie_data_color = (QColor*)malloc( setlist[dataset]->data_length*sizeof(QColor) );
            for( int i=0; i< setlist[dataset]->data_length; i++ ) 
                setlist[dataset]->pie_data_color[i] = datacolor[i];
        }
    }
}


/****************  slots  *****************/

void xyGraph::PointChart()
{
#if DEBUG_DATACHART
     printf("PointChart()\n");

#endif
     if( charttype == BARCHART || charttype == PIECHART )
          num_data_set = 0;
     
     charttype = POINTCHART; 
     options->setItemChecked( pointcid, true );
     repaintChart( );
}

     
void xyGraph::LineChart()
{
#if DEBUG_DATACHART
     printf("LineChart()\n");

#endif

     charttype = LINECHART;
     options->setItemChecked( linecid, true ); 
     options->setItemChecked( barcid, false ); 
     options->setItemChecked( piecid, false ); 
     repaintChart( );
}


void xyGraph::BarChart()
{
#if DEBUG_DATACHART
     printf("BarChart()\n");

#endif

     charttype = BARCHART;
     options->setItemChecked( linecid, false ); 
     options->setItemChecked( barcid, true ); 
     options->setItemChecked( piecid, false ); 
     repaintChart( );
}


void xyGraph::PieChart()
{
#if DEBUG_DATACHART
     printf("PieChart()\n");

#endif
     
     if( num_data_set>=2 ) {
     printf("Dlg_Pie_Set\n");
          QDialog *Dlg_Pie_Set = new QDialog( this );
//          Dlg_Pie_Set->setBackgroundColor( white );
          Dlg_Pie_Set->resize( 150, 150 );
          Dlg_Pie_Set->setCaption("Choose Dataset");          

          QPushButton *Btn_OK = new QPushButton( "OK", Dlg_Pie_Set, "OK" );
//          Btn_OK->setPaletteBackgroundColor( white );
          Btn_OK->resize( 50, 20 );
          Btn_OK->move( 50, 100 );
          QObject::connect( Btn_OK, SIGNAL( clicked() ),
                         Dlg_Pie_Set, SLOT( accept() ) );

          QLabel *Lbl_Choice = new QLabel( "Choose Dataset: ", Dlg_Pie_Set );
//          Lbl_Choice->setPaletteBackgroundColor( white );
          Lbl_Choice->move( 35, 10 );

          QComboBox* Combo_Choice = new QComboBox( Dlg_Pie_Set );
          Combo_Choice->resize( 70, 25 );
          Combo_Choice->move( 35, 50 );
          int i;
          QString str_pie_set;
          for( i=1; i<=num_data_set; i++ ) {
               str_pie_set.setNum( i );
               Combo_Choice->insertItem( str_pie_set );
          }
           
          if( Dlg_Pie_Set->exec() == QDialog::Accepted  ) {
               pie_set_num = Combo_Choice->currentItem();

               if( Btn_OK )  delete( Btn_OK );
               if( Lbl_Choice )  delete( Lbl_Choice );
               if( Combo_Choice )  delete( Combo_Choice );
               if( Dlg_Pie_Set )  delete( Dlg_Pie_Set );
              
               charttype = PIECHART; 
               options->setItemChecked( linecid, false ); 
               options->setItemChecked( barcid, false ); 
               options->setItemChecked( piecid, true ); 
   
               repaintChart( );
          }
     }
}


void xyGraph::zoomIn()
{
    double x1 = (dmaxx-dminx)/4;
    double y1 = (dmaxy-dminy)/4;
    setCoord( dminx+x1, dmaxx-x1, dminy+y1, dmaxy-y1 );
    repaintChart();

}


void xyGraph::zoomOut()
{
    double x1 = (dmaxx-dminx)/4;
    double y1 = (dmaxy-dminy)/4;
    setCoord( dminx-x1, dmaxx+x1, dminy-y1, dmaxy+y1 );
    repaintChart();
}


void xyGraph::scroll()
{
    viewscroll = !viewscroll;
    view->setItemChecked( scrollid, viewscroll );
}


void xyGraph::restore()
{
    dminx_old = dminx_ori;
    dminy_old = dminy_ori;
    dmaxx_old = dmaxx_ori;
    dmaxy_old = dmaxy_ori;
    dminx = dminx_ori;
    dminy = dminy_ori;
    dmaxx = dmaxx_ori;
    dmaxy = dmaxy_ori;
    setCoord( dminx, dmaxx, dminy, dmaxy );
    repaintChart();
}


void xyGraph::Settings()
{
     settings_general = new dc_settings_general();
     settings_general->title = title;
     settings_general->title_color = title_color;
     settings_general->minx = dminx_old;
     settings_general->maxx = dmaxx_old;
     settings_general->miny = dminy_old;
     settings_general->maxy = dmaxy_old;

     if( num_data_set >=1 )
          settings_setlist = (dc_settings**)malloc( num_data_set*sizeof( dc_settings *) ); 
     int i;
     for( i=0; i<=num_data_set-1; i++ ) {
          settings_setlist[i] = new dc_settings();
          loadSettings( i );
     }
     Dlg_Settings = new Q3TabDialog( this, "Chart Settings", true );
     Dlg_Settings->setCaption("Settings");
     Dlg_Settings->resize( 500, 350 );
     Dlg_Settings->setOkButton("Ok");
     Dlg_Settings->setApplyButton("Apply");
     Dlg_Settings->setCancelButton("Cancel");
     
     QObject::connect( Dlg_Settings, SIGNAL( applyButtonPressed() ), this, SLOT( applySettings() ) );
     
     Page_General = new QWidget( Dlg_Settings );
     
     Page_Dataset = new QWidget( Dlg_Settings );
     
     Q3VBoxLayout* g_layout = new Q3VBoxLayout( Page_General, 20, 10 );

     Q3GroupBox* GB_setTitle = new Q3GroupBox( "Set Title", Page_General );
     GB_setTitle->setColumnLayout(0, Qt::Horizontal );
     GB_setTitle->layout()->setSpacing( 20 );
     GB_setTitle->layout()->setMargin( 20 );    
     Q3HBoxLayout* setTitleLayout = new Q3HBoxLayout( GB_setTitle->layout() );
     QLabel* LB_Title = new QLabel( " Title ", GB_setTitle );
     LE_Title = new QLineEdit( GB_setTitle );
     LE_Title->setText( title );
     Btn_Title_Color = new QPushButton( "Choose Color", GB_setTitle );     
     Btn_Title_Color->setPaletteBackgroundColor( title_color );
     setTitleLayout->addWidget( LB_Title );
     setTitleLayout->addWidget( LE_Title );
     setTitleLayout->addWidget( Btn_Title_Color );
     QObject::connect( Btn_Title_Color, SIGNAL(clicked()), this, SLOT(onBtnTitleColorClicked()) );
     g_layout->addWidget( GB_setTitle );
     
     Q3ButtonGroup* BG_showGrids = new Q3ButtonGroup( "Show Grids", Page_General );
     BG_showGrids->setColumnLayout(0, Qt::Horizontal );
     BG_showGrids->layout()->setSpacing( 20 );
     BG_showGrids->layout()->setMargin( 20 );    
     BG_showGrids->setRadioButtonExclusive( true );
     Btn_Grids_Yes = new QRadioButton( "Yes               ", BG_showGrids );
     Btn_Grids_Yes->setChecked( true ); 
     Btn_Grids_No = new QRadioButton( "No    ", BG_showGrids );
     Q3HBoxLayout* gridsLayout = new Q3HBoxLayout( BG_showGrids->layout() );
     gridsLayout->setAlignment( Qt::AlignJustify );
     gridsLayout->addWidget( Btn_Grids_Yes );
     gridsLayout->addWidget( Btn_Grids_No );
     g_layout->addWidget( BG_showGrids );
     
     Q3GroupBox* GB_setXY = new Q3GroupBox( "Set X-Y", Page_General );
     GB_setXY->setColumnLayout(0, Qt::Horizontal );
     GB_setXY->layout()->setSpacing( 10 );
     GB_setXY->layout()->setMargin( 5 );    
     Q3HBoxLayout* setXYLayout = new Q3HBoxLayout( GB_setXY->layout() );
     QLabel* LB_minX = new QLabel( " minX", GB_setXY );
     LE_minX = new QLineEdit( GB_setXY );
     QString str1;
     str1.setNum(dminx_old);
     LE_minX->setText( str1 );
     QLabel* LB_maxX = new QLabel( "  maxX", GB_setXY );
     LE_maxX = new QLineEdit( GB_setXY );
     str1.setNum(dmaxx_old);
     LE_maxX->setText( str1 );
     QLabel* LB_minY = new QLabel( "  minY", GB_setXY );
     LE_minY = new QLineEdit( GB_setXY );
     str1.setNum( dminy_old );
     LE_minY->setText( str1 );
     QLabel* LB_maxY = new QLabel( "  maxY", GB_setXY );
     LE_maxY = new QLineEdit( GB_setXY );
     str1.setNum( dmaxy_old );
     LE_maxY->setText( str1 );
     setXYLayout->addWidget( LB_minX );
     setXYLayout->addWidget( LE_minX );
     setXYLayout->addWidget( LB_maxX );
     setXYLayout->addWidget( LE_maxX );
     setXYLayout->addWidget( LB_minY );
     setXYLayout->addWidget( LE_minY );
     setXYLayout->addWidget( LB_maxY );
     setXYLayout->addWidget( LE_maxY );
     g_layout->addWidget( GB_setXY );
/*
     QButtonGroup* BG_coord = new QButtonGroup( "Intelligent Coordinate", Page_General );
     BG_coord->setColumnLayout(0, Qt::Horizontal );
     BG_coord->layout()->setSpacing( 20 );
     BG_coord->layout()->setMargin( 20 );    
     BG_coord->setRadioButtonExclusive( true );
     Btn_Coord_Yes = new QRadioButton( "Enable             ", BG_coord );
     Btn_Coord_Yes->setChecked( true ); 
     Btn_Coord_No = new QRadioButton( "Disable", BG_coord );
     QHBoxLayout* coordLayout = new QHBoxLayout( BG_coord->layout() );
     coordLayout->setAlignment( Qt::AlignJustify );
     coordLayout->addWidget( Btn_Coord_Yes );
     coordLayout->addWidget( Btn_Coord_No );
     g_layout->addWidget( BG_coord );
*/

     Q3HBoxLayout* toplayout = new Q3HBoxLayout( Page_Dataset, 20 );
     Q3VBoxLayout* leftlayout = new Q3VBoxLayout( toplayout, 5 );
     Q3VBoxLayout* rightlayout = new Q3VBoxLayout( toplayout, 5 );
     
     QLabel* Lbl_Dataset = new QLabel( "Select a dataset:", Page_Dataset );
     
     LV_Dataset = new Q3ListView( Page_Dataset );
     LV_Dataset->resize( 150, 100 );
     LV_Dataset->addColumn("Index", 40 );
     LV_Dataset->addColumn("Description", 100 );
     int dataset;
     QString str;
     for( dataset=0; dataset<=num_data_set-1; dataset++ ) {
          Q3ListViewItem* item = new Q3ListViewItem( LV_Dataset );
          str.setNum( dataset );
          item->setText( 0, str );
          item->setText( 1, *(settings_setlist[dataset]->desc) );
          if( dataset==0 )  {
               LV_Dataset->setCurrentItem( item );
               olditem = item;
               item->setSelected( true );
          }
     }
     LV_Dataset->setAllColumnsShowFocus( true );
     QObject::connect( LV_Dataset, SIGNAL( clicked( Q3ListViewItem* )), this, SLOT(onSelectionChanged( Q3ListViewItem* )) );

     leftlayout->addWidget( Lbl_Dataset );
     leftlayout->addWidget( LV_Dataset );
     
     Q3VBoxLayout* leftlayout2 = new Q3VBoxLayout( leftlayout, 5 );
     Btn_Add = new QPushButton( " Add from file... ", Page_Dataset );
     Btn_Remove = new QPushButton( " Remove dataset ", Page_Dataset );
     leftlayout2->addWidget( Btn_Add );
     leftlayout2->addWidget( Btn_Remove );
     QObject::connect( Btn_Add, SIGNAL( clicked() ), this, SLOT( addFromFile() ) );
     QObject::connect( Btn_Remove, SIGNAL( clicked() ), this, SLOT( removeDataset() ) );

     QLabel* Lbl_Desc = new QLabel( "Description", Page_Dataset );
     LE_Desc = new QLineEdit( Page_Dataset );
     Q3HBoxLayout* desclayout = new Q3HBoxLayout( rightlayout, 10 );
     desclayout->addWidget( Lbl_Desc );
     desclayout->addWidget( LE_Desc );
      
     QLabel* Lbl_Visi = new QLabel( "Visibility", Page_Dataset );
     Q3ButtonGroup* BG_Visi = new Q3ButtonGroup( 0 );    
     //QButtonGroup* BG_Visi = new QButtonGroup( 1, Qt::Horizontal, "Visibility", Page_Dataset );
     BG_Visi->setRadioButtonExclusive( true );
     Btn_Visi_Yes = new QRadioButton( "Yes", Page_Dataset );
     Btn_Visi_Yes->setChecked( true );
     Btn_Visi_No = new QRadioButton( "No", Page_Dataset );
     BG_Visi->insert( Btn_Visi_Yes );
     BG_Visi->insert( Btn_Visi_No );

     Q3HBoxLayout* visilayout = new Q3HBoxLayout( rightlayout, 20 );
     visilayout->addWidget( Lbl_Visi );
     visilayout->addWidget( Btn_Visi_Yes );
     visilayout->addWidget( Btn_Visi_No );
     
     QLabel* Lbl_Color = new QLabel( "Color", Page_Dataset );
     Btn_Color = new QPushButton( "Choose Color", Page_Dataset );
     if( num_data_set>=1 )
          Btn_Color->setPaletteBackgroundColor( settings_setlist[0]->color );

     QObject::connect( Btn_Color, SIGNAL(clicked()), this, SLOT(onBtnColorClicked()) );
  
     Q3HBoxLayout* colorlayout = new Q3HBoxLayout( rightlayout, 27 );
     colorlayout->addWidget( Lbl_Color );
     colorlayout->addWidget( Btn_Color );
     
     QLabel* Lbl_Linesize = new QLabel( "Line Size", Page_Dataset );
     SBox_Linesize = new QSpinBox( 0, 20, 1, Page_Dataset );
     QObject::connect( SBox_Linesize, SIGNAL(valueChanged(int)), this, SLOT(onLinesizeChanged(int)) );
     QLabel* Lbl_Space = new QLabel( "  ", Page_Dataset );
     CB_showPoints = new QCheckBox( "Show Points", Page_Dataset );

     Q3HBoxLayout* linesizelayout = new Q3HBoxLayout( rightlayout, 10 );
     linesizelayout->addWidget( Lbl_Linesize );
     linesizelayout->addWidget( SBox_Linesize );
     linesizelayout->addWidget( Lbl_Space );
     linesizelayout->addWidget( CB_showPoints );
     
     QLabel* Lbl_Scale = new QLabel( "Scale", Page_Dataset );
     SBox_Scale = new QSpinBox( 10, 1000, 1, Page_Dataset );
     SBox_Scale->setSuffix( "%" );
     SBox_Scale->setValue( 100 );
     Q3HBoxLayout* scalelayout = new Q3HBoxLayout( rightlayout, 16 );
     scalelayout->addWidget( Lbl_Scale );
     scalelayout->addWidget( SBox_Scale );
     
     QLabel* Lbl_Trans = new QLabel( "Translate", Page_Dataset );
     QLabel* Lbl_Trans_X = new QLabel( "X:", Page_Dataset );
     LE_Trans_X = new QLineEdit( Page_Dataset );
     QLabel* Lbl_Trans_Y = new QLabel( "Y:", Page_Dataset );
     LE_Trans_Y = new QLineEdit( Page_Dataset );    
     Q3HBoxLayout* translayout = new Q3HBoxLayout( rightlayout, 5 );
     translayout->addWidget( Lbl_Trans );
     translayout->addWidget( Lbl_Trans_X );
     translayout->addWidget( LE_Trans_X );
     translayout->addWidget( Lbl_Trans_Y );
     translayout->addWidget( LE_Trans_Y );    
 
     QLabel* Lbl_Data = new QLabel( "Data", Page_Dataset );
     Btn_Data = new QPushButton( "View Data", Page_Dataset );
     QObject::connect( Btn_Data, SIGNAL(clicked()), this, SLOT(viewData()) );
     Q3HBoxLayout* datalayout = new Q3HBoxLayout( rightlayout, 27 );
     datalayout->addWidget( Lbl_Data );
     datalayout->addWidget( Btn_Data );    

     if( num_data_set<=0 ) 
          disableWidgets( );

     Dlg_Settings->addTab( Page_General, "General" );     
     Dlg_Settings->addTab( Page_Dataset, "Dataset" );
     
     Dlg_Settings->show();
     if( num_data_set>=1 )
          repaintPageDataset( 0 );

}


void xyGraph::About()
{
#if DEBUG_DATACHART
     printf("About()\n");

#endif

     QMessageBox::about( this, "About DataChart", "DataChart \n   "
         "--cross platform graph tool\nVersion 1.2\nCopyRight Chengan");
}


void xyGraph::ExportBmp()
{
     Export( "BMP" );
}


void xyGraph::ExportJpg()
{
     Export( "JPEG" );
}


void xyGraph::Export( QString savetype )
{
     QString filename = "";

     if( savetype == QString("BMP") ) {
        filename = Q3FileDialog::getSaveFileName(
            NULL,
            "Bitmap files (*.bmp)",
            this,
            "save file dialog"
            "Choose a filename to save under" );

        if( filename.find( '.', 0 ) == -1 ) {
            filename += ".bmp";
        }
     }
     else if( savetype == QString("JPEG") ) {
        filename = Q3FileDialog::getSaveFileName(
            NULL,
            "JPEG files (*.jpg)",
            this,
            "save file dialog"
            "Choose a filename to save under" );

        if( filename.find( '.', 0 ) == -1 ) {
            filename += ".jpg";
        }
     }
     else {
        filename = Q3FileDialog::getSaveFileName(
            NULL,
            "All files (*.*)",
            this,
            "save file dialog"
            "Choose a filename to save under" );
     }

     if( filename.isEmpty() ) return;
     QPixmap pixmap1( *pixmap );
     pixmap1.resize( size_x, size_y );
     pixmap1.save( filename, savetype );
}


void xyGraph::Print()
{
#if DEBUG_DATACHART
     printf("Print()\n");

#endif
     if ( !printer )
      printer = new QPrinter;
     if ( printer->setup() ) {
//     QPainter *p_painter = new QPainter( printer );
     QPainter p_painter( printer );
     p_painter.drawPixmap( -30, 0, *pixmap); 
     }
}


void xyGraph::onSelectionChanged( Q3ListViewItem* newitem )
{
#if DEBUG_DATACHART
     printf("onSelectionChanged()\n");

#endif
     if( newitem == 0 ) return;
     if( olditem==newitem )  return;
     bool flag1 = false;
     bool flag2 = false;
     (LE_Trans_X->text()).toDouble( &flag1 );
     (LE_Trans_Y->text()).toDouble( &flag2 );
     olditem->setSelected( false );
     newitem->setSelected( true );
     if( !flag1 || !flag2 ) {
          QMessageBox::warning( Page_Dataset, "Error", "incorrect offset value!" );
          LV_Dataset->setCurrentItem( olditem );
          olditem->setSelected( true );
          olditem->repaint();
          newitem->setSelected( false );
          return;
     }
     
//save olditem settings     
     int oldid = (olditem->text(0)).toInt();
     saveSettings( oldid );
     
//current item change to newitem
     int newid = (newitem->text(0)).toInt();
          
//load newitem settings
     repaintPageDataset( newid );
     LV_Dataset->setCurrentItem( newitem );
     olditem = newitem;
}


void xyGraph::applySettings( )
{
#if DEBUG_DATACHART
     printf("applySettings()\n");

#endif

     double x1, x2, y1, y2;
     x1 = (LE_minX->text()).toDouble();
     x2 = (LE_maxX->text()).toDouble();
     y1 = (LE_minY->text()).toDouble();
     y2 = (LE_maxY->text()).toDouble();
     
     title = LE_Title->text();
     title_color = settings_general->title_color;
     showgrid = Btn_Grids_Yes->isChecked( );
     //intCoord = Btn_Coord_Yes->isChecked( );
     setCoord( x1, x2, y1, y2 );
     
     if( num_data_set<=0 ) {
          repaintChart( );
          return;
     }
     saveSettings( ((LV_Dataset->currentItem())->text(0)).toInt() );
     int oldid = (olditem->text(0)).toInt();
     olditem->setText( 1, *(settings_setlist[oldid]->desc) );
     for( int i=0; i<=num_data_set-1; i++ ) {
          setlist[i]->id = settings_setlist[i]->id;
          setlist[i]->desc = settings_setlist[i]->desc;
          setlist[i]->visi = settings_setlist[i]->visi;
          setlist[i]->linesize = settings_setlist[i]->linesize;
          setlist[i]->showpoints = settings_setlist[i]->showpoints;
          setlist[i]->data_color = settings_setlist[i]->color;
          setlist[i]->scale_rate = settings_setlist[i]->scale_rate;
          setlist[i]->offset_x = settings_setlist[i]->offset_x;
          setlist[i]->offset_y = settings_setlist[i]->offset_y;  
     }
     //showgrid = CB_Grid->isChecked();
     repaintChart( );
}     


void xyGraph::onBtnColorClicked( )
{
     int id = (olditem->text(0)).toInt();
     QColor color = QColorDialog::getColor( settings_setlist[id]->color, this, "color dialog" );
     settings_setlist[id]->color = color;
     Btn_Color->setPaletteBackgroundColor( color );
}


void xyGraph::onBtnTitleColorClicked( )
{
     settings_general->title_color = QColorDialog::getColor( settings_general->title_color, this, "color dialog" );
     Btn_Title_Color->setPaletteBackgroundColor( settings_general->title_color );
}


void xyGraph::onLinesizeChanged( int value )
{
     if( value==0 ) CB_showPoints->setEnabled( false );
     else CB_showPoints->setEnabled( true );
}


void xyGraph::addFromFile( )
{
     bool flag1 = false;
     bool flag2 = false;
     if( num_data_set>=1 ) {
          (LE_Trans_X->text()).toDouble( &flag1 );
          (LE_Trans_Y->text()).toDouble( &flag2 );
          if( !flag1 || !flag2 ) {
               QMessageBox::warning( Page_Dataset, "Error", 
                                   "Enter correct offset values of the current dataset\nbefore add a new one!" );
               return;
          }
     saveSettings( ((LV_Dataset->currentItem())->text(0)).toInt() );
     }
     QString filename = Q3FileDialog::getOpenFileName( );
     QFile* file = new QFile( filename );
     double* newx;
     double* newy;
     int i=0;
     if ( file->open( QIODevice::ReadOnly ) ) {
          QString str;
          int c;
          newx = (double *)malloc( sizeof(double) );
          newy = (double *)malloc( sizeof(double) );
          while( !file->atEnd() ) {
               str="";
               c=file->getch();
               for( ; c==' ' && !file->atEnd(); c=file->getch() )
                    ;
               for( ; c!=' ' && c!='\n' && !file->atEnd(); c=file->getch() )
                    str+=QChar(c);
               if( str=="" ) break;
               newx = (double *)realloc( newx, (i+1)*sizeof(double) );
               newx[i] = str.toDouble( );
               str="";
               for( ; c==' ' && !file->atEnd(); c=file->getch() )
                    ;
               for( ; c!=' ' && c!='\n' && !file->atEnd(); c=file->getch() )
                    str+=QChar(c);
               if( str =="" ) break;
               newy = (double *)realloc( newy, (i+1)*sizeof(double) );
               newy[i] = str.toDouble( );
               if( c ==' ' ) {
                    for( ; c!='\n' && !file->atEnd(); c=file->getch() )
                         ;
               }
               i++;
          }
          addXY( newx, newy, i );
          if( num_data_set<=1 ) 
               enableWidgets( );
          
          if( num_data_set >=2 )
               settings_setlist = (dc_settings**)realloc( settings_setlist, num_data_set*sizeof( dc_settings *) ); 
          else 
               settings_setlist = (dc_settings**)malloc( num_data_set*sizeof( dc_settings *) ); 
          settings_setlist[num_data_set-1] = new dc_settings( );
          loadSettings( num_data_set-1 );
          QString str1;
          Q3ListViewItem* item = new Q3ListViewItem( LV_Dataset );
          str1.setNum( num_data_set-1 );
          item->setText( 0, str1 );
          item->setText( 1, *(settings_setlist[num_data_set-1]->desc) );
          LV_Dataset->setCurrentItem( item );
          olditem = item;
          item->setSelected( true );
               repaintPageDataset( num_data_set-1 );
          repaintChart( );
     }
     else  
          QMessageBox::warning( this, "Error", "Cannot open file!" );
}


void xyGraph::removeDataset( )
{
     int i;
     int current;
     QString str;
     //QListViewItemIterator it;
     int dataset;
     switch( QMessageBox::warning( this, "Warning", "Are you sure that you REALLY"
                                "\nwant to remove this dataset?", "Yes", "No", 0, 1, -1 ) ) {
     case 0:  
          current = ((LV_Dataset->currentItem())->text(0)).toInt();
          removeXY( current );
          /*settings_setlist = (dc_settings**)realloc( settings_setlist, num_data_set*sizeof( dc_settings *) );*/
          for( i=0; i<=num_data_set-1; i++ ) 
               loadSettings( i );
          LV_Dataset->clear();
          for( dataset=0; dataset<=num_data_set-1; dataset++ ) {
               Q3ListViewItem* item = new Q3ListViewItem( LV_Dataset );
               str.setNum( dataset );
               item->setText( 0, str );
               item->setText( 1, *(settings_setlist[dataset]->desc) );
               if( dataset==0 )  {
                    LV_Dataset->setCurrentItem( item );
                    olditem = item;
                    item->setSelected( true );
               }
          }

          if( num_data_set<=0 ) 
               disableWidgets( );
      
          if( num_data_set>=1 )
               repaintPageDataset( 0 );
          repaintChart( );
      /*
          LV_Dataset->takeItem(LV_Dataset->currentItem());
          QListViewItemIterator it( LV_Dataset->currentItem() );
          for( ; it.current() ; it++ ) {
               str.setNum( ((it.current()->text(0)).toInt())-1 );
               it.current()->setText( 0, str );
          }
          if( current==num_data_set )
               current--;
          printf("current: %d\n", current);
          if ( current>=0 ) {
               str.setNum( current );
               (LV_Dataset->findItem(str,0))->setSelected( true );
           printf("************\n");
               repaintPageDataset( current );
           printf("************\n");
          }*/
          return; 
     case 1:  
          return;
     }      
}


void xyGraph::viewData( )
{
     int current = ((LV_Dataset->currentItem())->text(0)).toInt();
     int i;
     QString str;
     datatext="";
     for( i=0; i<=setlist[current]->data_length-1; i++ ) {
          str.setNum(setlist[current]->data_x[i]);
          datatext+=str;
          datatext+=" ";
          str.setNum(setlist[current]->data_y[i]);
          datatext+=str;
          datatext+="\n";
     }
     Q3VBox* textViewer = new Q3VBox( 0 );
     textViewer->resize( 200, 300 );   
     textViewer->setMargin( 5 );
     textEditer = new Q3TextEdit( datatext, QString::null, textViewer );
     textEditer->setReadOnly( true );  
     Q3HBox *buttons = new Q3HBox( textViewer );
     buttons->setMargin( 5 );
     QPushButton* Btn_saveData = new QPushButton( "&Save data", buttons );
     QPushButton* Btn_close = new QPushButton( "&Close", buttons );
     
     QObject::connect( Btn_saveData, SIGNAL(clicked()), this, SLOT(View_saveData()) );
     QObject::connect( Btn_close, SIGNAL(clicked()), textViewer, SLOT(close()) );
     
     textViewer->show( );
}


void xyGraph::View_saveData( )
{
     QString filename = Q3FileDialog::getSaveFileName( );
     QFile file( filename );
     if ( file.open( QIODevice::WriteOnly ) ) {
          Q3TextStream stream( &file );
            stream << datatext << "\n";
          file.close();
     }
}


void xyGraph::setPieSet( int set_num )
{
     pie_set_num = set_num;
}


/*****************  paint functions  *******************/

void xyGraph::resetPainter( )
{
#if DEBUG_DATACHART
     printf("resetPainter()\n");

#endif
     //painter->setWindow( 0, 0, max_x, max_y );
     //painter->setViewport( 0, (menu_height+title_height), SIZEX, SIZEY );
}


void xyGraph::paintEvent( QPaintEvent* )
{
#if DEBUG_DATACHART
     printf("paintEvent()\n");

#endif
     repaintChart( );
}


void xyGraph::drawTitle( QString title)
{
#if DEBUG_DATACHART
     printf("drawTitle()\n");

#endif
      painter->setPen( title_color );
     if( title_type == 1 ) {
         painter->setFont( QFont( "times", 12, QFont::Bold ) );
         painter->drawText( (int)(size_x/4-4.5*title.length()), menu_height, title1 );
         painter->drawText( (int)(3*size_x/4-4.5*title.length()), menu_height, title2 );
     }
     else {
         painter->setFont( QFont( "times", 15, QFont::Bold ) );
         painter->drawText( (int)(size_x/2-6.5*title.length()), menu_height, title );
     }
}


void xyGraph::drawCoordinate( )
{
#if DEBUG_DATACHART
     printf("drawCoordinate()\n");

#endif

     painter->setPen( Qt::blue );
     painter->drawLine( lmargin, size_y-bmargin, size_x-rmargin, size_y-bmargin );
     painter->drawLine( lmargin, size_y-bmargin, lmargin, tmargin_all );
//     painter->drawLine( size_x-rmargin, tmargin_all, size_x-rmargin, size_y-bmargin );
//     painter->drawLine( size_x-rmargin, tmargin_all, lmargin, tmargin_all );
     painter->setPen( Qt::black );
     painter->setFont( QFont( "times", 12 ) );
     double xcoord, ycoord;
     double dxcoord, dycoord;
     int i;
     char dxcoord_str[10], dycoord_str[10];
     painter->drawText( size_x-rmargin+5, size_y-bmargin+25,desc_x );
     painter->drawText( lmargin-20, tmargin_all-15, desc_y );
     painter->setFont( QFont( "times", 9 ) );
     
     if( charttype == POINTCHART ) {
          for ( i=0, xcoord=lmargin, dxcoord=dminx; xcoord<size_x-rmargin+1; 
               dxcoord+=dstepx, xcoord+=grid_size_x, i++ ) {
               dc_FloatToStr( dxcoord, dxcoord_str, dexpx );
               painter->setPen( Qt::black );
               if( grid_size_x>=36 || i%2==0 )
                    painter->drawText( (int)xcoord, size_y-bmargin+10, dxcoord_str );
               painter->setPen( Qt::blue );
               painter->drawLine( (int)xcoord, size_y-bmargin, (int)xcoord, size_y-bmargin-3 );
          }
     }
     for ( ycoord=size_y-bmargin, dycoord=dminy; ycoord>tmargin_all-1;
           dycoord+=dstepy, ycoord-=grid_size_y ) {
          dc_FloatToStr( dycoord, dycoord_str, dexpy );
          painter->setPen( Qt::black );
          painter->drawText( lmargin-(6*strlen(dycoord_str)), (int)ycoord, dycoord_str );
          painter->setPen( Qt::blue );
          painter->drawLine( lmargin, (int)ycoord, lmargin+3, (int)ycoord );
     }
}


void xyGraph::drawGrid( )
{
#if DEBUG_DATACHART
     printf("drawGrid()\n");

#endif

     painter->setPen( Qt::gray );
     double c, l;
     for( c=bmargin; c<size_y-tmargin_all-grid_size_y-1; c+=grid_size_y )
          painter->drawLine( lmargin, (int)(size_y-grid_size_y-c), size_x-rmargin, (int)(size_y-grid_size_y-c) );
     for( l=lmargin; l<size_x-rmargin-grid_size_x-1; l+=grid_size_x )
          painter->drawLine( (int)(grid_size_x+l), size_y-bmargin, (int)(grid_size_x+l), tmargin_all );
}  


void xyGraph::drawPoints( int dataset )
{
#if DEBUG_DATACHART
     printf("drawPoints()\n");

#endif
     resetPainter( );
     int i =0 ;
     while ( i< setlist[dataset]->data_length ) {
          int pointx = (int)(lmargin+((setlist[dataset]->scale_rate*setlist[dataset]->data_x[i]+setlist[dataset]->offset_x-dminx)*ratex));
          int pointy = (int)(size_y-bmargin-((setlist[dataset]->scale_rate*setlist[dataset]->data_y[i]+setlist[dataset]->offset_y-dminy)*ratey));
          if( IsPointInregion( pointx, pointy ) ) {
                    painter->setPen( QPen(setlist[dataset]->data_color,1) );
                    painter->setBrush( setlist[dataset]->data_color );
                    //painter->drawPie( pointx, pointy, 3, 3, 0, 5760 );
                    painter->drawEllipse( pointx, pointy, pointsize, pointsize );
          }
          i++;
     }
}


void xyGraph::drawPointsValue( int i, int dataset )
{
#if DEBUG_DATACHART
     printf("drawPointsValue()\n");

#endif

     int pointx = lmargin+(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_x[i]+setlist[dataset]->offset_x-dminx)*ratex);
     int pointy = size_y-tmargin_all-(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_y[i]+setlist[dataset]->offset_y-dminy)*ratey);
     if( !IsPointInregion( pointx, pointy+(menu_height+title_height) ) ) 
          return;
     painter->setPen( Qt::black );
     painter->setFont( QFont( "times", 8 ) );
     QString xstr="100";
     QString ystr="100";
     xstr.setNum( setlist[dataset]->scale_rate*setlist[dataset]->data_x[i]+setlist[dataset]->offset_x );
     ystr.setNum( setlist[dataset]->scale_rate*setlist[dataset]->data_y[i]+setlist[dataset]->offset_y );

     drawText( (int)(pointx*getRateXFrame())+5, (int)(getRateYFrame()*pointy)-5+(menu_height+title_height), "(" + xstr + "," + ystr + ")" );
}


void xyGraph::drawChartIndex( )
{
     int dataset;
     if( reference_type == 0 ) {
        painter->setPen( Qt::black );
        painter->setFont( QFont( "times", 8 ) );
        painter->drawText( size_x-rmargin+10, tmargin_all-10, "COLOR  ID  DESC");
        int i = 0;
        for( dataset=0; dataset<num_data_set;dataset++) {  
            if( !setlist[dataset]->visi )  continue;
            painter->drawRect( size_x-rmargin+15, tmargin_all+15*i, 20, 10 );
            painter->fillRect( size_x-rmargin+15, tmargin_all+15*i, 20, 10, setlist[dataset]->data_color );
            painter->setFont( QFont( "times", 8 ) );
            QString set_idx;
            set_idx.setNum( dataset );
            painter->drawText( size_x-rmargin+15+35, tmargin_all+10+15*i, set_idx );
            painter->drawText( size_x-rmargin+15+35+20, tmargin_all+10+15*i, *(setlist[dataset]->desc) );
            i++;
        }
     }
     else if( reference_type == 1 ) {

        painter->setPen( Qt::darkYellow );
        painter->setBrush( Qt::white );
        painter->drawRect( size_x-rmargin+40, tmargin_all, rmargin-80, max_x ); 

        painter->setPen( Qt::blue );
        painter->setFont( QFont( "times", 14 ) );
        painter->drawText( size_x-rmargin+60, tmargin_all+40, "Reference" );

        painter->fillRect( size_x-rmargin+60, tmargin_all+70, 50, 10, setlist[0]->data_color );
        painter->setPen( Qt::black );
        painter->setFont( QFont( "times", 10 ) );
        painter->drawText( size_x-rmargin+60, tmargin_all+100, "Propagation Model" );
                if( num_data_set >=2 ) {
        painter->fillRect( size_x-rmargin+60, tmargin_all+150, 50, 10, setlist[1]->data_color );
        painter->setPen( Qt::black );
        painter->setFont( QFont( "times", 10 ) );
        painter->drawText( size_x-rmargin+60, tmargin_all+180, "Road Test Data" );
                }
     }
     else return;
}

     
void xyGraph::drawLine( int dataset )
{
#if DEBUG_DATACHART
     printf("drawLine()\n");

#endif

     int i =0 ;
     int pointx1, pointx2, pointy1, pointy2;
     painter->setPen( QPen(setlist[dataset]->data_color, setlist[dataset]->linesize) );
     while ( i < setlist[dataset]->data_length-1 ) {
          pointx1 = lmargin+(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_x[i]+setlist[dataset]->offset_x-dminx)*ratex);
          pointy1 = size_y-bmargin-(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_y[i]+setlist[dataset]->offset_y-dminy)*ratey );
          pointx2 = lmargin+(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_x[i+1]+setlist[dataset]->offset_x-dminx)*ratex);
          pointy2 = size_y-bmargin-(int)((setlist[dataset]->scale_rate*setlist[dataset]->data_y[i+1]+setlist[dataset]->offset_y-dminy)*ratey);
 
//          if( IsPointInregion( pointx1, pointy1 ) && IsPointInregion( pointx2, pointy2 ) )
          painter->drawLine( pointx1, pointy1, pointx2, pointy2 );
          i++;
     }
}


void xyGraph::drawLineChart( int dataset )
{
     drawCoordinate( );
     
     int step = (size_x-lmargin-rmargin)/(setlist[dataset]->data_length+1);
     int n, lastx=0, lasty=0, thisx=0, thisy=0, starty=0;
     ratey = (double) ((size_y-tmargin-bmargin)/dmaxy);
     painter->setFont( QFont( "times", 10 ) );
     starty = size_y - bmargin;
     for( n=0; n<setlist[dataset]->data_length; n++ ) {
          lastx = thisx;
          lasty = thisy;
          thisx = lmargin + step *n +(int)(step/2);
          thisy = size_y-bmargin-(int)(setlist[dataset]->data_y[n]*ratey);
          QString ystr;
          ystr.setNum( setlist[dataset]->data_y[n] );
          painter->setPen( QPen(setlist[dataset]->data_color,3) );
          painter->drawEllipse( thisx, thisy, 2, 2 );
          painter->setPen( QPen(setlist[dataset]->data_color,1) );
          if( n!= 0 )
               painter->drawLine( lastx, lasty, thisx, thisy );
          painter->drawText( thisx, thisy, ystr );
          painter->setPen( Qt::black );
          painter->drawText( thisx, size_y-bmargin+10, setlist[dataset]->idx[n] );
     }
}


void xyGraph::drawBarChart( int dataset )
{
#if DEBUG_DATACHART
     printf("drawBarChart()\n");

#endif

     drawCoordinate( );
     int step = (size_x-lmargin-rmargin)/(setlist[dataset]->data_length+1);
     int n, thisx, starty, endy;
     ratey = (double) ((size_y-tmargin-bmargin)/dmaxy);
     painter->setPen( QPen(setlist[dataset]->data_color,(int)((step-8)/num_data_set)) );
     for( n=0; n<setlist[dataset]->data_length; n++ ) {
          thisx = lmargin + step *n + (int)(step/2) + (int)(dataset*((step-8)/num_data_set));
          starty = size_y - bmargin;
          endy = size_y-bmargin-(int)(setlist[dataset]->data_y[n]*ratey);
          painter->drawLine( thisx, starty, thisx, endy );
     }
//     drawBarValue( dataset );
}


void xyGraph::drawBarValue( int dataset )
{
#if DEBUG_DATACHART
     printf("drawBarValue()\n");

#endif

     int step = (size_x-lmargin-rmargin)/(setlist[dataset]->data_length+1);
     int n, thisx, starty, endy;
     ratey = (double) ((size_y-tmargin-bmargin)/dmaxy);
     painter->setFont( QFont( "times", 10 ) );
     for( n=0; n<setlist[dataset]->data_length; n++ ) {
          thisx = lmargin + step *n +(int)(step/2) ;
          starty = size_y - bmargin;
          endy = size_y-bmargin-(int)(setlist[dataset]->data_y[n]*ratey);
          QString ystr;
          ystr.setNum( setlist[dataset]->data_y[n] );
          painter->setPen( setlist[dataset]->data_color );
          painter->drawText( thisx + (int)(dataset*((step-8)/num_data_set)), endy, ystr); 
          painter->setPen( Qt::black );
          painter->drawText( thisx, size_y-bmargin+10, setlist[dataset]->idx[n] );
     }
}



void xyGraph::drawOnePie( int dataset, int startx, int starty )
{
     double total = 0;
     int i = 0;
     int arc = 0;
     int arcLen= 0;
     int valuex, valuey;
     double valueAngle = 0;
 
     for( i=0; i< setlist[dataset]->data_length; i++ )
          total += setlist[dataset]->data_y[i];

     painter->setPen( Qt::gray );
     
     for( i=0; i< setlist[dataset]->data_length; i++ ) {
          if( pie_color_type == 0 )
               painter->setBrush( chooseColorMore( i ) );
          else 
               painter->setBrush( setlist[dataset]->pie_data_color[i] );
          if( i == setlist[dataset]->data_length-1 )
               arcLen = (int)( 360*16 - arc );
          else arcLen = (int)((setlist[dataset]->data_y[i]/total)*16*360);

          painter->drawPie( startx, starty, max_x, max_y, arc, arcLen );
          painter->setPen( Qt::black );
          painter->setFont( QFont( "times", 10 ) );
          valueAngle = (double)(PI*(arc+arcLen/2)/16/180);
          valuex = lmargin + max_x/2 + (int)((1+i%2)*(max_x/6)*cos(valueAngle));
          valuey = tmargin_all + max_y/2 - (int)((1+i%2)*(max_y/6)*sin(valueAngle));

          arc += arcLen;
      
     }

     arc=0;
     arcLen=0;
     for( i=0; i< setlist[dataset]->data_length; i++ ) {
          if( i == setlist[dataset]->data_length-1 )
               arcLen = (int)( 360*16 - arc );
          else arcLen = (int)((setlist[dataset]->data_y[i]/total)*16*360);

          painter->setPen( Qt::black );
          painter->setFont( QFont( "times", 10 ) );
          valueAngle = (double)(PI*(arc+arcLen/2)/16/180);
          valuex = startx + max_x/2 + (int)((1+i%2)*(max_x/6)*cos(valueAngle));
          valuey = starty + max_y/2 - (int)((1+i%2)*(max_y/6)*sin(valueAngle));
          if( setlist[dataset]->data_y[i]/total >= 0.005 )
               painter->drawText( valuex-3*strlen(setlist[dataset]->idx[i]), valuey, setlist[dataset]->idx[i] );

          arc += arcLen;
     }
}


void xyGraph::drawPieChart( int num_set )
{
     int i = 0;
     if( num_set == 1 ) {
     int dataset = 0;
     double total = 0;
     for( i=0; i< setlist[dataset]->data_length; i++ )
          total += setlist[dataset]->data_y[i];

     drawOnePie( dataset, lmargin, tmargin_all );
     //draw Pie index
     int w1, w2, w3, w4, w_total, index_begin;
     w1 = 45;  //color
     w2 = (int)(6.5*getMaxStrLen(setlist[dataset]->idx, setlist[dataset]->data_length ))+5; //signal index
     w3 = 70; //area
     w4 = 80; //percentage
     w_total = w1+w2+w3+w4;
     if( w_total >= size_x-20 ) {
         lmargin += (w_total+20-size_x)/2;
         rmargin += (w_total+20-size_x)/2;
         size_x = lmargin+rmargin+max_x;
         resize( size_x, size_y );
     }

     index_begin = (int)((size_x-w_total)/2);

     painter->setPen( Qt::black );
     painter->setFont( QFont( "times", 8 ) );
     painter->drawLine( index_begin-3, size_y-bmargin+17, size_x-index_begin+2, size_y-bmargin+17 );
     painter->drawText( index_begin, size_y-bmargin+28, "COLOR" );
     painter->drawText( index_begin+w1, size_y-bmargin+28, "SIGNAL" );
     painter->drawText( index_begin+w1+w2, size_y-bmargin+28, "AREA" );
     painter->drawText( index_begin+w1+w2+w3, size_y-bmargin+28, "PERCENT(%)" );    
     for( i=0; i< setlist[dataset]->data_length; i++ ) {
          painter->drawLine( index_begin-3, size_y-bmargin+32+15*i, size_x-index_begin+3, size_y-bmargin+32+15*i );
          painter->drawRect( index_begin, size_y-bmargin+35+15*i, 35, 10 );
          if( pie_color_type == 0 )
                painter->fillRect( index_begin, size_y-bmargin+35+15*i, 35, 10, chooseColorMore(i) );
          else 
                painter->fillRect( index_begin, size_y-bmargin+35+15*i, 35, 10, setlist[dataset]->pie_data_color[i] );
          painter->setFont( QFont( "times", 10 ) );
          painter->drawText( index_begin+w1, size_y-bmargin+10+35+15*i, setlist[dataset]->idx[i] );
          QString area;
          setDouble( area, setlist[dataset]->data_y[i] );
          painter->drawText( index_begin+w1+w2, size_y-bmargin+10+35+15*i, area );
          QString percentage;
          setDouble( percentage, 100*(setlist[dataset]->data_y[i]/total) );
          painter->drawText( index_begin+w1+w2+w3, size_y-bmargin+10+35+15*i, percentage );
     }
     painter->drawLine( index_begin-3, size_y-bmargin+32+15*i, size_x-index_begin+3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin-3, size_y-bmargin+17, index_begin-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1-3, size_y-bmargin+17, index_begin+w1-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2-3, size_y-bmargin+17, index_begin+w1+w2-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2+w3-3, size_y-bmargin+17, index_begin+w1+w2+w3-3, size_y-bmargin+32+15*i );
     painter->drawLine( size_x-index_begin+3, size_y-bmargin+17, size_x-index_begin+3, size_y-bmargin+32+15*i );
     }

//charttype == PIECHART2
     if( num_set == 2 ) {
//      int dataset = 0;
     double total0 = 0;
     double total1 = 0;
     for( i=0; i< setlist[0]->data_length; i++ )
          total0 += setlist[0]->data_y[i];
     for( i=0; i< setlist[1]->data_length; i++ )
          total1 += setlist[1]->data_y[i];

     int w1, w2, w3, w4, w5, w6, w7, w8, w_total, index_begin;
     w1 = 45;  //color
     w2 = (int)(6.5*getMaxStrLen(setlist[0]->idx, setlist[0]->data_length ))+5; //signal index
     w3 = 70; //area
     w4 = 80; //percentage
     w5 = 70;
     w6 = 80;
     w7 = 70;
     w8 = 80;
     w_total = w1+w2+w3+w4+w5+w6+w7+w8;
     int mmargin = 50;

     drawOnePie( 0, lmargin, tmargin_all );

     drawOnePie( 1, lmargin+max_x+mmargin, tmargin_all );

     index_begin = (int)((size_x-w_total)/2);

     painter->setPen( Qt::black );
     painter->setFont( QFont( "times", 8 ) );
     painter->drawLine( index_begin-3, size_y-bmargin+17, size_x-index_begin+2, size_y-bmargin+17 );
     painter->drawText( index_begin, size_y-bmargin+28, "COLOR" );
     painter->drawText( index_begin+w1, size_y-bmargin+28, "SIGGAL" );
     painter->drawText( index_begin+w1+w2, size_y-bmargin+28, "AREA1(m2)" );
     painter->drawText( index_begin+w1+w2+w3, size_y-bmargin+28, "PERCENT1(%)" );    
     painter->drawText( index_begin+w1+w2+w3+w4, size_y-bmargin+28, "AREA2(m2)" );
     painter->drawText( index_begin+w1+w2+w3+w4+w5, size_y-bmargin+28, "PERCENT2(%)" );    
     drawDelta( index_begin+w1+w2+w3+w4+w5+w6, size_y-bmargin+28 );
     painter->drawText( index_begin+w1+w2+w3+w4+w5+w6+10, size_y-bmargin+28, "AREA(m2)" );
     drawDelta( index_begin+w1+w2+w3+w4+w5+w6+w7, size_y-bmargin+28 );
     painter->drawText( index_begin+w1+w2+w3+w4+w5+w6+w7+10, size_y-bmargin+28, "PERCENT(%)" );

     for( i=0; i< setlist[0]->data_length; i++ ) {
          painter->drawLine( index_begin-3, size_y-bmargin+32+15*i, size_x-index_begin+3, size_y-bmargin+32+15*i );
          painter->drawRect( index_begin, size_y-bmargin+35+15*i, 35, 10 );
          if( pie_color_type == 0 )
                painter->fillRect( index_begin, size_y-bmargin+35+15*i, 35, 10, chooseColorMore(i) );
          else 
                painter->fillRect( index_begin, size_y-bmargin+35+15*i, 35, 10, setlist[0]->pie_data_color[i] );
          painter->setFont( QFont( "times", 10 ) );
          painter->drawText( index_begin+w1, size_y-bmargin+10+35+15*i, setlist[0]->idx[i] );
          QString area1;
          setDouble( area1, setlist[0]->data_y[i] );
          painter->drawText( index_begin+w1+w2, size_y-bmargin+10+35+15*i, area1 );
          QString percentage1;
          setDouble( percentage1, 100*(setlist[0]->data_y[i]/total0) );
          painter->drawText( index_begin+w1+w2+w3, size_y-bmargin+10+35+15*i, percentage1 );
          QString area2;
                  setDouble( area2, setlist[1]->data_y[i] );
                  //printf( "setlist[1]->data_y[%d]: %f\n", i, setlist[1]->data_y[i] ); 
          painter->drawText( index_begin+w1+w2+w3+w4, size_y-bmargin+10+35+15*i, area2 );
          QString percentage2;
          setDouble( percentage2, 100*(setlist[1]->data_y[i]/total1) );
          painter->drawText( index_begin+w1+w2+w3+w4+w5, size_y-bmargin+10+35+15*i, percentage2 );
          QString delta_area;
          setDouble( delta_area, setlist[1]->data_y[i]-setlist[0]->data_y[i] );
          painter->drawText( index_begin+w1+w2+w3+w4+w5+w6, size_y-bmargin+10+35+15*i, delta_area );
          QString delta_percentage;
          setDouble( delta_percentage, 100*(setlist[1]->data_y[i]/total0-setlist[0]->data_y[i]/total1) );
          painter->drawText( index_begin+w1+w2+w3+w4+w5+w6+w7, size_y-bmargin+10+35+15*i, delta_percentage );
     }
     painter->drawLine( size_x-index_begin+3, size_y-bmargin+17, size_x-index_begin+3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin-3, size_y-bmargin+32+15*i, size_x-index_begin+3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin-3, size_y-bmargin+17, index_begin-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1-3, size_y-bmargin+17, index_begin+w1-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2-3, size_y-bmargin+17, index_begin+w1+w2-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2+w3-3, size_y-bmargin+17, index_begin+w1+w2+w3-3, size_y-bmargin+32+15*i );
      painter->drawLine( index_begin+w1+w2+w3+w4-3, size_y-bmargin+17, index_begin+w1+w2+w3+w4-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2+w3+w4+w5-3, size_y-bmargin+17, index_begin+w1+w2+w3+w4+w5-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2+w3+w4+w5+w6-3, size_y-bmargin+17, index_begin+w1+w2+w3+w4+w5+w6-3, size_y-bmargin+32+15*i );
     painter->drawLine( index_begin+w1+w2+w3+w4+w5+w6+w7-3, size_y-bmargin+17, index_begin+w1+w2+w3+w4+w5+w6+w7-3, size_y-bmargin+32+15*i );
     }

     else return;
}


void xyGraph::zoomGraph()
{
#if DEBUG_DATACHART
     printf("zoomGraph()\n");

#endif
     int dataset;
     xyGraph *zoomgraph = new xyGraph( valuestartx, valueendx, valuestarty, valueendy, title, POINTCHART );
     for( dataset=0; dataset<=num_data_set-1; dataset++)
          zoomgraph->addXY( setlist[dataset]->data_x, setlist[dataset]->data_y, 
                            setlist[dataset]->data_length, setlist[dataset]->linesize, setlist[dataset]->data_color );
     zoomgraph->setCoordDesc( desc_x, desc_y );
     zoomgraph->setShowgrid( showgrid );
     zoomgraph->setTitle( title, title_color );
     zoomgraph->setReference( reference_type );
     zoomgraph->repaintChart( );
     zoomgraph->show();

}


/***********  mouse event  **************/

void xyGraph::mousePressEvent( QMouseEvent * e )
{
#if DEBUG_DATACHART
     printf("mousePressEvent()\n");

#endif
    
     if( viewscroll ) {
         if( e->button() == Qt::RightButton ) {
                viewscroll = false;
                QCursor cursor1( Qt::ArrowCursor );
                setCursor( cursor1 );
                view->setItemChecked( scrollid, false );
                return;
         }
         if( e->button() == Qt::LeftButton ) {
             if( IsPointInregion( e->x(), e->y() ) ) {
                 scrollx_old = e->x();
                 scrolly_old = e->y();
                 QCursor cursor1( Qt::SizeAllCursor );
                 setCursor( cursor1 );
                 scrollstartx = e->x();
                 scrollstarty = e->y();
                 scrollbegin = true;
             }
         }
     }
     else {
         if( e->button() == Qt::LeftButton ) { 
             xmousepress = modifiedX( e->x() );
             ymousepress = modifiedY( e->y() );
             xmousecurrent = modifiedX( e->x() );
             ymousecurrent = modifiedY( e->y() );
             //printf("mouse left button pressed at ( %d, %d )\n", xmousepress, ymousepress );
             mousepressed = TRUE;
         }
     }
}


void xyGraph::mouseReleaseEvent( QMouseEvent * e )
{
#if DEBUG_DATACHART
     printf("mouseReleaseEvent()\n");

#endif
  
     mousepressed = false;
     if( charttype != POINTCHART )   return;
     if( viewscroll ) {
          scrollbegin = false;
     }
     else {
          if( e->button() == Qt::LeftButton ) {
                xmouserelease = modifiedX( e->x() );
                ymouserelease = modifiedY( e->y() );
                xmousecurrent = modifiedX( e->x() );
                ymousecurrent = modifiedY( e->y() );
                printf("mouse left button released at ( %d, %d )\n", xmouserelease, ymouserelease );
                regionw = fabs( (double)(xmousepress - xmouserelease) );
                regionh = fabs( (double)(ymousepress - ymouserelease) );
                if( regionw && regionh ){
                    zoomGraph();
                    mousepressed = FALSE;
                }
                repaintChart( );
            }
            if( e->button() == Qt::RightButton ) {
            printf("mouse right button released.\n");
            }
     }
}


void xyGraph::mouseMoveEvent( QMouseEvent * e )
{
     int i, j=0;
     int pointx, pointy;
     if( charttype != POINTCHART )   return;
     if( viewscroll && IsPointInregion( e->x(), e->y() ) && !cursor_inregion ) {
         cursor_inregion = true;
         QCursor cursor1( Qt::SizeAllCursor );
         setCursor( cursor1 );
     }
     if( viewscroll && !IsPointInregion( e->x(), e->y() ) && cursor_inregion ) {
         cursor_inregion = false;
         QCursor cursor1( Qt::ArrowCursor );
         setCursor( cursor1 );
     }
     if( viewscroll && scrollbegin ) {
          if( IsPointInregion( e->x(), e->y() ) ) {
               dminx_old -= (e->x()-scrollx_old)/ratex;
               dmaxx_old -= (e->x()-scrollx_old)/ratex;
               dminy_old += (e->y()-scrolly_old)/ratey;
               dmaxy_old += (e->y()-scrolly_old)/ratey;
               setCoord( dminx_old, dmaxx_old, dminy_old, dmaxy_old );
               repaintChart();
               scrollx_old = e->x();
               scrolly_old = e->y();
          }
     }
     else if( !mousepressed )
     {
          painter->end();
          painter->begin(this); 
          painter->drawPixmap(0,0,*pixmap);
          for( i=0; i<num_data_set; i++ ){
               if( setlist[i]->linesize>0 && setlist[i]->showpoints==false ) continue;
               j=0;
               while ( j< setlist[i]->data_length ) {
                    pointx = lmargin+(int)((setlist[i]->scale_rate*setlist[i]->data_x[j]+setlist[i]->offset_x-dminx)*ratex);
                    pointy = size_y-tmargin_all-(int)((setlist[i]->scale_rate*setlist[i]->data_y[j]+setlist[i]->offset_y-dminy)*ratey);
                    if( e->x()<=getRateXFrame()*pointx+3 && 
                        e->x()>=getRateXFrame()*pointx-3 && 
                        e->y()<=getRateYFrame()*pointy+3+getRateYFrame()*(menu_height+title_height) && 
                        e->y()>=getRateYFrame()*pointy-3+getRateYFrame()*(menu_height+title_height) )
                        drawPointsValue( j, i );
                    j++;
               }
          }
     }
     else {
          painter->end();
          painter->begin(this); 
          painter->drawPixmap( 0, 0, *pixmap );  
  
          xmousecurrent = modifiedX( e->x() );
          ymousecurrent = modifiedY( e->y() );
          regionw = fabs( (double)(xmousepress - xmousecurrent));
          regionh = fabs( (double)(ymousepress - ymousecurrent));
          regionstartx = ( xmousepress<xmousecurrent?xmousepress:xmousecurrent );
          regionstarty = ( ymousepress<ymousecurrent?ymousepress:ymousecurrent );
     
          valuestartx = (regionstartx-lmargin)/ratex + dminx;
          valueendy = (size_y-bmargin-regionstarty)/ratey + dminy;
          valueendx = valuestartx+regionw/ratex;
          valuestarty = valueendy-regionh/ratey;
     
          regionrect = new QRect( regionstartx, regionstarty, (int)regionw, (int)regionh );

          painter->setPen( Qt::green );
          painter->drawRect( *regionrect );
     }
}

/*************  settings  *****************/

void xyGraph::loadSettings( int i )
{
     settings_setlist[i]->id = setlist[i]->id;
     settings_setlist[i]->desc = setlist[i]->desc;
     settings_setlist[i]->visi = setlist[i]->visi;
     settings_setlist[i]->linesize = setlist[i]->linesize;
     settings_setlist[i]->showpoints = setlist[i]->showpoints;
     settings_setlist[i]->color = setlist[i]->data_color;
     settings_setlist[i]->scale_rate = setlist[i]->scale_rate;
     settings_setlist[i]->offset_x = setlist[i]->offset_x;
     settings_setlist[i]->offset_y = setlist[i]->offset_y; 
}


void xyGraph::saveSettings( int i )
{
     *(settings_setlist[i]->desc) = LE_Desc->text();
     settings_setlist[i]->visi = (Btn_Visi_Yes->isChecked()?true:false);
//     settings_setlist[i]->color = setlist[i]->data_color;
     settings_setlist[i]->linesize = SBox_Linesize->value();
     settings_setlist[i]->showpoints = CB_showPoints->isChecked( );
     settings_setlist[i]->scale_rate = (double)(SBox_Scale->value()/100.0);
     settings_setlist[i]->offset_x = (LE_Trans_X->text()).toDouble();
     settings_setlist[i]->offset_y = (LE_Trans_Y->text()).toDouble();
}


void xyGraph::repaintPageDataset( int dataset )
{
     QString str;
     int oldid = (olditem->text(0)).toInt();
     olditem->setText( 1, *(settings_setlist[oldid]->desc) );
     LE_Desc->setText( *(settings_setlist[dataset]->desc) );
     Btn_Visi_Yes->setChecked( settings_setlist[dataset]->visi );
     Btn_Visi_No->setChecked( !settings_setlist[dataset]->visi );
     SBox_Linesize->setValue( settings_setlist[dataset]->linesize );
     if( SBox_Linesize->value() >0 )  CB_showPoints->setEnabled( true );
     else  CB_showPoints->setEnabled( false );
     SBox_Scale->setValue( (int)(settings_setlist[dataset]->scale_rate*100) );
     str.setNum( settings_setlist[dataset]->offset_x );
     LE_Trans_X->setText( str );
     str.setNum( settings_setlist[dataset]->offset_y );
     LE_Trans_Y->setText( str );
     Btn_Color->setPaletteBackgroundColor( settings_setlist[dataset]->color );
}
 

void xyGraph::disableWidgets( )
{
          LV_Dataset->setEnabled( false ); 
          Btn_Remove->setEnabled( false ); 
          LE_Desc->setEnabled( false ); 
          Btn_Visi_Yes->setEnabled( false ); 
          Btn_Visi_No->setEnabled( false ); 
          SBox_Scale->setEnabled( false ); 
          Btn_Color->setEnabled( false ); 
          SBox_Linesize->setEnabled( false ); 
          CB_showPoints->setEnabled( false );
          LE_Trans_X->setEnabled( false ); 
          LE_Trans_Y->setEnabled( false ); 
          Btn_Data->setEnabled( false ); 
          Btn_Color->setPaletteBackgroundColor( Qt::white );
}


void xyGraph::enableWidgets( )
{
          LV_Dataset->setEnabled( true ); 
          Btn_Remove->setEnabled( true ); 
          LE_Desc->setEnabled( true ); 
          Btn_Visi_Yes->setEnabled( true ); 
          Btn_Visi_No->setEnabled( true ); 
          SBox_Scale->setEnabled( true ); 
          Btn_Color->setEnabled( true ); 
          SBox_Linesize->setEnabled( true ); 
          CB_showPoints->setEnabled( true );
          LE_Trans_X->setEnabled( true ); 
          LE_Trans_Y->setEnabled( true ); 
          Btn_Data->setEnabled( true ); 
}

   
/*************  tool functions  ************/

void xyGraph::checkBarPieData( double *y, int n )
{
     int i = 0;
     bool flag = true;
     for ( i=0; i<n; i++) {
          if( y[i]<0 ) flag = false;
     }
     if ( flag ) return;

     switch( QMessageBox::warning( this, "warning",
          "The values must be positive!\n"
          "Convert those negative values to 0, or use their abstract values?", 
          "Convert to 0", "Use abstract value", "Abort" ) ) {
     case 0:
          for( i=0; i<n; i++) {
               if( y[i]<0 ) y[i]=0;
          }
          break;
     case 1:
          for( i=0; i<n; i++) {
               if( y[i]<0 ) y[i]=fabs( y[i] );
          }
          break;
     case 2:
          exit(0);
     }
}


bool xyGraph::IsMouseeventInregion()
{
     if( xmousepress < lmargin || xmousepress > size_x-rmargin )
           return FALSE;
     if( xmouserelease < lmargin || xmouserelease > size_x-rmargin )
           return FALSE;
     if( ymousepress < (menu_height+title_height)+tmargin || ymousepress > size_y-bmargin+(menu_height+title_height) )
           return FALSE;
     if( ymouserelease < (menu_height+title_height)+tmargin || ymouserelease > size_y-bmargin+(menu_height+title_height) )
           return FALSE;
     return TRUE;
}


bool xyGraph::IsPointInregion( int x, int y )
{
     if( x < lmargin || x > size_x-rmargin )
          return false;
     if( y < tmargin_all || y > size_y-bmargin )
          return false;
     return true;
}


int xyGraph::modifiedX( int x )
{
     if( x < lmargin )          x = lmargin;
     if( x > size_x-rmargin )     x = size_x-rmargin;
     return x;
}


int xyGraph::modifiedY( int y )
{ 
     if( y < (menu_height+title_height)+tmargin )       y = (menu_height+title_height)+tmargin;
     if( y > size_y-bmargin+(menu_height+title_height) )  y = size_y-bmargin+(menu_height+title_height);
     return y;
}


double xyGraph::calStep( double min, double max, int flag )
{
     double d = fabs(max - min);
//     printf("min: %f\tmax: %f\n", min, max );
     int di;
     int exp=0;
     if( d<1.0 ) 
          for( ; d<1.0; d*=10, exp-- ) ;
     else if( d>=10 )
          for( ; d>=10; d/=10, exp++ ) ;
     di = (int)d;
     d = (double)di;
     if( flag ==1 ) //x
          dexpx = exp-1;
     if( flag==2 )
          dexpy = exp-1;
     if( exp<0 )
          for( ; exp<0; d/=10, exp++ ) ;
     if( exp>0 )
          for( ; exp>0; d*=10, exp-- ) ;
     return (d/10);
}


void xyGraph::dc_FloatToStr( double value, char* s, int exp )
{
     if( exp>=0 ) {
          sprintf( s, "%d", (int)value );
          return;
     }
     switch( exp ) {
     case -1:  sprintf( s, "%.1f", value ); break;
     case -2:  sprintf( s, "%.2f", value ); break;
     case -3:  sprintf( s, "%.3f", value ); break;
     case -4:  sprintf( s, "%.4f", value ); break;
     case -5:  sprintf( s, "%.5f", value ); break;
     case -6:  sprintf( s, "%.6f", value ); break;
     default :  sprintf( s, "%.7f", value );
     }
     return;
}

double xyGraph::nearestValue( double value, double step, int flag ) 
//if flag>=0, find the bigger one, else, find the smaller one
{
     int times = (int)(value/step);
     double newvalue1, newvalue2;
     if( value>=0 ) {
          newvalue1 = times*step;
          if ( newvalue1==value ) newvalue2 = value;
          else newvalue2 = (times+1)*step;
     }
     else{
          newvalue2 = times*step;
          if( newvalue2==value ) newvalue1 = value;
          else newvalue1 = (times-1)*step;

     }
     return ((flag<0)?newvalue1:newvalue2);
}


QColor xyGraph::chooseColor( int dataset )
{
     switch( dataset%14 ) {
     case 0:  return Qt::red;
     case 1:  return Qt::cyan;
     case 2:  return Qt::magenta;
     case 3:  return Qt::blue;
     case 4:  return Qt::green;
     case 5:  return Qt::yellow;
     case 6:  return Qt::gray;
     case 7:  return Qt::darkRed;
     case 8:  return Qt::darkCyan;
     case 9:  return Qt::darkMagenta;
     case 10:  return Qt::darkBlue;
     case 11:  return Qt::darkGreen;
     case 12:  return Qt::darkYellow;
     }
     return Qt::darkGray;
}


QColor xyGraph::chooseColorMore( int i )
{
     int n = (int)((i%36)/6);
     int b = i%6;
     switch( n ) {
     case 0: return QColor(b*60,   255,255,QColor::Hsv);
     case 1: return QColor(b*60+30,255,255,QColor::Hsv);
     case 2: return QColor(b*60+10,255,255,QColor::Hsv);
     case 3: return QColor(b*60+50,255,255,QColor::Hsv);
     case 4: return QColor(b*60+20,255,255,QColor::Hsv);
     case 5: return QColor(b*60+40,255,255,QColor::Hsv);
     }
     return Qt::black;
}


double xyGraph::getRateXFrame()
{
    return (double)width()/(double)size_x;
}


double xyGraph::getRateYFrame()
{
    return (double)height()/(double)size_y;
}


int xyGraph::getMaxStrLen( char** strlist, int num )
{
    unsigned int maxlen = 0;
    for( int i=0; i<num; i++ ) 
        maxlen = ( maxlen>=strlen(strlist[i]) ) ? maxlen : strlen(strlist[i]);
    return maxlen;
}


void xyGraph::drawDelta( int x, int y )
{
    painter->setPen( Qt::black );
    painter->drawLine( x, y, x+8, y );
    painter->drawLine( x, y, x+4, y-8 );
    painter->drawLine( x+8, y, x+4, y-8 );
}

 
void xyGraph::setDouble( QString &s, double num )
{
        if( num<=0.000001 && num>=-0.000001 )
               s.setNum(0.0);
        else s.setNum( num, 'g', 9 );
}
