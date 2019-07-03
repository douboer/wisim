
/******************************************************************************************
**** PROGRAM: run_simulation_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include "run_simulation_dia.h"
#include "statistics.h"

#include <stdlib.h>
#include <qvariant.h>
#include <qstring.h>
#include <q3frame.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3filedialog.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3CString>
#include <Q3VBoxLayout>

//*****************************************************************************
//Function : RunSimulation::RunSimulation
//*****************************************************************************
RunSimulation::RunSimulation(NetworkClass* np_param, QWidget* parent) 
    : QDialog( parent, 0, true )
{
    setName( "RunSimulation" );
    np = np_param; 

    run_simulation_vlayout = new Q3VBoxLayout(this, 4, 2, "run_simulation_vlayout");

    //number of initial event layout
    num_event_ini_hlayout = new Q3HBoxLayout(0, 4, 2, "num_event_ini_hlayout");
    num_event_ini_lbl = new QLabel( this, "num_event_ini_lbl" );
    num_event_ini_hlayout->addWidget(num_event_ini_lbl, 0, Qt::AlignLeft);

    num_event_ini_spinbox = new QSpinBox( this, "num_event_ini_spinbox" );
    num_event_ini_spinbox->setMinValue( 0 );
    num_event_ini_spinbox->setMaxValue( ((DEMO==0) ? 1000000 : 50000) );

    // specify number of run events
#if 0
    num_event_ini_spinbox->setLineStep( 1000 );
    num_event_ini_spinbox->setValue( 10000 );

    // specify duration of run events
#else
    num_event_ini_spinbox->setLineStep( 30 );
    num_event_ini_spinbox->setValue( 300 );
#endif

    num_event_ini_spinbox->setMaximumWidth(100);
    num_event_ini_spinbox->setMinimumWidth(100);
    num_event_ini_hlayout->addWidget(num_event_ini_spinbox, 0, Qt::AlignLeft);

    run_simulation_vlayout->addLayout(num_event_ini_hlayout);

    //number of run event layout
    num_event_run_hlayout = new Q3HBoxLayout(0, 4, 2, "num_event_run_hlayout");
    num_event_run_lbl = new QLabel( this, "num_event_run_lbl" );
    num_event_run_hlayout->addWidget(num_event_run_lbl, 0, Qt::AlignLeft);

    num_event_run_spinbox = new QSpinBox( this, "num_event_run_spinbox" );
    num_event_run_spinbox->setMinValue( 0 );
    num_event_run_spinbox->setMaxValue( ((DEMO==0) ? 1000000000 : 50000) );

    // specify number of run events
#if 0
    num_event_run_spinbox->setLineStep( 10000 );
    num_event_run_spinbox->setValue( 100000 );

    // specify duration of run events
#else
    num_event_run_spinbox->setLineStep( 30 );
    num_event_run_spinbox->setValue( 3000 );
#endif

    num_event_run_spinbox->setMaximumWidth(100);
    num_event_run_spinbox->setMinimumWidth(100);
    num_event_run_hlayout->addWidget(num_event_run_spinbox, 0, Qt::AlignLeft);

    run_simulation_vlayout->addLayout(num_event_run_hlayout);

    //advance, run and cancel layout
    adv_run_cancel_hlayout = new Q3HBoxLayout(0, 4, 2, "adv_run_cancel_hlayout");
    advanced_btn = new QPushButton(this,"advanced_btn");
    advanced_btn->setMaximumWidth( 120 );
    advanced_btn->setMinimumWidth( 120 );
    adv_run_cancel_hlayout->addWidget(advanced_btn, 0, Qt::AlignRight);

    run_btn = new QPushButton(this,"run_btn");
    run_btn->setMaximumWidth( 100 );
    adv_run_cancel_hlayout->addWidget(run_btn, 0, Qt::AlignRight);

    cancel_btn = new QPushButton(this,"cancel_btn");
    cancel_btn->setMaximumWidth( 100 );
    adv_run_cancel_hlayout->addWidget(cancel_btn, 0, Qt::AlignRight);

    run_simulation_vlayout->addLayout(adv_run_cancel_hlayout);

    languageChange();
    //signals and slots connections
    connect( advanced_btn, SIGNAL( clicked() ), this, SLOT( advanced_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( run_btn, SIGNAL( clicked() ), this, SLOT( run_btn_clicked() ) );

    //setup the extention of the Advanced widget
    advancedShown = FALSE;
    setExtension( new AdvRunSimulation( np, this) );
    setOrientation( Qt::Vertical );

    exec();
}

/*
 *  Destroys the object and frees any allocated resources
 */
RunSimulation::~RunSimulation()
{
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void RunSimulation::languageChange()
{
    setCaption( tr( "Run Simulation Settings" ) );

#if 0  // 2007-4-4 modified the type of run simulation
    num_event_ini_lbl->setText( tr( "Number of Initialization Events" ) );
    num_event_run_lbl->setText( tr( "Number of Run Events" ) );
#else
    num_event_ini_lbl->setText( tr( "Duration of Initialization Simulation(seconds)" ) );
    num_event_run_lbl->setText( tr( "Duration of Simulation(seconds)               " ) );
#endif


    advanced_btn->setText( tr( "&Advanced" ) );
    run_btn->setText( tr( "&Run" ) );
    cancel_btn->setText( tr( "&Cancel" ) );
}

/*
 * Begin to run the simulation. First initial the network,
 * then reset all the call staticstics and run again
 */
void RunSimulation::run_btn_clicked()
{
    sprintf( np->line_buf, "switch_mode -mode edit_geom" );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "reseed_rangen -seed 2244344" );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "switch_mode -mode simulate" );
    np->process_command(np->line_buf);

#if 0  // 2007-4-4 modified the type of run simulation
    sprintf( np->line_buf, "run_event -num_event %d",num_event_ini_spinbox->value() );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "reset_call_statistics" );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "run_event -num_event %d",num_event_run_spinbox->value() );
    np->process_command(np->line_buf);
#else
    sprintf( np->line_buf, "run_simulation -time %d",num_event_ini_spinbox->value() );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "reset_call_statistics" );
    np->process_command(np->line_buf);

    sprintf( np->line_buf, "run_simulation -time %d",num_event_run_spinbox->value() );
    np->process_command(np->line_buf);
#endif
}

/*
 * Cancel the run simulation dialog setup.
 */
void RunSimulation::cancel_btn_clicked()
{
    delete this;
}

/*
 * Extend the advanced run simulation setup
 */
void RunSimulation::advanced_btn_clicked()
{
    advancedShown = !advancedShown;
    showExtension( advancedShown );
    if ( !advancedShown ){
        advanced_btn->setText( tr("&Advanced") );
    } else {
        advanced_btn->setText( tr("&Advanced <<<") );
    }
}


//*****************************************************************************
//Function : AdvRunSimulation
//*****************************************************************************
AdvRunSimulation::AdvRunSimulation(NetworkClass* np_param, QWidget* parent)
    : QWidget( parent, 0 )
{
    setName( "AdvRunSimulation" );
    np = np_param;
    record_crr_btn_down = false;
    record_cun_btn_down = false;
    record_event_btn_down = false;

    adv_simulation_vlayout = new Q3VBoxLayout( this, 4, 2, "adv_simulation_vlayout" );

    // Create a horizontal frame line
    Q3Frame *line = new Q3Frame( this );
    line->setLineWidth( 2 );
    line->setMidLineWidth( 1 );
    line->setFrameStyle( Q3Frame::HLine | Q3Frame::Raised );
    adv_simulation_vlayout->addWidget( line );

    //manual reset title layout
    manual_run_title_hlayout = new Q3HBoxLayout( 0, 4, 2, "manual_run_title_hlayout" );
    manual_run_title_lbl = new QLabel( this, "manual_run_title_lbl" );
    manual_run_title_hlayout->addWidget(manual_run_title_lbl, 0, Qt::AlignLeft );

    Q3Frame *line1 = new Q3Frame( this );
    line1->setLineWidth( 1 );
    line1->setMidLineWidth( 0 );
    line1->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
    manual_run_title_hlayout->addWidget( line1 );

    adv_simulation_vlayout->addLayout( manual_run_title_hlayout );

    manual_run_hlayout = new Q3HBoxLayout( 0, 4, 2, "manual_run_hlayout" );
    num_event_lbl = new QLabel( this, "num_event_lbl" );
    num_event_lbl->setMaximumWidth( 100 );
    manual_run_hlayout->addWidget( num_event_lbl, 0, Qt::AlignCenter );

    num_event_spinbox = new QSpinBox( this, "num_event_spinbox" );
    num_event_spinbox->setMinValue( 0 );
    num_event_spinbox->setMaxValue( 100000000 );
    num_event_spinbox->setLineStep( 1000 );
    num_event_spinbox->setValue( 1000 );
    num_event_spinbox->setMaximumWidth(100);
    num_event_spinbox->setMinimumWidth(100);
    manual_run_hlayout->addWidget( num_event_spinbox, 0, Qt::AlignCenter );

    manual_reset_btn = new QPushButton( this, "manual_reset_btn" );
    manual_reset_btn->setMaximumWidth( 100 );
    manual_run_hlayout->addWidget( manual_reset_btn, 0, Qt::AlignCenter );

    manual_run_btn = new  QPushButton( this, "manual_run_btn" );
    manual_run_btn->setMaximumWidth( 100 );
    manual_run_hlayout->addWidget( manual_run_btn, 0, Qt::AlignCenter );

    adv_simulation_vlayout->addLayout( manual_run_hlayout );

    //record crr title layout
    record_crr_title_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_crr_title_hlayout" );
    record_crr_title_lbl = new QLabel( this, "record_crr_title_lbl" );
    record_crr_title_hlayout->addWidget( record_crr_title_lbl, 0, Qt::AlignLeft );

    Q3Frame *line2 = new Q3Frame( this );
    line2->setLineWidth( 1 );
    line2->setMidLineWidth( 0 );
    line2->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
    record_crr_title_hlayout->addWidget( line2 );

    adv_simulation_vlayout->addLayout( record_crr_title_hlayout );

    //file name set
    record_crr_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_crr_hlayout" );
    record_crr_file_lbl = new QLabel( this, "record_crr_file_lbl" );
    record_crr_file_lbl->setMaximumWidth( 100 );
    record_crr_hlayout->addWidget( record_crr_file_lbl, 0, Qt::AlignCenter );

    record_crr_file_val = new QLineEdit( this, "record_crr_file_val" );
    record_crr_file_val->setMaximumWidth( 200 );
    record_crr_hlayout->addWidget( record_crr_file_val, 0, Qt::AlignCenter );

    record_crr_file_toolbtn = new QToolButton( this, "record_crr_file_toolbtn" );
    record_crr_file_toolbtn->setMaximumWidth( 100 );
    record_crr_hlayout->addWidget( record_crr_file_toolbtn, 0, Qt::AlignLeft );

    record_crr_btn = new QPushButton( this, "record_crr_btn" );
    record_crr_btn->setMaximumWidth( 100 );
    if ( np->stat->measure_crr ) {
        record_crr_btn->setDown( true );
        record_crr_btn_down = true;
    }
    record_crr_hlayout->addWidget( record_crr_btn, 0, Qt::AlignCenter );

    stop_crr_btn = new QPushButton( this, "stop_crr_btn" );
    stop_crr_btn->setMaximumWidth( 100 );
    record_crr_hlayout->addWidget( stop_crr_btn, 0, Qt::AlignCenter );

    adv_simulation_vlayout->addLayout( record_crr_hlayout );

    //number set
    record_crr_num_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_crr_num_hlayout" );
    record_crr_num_lbl = new QLabel( this, "record_crr_num_lbl" );
    record_crr_num_lbl->setMaximumWidth( 100 );
    record_crr_num_hlayout->addWidget( record_crr_num_lbl, 0, Qt::AlignCenter );

    record_crr_num_val = new QLineEdit( this, "record_crr_num_val" );
    record_crr_num_val->setMaximumWidth( 100 );
    record_crr_num_val->setText( "1001" );
    record_crr_num_hlayout->addWidget( record_crr_num_val, 0, Qt::AlignCenter );

    QSpacerItem *spacer0 = new QSpacerItem( 40, 20 );
    record_crr_num_hlayout->addItem( spacer0 );

    QSpacerItem *spacer1 = new QSpacerItem( 40, 20 );
    record_crr_num_hlayout->addItem( spacer1 );

    adv_simulation_vlayout->addLayout( record_crr_num_hlayout );

    //minimum and maximum set
    record_crr_val_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_crr_val_hlayout" );
    record_crr_min_lbl = new QLabel( this, "record_crr_min_lbl" );
    record_crr_min_lbl->setMaximumWidth( 100 );
    record_crr_val_hlayout->addWidget( record_crr_min_lbl, 0, Qt::AlignCenter );

    record_crr_min_val = new QLineEdit( this, "record_crr_min_val" );
    record_crr_min_val->setMaximumWidth( 100 );
    record_crr_min_val->setText( "0" );
    record_crr_val_hlayout->addWidget( record_crr_min_val, 0, Qt::AlignCenter );

    record_crr_max_lbl = new QLabel( this, "record_crr_max_lbl" );
    record_crr_max_lbl->setMaximumWidth( 100 );
    record_crr_val_hlayout->addWidget( record_crr_max_lbl, 0, Qt::AlignCenter );

    record_crr_max_val = new QLineEdit( this, "record_crr_max_val" );
    record_crr_max_val->setMaximumWidth( 100 );
    record_crr_max_val->setText( "10.0" );
    record_crr_val_hlayout->addWidget( record_crr_max_val, 0, Qt::AlignCenter );

    adv_simulation_vlayout->addLayout( record_crr_val_hlayout );

    //record cun set
    record_cun_title_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_cun_title_hlayout" );
    record_cun_title_lbl = new QLabel( this, "record_cun_title_lbl" );
    record_cun_title_hlayout->addWidget( record_cun_title_lbl, 0, Qt::AlignLeft );

    Q3Frame *line3 = new Q3Frame( this );
    line3->setLineWidth( 1 );
    line3->setMidLineWidth( 0 );
    line3->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
    record_cun_title_hlayout->addWidget( line3 );

    adv_simulation_vlayout->addLayout( record_cun_title_hlayout );

    //record cun filename setting
    record_cun_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_cun_hlayout" );
    record_cun_file_lbl = new QLabel( this, "record_cun_file_lbl" );
    record_cun_file_lbl->setMaximumWidth( 100 );
    record_cun_hlayout->addWidget( record_cun_file_lbl, 0, Qt::AlignCenter );

    record_cun_file_val = new QLineEdit( this, "record_cun_file_val" );
    record_cun_file_val->setMaximumWidth( 200 );
    record_cun_hlayout->addWidget( record_cun_file_val, 0, Qt::AlignCenter );

    record_cun_file_toolbtn = new QToolButton( this, "record_cun_file_toolbtn" );
    record_cun_file_toolbtn->setMaximumWidth( 100 );
    record_cun_hlayout->addWidget( record_cun_file_toolbtn, 0, Qt::AlignLeft );

    record_cun_btn = new QPushButton( this, "record_cun_btn" );
    record_cun_btn->setMaximumWidth( 100 );
    if ( np->stat->plot_num_comm ) {
        record_cun_btn->setDown( true );
        record_cun_btn_down = true;
    }
    record_cun_hlayout->addWidget( record_cun_btn, 0, Qt::AlignCenter );

    stop_cun_btn = new QPushButton( this, "stop_cun_btn" );
    stop_cun_btn->setMaximumWidth( 100 );
    record_cun_hlayout->addWidget( stop_cun_btn, 0, Qt::AlignCenter );

    adv_simulation_vlayout->addLayout( record_cun_hlayout );

    //record event setting
    record_event_title_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_event_title_hlayout");
    record_event_title_lbl = new QLabel( this, "record_event_title_lbl" );
    record_event_title_hlayout->addWidget( record_event_title_lbl, 0, Qt::AlignLeft );

    Q3Frame *line4 = new Q3Frame( this );
    line4->setLineWidth( 1 );
    line4->setMidLineWidth( 0 );
    line4->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
    record_event_title_hlayout->addWidget( line4 );

    adv_simulation_vlayout->addLayout( record_event_title_hlayout );

    //record event filename setting
    record_event_hlayout = new Q3HBoxLayout( 0, 4, 2, "record_event_hlayout" );
    record_event_file_lbl = new QLabel( this, "record_event_file_lbl" );
    record_event_file_lbl->setMaximumWidth( 100 );
    record_event_hlayout->addWidget( record_event_file_lbl, 0, Qt::AlignCenter );

    record_event_file_val = new QLineEdit( this, "record_event_file_val" );
    record_event_file_val->setMaximumWidth( 200 );
    record_event_hlayout->addWidget( record_event_file_val, 0, Qt::AlignCenter );

    record_event_file_toolbtn = new QToolButton( this, "record_event_file_toolbtn" );
    record_event_file_toolbtn->setMaximumWidth( 100 );
    record_event_hlayout->addWidget( record_event_file_toolbtn, 0, Qt::AlignLeft );

    record_event_btn = new QPushButton( this, "record_event_btn");
    record_event_btn->setMaximumWidth( 100 );
    if ( np->stat->plot_event ) {
        record_event_btn->setDown( true );
        record_event_btn_down = true;
    }
    record_event_hlayout->addWidget( record_event_btn, 0, Qt::AlignCenter );

    stop_event_btn = new QPushButton( this, "stop_event_btn" );
    stop_event_btn->setMaximumWidth( 100 );
    record_event_hlayout->addWidget( stop_event_btn, 0, Qt::AlignCenter );

    adv_simulation_vlayout->addLayout( record_event_hlayout );

    //set the title of the widgets
    languageChange();

    //signal and slots connections
    connect(manual_reset_btn, SIGNAL( clicked() ), this, SLOT( manual_reset_btn_clicked() ) );
    connect(manual_run_btn, SIGNAL( clicked() ), this, SLOT( manual_run_btn_clicked() ) );

    connect(record_crr_file_toolbtn, SIGNAL( clicked() ), this, SLOT( record_crr_file_toolbtn_clicked() ) );
    connect(record_crr_btn, SIGNAL( clicked() ), this, SLOT( record_crr_btn_clicked() ) );
    connect(stop_crr_btn, SIGNAL( clicked() ), this, SLOT( stop_crr_btn_clicked() ) );

    connect(record_cun_file_toolbtn, SIGNAL( clicked() ), this, SLOT( record_cun_file_toolbtn_clicked() ) );
    connect(record_cun_btn, SIGNAL( clicked() ), this, SLOT( record_cun_btn_clicked() ) );
    connect(stop_cun_btn, SIGNAL( clicked() ), this, SLOT( stop_cun_btn_clicked() ) );

    connect(record_event_file_toolbtn, SIGNAL( clicked() ), this, SLOT( record_event_file_toolbtn_clicked() ) );
    connect(record_event_btn, SIGNAL( clicked() ), this, SLOT( record_event_btn_clicked() ) );
    connect(stop_event_btn, SIGNAL( clicked() ), this, SLOT( stop_event_btn_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AdvRunSimulation::~AdvRunSimulation()
{
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AdvRunSimulation::languageChange()
{
    //setting for manual run
    manual_run_title_lbl->setText( tr("Advanced Simulation Settings") );
    num_event_lbl->setText( tr("Number of Events") );
    manual_reset_btn->setText( tr("Reset") );
    manual_run_btn->setText( tr("Run") );

    //setting for crr
    record_crr_title_lbl->setText( tr("Record Co-channel Reuse Ratio Statistics") );
    record_crr_file_lbl->setText( tr("File Name") );
    record_crr_file_val->setText( "crr.wvfm" );
    record_crr_min_lbl->setText( tr("Minimum CRR") );
    record_crr_max_lbl->setText( tr("Maximum CRR") );
    record_crr_num_lbl->setText( tr("Number of CRR") );
    record_crr_file_toolbtn->setText( "..." );
    record_crr_btn->setText( tr("Record") );
    stop_crr_btn->setText( tr("Stop") );

    //set for comm number
    record_cun_title_lbl->setText( tr("Record Number of COMM User Statistics") );
    record_cun_file_lbl->setText( tr("File Name") );
    record_cun_file_val->setText( "num_comm.wvfm" );
    record_cun_file_toolbtn->setText( "..." );
    record_cun_btn->setText( tr("Record") );
    stop_cun_btn->setText( tr("Stop") );

    //setting for record event
    record_event_title_lbl->setText( tr("Record Simulation Events") );
    record_event_file_lbl->setText( tr("File Name") );
    record_event_file_val->setText( "plot_event.txt" );
    record_event_file_toolbtn->setText( "..." );
    record_event_btn->setText( tr("Record") );
    stop_event_btn->setText( tr("Stop") );
}

void AdvRunSimulation::manual_reset_btn_clicked()
{
    sprintf( np->line_buf, "reset_call_statistics" );
    np->process_command(np->line_buf);
}

void AdvRunSimulation::manual_run_btn_clicked()
{
    sprintf( np->line_buf, "run_event -num_event %d",num_event_spinbox->value() );
    np->process_command(np->line_buf);
}

void AdvRunSimulation::record_crr_file_toolbtn_clicked()
{
    Q3FileDialog *record_crr_file_dia = new Q3FileDialog;
    QString record_crr_filename = record_crr_file_dia->getSaveFileName(
                    record_crr_file_val->text(), tr("Files") + " (*.wvfm);;" + tr("All Files") + " (*)",
                    this, tr("Record CRR Statistics"), tr("Choose File") );

    if ( !record_crr_filename.isEmpty() ) {
        record_crr_file_val->setText( record_crr_filename );
    }
}

void AdvRunSimulation::record_crr_btn_clicked()
{
    if ( !record_crr_btn_down ) {
        sprintf( np->line_buf, "measure_crr -action start -min_crr %s -max_crr %s -num_crr %s",
                 record_crr_min_val->text().latin1(), record_crr_max_val->text().latin1(),
                 record_crr_num_val->text().latin1() );
        np->process_command(np->line_buf);

        record_crr_btn->setDown( true );
        record_crr_btn_down = true;
    }
    record_crr_btn->setDown( true );
}

void AdvRunSimulation::stop_crr_btn_clicked()
{
    if ( record_crr_btn_down ) {
        Q3CString qcs(2*record_crr_file_val->text().length());
        qcs = record_crr_file_val->text().local8Bit();

        sprintf( np->line_buf, "plot_crr_cdf -f \'%s\'", (const char *) qcs);
        np->process_command(np->line_buf);

        sprintf( np->line_buf, "measure_crr -action stop" );
        np->process_command(np->line_buf);
        record_crr_btn_down = false;
    }

    record_crr_btn->setDown( false );
}

void AdvRunSimulation::record_cun_file_toolbtn_clicked()
{
    Q3FileDialog *record_cun_file_dia = new Q3FileDialog;
    QString record_cun_filename = record_cun_file_dia->getSaveFileName(
                    record_cun_file_val->text(), tr("Files") + " (*.wvfm);;" + tr("All Files") + " (*)",
                    this, tr("Record Number of COMM User Statistics"), tr("Choose File") );

    if ( !record_cun_filename.isEmpty() ) {
        record_cun_file_val->setText( record_cun_filename );
    }
}

void AdvRunSimulation::record_cun_btn_clicked()
{
    if ( !record_cun_btn_down ) {
        Q3CString qcs(2*record_cun_file_val->text().length());
        qcs = record_cun_file_val->text().local8Bit();

        sprintf( np->line_buf, "plot_num_comm -action start -f \'%s\'", (const char *) qcs );
        np->process_command(np->line_buf);

        record_cun_btn->setDown( true );
        record_cun_btn_down = true;
    }
    record_cun_btn->setDown( true );
}

void AdvRunSimulation::stop_cun_btn_clicked()
{
    if ( record_cun_btn_down ) {
        sprintf( np->line_buf, "plot_num_comm -action stop" );
        np->process_command(np->line_buf);
        record_cun_btn_down = false;
    }

    record_cun_btn->setDown( false );
}

void AdvRunSimulation::record_event_file_toolbtn_clicked()
{
    Q3FileDialog *record_event_file_dia = new Q3FileDialog;
    QString record_event_filename = record_event_file_dia->getSaveFileName(
                    record_event_file_val->text(), tr("Files") + " (*.txt);;" + tr("All Files") + " (*)",
                    this, tr("Record Simulation Events"), tr("Choose File") );

    if ( !record_event_filename.isEmpty() ) {
        record_event_file_val->setText( record_event_filename );
    }
}

void AdvRunSimulation::record_event_btn_clicked()
{
    if ( !record_event_btn_down ) {
        Q3CString qcs(2*record_event_file_val->text().length());
        qcs = record_event_file_val->text().local8Bit();

        sprintf( np->line_buf, "plot_event -action start -f \'%s\'", (const char *) qcs );
        np->process_command(np->line_buf);

        record_event_btn->setDown( true );
        record_event_btn_down = true;
    }
    record_event_btn->setDown( true );
}

void AdvRunSimulation::stop_event_btn_clicked()
{
    if ( record_event_btn_down ) {
        Q3CString qcs(2*record_event_file_val->text().length());
        qcs = record_event_file_val->text().local8Bit();
        sprintf( np->line_buf, "plot_event -action stop -f \'%s\'", (const char *) qcs );
        np->process_command(np->line_buf);
        record_event_btn_down = false;
    }

    record_event_btn->setDown( false );
}

