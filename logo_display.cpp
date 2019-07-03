#include <qobject.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPaintEvent>
#include "logo.h"
#include "logo_display.h"

LogoDisplay::LogoDisplay(QWidget *parent, const char *name, Qt::WFlags f) : QWidget(parent, name, f)
{
    //pixmap = new QPixmap((const char **) WiSim_logo);
    pixmap = new QPixmap("splash_pic.png");

    // setFixedSize(QSize(pixmap->width(), pixmap->height()));
    setGeometry( (parent->width()-pixmap->width())/2, (parent->height()-pixmap->height())/2, pixmap->width(), pixmap->height());

    show();

    t = new QTimer();
    QObject::connect( t, SIGNAL(timeout()), this, SLOT(undisplay()) );
    t->start( 5000, TRUE);
}

LogoDisplay::~LogoDisplay()
{
    delete pixmap;
}

void LogoDisplay::undisplay()
{
    delete t;
    delete this;
}

void LogoDisplay::paintEvent( QPaintEvent * )
{
    QPainter paint( this );                     // paint widget
    paint.drawPixmap(0, 0, *pixmap );           // draw picture
}

