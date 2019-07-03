/******************************************************************************************/
/**** PROGRAM: prop_type_name_dia.cpp                                                  ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/


#include <qvariant.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <iostream>

#include "prop_type_name_dia.h"
#include "prop_model.h"
#include "WiSim.h"
#include "cconst.h"

PropTypeNameDia::PropTypeNameDia( NetworkClass* np_param , QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
    setName( "PropTypeNameDia" );

    np = np_param;

    PropTypeNameDiaLayout = new Q3GridLayout( this, 1, 1, 11, 15);

    prop_type_buttonGroup = new Q3ButtonGroup(this);
    prop_type_buttonGroup->setColumnLayout(0, Qt::Vertical );
    prop_type_buttonGroup->layout()->setSpacing( 15 );
    prop_type_buttonGroup->layout()->setMargin( 11 );
    prop_type_buttonGroupLayout = new Q3VBoxLayout( prop_type_buttonGroup->layout() );
    prop_type_buttonGroupLayout->setAlignment( Qt::AlignTop );

    expo_radioButton = new QRadioButton( prop_type_buttonGroup, "expo_radioButton" );
    expo_radioButton->setChecked( TRUE );
    prop_type_buttonGroupLayout->addWidget( expo_radioButton );

    seg_radioButton = new QRadioButton( prop_type_buttonGroup, "seg_radioButton" );
    prop_type_buttonGroupLayout->addWidget( seg_radioButton );

#if HAS_CLUTTER
    clt_radioButton = new QRadioButton( prop_type_buttonGroup, "clt_radioButton" );
    prop_type_buttonGroupLayout->addWidget( clt_radioButton );
    clt_radioButton->setEnabled(false);
#endif

    PropTypeNameDiaLayout->addMultiCellWidget( prop_type_buttonGroup, 0, 0, 0, 4 );

    //------------------------------------------------------------------------------
    ok_pushButton = new QPushButton(this);
    PropTypeNameDiaLayout->addWidget( ok_pushButton, 3, 1 );

    cancel_pushButton = new QPushButton( this);
    PropTypeNameDiaLayout->addWidget( cancel_pushButton, 3, 3 );

    //------------------------------------------------------------------------------
    name_layout = new Q3HBoxLayout( 0, 0, 6, "name_layout"); 

       name_textLabel = new QLabel( this, "name_textLabel" );
    name_layout->addWidget( name_textLabel );

    name_lineEdit = new QLineEdit( this, "name_lineEdit" );
    name_layout->addWidget( name_lineEdit );

    PropTypeNameDiaLayout->addMultiCellLayout( name_layout, 1, 1, 0, 4 );

    languageChange();
    resize( QSize(471, 197).expandedTo(minimumSizeHint()) );

    set_default_name();

    //slot and signal connection
    connect(prop_type_buttonGroup, SIGNAL(clicked(int)), this,  SLOT(prop_type_btngroup_clicked(int)));
// xxxxxxx    connect(ok_pushButton,         SIGNAL(clicked()),    this,  SLOT(prop_ok_btn_clicked()));
    connect(cancel_pushButton,     SIGNAL(clicked()),    this,  SLOT(prop_cancel_btn_clicked()));

    // tab order
    setTabOrder( expo_radioButton, name_lineEdit );
    setTabOrder( name_lineEdit,    ok_pushButton );
    setTabOrder( ok_pushButton,    cancel_pushButton );
}

/*
 *  Destroys the object and frees any allocated resources
 */
PropTypeNameDia::~PropTypeNameDia()
{

}

void PropTypeNameDia::languageChange()
{
    setCaption( tr( "Create New Propagation Model" ) );
    prop_type_buttonGroup->setTitle( tr( "Set Propagation Type" ) );
    expo_radioButton->setText( tr( "Exponential Propagation Model" ) );
    seg_radioButton->setText( tr( "Segment Propagation Model" ) );
#if HAS_CLUTTER
    clt_radioButton->setText( tr( "Clutter Propagation Model" ) );
#endif
    cancel_pushButton->setText( tr( "&Cancel" ) );
    cancel_pushButton->setAccel( QKeySequence( "Alt+C" ) );
    ok_pushButton->setText( tr( "&Ok" ) );
    ok_pushButton->setAccel( QKeySequence( "Alt+O" ) );
    name_textLabel->setText( tr( "Propagation Model Name" ) );
    name_lineEdit->setText( "expo_prop_mod_0" );
}

void PropTypeNameDia::set_default_name()
{
    int prop_idx      = 0;
    int type_prop_num = 0;
    char *str;

    if ( expo_radioButton->isChecked() ) { 
        for( prop_idx=0; prop_idx<np->num_prop_model; prop_idx++) {
            str = np->prop_model_list[prop_idx]->get_strid();
            if ( strncmp( str, "Exponential_Model", 17 ) == 0 ) {
                type_prop_num ++;
            }
        }
        name_lineEdit->setText(QString("Exponential_Model_%1").arg(type_prop_num));
    }

    if ( seg_radioButton->isChecked() ) {
        for( prop_idx=0; prop_idx<np->num_prop_model; prop_idx++) {
            str = np->prop_model_list[prop_idx]->get_strid();
            if ( strncmp(str, "Segment_Model", 13 ) == 0 ) {
                type_prop_num ++;
            }
        }
        name_lineEdit->setText(QString("Segment_Model_%1").arg(type_prop_num));
    }

#if HAS_CLUTTER
    if ( clt_radioButton->isChecked() ) {
        for( prop_idx=0; prop_idx<np->num_prop_model; prop_idx++) {
            str = np->prop_model_list[prop_idx]->get_strid();
            if ( strncmp(str, "Clutter_Model", 13 ) == 0 ) {
                type_prop_num ++;
            }
        }
        name_lineEdit->setText(QString("Clutter_Model_%1").arg(type_prop_num));
    }
#endif
}

#if 0
// void PropTypeNameDia::prop_ok_btn_clicked()
// {
//     hide();
//     prop_name = name_lineEdit->text();
// 
//     if ( expo_radioButton->isChecked() ) { 
//         prop_type = CConst::PropExpo;
//     } else if ( seg_radioButton->isChecked() ) {
//         prop_type = CConst::PropSegment;
// #if HAS_CLUTTER
//     } else if ( clt_radioButton->isChecked() ) {
//         prop_type = CConst::PropClutterWtExpoSlope;
// #endif
//     } else {
//         CORE_DUMP;
//     }
// }
#endif

void PropTypeNameDia::prop_cancel_btn_clicked()
{
    delete this;
}

void PropTypeNameDia::prop_type_btngroup_clicked(int i)
{
    set_default_name();
}
