
/******************************************************************************************
**** PROGRAM: expo_prop_wizard.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

/*******************************************************************************************
**** PROGRAM: clutter_data_analysis.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <qvariant.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>

#include "expo_prop_wizard.h"
#include "wisim.h"

ExpoPropWizard::ExpoPropWizard( NetworkClass* np_param, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : Q3Wizard( parent, name, modal, fl ), np( np_param )
{
    if ( !name )
	setName( "ExpoPropWizard" );

    WizardPage = new QWidget( this, "WizardPage" );
    WizardPageLayout = new Q3GridLayout( WizardPage, 1, 1, 11, 6, "WizardPageLayout"); 

    //----------------------------------------------------------------------
    name_layout = new Q3HBoxLayout( 0, 0, 6, "name_layout"); 

    name_textLabel = new QLabel( WizardPage, "name_textLabel" );
    name_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, name_textLabel->sizePolicy().hasHeightForWidth() ) );
    name_layout->addWidget( name_textLabel );

    name_lineEdit = new QLineEdit( WizardPage, "name_lineEdit" );
    name_lineEdit->setEnabled( FALSE );
    name_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, name_lineEdit->sizePolicy().hasHeightForWidth() ) );
    name_layout->addWidget( name_lineEdit );

    WizardPageLayout->addLayout( name_layout, 0, 0 );

    //----------------------------------------------------------------------
    type_layout = new Q3HBoxLayout( 0, 0, 6, "type_layout"); 

    type_textLabel = new QLabel( WizardPage, "type_textLabel" );
    type_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, type_textLabel->sizePolicy().hasHeightForWidth() ) );
    type_layout->addWidget( type_textLabel );

    type_lineEdit = new QLineEdit( WizardPage, "type_lineEdit" );
    type_lineEdit->setEnabled( FALSE );
    type_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, type_lineEdit->sizePolicy().hasHeightForWidth() ) );
    type_layout->addWidget( type_lineEdit );

    WizardPageLayout->addLayout( type_layout, 1, 0 );

    //----------------------------------------------------------------------
    param_buttonGroup = new Q3ButtonGroup( WizardPage, "param_buttonGroup" );
    param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    param_buttonGroup->layout()->setSpacing( 15 );
    param_buttonGroup->layout()->setMargin( 11 );

    param_buttonGroupLayout = new Q3GridLayout( param_buttonGroup->layout() );
    param_buttonGroupLayout->setAlignment( Qt::AlignTop );

    exp_textLabel = new QLabel( param_buttonGroup, "exp_textLabel" );
    exp_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, exp_textLabel->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( exp_textLabel, 0, 0 );

    exp_lineEdit = new QLineEdit( param_buttonGroup, "exp_lineEdit" );
    exp_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, exp_lineEdit->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( exp_lineEdit, 0, 1 );

    coe_textLabel = new QLabel( param_buttonGroup, "coe_textLabel" );
    param_buttonGroupLayout->addWidget( coe_textLabel, 1, 0 );

    coe_lineEdit = new QLineEdit( param_buttonGroup, "coe_lineEdit" );
    param_buttonGroupLayout->addWidget( coe_lineEdit, 1, 1 );

    WizardPageLayout->addWidget( param_buttonGroup, 2, 0 );

    //----------------------------------------------------------------------
    addPage( WizardPage, QString("") );

    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );

    languageChange();
    resize( QSize(500, 322).expandedTo(minimumSizeHint()) );

    helpButton()->setDisabled( true );

}

/*
 *  Destroys the object and frees any allocated resources
 */
ExpoPropWizard::~ExpoPropWizard()
{

}

void ExpoPropWizard::accept(){
//    prop_name = name_lineEdit->text();
    expo_val  = exp_lineEdit->text().toDouble();
    coe_val   = coe_lineEdit->text().toDouble();

    QDialog::accept();
}

/*
 *  Sets the strings of the subwidgets
 */
void ExpoPropWizard::languageChange()
{
    setCaption( tr( "Exponential Propagation Model" ) );

    type_textLabel->setText( tr( "Type" ) );
    name_textLabel->setText( tr( "Name" ) );
    param_buttonGroup->setTitle( tr( "Propagation Model Parameters" ) );
    exp_textLabel->setText( tr( "Exponent" ) );
    coe_textLabel->setText( tr( "Coefficient" ) );
    setTitle( WizardPage, tr( "Exponential Propagation Model Parameters" ) );

    exp_lineEdit->setText( QString("%1").arg(3) );
    coe_lineEdit->setText( QString("%1").arg(6.25e+07) );

    backButton()->setText(tr("&Back") + " <");
    nextButton()->setText(tr("&Next") + " >");
    finishButton()->setText(tr("&Finish"));
    cancelButton()->setText(tr("&Cancel"));
    helpButton()->setText(tr("&Help"));

    setFinishEnabled( WizardPage, true );    
}

