#include <qdialog.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

class QPixmap;
class QTimer;

class LogoDisplay : public QWidget
{

Q_OBJECT

public:
    LogoDisplay(QWidget *parent=0, const char *name=0, Qt::WFlags f = 0);
   ~LogoDisplay();

public slots:
    void undisplay();

protected:
    void        paintEvent( QPaintEvent * );

private:
    QPixmap    *pixmap;
    QTimer     *t;
};

