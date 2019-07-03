#include "showhandover.h"
#include "WiSim.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>

showHandoverDia::showHandoverDia( NetworkClass* np_param, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{

    np = np_param;

    if ( !name )
	setName( "showHandoverDia" );

    setSizeGripEnabled( TRUE );
    fromToDialogLayout = new Q3GridLayout( this, 1, 1, 11, 6, "fromToDialogLayout"); 

    fromTextLabel = new QLabel( this, "fromTextLabel" );
    fromToDialogLayout->addWidget( fromTextLabel, 0, 0 );

    toTextLabel = new QLabel( this, "toTextLabel" );
    fromToDialogLayout->addWidget( toTextLabel, 1, 0 );

    spacer4 = new QSpacerItem( 20, 31, QSizePolicy::Minimum, QSizePolicy::Expanding );
    fromToDialogLayout->addItem( spacer4, 2, 2 );

    fromLineEdit = new QLineEdit( this, "fromLineEdit" );
    fromToDialogLayout->addWidget( fromLineEdit, 0, 2 );

    toLineEdit = new QLineEdit( this, "toLineEdit" );
    fromToDialogLayout->addWidget( toLineEdit, 1, 2 );

    layout = new Q3HBoxLayout( 0, 0, 6, "layout"); 

    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout->addItem( spacer1 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    layout->addWidget( buttonOk );

    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout->addItem( spacer2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    layout->addWidget( buttonCancel );

    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout->addItem( spacer3 );

    fromToDialogLayout->addMultiCellLayout( layout, 3, 3, 0, 2 );

    spacer5 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    fromToDialogLayout->addItem( spacer5, 0, 1 );

    spacer6 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    fromToDialogLayout->addItem( spacer6, 1, 1 );

    languageChange();

    resize( QSize(300, 100).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( okBtnClicked() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancelBtnClicked() ) );
}


showHandoverDia::~showHandoverDia()
{
    // no need to delete child widgets, Qt does it all for us
}


void showHandoverDia::languageChange()
{
    setCaption( tr( "show handover dialog" ) );

    fromTextLabel->setText( tr( "From" ) );
    toTextLabel->setText( tr( "To" ) );

    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );

    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );

    fromLineEdit->setText( tr( "0" ) );
    toLineEdit->setText( tr( "1" ) );
}


void showHandoverDia::okBtnClicked()
{
    this->hide();

    // execute command line here
    if ( fromLineEdit->edited() || toLineEdit->edited() ) {
        sprintf(np->line_buf, "goto -s %s -e %s", 
                fromLineEdit->text().latin1(), toLineEdit->text().latin1());
        np->process_command(np->line_buf);
    }

    delete this;
}


void showHandoverDia::cancelBtnClicked()
{
    this->hide();
    delete this;
}
