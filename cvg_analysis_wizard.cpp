#include "cvg_analysis_wizard.h"
#include "wisim.h"
#include "pref.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <iostream>

/*
     structional fuction CvgAnalysis::CvgAnalysis
 */
CvgAnalysis::CvgAnalysis( NetworkClass* np_param,int iNoLayer, QWidget* parent, const char* name)
    : Q3Wizard( parent, 0, true ), np( np_param )
{
    std::cout << "CALL CvgAnalysis::CvgAnalysis() " << std::endl;
    
    extension = FALSE;
    iNoLayer2 = iNoLayer;
    m_bNoThrd = false;
    
    if ( !name )
	setName( "CvgAnalysis" );

    /* 
       second page of the wizard
     */
    WizardPage_2 = new QWidget( this, "WizardPage_2" );
    WizardPageLayout_2 = new Q3GridLayout( WizardPage_2, 1, 1, 11, 6, "WizardPageLayout_2"); 

    cvg_gen_param_buttonGroup = new Q3ButtonGroup( WizardPage_2, "cvg_gen_param_buttonGroup" );
    cvg_gen_param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    cvg_gen_param_buttonGroup->layout()->setSpacing( 15 );
    cvg_gen_param_buttonGroup->layout()->setMargin( 11 );
    cvg_gen_param_buttonGroupLayout = new Q3GridLayout( cvg_gen_param_buttonGroup->layout() );
    cvg_gen_param_buttonGroupLayout->setAlignment( Qt::AlignTop );

    if ( iNoLayer == 0) { // layer
        max_layer_textLabel = new QLabel( cvg_gen_param_buttonGroup, "max_layer_textLabel" );
        max_layer_textLabel->setText( tr( "Max Number of Layers" ) );
        cvg_gen_param_buttonGroupLayout->addWidget( max_layer_textLabel, 1, 0 );

        sig_thr_lineEdit = new QLineEdit( cvg_gen_param_buttonGroup, "sig_thr_lineEdit" );
        sig_thr_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, 
                    (QSizePolicy::SizeType)0, 5, 0, sig_thr_lineEdit->sizePolicy().hasHeightForWidth() ) );
        sig_thr_lineEdit->setMinimumSize( QSize( 150, 0 ) );
        sig_thr_lineEdit->setMaximumSize( QSize( 400, 32767 ) );
        sig_thr_lineEdit->setValidator( new QDoubleValidator( sig_thr_lineEdit ) );
        cvg_gen_param_buttonGroupLayout->addWidget( sig_thr_lineEdit, 0, 1 );

        max_layer_spinBox = new QSpinBox( cvg_gen_param_buttonGroup, "max_layer_spinBox" );
        max_layer_spinBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, 
                    (QSizePolicy::SizeType)0, 5, 0, max_layer_spinBox->sizePolicy().hasHeightForWidth() ) );
        max_layer_spinBox->setMinimumSize( QSize( 150, 0 ) );
        max_layer_spinBox->setMaximumSize( QSize( 400, 32767 ) );
        max_layer_spinBox->setMinValue( 1 );
        max_layer_spinBox->setMaxValue( 256 );
        cvg_gen_param_buttonGroupLayout->addWidget( max_layer_spinBox, 1, 1 );

        sig_thr_textLabel = new QLabel( cvg_gen_param_buttonGroup, "sig_thr_textLabel" );
        sig_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, 
                    (QSizePolicy::SizeType)5, 5, 0, sig_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
        cvg_gen_param_buttonGroupLayout->addWidget( sig_thr_textLabel, 0, 0 );

        WizardPageLayout_2->addWidget( cvg_gen_param_buttonGroup, 0, 0 );
    } else { // for PA
        RadioBtnNoThrd  = new QRadioButton( cvg_gen_param_buttonGroup, "threshold_radioButton" );
        RadioBtnNoThrd->setChecked( true );
        RadioBtnNoThrd->setGeometry( QRect( 20, 40, 71, 21 ) );
        RadioBtnNoThrd->setText( tr( "No Threshold" ) );
        cvg_gen_param_buttonGroupLayout->addWidget(RadioBtnNoThrd, 0, 0);
        
        RadioBtnYesThrd = new QRadioButton( cvg_gen_param_buttonGroup, "level_radioButton" );
        RadioBtnYesThrd->setEnabled( true );
        RadioBtnYesThrd->setGeometry( QRect( 20, 80, 96, 21 ) );
        RadioBtnYesThrd->setText( tr( "Use Threshold" ) );
        cvg_gen_param_buttonGroupLayout->addWidget( RadioBtnYesThrd, 0, 1 );

        sig_thr_textLabel = new QLabel( cvg_gen_param_buttonGroup, "sig_thr_textLabel" );
        sig_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, 
                    (QSizePolicy::SizeType)5, 5, 0, sig_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
        cvg_gen_param_buttonGroupLayout->addWidget( sig_thr_textLabel, 1, 0 );

        sig_thr_lineEdit = new QLineEdit( cvg_gen_param_buttonGroup, "sig_thr_lineEdit" );
        sig_thr_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, 
                    (QSizePolicy::SizeType)0, 5, 0, sig_thr_lineEdit->sizePolicy().hasHeightForWidth() ) );
        sig_thr_lineEdit->setMinimumSize( QSize( 150, 0 ) );
        sig_thr_lineEdit->setMaximumSize( QSize( 400, 32767 ) );
        sig_thr_lineEdit->setValidator( new QDoubleValidator( sig_thr_lineEdit ) );
        sig_thr_lineEdit->setEnabled(false);
        cvg_gen_param_buttonGroupLayout->addWidget( sig_thr_lineEdit, 1, 1 );
        
        WizardPageLayout_2->addWidget( cvg_gen_param_buttonGroup, 0, 0 );
    }

    //----------------------------------------------------------------------------------------
    cvg_adv_param_buttonGroup = new Q3ButtonGroup( WizardPage_2, "cvg_adv_param_buttonGroup" );
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

    cvg_adv_param_buttonGroup->setHidden( true );
    //----------------------------------------------------------------------------------------

    WizardPageLayout_2->addWidget( cvg_adv_param_buttonGroup, 1, 0 );
    addPage( WizardPage_2, QString("") );

    languageChange();

    resize( QSize(419, 152).expandedTo(minimumSizeHint()) );
    
    //initial
    cvg_type = QString( tr("Layer"));

    //slot and signal
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( helpButton(), SIGNAL( clicked() ), this, SLOT( adv_button_clicked() ) );
    connect( cvg_gen_param_buttonGroup, SIGNAL( clicked(int) ), this, SLOT( chooseThreshold(int) ) );

    // tab order
    if ( iNoLayer == 0) {
        setTabOrder( sig_thr_lineEdit, max_layer_spinBox );
        setTabOrder( max_layer_spinBox, scan_fraction_area_lineEdit );
        setTabOrder( scan_fraction_area_lineEdit, init_sample_resolution_lineEdit );
    }

    setExtension(cvg_adv_param_buttonGroup);
    setOrientation(Qt::Vertical);
}


CvgAnalysis::~CvgAnalysis()
{
}


void CvgAnalysis::languageChange()
{
    setCaption( tr( "Coverage Analysis" ) );
    helpButton()->setText(tr("Advanced") + " >>>");
    cancelButton()->setText(tr("Cancel"));
    finishButton()->setText(tr ("OK"));
    nextButton()->setText(tr ("Next"));
    
    cvg_gen_param_buttonGroup->setTitle( tr( "General" ) );
    cvg_adv_param_buttonGroup->setTitle( tr( "Advanced" ) );
    scan_fraction_area_textLabel->setText( tr( "Scan Fractional Area" ) );
    init_sample_resolution_textLabel->setText( tr( "Initial Sample Resolution" ) );
    scan_fraction_area_lineEdit->setText( "0.995" );
    init_sample_resolution_lineEdit->setText( "16" );
    setTitle( WizardPage_2, tr( "Set Coverage Analysis Parameters and Run Simulation" ) );

    setFinishEnabled( WizardPage_2, true );
}

void CvgAnalysis::setType( int i )
{
    switch(i)
    {
        case 0:
            cvg_type = QString( tr("Layer") );
            break;
        case 1:
            cvg_type = QString( tr("Level") );
            break;
        case 2:
            cvg_type = QString( tr("SIR") );
            break;
        case 3:
            cvg_type = QString( tr("PA") );
            break;
        default:
            break;
    }
}


void CvgAnalysis::adv_button_clicked()
{
  if (!extension) {
    showExtension(true);
    helpButton()->setText(tr( "<<Advanced" ));    
  }
  else { 
    showExtension(false);
    helpButton()->setText(tr("Advanced") + " >>>" );
  }
  extension = !extension;
}

void CvgAnalysis::accept()
{
  //save these parameters after finish button clicked;
  //cvg_type defined above
  //m_bNoThrd is saved by chooseThreshold;
  
  if (iNoLayer2 != 1) { 
    max_layer      = max_layer_spinBox->value();
    sig_thr        = sig_thr_lineEdit->text().toDouble();
  }
  else if (m_bNoThrd) {
    sig_thr        = sig_thr_lineEdit->text().toDouble();
  }

  scan_fra_area  = scan_fraction_area_lineEdit->text().toDouble();
  sam_res        = init_sample_resolution_lineEdit->text().toInt();
  
  QDialog::accept();
}

void CvgAnalysis::cancel_btn_clicked()
{
    delete this;
}

void CvgAnalysis::chooseThreshold(int i) {

    if ( i == 0 ) {
      m_bNoThrd = false;
      sig_thr_lineEdit->setEnabled(false);
    } else if ( i == 1) {
      m_bNoThrd = true;
      sig_thr_lineEdit->setEnabled(true);
    }	

}
