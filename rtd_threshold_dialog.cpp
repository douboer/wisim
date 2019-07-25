
/******************************************************************************************
**** PROGRAM: rtd_threshold_dialog.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <qapplication.h>
#include <q3buttongroup.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <q3table.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <q3whatsthis.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "wisim.h"
#include "cconst.h"
#include "pref.h"
#include "road_test_data.h"
#include "rtd_threshold_dialog.h"

/******************************************************************************************/
/**** RTDThresholdDialog::RTDThresholdDialog                                           ****/
/******************************************************************************************/
RTDThresholdDialog::RTDThresholdDialog( NetworkClass* np_param, QWidget* parent, const char* name)
    : Q3Wizard( parent, name, true )
{
    np = np_param;

    if ( !name )
    setName( "RTDThresholdDialog" );

    /* 
       first page of the wizard
     */
    WizardPage = new QWidget( this, "WizardPage" );
    WizardPageLayout = new Q3VBoxLayout( WizardPage, 11, 6, "WizardPageLayout"); 

    gen_param_buttonGroup = new Q3ButtonGroup( WizardPage, "gen_param_buttonGroup" );
    gen_param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    gen_param_buttonGroup->layout()->setSpacing( 15 );
    gen_param_buttonGroup->layout()->setMargin( 11 );
    gen_param_buttonGroupLayout = new Q3GridLayout( gen_param_buttonGroup->layout() );
    gen_param_buttonGroupLayout->setAlignment( Qt::AlignTop );

    /*** Labels ****/
    num_thr_textLabel = new QLabel( gen_param_buttonGroup, "num_thr_textLabel" );
    num_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, num_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
    gen_param_buttonGroupLayout->addWidget( num_thr_textLabel, 0, 0 );

    min_thr_textLabel = new QLabel( gen_param_buttonGroup, " min_thr_textLabel" );
    gen_param_buttonGroupLayout->addWidget(  min_thr_textLabel, 1, 0 );

    max_thr_textLabel  = new QLabel( gen_param_buttonGroup, " max_thr_textLabel" );
    gen_param_buttonGroupLayout->addWidget(  max_thr_textLabel, 2, 0 );


    /**** Widgets ****/
    num_thr_spinBox = new QSpinBox( gen_param_buttonGroup, "num_thr_spinBox" );
    num_thr_spinBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 5, 0, num_thr_spinBox->sizePolicy().hasHeightForWidth() ) );
    num_thr_spinBox->setMinimumSize( QSize( 150, 0 ) );
    num_thr_spinBox->setMaximumSize( QSize( 400, 32767 ) );
    num_thr_spinBox->setMinValue(1);
    gen_param_buttonGroupLayout->addWidget( num_thr_spinBox, 0, 1 );

    min_thr_lineEdit = new QLineEdit( gen_param_buttonGroup, " min_thr_lineEdit" );
    min_thr_lineEdit->setEnabled( TRUE );
    gen_param_buttonGroupLayout->addWidget(  min_thr_lineEdit, 1, 1 );

    max_thr_lineEdit = new QLineEdit( gen_param_buttonGroup, " max_thr_lineEdit" );
    max_thr_lineEdit->setEnabled( TRUE );
    gen_param_buttonGroupLayout->addWidget(  max_thr_lineEdit, 2, 1 );

    WizardPageLayout->addWidget( gen_param_buttonGroup );

    addPage( WizardPage, QString("") );
    //-------------------------------------------------------------------------------------

    //init WizardPage
    num_thr_spinBox->setValue( RoadTestPtClass::num_level );

    QString s;
    s = QString("%1").arg(RoadTestPtClass::level_list[0] - np->preferences->pwr_offset);
    min_thr_lineEdit->setText(s);

    s = QString("%1").arg(RoadTestPtClass::level_list[RoadTestPtClass::num_level-1] - np->preferences->pwr_offset);
    max_thr_lineEdit->setText(s);
    
    /* 
       second page of the wizard
     */
    WizardPage1 = new QWidget( this, "WizardPage1" );
    WizardPage1Layout = new Q3HBoxLayout( WizardPage1, 11, 6, "WizardPageLayout"); 

    table = new Q3Table( WizardPage1, " table" );
    table->setNumCols( 1 );
    table->setColumnWidth(0,  200 );
    WizardPage1Layout->addWidget(  table );
    addPage( WizardPage1, QString("") );

    //init WizardPage1
    double step;
    double thr_val;
    int    i;
    QString thr_str; 
    min_thr =  min_thr_lineEdit->text().toDouble();
    max_thr =  max_thr_lineEdit->text().toDouble();
    num_thr = num_thr_spinBox->value();

    for ( i=0; i< table->numRows(); i++ )
    {
         table->removeRow( 0 );
    }

    step = (max_thr-min_thr)/(num_thr-1);
    thr_val = min_thr;
    for ( i=0; i<num_thr; i++ ) {
        thr_str.sprintf( "%f", thr_val ); 
        table->insertRows( i );
        table->setText( i, 0, thr_str ); 

        thr_str.sprintf( "threshold_%d", i );
        Q3Header *th =  table->verticalHeader();
        th->setLabel( i, thr_str );
        
        m_thr_vector.push_back( thr_val );

        thr_val = thr_val + step;
    }

    helpButton()->hide();
    languageChange();
    resize( QSize(500, 200).expandedTo(minimumSizeHint()) );

    //signals and slots
    connect(  table, SIGNAL( valueChanged( int, int ) ), this , SLOT( table_valueChanged( int, int ) ) );
    connect( finishButton(), SIGNAL( clicked () ), this, SLOT( maccept() ) );
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( thr_changed( ) ) );
    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( next_btn_clicked() ) );
    connect( backButton(), SIGNAL( clicked() ), this, SLOT( back_clicked() ) );

    exec();
}

RTDThresholdDialog::~RTDThresholdDialog()
{
}

void RTDThresholdDialog::next_btn_clicked()
{
    helpButton()->setDisabled(true);
}

void RTDThresholdDialog::back_clicked()
{
    int rows =  table->numRows();
    for ( int i=0; i<rows; i++ )
    {
         table->removeRow( 0 );
    }

    helpButton()->setDisabled(false);
}


void RTDThresholdDialog::thr_changed()
{
    int i;
    int rows =  table->numRows();
    for ( i=0; i<rows; i++ )
    {
         table->removeRow( 0 );
    }
    m_thr_vector.clear();

    double step, thr_val;
    QString thr_str; 
    min_thr =  min_thr_lineEdit->text().toDouble();
    max_thr =  max_thr_lineEdit->text().toDouble();
    num_thr = num_thr_spinBox->value();
    step = (max_thr-min_thr)/(num_thr-1);
    thr_val = min_thr;
    for ( i=0; i<num_thr; i++ ) {
        thr_str.sprintf( "%f", thr_val ); 
         table->insertRows( i );
         table->setText( i, 0, thr_str ); 

        thr_str.sprintf( "threshold_%d", i );
        Q3Header *th =  table->verticalHeader();
        th->setLabel( i, thr_str );
        
        m_thr_vector.push_back( thr_val );

        thr_val = thr_val + step;
    }

}

void RTDThresholdDialog::table_valueChanged( int i, int )
{
    m_thr_vector[i] =  table->text( i, 0 ).toDouble();
}

Q3ValueVector <double>& RTDThresholdDialog::getThrVector ( )
{
    return m_thr_vector;
}

void RTDThresholdDialog::maccept()
{
    num_thr = num_thr_spinBox->value();
    min_thr = min_thr_lineEdit->text().toDouble();
    max_thr = max_thr_lineEdit->text().toDouble();

    //here call process command
    /* set_road_test_data -thr_list 't0 t1 ... tn-1'                 Set thresholds for road test data  */
    /* set_color -level idx -road_test_data color                                                       */
    int i = 0;
    QString str;
    QString cmd_str;
    cmd_str += "'";
    for( i=0; i<(int) m_thr_vector.count(); i++ )
    {
        str.sprintf( "%f ", m_thr_vector[i] + np->preferences->pwr_offset);
        cmd_str += str;
    }

    cmd_str += "'";

    //call process_command() to set threshold value
    sprintf(np->line_buf, "set_road_test_data -thr_list %s", cmd_str.latin1() );
    np->process_command(np->line_buf);

    QDialog::accept();
}

void RTDThresholdDialog::languageChange()
{
    setCaption(tr("Road Test Points Setting"));
    QString s;
    
    gen_param_buttonGroup->setTitle( tr( "General" ) );

    num_thr_textLabel->setText( tr( "Number of Thresholds" ) );

    s = tr("Minimum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    min_thr_textLabel->setText(s);

    s = tr("Maximum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    max_thr_textLabel->setText(s);

    setTitle( WizardPage, tr( "Set Threshold Parameters" ) );
    table->horizontalHeader()->setLabel( 0, tr( "threshold" ) );
    helpButton()->setText( tr( "Advanced >>>" ) );
    setTitle( WizardPage1, tr( "Threshold" ) );

    setFinishEnabled( WizardPage1, true );

    backButton()->setText(tr("&Back") + " <");
    nextButton()->setText(tr("&Next") + " >");
    finishButton()->setText(tr("&Finish"));
    cancelButton()->setText(tr("&Cancel"));
    helpButton()->setText(tr("&Help"));

}

void RTDThresholdDialog::cancel_btn_clicked()
{
    delete this;
}
/******************************************************************************************/
