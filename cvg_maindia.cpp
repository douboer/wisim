
/******************************************************************************************
**** PROGRAM: cvg_maindia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <math.h>
#include <iostream>

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <q3table.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "cconst.h"
#include "WiSim.h"
#include "WiSim_gui.h"
#include "coverage.h"
#include "cvg_analysis_wizard.h"
#include "cvg_level_wizard.h"
#include "cvg_maindia.h"
#include "cvg_page_set.h"
#include "cvg_part_wizard.h"
#include "cvg_type_name_dia.h"
#include "icons_test.h"
#include "list.h"
#include "pref.h"

#define CGDEBUG  0

/*
   constructor function
 */
CvgAnaDia::CvgAnaDia( FigureEditor* editor_param, QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    std::cout << "CvgAnaDia::CvgAnaDia() \n";
    
    m_select_index = 0;
    type           = undef;

    editor = editor_param;
    np = editor->get_np();

    if ( !name )
	setName( "CvgAnaDia" );
    setMinimumSize( QSize( 650, 350 ) );

    //  Set this property, so all objects will be deleted after call function clear().
    info_layer_wid_vector.setAutoDelete ( true );
    info_level_wid_vector.setAutoDelete ( true );
    info_pa_wid_vector.setAutoDelete ( true );

    m_item_count = 0;

    CvgAnaDiaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "CvgAnaDiaLayout"); 

    ok_cancel_Layout = new Q3HBoxLayout( 0, 0, 6, "ok_cancel_Layout"); 

    QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_Layout->addItem( sp1 );
    // advanced button
    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    ok_cancel_Layout->addWidget( buttonOk );

    QSpacerItem* sp2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_Layout->addItem( sp2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    ok_cancel_Layout->addWidget( buttonCancel );

    QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_Layout->addItem( sp3 );

    CvgAnaDiaLayout->addMultiCellLayout( ok_cancel_Layout, 1, 1, 0, 1 );

    //  toolButton Layout
    toolbtn_listview_layout = new Q3VBoxLayout( 0, 0, 0, "toolbtn_listview_layout"); 

    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, buttonGroup1->sizePolicy().hasHeightForWidth() ) );
    buttonGroup1->setMaximumSize( QSize( 200, 32767 ) );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 6 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new Q3HBoxLayout( buttonGroup1->layout() );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );

    create_toolButton = new QToolButton( buttonGroup1, "create_toolButton" );
    create_toolButton->setIconSet( QIcon( TestIcon::icon_create ) );
    buttonGroup1Layout->addWidget( create_toolButton );
    spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer1 );

    delete_toolButton = new QToolButton( buttonGroup1, "delete_toolButton" );
    delete_toolButton->setIconSet( QIcon( TestIcon::icon_delete ) );
    buttonGroup1Layout->addWidget( delete_toolButton );
    spacer2 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer2 );

    /*
        implement for other coverage tool or setting such as :
        "*     write_coverage_analysis -name name -f filenname               Write coverage analysis results into file         *"
        "*     write_coverage_analysis_db -name name                         Write coverage analysis results into file         *"
        "*     report_coverage_analysis -name name -f filenname              Report coverage analysis data                     *"
        "*     read_coverage_analysis -f filenname
        click this button show or setting these param with dialog which create in cvg_type_page
     */

    toolbtn_listview_layout->addWidget( buttonGroup1 );

    listView = new Q3ListView( this, "listView" );
    listView->addColumn( "TESTS                                                       ");
    listView->header()->setClickEnabled( FALSE, listView->header()->count() - 1 );
    listView->header()->setResizeEnabled( FALSE, listView->header()->count() - 1 );
    listView->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)7, 0, 0, listView->sizePolicy().hasHeightForWidth() ) );
    listView->setMaximumSize( QSize( 200, 32767 ) );
    listView->header()->hide();        
    toolbtn_listview_layout->addWidget( listView );

    // insert item and subitem to listView
    top_level_item = new Q3ListViewItem( listView);
    top_level_item->setOpen( TRUE );

    CvgAnaDiaLayout->addLayout( toolbtn_listview_layout, 0, 0 );

    cvg_info_layout = new Q3VBoxLayout( 0, 0, 0, "cvg_info_layout"); 

    head_lbl = new QLabel( this, "head_lbl" );
    head_lbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 10, 0, head_lbl->sizePolicy().hasHeightForWidth() ) );
    head_lbl->setMinimumSize( QSize( 0, 30 ) );
    head_lbl->setMaximumSize( QSize( 32767, 30 ) );
    head_lbl->setPaletteBackgroundColor( QColor( 85, 170, 255 ) );
    cvg_info_layout->addWidget( head_lbl );

    about_wid = new QWidget;
    about_widLayout = new Q3HBoxLayout( about_wid, 11, 6, "about_widLayout");
    about_lbl = new QLabel( about_wid, "about_lbl" );
    about_widLayout->addWidget( about_lbl);

    cvg_widstack = new Q3WidgetStack( this ); 
    cvg_info_layout->addWidget( cvg_widstack );

    CvgAnaDiaLayout->addLayout( cvg_info_layout, 0, 1 );

    cvg_widstack->addWidget(about_wid);

    // initially read created coverage analysis, and show names on listView and
    // show params on widgetStack.
    // layer and sir-layer can be looked as same type of GUI

    if (np->num_coverage_analysis > 0)
      buttonOk->setDisabled( FALSE );
    else
      buttonOk->setDisabled( TRUE );

    initForm();

    //signal and slot
    connect( create_toolButton, SIGNAL( clicked() ), this, SLOT( create_toolButton_clicked() ) );
    connect( delete_toolButton, SIGNAL( clicked() ), this, SLOT( delete_toolButton_clicked() ) );
    connect( listView, SIGNAL( selectionChanged(Q3ListViewItem* ) ), this, SLOT( listView_selectchanged(Q3ListViewItem* ) ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );

    languageChange();
    resize( QSize(650, 150).expandedTo(minimumSizeHint() ) );

    show();
}

/*
 */
CvgAnaDia::~CvgAnaDia()
{
    qDebug("CvgAnaDia : destructor...  BEGIN\n");

    /* 
       Removes all items from QPtrLists, 
       these items will be deleted, if set QPtrLists property setAutoDelete( true );
     */
    info_layer_wid_vector.clear();
    info_level_wid_vector.clear();
    info_pa_wid_vector.clear();

    qDebug("CvgAnaDia : destructor...  END\n");
}


void CvgAnaDia::languageChange()
{
    setCaption( tr( "Coverage analysis" ) );

    buttonOk->setText( tr( "&Advanced" ) );
    buttonCancel->setText( tr( "&Close" ) );

    buttonGroup1->setTitle( QString::null );

    create_toolButton->setTextLabel( tr("Create") );
    create_toolButton->setAutoRaise( true );

    delete_toolButton->setTextLabel( tr("Delete") );
    delete_toolButton->setAutoRaise( true );

    about_wid->setCaption( tr( "about window" ) );
    about_lbl->setText("<h3><p align=\"center\">" + tr("Coverage Analysis") + "</p></h3>");

    listView->header()->setLabel( 0, tr( "xxxx" ) );

    top_level_item->setText( 0, tr( "Coverage Analysis List" ) );
    head_lbl->setText( tr( "Coverage Analysis" ) );
}

void CvgAnaDia::create_toolButton_clicked()
{
    cvg_type_name_dia = new CvgTypeNameDia( np, 0 );
    cvg_type_name_dia->show();

    connect( cvg_type_name_dia->ok_btn, SIGNAL( clicked() ), this, SLOT( cvg_type_name_dia_ok_btn_clicked() ) );
}

void CvgAnaDia::cvg_type_name_dia_ok_btn_clicked()
{
    uint i;

    cvg_type_name_dia->hide();

    Q3ListViewItemIterator it( top_level_item, Q3ListViewItemIterator::Selected );

    /*
       default value is false, all names are different
       check_name=TRUE dicates that the name of coverage have existed
     */
    bool check_name = FALSE ;    

    QString str = QString("coverage name : %1 \n").arg(cvg_type_name_dia->cvg_name);

    for ( i=0; i<info_layer_wid_vector.count(); i++ ) {
        if ( info_layer_wid_vector.at(i)->nameSubItem() == cvg_type_name_dia->cvg_name ) {  
            check_name = TRUE;
            break;
        } 
    }

    for (i=0; i<info_level_wid_vector.count(); i++ ) {
        if ( info_level_wid_vector.at(i)->nameSubItem() == cvg_type_name_dia->cvg_name ) {
            check_name = TRUE;
            break;
        }
    }

    for (i=0; i<info_pa_wid_vector.count(); i++ ) {
        if ( info_pa_wid_vector.at(i)->nameSubItem() == cvg_type_name_dia->cvg_name ) {
            check_name = TRUE;
            break;
        }
    }
    if ( !check_name ) {
      m_cvg_type = cvg_type_name_dia->cvg_type;  //"level"
      m_cvg_type_idx = cvg_type_name_dia->cvg_type_idx;
      m_cvg_name = cvg_type_name_dia->cvg_name;

      // added by Wei Ben on Nov. 10, 2004 for supplying an option
      // to choose part/all cells
      cvg_part_wizard = new Cvg_Part_Wizard(editor, np);
      cvg_part_wizard->show();
      connect( cvg_part_wizard->finishButton(), SIGNAL( clicked() ), this, SLOT( pre_accept() ) );
      connect( cvg_part_wizard, SIGNAL( chooseAllSig() ), this, SLOT( pre_accept() ) );

      delete cvg_type_name_dia;
    } else {
        delete cvg_type_name_dia;
        QMessageBox::warning( this, "Warning",
                              QString( "The name of coverage analysis has existed!"),
                              QMessageBox::Warning,
                              0
                            );
    }

}

void CvgAnaDia::delete_toolButton_clicked( )
{
    if(m_item_count<=0)
        return;

    uint i;

    Q3ListViewItemIterator it( top_level_item, Q3ListViewItemIterator::Selected );

    Q3PtrListIterator <CvgLayerPage> layer_vector(info_layer_wid_vector);
    Q3PtrListIterator <CvgLevelPage> level_vector(info_level_wid_vector);
    Q3PtrListIterator <CvgPAPage> pa_vector(info_pa_wid_vector);

    m_select_index = 0;

    for ( i=0; i<info_layer_wid_vector.count(); i++ ) {
        if ( info_layer_wid_vector.at(i)->m_cvg_item == it.current() ) {  
            type = isLayer; 
            m_select_index = i;
            break;
        } 
    }

    for (i=0; i<info_level_wid_vector.count(); i++ ) {
      if ( info_level_wid_vector.at(i)->m_cvg_item == it.current() ) {
        type = isLevel;
        m_select_index = i;
        break;
      }
    }

    for ( i=0; i<info_pa_wid_vector.count(); i++ ) {
        if ( info_pa_wid_vector.at(i)->m_cvg_item == it.current() ) {
            type = isPA;
            m_select_index = i;
            break;
        }
    }

    if( type == isLevel ) {
        //delete highlight item and widget
        cvg_widstack->removeWidget( info_level_wid_vector.at(m_select_index)->m_cvg_wid );
        top_level_item->takeItem( info_level_wid_vector.at(m_select_index)->m_cvg_item );
        info_level_wid_vector.remove( m_select_index );

        //call function process_command() to remove the datas about this coverage analysis 
        sprintf(np->line_buf, "delete_coverage_analysis -name %s", m_cvg_name.latin1() );
        np->process_command(np->line_buf);

        m_item_count--;

        //raise another widget in widgetStack
        if ( m_item_count == 0  ){
            cvg_widstack->raiseWidget( about_wid );
            head_lbl->setText("coverage analysis");

            buttonOk->setDisabled( TRUE );

        } else {
            buttonOk->setDisabled( FALSE );
            if ( info_level_wid_vector.at(0) )
            {
                cvg_widstack->raiseWidget( info_level_wid_vector.at(0)->m_cvg_wid );   
                head_lbl->setText( info_level_wid_vector.at(0)->nameSubItem());
                //liutao,070601
                listView_selectchanged( top_level_item );
                buttonOk->setDisabled( TRUE );
               return;
            } else if ( info_layer_wid_vector.at(0) ) {
                cvg_widstack->raiseWidget( info_layer_wid_vector.at(0)->m_cvg_wid );   
                head_lbl->setText( info_layer_wid_vector.at(0)->nameSubItem());
            }
        }
    } 
    else if ( type == isLayer )
    {
        //delete highlight item and widget
        cvg_widstack->removeWidget( info_layer_wid_vector.at(m_select_index)->m_cvg_wid );
        top_level_item->takeItem( info_layer_wid_vector.at(m_select_index)->m_cvg_item );
        info_layer_wid_vector.remove( m_select_index );

        //call function process_command() to remove the datas about this coverage analysis 
        sprintf(np->line_buf, "delete_coverage_analysis -name %s", m_cvg_name.latin1() );
        np->process_command(np->line_buf);

        m_item_count--;

        //raise another widget in widgetStack
        if ( m_item_count == 0  ){
            cvg_widstack->raiseWidget( about_wid );
            head_lbl->setText("coverage analysis");

            buttonOk->setDisabled( TRUE );
        } else {
            buttonOk->setDisabled( FALSE );
            if ( info_level_wid_vector.at(0) )
            {
                cvg_widstack->raiseWidget( info_level_wid_vector.at(0)->m_cvg_wid );   
                head_lbl->setText( info_level_wid_vector.at(0)->nameSubItem());
                //liutao,070601
                listView_selectchanged( top_level_item );
                buttonOk->setDisabled( TRUE );
                return;
            } else if ( info_layer_wid_vector.at(0) ) {
                cvg_widstack->raiseWidget( info_layer_wid_vector.at(0)->m_cvg_wid );   
                head_lbl->setText( info_layer_wid_vector.at(0)->nameSubItem());
            }
        }
    }
    else if ( type == isPA) {

        //delete highlight item and widget
        cvg_widstack->removeWidget( info_pa_wid_vector.at(m_select_index)->m_cvg_wid );
        top_level_item->takeItem( info_pa_wid_vector.at(m_select_index)->m_cvg_item );
        info_pa_wid_vector.remove( m_select_index );

        //call function process_command() to remove the datas about this coverage analysis
        sprintf(np->line_buf, "delete_coverage_analysis -name %s", m_cvg_name.latin1() );
        np->process_command(np->line_buf);

        m_item_count--;

        //raise another widget in widgetStack
        if ( m_item_count == 0  ){
            cvg_widstack->raiseWidget( about_wid );
            head_lbl->setText("coverage analysis");

            buttonOk->setDisabled( TRUE );
        } else {
            buttonOk->setDisabled( FALSE );
            if ( info_level_wid_vector.at(0) )
            {
                cvg_widstack->raiseWidget( info_level_wid_vector.at(0)->m_cvg_wid );
                head_lbl->setText( info_level_wid_vector.at(0)->nameSubItem());
                //liutao,070601
                listView_selectchanged( top_level_item );
                buttonOk->setDisabled( TRUE );
                return;
            } else if ( info_pa_wid_vector.at(0) ) {
                cvg_widstack->raiseWidget( info_pa_wid_vector.at(0)->m_cvg_wid );
                head_lbl->setText( info_pa_wid_vector.at(0)->nameSubItem());
            }
        }
    }
    //liutao,070601
    listView_selectchanged( top_level_item );
    buttonOk->setDisabled( TRUE );
    
}

void CvgAnaDia::accept( ) 
{
    if ( !checkParam() ) return;

    if(m_cvg_type_idx == CConst::levelCoverage)
      doLevelCvg();

    if (m_cvg_type_idx == CConst::layerCoverage)
      doLayerCvg();

    if (m_cvg_type_idx == CConst::sirLayerCoverage)
      doSirCvg();

    if (m_cvg_type_idx == CConst::pagingAreaCoverage)
      doPACvg();
}




void CvgAnaDia::listView_selectchanged( Q3ListViewItem* v_listitem )
{
    uint i;
    bool bFound = false;

    Q3ListViewItemIterator it( top_level_item, Q3ListViewItemIterator::Selected );

    Q3PtrListIterator <CvgLayerPage> layer_vector(info_layer_wid_vector);
    Q3PtrListIterator <CvgLevelPage> level_vector(info_level_wid_vector);
    Q3PtrListIterator <CvgPAPage> pa_vector(info_pa_wid_vector);

    m_select_index = 0;
    type = undef;

    for ( i=0; i<info_layer_wid_vector.count(); i++ ) {
        if ( info_layer_wid_vector.at(i)->m_cvg_item == it.current() ) {
            std::cout << " info_layer_wid_vector.at(i)->m_cvg_item == it.current() " << std::endl;
            cvg_widstack->raiseWidget( info_layer_wid_vector.at(i)->m_cvg_wid);
            head_lbl->setText( info_layer_wid_vector.at(i)->nameSubItem());
            type = isLayer;
            m_select_index = i;
            m_cvg_name = info_layer_wid_vector.at(i)->nameSubItem();
            // std::cout << m_cvg_name << " " << m_select_index << std::endl;
            bFound = true;
            break;
        }
    }
    if ( ! bFound ) {
        for (i=0; i<info_level_wid_vector.count(); i++ ) {
            if ( info_level_wid_vector.at(i)->m_cvg_item == it.current() ) {
                std::cout << " info_level_wid_vector.at(i)->m_cvg_item == it.current() " << std::endl;
                cvg_widstack->raiseWidget( info_level_wid_vector.at(i)->m_cvg_wid);
                head_lbl->setText( info_level_wid_vector.at(i)->nameSubItem());
                type = isLevel; 
                m_select_index = i;
                m_cvg_name = info_level_wid_vector.at(i)->nameSubItem(); 
                // std::cout << m_cvg_name << " " << m_select_index << std::endl;
                bFound = true;
                break;
            }
        }
    }
    if ( ! bFound) {
      for ( i=0; i<info_pa_wid_vector.count(); i++ ) {
        if ( info_pa_wid_vector.at(i)->m_cvg_item == it.current() ) {
            std::cout << " info_pa_wid_vector.at(i)->m_cvg_item == it.current() " << std::endl;
            cvg_widstack->raiseWidget( info_pa_wid_vector.at(i)->m_cvg_wid);
            head_lbl->setText( info_pa_wid_vector.at(i)->nameSubItem());
            type = isPA;
            m_select_index = i;
            m_cvg_name = info_pa_wid_vector.at(i)->nameSubItem();
            //std::cout << m_cvg_name << " " << m_select_index << std::endl;
            break;
        }
      }

    }

    buttonOk->setDisabled( FALSE );

    if ( v_listitem == top_level_item )
    {
        cvg_widstack->raiseWidget( about_wid );
        head_lbl->setText("Coverage analysis");

        buttonOk->setDisabled( TRUE );
    } else {
        buttonOk->setDisabled( FALSE );
    }
}

void CvgAnaDia::cancel_btn_clicked( )
{
    delete this;
}

//andvance button clicken, and let the anvance item extension
void CvgAnaDia::ok_btn_clicked()
{
    if ( type == isLayer ) {
        if ( info_layer_wid_vector.at(m_select_index)->extension ) {
            info_layer_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( TRUE );
            info_layer_wid_vector.at(m_select_index)->extension = FALSE;
        } else {
            info_layer_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( FALSE );
            info_layer_wid_vector.at(m_select_index)->extension = TRUE;
        }

        resize( QSize( 650, 150 ) );
        updateGeometry ();
    } else if ( type == isLevel ) {
        if ( info_level_wid_vector.at(m_select_index)->extension ) {
            info_level_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( TRUE );
            info_level_wid_vector.at(m_select_index)->extension = FALSE;
        } else {
            info_level_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( FALSE );
            info_level_wid_vector.at(m_select_index)->extension = TRUE;
        }

        resize( QSize( 650, 150 ) );
        updateGeometry ();
    } else if ( type == isPA ) {
        if ( info_pa_wid_vector.at(m_select_index)->extension ) {
            info_pa_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( TRUE );
            info_pa_wid_vector.at(m_select_index)->extension = FALSE;
        } else {
            info_pa_wid_vector.at(m_select_index)->m_cvg_wid->buttonGroup3->setHidden( FALSE );
            info_pa_wid_vector.at(m_select_index)->extension = TRUE;
        }

        resize( QSize( 650, 150 ) );
        updateGeometry ();
    }
}

/*******************************************************************/
/* pre_accept() function is added to reponse the cvg_part_wizard   */
/* finishButton() slot and chooseAllSig() slot.                    */
/* Auther: Wei Ben                                                 */
/* Date  : Nov. 10, 2004                                           */
/* Modifed: Dec. 15, 2004 for adding Paging Area Coverage          */
/*******************************************************************/
void CvgAnaDia::pre_accept() {
    QString s;
    
    m_bChoosePart = cvg_part_wizard->getChoice();
    m_use_gpm = cvg_part_wizard->useGPMCheckBox->isChecked();
    
    if (m_bChoosePart) {
        m_vDisplayList = cvg_part_wizard->getPartCellList();
        m_vPartList =  cvg_part_wizard->getPartCellVector();
        m_dPartDistance = cvg_part_wizard->getMaxDist();
    }
    
    delete cvg_part_wizard;
    
    if (m_cvg_type_idx == CConst::levelCoverage) {
        
        cvg_level_wizard = new CvgLevelWizard( np ); 
        cvg_level_wizard->show();
        connect( cvg_level_wizard->finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );
        printf("begin to run commands for LevelCoverage\n");
    }
    
    if (m_cvg_type_idx == CConst::pagingAreaCoverage ) {
        // deal with paging area coverage analysis. Dec.15, 2004
        
        cvg_layer_wizard = new CvgAnalysis( np, 1 );
        
        s = tr("Signal Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
        cvg_layer_wizard->sig_thr_textLabel->setText(s);
        cvg_layer_wizard->sig_thr_lineEdit->setText( QString("%1").arg( (26.0-110.0-10.0*log(2.0)/log(10.0)) - np->preferences->pwr_offset) );
        
        cvg_layer_wizard->setType(3);
        cvg_layer_wizard->show();
        connect( cvg_layer_wizard->finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );
        
        printf(" begin to run the commands for PA analysis\n");
    }
    
    if ( m_cvg_type_idx == CConst::layerCoverage ) {
        
        cvg_layer_wizard = new CvgAnalysis( np,0 );
        s = tr("Signal Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
        cvg_layer_wizard->sig_thr_textLabel->setText(s);
        cvg_layer_wizard->sig_thr_lineEdit->setText( QString("%1").arg( (26.0-110.0-10.0*log(2.0)/log(10.0)) - np->preferences->pwr_offset) );
        cvg_layer_wizard->setType(0);
        cvg_layer_wizard->show();
        connect( cvg_layer_wizard->finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );
        
        
    }
    if ( m_cvg_type_idx == CConst::sirLayerCoverage ) {
        
        cvg_layer_wizard = new CvgAnalysis( np,0 );
        cvg_layer_wizard->setType(2);
        s = tr("SIR Threshold (dB)");
        cvg_layer_wizard->sig_thr_textLabel->setText(s);
        cvg_layer_wizard->sig_thr_lineEdit->setText( QString("%1").arg(17.0) );
        cvg_layer_wizard->show();
        connect( cvg_layer_wizard->finishButton(), SIGNAL( clicked() ), this, SLOT( accept() ) );
        
    }
}


/****************************************/
/* added by Wei Ben on Nov. 10, 2004    */
/* create the '0 1 2 3' string          */
/****************************************/
void CvgAnaDia::createCmd(char *buf) {
    char *begin_end = "\'";
    char *divid = " ";
    unsigned int i;
    sprintf(buf, "%s", begin_end);
    for ( i = 0; i < m_vPartList.size(); i++) {
        if (m_vPartList[i] != -1) {
            if (i == 0)
                sprintf(buf, "%s%d", buf, m_vPartList[i]);
            else
                sprintf(buf, "%s%s%d", buf, divid, m_vPartList[i]);
        }
    }
    sprintf(buf, "%s%s", buf, begin_end);
    printf ("%s is string\n",buf);
}


// ---------------------------------------
// for easy maitain
// Auther: Wei Ben
// Date  : Dec. 17, 2004
// ---------------------------------------
void CvgAnaDia::doLevelCvg()
{
    std::cout << "CvgAnaDia::doLevelCvg() \n";
    
    int i;
    char buf[500]; // assume 500 is enough for cell list
    cvg_level_wizard->hide();
    cvg_level_page = new CvgLevelPage( np, this );
    
    m_item_count++;
    
    cvg_level_page->item_index = m_item_count-1;
    cvg_level_page->setNameSubItem( m_cvg_name );
    m_num_thr       = cvg_level_wizard->num_thr;
    m_min_thr       = cvg_level_wizard->min_thr;
    m_max_thr       = cvg_level_wizard->max_thr;
    m_scan_fra_area = cvg_level_wizard->scan_fra_area;
    m_sam_res       = cvg_level_wizard->sam_res;
    
    m_thr_vector    = cvg_level_wizard->getThrVector();
    
    //call process_command to create level type of coverage
    sprintf(np->line_buf, "create_coverage_analysis -name %s -type level",
            m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -num_scan_type %d",
            m_cvg_name.latin1(), m_num_thr+ 1);
    np->process_command(np->line_buf);
    
    // -----------------------------------------------------
    // added by Wei Ben for part cells simulation
    // e.g. set_coverage_analysis -name level_type_0 -cell '0 1 2' -dmax 100.0
    // Notice: the cell list is saved in m_vPartList;
    //         the dmax is saved in m_dPartDistance;
    if (m_bChoosePart) {
        createCmd(buf);
        sprintf(np->line_buf, "set_coverage_analysis -name %s -cell %s -dmax %f",
                m_cvg_name.latin1(), buf, m_dPartDistance);
        printf("%s = the command\n", np->line_buf);
        np->process_command(np->line_buf);
    }
    // finish adding
    // -------------------------------------------------------

    sprintf(np->line_buf, "set_coverage_analysis -name %s -use_gpm %d", m_cvg_name.latin1(), (m_use_gpm ? 1 : 0));
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            m_cvg_name.latin1(), m_scan_fra_area, m_sam_res);
    np->process_command(np->line_buf);
    
    //remove all thr_tab_dia cell first
    int rows = cvg_level_page->m_cvg_wid->thr_tab_dia->table->numRows();
    for ( i=0; i<rows; i++ ) {
        cvg_level_page->m_cvg_wid->thr_tab_dia->table->removeRow( 0 );                        //must be zero, not i
    }
    
    //write threshold value to view-thr_tab_dia
    QString thr_str;

    //std::cout << "thr_str " << thr_str << std::endl;
    std::cout << "m_thr_vector.count() " << m_thr_vector.count() << std::endl;

    for( i=0; i<(int) m_thr_vector.count(); i++ )
    {
        thr_str.sprintf( "%f", m_thr_vector[i]);
        
        //XXXXX MOD 
        // std::cout << "thr_str " << thr_str << std::endl;
        
        cvg_level_page->m_cvg_wid->thr_tab_dia->table->insertRows( i );
        cvg_level_page->m_cvg_wid->thr_tab_dia->table->setText( i, 0, thr_str );
        
        //call process_command() to set threshold value
        sprintf(np->line_buf, "set_coverage_analysis -name %s -thr_idx %d -threshold_db %f",
                m_cvg_name.latin1(), i, thr_str.toDouble()+np->preferences->pwr_offset );
        np->process_command(np->line_buf);
        
        thr_str.sprintf( "threshold_%d", i );
        Q3Header *th = cvg_level_page->m_cvg_wid->thr_tab_dia->table->verticalHeader();
        th->setLabel( i, thr_str );
    }
    sprintf(np->line_buf, "run_coverage -name %s", m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    delete cvg_level_wizard;
    
    //   write above params to cvg_info_wid which is included in cvg_level_page(is a CvgPage object),
    //   and add page to widgetstack
    cvg_level_page->m_cvg_wid->setParam( m_cvg_type, m_cvg_name, m_num_thr, m_min_thr, m_max_thr, m_scan_fra_area, m_sam_res );
    // added by Wei Ben: only one line
    cvg_level_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
    
    top_level_item->insertItem( cvg_level_page->m_cvg_item );
    cvg_level_page->m_cvg_item->setText(0, m_cvg_name);
    //add page to widgetstack and show the added dialog
    cvg_widstack->addWidget( cvg_level_page->m_cvg_wid );
    cvg_widstack->raiseWidget( cvg_level_page->m_cvg_wid);
    head_lbl->setText( m_cvg_name );
    info_level_wid_vector.append( cvg_level_page );
}

void CvgAnaDia::doLayerCvg() {
    QString s;
    
    char buf[500]; // assume 500 is enough for cell list
    
    cvg_layer_wizard->hide();
    
    m_item_count++;
    m_scan_fra_area = cvg_layer_wizard->scan_fra_area;
    m_sam_res       = cvg_layer_wizard->sam_res;
    
    cvg_layer_page = new CvgLayerPage( np, this );
    
    cvg_layer_page->item_index = m_item_count-1;
    cvg_layer_page->setNameSubItem( m_cvg_name );
    m_sig_thr       = cvg_layer_wizard->sig_thr;
    m_max_layer     = cvg_layer_wizard->max_layer;
    
    sprintf(np->line_buf, "create_coverage_analysis -name %s -type layer",
            m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    s = tr("Signal Threshold ") + " (" + QString(np->preferences->pwr_str_short) + ")";
    cvg_layer_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText(s);
    //m_sig_thr       += np->preferences->pwr_offset;
    sprintf(np->line_buf, "set_coverage_analysis -name %s -threshold_db %f -num_scan_type %d",
            m_cvg_name.latin1(), m_sig_thr + np->preferences->pwr_offset, m_max_layer + 1);
    np->process_command(np->line_buf);
    
    
    // -----------------------------------------------------
    // added by Wei Ben for part cells simulation
    // e.g. set_coverage_analysis -name level_type_0 -cell '0 1 2' -dmax 100.0
    // Notice: the cell list is saved in m_vPartList;
    //         the dmax is saved in m_dPartDistance;
    if (m_bChoosePart) {
        createCmd(buf);
        sprintf(np->line_buf, "set_coverage_analysis -name %s -cell %s -dmax %f",
                m_cvg_name.latin1(), buf, m_dPartDistance);
        printf("%s = the command\n", np->line_buf);
        np->process_command(np->line_buf);
    }
    // finish adding
    // -------------------------------------------------------
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -use_gpm %d", m_cvg_name.latin1(), (m_use_gpm ? 1 : 0));
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            m_cvg_name.latin1(), m_scan_fra_area, m_sam_res );
    np->process_command(np->line_buf);
    
    delete cvg_layer_wizard;
    
    
    cvg_layer_page->m_cvg_wid->setParam( m_cvg_type, m_cvg_name, m_sig_thr, m_max_layer, m_scan_fra_area, m_sam_res );
    // added by Wei Ben: only one line
    cvg_layer_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
    
    top_level_item->insertItem( cvg_layer_page->m_cvg_item );
    cvg_layer_page->m_cvg_item->setText(0, m_cvg_name);
    
    //add page to widgetstack and show the added dialog
    cvg_widstack->addWidget( cvg_layer_page->m_cvg_wid );
    cvg_widstack->raiseWidget( cvg_layer_page->m_cvg_wid);
    
    
    head_lbl->setText( m_cvg_name );
    
    sprintf(np->line_buf, "run_coverage -name %s", m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    info_layer_wid_vector.append( cvg_layer_page );       //add to vector
    
    
}

/*
 *  the SIR Coverage Commands
 */
void CvgAnaDia::doSirCvg() {
    
    char buf[500]; // assume 500 is enough for cell list
    
    cvg_layer_wizard->hide();
    
    m_item_count++;
    m_scan_fra_area = cvg_layer_wizard->scan_fra_area;
    m_sam_res       = cvg_layer_wizard->sam_res;
    
    cvg_layer_page = new CvgLayerPage( np, this );
    cvg_layer_page->item_index = m_item_count-1;
    cvg_layer_page->setNameSubItem( m_cvg_name );
    m_sig_thr       = cvg_layer_wizard->sig_thr;
    m_max_layer     = cvg_layer_wizard->max_layer;
    
    sprintf(np->line_buf, "create_coverage_analysis -name %s -type sir_layer",
            m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    cvg_layer_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText( "SIR Threshold (dB)" );
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s", m_cvg_name.latin1());
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -threshold_db %f -num_scan_type %d",
            m_cvg_name.latin1(), m_sig_thr, m_max_layer + 1);
    np->process_command(np->line_buf);
    
    // -----------------------------------------------------
    // added by Wei Ben for part cells simulation
    // e.g. set_coverage_analysis -name level_type_0 -cell '0 1 2' -dmax 100.0
    // Notice: the cell list is saved in m_vPartList;
    //         the dmax is saved in m_dPartDistance;
    if (m_bChoosePart) {
        createCmd(buf);
        sprintf(np->line_buf, "set_coverage_analysis -name %s -cell %s -dmax %f",
                m_cvg_name.latin1(), buf, m_dPartDistance);
        printf("%s = the command\n", np->line_buf);
        np->process_command(np->line_buf);
    }
    // finish adding
    // -------------------------------------------------------
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -use_gpm %d", m_cvg_name.latin1(), (m_use_gpm ? 1 : 0));
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            m_cvg_name.latin1(), m_scan_fra_area, m_sam_res );
    np->process_command(np->line_buf);
    
    delete cvg_layer_wizard;
    
    
    //   write above params to cvg_info_wid which is included in cvg_page(is a CvgPage object)  ;
    
    cvg_layer_page->m_cvg_wid->setParam( m_cvg_type, m_cvg_name, m_sig_thr, m_max_layer, m_scan_fra_area, m_sam_res );
    // added by Wei Ben: only one line
    cvg_layer_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
    
    top_level_item->insertItem( cvg_layer_page->m_cvg_item );
    cvg_layer_page->m_cvg_item->setText(0, m_cvg_name);
    
    //add page to widgetstack and show the added dialog
    cvg_widstack->addWidget( cvg_layer_page->m_cvg_wid );
    cvg_widstack->raiseWidget( cvg_layer_page->m_cvg_wid);
    
    head_lbl->setText( m_cvg_name );
    
    sprintf(np->line_buf, "run_coverage -name %s", m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    info_layer_wid_vector.append( cvg_layer_page );       //add to vector
}


/*********************************************
 * PA Coverage commands
 *
 *********************************************/
void CvgAnaDia::doPACvg() {
    
    QString s;
    
    char buf[500]; // assume 500 is enough for cell list
    cvg_layer_wizard->hide();
    
    m_item_count++;
    
    m_scan_fra_area = cvg_layer_wizard->scan_fra_area;
    m_sam_res       = cvg_layer_wizard->sam_res;
    
    //call process_command() to create new layer type of coverage, include PA
    // added PA on Dec. 15, 2004
    cvg_pa_page = new CvgPAPage( np,cvg_layer_wizard->m_bNoThrd, this );
    cvg_pa_page->item_index = m_item_count-1;
    cvg_pa_page->setNameSubItem( m_cvg_name );
    
    sprintf(np->line_buf, "create_coverage_analysis -name %s -type paging_area",
            m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    if (cvg_layer_wizard->m_bNoThrd) {
        s = tr("Signal Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
        cvg_pa_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText(s);
        m_sig_thr       = cvg_layer_wizard->sig_thr;
        sprintf(np->line_buf, "set_coverage_analysis -name %s -threshold_db %f",
                m_cvg_name.latin1(), m_sig_thr + np->preferences->pwr_offset);
        np->process_command(np->line_buf);
    }
    //else {
    //  sprintf(np->line_buf, "set_coverage_analysis -name %s", m_cvg_name.latin1());
    //  np->process_command(np->line_buf);
    //}
    
    
    // -----------------------------------------------------
    // added by Wei Ben for part cells simulation
    // e.g. set_coverage_analysis -name level_type_0 -cell '0 1 2' -dmax 100.0
    // Notice: the cell list is saved in m_vPartList;
    //         the dmax is saved in m_dPartDistance;
    if (m_bChoosePart) {
        createCmd(buf);
        sprintf(np->line_buf, "set_coverage_analysis -name %s -cell %s -dmax %f",
                m_cvg_name.latin1(), buf, m_dPartDistance);
        printf("%s = the command\n", np->line_buf);
        np->process_command(np->line_buf);
    }
    // finish adding
    // -------------------------------------------------------
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -use_gpm %d", m_cvg_name.latin1(), (m_use_gpm ? 1 : 0));
    np->process_command(np->line_buf);
    
    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            m_cvg_name.latin1(), m_scan_fra_area, m_sam_res );
    np->process_command(np->line_buf);
    
    delete cvg_layer_wizard;
    
    
    //   write above params to cvg_info_wid which is included in cvg_page(is a CvgPage object)  ;
    if (cvg_layer_wizard->m_bNoThrd)
        cvg_pa_page->m_cvg_wid->setParam( m_cvg_type, m_cvg_name, m_sig_thr,  m_scan_fra_area, m_sam_res );
    else
        cvg_pa_page->m_cvg_wid->setParam( m_cvg_type, m_cvg_name, m_scan_fra_area, m_sam_res );
    
    cvg_pa_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
    
    top_level_item->insertItem( cvg_pa_page->m_cvg_item );
    cvg_pa_page->m_cvg_item->setText(0, m_cvg_name);
    
    //add page to widgetstack and show the added dialog
    cvg_widstack->addWidget( cvg_pa_page->m_cvg_wid );
    cvg_widstack->raiseWidget( cvg_pa_page->m_cvg_wid);
    
    head_lbl->setText( m_cvg_name );
    
    sprintf(np->line_buf, "run_coverage -name %s", m_cvg_name.latin1() );
    np->process_command(np->line_buf);
    
    info_pa_wid_vector.append( cvg_pa_page );       //add to vector
    
}



/*
 * check the validity of the parameters for different type of coverage analysis
 *
 */
bool CvgAnaDia::checkParam() {
    double m_scan = 0.995;
    int    m_samp = 1;

    if(m_cvg_type_idx == CConst::levelCoverage) {
        m_scan = cvg_level_wizard->scan_fra_area;
        m_samp = cvg_level_wizard->sam_res;
    }
    if (m_cvg_type_idx == CConst::layerCoverage) {
        m_scan = cvg_layer_wizard->scan_fra_area;
        m_samp = cvg_layer_wizard->sam_res;
    }

    if (m_cvg_type_idx == CConst::sirLayerCoverage) {
        m_scan = cvg_layer_wizard->scan_fra_area;
        m_samp = cvg_layer_wizard->sam_res;
    }
    if (m_cvg_type_idx == CConst::pagingAreaCoverage) {
        m_scan = cvg_layer_wizard->scan_fra_area;
        m_samp = cvg_layer_wizard->sam_res;
    }

    if ( m_scan < 0 || m_scan > 1 ) 
    {
        QMessageBox::warning( this, "Warning",
                QString( tr("The value of \"Scan Fractional Area\" must between 0 and 1")),
                QMessageBox::Warning,
                0
                );
        return false;
    }

    if ( m_samp < 1 ) 
    {
        QMessageBox::warning( this, "Warning",
                QString( tr("The value of \"Initial Sample Resolution\" must large than 1")),
                QMessageBox::Warning,
                0
                );
        return false;
    }

    return true;
}


/********************************************
 * filling the listview according to the number_coverage_anlysis
 * also the info. widgets
 *********************************************/

void CvgAnaDia::initForm() {

    std::cout << "CvgAnaDia::initForm()\n";
    
    QString s;
    int i = 0;
    for( int cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++ ) {
        
        m_cvg_name.sprintf( "%s", np->coverage_list[cvg_idx]->strid );

        // the coverage analysis do not be created successfully
        if ( np->coverage_list[cvg_idx]->polygon_list == 0 )
            continue;

        if( np->coverage_list[cvg_idx]->type == CConst::layerCoverage || \
                np->coverage_list[cvg_idx]->type == CConst::sirLayerCoverage ) {
            
            m_item_count++;
            m_select_index = m_item_count-1;
            type = isLayer;
            
            cvg_layer_page = new CvgLayerPage( np, this );
            
            cvg_layer_page->item_index = m_item_count-1;
            cvg_layer_page->setNameSubItem( m_cvg_name );
            
            
            if (np->coverage_list[cvg_idx]->type == CConst::layerCoverage) {
                m_sig_thr       = 10*log(np->coverage_list[cvg_idx]->threshold)/log(10.0);
                m_sig_thr       = m_sig_thr -  np->preferences->pwr_offset;
            }
            else {
                m_sig_thr       = 10*log(np->coverage_list[cvg_idx]->threshold)/log(10.0);

                // Commented out 05.06.07 -- MFK
                // SIR threshold is relative value, not absolute value, therefore it should not be modified
                // by power offset.
                // m_sig_thr       = m_sig_thr -  np->preferences->pwr_offset;                // MOD 05.3.7
            }
            
            m_max_layer     = np->coverage_list[cvg_idx]->scan_list->getSize()-1;
            m_scan_fra_area = np->coverage_list[cvg_idx]->scan_fractional_area;
            m_sam_res       = np->coverage_list[cvg_idx]->init_sample_res;
            
            // ---------------------------------------------------
            // get the cell list from np object; Added by Wei Ben
            // get the displayed cells list.
            m_dPartDistance = np->coverage_list[cvg_idx]->dmax;
            QString ss;
            ListClass<int> *cvg_cells = np->coverage_list[cvg_idx]->cell_list;
            if (cvg_cells != NULL) { // for choose part cells
                m_bChoosePart = true;
                m_vDisplayList.clear();
                for( int kk = 0; kk < cvg_cells->getSize(); kk++) {
                    ss.sprintf("Cell_%d", (*cvg_cells)[kk]);
                    m_vDisplayList.push_back(ss);
                }
                
                cvg_layer_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            else { // for choose All
                m_bChoosePart = false;
                m_dPartDistance = -1;
                m_vDisplayList.clear();
                cvg_layer_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            // finish adding
            // ---------------------------------------------------
            
            
            if ( np->coverage_list[cvg_idx]->type == CConst::layerCoverage ) {
                cvg_layer_page->m_cvg_wid->setParam( QString(tr("Layer") ), m_cvg_name, m_sig_thr, m_max_layer, m_scan_fra_area, m_sam_res );
                s = tr("Signal Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
                cvg_layer_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText(s);
            } else if ( np->coverage_list[cvg_idx]->type != CConst::pagingAreaCoverage) {
                cvg_layer_page->m_cvg_wid->setParam( QString(tr("SIR (dB)") ), m_cvg_name, m_sig_thr, m_max_layer, m_scan_fra_area, m_sam_res );
                cvg_layer_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText( "SIR" );
            }
            
            top_level_item->insertItem( cvg_layer_page->m_cvg_item );
            cvg_layer_page->m_cvg_item->setText(0, m_cvg_name);
            cvg_widstack->addWidget( cvg_layer_page->m_cvg_wid );
            
            info_layer_wid_vector.append( cvg_layer_page );
        }
        
        // deal with paging Area
        if ( np->coverage_list[cvg_idx]->type == CConst::pagingAreaCoverage ) {
            m_item_count++;
            m_select_index = m_item_count-1;
            type = isPA;
            
            if (np->coverage_list[cvg_idx]->has_threshold == 1)
                cvg_pa_page = new CvgPAPage( np, true, this );
            else
                cvg_pa_page = new CvgPAPage( np, false, this );
            
            cvg_pa_page->item_index = m_item_count-1;
            cvg_pa_page->setNameSubItem( m_cvg_name );
            
            m_sig_thr       = 10*log(np->coverage_list[cvg_idx]->threshold)/log(10.0);
            m_sig_thr       = m_sig_thr - np->preferences->pwr_offset;
            m_scan_fra_area = np->coverage_list[cvg_idx]->scan_fractional_area;
            m_sam_res       = np->coverage_list[cvg_idx]->init_sample_res;
            
            // ---------------------------------------------------
            // get the cell list from np object; Added by Wei Ben
            // get the displayed cells list
            m_dPartDistance = np->coverage_list[cvg_idx]->dmax;
            QString ss;
            ListClass<int> *cvg_cells = np->coverage_list[cvg_idx]->cell_list;
            if (cvg_cells != NULL) {
                m_bChoosePart = true;
                m_vDisplayList.clear();
                for( int kk = 0; kk < cvg_cells->getSize(); kk++) {
                    ss.sprintf("Cell_%d", (*cvg_cells)[kk]);
                    m_vDisplayList.push_back(ss);
                }
                
                cvg_pa_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            else { // for choose All
                m_bChoosePart = false;
                m_dPartDistance = -1;
                m_vDisplayList.clear();
                cvg_pa_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            // finish adding
            // ---------------------------------------------------
            
            if (np->coverage_list[cvg_idx]->has_threshold == 1){
                s = tr("Signal Threshold ") + " (" + QString(np->preferences->pwr_str_short) + ")";
                cvg_pa_page->m_cvg_wid->info_cvg_sig_thr_textLabel->setText(s);
                cvg_pa_page->m_cvg_wid->setParam( QString(tr("PA") ), m_cvg_name, m_sig_thr, m_scan_fra_area, m_sam_res );
            }
            else
                cvg_pa_page->m_cvg_wid->setParam( QString(tr("PA") ), m_cvg_name, m_scan_fra_area, m_sam_res );
            
            top_level_item->insertItem( cvg_pa_page->m_cvg_item );
            cvg_pa_page->m_cvg_item->setText(0, m_cvg_name);
            cvg_widstack->addWidget( cvg_pa_page->m_cvg_wid );
            
            info_pa_wid_vector.append( cvg_pa_page );
            
        }
        
        if( np->coverage_list[cvg_idx]->type == CConst::levelCoverage ) {
            cvg_level_page = new CvgLevelPage( np, this );
            m_item_count++;
            m_select_index = m_item_count-1;
            type = isLevel;
            
            cvg_level_page->item_index = m_item_count-1;
            cvg_level_page->setNameSubItem( m_cvg_name );
            
            m_num_thr       = np->coverage_list[cvg_idx]->scan_list->getSize() - 1;
            
            m_min_thr = 10*log(np->coverage_list[cvg_idx]->level_list[0])/log(10.0);
            m_min_thr      -= np->preferences->pwr_offset;

            m_max_thr       = 10*log(np->coverage_list[cvg_idx]->level_list[m_num_thr-1])/log(10.0);
            m_max_thr      -= np->preferences->pwr_offset;
            m_scan_fra_area = np->coverage_list[cvg_idx]->scan_fractional_area;
            m_sam_res       = np->coverage_list[cvg_idx]->init_sample_res;
            
            int rows = cvg_level_page->m_cvg_wid->thr_tab_dia->table->numRows();
            for ( i=0; i<rows; i++ )
            {
                cvg_level_page->m_cvg_wid->thr_tab_dia->table->removeRow( 0 );
            }
            
            QString thr_str;
            double tmp;
            for( i=0; i<m_num_thr; i++ )
            {
                tmp = 10*log(np->coverage_list[cvg_idx]->level_list[i])/log(10.0) - np->preferences->pwr_offset;

                thr_str.sprintf( "%f", tmp);
                cvg_level_page->m_cvg_wid->thr_tab_dia->table->insertRows( i );
                cvg_level_page->m_cvg_wid->thr_tab_dia->table->setText( i, 0, thr_str );

                thr_str.sprintf( "threshold_%d", i );
                Q3Header *th = cvg_level_page->m_cvg_wid->thr_tab_dia->table->verticalHeader();
                th->setLabel( i, thr_str );
            }
            
            // ---------------------------------------------------
            // get the cell list from np object; Added by Wei Ben
            // get the displayed cells list
            m_dPartDistance = np->coverage_list[cvg_idx]->dmax;
            QString ss;
            ListClass<int> *cvg_cells = np->coverage_list[cvg_idx]->cell_list;
            if (cvg_cells != NULL) {
                m_bChoosePart = true;
                m_vDisplayList.clear();
                for( int kk = 0; kk < cvg_cells->getSize(); kk++) {
                    ss.sprintf("Cell_%d", (*cvg_cells)[kk]);
                    m_vDisplayList.push_back(ss);
                }
                
                cvg_level_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            else { // for choose All
                m_bChoosePart = false;
                m_dPartDistance = -1;
                m_vDisplayList.clear();
                cvg_level_page->m_cvg_wid->setPartParam(m_dPartDistance, m_vDisplayList, m_bChoosePart);
            }
            // finish adding
            // ---------------------------------------------------
            
            cvg_level_page->m_cvg_wid->setParam( QString(tr("level")), m_cvg_name, m_num_thr, m_min_thr, m_max_thr, m_scan_fra_area, m_sam_res );
            top_level_item->insertItem( cvg_level_page->m_cvg_item );
            cvg_level_page->m_cvg_item->setText(0, m_cvg_name);
            cvg_widstack->addWidget( cvg_level_page->m_cvg_wid );
            info_level_wid_vector.append( cvg_level_page );
        }
    } // end for

}
