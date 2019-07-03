/************************************/
/* Simple dialog for displaying the */
/* user's choice cells              */
/*************************************/
  
#ifndef DISP_CELLS_H
#define DISP_CELLS_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3ListView;
class Q3ListViewItem;
class QPushButton;
class QStringList;

class disp_cells : public QDialog
{
    Q_OBJECT

public:
    disp_cells( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~disp_cells();

    Q3ListView* listView4;
    QPushButton* pushButton6;

    void setCellList(const QStringList&);
    

protected:
    Q3VBoxLayout* disp_cellsLayout;
    QSpacerItem* spacer5;
    Q3HBoxLayout* layout22;
    QSpacerItem* spacer6;

protected slots:
    virtual void languageChange();

private:
    void init();
    QStringList m_Display;

};

#endif // DISP_CELLS_H

