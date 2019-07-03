
/******************************************************************************************
**** PROGRAM: generalparam.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef GENERALPARAM_H
#define GENERALPARAM_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3ButtonGroup;
class QLabel;
class QComboBox;
class QLineEdit;
class QSpinBox;

class NetworkClass;

class generalParam : public QWidget
{
    Q_OBJECT

public:
    generalParam( NetworkClass* np, QWidget* parent = 0, const char* name = 0 );
    ~generalParam();

    Q3ButtonGroup* buttonGroup1;
    Q3ButtonGroup* buttonGroup2;
    Q3ButtonGroup* buttonGroup3;

    QLabel* memoryless_ps_lbl;
    QComboBox* memoryless_ps_combobox;
    QLabel* ac_hide_timer_lbl;
    QLineEdit* ac_hide_timer_val;
    QSpinBox* ac_hide_thr_spinbox;
    QSpinBox* ac_use_thr_spinbox;
    QLabel* ac_use_timer_lbl;
    QLineEdit* ac_use_timer_val;

    QLabel* ac_use_thr_lbl;
    QLabel* ac_hide_thr_lbl;

    QLabel* sync_level_allocation_threshold_db_lbl;
    QLabel* cch_allocation_threshold_db_lbl;

    QLineEdit* sync_level_allocation_threshold_db_val;
    QLineEdit* cch_allocation_threshold_db_val;

    Q3ButtonGroup* buttonGp;
    Q3GridLayout*  buttonGpLayout;
    QLabel*       num_freq_lbl;
    QLineEdit*    num_freq_val;
    QLabel*       num_tch_time_slot_lbl;
    QLineEdit*    num_tch_time_slot_val;
    QLabel*       num_cch_time_slot_lbl;
    QLineEdit*    num_cch_time_slot_val;
    QLabel*       cch_freq_lbl;
    QLineEdit*    cch_freq_val;

    Q3ButtonGroup* csid_fmt_buttonGrp;
    Q3GridLayout*  csid_fmt_buttonGrpLayout;
    QLabel*       csid_fmt_lbl;
    QComboBox*    csid_fmt_combobox;

protected:
   Q3GridLayout* generalParamLayout;

   Q3GridLayout* buttonGroup1Layout;
   Q3GridLayout* buttonGroup2Layout;
   Q3GridLayout* buttonGroup3Layout;

   QSpacerItem* spacer1;
   QSpacerItem* spacer2;
   QSpacerItem* spacer3;
   QSpacerItem* spacer4;
   QSpacerItem* spacer5;
   QSpacerItem* spacer6;
   QSpacerItem* spacer7;
   QSpacerItem* spacer8;

private:
   NetworkClass* np;

protected slots:
   virtual void languageChange();

   void ac_hide_thr_valchanged( int );
   void ac_use_thr_valchanged( int );
   void csid_fmt_select(int);

public slots:
   //    virtual void cancel_btn_clicked();
   virtual void ok_btn_clicked();

};

#endif // GENERALPARAM_H
