/****************************************************************************
** Form interface generated from reading ui file 'reg_form.ui'
**
** Created: Wed Jan 12 10:36:32 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef REGFORM_H
#define REGFORM_H

#include <qvariant.h>
#include <q3wizard.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include "reg_externalForm.h"
#include "reg_internalForm.h"
#include "reg_lastForm.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QWidget;
class Q3ButtonGroup;
class QRadioButton;
class QString;


typedef struct {
  QString name;
  QString addr;
  QString post_code;
  QString phone;
  QString phoneOpt;
  QString email;
} info;


class regForm : public Q3Wizard
{
    Q_OBJECT

public:
    regForm(  unsigned char *reg_info, int ris, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~regForm();

    QWidget* WizardPage;
    Q3ButtonGroup* userTypeGrp;
    QRadioButton* radioButton1;
    QRadioButton* radioButton2;

protected:
    InternalForm *internal_form;
    ExternalForm *external_form;
    lastForm *last_form;

    Q3GridLayout* WizardPageLayout;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;

protected slots:
    virtual void languageChange();

    void getType(int i );
    void enableNext(bool yes);
    void cancel();

    void sendReg();
    void createReg();

private:
    void init();

    int m_type;
    info content;

    unsigned char *reg_info;
    int ris;

    char *reg_file_rel;
    char *reg_file_full;

};

#endif // REGFORM_H
