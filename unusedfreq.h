#ifndef UNUSEDFREQ_H
#define UNUSEDFREQ_H

#include <qapplication.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <q3table.h>
#include <q3ptrlist.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <vector>

class Q3HBoxLayout;
class Q3VBoxLayout;
class Q3Table;

class NetworkClass;
class SectorClass;
class MyTableItem;
class unusedFreq;

/******************************************************************************************/
/**** CLASS: unusedfreq.h                                                              ****/
/******************************************************************************************/
class MyCheckTableItem : public Q3CheckTableItem
{
public:
    MyCheckTableItem( Q3Table *table, const QString &txt ) : Q3CheckTableItem( table, txt )
    {
        //default background color value
        m_crBkg = table->paletteBackgroundColor();
    }

    QWidget *createEditor() const;
    void paint(QPainter* p,const QColorGroup& cg,const QRect& cr,bool selected)
    {
        //here use the set background color and repaint the contents of the cell
        QColorGroup g( cg );
        g.setColor( QColorGroup::Base, m_crBkg );
        Q3TableItem::paint( p, g, cr, selected );
    }

    void setBkgColor(QColor cr) { 
        m_crBkg = cr; 
    }

    QColor bkgColor() { 
        return m_crBkg; 
    }

private:
    QCheckBox* cb;
    QColor m_crBkg;
};


class MyTableItem : public Q3TableItem
{
public:
    MyTableItem(Q3Table* table, EditType et, const QString text) : Q3TableItem(table,et,text)
    {
        //init with default QTable background color
        m_crBkg = table->paletteBackgroundColor();
    }

    void paint(QPainter* p,const QColorGroup& cg,const QRect& cr,bool selected)
    {
        //here use the set background color and repaint the contents of the cell
        QColorGroup g( cg );
        g.setColor( QColorGroup::Base, m_crBkg );
        Q3TableItem::paint( p, g, cr, selected );
    }

    void setBkgColor(QColor cr) { 
        m_crBkg = cr; 
    }

    QColor bkgColor() { 
        return m_crBkg; 
    }

private:
    QColor m_crBkg;
};


class unusedFreq : public QDialog
{
    Q_OBJECT

public:
    unusedFreq(NetworkClass *np_param, SectorClass *sector, int m_cell_idx, int
    m_sector_idx, int mc_num_rows, QWidget *parent=0,const char *name=0);
    ~unusedFreq();

private slots:
    void  unused_freq_okbtn_clicked();
    void  unused_freq_cancelbtn_clicked();
    void  table_valueChanged(int, int, int);
    void  update_C( bool );
    void  update_C1( bool );

private:
    Q3VBoxLayout *unused_freq_layout;
    Q3Table* table;


    QLabel *unused_freq_caption;
    QLabel *unused_freq_lbl;
    QLineEdit *unused_freq_val;
    QLabel *num_unused_freq_lbl;
    QLineEdit *num_unused_freq_val;

    QPushButton *unused_freq_okbtn;
    QPushButton *unused_freq_cancelbtn;

    MyTableItem* tab_item;
    Q3PtrList <MyTableItem> tab_list;
    
    Q3ComboTableItem* combo_item;
    Q3PtrList <Q3ComboTableItem> combo_list;

    QRadioButton* check_item;
    Q3PtrList <QRadioButton> check_list;

    QRadioButton* check_item1;
    Q3PtrList <QRadioButton> check_list1;
    
    std::vector <bool> raw_pm_vec;
    QStringList  text_list;

    int raw_select;
    int m_chan_idx;
    bool flag;

    NetworkClass *np;
    SectorClass *sector;
    int num_rows;

    int sector_idx;
    int cell_idx;
};
/******************************************************************************************/
#endif
