
/******************************************************************************************
**** PROGRAM: cvg_info_level_wid.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#include <qvariant.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3table.h>
#include <qlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <q3whatsthis.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "cvg_info_level_wid.h"
#include "WiSim.h"
#include "cvg_part_disp.h"
#include "pref.h"

/*
    CvgThrTable::CvgThrTable
 */
CvgThrTable::CvgThrTable( NetworkClass* np_param, QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    np = np_param;
    
    if ( !name )
	setName( "CvgThrTable" );

    CvgThrTableLayout = new Q3VBoxLayout( this, 11, 6, "CvgThrTableLayout"); 

    table = new Q3Table( this, "table" );
    table->setReadOnly( true );
    table->setPaletteBackgroundColor( this->paletteBackgroundColor());
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "threshold" ) );
    table->setNumCols( 1 );
    CvgThrTableLayout->addWidget( table );

    layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 

    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer1 );

    cancel_btn = new QPushButton( this, "cancel_btn" );
    layout1->addWidget( cancel_btn );
    cancel_btn->setText( tr("&Cancel") );

    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer2 );

    CvgThrTableLayout->addLayout( layout1 );

    //signals and slots
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );

    resize( QSize(321, 348).expandedTo(minimumSizeHint()) );
}

/*
   destructor function
 */
CvgThrTable::~CvgThrTable()
{
}

/*
   function for set text
 */
void CvgThrTable::languageChange()
{
    setCaption( tr( "Threshold Settings" ) );
    table->horizontalHeader()->setLabel( 0, tr( "Threshold" ) );

    cancel_btn->setText( tr( "&Cancel" ) );
    cancel_btn->setAccel( QKeySequence("Alt+C") );
}

void CvgThrTable::cancel_btn_clicked()
{
    hide();
}


/*
    CvgInfoLevelWid::CvgInfoLevelWid
 */
CvgInfoLevelWid::CvgInfoLevelWid( NetworkClass* np_param,  QWidget* parent, const char* name )
    : QWidget( parent, name)
{
    np = np_param;
    
    if ( !name )
	setName( "CvgInfoLevelWid" );

    thr_tab_dia = new CvgThrTable( np );
    // add by Wei Ben
    m_oDispDialog = new disp_cells();

    CvgInfoLevelWidLayout = new Q3VBoxLayout( this, 11, 6, "CvgInfoLevelWidLayout"); 

    //------------------------------------------------------------------------------------------
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

    // ------------------------------------------------------------
    info_cvg_name_textLabel = new QLabel( buttonGroup1, "info_cvg_name_textLabel" );
    buttonGroup1Layout->addWidget( info_cvg_name_textLabel, 2, 0 );

    info_cvg_type_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_type_lineEdit" );
    info_cvg_type_lineEdit->setEnabled( FALSE );
    info_cvg_type_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, info_cvg_type_lineEdit->sizePolicy().hasHeightForWidth() ) );
    buttonGroup1Layout->addWidget( info_cvg_type_lineEdit, 0, 1 );

    info_cvg_name_lineEdit = new QLineEdit( buttonGroup1, "info_cvg_name_lineEdit" );
    info_cvg_name_lineEdit->setEnabled( FALSE );
    buttonGroup1Layout->addWidget( info_cvg_name_lineEdit, 2, 1 );

    CvgInfoLevelWidLayout->addWidget( buttonGroup1 );

    //------------------------------------------------------------------------------------------
    buttonGroup2 = new Q3ButtonGroup( this, "buttonGroup2" );
    buttonGroup2->setColumnLayout(0, Qt::Vertical );
    buttonGroup2->layout()->setSpacing( 15 );
    buttonGroup2->layout()->setMargin( 11 );
    buttonGroup2Layout = new Q3GridLayout( buttonGroup2->layout() );
    buttonGroup2Layout->setAlignment( Qt::AlignTop );

    info_min_thr_lineEdit = new QLineEdit( buttonGroup2, "info_min_thr_lineEdit" );
    info_min_thr_lineEdit->setEnabled( FALSE );
    buttonGroup2Layout->addWidget( info_min_thr_lineEdit, 1, 1 );

    info_max_thr_textLabel = new QLabel( buttonGroup2, "info_max_thr_textLabel" );
    buttonGroup2Layout->addWidget( info_max_thr_textLabel, 2, 0 );

    info_max_thr_lineEdit = new QLineEdit( buttonGroup2, "info_max_thr_lineEdit" );
    info_max_thr_lineEdit->setEnabled( FALSE );
    buttonGroup2Layout->addWidget( info_max_thr_lineEdit, 2, 1 );

    info_min_thr_textLabel = new QLabel( buttonGroup2, "info_min_thr_textLabel" );
    buttonGroup2Layout->addWidget( info_min_thr_textLabel, 1, 0 );

    layout2 = new Q3HBoxLayout( 0, 0, 15, "layout2"); 

    view_thr_textLabel = new QLabel( buttonGroup2, "view_thr_textLabel" );
    view_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, view_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
    layout2->addWidget( view_thr_textLabel );

    view_pushButton = new QPushButton( buttonGroup2, "view_pushButton" );
    view_pushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 5, 0, view_pushButton->sizePolicy().hasHeightForWidth() ) );
    view_pushButton->setMinimumSize( QSize( 100, 0 ) );
    view_pushButton->setMaximumSize( QSize( 100, 32767 ) );
    layout2->addWidget( view_pushButton );

    info_spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( info_spacer1 );

    buttonGroup2Layout->addMultiCellLayout( layout2, 3, 3, 0, 1 );

    num_thr_textLabel = new QLabel( buttonGroup2, "num_thr_textLabel" );
    num_thr_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, num_thr_textLabel->sizePolicy().hasHeightForWidth() ) );
    buttonGroup2Layout->addWidget( num_thr_textLabel, 0, 0 );

    num_thr_lineEdit = new QLineEdit( buttonGroup2, "num_thr_lineEdit" );
    num_thr_lineEdit->setEnabled( FALSE );
    num_thr_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 4, 0, num_thr_lineEdit->sizePolicy().hasHeightForWidth() ) );

    buttonGroup2Layout->addWidget( num_thr_lineEdit, 0, 1 );
    CvgInfoLevelWidLayout->addWidget( buttonGroup2 );

    //------------------------------------------------------------------------------------------
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
    CvgInfoLevelWidLayout->addWidget( buttonGroup3 );

    buttonGroup3->setHidden( true );

    connect( view_pushButton, SIGNAL( clicked() ), this, SLOT( view_btn_clicked() ) ); 
    connect(viewPartList, SIGNAL(clicked()), this, SLOT(part_btn_clicked()));
    
    languageChange();
    resize( QSize(513, 477).expandedTo(minimumSizeHint()) );
}

/*
    destructor function
 */
CvgInfoLevelWid::~CvgInfoLevelWid()
{
    if( thr_tab_dia ) 
        delete thr_tab_dia;
}

/*
      function for set text
 */
void CvgInfoLevelWid::languageChange()
{
    setCaption( tr( "Coverage Level Type" ) );
    QString s;

    buttonGroup1->setTitle( tr( "Type and Name" ) );
    info_cvg_type_textLabel->setText( tr( "Type" ) );
    info_cvg_name_textLabel->setText( tr( "Name" ) );
    buttonGroup2->setTitle( tr( "General" ) );

    s = tr("Maximum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    info_max_thr_textLabel->setText(s);

    s = tr("Minimum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    info_min_thr_textLabel->setText(s);

    view_thr_textLabel->setText( tr( "View Thresholds" ) );
    view_pushButton->setText( tr( "&View" ) );
    view_pushButton->setAccel( QKeySequence( "Alt+V" ) );
    num_thr_textLabel->setText( tr( "Number of Thresholds" ) );
    buttonGroup3->setTitle( tr( "Advanced" ) );
    info_scan_fraction_area_textLabel->setText( tr( "Scan Fractional Area" ) );
    info_init_sample_resolution_textLabel->setText( tr( "Initial Sample Resolution" ) );

    //added by Wei Ben
    chooseAll->setText( tr( "All" ) );
    choosePart->setText( tr( "Part" ) );
    viewPartList->setText( tr( "View Part List" ) );
    maxDistLabel->setText( tr( "Max. Distance" ) );

}

    
void CvgInfoLevelWid::setParam( const QString& cvg_type, const QString& cvg_name, int num_thr, double min_thr, double max_thr, double scan_fra_area, int sam_res )
{
    QString str;

    info_cvg_type_lineEdit->setText( cvg_type );
    info_cvg_name_lineEdit->setText( cvg_name );
    
    str.sprintf("%d", num_thr);
    num_thr_lineEdit->setText( str );
    
    str.sprintf("%f", min_thr);
    info_min_thr_lineEdit->setText( str );
    
    str.sprintf("%f", max_thr);
    info_max_thr_lineEdit->setText( str );

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
void CvgInfoLevelWid::setPartParam( const double distance, const QStringList& dispList, bool isPart) {

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
 


void CvgInfoLevelWid::view_btn_clicked()
{
/*
     remove all cells first( in other place )
 */
    thr_tab_dia->exec();
}

 
void CvgInfoLevelWid::part_btn_clicked() {

  m_oDispDialog->exec();

}
