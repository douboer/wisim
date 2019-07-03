
/******************************************************************************************
**** PROGRAM: read_mif_dia.cpp
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef READ_MIF_DIA_H
#define READ_MIF_DIA_H

#include <qlabel.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>

#include "WiSim.h"

class ReadMIFDia : public QDialog
{
    Q_OBJECT

public:
    ReadMIFDia(NetworkClass* np_param, QWidget* parent = 0);
    ~ReadMIFDia();

    QLabel *file_name_lbl;
    QLineEdit *file_name_val;
    QToolButton *file_name_toolbtn;

    QLabel *color_lbl;
    QPushButton *color_btn;

    Q3ButtonGroup *read_type_btngroup;
    QRadioButton *filt_radiobtn;
    QRadioButton *include_radiobtn;

    QLabel *map_layer_name_lbl;
    QLineEdit *map_layer_val;

    QPushButton *ok_btn;
    QPushButton *cancel_btn;

public slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();
    void color_btn_clicked();
    void file_name_toolbtn_clicked();
    void read_type_btngroup_clicked(int i);

protected:
    Q3VBoxLayout *read_mif_vLayout;
    Q3GridLayout *read_type_btngroupLayout;

private:
    NetworkClass*     np;
    int color;
};

#endif // READ_MIF_DIA_H
