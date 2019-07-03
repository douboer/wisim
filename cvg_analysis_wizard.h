/*
   wizard for layer and sir_layer type of coverage
 */
#ifndef CVG_ANALYSIS_H
#define CVG_ANALYSIS_H

#include <qvariant.h>
#include <q3wizard.h>
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
class QRadioButton;

class NetworkClass;

class CvgAnalysis : public Q3Wizard
{
    Q_OBJECT

public:
    CvgAnalysis( NetworkClass*, int iNoLayer, QWidget* parent = 0, const char* name = 0 );
    ~CvgAnalysis();


    QWidget* WizardPage_2;

    // for layer display widget
    Q3ButtonGroup* cvg_gen_param_buttonGroup;
    QLabel* max_layer_textLabel;
    QLineEdit* sig_thr_lineEdit;
    QSpinBox* max_layer_spinBox;
    QLabel* sig_thr_textLabel;

    // for PA display widget
    QRadioButton* RadioBtnNoThrd;
    QRadioButton* RadioBtnYesThrd;
    

    Q3ButtonGroup* cvg_adv_param_buttonGroup;
    Q3GridLayout* cvg_adv_param_buttonGroupLayout;
    QLabel* scan_fraction_area_textLabel;
    QLabel* init_sample_resolution_textLabel;
    QLineEdit* scan_fraction_area_lineEdit;
    QLineEdit* init_sample_resolution_lineEdit;

    // get coverage parameters
    QString cvg_type;
    QString cvg_name;
    double  sig_thr;
    int     max_layer;
    double  scan_fra_area;
    int     sam_res;
    bool    m_bNoThrd;

    bool    extension;
protected:
    Q3GridLayout* WizardPageLayout;
    Q3VBoxLayout* buttonGroup1Layout;
    Q3HBoxLayout* buttonGroup2Layout;

    Q3GridLayout* WizardPageLayout_2;
    Q3GridLayout* cvg_gen_param_buttonGroupLayout;

private:
    NetworkClass* np;
    int iNoLayer2;

private slots:
    virtual void accept();   // call it when finishButton clicked
    void adv_button_clicked();

public slots:
    void setType(int i);

protected slots:
    virtual void languageChange();
    void cancel_btn_clicked();
    void chooseThreshold(int);

};

#endif
