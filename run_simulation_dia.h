
/******************************************************************************************
**** PROGRAM: run_simulation_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef RUN_SIMULATION_H
#define RUN_SIMULATION_H

#include <qlabel.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include "WiSim.h"

//*****************************************************************************
//CLASS: RunSimulation class
//*****************************************************************************
class RunSimulation : public QDialog
{
    Q_OBJECT

public:
    RunSimulation(NetworkClass *np_param, QWidget* parent = 0);
    ~RunSimulation();

protected slots:
    virtual void languageChange();

private slots:
    virtual void run_btn_clicked();
    virtual void cancel_btn_clicked();
    virtual void advanced_btn_clicked();

private:
    NetworkClass *np;

    Q3HBoxLayout  *num_event_ini_hlayout;
    Q3HBoxLayout  *num_event_run_hlayout;
    Q3HBoxLayout  *adv_run_cancel_hlayout;
    Q3VBoxLayout  *run_simulation_vlayout;

    QLabel       *num_event_ini_lbl;
    QLabel       *num_event_run_lbl;
    QSpinBox     *num_event_ini_spinbox;
    QSpinBox     *num_event_run_spinbox;

    QPushButton  *advanced_btn;
    QPushButton  *run_btn;
    QPushButton  *cancel_btn;

    bool         advancedShown;
};

//*****************************************************************************
//CLASS: AdvRunSimulation class
//*****************************************************************************
class AdvRunSimulation : public QWidget
{
    Q_OBJECT

public:
    AdvRunSimulation(NetworkClass *np_param, QWidget* parent = 0);
    ~AdvRunSimulation();

private slots:
    virtual void languageChange();

    virtual void manual_reset_btn_clicked();
    virtual void manual_run_btn_clicked();

    virtual void record_crr_file_toolbtn_clicked();
    virtual void record_crr_btn_clicked();
    virtual void stop_crr_btn_clicked();

    virtual void record_cun_file_toolbtn_clicked();
    virtual void record_cun_btn_clicked();
    virtual void stop_cun_btn_clicked();

    virtual void record_event_file_toolbtn_clicked();
    virtual void record_event_btn_clicked();
    virtual void stop_event_btn_clicked();

private:
    NetworkClass* np;
    bool          record_crr_btn_down;
    bool          record_cun_btn_down;
    bool          record_event_btn_down;

    //Layout 
    Q3VBoxLayout* adv_simulation_vlayout;
    Q3HBoxLayout* manual_top_title_hlayout;

    Q3HBoxLayout* manual_run_title_hlayout;
    Q3HBoxLayout* manual_run_hlayout;

    Q3HBoxLayout* record_crr_title_hlayout;
    Q3HBoxLayout* record_crr_hlayout;
    Q3HBoxLayout* record_crr_val_hlayout;
    Q3HBoxLayout* record_crr_num_hlayout;

    Q3HBoxLayout* record_cun_title_hlayout;
    Q3HBoxLayout* record_cun_hlayout;

    Q3HBoxLayout* record_event_title_hlayout;
    Q3HBoxLayout* record_event_hlayout;

    //set for manual run
    QLabel*      manual_run_title_lbl;
    QLabel*      num_event_lbl;
    QSpinBox*    num_event_spinbox;
    QPushButton* manual_reset_btn;
    QPushButton* manual_run_btn;

    //set for record crr
    QLabel* record_crr_title_lbl;
    QLabel* record_crr_file_lbl;
    QLabel* record_crr_min_lbl;
    QLabel* record_crr_max_lbl;
    QLabel* record_crr_num_lbl;

    QLineEdit* record_crr_file_val;
    QLineEdit* record_crr_min_val;
    QLineEdit* record_crr_max_val;
    QLineEdit* record_crr_num_val;
    QToolButton *record_crr_file_toolbtn;

    QPushButton* record_crr_btn;
    QPushButton* stop_crr_btn;

    //set for number of COMM users
    QLabel* record_cun_title_lbl;

    QLabel* record_cun_file_lbl;
    QLineEdit* record_cun_file_val;
    QToolButton *record_cun_file_toolbtn;

    QPushButton* record_cun_btn;
    QPushButton* stop_cun_btn;

    //set for record event
    QLabel* record_event_title_lbl;

    QLabel* record_event_file_lbl;
    QLineEdit* record_event_file_val;
    QToolButton* record_event_file_toolbtn;

    QPushButton* record_event_btn;
    QPushButton* stop_event_btn;
};

#endif
