/****************************************************************************
** Form implementation generated from reading ui file 'modcltcoefdia.ui'
**
** Created: 星期四 四月 10 09:36:05 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**        -- chengan
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mod_clt_coef_dia.h"
#include "WiSim.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>

/*
 *  Constructs a modCltCoefDia as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
modCltCoefDia::modCltCoefDia( NetworkClass* np_param, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "modCltCoefDia" );
    setSizeGripEnabled( TRUE );
    modCltCoefDiaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "modCltCoefDiaLayout"); 

    np = np_param;

    Layout1 = new Q3HBoxLayout( 0, 0, 6, "Layout1"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    modCltCoefDiaLayout->addMultiCellLayout( Layout1, 4, 4, 0, 3 );

    clt_coodx_textLabel = new QLabel( this, "clt_coodx_textLabel" );

    modCltCoefDiaLayout->addWidget( clt_coodx_textLabel, 0, 0 );

    clt_coody_lineEdit = new QLineEdit( this, "clt_coody_lineEdit" );
    clt_coody_lineEdit->setEnabled( FALSE );

    modCltCoefDiaLayout->addWidget( clt_coody_lineEdit, 0, 3 );

    clt_coody_textLabel = new QLabel( this, "clt_coody_textLabel" );

    modCltCoefDiaLayout->addWidget( clt_coody_textLabel, 0, 2 );

    clt_coodx_lineEdit = new QLineEdit( this, "clt_coodx_lineEdit" );
    clt_coodx_lineEdit->setEnabled( FALSE );

    modCltCoefDiaLayout->addWidget( clt_coodx_lineEdit, 0, 1 );

    clt_idx_textLabel = new QLabel( this, "clt_idx_textLabel" );

    modCltCoefDiaLayout->addWidget( clt_idx_textLabel, 1, 0 );

    clt_size_textLabel = new QLabel( this, "clt_size_textLabel" );

    modCltCoefDiaLayout->addWidget( clt_size_textLabel, 2, 0 );

    clt_coef_textLabel = new QLabel( this, "clt_coef_textLabel" );

    modCltCoefDiaLayout->addWidget( clt_coef_textLabel, 3, 0 );

    clt_idx_lineEdit = new QLineEdit( this, "clt_idx_lineEdit" );
    clt_idx_lineEdit->setEnabled( FALSE );
    clt_idx_lineEdit->setFrameShadow( QLineEdit::Sunken );

    modCltCoefDiaLayout->addMultiCellWidget( clt_idx_lineEdit, 1, 1, 1, 3 );

    clt_size_lineEdit = new QLineEdit( this, "clt_size_lineEdit" );
    clt_size_lineEdit->setEnabled( FALSE );
    clt_size_lineEdit->setFrameShadow( QLineEdit::Sunken );

    modCltCoefDiaLayout->addMultiCellWidget( clt_size_lineEdit, 2, 2, 1, 3 );

    clt_coef_lineEdit = new QLineEdit( this, "clt_coef_lineEdit" );
    clt_coef_lineEdit->setEnabled( TRUE );
    clt_coef_lineEdit->setFrameShadow( QLineEdit::Sunken );

    modCltCoefDiaLayout->addMultiCellWidget( clt_coef_lineEdit, 3, 3, 1, 3 );
    languageChange();
    resize( QSize(509, 185).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ok_clicked() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
modCltCoefDia::~modCltCoefDia()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void modCltCoefDia::languageChange()
{
    setCaption( tr( "Modify Clutter Coeffecient Dialog" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    clt_coodx_textLabel->setText( tr( "Clutter CoordX" ) );
    clt_coody_textLabel->setText( tr( "Clutter CoordY" ) );
    clt_idx_textLabel->setText( tr( "Clutter Index" ) );
    clt_size_textLabel->setText( tr( "Clutter Size" ) );
    clt_coef_textLabel->setText( tr( "Coeffecient" ) );
}

void modCltCoefDia::set_pm_idx(int pidx)
{
    pm_idx = pidx;
}

void modCltCoefDia::ok_clicked()
{
    hide();

    //set_clutter_coeff -pm_idx i -clutter_idx i -val val    
    if ( clt_coef_lineEdit->edited() ) {
        sprintf(np->line_buf, " set_clutter_coeff -pm_idx %d -clutter_idx %s -val %s",
                pm_idx, clt_idx_lineEdit->text().latin1(), clt_coef_lineEdit->text().latin1());
        np->process_command(np->line_buf);
    }

    delete this;
}

void modCltCoefDia::cancel_clicked()
{
    hide();

    delete this;
}
