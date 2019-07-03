
/******************************************************************************************
**** PROGRAM: set_strid_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef SET_STRID_DIA_H
#define SET_STRID_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QPushButton;
class QTabWidget;
class QWidget;
class QLabel;
class QLineEdit;
class NetworkClass;

class SetStridDiaClass : public QDialog
{
    Q_OBJECT

public:
    SetStridDiaClass( int s_rtti, int s_index, int s_p_idx, NetworkClass *np, QWidget* parent = 0, const char* name = 0);

public slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();

private:
    NetworkClass *np;
    QLineEdit *set_strid_dia_name_val;
    int rtti, index, p_idx;
};

#endif // SET_STRID_DIA_H
