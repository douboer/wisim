/****************************************************************************
** Form implementation generated from reading ui file 'lastform.ui'
**
** Created: Wed Jan 12 10:05:57 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "reg_lastForm.h"

#include <qvariant.h>
#include <q3textedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>

/*
 *  Constructs a lastForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
lastForm::lastForm( QWidget* parent, const char* name, Qt::WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
    setName( "lastForm" );
    lastFormLayout = new Q3GridLayout( this, 1, 1, 11, 6, "lastFormLayout");

    textEdit1 = new Q3TextEdit( this, "textEdit1" );

    lastFormLayout->addWidget( textEdit1, 0, 0 );
    languageChange();
    resize( QSize(214, 150).expandedTo(minimumSizeHint()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
lastForm::~lastForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void lastForm::languageChange()
{
    setCaption( tr( "Form5" ) );
}

