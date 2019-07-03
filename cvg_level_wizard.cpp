
/******************************************************************************************
**** PROGRAM: cvg_level_wizard.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#include "cvg_level_wizard.h"

#include <iostream>
#include <math.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <q3table.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "WiSim.h"
#include "pref.h"

/*
     wizard for level type of coverage
 */
CvgLevelWizard::CvgLevelWizard( NetworkClass* np_param, QWidget* parent, const char* name)
    : Q3Wizard( parent, name, true )
{
    std::cout << "CALL CvgLevelWizard::CvgLevelWizard() " << std::endl;
    
    int i;
    extension = FALSE;

    np = np_param;

    if ( !name )
	setName( "CvgLevelWizard" );

    /* 
       second page of the wizard
     */
    WizardPage1 = new QWidget( this, "WizardPage1" );
    WizardPage1Layout = new Q3VBoxLayout( WizardPage1, 11, 6, "WizardPage1Layout"); 

    cvg_gen_param_buttonGroup = new Q3ButtonGroup( WizardPage1, "cvg_gen_param_buttonGroup" );
    cvg_gen_param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    cvg_gen_param_buttonGroup->layout()->setSpacing( 15 );
    cvg_gen_param_buttonGroup->layout()->setMargin( 11 );
    cvg_gen_param_buttonGroupLayout = new Q3GridLayout( cvg_gen_param_buttonGroup->layout() );
    cvg_gen_param_buttonGroupLayout->setAlignment( Qt::AlignTop );

    info_max_thr_textLabel = new QLabel( cvg_gen_param_buttonGroup, "info_max_thr_textLabel" );
    cvg_gen_param_buttonGroupLayout->addWidget( info_max_thr_textLabel, 2, 0 );

    info_min_thr_textLabel = new QLabel( cvg_gen_param_buttonGroup, "info_min_thr_textLabel" );
    cvg_gen_param_buttonGroupLayout->addWidget( info_min_thr_textLabel, 1, 0 );

    num_thr_textLabel = new QLabel( cvg_gen_param_buttonGroup, "num_thr_textLabel" );
    num_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, 
                (QSizePolicy::SizeType)5, 5, 0, num_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
    cvg_gen_param_buttonGroupLayout->addWidget( num_thr_textLabel, 0, 0 );

    info_max_thr_lineEdit = new QLineEdit( cvg_gen_param_buttonGroup, "info_max_thr_lineEdit" );
    info_max_thr_lineEdit->setEnabled( TRUE );
    cvg_gen_param_buttonGroupLayout->addWidget( info_max_thr_lineEdit, 2, 1 );

    info_min_thr_lineEdit = new QLineEdit( cvg_gen_param_buttonGroup, "info_min_thr_lineEdit" );
    info_min_thr_lineEdit->setEnabled( TRUE );
    cvg_gen_param_buttonGroupLayout->addWidget( info_min_thr_lineEdit, 1, 1 );

    num_thr_spinBox = new QSpinBox( cvg_gen_param_buttonGroup, "num_thr_spinBox" );
    num_thr_spinBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, 
                (QSizePolicy::SizeType)0, 5, 0, num_thr_spinBox->sizePolicy().hasHeightForWidth() ) );
    num_thr_spinBox->setMinimumSize( QSize( 150, 0 ) );
    num_thr_spinBox->setMaximumSize( QSize( 400, 32767 ) );
    cvg_gen_param_buttonGroupLayout->addWidget( num_thr_spinBox, 0, 1 );

    WizardPage1Layout->addWidget( cvg_gen_param_buttonGroup );

    //-------------------------------------------------------------------------------------
    cvg_adv_param_buttonGroup = new Q3ButtonGroup( WizardPage1, "cvg_adv_param_buttonGroup" );
    cvg_adv_param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    cvg_adv_param_buttonGroup->layout()->setSpacing( 15 );
    cvg_adv_param_buttonGroup->layout()->setMargin( 11 );
    cvg_adv_param_buttonGroupLayout = new Q3GridLayout( cvg_adv_param_buttonGroup->layout() );
    cvg_adv_param_buttonGroupLayout->setAlignment( Qt::AlignTop );

    scan_fraction_area_textLabel = new QLabel( cvg_adv_param_buttonGroup, "scan_fraction_area_textLabel" );
    scan_fraction_area_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, 
                (QSizePolicy::SizeType)5, 5, 0, scan_fraction_area_textLabel->sizePolicy().hasHeightForWidth() ) );
    cvg_adv_param_buttonGroupLayout->addWidget( scan_fraction_area_textLabel, 0, 0 );

    init_sample_resolution_textLabel = new QLabel( cvg_adv_param_buttonGroup, "init_sample_resolution_textLabel" );
    cvg_adv_param_buttonGroupLayout->addWidget( init_sample_resolution_textLabel, 1, 0 );

    scan_fraction_area_lineEdit = new QLineEdit( cvg_adv_param_buttonGroup, "scan_fraction_area_lineEdit" );
    scan_fraction_area_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, 
                (QSizePolicy::SizeType)0, 5, 0, scan_fraction_area_lineEdit->sizePolicy().hasHeightForWidth() ) );
    cvg_adv_param_buttonGroupLayout->addWidget( scan_fraction_area_lineEdit, 0, 1 );

    init_sample_resolution_lineEdit = new QLineEdit( cvg_adv_param_buttonGroup, "init_sample_resolution_lineEdit" );
    cvg_adv_param_buttonGroupLayout->addWidget( init_sample_resolution_lineEdit, 1, 1 );

    WizardPage1Layout->addWidget( cvg_adv_param_buttonGroup );

    cvg_adv_param_buttonGroup->setHidden( true );
    
    addPage( WizardPage1, QString("") );
    //-------------------------------------------------------------------------------------

    //init WizardPage1
    info_min_thr_lineEdit->setText( QString("%1").arg( ( 0.0-110.0-10.0*log(2.0)/log(10.0)) - np->preferences->pwr_offset, 0, 'f', 0));
    info_max_thr_lineEdit->setText( QString("%1").arg( (80.0-110.0-10.0*log(2.0)/log(10.0)) - np->preferences->pwr_offset) );
    num_thr_spinBox->setValue( 9 );

    std::cout << "0.0-110.0-10.0*log(2.0)/log(10.0) " << 0.0-110.0-10.0*log(2.0)/log(10.0) << std::endl;
    std::cout << "pwr_offset " << np->preferences->pwr_offset << std::endl;

    /* 
       third page of the wizard
     */
    WizardPage2 = new QWidget( this, "WizardPage2" );
    WizardPage2Layout = new Q3HBoxLayout( WizardPage2, 11, 6, "WizardPageLayout_2"); 

    cvg_table = new Q3Table( WizardPage2, "cvg_table" );
    cvg_table->setNumCols( 1 );
    cvg_table->setColumnWidth(0, cvg_table->visibleWidth() );
    //cvg_table->showGrid( );
    WizardPage2Layout->addWidget( cvg_table );
    addPage( WizardPage2, QString("") );

    //init WizardPage2
    double step, thr_val;
    QString thr_str; 
    min_thr = info_min_thr_lineEdit->text().toDouble();
    max_thr = info_max_thr_lineEdit->text().toDouble();
    num_thr = num_thr_spinBox->value();

    //clear pre-value  and write new value
    for ( i=0; i<cvg_table->numRows(); i++ )
    {
        cvg_table->removeRow( 0 );
    }

    step = (max_thr-min_thr)/(num_thr-1);
    thr_val = min_thr;
    for ( i=0; i<num_thr; i++ ) {
        thr_str.sprintf( "%f", thr_val ); 
        cvg_table->insertRows( i );
        cvg_table->setText( i, 0, thr_str ); 

        thr_str.sprintf( "threshold_%d", i );
        Q3Header *th = cvg_table->verticalHeader();
        th->setLabel( i, thr_str );
        
        m_thr_vector.push_back( thr_val );

        thr_val = thr_val + step;
        ////qDebug( "initial create table item %d \n ", i );
    }
    
    languageChange();
    resize( QSize(416, 151).expandedTo(minimumSizeHint()) );

    //signals and slots
    connect( cvg_table, SIGNAL( valueChanged( int, int ) ), this , SLOT( table_valueChanged( int, int ) ) );
    connect( finishButton(), SIGNAL( clicked () ), this, SLOT( accept() ) );
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( thr_changed( ) ) );
    connect( helpButton(), SIGNAL( clicked() ), this, SLOT( adv_button_clicked( ) ) );
    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( next_btn_clicked() ) );
    connect( backButton(), SIGNAL( clicked() ), this, SLOT( back_clicked() ) );

    // 
    setExtension(cvg_adv_param_buttonGroup);
    setOrientation(Qt::Vertical);

}

CvgLevelWizard::~CvgLevelWizard()
{

}

void CvgLevelWizard::next_btn_clicked()
{
    helpButton()->setDisabled(true);

    // if next button click, do not show advance menu
    showExtension(false);

    helpButton()->setText(tr( "Advanced") + " >>" );
    extension = !extension;

}

void CvgLevelWizard::back_clicked()
{
    //clear pre-value  and write new value
    int rows = cvg_table->numRows();
    for ( int i=0; i<rows; i++ )
        cvg_table->removeRow( 0 );           //important must be 0, not i

    helpButton()->setDisabled(false);
}


void CvgLevelWizard::adv_button_clicked()
{

  if (!extension) {
    showExtension(true);
    helpButton()->setText(QString("<< ") + tr("Advanced"));    
  }
  else { 
    showExtension(false);
    helpButton()->setText(tr( "Advanced") + " >>" );
  }

  extension = !extension;
}
    
void CvgLevelWizard::thr_changed()
{
    int i;
    //clear pre-value  and write new value
    int rows = cvg_table->numRows();
    for ( i=0; i<rows; i++ )
        cvg_table->removeRow( 0 );

    m_thr_vector.clear();

    double step, thr_val;
    QString thr_str; 

    min_thr = info_min_thr_lineEdit->text().toDouble();
    max_thr = info_max_thr_lineEdit->text().toDouble();
    num_thr = num_thr_spinBox->value();
    step = (max_thr-min_thr)/(num_thr-1);
    thr_val = min_thr;
    for ( i=0; i<num_thr; i++ ) {
        thr_str.sprintf( "%f", thr_val ); 
        cvg_table->insertRows( i );
        cvg_table->setText( i, 0, thr_str ); 

        thr_str.sprintf( "threshold_%d", i );
        Q3Header *th = cvg_table->verticalHeader();
        th->setLabel( i, thr_str );
        
        m_thr_vector.push_back( thr_val );

        thr_val = thr_val + step;

        //XXXXXXX  2005.3.2 CG
        //std::cout << " cvg_table->text( i, 0 ) " << cvg_table->text( i, 0 ) << std::endl;
    }
}

void CvgLevelWizard::table_valueChanged( int i, int j )
{
    m_thr_vector[i] = cvg_table->text( i, 0 ).toDouble();
    ////qDebug("row %d vol %d , table value changed \n ", i, j );
}

Q3ValueVector <double>& CvgLevelWizard::getThrVector ( )
{
    return m_thr_vector;
}

void CvgLevelWizard::accept()
{
    num_thr = num_thr_spinBox->value();
    min_thr = info_min_thr_lineEdit->text().toDouble();
    max_thr = info_max_thr_lineEdit->text().toDouble();
    scan_fra_area = scan_fraction_area_lineEdit->text().toDouble();
    sam_res = init_sample_resolution_lineEdit->text().toInt();

    QDialog::accept();
}

void CvgLevelWizard::languageChange()
{
    setCaption( tr( "Coverage Analysis" ) );
    QString s;

    cvg_gen_param_buttonGroup->setTitle( tr( "General" ) );

    s = tr("Maximum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    info_max_thr_textLabel->setText(s);

    s = tr("Minimum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    info_min_thr_textLabel->setText(s);

    num_thr_textLabel->setText( tr( "Number of Thresholds" ) );
    cvg_adv_param_buttonGroup->setTitle( tr( "Advanced" ) );
    scan_fraction_area_textLabel->setText( tr( "Scan Fractional Area" ) );
    init_sample_resolution_textLabel->setText( tr( "Initial Sample Resolution" ) );
    scan_fraction_area_lineEdit->setText( "0.995"  );
    init_sample_resolution_lineEdit->setText( "16" );
    setTitle( WizardPage1, tr( "Set Coverage Analysis Parameters and Run Simulation" ) );

    cvg_table->horizontalHeader()->setLabel( 0, tr( "threshold" ) );

    helpButton()->setText( tr( "Advanced") + " >>>" );

    setTitle( WizardPage2, tr( "Threshold Settings" ) );

    setFinishEnabled( WizardPage2, true );

    backButton()->setText(tr("&Back") + " <");
    nextButton()->setText(tr("&Next") + " >");
    finishButton()->setText(tr("&Finish"));
    cancelButton()->setText(tr("&Cancel"));
    helpButton()->setText( tr( "Advanced") + " >>>" );
}

void CvgLevelWizard::cancel_btn_clicked()
{
    delete this;
}
