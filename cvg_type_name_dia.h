/******************************************************************************************/
/**** PROGRAM: cvg_type_name_dia.h                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef CVG_TYPE_NAME_DIA_H 
#define CVG_TYPE_NAME_DIA_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>
// #include <qlayout.h>
// #include <qlabel.h>
// #include <qbuttongroup.h>
// #include <qradiobutton.h>
// #include <qlineedit.h>
// #include <qpushbutton.h>

class QRadioButton;
class QLabel;
class QLineEdit;
class Q3ButtonGroup;
class Q3GridLayout;
class Q3HBoxLayout;

class NetworkClass;

/******************************************************************************************/
/****                            CLASS: CvgTypeNameDia				       ****/
/******************************************************************************************/
class CvgTypeNameDia : public QDialog
{
    Q_OBJECT

public:
    CvgTypeNameDia(NetworkClass *np_param, QWidget* parent);		
    ~CvgTypeNameDia();		

    QRadioButton *layer_radiobtn;
    QRadioButton *level_radiobtn;
    QRadioButton *sir_layer_radiobtn;
    QRadioButton *pa_layer_radiobtn;

    QPushButton  *ok_btn;
    QPushButton  *cancel_btn;

    QLabel       *cvg_name_lbl;
    QLineEdit    *cvg_name_val;

    QString cvg_type;
    int     cvg_type_idx;
    QString cvg_name;
private slots:
    virtual void cvg_type_btngroup_clicked(int);
    void cvg_ok_btn_clicked();
    void cvg_cancel_btn_clicked();

private:
    NetworkClass *np;
    Q3ButtonGroup *cvg_type_btngroup;
    Q3GridLayout  *cvg_type_name_grid;
    Q3HBoxLayout  *ok_cancel_hlayout;
};
#endif

