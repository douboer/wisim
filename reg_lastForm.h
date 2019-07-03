/****************************************************************************
** Form interface generated from reading ui file 'lastform.ui'
**
** Created: Wed Jan 12 10:05:53 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LASTFORM_H
#define LASTFORM_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3TextEdit;

class lastForm : public QWidget
{
    Q_OBJECT

public:
    lastForm( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~lastForm();

    Q3TextEdit* textEdit1;

protected:
    Q3GridLayout* lastFormLayout;

protected slots:
    virtual void languageChange();

};

#endif // LASTFORM_H
