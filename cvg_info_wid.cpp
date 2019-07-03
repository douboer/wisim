
/******************************************************************************************
**** PROGRAM: cvg_info_wid.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#include <qvariant.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "cvg_info_wid.h"
#include "WiSim.h"

#include <qstring.h>
// added by Wei Ben
#include "cvg_part_disp.h"

/*
    function : CvgInfoLayerWid::CvgInfoLayerWid()
 */
CvgInfoLayerWid::CvgInfoLayerWid( NetworkClass* np_param, QWidget* parent, const char* name )
    : QWidget( parent, name ), np( np_param )
{
  
    if ( !name )
	setName( "CvgInfoLayerWid" );

    // add by Wei Ben
    m_oDispDialog = new disp_cells();

    CvgInfoLayerWidLayout = new Q3GridLayout( this, 1, 1, 11, 6, "CvgInfoLayerWidLayout"); 

    //-----------------------------------------------------------
    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 15 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new Q3GridLayout( buttonGroup1->layout() );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );

    info_cvg_type_textLabel = new QLabel( buttonGroup1, "info_cvg_type_textLabel" );
    info_cvg_type_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_cvg_type_textLabel->sizePolicy().hasHeightForWidth() ) );

    buttonGroup1Layout->addWidget( info_cvg_type_textLabel, 0, 0 );

    // ----------------------------------------------------------
    // added by Wei Ben 
    /*
    layout11 = new QHBoxLayout( 0, 11, 6, "layout11"); 

    chooseAll = new QCheckBox( buttonGroup1, "chooseAll" );
    layout11->addWidget( chooseAll );
    choosePart = new QCheckBox( buttonGroup1, "choosePart" );
    layout11->addWidget( choosePart );
    viewPartList = new QPushButton( buttonGroup1, "viewPartList" );
    layout11->addWidget( viewPartList );

    buttonGroup1Layout->addLayout( layout11, 1, 1);
    */

    layout14 = new Q3GridLayout( 0, 1, 1, 11, 6, "layout14"); 

    viewPartList = new QPushButton( buttonGroup1, "viewPartList" );

    layout14->addMultiCellWidget( viewPartList, 0, 0, 2, 3 );

    chooseAll = new QCheckBox( buttonGroup1, "chooseAll" );
    chooseAll->setEnabled(false);
    layout14->addWidget( chooseAll, 0, 0 );

    choosePart = new QCheckBox( buttonGroup1, "choosePart" );
    choosePart->setEnabled(false);
    layout14->addWidget( choosePart, 0, 1 );

    maxDistLabel = new QLabel( buttonGroup1, "maxDistLabel" );

    layout14->addMultiCellWidget( maxDistLabel, 1, 1, 0, 2 );

    maxDistlineEdit = new QLineEdit( buttonGroup1, "maxDistlineEdit" );
    maxDistlineEdit->setEnabled(false);
    layout14->addWidget( maxDistlineEdit, 1, 3 );
    buttonGroup1Layout->addLayout( layout14, 1, 1);
    // ------------------ finish adding ------------------------



    info_cvg_name_textLabel = new QLabel( buttonGroup1, "info_cvg_name_textLabel" );
    buttonGroup1Layout->addWidget( info_cvg_name_textLabel, 2, 0 );

    info_cvg_type_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_type_lineEdit" );
    info_cvg_type_lineEdit->setEnabled( FALSE );
    info_cvg_type_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_cvg_type_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup1Layout->addWidget( info_cvg_type_lineEdit, 0, 1 );

    info_cvg_name_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_name_lineEdit" );
    info_cvg_name_lineEdit->setEnabled( FALSE );
    buttonGroup1Layout->addWidget( info_cvg_name_lineEdit, 2, 1 );

    CvgInfoLayerWidLayout->addWidget( buttonGroup1, 0, 0 );

    //-----------------------------------------------------------
    buttonGroup2 = new Q3ButtonGroup( this, "buttonGroup2" );
    buttonGroup2->setColumnLayout(0, Qt::Vertical );
    buttonGroup2->layout()->setSpacing( 15 );
    buttonGroup2->layout()->setMargin( 11 );

    buttonGroup2Layout = new Q3GridLayout( buttonGroup2->layout() );
    buttonGroup2Layout->setAlignment( Qt::AlignTop );

    info_cvg_sig_thr_textLabel = new QLabel( buttonGroup2, "info_cvg_sig_thr_textLabel" );
    info_cvg_sig_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_cvg_sig_thr_textLabel->sizePolicy().hasHeightForWidth() ) );

    buttonGroup2Layout->addWidget( info_cvg_sig_thr_textLabel, 0, 0 );

    info_max_layer_textLabel = new QLabel( buttonGroup2, "info_max_layer_textLabel" );
    buttonGroup2Layout->addWidget( info_max_layer_textLabel, 1, 0 );

    info_cvg_sig_thr_lineEdit = new QLineEdit( buttonGroup2, "info_cvg_sig_thr_lineEdit" );
    info_cvg_sig_thr_lineEdit->setEnabled( FALSE );
    info_cvg_sig_thr_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_cvg_sig_thr_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup2Layout->addWidget( info_cvg_sig_thr_lineEdit, 0, 1 );

    info_max_layer_lineEdit = new QLineEdit( buttonGroup2, "info_max_layer_lineEdit" );
    info_max_layer_lineEdit->setEnabled( FALSE );

    buttonGroup2Layout->addWidget( info_max_layer_lineEdit, 1, 1 );

    CvgInfoLayerWidLayout->addWidget( buttonGroup2, 1, 0 );

    //-----------------------------------------------------------
    buttonGroup3 = new Q3ButtonGroup( this, "buttonGroup3" );
    buttonGroup3->setColumnLayout(0, Qt::Vertical );
    buttonGroup3->layout()->setSpacing( 15 );
    buttonGroup3->layout()->setMargin( 11 );

    buttonGroup3Layout = new Q3GridLayout( buttonGroup3->layout() );
    buttonGroup3Layout->setAlignment( Qt::AlignTop );

    info_scan_fraction_area_textLabel = new QLabel( buttonGroup3, "info_scan_fraction_area_textLabel" );
    info_scan_fraction_area_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_scan_fraction_area_textLabel->sizePolicy().hasHeightForWidth() ) );

    buttonGroup3Layout->addWidget( info_scan_fraction_area_textLabel, 0, 0 );

    info_init_sample_resolution_textLabel = new QLabel( buttonGroup3, "info_init_sample_resolution_textLabel" );
    buttonGroup3Layout->addWidget( info_init_sample_resolution_textLabel, 1, 0 );

    info_scan_fraction_area_lineEdit = new QLineEdit( buttonGroup3, "info_scan_fraction_area_lineEdit" );
    info_scan_fraction_area_lineEdit->setEnabled( FALSE );
    info_scan_fraction_area_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_scan_fraction_area_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup3Layout->addWidget( info_scan_fraction_area_lineEdit, 0, 1 );

    info_init_sample_resolution_lineEdit = new QLineEdit( buttonGroup3, "info_init_sample_resolution_lineEdit" );
    info_init_sample_resolution_lineEdit->setEnabled( FALSE );
    buttonGroup3Layout->addWidget( info_init_sample_resolution_lineEdit, 1, 1 );

    CvgInfoLayerWidLayout->addWidget( buttonGroup3, 2, 0 );

    buttonGroup3->hide();

    connect(viewPartList, SIGNAL(clicked()), this, SLOT(part_btn_clicked()));

    languageChange();
    resize( QSize(501, 392).expandedTo(minimumSizeHint()) );

//    connect( info_run_btn, SIGNAL( clicked() ), this, SLOT( run_btn_clicked() ) );

    // tab order
    setTabOrder( info_cvg_type_lineEdit, info_cvg_name_lineEdit );
    setTabOrder( info_cvg_name_lineEdit, info_cvg_sig_thr_lineEdit );
    setTabOrder( info_cvg_sig_thr_lineEdit, info_max_layer_lineEdit );
    setTabOrder( info_max_layer_lineEdit, info_scan_fraction_area_lineEdit );
    setTabOrder( info_scan_fraction_area_lineEdit, info_init_sample_resolution_lineEdit );
//    setTabOrder( info_init_sample_resolution_lineEdit, info_run_btn );
}

/*
    destructor function
 */
CvgInfoLayerWid::~CvgInfoLayerWid()
{
    //done by qt
}

/*
    set text
 */
void CvgInfoLayerWid::languageChange()
{
    setCaption( qApp->translate("CvgInfoLayerWid", "coverage info" ) );
    buttonGroup1->setTitle( qApp->translate("CvgInfoLayerWid", "coverage analysis type and name" ) );
    info_cvg_type_textLabel->setText( qApp->translate("CvgInfoLayerWid", "coverage analysis type" ) );
    info_cvg_name_textLabel->setText( qApp->translate("CvgInfoLayerWid", "coverage analysis name" ) );

    buttonGroup2->setTitle( qApp->translate("CvgInfoLayerWid", "coverage analysis general parameters" ) );
    info_cvg_sig_thr_textLabel->setText( qApp->translate("CvgInfoLayerWid", "sigal threshold" ) );
    info_max_layer_textLabel->setText( qApp->translate("CvgInfoLayerWid", "max layer" ) );

    buttonGroup3->setTitle( qApp->translate("CvgInfoLayerWid", "coverage analysis advance parameters" ) );
    info_scan_fraction_area_textLabel->setText( qApp->translate("CvgInfoLayerWid", "scan fraction area" ) );
    info_init_sample_resolution_textLabel->setText( qApp->translate("CvgInfoLayerWid", "initial sample resolution" ) );

    //added by Wei Ben
    chooseAll->setText( tr( "All" ) );
    choosePart->setText( tr( "Part" ) );
    viewPartList->setText( tr( "View Selected Cell List" ) );
    maxDistLabel->setText( tr( "Max. Distance" ) );

}

void CvgInfoLayerWid::setParam( const QString& cvg_type, const QString& cvg_name, double sig_thr, int max_layer, double scan_fra_area, int sam_res )
{
    QString str;

    info_cvg_type_lineEdit->setText( cvg_type );
    info_cvg_name_lineEdit->setText( cvg_name );
    
    str.sprintf("%f", sig_thr);
    info_cvg_sig_thr_lineEdit->setText( str );

    str.sprintf("%d", max_layer);
    info_max_layer_lineEdit->setText( str );

    str.sprintf("%f", scan_fra_area);
    info_scan_fraction_area_lineEdit->setText( str );

    str.sprintf("%d", sam_res);
    info_init_sample_resolution_lineEdit->setText( str );
}

/*************************************************/
/* Set the parameters for cell multi selection   */
/* Auther: Wei Ben                               */
/* Date  : Nov. 11, 2004                         */
/*************************************************/
void CvgInfoLayerWid::setPartParam( const double distance, const QStringList& dispList, bool isPart) {

  QString str2;
  str2.sprintf("%f", distance);
  
  if (isPart) {
    choosePart->setChecked(true);
    chooseAll->setEnabled(false);
    maxDistlineEdit->setText(str2);
    viewPartList->setEnabled(true);

    // create a window for displaying partlist values
    qDebug(dispList.join("$"));
    m_oDispDialog->setCellList(dispList);
    
    
  }
  else {
    choosePart->setEnabled(false);
    chooseAll->setChecked(true);    
    
    viewPartList->setEnabled(false);
  }


}
 
void CvgInfoLayerWid::part_btn_clicked() {

  m_oDispDialog->exec();

}


// ------------------------------------------------------
// Class Function for class CvgInfoPAWid
// ------------------------------------------------------
/*
    function : CvgInfoLayerWid::CvgInfoLayerWid()
 */
CvgInfoPAWid::CvgInfoPAWid( NetworkClass* np_param, bool bYesThrd, QWidget* parent, const char* name )
    : QWidget( parent, name ), np( np_param )
{
  
    if ( !name )
	setName( "CvgInfoPAWid" );

    // add by Wei Ben
    m_oDispDialog = new disp_cells();

    CvgInfoPAWidLayout = new Q3GridLayout( this, 1, 1, 11, 6, "CvgInfoPAWidLayout"); 

    //-----------------------------------------------------------
    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 15 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new Q3GridLayout( buttonGroup1->layout() );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );

    info_cvg_type_textLabel = new QLabel( buttonGroup1, "info_cvg_type_textLabel" );
    info_cvg_type_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_cvg_type_textLabel->sizePolicy().hasHeightForWidth() ) );

    buttonGroup1Layout->addWidget( info_cvg_type_textLabel, 0, 0 );

    // ----------------------------------------------------------
    // added by Wei Ben 
    layout14 = new Q3GridLayout( 0, 1, 1, 11, 6, "layout14"); 

    viewPartList = new QPushButton( buttonGroup1, "viewPartList" );

    layout14->addMultiCellWidget( viewPartList, 0, 0, 2, 3 );

    chooseAll = new QCheckBox( buttonGroup1, "chooseAll" );
    chooseAll->setEnabled(false);
    layout14->addWidget( chooseAll, 0, 0 );

    choosePart = new QCheckBox( buttonGroup1, "choosePart" );
    choosePart->setEnabled(false);
    layout14->addWidget( choosePart, 0, 1 );

    maxDistLabel = new QLabel( buttonGroup1, "maxDistLabel" );

    layout14->addMultiCellWidget( maxDistLabel, 1, 1, 0, 2 );

    maxDistlineEdit = new QLineEdit( buttonGroup1, "maxDistlineEdit" );
    maxDistlineEdit->setEnabled(false);
    layout14->addWidget( maxDistlineEdit, 1, 3 );
    buttonGroup1Layout->addLayout( layout14, 1, 1);
    // ------------------ finish adding ------------------------



    info_cvg_name_textLabel = new QLabel( buttonGroup1, "info_cvg_name_textLabel" );
    buttonGroup1Layout->addWidget( info_cvg_name_textLabel, 2, 0 );

    info_cvg_type_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_type_lineEdit" );
    info_cvg_type_lineEdit->setEnabled( FALSE );
    info_cvg_type_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_cvg_type_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup1Layout->addWidget( info_cvg_type_lineEdit, 0, 1 );

    info_cvg_name_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_name_lineEdit" );
    info_cvg_name_lineEdit->setEnabled( FALSE );
    buttonGroup1Layout->addWidget( info_cvg_name_lineEdit, 2, 1 );

    CvgInfoPAWidLayout->addWidget( buttonGroup1, 0, 0 );

    //-----------------------------------------------------------
    if( bYesThrd) {
      buttonGroup2 = new Q3ButtonGroup( this, "buttonGroup2" );
      buttonGroup2->setColumnLayout(0, Qt::Vertical );
      buttonGroup2->layout()->setSpacing( 15 );
      buttonGroup2->layout()->setMargin( 11 );
      
      buttonGroup2Layout = new Q3GridLayout( buttonGroup2->layout() );
      buttonGroup2Layout->setAlignment( Qt::AlignTop );
    
      info_cvg_sig_thr_textLabel = new QLabel( buttonGroup2, "info_cvg_sig_thr_textLabel" );
      info_cvg_sig_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_cvg_sig_thr_textLabel->sizePolicy().hasHeightForWidth() ) );

      buttonGroup2Layout->addWidget( info_cvg_sig_thr_textLabel, 0, 0 );

      info_cvg_sig_thr_lineEdit = new QLineEdit( buttonGroup2, "info_cvg_sig_thr_lineEdit" );
      info_cvg_sig_thr_lineEdit->setEnabled( FALSE );
      info_cvg_sig_thr_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_cvg_sig_thr_lineEdit->sizePolicy().hasHeightForWidth() ) );

      buttonGroup2Layout->addWidget( info_cvg_sig_thr_lineEdit, 0, 1 );
      CvgInfoPAWidLayout->addWidget( buttonGroup2, 1, 0 );
    
      buttonGroup2->setTitle( qApp->translate("CvgInfoPAWid", "coverage analysis general parameters" ) );
      info_cvg_sig_thr_textLabel->setText( qApp->translate("CvgInfoPAWid", "sigal threshold" ) );
    
    }

    //-----------------------------------------------------------
    buttonGroup3 = new Q3ButtonGroup( this, "buttonGroup3" );
    buttonGroup3->setColumnLayout(0, Qt::Vertical );
    buttonGroup3->layout()->setSpacing( 15 );
    buttonGroup3->layout()->setMargin( 11 );

    buttonGroup3Layout = new Q3GridLayout( buttonGroup3->layout() );
    buttonGroup3Layout->setAlignment( Qt::AlignTop );

    info_scan_fraction_area_textLabel = new QLabel( buttonGroup3, "info_scan_fraction_area_textLabel" );
    info_scan_fraction_area_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, info_scan_fraction_area_textLabel->sizePolicy().hasHeightForWidth() ) );

    buttonGroup3Layout->addWidget( info_scan_fraction_area_textLabel, 0, 0 );

    info_init_sample_resolution_textLabel = new QLabel( buttonGroup3, "info_init_sample_resolution_textLabel" );
    buttonGroup3Layout->addWidget( info_init_sample_resolution_textLabel, 1, 0 );

    info_scan_fraction_area_lineEdit = new QLineEdit( buttonGroup3, "info_scan_fraction_area_lineEdit" );
    info_scan_fraction_area_lineEdit->setEnabled( FALSE );
    info_scan_fraction_area_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_scan_fraction_area_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup3Layout->addWidget( info_scan_fraction_area_lineEdit, 0, 1 );

    info_init_sample_resolution_lineEdit = new QLineEdit( buttonGroup3, "info_init_sample_resolution_lineEdit" );
    info_init_sample_resolution_lineEdit->setEnabled( FALSE );
    buttonGroup3Layout->addWidget( info_init_sample_resolution_lineEdit, 1, 1 );

    CvgInfoPAWidLayout->addWidget( buttonGroup3, 2, 0 );

    buttonGroup3->hide();

    connect(viewPartList, SIGNAL(clicked()), this, SLOT(part_btn_clicked()));

    languageChange();
    resize( QSize(501, 392).expandedTo(minimumSizeHint()) );

//    connect( info_run_btn, SIGNAL( clicked() ), this, SLOT( run_btn_clicked() ) );

    // tab order
    //    setTabOrder( info_cvg_type_lineEdit, info_cvg_name_lineEdit );
    //    setTabOrder( info_cvg_name_lineEdit, info_cvg_sig_thr_lineEdit );
    //    setTabOrder( info_cvg_sig_thr_lineEdit, info_max_layer_lineEdit );
    //    setTabOrder( info_max_layer_lineEdit, info_scan_fraction_area_lineEdit );
    //    setTabOrder( info_scan_fraction_area_lineEdit, info_init_sample_resolution_lineEdit );
    //    setTabOrder( info_init_sample_resolution_lineEdit, info_run_btn );
}

/*
    destructor function
 */
CvgInfoPAWid::~CvgInfoPAWid()
{
    //done by qt
}

/*
    set text
 */
void CvgInfoPAWid::languageChange()
{
    setCaption( qApp->translate("CvgInfoPAWid", "coverage info" ) );
    buttonGroup1->setTitle( qApp->translate("CvgInfoPAWid", "coverage analysis type and name" ) );
    info_cvg_type_textLabel->setText( qApp->translate("CvgInfoPAWid", "coverage analysis type" ) );
    info_cvg_name_textLabel->setText( qApp->translate("CvgInfoPAWid", "coverage analysis name" ) );

//    info_max_layer_textLabel->setText( qApp->translate("CvgInfoPAWid", "max layer" ) );

    buttonGroup3->setTitle( qApp->translate("CvgInfoPAWid", "coverage analysis advance parameters" ) );
    info_scan_fraction_area_textLabel->setText( qApp->translate("CvgInfoPAWid", "scan fraction area" ) );
    info_init_sample_resolution_textLabel->setText( qApp->translate("CvgInfoPAWid", "initial sample resolution" ) );

    //added by Wei Ben
    chooseAll->setText( tr( "All" ) );
    choosePart->setText( tr( "Part" ) );
    viewPartList->setText( tr( "View Selected Cell List" ) );
    maxDistLabel->setText( tr( "Max. Distance" ) );

}

void CvgInfoPAWid::setParam( const QString& cvg_type, const QString& cvg_name, double sig_thr, double scan_fra_area, int sam_res )
{
    QString str;

    info_cvg_type_lineEdit->setText( cvg_type );
    info_cvg_name_lineEdit->setText( cvg_name );
    
    str.sprintf("%f", sig_thr);
    info_cvg_sig_thr_lineEdit->setText( str );

    str.sprintf("%f", scan_fra_area);
    info_scan_fraction_area_lineEdit->setText( str );

    str.sprintf("%d", sam_res);
    info_init_sample_resolution_lineEdit->setText( str );
}

/******************************
 * no threshold
 *******************************/
void CvgInfoPAWid::setParam( const QString& cvg_type, const QString& cvg_name, double scan_fra_area, int sam_res )
{
    QString str;

    info_cvg_type_lineEdit->setText( cvg_type );
    info_cvg_name_lineEdit->setText( cvg_name );
    
    str.sprintf("%f", scan_fra_area);
    info_scan_fraction_area_lineEdit->setText( str );

    str.sprintf("%d", sam_res);
    info_init_sample_resolution_lineEdit->setText( str );
}

/*************************************************/
/* Set the parameters for cell multi selection   */
/* Auther: Wei Ben                               */
/* Date  : Nov. 11, 2004                         */
/*************************************************/
void CvgInfoPAWid::setPartParam( const double distance, const QStringList& dispList, bool isPart) {

  QString str2;
  str2.sprintf("%f", distance);
  
  if (isPart) {
    choosePart->setChecked(true);
    chooseAll->setEnabled(false);
    maxDistlineEdit->setText(str2);
    viewPartList->setEnabled(true);

    // create a window for displaying partlist values
    qDebug(dispList.join("$"));
    m_oDispDialog->setCellList(dispList);
    
    
  }
  else {
    choosePart->setEnabled(false);
    chooseAll->setChecked(true);    
    
    viewPartList->setEnabled(false);
  }


}
 
void CvgInfoPAWid::part_btn_clicked() {

  m_oDispDialog->exec();

}
