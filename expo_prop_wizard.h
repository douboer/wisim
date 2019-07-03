
/******************************************************************************************
**** PROGRAM: expo_prop_wizard.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

/*******************************************************************************************
**** PROGRAM: expo_prop_wizard.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef EXPO_PROP_WIZARD_H
#define EXPO_PROP_WIZARD_H

#include <qvariant.h>
#include <q3wizard.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

#include "prop_model.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QWidget;
class QLabel;
class QLineEdit;
class Q3ButtonGroup;
class QRadioButton;
class NetworkClass;

class ExpoPropWizard : public Q3Wizard
{
    Q_OBJECT

public:
    ExpoPropWizard( NetworkClass*,  QWidget* parent = 0, const char* name = 0, 
                bool modal = FALSE, Qt::WFlags fl = 0 );
    ~ExpoPropWizard();

    QWidget* WizardPage;
    QLabel* name_textLabel;
    QLineEdit* name_lineEdit;
    QLabel* type_textLabel;
    QLineEdit* type_lineEdit;
    Q3ButtonGroup* param_buttonGroup;
    QLabel* exp_textLabel;
    QLineEdit* exp_lineEdit;
    QLabel* coe_textLabel;
    QLineEdit* coe_lineEdit;

    QString prop_type;
    QString prop_name;
    double expo_val;
    double coe_val;
protected:
    NetworkClass* np;
    
    Q3GridLayout* WizardPageLayout;
    Q3HBoxLayout* name_layout;
    Q3HBoxLayout* type_layout;
    Q3VBoxLayout* prop_type_buttonGroupLayout;
    Q3GridLayout* param_buttonGroupLayout;

protected slots:
    virtual void languageChange();
    void accept();

};
#endif // EXPO_PROP_WIZARD_H
