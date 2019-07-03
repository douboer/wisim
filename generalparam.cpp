
/******************************************************************************************
**** PROGRAM: generalparam.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#include <qvariant.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "generalparam.h"
#include "WiSim.h"
#include "cconst.h"
#include "phs.h"


generalParam::generalParam( NetworkClass* np_param, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    np = np_param;
    
    if ( !name )
	setName( "generalParam" );
    generalParamLayout = new Q3GridLayout( this, 1, 1, 11, 6, "generalParamLayout"); 

    //-----------------------------------------------------------------------------------------------------
    buttonGp = new Q3ButtonGroup( this, "buttonGp" );
    buttonGp->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 6, buttonGp->sizePolicy().hasHeightForWidth() ) );
    buttonGp->setColumnLayout(0, Qt::Vertical );
    buttonGp->layout()->setSpacing( 11 );
    buttonGp->layout()->setMargin( 11 );
    buttonGpLayout = new Q3GridLayout( buttonGp->layout() );
    buttonGpLayout->setAlignment( Qt::AlignTop );

    num_freq_lbl = new QLabel( buttonGp, "num_freq_lbl" );
    buttonGpLayout->addWidget( num_freq_lbl, 0, 0 );
    spacer5 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGpLayout->addItem( spacer5, 0, 1 );
    num_freq_val = new QLineEdit( buttonGp, "num_freq_val" );
    num_freq_val->setMaximumSize( QSize( 120, 32767 ) );
    buttonGpLayout->addWidget( num_freq_val, 0, 2 );

    num_tch_time_slot_lbl = new QLabel( buttonGp, "num_tch_time_slot_lbl" );
    buttonGpLayout->addWidget( num_tch_time_slot_lbl, 1, 0 );
    spacer6 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGpLayout->addItem( spacer6, 1, 1 );
    num_tch_time_slot_val = new QLineEdit( buttonGp, "num_tch_time_slot_val" );
    num_tch_time_slot_val->setMaximumSize( QSize( 120, 32767 ) );
    buttonGpLayout->addWidget( num_tch_time_slot_val, 1, 2 );

    num_cch_time_slot_lbl = new QLabel( buttonGp, "num_cch_time_slot_lbl" );
    buttonGpLayout->addWidget( num_cch_time_slot_lbl, 2, 0 );
    spacer7 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGpLayout->addItem( spacer7, 2, 1 );
    num_cch_time_slot_val = new QLineEdit( buttonGp, "num_cch_time_slot_val" );
    num_cch_time_slot_val->setMaximumSize( QSize( 120, 32767 ) );
    buttonGpLayout->addWidget( num_cch_time_slot_val, 2, 2 );

    cch_freq_lbl = new QLabel( buttonGp, "cch_freq_lbl" );
    buttonGpLayout->addWidget( cch_freq_lbl, 3, 0 );
    spacer8 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGpLayout->addItem( spacer8, 3, 1 );
    cch_freq_val = new QLineEdit( buttonGp, "cch_freq_val" );
    cch_freq_val->setMaximumSize( QSize( 120, 32767 ) );
    buttonGpLayout->addWidget( cch_freq_val, 3, 2 );

    generalParamLayout->addWidget( buttonGp, 1, 0 );

    //-----------------------------------------------------------------------------------------------------
    buttonGroup2 = new Q3ButtonGroup( this, "buttonGroup2" );
    buttonGroup2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 6, buttonGroup2->sizePolicy().hasHeightForWidth() ) );
    buttonGroup2->setColumnLayout(0, Qt::Vertical );
    buttonGroup2->layout()->setSpacing( 11 );
    buttonGroup2->layout()->setMargin( 11 );
    buttonGroup2Layout = new Q3GridLayout( buttonGroup2->layout() );
    buttonGroup2Layout->setAlignment( Qt::AlignTop );

    ac_hide_timer_lbl = new QLabel( buttonGroup2, "ac_hide_timer_lbl" );
    buttonGroup2Layout->addWidget( ac_hide_timer_lbl, 1, 0 );

    ac_hide_timer_val = new QLineEdit( buttonGroup2, "ac_hide_timer_val" );
    ac_hide_timer_val->setMaximumSize( QSize( 120, 32767 ) );

    buttonGroup2Layout->addWidget( ac_hide_timer_val, 1, 2 );

    ac_hide_thr_spinbox = new QSpinBox( buttonGroup2, "ac_hide_thr_spinbox" );
    ac_hide_thr_spinbox->setMaximumSize( QSize( 120, 32767 ) );
    ac_hide_thr_spinbox->setMinValue( 0 );
    ac_hide_thr_spinbox->setMaxValue( 1000 );

    buttonGroup2Layout->addWidget( ac_hide_thr_spinbox, 0, 2 );

    ac_use_thr_spinbox = new QSpinBox( buttonGroup2, "ac_use_thr_spinbox" );
    ac_use_thr_spinbox->setMaximumSize( QSize( 120, 32767 ) );
    ac_use_thr_spinbox->setMinValue( 0 );
    ac_use_thr_spinbox->setMaxValue( 1000 );

    buttonGroup2Layout->addWidget( ac_use_thr_spinbox, 2, 2 );
    ac_use_timer_lbl = new QLabel( buttonGroup2, "ac_use_timer_lbl" );

    buttonGroup2Layout->addWidget( ac_use_timer_lbl, 3, 0 );

    ac_use_timer_val = new QLineEdit( buttonGroup2, "ac_use_timer_val" );
    ac_use_timer_val->setMaximumSize( QSize( 120, 32767 ) );

    buttonGroup2Layout->addWidget( ac_use_timer_val, 3, 2 );

    ac_use_thr_lbl = new QLabel( buttonGroup2, "ac_use_thr_lbl" );
    buttonGroup2Layout->addWidget( ac_use_thr_lbl, 2, 0 );

    ac_hide_thr_lbl = new QLabel( buttonGroup2, "ac_hide_thr_lbl" );
    buttonGroup2Layout->addWidget( ac_hide_thr_lbl, 0, 0 );
    
    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGroup2Layout->addItem( spacer1, 1, 1 );

    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGroup2Layout->addItem( spacer2, 3, 1 );

    generalParamLayout->addWidget( buttonGroup2, 3, 0 );

    //-----------------------------------------------------------------------------------------------------
    buttonGroup3 = new Q3ButtonGroup( this, "buttonGroup3" );
    buttonGroup3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 4, buttonGroup3->sizePolicy().hasHeightForWidth() ) );
    buttonGroup3->setColumnLayout(0, Qt::Vertical );
    buttonGroup3->layout()->setSpacing( 11 );
    buttonGroup3->layout()->setMargin( 11 );
    buttonGroup3Layout = new Q3GridLayout( buttonGroup3->layout() );
    buttonGroup3Layout->setAlignment( Qt::AlignTop );

    sync_level_allocation_threshold_db_lbl = new QLabel( buttonGroup3, "sync_level_allocation_threshold_db_lbl" );
    buttonGroup3Layout->addWidget( sync_level_allocation_threshold_db_lbl, 1, 0 );

    cch_allocation_threshold_db_lbl = new QLabel( buttonGroup3, "cch_allocation_threshold_db_lbl" );
    buttonGroup3Layout->addWidget( cch_allocation_threshold_db_lbl, 0, 0 );

    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGroup3Layout->addItem( spacer3, 0, 1 );

    spacer4 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonGroup3Layout->addItem( spacer4, 1, 1 );

    cch_allocation_threshold_db_val = new QLineEdit( buttonGroup3, "cch_allocation_threshold_db_val" );
    cch_allocation_threshold_db_val->setMaximumSize( QSize( 140, 32767 ) );
    buttonGroup3Layout->addWidget( cch_allocation_threshold_db_val, 0, 2 );

    generalParamLayout->addWidget( buttonGroup3, 2, 0 );

    sync_level_allocation_threshold_db_val = new QLineEdit( buttonGroup3, "sync_level_allocation_threshold_db_val" );
    sync_level_allocation_threshold_db_val->setMaximumSize( QSize( 140, 32767 ) );
    buttonGroup3Layout->addWidget( sync_level_allocation_threshold_db_val, 1, 2 );

    cch_allocation_threshold_db_val->setEnabled(false);
    sync_level_allocation_threshold_db_val->setEnabled(false);
    cch_allocation_threshold_db_lbl->setEnabled(false);
    sync_level_allocation_threshold_db_lbl->setEnabled(false);

    //-----------------------------------------------------------------------------------------------------
    csid_fmt_buttonGrp = new Q3ButtonGroup( this, "csid_fmt_buttonGrp" );
    csid_fmt_buttonGrp->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 3, csid_fmt_buttonGrp->sizePolicy().hasHeightForWidth() ) );
    csid_fmt_buttonGrp->setColumnLayout(0, Qt::Vertical );
    csid_fmt_buttonGrp->layout()->setSpacing( 11 );
    csid_fmt_buttonGrp->layout()->setMargin( 11 );
    csid_fmt_buttonGrpLayout = new Q3GridLayout( csid_fmt_buttonGrp->layout() );
    csid_fmt_buttonGrpLayout->setAlignment( Qt::AlignTop );

    csid_fmt_lbl = new QLabel( csid_fmt_buttonGrp, "csid_fmt_lbl" );
    csid_fmt_buttonGrpLayout->addWidget( csid_fmt_lbl, 0, 0 );

    csid_fmt_combobox = new QComboBox( FALSE, csid_fmt_buttonGrp, "csid_fmt_combobox" );
    csid_fmt_combobox->setMaximumSize( QSize( 120, 32767 ) );
    csid_fmt_combobox->insertItem(tr("19 bits PA length"), 0);
    csid_fmt_combobox->insertItem(tr("16 bits PA length"), 1);

    csid_fmt_buttonGrpLayout->addWidget( csid_fmt_combobox, 0, 1 );
    generalParamLayout->addWidget( csid_fmt_buttonGrp, 4, 0 );

    //initial
    //read param and write them to gui
    if ( np->technology() == CConst::PHS ) {

        QString str;

        str.sprintf( "%d",  ((PHSNetworkClass*) np)->num_freq);
        num_freq_val->setText ( str ); 

        str.sprintf( "%d",  ((PHSNetworkClass*) np)->num_slot);
        num_tch_time_slot_val->setText ( str ); 

        str.sprintf( "%d",  ((PHSNetworkClass*) np)->num_cntl_chan_slot);
        num_cch_time_slot_val->setText ( str ); 

        str.sprintf( "%d",  ((PHSNetworkClass*) np)->cntl_chan_freq);
        cch_freq_val->setText ( str ); 
        
        ac_hide_thr_spinbox->setValue( ((PHSNetworkClass*) np)->ac_hide_thr );
        ac_use_thr_spinbox->setValue( ((PHSNetworkClass*) np)->ac_use_thr );
        
        str.sprintf( "%f",  ((PHSNetworkClass*) np)->ac_hide_timer );
        ac_hide_timer_val->setText( str );
        
        str.sprintf( "%f",  ((PHSNetworkClass*) np)->ac_use_timer );
        ac_use_timer_val->setText( str );

        str.sprintf( "%f",  ((NetworkClass*) np)->cch_allocation_threshold_db );
        cch_allocation_threshold_db_val->setText( str );

        str.sprintf( "%f",  ((NetworkClass*) np)->sync_level_allocation_threshold_db );
        sync_level_allocation_threshold_db_val->setText( str );

        if ( ((PHSNetworkClass*) np)->csid_format == CConst::CSID16NP ) {
            csid_fmt_combobox->setCurrentItem( 1 );
        } else {
            csid_fmt_combobox->setCurrentItem( 0 );
        }
    }
//        cch_allocation_threshold_db;
//        sync_level_allocation_threshold_db;

    languageChange();
    resize( QSize(505, 326).expandedTo(minimumSizeHint()) );

    //singal and slot
    connect( ac_hide_thr_spinbox, SIGNAL( valueChanged( int ) ), this, SLOT( ac_hide_thr_valchanged( int ) ) ); 
    connect( ac_use_thr_spinbox, SIGNAL( valueChanged( int ) ), this, SLOT( ac_use_thr_valchanged( int ) ) ); 
    connect( csid_fmt_combobox, SIGNAL( activated( int ) ), this, SLOT( csid_fmt_select( int ) ) );

    // tab order
    setTabOrder( num_freq_val, num_tch_time_slot_val);
    setTabOrder( num_tch_time_slot_val, num_cch_time_slot_val);
    setTabOrder( num_cch_time_slot_val, cch_freq_val);
    setTabOrder( cch_freq_val, cch_allocation_threshold_db_val );
    setTabOrder( cch_allocation_threshold_db_val, sync_level_allocation_threshold_db_val );
    setTabOrder( sync_level_allocation_threshold_db_val, ac_hide_thr_spinbox);
    setTabOrder( ac_hide_thr_spinbox, ac_hide_timer_val );
    setTabOrder( ac_hide_timer_val, ac_use_thr_spinbox );
    setTabOrder( ac_use_thr_spinbox, ac_use_timer_val );
}

/*
 */
generalParam::~generalParam()
{

}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void generalParam::languageChange()
{
    setCaption( tr( "General Parameters" ) );

    buttonGroup2->setTitle( QString::null );
    buttonGroup3->setTitle( QString::null );

    num_freq_lbl->setText( tr("Number of Frequencies") );
    num_tch_time_slot_lbl->setText( tr("Number of TCH Time Slots") );
    num_cch_time_slot_lbl->setText( tr("Number of CCH Time Slots") );
    cch_freq_lbl->setText( tr("CCH Frequencies") );

    ac_hide_timer_lbl->setText( tr( "Access Hide Timer" ) );
    ac_use_timer_lbl->setText( tr( "Access Use Timer" ) );
    ac_use_thr_lbl->setText( tr( "Access Use Threshold" ) );
    ac_hide_thr_lbl->setText( tr( "Access Hide Threshold" ) );

    sync_level_allocation_threshold_db_lbl->setText( tr( "Min RSSI Level to Allow Sync Level Assignment" ) );
    cch_allocation_threshold_db_lbl->setText( tr( "Max RSSI Level for CCH Allocation" ) );
 
    csid_fmt_lbl->setText( tr( "CSID Format" ) );
}

void generalParam::ok_btn_clicked()
{

    if (num_freq_val->edited()) {
        sprintf(np->line_buf, "set -num_freq %s", num_freq_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (num_tch_time_slot_val->edited()) {
        sprintf(np->line_buf, "set -num_slot %s", num_tch_time_slot_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (num_cch_time_slot_val->edited()) {
        sprintf(np->line_buf, "set -num_cntl_chan_slot %s", num_cch_time_slot_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (cch_freq_val->edited()) {
        sprintf(np->line_buf, "set -cntl_chan_freq %s", cch_freq_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (cch_allocation_threshold_db_val->edited()) {
        sprintf(np->line_buf, "set -cch_allocation_threshold_db %s", cch_allocation_threshold_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (sync_level_allocation_threshold_db_val->edited()) {
        sprintf(np->line_buf, "set -sync_level_allocation_threshold_db %s", sync_level_allocation_threshold_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (ac_hide_timer_val->edited()) {
        sprintf(np->line_buf, "set -ac_hide_timer %s", ac_hide_timer_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (ac_use_timer_val->edited()) {
        sprintf(np->line_buf, "set -ac_use_timer %s", ac_use_timer_val->text().latin1());
        np->process_command(np->line_buf);
    }
}


void generalParam::ac_hide_thr_valchanged( int i )
{
    sprintf(np->line_buf, "set -ac_hide_thr %d", i );
    np->process_command(np->line_buf);
}

void generalParam::ac_use_thr_valchanged( int i )
{
    sprintf(np->line_buf, "set -ac_use_thr %d", i); 
    np->process_command(np->line_buf);
}

void generalParam::csid_fmt_select( int i )
{
    sprintf(np->line_buf, "set_csid_format -fmt %d", i);
    np->process_command(np->line_buf);

    sprintf(np->line_buf, "set_color -all_pa");
    np->process_command(np->line_buf);
}
