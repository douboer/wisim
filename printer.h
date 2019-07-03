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
//Added by qt3to4:
#include <QCloseEvent>
#include "WiSim_gui.h"


/******************************************************************************************/
/**** FUNCTION: MainWindowClass::print                                                 ****/
/******************************************************************************************/

/*const char papertype[30][3][100] = {  
                        {"A0 - 841 x 1189 mm",                                "841",  "1189"},
                        {"A1 - 594 x 841 mm",                                 "594",  "841"},
                        {"A2 - 420 x 594 mm",                                 "420",  "594"},
                        {"A3 - 297 x 420 mm",                                 "297",  "420"},
                        {"A4 - 210 x 297 mm, 8.26 x 11.7 inches",             "210",  "297"},
                        {"A5 - 148 x 210 mm",                                 "148",  "210"},
                        {"A6 - 105 x 148 mm",                                 "105",  "148"},
                        {"A7 - 74 x 105 mm",                                  "74",   "105"},
                        {"A8 - 52 x 74 mm",                                   "52",   "74"},
                        {"A9 - 37 x 52 mm",                                   "37",   "52"},
                        {"B0 - 1030 x 1456 mm",                               "1030", "1456"},
                        {"B1 - 728 x 1030 mm",                                "728",  "1030"},
                        {"B10 - 32 x 45 mm",                                  "32",   "45"},
                        {"B2 - 515 x 728 mm",                                 "515",  "728"},
                        {"B3 - 364 x 515 mm",                                 "364",  "515"},
                        {"B4 - 257 x 364 mm",                                 "257",  "364"},
                        {"B5 - 182 x 257 mm, 7.17 x 10.13 inches",            "182",  "257"},
                        {"B6 - 128 x 182 mm",                                 "128",  "182"},
                        {"B7 - 91 x 128 mm",                                  "91",   "128"},
                        {"B8 - 64 x 91 mm",                                   "64",   "91"},
                        {"B9 - 45 x 64 mm",                                   "45",   "64"},
                        {"C5E - 163 x 229 mm",                                "163",  "229"},
                        {"Comm10E - 105 x 241 mm, US Common #10 Envelope",    "105",  "241"},
                        {"DLE - 110 x 220 mm",                                "110",  "220"},
                        {"Executive - 7.5 x 10 inches, 191 x 254 mm",         "191",  "254"},
                        {"Folio - 210 x 330 mm",                              "210",  "330"},
                        {"Ledger - 432 x 279 mm",                             "432",  "279"},
                        {"Legal - 8.5 x 14 inches, 216 x 356 mm",             "216",  "356"},
                        {"Letter - 8.5 x 11 inches, 216 x 279 mm",            "216",  "279"},
                        {"Tabloid - 279 x 432 mm",                            "279",  "432"}
};*/
/* the sequence is:        A4,  B5, Letter,Legal,Executive,
                           A0,  A1,  A2,  A3,  A5,  A6,   A7,  A8,  A9,  B0,  B1,
                           B10, B2,  B3,  B4,  B6,  B7,   B8,  B9,  C5E,Comm10E,
                           DLE, Folio, Ledger, Tabloid, Custom, NPageSize = Custom };*/

const int paperwidth[32] = { 210, 176, 216, 216, 191,
                           841, 594, 420, 297, 148, 105,  74,  52,  37,1000, 707,
                           31,  500, 353, 250, 125,  88,  62,  44, 163, 105, 
                           110, 210, 432, 279,   0,   0 };

const int paperheight[32]= { 297, 250, 279, 356, 254,
                          1189, 841, 594, 420, 210, 148, 105,  74,  52,1414,1000,
                            44, 707, 500, 353, 176, 125,  88,  62, 229, 241,
                           220, 330, 279, 432,   0,   0 };
 
class myQCanvasView: public Q3CanvasView{
    Q_OBJECT

public:
    myQCanvasView(Q3Canvas *canvas, QWidget *parent = 0, 
                  const char *name = 0, Qt::WFlags f = 0);
    virtual void drawContents(QPainter *p,int cx, int cy, int cw, int ch);
private:
    Q3Canvas *canvas;
    QPainter *p;
    QWidget *parent;
    const char *name;
    Qt::WFlags f;
    int cx, cy, cw, ch;
};


class PrintRect : public QDialog {
    Q_OBJECT

public:
     PrintRect();
     ~PrintRect();
     void RepaintRect();
     void CalPosition();
     Q3Canvas * thiscanvas;
     FigureEditor * thisview;
     QSpinBox * sizebox;
     double x, y, w, h;
     double size;
     int step;
     QPrinter * printer;    
     Q3CanvasRectangle *i;
signals:
     void sizeChanged( int );

public slots:
     void RectMoveUP();
     void RectMoveDOWN();
     void RectMoveLEFT();
     void RectMoveRIGHT();
     void SetStep( int );
     void RectSizeChanged( int );
     void StartPrint();
     void DeleteRect();
     void closeEvent( QCloseEvent* );
};


class PrintDialog : public QDialog {
     Q_OBJECT

public:
     void closeEvent( QCloseEvent* );
     PrintRect* printrect;

};



      


























/******************************************************************************************/
