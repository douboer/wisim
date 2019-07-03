#ifndef SECTOR_PROP_TABLE_H
#define SECTOR_PROP_TABLE_H

#include <qvariant.h>
#include <qdialog.h>
#include <q3ptrlist.h>
#include <q3table.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

#include "prop_model.h"
#include <vector>

class Q3VBoxLayout;
class Q3HBoxLayout;
class QLabel;
class Q3GridLayout;
class QMessageBox;
class QSpacerItem;
class QPushButton;
class Q3ComboTableItem;
class Q3ButtonGroup;
class QComboBox;
class NetWorkClass;
class SectorPropTable;

class TableItem : public Q3TableItem
{
public:
    TableItem( Q3Table *t, EditType et, const QString &txt ) : Q3TableItem( t, et, txt ) {}
    void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

// re-imprement QTableItem for QPushButton item
class QPushButtonTableItem : public Q3TableItem
{
public:
    QPushButtonTableItem( Q3Table *table, const QString &txt );
    virtual void paint( QPainter *p, const QColorGroup &cg,
            const QRect &cr, bool selected );
    void setText( const QString &t );
    virtual QWidget *createEditor() const;
    QPushButton *getWidget();

private:
    QPushButton *pb;
    QPixmap pix;
    uint wordwrap : 1;
};


class SectorPropTable : public QDialog
{
    Q_OBJECT

public:
    SectorPropTable( NetworkClass*, QWidget* parent = 0, const char* name = 0 );
    ~SectorPropTable();

    Q3Table* table;

protected:
    Q3GridLayout* SectorPropTableLayout;
    QSpacerItem* spacer1;
    QPushButton* ok_btn;
    QSpacerItem* spacer2;
    QPushButton* cancel_btn;
    QSpacerItem* spacer3;
    TableItem*   tab_item;

    Q3ButtonGroup*set_unassigned_prop_model_buttonGroup;
    QLabel*      set_unassigned_prop_model_lbl;
    QPushButton* set_unassigned_prop_model_btn;
    Q3HBoxLayout* set_unassigned_prop_model_buttonGroupLayout;

    Q3ButtonGroup*set_all_unassigned;
    QLabel*      set_all_unassigned_lbl;
    QPushButton* set_all_unassigned_btn;
    Q3HBoxLayout* set_all_unassignedLayout;

    Q3ComboTableItem* combo_item;
    QPushButtonTableItem* btn_item;
private:
    NetworkClass* np;
    
    Q3PtrList <Q3ComboTableItem> combo_list;  
    std::vector <int> raw_pm_vec;
    QStringList    text_list;

    bool find_unassigned_prop;
    bool have_assigned_prop;
    bool no_assigned_prop;
    bool all_unassigned;

protected slots:
    virtual void languageChange();
    
    void ok_btn_clicked();
    void cancel_btn_clicked();
    void set_unassigned_prop_model_btn_clicked();
    void set_all_unassigned_btn_clicked();
    void combo_selected(int, int);

};

#endif // SECTOR_PROP_TABLE_H
