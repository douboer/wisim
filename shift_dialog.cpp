
/******************************************************************************************
**** PROGRAM: shift_dialog.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include "shift_dialog.h"
#include "gconst.h"
#include "map_layer.h"
#include "icons_test.h"
#include "list.h"

#include <qvariant.h>
#include <q3buttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <iostream>

ShiftDialog::ShiftDialog( NetworkClass *np_param, int rtti_val, QWidget* parent )
    : QDialog( parent, 0, true)

{
    np            = np_param;
    rtti_value    = rtti_val;
    map_layer_idx = 0;
    int i;


    setName( "ShiftDialog" );
    ShiftDialogLayout = new Q3GridLayout( this, 1, 1, 15, 15, "ShiftDialogLayout"); 


    //-------------------------------------------------------------------------------
    adjust_buttonGroup = new Q3ButtonGroup( this, "adjust_buttonGroup" );
    adjust_buttonGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, adjust_buttonGroup->sizePolicy().hasHeightForWidth() ) );
    adjust_buttonGroup->setMinimumSize( QSize( 0, 40 ) );
    adjust_buttonGroup->setColumnLayout(0, Qt::Vertical );
    adjust_buttonGroup->layout()->setSpacing( 6 );
    adjust_buttonGroup->layout()->setMargin( 11 );
    adjust_buttonGroupLayout = new Q3GridLayout( adjust_buttonGroup->layout() );
    adjust_buttonGroupLayout->setAlignment( Qt::AlignTop );

    left_pushButton = new QPushButton( adjust_buttonGroup, "left_pushButton" );
    left_pushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, left_pushButton->sizePolicy().hasHeightForWidth() ) );
    left_pushButton->setMinimumSize( QSize( 0, 35 ) );

    adjust_buttonGroupLayout->addWidget( left_pushButton, 1, 0 );

    right_pushButton = new QPushButton( adjust_buttonGroup, "right_pushButton" );
    right_pushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, right_pushButton->sizePolicy().hasHeightForWidth() ) );
    right_pushButton->setMinimumSize( QSize( 0, 35 ) );

    adjust_buttonGroupLayout->addWidget( right_pushButton, 1, 6 );
    spacer6 = new QSpacerItem( 60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer6, 1, 1, 1, 2 );
    spacer7 = new QSpacerItem( 50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer7, 1, 1, 4, 5 );
    spacer5 = new QSpacerItem( 20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding );
    adjust_buttonGroupLayout->addItem( spacer5, 1, 3 );

    down_pushButton = new QPushButton( adjust_buttonGroup, "down_pushButton" );
    down_pushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, down_pushButton->sizePolicy().hasHeightForWidth() ) );
    down_pushButton->setMinimumSize( QSize( 0, 40 ) );
    down_pushButton->setMaximumSize( QSize( 80, 32767 ) );

    adjust_buttonGroupLayout->addMultiCellWidget( down_pushButton, 2, 2, 2, 4 );
    spacer8 = new QSpacerItem( 130, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer8, 0, 0, 0, 1 );

    up_pushButton = new QPushButton( adjust_buttonGroup, "up_pushButton" );
    up_pushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, up_pushButton->sizePolicy().hasHeightForWidth() ) );
    up_pushButton->setMinimumSize( QSize( 0, 40 ) );
    up_pushButton->setMaximumSize( QSize( 80, 32767 ) );

    adjust_buttonGroupLayout->addMultiCellWidget( up_pushButton, 0, 0, 2, 4 );
    spacer9 = new QSpacerItem( 130, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer9, 0, 0, 5, 6 );
    spacer10 = new QSpacerItem( 130, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer10, 2, 2, 0, 1 );
    spacer11 = new QSpacerItem( 130, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    adjust_buttonGroupLayout->addMultiCell( spacer11, 2, 2, 5, 6 );


    //----------
#if 0 
    up_pushButton->setDefault( false );
    down_pushButton->setDefault( false );
    left_pushButton->setDefault( false );
    right_pushButton->setDefault( false );
#endif

    up_pushButton->setAutoDefault( false );
    down_pushButton->setAutoDefault( false );
    left_pushButton->setAutoDefault( false );
    right_pushButton->setAutoDefault( false );

    up_pushButton->setFlat( true );
    down_pushButton->setFlat( true );
    left_pushButton->setFlat( true );
    right_pushButton->setFlat( true );

    up_pushButton->setIconSet( QIcon( TestIcon::icon_up ) );
    down_pushButton->setIconSet( QIcon( TestIcon::icon_down ) );
    left_pushButton->setIconSet( QIcon( TestIcon::icon_left ) );
    right_pushButton->setIconSet( QIcon( TestIcon::icon_right ) );
    //----------
    
    //-------------------------------------------------------------------------------

    spacer4 = new QSpacerItem( 20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding );

    layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 
    QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer );

    ok_button = new QPushButton( this, "ok_button" );
    layout1->addWidget( ok_button );
    QSpacerItem* spacer_2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer_2 );

    cancel_button = new QPushButton( this, "cancel_button" );
    layout1->addWidget( cancel_button );
    QSpacerItem* spacer_3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer_3 );


    switch(rtti_val) {
        case GConst::mapBackgroundRTTI:
            printf("Shifting Map Background\n");

            ShiftDialogLayout->addWidget( adjust_buttonGroup, 0, 0 );
            ShiftDialogLayout->addItem( spacer4, 1, 0 );
            ShiftDialogLayout->addLayout( layout1, 2, 0 );

            resize( QSize(300, 250).expandedTo(minimumSizeHint()) );
            break;
        case GConst::roadTestDataRTTI:
            printf("Shifting Road Test Data\n");

            ShiftDialogLayout->addWidget( adjust_buttonGroup, 0, 0 );
            ShiftDialogLayout->addItem( spacer4, 1, 0 );
            ShiftDialogLayout->addLayout( layout1, 2, 0 );

            resize( QSize(300, 250).expandedTo(minimumSizeHint()) );
            break;
        case GConst::mapLayerRTTI:
            printf("Shifting Map Layer\n");

            mapLayerLabel = new QLabel( this, "mapLayerLabel" );
            mapLayerLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 5, 0,
            mapLayerLabel->sizePolicy().hasHeightForWidth() ) );
            
            mapLayerComboBox = new QComboBox( FALSE, this, "mapLayerComboBox" );
            mapLayerComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 8, 0, mapLayerComboBox->sizePolicy().hasHeightForWidth() ) );

            for(i = 0; i<np->map_layer_list->getSize(); i++) {
                mapLayerComboBox->insertItem( (*(np->map_layer_list))[i]->name, i );
            }

            ShiftDialogLayout->addWidget( mapLayerLabel, 0, 0 );
            ShiftDialogLayout->addWidget( mapLayerComboBox, 0, 1 );
            ShiftDialogLayout->addMultiCellWidget( adjust_buttonGroup, 1, 1, 0, 1 );
            ShiftDialogLayout->addMultiCell( spacer4, 2, 2,  0, 1 );
            ShiftDialogLayout->addMultiCellLayout( layout1, 3, 3, 0, 1 );

            resize( QSize(300, 280).expandedTo(minimumSizeHint()) );

            connect( mapLayerComboBox, SIGNAL( activated(int)), this, SLOT( mapLayerComboBox_select(int) ));
            break;
        default:
            printf("RTTI = %d unrecognized\n", rtti_val);
            break;
    }

    
    languageChange();

    /////////connect////////////
    connect(ok_button, SIGNAL(clicked()), this, SLOT(ok_btn_clicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(cancel_btn_clicked()));
    connect(left_pushButton, SIGNAL(clicked()), this, SLOT(left_btn_clicked()));
    connect(right_pushButton, SIGNAL(clicked()), this, SLOT(right_btn_clicked()));
    connect(up_pushButton, SIGNAL(clicked()), this, SLOT(up_btn_clicked()));
    connect(down_pushButton, SIGNAL(clicked()), this, SLOT(down_btn_clicked()));

    /////////end connect////////
    

    exec();
}

ShiftDialog::~ShiftDialog()
{

}


void ShiftDialog::mapLayerComboBox_select( int i )
{
    map_layer_idx = i;
}

void ShiftDialog::left_btn_clicked()
{
    switch(rtti_value) {
        case GConst::mapBackgroundRTTI:
            sprintf(np->line_buf, "shift_map_background -x -10 -y 0");
            np->process_command(np->line_buf);
            break;
        case GConst::roadTestDataRTTI:
            sprintf(np->line_buf, "shift_road_test_data -x -10 -y 0");
            np->process_command(np->line_buf);
            break;
        case GConst::mapLayerRTTI:
            sprintf(np->line_buf, "shift_map_layer -map_layer_idx %d -x -10 -y 0", map_layer_idx);
            np->process_command(np->line_buf);
            break;
        default:
            break;
    }
}

void ShiftDialog::right_btn_clicked()
{
    switch(rtti_value) {
        case GConst::mapBackgroundRTTI:
            sprintf(np->line_buf, "shift_map_background -x +10 -y 0");
            np->process_command(np->line_buf);
            break;
        case GConst::roadTestDataRTTI:
            sprintf(np->line_buf, "shift_road_test_data -x +10 -y 0");
            np->process_command(np->line_buf);
            break;
        case GConst::mapLayerRTTI:
            sprintf(np->line_buf, "shift_map_layer -map_layer_idx %d -x +10 -y 0", map_layer_idx);
            np->process_command(np->line_buf);
            break;
        default:
            break;
    }
}

void ShiftDialog::up_btn_clicked()
{
    switch(rtti_value) {
        case GConst::mapBackgroundRTTI:
            sprintf(np->line_buf, "shift_map_background -x 0 -y +10");
            np->process_command(np->line_buf);
            break;
        case GConst::roadTestDataRTTI:
            sprintf(np->line_buf, "shift_road_test_data -x 0 -y +10");
            np->process_command(np->line_buf);
            break;
        case GConst::mapLayerRTTI:
            sprintf(np->line_buf, "shift_map_layer -map_layer_idx %d -x 0 -y +10", map_layer_idx);
            np->process_command(np->line_buf);
            break;
        default:
            break;
    }

}

void ShiftDialog::down_btn_clicked()
{
    switch(rtti_value) {
        case GConst::mapBackgroundRTTI:
            sprintf(np->line_buf, "shift_map_background -x 0 -y -10");
            np->process_command(np->line_buf);
            break;
        case GConst::roadTestDataRTTI:
            sprintf(np->line_buf, "shift_road_test_data -x 0 -y -10");
            np->process_command(np->line_buf);
            break;
        case GConst::mapLayerRTTI:
            sprintf(np->line_buf, "shift_map_layer -map_layer_idx %d -x 0 -y -10", map_layer_idx);
            np->process_command(np->line_buf);
            break;
        default:
            break;
    }
}

void ShiftDialog::ok_btn_clicked()
{
    hide();
    delete this;
}

void ShiftDialog::cancel_btn_clicked()
{
    hide();
    delete this;
}

void ShiftDialog::languageChange()
{
    switch(rtti_value) {
        case GConst::mapBackgroundRTTI:
            setCaption( tr( "Shift Background Map" ) );
            adjust_buttonGroup->setTitle( tr( "Shift Background Map" ) );
            break;
        case GConst::roadTestDataRTTI:
            setCaption( tr( "Shift Road Test Data" ) );
            adjust_buttonGroup->setTitle( tr( "Shift Road Test Data" ) );
            break;
        case GConst::mapLayerRTTI:
            setCaption( tr( "Shift Map Layer" ) );
            mapLayerLabel->setText( tr("Map Layer") );
            adjust_buttonGroup->setTitle( tr( "Shift Map Layer" ) );
            break;
        default:
            break;
    }
#if 0
    left_pushButton->setText( tr( "&Left" ) );
    right_pushButton->setText( tr( "&Right" ) );
    down_pushButton->setText( tr( "&Down" ) );
    up_pushButton->setText( tr( "&Up" ) );
#endif
    ok_button->setText( tr( "&Ok" ) );
    cancel_button->setText( tr( "&Cancel" ) );
}

