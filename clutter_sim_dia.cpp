
/*******************************************************************************************
**** PROGRAM: clutter_sim_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
/******************************************************************************************/
/**** PROGRAM: clutter_sim_dia.cpp                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>

#include "clutter_sim_dia.h"
#include "wisim.h"

ClutterSim::ClutterSim( NetworkClass* np_param, QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    np = np_param;
    
    setSizeGripEnabled( TRUE );
    ClutterSimLayout = new Q3GridLayout( this, 1, 1, 11, 6, "ClutterSimLayout"); 

    layout2 = new Q3HBoxLayout( 0, 0, 6, "layout2"); 
    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    layout2->addWidget( buttonOk );
    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer2 );
    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    layout2->addWidget( buttonCancel );
    ClutterSimLayout->addMultiCellLayout( layout2, 3, 3, 0, 3 );

    sim_type_textLabel = new QLabel( this, "sim_type_textLabel" );
    ClutterSimLayout->addWidget( sim_type_textLabel, 0, 0 );

    from_scan_textLabel = new QLabel( this, "from_scan_textLabel" );
    ClutterSimLayout->addWidget( from_scan_textLabel, 1, 0 );

    from_scan_lineEdit = new QLineEdit( this, "from_scan_lineEdit" );
    ClutterSimLayout->addWidget( from_scan_lineEdit, 1, 1 );

    num_scan_textLabel = new QLabel( this, "num_scan_textLabel" );
    ClutterSimLayout->addWidget( num_scan_textLabel, 1, 2 );

    num_scan_lineEdit = new QLineEdit( this, "num_scan_lineEdit" );
    ClutterSimLayout->addWidget( num_scan_lineEdit, 1, 3 );

    useheight_textLabel = new QLabel( this, "useheight_textLabel" );
    ClutterSimLayout->addMultiCellWidget( useheight_textLabel, 2, 2, 0, 1 );

    useheight_comboBox = new QComboBox( FALSE, this, "useheight_comboBox" );
    ClutterSimLayout->addMultiCellWidget( useheight_comboBox, 2, 2, 2, 3 );

    sim_type_comboBox = new QComboBox( FALSE, this, "sim_type_comboBox" );
    ClutterSimLayout->addMultiCellWidget( sim_type_comboBox, 0, 0, 2, 3 );

    languageChange();
    resize( QSize(395, 219).expandedTo(minimumSizeHint()) );

    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( sim_type_comboBox, SIGNAL( activated( int ) ), this, SLOT( sim_type_comboBox_select( int ) ) );
    connect( useheight_comboBox, SIGNAL( activated( int ) ), this, SLOT( useheight_comboBox_select( int ) ) );
}


ClutterSim::~ClutterSim()
{
}


void ClutterSim::languageChange()
{
    setCaption( tr( "Clutter Simulation Dialog" ) );

    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    sim_type_textLabel->setText( tr( "Choose Sim Type" ) );
    from_scan_textLabel->setText( tr( "Range : From" ) );
    num_scan_textLabel->setText( tr( "Number" ) );
    useheight_textLabel->setText( tr( "Use Antenna Height" ) );

    useheight_comboBox->clear();
    useheight_comboBox->insertItem( tr( "Yes" ) );
    useheight_comboBox->insertItem( tr( "No" ) );

    sim_type_comboBox->clear();
    sim_type_comboBox->insertItem( tr( "Single" ) );
    sim_type_comboBox->insertItem( tr( "Range" ) );
    sim_type_comboBox->insertItem( tr( "All" ) );
    sim_type_comboBox->insertItem( tr( "Global" ) );
}


void ClutterSim::ok_btn_clicked()
{

}

void ClutterSim::cancel_btn_clicked()
{
    delete this;
}

void ClutterSim::sim_type_comboBox_select( int i )
{
    int index = i;
#if 0
    switch (index) {
        case 0:
            type = Single;
            break;
        case 1:
            type = Range;
            break;
        case 2:
            type = All;
            break;
        case 2:
            type = Global;
            break;
        default:
            break;
    }
#endif
}

void ClutterSim::useheight_comboBox_select( int i )
{
    int index = i; 
    switch (index) {
        case 0:
            useheight = TRUE;
            break;
        case 1:
            useheight = FALSE;
            break;
        default:
            break;
    }
}

