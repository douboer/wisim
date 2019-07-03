/****************************************************************************
** Form interface generated from reading ui file 'modcltcoefdia.ui'
**
** Created: 星期四 四月 10 09:35:42 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**      -- chengan
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MODCLTCOEFDIA_H
#define MODCLTCOEFDIA_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;
class QLabel;
class QLineEdit;
class NetworkClass;

class modCltCoefDia : public QDialog
{
    Q_OBJECT

public:
    modCltCoefDia( NetworkClass*, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~modCltCoefDia();

    QPushButton* buttonHelp;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QLabel* clt_coodx_textLabel;
    QLineEdit* clt_coody_lineEdit;
    QLabel* clt_coody_textLabel;
    QLineEdit* clt_coodx_lineEdit;
    QLabel* clt_idx_textLabel;
    QLabel* clt_size_textLabel;
    QLabel* clt_coef_textLabel;
    QLineEdit* clt_idx_lineEdit;
    QLineEdit* clt_size_lineEdit;
    QLineEdit* clt_coef_lineEdit;

    void set_pm_idx(int pm_idx);

private:
    int pm_idx;
    NetworkClass* np;

protected:
    Q3GridLayout* modCltCoefDiaLayout;
    Q3HBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

public slots:
    void ok_clicked();
    void cancel_clicked();
};

#endif // MODCLTCOEFDIA_H
