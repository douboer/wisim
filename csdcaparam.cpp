#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "cconst.h"
#include "csdcaparam.h"
#include "wisim.h"
#include "phs.h"

csDcaParam::csDcaParam( NetworkClass *np_param, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    np = np_param;
    
    if ( !name )
	setName( "csDcaParam" );

    csDcaParamLayout = new Q3GridLayout( this, 1, 1, 11, 22, "csDcaParamLayout"); 

    sir_threshold_call_request_cs_db_lbl = new QLabel( this, "sir_threshold_call_request_cs_db_lbl" );
    csDcaParamLayout->addWidget( sir_threshold_call_request_cs_db_lbl, 1, 0 );

    sir_threshold_call_drop_cs_db_lbl = new QLabel( this, "sir_threshold_call_drop_cs_db_lbl" );
    csDcaParamLayout->addWidget( sir_threshold_call_drop_cs_db_lbl, 2, 0 );

    sir_threshold_call_request_cs_db_val = new QLineEdit( this, "sir_threshold_call_request_cs_db_val" );
    csDcaParamLayout->addWidget( sir_threshold_call_request_cs_db_val, 1, 1 );

    sir_threshold_call_drop_cs_db_val = new QLineEdit( this, "sir_threshold_call_drop_cs_db_val" );
    csDcaParamLayout->addWidget( sir_threshold_call_drop_cs_db_val, 2, 1 );

    int_threshold_call_request_cs_db_lbl = new QLabel( this, "int_threshold_call_request_cs_db_lbl" );
    csDcaParamLayout->addWidget( int_threshold_call_request_cs_db_lbl, 3, 0 );

    int_threshold_call_request_cs_db_val = new QLineEdit( this, "int_threshold_call_request_cs_db_val" );
    csDcaParamLayout->addWidget( int_threshold_call_request_cs_db_val, 3, 1 );

    int_threshold_call_drop_cs_db_lbl = new QLabel( this, "int_threshold_call_drop_cs_db_lbl" );
    csDcaParamLayout->addWidget( int_threshold_call_drop_cs_db_lbl, 4, 0 );

    int_threshold_call_drop_cs_db_val = new QLineEdit( this, "int_threshold_call_drop_cs_db_val" );
    csDcaParamLayout->addWidget( int_threshold_call_drop_cs_db_val, 4, 1 );

    cs_dca_algorithm = new Q3ButtonGroup( this, "cs_dca_algorithm" );
    cs_dca_algorithm->setColumnLayout(0, Qt::Vertical );
    cs_dca_algorithm->layout()->setSpacing( 6 );
    cs_dca_algorithm->layout()->setMargin( 11 );

    cs_dca_algorithmLayout = new Q3GridLayout( cs_dca_algorithm->layout() );
    cs_dca_algorithmLayout->setAlignment( Qt::AlignTop );

    /*  
        radio buttons and the spacers
     */
    spacer1 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    cs_dca_algorithmLayout->addItem( spacer1, 0, 1 );

    cs_sir = new QRadioButton( cs_dca_algorithm, "cs_sir" );
    cs_dca_algorithmLayout->addMultiCellWidget( cs_sir, 1, 1, 0, 1 );

    spacer2 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    cs_dca_algorithmLayout->addItem( spacer2, 2, 0 );

    cs_int = new QRadioButton( cs_dca_algorithm, "cs_int" );
    cs_dca_algorithmLayout->addMultiCellWidget( cs_int, 3, 3, 0, 1 );

    spacer3 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    cs_dca_algorithmLayout->addItem( spacer3, 4, 0 );

    cs_melco = new QRadioButton( cs_dca_algorithm, "cs_melco" );
    cs_dca_algorithmLayout->addMultiCellWidget( cs_melco, 5, 5, 0, 1 );

    spacer4 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    cs_dca_algorithmLayout->addItem( spacer4, 6, 1 );

    //initialization
    switch (((PHSNetworkClass*) np)->cs_dca_alg) {
        case (CConst::SIRDCA):
            init_cs_dca_alg( 0 );
            break;
        case (CConst::IntDCA):
            init_cs_dca_alg( 1 );
            break;
    }

    /*------------------------------*/
    //read param and write them to GUI
    if ( np->technology() == CConst::PHS ) {
        QString str;

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->sir_threshold_call_request_cs_db );
        sir_threshold_call_request_cs_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->int_threshold_call_request_cs_db );
        int_threshold_call_request_cs_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->sir_threshold_call_drop_cs_db );
        sir_threshold_call_drop_cs_db_val->setText( str );

        str.sprintf( "%f",  ((PHSNetworkClass*) np)->int_threshold_call_drop_cs_db );
        int_threshold_call_drop_cs_db_val->setText( str );
    }

    csDcaParamLayout->addMultiCellWidget( cs_dca_algorithm, 0, 0, 0, 1 );
    languageChange();
    resize( QSize(527, 322).expandedTo(minimumSizeHint()) );

    // initilazation
    cs_melco->setHidden(true);

    //sigal and slot
    connect( cs_dca_algorithm, SIGNAL( clicked( int ) ), this, SLOT( set_cs_dca_alg( int ) ) );

    /*  
        tab order
     */
    setTabOrder( cs_sir, cs_int );
    setTabOrder( cs_int, cs_melco );
    setTabOrder( cs_melco, sir_threshold_call_request_cs_db_val );
    setTabOrder( sir_threshold_call_request_cs_db_val, sir_threshold_call_drop_cs_db_val );
    setTabOrder( sir_threshold_call_drop_cs_db_val, int_threshold_call_request_cs_db_val );
    setTabOrder( int_threshold_call_request_cs_db_val, int_threshold_call_drop_cs_db_val );
}


csDcaParam::~csDcaParam()
{
}


/*
 */
void csDcaParam::languageChange()
{
    setCaption( tr( "CS DCA Parameters" ) );

    sir_threshold_call_request_cs_db_lbl->setText( tr( "SIR threshold at a CS for a call request" ) );
    sir_threshold_call_drop_cs_db_lbl->setText( tr( "SIR threshold at a CS for a call drop" ) );
    int_threshold_call_request_cs_db_lbl->setText( tr( "INT threshold at a CS for a call request" ) );
    int_threshold_call_drop_cs_db_lbl->setText( tr( "INT threshold at a CS for a call drop" ) );

    cs_dca_algorithm->setTitle( tr( "CS DCA algorithm" ) );
    cs_sir->setText( tr( "SIR(signal-to-interference ratio)" ) );
    cs_int->setText( tr( "INT(interference)" ) );
    cs_melco->setText( tr( "melco" ) );
}

void csDcaParam::ok_btn_clicked()
{

    if (sir_threshold_call_request_cs_db_val->edited()) {
        sprintf(np->line_buf, "set -sir_threshold_call_request_cs_db %s", sir_threshold_call_request_cs_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (sir_threshold_call_drop_cs_db_val->edited()) {
        sprintf(np->line_buf, "set -sir_threshold_call_drop_cs_db %s", sir_threshold_call_drop_cs_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (int_threshold_call_request_cs_db_val->edited()) {
        sprintf(np->line_buf, "set -int_threshold_call_request_cs_db %s", int_threshold_call_request_cs_db_val->text().latin1());
        np->process_command(np->line_buf);
    }

    if (int_threshold_call_drop_cs_db_val->edited()) {
        sprintf(np->line_buf, "set -int_threshold_call_drop_cs_db %s", int_threshold_call_drop_cs_db_val->text().latin1());
        np->process_command(np->line_buf);
    }
}

#if CGDEBUG
void csDcaParam::cancel_btn_clicked()
{
}
#endif

void csDcaParam::set_cs_dca_alg( int i )
{
    switch (i) 
    {
        case 0:
            sir_threshold_call_request_cs_db_val->setDisabled(false);
            int_threshold_call_request_cs_db_val->setDisabled(true);
            sir_threshold_call_drop_cs_db_val->setDisabled(false);
            int_threshold_call_drop_cs_db_val->setDisabled(true);

            sprintf(np->line_buf,"set -cs_dca_alg sir");
            np->process_command(np->line_buf);

            break;
        case 1:
            sir_threshold_call_request_cs_db_val->setDisabled(true);
            int_threshold_call_request_cs_db_val->setDisabled(false);
            sir_threshold_call_drop_cs_db_val->setDisabled(true);
            int_threshold_call_drop_cs_db_val->setDisabled(false);

            sprintf(np->line_buf,"set -cs_dca_alg int");
            np->process_command(np->line_buf);

            break;
    }
}

void csDcaParam::init_cs_dca_alg( int i )
{
    switch (i) 
    {
        case 0:
            sir_threshold_call_request_cs_db_val->setDisabled(false);
            int_threshold_call_request_cs_db_val->setDisabled(true);
            sir_threshold_call_drop_cs_db_val->setDisabled(false);
            int_threshold_call_drop_cs_db_val->setDisabled(true);

            cs_sir->setChecked( true );
            cs_int->setChecked( false );

            break;
        case 1:
            sir_threshold_call_request_cs_db_val->setDisabled(true);
            int_threshold_call_request_cs_db_val->setDisabled(false);
            sir_threshold_call_drop_cs_db_val->setDisabled(true);
            int_threshold_call_drop_cs_db_val->setDisabled(false);

            cs_int->setChecked( true );
            cs_sir->setChecked( false );

            break;
    }
}



