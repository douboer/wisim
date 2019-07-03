
/******************************************************************************************
**** PROGRAM: cvg_level_wizard.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef CVG_LEVEL_WIZARD_H
#define CVG_LEVEL_WIZARD_H

#include <qvariant.h>
#include <q3wizard.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QWidget;
class Q3ButtonGroup;
class QLabel;
class QLineEdit;
class QSpinBox;
class Q3Table;
class NetworkClass;

class CvgLevelWizard : public Q3Wizard
{
    Q_OBJECT

public:
    CvgLevelWizard( NetworkClass*, QWidget* parent = 0, const char* name = 0);
    ~CvgLevelWizard();

    QWidget* WizardPage1;

    Q3ButtonGroup* cvg_gen_param_buttonGroup;
    QLabel* info_max_thr_textLabel;
    QLabel* info_min_thr_textLabel;
    QLabel* num_thr_textLabel;
    QLineEdit* info_max_thr_lineEdit;
    QLineEdit* info_min_thr_lineEdit;
    QSpinBox* num_thr_spinBox;
    Q3ButtonGroup* cvg_adv_param_buttonGroup;
    QLabel* scan_fraction_area_textLabel;
    QLabel* init_sample_resolution_textLabel;
    QLineEdit* scan_fraction_area_lineEdit;
    QLineEdit* init_sample_resolution_lineEdit;

    QWidget* WizardPage2;
    Q3Table* cvg_table;

    Q3ValueVector <double>& getThrVector( );

    int    num_thr;
    double min_thr;
    double max_thr;
    double scan_fra_area;
    int    sam_res;

    bool   extension;
private:
    Q3ValueVector <double> m_thr_vector;

    NetworkClass* np;

protected:

    Q3VBoxLayout* WizardPage1Layout;
    Q3HBoxLayout* WizardPage2Layout;

    Q3GridLayout* cvg_gen_param_buttonGroupLayout;
    Q3GridLayout* cvg_adv_param_buttonGroupLayout;

protected slots:
    virtual void accept();
    void adv_button_clicked();

    virtual void languageChange();
    void table_valueChanged(int, int );
    void cancel_btn_clicked();
    void thr_changed();
    void back_clicked();
    void next_btn_clicked();

};

#endif // CVG_LEVEL_WIZARD_H
