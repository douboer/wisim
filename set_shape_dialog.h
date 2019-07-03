/**********************************************************/
/**** FILE: set_shape_dialog.h                         ****/
/**** Auther: Wei Ben                                  ****/
/**** Date:   Sept. 14, 2004                           ****/
/**********************************************************/

#ifndef SET_SHAPE_DIALOG_H
#define SET_SHAPE_DIALOG_H

#include <qdialog.h>

// for StyledButton
#include <Q3Button>
#include <qpixmap.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

class QColor;
class QBrush;
//---------------

class QPushButton;
class Q3HBoxLayout;
class Q3IconView;
class Q3IconViewItem;
class Q3VBoxLayout;
class QBitMap;
class QSpacerItem;

class NetworkClass;
class FigureEditor;

class SetGroupShapeDialog : public QDialog
{
    Q_OBJECT

public:
    SetGroupShapeDialog(FigureEditor* editor_param,  QWidget* parent = 0, const char* name = "options form",
		 bool modal = TRUE, Qt::WFlags f = 0 );
    ~SetGroupShapeDialog();
    
private:
    FigureEditor *editor;
    NetworkClass *np;
    
    Q3IconView *oIconView;
    Q3IconViewItem *oIconViewItem;
    Q3HBoxLayout *oHBoxLayout;
    Q3VBoxLayout *oVBoxLayout;
    QPushButton *oYesButton;
    QPushButton *oCancelButton;
    QBitmap *oDefaultMapOne;
    QBitmap *oDefaultMapTwo;

    QSpacerItem *spacer;

    int iOptionsNum;
    QBitmap **opMapList;

    int bm_idx;
    int iBeginPos;
    
    
protected slots:
  
  void cancel();
  void changeShape();
  void getChoice(int);
 
    
};



class StyledButton : public Q3Button
{
    Q_OBJECT

    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( EditorType editor READ editor WRITE setEditor )
    Q_PROPERTY( bool scale READ scale WRITE setScale )

    Q_ENUMS( EditorType )

public:
    enum EditorType { ColorEditor, PixmapEditor };

    StyledButton( QWidget* parent = 0, const char* name = 0 );
    StyledButton( const QBrush& b, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );
    ~StyledButton();

    void setEditor( EditorType );
    EditorType editor() const;

    void setColor( const QColor& );
    void setPixmap( const QPixmap& );

    QPixmap* pixmap() const;
    QColor color() const;

    void setScale( bool );
    bool scale() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    int index;

public slots:
    virtual void onEditor();

signals:
    void changed();
    void chooseIndex(int );

protected:
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
#ifndef QTxNO_DRAGANDDROP
    void dragEnterEvent ( QDragEnterEvent * );
    void dragMoveEvent ( QDragMoveEvent * );
    void dragLeaveEvent ( QDragLeaveEvent * );
    void dropEvent ( QDropEvent * );
#endif // QT_NO_DRAGANDDROP
    void drawButton( QPainter* );
    void drawButtonLabel( QPainter* );
    void resizeEvent( QResizeEvent* );
    void scalePixmap();

private:
    QPixmap* pix;
    QPixmap* spix;  // the pixmap scaled down to fit into the button
    QColor col;
    EditorType edit;
    bool s;
    QPoint pressPos;
    bool mousePressed;

};



#endif // SET_SHAPE_DIALOG_H
