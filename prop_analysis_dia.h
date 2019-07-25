/******************************************************************************************/
/**** PROGRAM: prop_analysis_dia.h                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/
#ifndef PROP_ANALYSIS_DIA_H
#define PROP_ANALYSIS_DIA_H

#include <qlabel.h>
#include <q3frame.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <q3scrollview.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <vector>

#include "wisim.h"

class QLabel;
class QSpinBox;
class QLineEdit;
class QCheckBox;

class PropAnalysisDia : public QDialog
{
    Q_OBJECT

public:
    PropAnalysisDia( NetworkClass* np_param, QWidget* parent = 0);
    ~PropAnalysisDia();

    QRadioButton* seg_type_radioButton;
    QRadioButton* clt_type_radioButton;

    //QCheckBox       **cell_sector_checkbox;
    std::vector <QCheckBox*> cell_sector_ckb_vec;

protected:

    Q3ButtonGroup* clutter_param_group;
    Q3GridLayout*  clutter_param_groupLayout;

    QLabel*       init_clt_res_lbl;
    QComboBox*    init_clt_res_val;
    QLabel*       num_clt_split_lbl;
    QSpinBox*     num_clt_split_val;
    QLabel*       clt_res_lbl;
    QLabel*       clt_res_val;

    Q3ButtonGroup* prop_type_buttonGroup;
    Q3VBoxLayout*  prop_type_buttonGroupLayout;

    Q3GridLayout     *prop_analysis_diaLayout;
    Q3GridLayout     *prop_analysis_frameLayout;
    Q3HBoxLayout     *ok_cancel_hboxLayout;

    Q3ScrollView     *prop_analysis_scrollview;
    Q3Frame          *prop_analysis_frame;

    QComboBox       *cell_sector_name_combobox;
    QCheckBox       *all_sector_checkbox;

    Q3ButtonGroup    *scope_option_btnGroup;
    QRadioButton    *global_scope_radiobtn;
    QRadioButton    *individual_scope_radiobtn;

    Q3ButtonGroup    *other_option_btnGroup;
    QCheckBox       *useheight_checkbox;
    QCheckBox       *adjust_antenna_checkbox;

    QPushButton     *ok_btn;
    QPushButton     *cancel_btn;

private slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();
    void all_sector_selected(int);
    void cell_sector_name_combobox_changed(int);
    void resizeEvent( QResizeEvent* );
    void init_clt_res_changed(int);
    void num_clt_split_changed(int);

    void prop_type_selected(int);
    void scope_option_selected(int);

private:

    int prop_type;
    enum PropType { Seg, Clt };
    enum CltType  { Simp, Full };

    int useheight;
    int adjust_angles;
    int global;
    
    int            sector_number;
    char           **cell_sector_name;
    NetworkClass   *np;
};

#endif // PROP_ANALYSIS_DIA_H
