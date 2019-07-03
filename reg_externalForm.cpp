/****************************************************************************
** Form implementation generated from reading ui file 'externalForm.ui'
**
** Created: Wed Jan 12 10:14:51 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.1   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "reg_externalForm.h"

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
 *  Constructs a ExternalForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
ExternalForm::ExternalForm( QWidget* parent, const char* name, Qt::WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
    setName( "ExternalForm" );

    QWidget* privateLayoutWidget = new QWidget( this, "layout36" );
    privateLayoutWidget->setGeometry( QRect( 20, 21, 382, 236 ) );
    layout36 = new Q3GridLayout( privateLayoutWidget, 1, 1, 11, 6, "layout36"); 

    layout9 = new Q3HBoxLayout( 0, 0, 6, "layout9"); 

    layout6 = new Q3VBoxLayout( 0, 0, 6, "layout6"); 

    textLabel5 = new QLabel( privateLayoutWidget, "textLabel5" );
    layout6->addWidget( textLabel5 );

    lnEditPhone = new QLineEdit( privateLayoutWidget, "lnEditPhone" );
    layout6->addWidget( lnEditPhone );
    layout9->addLayout( layout6 );
    spacer7_2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout9->addItem( spacer7_2 );

    layout7 = new Q3VBoxLayout( 0, 0, 6, "layout7"); 

    textLabel6 = new QLabel( privateLayoutWidget, "textLabel6" );
    layout7->addWidget( textLabel6 );

    lnEditPhoneOpt = new QLineEdit( privateLayoutWidget, "lnEditPhoneOpt" );
    layout7->addWidget( lnEditPhoneOpt );
    layout9->addLayout( layout7 );

    layout36->addMultiCellLayout( layout9, 2, 2, 0, 1 );
    spacer12 = new QSpacerItem( 185, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout36->addItem( spacer12, 3, 1 );
    layout10 = new Q3VBoxLayout( 0, 0, 6, "layout10"); 

    textLabel7 = new QLabel( privateLayoutWidget, "textLabel7" );
    layout10->addWidget( textLabel7 );

    lnEditEmail = new QLineEdit( privateLayoutWidget, "lnEditEmail" );
    layout10->addWidget( lnEditEmail );

    layout36->addLayout( layout10, 3, 0 );

    textLabel8 = new QLabel( privateLayoutWidget, "textLabel8" );

    layout36->addMultiCellWidget( textLabel8, 4, 4, 0, 1 );

    layout34 = new Q3HBoxLayout( 0, 0, 6, "layout34"); 

    layout4 = new Q3VBoxLayout( 0, 0, 6, "layout4"); 

    textLabel3 = new QLabel( privateLayoutWidget, "textLabel3" );
    layout4->addWidget( textLabel3 );

    lnEditAddr = new QLineEdit( privateLayoutWidget, "lnEditAddr" );
    layout4->addWidget( lnEditAddr );
    layout34->addLayout( layout4 );
    spacer7 = new QSpacerItem( 70, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout34->addItem( spacer7 );

    layout33 = new Q3VBoxLayout( 0, 0, 6, "layout33"); 

    textLabel3_2 = new QLabel( privateLayoutWidget, "textLabel3_2" );
    layout33->addWidget( textLabel3_2 );

    lnEditAddr_2 = new QLineEdit( privateLayoutWidget, "lnEditAddr_2" );
    layout33->addWidget( lnEditAddr_2 );
    layout34->addLayout( layout33 );

    layout36->addMultiCellLayout( layout34, 1, 1, 0, 1 );

    layout12 = new Q3HBoxLayout( 0, 0, 6, "layout12"); 

    layout3 = new Q3VBoxLayout( 0, 0, 6, "layout3"); 

    textLabel2 = new QLabel( privateLayoutWidget, "textLabel2" );
    layout3->addWidget( textLabel2 );

    lnEditName = new QLineEdit( privateLayoutWidget, "lnEditName" );
    layout3->addWidget( lnEditName );
    layout12->addLayout( layout3 );
    spacer10 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12->addItem( spacer10 );

    layout36->addMultiCellLayout( layout12, 0, 0, 0, 1 );
    languageChange();
    resize( QSize(422, 278).expandedTo(minimumSizeHint()) );

    // tab order
    setTabOrder( lnEditName, lnEditAddr );
    setTabOrder( lnEditAddr, lnEditAddr_2);
    setTabOrder( lnEditAddr_2, lnEditPhone);
    setTabOrder( lnEditPhone, lnEditPhoneOpt);
    setTabOrder( lnEditPhoneOpt, lnEditEmail);



    // add signal/slot pairs
    connect( lnEditName, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkRequires()) );
    connect( lnEditAddr, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkRequires()) ) ;
    connect( lnEditAddr_2, SIGNAL(textChanged(const QString &)), \
                                this, SLOT(checkRequires()) );
    connect( lnEditEmail, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkRequires()) );
    connect( lnEditPhone, SIGNAL(textChanged(const QString &) ), \
                                this, SLOT(checkRequires()) );
    

}

/*
 *  Destroys the object and frees any allocated resources
 */
ExternalForm::~ExternalForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ExternalForm::languageChange()
{
    setCaption( tr( "Form4" ) );
    textLabel5->setText(   tr("Telephone")           + " (*)" );
    textLabel6->setText(   tr("Secondary Telephone") );
    textLabel7->setText(   tr("Email Address")       + " (*)" );
    textLabel3->setText(   tr("Mailing Address")     + " (*)" );
    textLabel3_2->setText( tr("Postal Code")         + " (*)" );
    textLabel2->setText(   tr("Name")                + " (*)" );

    textLabel8->setText(   tr("Note: Items marked with an asterisk \"*\" are required.") );
}

void ExternalForm::checkRequires() {

  if ( ( lnEditName->text() != "") && (lnEditAddr->text() != "") &&  \
       ( lnEditPhone->text() != "") && (lnEditAddr_2->text() != "") && \
       ( lnEditEmail->text() != "") ) {

    emit finishCheck(true);
  
  }
  else
    emit finishCheck(false);

}
