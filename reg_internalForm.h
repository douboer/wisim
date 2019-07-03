/****************************************************************************
** Form interface generated from reading ui file 'internalForm.ui'
**
** Created: Wed Jan 12 10:14:49 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef INTERNALFORM_H
#define INTERNALFORM_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;

class InternalForm : public QWidget
{
    Q_OBJECT

public:
    InternalForm( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~InternalForm();

    QLabel* textLabel2;
    QLineEdit* lnEditName;
    QLabel* textLabel7;
    QLineEdit* lnEditEmail;
    QLabel* textLabel5;
    QLineEdit* lnEditPhone;
    QLabel* textLabel6;
    QLineEdit* lnEditPhoneOpt;
    QLabel* textLabel3;
    QLineEdit* lnEditAddr;
    QLabel* textLabel8;

protected:
    Q3GridLayout* InternalFormLayout;
    Q3GridLayout* layout14;
    Q3HBoxLayout* layout12;
    QSpacerItem* spacer10;
    Q3VBoxLayout* layout3;
    Q3HBoxLayout* layout11;
    QSpacerItem* spacer12;
    Q3VBoxLayout* layout10;
    Q3HBoxLayout* layout9;
    QSpacerItem* spacer7_2;
    Q3VBoxLayout* layout6;
    Q3VBoxLayout* layout7;
    Q3HBoxLayout* layout13;
    QSpacerItem* spacer7;
    Q3VBoxLayout* layout4;
    Q3VBoxLayout* layout5;

protected slots:
    virtual void languageChange();
    void checkReq();

signals:
    void done(bool);
};

#endif // INTERNALFORM_H
