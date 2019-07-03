/****************************************************************************
** Form implementation generated from reading ui file 'internalForm.ui'
**
** Created: Wed Jan 12 10:14:53 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "reg_internalForm.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

/*
 *  Constructs a InternalForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
InternalForm::InternalForm( QWidget* parent, const char* name, Qt::WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
    setName( "InternalForm" );
    InternalFormLayout = new Q3GridLayout( this, 1, 1, 11, 6, "InternalFormLayout"); 

    layout14 = new Q3GridLayout( 0, 1, 1, 0, 6, "layout14"); 

    layout12 = new Q3HBoxLayout( 0, 0, 6, "layout12"); 

    layout3 = new Q3VBoxLayout( 0, 0, 6, "layout3"); 

    textLabel2 = new QLabel( this, "textLabel2" );
    layout3->addWidget( textLabel2 );

    lnEditName = new QLineEdit( this, "lnEditName" );
    layout3->addWidget( lnEditName );
    layout12->addLayout( layout3 );
    spacer10 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12->addItem( spacer10 );

    layout14->addLayout( layout12, 0, 0 );

    layout11 = new Q3HBoxLayout( 0, 0, 6, "layout11"); 

    layout10 = new Q3VBoxLayout( 0, 0, 6, "layout10"); 

    textLabel7 = new QLabel( this, "textLabel7" );
    layout10->addWidget( textLabel7 );

    lnEditEmail = new QLineEdit( this, "lnEditEmail" );
    layout10->addWidget( lnEditEmail );
    layout11->addLayout( layout10 );
    spacer12 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout11->addItem( spacer12 );

    layout14->addLayout( layout11, 3, 0 );

    layout9 = new Q3HBoxLayout( 0, 0, 6, "layout9"); 

    layout6 = new Q3VBoxLayout( 0, 0, 6, "layout6"); 

    textLabel5 = new QLabel( this, "textLabel5" );
    layout6->addWidget( textLabel5 );

    lnEditPhone = new QLineEdit( this, "lnEditPhone" );
    layout6->addWidget( lnEditPhone );
    layout9->addLayout( layout6 );
    spacer7_2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout9->addItem( spacer7_2 );

    layout7 = new Q3VBoxLayout( 0, 0, 6, "layout7"); 

    textLabel6 = new QLabel( this, "textLabel6" );
    layout7->addWidget( textLabel6 );

    lnEditPhoneOpt = new QLineEdit( this, "lnEditPhoneOpt" );
    layout7->addWidget( lnEditPhoneOpt );
    layout9->addLayout( layout7 );

    layout14->addLayout( layout9, 2, 0 );

    layout13 = new Q3HBoxLayout( 0, 0, 6, "layout13"); 

    layout4 = new Q3VBoxLayout( 0, 0, 6, "layout4"); 

    textLabel3 = new QLabel( this, "textLabel3" );
    layout4->addWidget( textLabel3 );

    lnEditAddr = new QLineEdit( this, "lnEditAddr" );
    layout4->addWidget( lnEditAddr );
    layout13->addLayout( layout4 );
    spacer7 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout13->addItem( spacer7 );

    layout5 = new Q3VBoxLayout( 0, 0, 6, "layout5"); 
    layout13->addLayout( layout5 );

    layout14->addLayout( layout13, 1, 0 );

    InternalFormLayout->addLayout( layout14, 0, 0 );

    textLabel8 = new QLabel( this, "textLabel8" );

    InternalFormLayout->addWidget( textLabel8, 1, 0 );
    languageChange();
    resize( QSize(402, 260).expandedTo(minimumSizeHint()) );
    // tab order
    setTabOrder( lnEditName, lnEditAddr );
    setTabOrder( lnEditAddr, lnEditPhone);
    setTabOrder( lnEditPhone, lnEditPhoneOpt);
    setTabOrder( lnEditPhoneOpt, lnEditEmail);

    // add signal/slot pairs
    connect( lnEditName, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkReq()) );
    connect( lnEditAddr, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkReq()) ) ;

    connect( lnEditEmail, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkReq()) );
    connect( lnEditPhone, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkReq()) );

}

/*
 *  Destroys the object and frees any allocated resources
 */
InternalForm::~InternalForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void InternalForm::languageChange()
{
    setCaption( tr( "Form3" ) );
    textLabel2->setText( tr( "Name (*)" ) );
    textLabel7->setText( tr( "Email Address  (*)" ) );
    textLabel5->setText( tr( "Ext. number (*)" ) );
    textLabel6->setText( tr( "Cell Number" ) );
    textLabel3->setText( tr( "Dep. (*)" ) );
    textLabel8->setText( tr( "Notice: \"*\" items are required, please fill out this form completely." ) );
}

/*
 * Simple check the validation
 *
 */
void InternalForm::checkReq() {

  if ( ( lnEditName->text() != "") && (lnEditAddr->text() != "") &&  \
       ( lnEditPhone->text() != "") &&  \
       ( lnEditEmail->text() != "") ) {

    emit done(true);

  }
  else
    emit done(false);
}
