
/******************************************************************************************
**** PROGRAM: rtd_threshold_dialog.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef RTD_THRESHOLD_DIALOG_H
#define RTD_THRESHOLD_DIALOG_H

#include <q3wizard.h>
#include <qvariant.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class NetworkClass;

class Q3ButtonGroup;
class QComboBox;
class Q3GridLayout;
class Q3HBoxLayout;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpacerItem;
class QSpinBox;
class Q3Table;
class Q3VBoxLayout;
class QWidget;

class NetworkClass;

template<class T> class ListClass;

class RTDThresholdDialog : public Q3Wizard
{
    Q_OBJECT

public:
    RTDThresholdDialog( NetworkClass*, QWidget* parent = 0, const char* name = 0);
    ~RTDThresholdDialog();

    QWidget*      WizardPage;
    Q3ButtonGroup* gen_param_buttonGroup;

    QLabel*       num_thr_textLabel;
    QLabel*       min_thr_textLabel;
    QLabel*       max_thr_textLabel;

    QSpinBox*     num_thr_spinBox;
    QLineEdit*    min_thr_lineEdit;
    QLineEdit*    max_thr_lineEdit;

    QWidget*      WizardPage1;
    Q3Table*       table;

    Q3ValueVector <double>& getThrVector();

    int    num_thr;
    double min_thr;
    double max_thr;

private:
    Q3ValueVector <double> m_thr_vector;
    NetworkClass* np;

protected:
    Q3VBoxLayout* WizardPageLayout;
    Q3HBoxLayout* WizardPage1Layout;
    Q3GridLayout* gen_param_buttonGroupLayout;

protected slots:
    void maccept();

    void languageChange();
    void table_valueChanged(int, int );
    void cancel_btn_clicked();
    void thr_changed();
    void back_clicked();
    void next_btn_clicked();

};

#endif // RTD_THRESHOLD_DIALOG_H
