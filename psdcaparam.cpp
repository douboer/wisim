/****************************************************************************
**  psdcaparam.cpp
****************************************************************************/
#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <iostream>

#include "cconst.h"
#include "psdcaparam.h"
#include "WiSim.h"
#include "phs.h"

psDcaParam::psDcaParam( NetworkClass* np_param, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    np = np_param;

    if ( !name )
    setName( "psDcaParam" );

    psDcaParamLayout = new Q3GridLayout( this, 1, 1, 11, 22, "psDcaParamLayout"); 
    
    /*------------------------*/
    ps_mem_buttonGp = new Q3ButtonGroup( this, "ps_mem_buttonGp" );
    ps_mem_buttonGp->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 3, ps_mem_buttonGp->sizePolicy().hasHeightForWidth() ) );
    ps_mem_buttonGp->setColumnLayout(0, Qt::Vertical );
    ps_mem_buttonGp->layout()->setSpacing( 11 );
    ps_mem_buttonGp->layout()->setMargin( 11 );
    ps_mem_buttonGpLayout = new Q3GridLayout( ps_mem_buttonGp->layout() );
    ps_mem_buttonGpLayout->setAlignment( Qt::AlignTop );

    memoryless_ps_lbl = new QLabel( ps_mem_buttonGp, "memoryless_ps_lbl" );
    ps_mem_buttonGpLayout->addWidget( memoryless_ps_lbl, 0, 0 );

    memoryless_ps_combobox = new QComboBox( FALSE, ps_mem_buttonGp, "memoryless_ps_combobox" );
    memoryless_ps_combobox->setMaximumSize( QSize( 120, 32767 ) );
    memoryless_ps_combobox->insertItem(tr("has memory"),0);
    memoryless_ps_combobox->insertItem(tr("no memory"),1);

    ps_mem_buttonGpLayout->addWidget( memoryless_ps_combobox, 0, 1 );
    psDcaParamLayout->addMultiCellWidget( ps_mem_buttonGp, 0, 0, 0, 1 );
    /*------------------------*/

    sir_threshold_call_request_ps_db_lbl = new QLabel( this, "sir_threshold_call_request_ps_db_lbl" );
    psDcaParamLayout->addWidget( sir_threshold_call_request_ps_db_lbl, 2, 0 );

    sir_threshold_call_request_ps_db_val = new QLineEdit( this, "sir_threshold_call_request_ps_db_val" );
    psDcaParamLayout->addWidget( sir_threshold_call_request_ps_db_val, 2, 1 );

    sir_threshold_call_drop_ps_db_lbl = new QLabel( this, "sir_threshold_call_drop_ps_db_lbl" );
    psDcaParamLayout->addWidget( sir_threshold_call_drop_ps_db_lbl, 3, 0 );

    sir_threshold_call_drop_ps_db_val = new QLineEdit( this, "sir_threshold_call_drop_ps_db_val" );
    psDcaParamLayout->addWidget( sir_threshold_call_drop_ps_db_val, 3, 1 );


    /*------------------------*/
    int_threshold_call_request_ps_db_lbl = new QLabel( this, "int_threshold_call_request_ps_db_lbl" );
    psDcaParamLayout->addWidget( int_threshold_call_request_ps_db_lbl, 4, 0 );

    int_threshold_call_request_ps_db_val = new QLineEdit( this, "int_threshold_call_request_ps_db_val" );
    psDcaParamLayout->addWidget( int_threshold_call_request_ps_db_val, 4, 1 );

    int_threshold_call_drop_ps_db_lbl = new QLabel( this, "int_threshold_call_drop_ps_db_lbl" );
    psDcaParamLayout->addWidget( int_threshold_call_drop_ps_db_lbl, 5, 0 );

    int_threshold_call_drop_ps_db_val = new QLineEdit( this, "int_threshold_call_drop_ps_db_val" );
    psDcaParamLayout->addWidget( int_threshold_call_drop_ps_db_val, 5, 1 );

    /*------------------------*/
    ps_dca_algorithm = new Q3ButtonGroup( this, "ps_dca_algorithm" );
    ps_dca_algorithm->setColumnLayout(0, Qt::Vertical );
    ps_dca_algorithm->layout()->setSpacing( 6 );
    ps_dca_algorithm->layout()->setMargin( 11 );
    ps_dca_algorithmLayout = new Q3GridLayout( ps_dca_algorithm->layout() );
    ps_dca_algorithmLayout->setAlignment( Qt::AlignTop );

    spacer1 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ps_dca_algorithmLayout->addItem( spacer1, 0, 1 );

    ps_sir = new QRadioButton( ps_dca_algorithm, "ps_sir" );
    ps_dca_algorithmLayout->addMultiCellWidget( ps_sir, 1, 1, 0, 1 );

    spacer2 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ps_dca_algorithmLayout->addItem( spacer2, 2, 0 );

    ps_int = new QRadioButton( ps_dca_algorithm, "ps_int" );
    ps_dca_algorithmLayout->addMultiCellWidget( ps_int, 3, 3, 0, 1 );

    spacer3 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ps_dca_algorithmLayout->addItem( spacer3, 4, 0 );

    ps_int_sir = new QRadioButton( ps_dca_algorithm, "ps_int_sir" );
    ps_dca_algorithmLayout->addMultiCellWidget( ps_int_sir, 5, 5, 0, 1 );

    spacer4 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ps_dca_algorithmLayout->addItem( spacer4, 6, 1 );

    psDcaParamLayout->addMultiCellWidget( ps_dca_algorithm, 1, 1, 0, 1 );

    //initialization
    switch (((PHSNetworkClass*) np)->ps_dca_alg) {
        case (CConst::SIRDCA):
            init_ps_dca_alg( 0 );
            break;
        case (CConst::IntDCA):
            init_ps_dca_alg( 1 );
            break;
        case (CConst::SIRIntDCA):
            init_ps_dca_alg( 2 );
            break;
        case (CConst::IntSIRDCA):
            init_ps_dca_alg( 3 );
            break;
    }

    if ( np->memoryless_ps ) {
        memoryless_ps_combobox->setCurrentItem( 1 );
    } else {
        memoryless_ps_combobox->setCurrentItem( 0 );
    }

    //read param and write them to GUI
    if ( np->technology() == CConst::PHS ) {
        QString str;

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->sir_threshold_call_request_ps_db );
        sir_threshold_call_request_ps_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->int_threshold_call_request_ps_db );
        int_threshold_call_request_ps_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->sir_threshold_call_drop_ps_db );
        sir_threshold_call_drop_ps_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->int_threshold_call_drop_ps_db );
        int_threshold_call_drop_ps_db_val->setText( str );
    }

    /*------------------------*/
    languageChange();
    resize( QSize(527, 322).expandedTo(minimumSizeHint()) );

    //signal and slot
    connect( ps_dca_algorithm, SIGNAL( clicked(int ) ), this, SLOT( set_ps_dca_alg( int ) ) );
    connect( memoryless_ps_combobox, SIGNAL( activated( int ) ), this, SLOT( ps_type_select( int ) ) );


    // tab order
    setTabOrder( ps_sir, ps_int );
    setTabOrder( ps_int, ps_int_sir );
    setTabOrder( ps_int_sir, sir_threshold_call_request_ps_db_val );
    setTabOrder( sir_threshold_call_request_ps_db_val, sir_threshold_call_drop_ps_db_val );
    setTabOrder( sir_threshold_call_drop_ps_db_val, int_threshold_call_request_ps_db_val );
    setTabOrder( int_threshold_call_request_ps_db_val, int_threshold_call_drop_ps_db_val );
}


psDcaParam::~psDcaParam()
{
    // no need to delete child widgets, Qt does it all for us
}


void psDcaParam::languageChange()
{
    setCaption( tr( "PS DCA Parameters" ) );

    memoryless_ps_lbl->setText( tr( "PS memoryless" ) );
    sir_threshold_call_request_ps_db_lbl->setText( tr( "SIR threshold at a PS for a call request" ) );
    sir_threshold_call_drop_ps_db_lbl->setText( tr( "SIR threshold at a PS for a call drop" ) );
    int_threshold_call_request_ps_db_lbl->setText( tr( "INT threshold at a PS for a call request" ) );
    int_threshold_call_drop_ps_db_lbl->setText( tr( "INT threshold at a PS for a call drop" ) );
    ps_dca_algorithm->setTitle( tr( "PS DCA algorithm" ) );
    ps_int_sir->setText( tr( "int_sir" ) );
    ps_sir->setText( tr( "SIR(signal-to-interference ratio)" ) );
    ps_int->setText( tr( "INT(interference)" ) );
}


void psDcaParam::ok_btn_clicked()
{
    if (sir_threshold_call_request_ps_db_val->edited()) {
        sprintf(np->line_buf, "set -sir_threshold_call_request_ps_db %s", sir_threshold_call_request_ps_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (sir_threshold_call_drop_ps_db_val->edited()) {
        sprintf(np->line_buf, "set -sir_threshold_call_drop_ps_db %s", sir_threshold_call_drop_ps_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (int_threshold_call_request_ps_db_val->edited()) {
        sprintf(np->line_buf, "set -int_threshold_call_request_ps_db %s", int_threshold_call_request_ps_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (int_threshold_call_drop_ps_db_val->edited()) {
        sprintf(np->line_buf, "set -int_threshold_call_drop_ps_db %s", int_threshold_call_drop_ps_db_val->text().latin1());
        np->process_command(np->line_buf);
    }
}

#if CGDEBUG
void psDcaParam::cancel_btn_clicked()
{

}
#endif

void psDcaParam::set_ps_dca_alg( int i )
{

    //qDebug("psDcaParam::set_ps_dca_alg\n");
    switch (i)
    {
        case 0:
            sir_threshold_call_request_ps_db_val->setDisabled(false);
            int_threshold_call_request_ps_db_val->setDisabled(true);
            sir_threshold_call_drop_ps_db_val->setDisabled(false);
            int_threshold_call_drop_ps_db_val->setDisabled(true);

            sprintf(np->line_buf,"set -ps_dca_alg sir");
            np->process_command(np->line_buf);

            break;
        case 1:
            sir_threshold_call_request_ps_db_val->setDisabled(true);
            int_threshold_call_request_ps_db_val->setDisabled(false);
            sir_threshold_call_drop_ps_db_val->setDisabled(true);
            int_threshold_call_drop_ps_db_val->setDisabled(false);

            sprintf(np->line_buf,"set -ps_dca_alg int");
            np->process_command(np->line_buf);

            break;
        case 2:
            sir_threshold_call_request_ps_db_val->setDisabled(false);
            int_threshold_call_request_ps_db_val->setDisabled(false);
            sir_threshold_call_drop_ps_db_val->setDisabled(false);
            int_threshold_call_drop_ps_db_val->setDisabled(false);
            
            sprintf(np->line_buf,"set -ps_dca_alg int_sir");
            np->process_command(np->line_buf);
            break;
        }
}

/*
 * Initialize the button group
 */
void psDcaParam::init_ps_dca_alg( int i )
{
    //qDebug("psDcaParam::set_ps_dca_alg\n");
    switch (i)
    {
        // INT algorithm
        case 0:
            sir_threshold_call_request_ps_db_val->setDisabled(false);
            int_threshold_call_request_ps_db_val->setDisabled(true);
            sir_threshold_call_drop_ps_db_val->setDisabled(false);
            int_threshold_call_drop_ps_db_val->setDisabled(true);

            ps_sir->setChecked( true );
            ps_int->setChecked( false );
            ps_int_sir->setChecked( false );

            break;
        // SIR algorithm
        case 1:
            sir_threshold_call_request_ps_db_val->setDisabled(true);
            int_threshold_call_request_ps_db_val->setDisabled(false);
            sir_threshold_call_drop_ps_db_val->setDisabled(true);
            int_threshold_call_drop_ps_db_val->setDisabled(false);

            ps_int->setChecked( true );
            ps_sir->setChecked( false );
            ps_int_sir->setChecked( false );

            break;
        // SIR-INT algorithm
        case 2:
            sir_threshold_call_request_ps_db_val->setDisabled(false);
            int_threshold_call_request_ps_db_val->setDisabled(false);
            sir_threshold_call_drop_ps_db_val->setDisabled(false);
            int_threshold_call_drop_ps_db_val->setDisabled(false);

            ps_int_sir->setChecked( true );
            ps_sir->setChecked( false );
            ps_int->setChecked( false );

            break;
        // INT-SIR algorithm
        case 3:
            sir_threshold_call_request_ps_db_val->setDisabled(false);
            int_threshold_call_request_ps_db_val->setDisabled(false);
            sir_threshold_call_drop_ps_db_val->setDisabled(false);
            int_threshold_call_drop_ps_db_val->setDisabled(false);

            ps_int_sir->setChecked( true );
            ps_sir->setChecked( false );
            ps_int->setChecked( false );

            break;
    }
}

void psDcaParam::ps_type_select( int i )
{
    switch (i) {
        case 0:
            sprintf(np->line_buf, "set -memoryless_ps %s", "0");
            np->process_command(np->line_buf);
            //memoryless_ps_combobox->setCurrentItem(0);
            break;
        case 1:
            sprintf(np->line_buf, "set -memoryless_ps %s", "1");
            np->process_command(np->line_buf);
            //memoryless_ps_combobox->setCurrentItem(1);
            break;
        }
}
