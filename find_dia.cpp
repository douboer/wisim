/******************************************************************************************/
/**** PROGRAM: find_dia.cpp                                                            ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <iostream>

#include "gconst.h"
#include "WiSim.h"
#include "phs.h"
#include "WiSim_gui.h"
#include "find_dia.h"
#include "list.h"

void FigureEditor::create_find_dialog() { new FindDialog(np, this); }

FindDialog::FindDialog(NetworkClass*np_param, FigureEditor *p_editor, QWidget* parent)
    : QDialog( parent, 0, true)
{

    int i;
    np = np_param;
    editor = p_editor;

    QString filt;

//    chooseWizardPage = new QWidget( this, "chooseWizardPage" );
//    chooseWizardPage->show();
  
    Q3VBoxLayout* toplayout = new Q3VBoxLayout( this, 20, 20 );
    
    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
//    buttonGroup1->setGeometry( QRect( 40, 20, 200, 120 ) );
    
    setCaption( tr( "Find Dialog" ) );
    buttonGroup1->setTitle( tr( "Choice Options" ) );
    
    unassigned_pm_ckbox = new QRadioButton( buttonGroup1, "unassigned_pm_ckbox" );
    unassigned_pm_ckbox->setGeometry( QRect( 20, 40, 250, 21 ) );
    unassigned_pm_ckbox->setChecked( TRUE );
    unassigned_pm_ckbox->setText( tr( "Unassigned Propagation Model" ) );

     
    CSID_find = new QRadioButton( buttonGroup1, "CSID_find" );
    CSID_find->setEnabled( true );
    CSID_find->setGeometry( QRect( 20, 80, 96, 21 ) );
    CSID_find->setText( tr("CSID")+":");    
    
    QWidget* privateLayoutWidget = new QWidget( buttonGroup1, "layout6" );
    privateLayoutWidget->setGeometry( QRect( 140, 80, 150, 71 ) );
    layout6 = new Q3VBoxLayout( privateLayoutWidget, 11, 10, "layout6");       
    
    CSID_input = new QLineEdit( privateLayoutWidget, "CSID_input" );
    layout6->addWidget( CSID_input );

    CSID_input->setInputMask( QString::null );
    CSID_input->setEnabled(true);
    
    gw_csc_cs_find = new QRadioButton( buttonGroup1, "gw_csc_cs_find" );
    gw_csc_cs_find->setEnabled( true );
    gw_csc_cs_find->setGeometry( QRect( 20, 120, 110, 21 ) );
    gw_csc_cs_find->setText( tr("GW-CSC-CS")+":");        

      
    gw_csc_cs_input = new QLineEdit( privateLayoutWidget, "gw_csc_cs_input" );
    layout6->addWidget( gw_csc_cs_input );

    gw_csc_cs_input->setInputMask( QString::null );
    toplayout->addWidget( buttonGroup1 );
            
   
   
    okCancelLayout = new Q3HBoxLayout( toplayout, 8, "okCancelLayout");
    ok_btn = new QPushButton( this, "ok_btn" );
    okCancelLayout->addWidget( ok_btn );
    
    cancel_btn = new QPushButton( this, "cancel_btn" );
    okCancelLayout->addWidget( cancel_btn );

    cancel_btn->setText( tr( "&Cancel" ) );
    cancel_btn->setAccel( QKeySequence( tr( "Alt+C" ) ) );

    ok_btn->setText( tr( "&Ok" ) );
    ok_btn->setAccel( QKeySequence( tr( "Alt+O" ) ) );

    init();        
    connect( buttonGroup1, SIGNAL( clicked(int) ), this, SLOT( CheckToJump( int ) ) );
    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_clicked() ) );
    connect( this,   SIGNAL( selection_changed(ListClass<int> *) ), editor, SIGNAL( selection_changed(ListClass<int> *) ) );
    connect( gw_csc_cs_input, SIGNAL( textChanged(const QString&) ), this, SLOT( changeDistValue1(const QString&) ) );     
    exec();

}





FindDialog::~FindDialog()
{
    // no need to delete child widgets, Qt does it all for us
}    

/****************************************************************************
**    DEFINE SLOT
*****************************************************************************/
void FindDialog::ok_btn_clicked()
{
    int i, cell_idx, sector_idx;
    char *chptr;
    CellClass *cell;
    SectorClass *sector;
    PHSSectorClass *sector1;

    chptr = np->line_buf;
    
    editor->select_cell_list->reset();

        if (unassigned_pm_ckbox->isChecked()) {
        for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                if (sector->prop_model == -1) {
                    editor->select_cell_list->ins_elem(cell_idx);
		    editor->setVisibility(GConst::cellRTTI);
                    emit selection_changed(editor->select_cell_list);  
                }
		
            }
        }
      if(editor->select_cell_list->getSize() == 0) {
		    QMessageBox::warning( this, tr("Report"), 
                                   QString( tr("No unassigned propagation model found") +"!\n"),
                                   tr("&OK"), QString::null, 0,1 );  
				   }
				   delete this;
    }
    
    if (CSID_find->isChecked()) {
        char *str = strdup(CSID_input->text().latin1());
        unsigned char *csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
        if (!hexstr_to_hex(csid_hex, str, PHSSectorClass::csid_byte_length)) {
            QMessageBox::warning( this, tr("Warning"), 
                                  QString( tr("Please ender valid CSID") +"!\n"),
                                  tr("&OK"), QString::null, 0,1 );         
        } else {
            for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                    sector1 = (PHSSectorClass*) cell->sector_list[sector_idx];
                    if (strncmp((char *) sector1->csid_hex, (char *) csid_hex, PHSSectorClass::csid_byte_length)==0) {
                        editor->select_cell_list->ins_elem(cell_idx);
		        editor->setVisibility(GConst::cellRTTI);
                        emit selection_changed(editor->select_cell_list);  		    
                    }
                }
            }
	    if(editor->select_cell_list->getSize() == 0) {
		    QMessageBox::warning( this, tr("Report"), 
                                   QString( tr("Search CSID not found")+"!\n"),
                                   tr("&OK"), QString::null, 0,1 );  
            } else {
	        delete this;
	    }
	}
        free(str);
        free(csid_hex);
    }
    if (gw_csc_cs_find->isChecked()) {
        if ( gw_csc_cs_input->text().isEmpty () ) {
             QMessageBox::warning( this, tr("Warning"), 
                                   QString( tr("Please input GW-CSC-CS")+"!\n"),
                                   tr("&OK"), QString::null, 0,1 );         
    }
        else{
        for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector1 = (PHSSectorClass*) cell->sector_list[sector_idx];
                if (sector1-> gw_csc_cs == Input_gw_csc_cs) {
                    editor->select_cell_list->ins_elem(cell_idx);
		    editor->setVisibility(GConst::cellRTTI);
                    emit selection_changed(editor->select_cell_list);  		    
                }
            }
        }
	if(editor->select_cell_list->getSize() == 0) {
		    QMessageBox::warning( this, tr("Report"), 
                                   QString( tr("Search GW-CSC-CS not found")+"!\n"),
                                   tr("&OK"), QString::null, 0,1 );  
	}
        else{
	delete this;
	}
    }
              
	}


}


void FindDialog::cancel_clicked()
{
    delete this;
}

void FindDialog::CheckToJump(int i)
{
  if ( i == 0 ) {
     CSID_input->setEnabled(false);
     gw_csc_cs_input->setEnabled(false);
    }
   if ( i == 1) {
      CSID_input->setEnabled(true);
     gw_csc_cs_input->setEnabled(false);
     } 
   if ( i == 2){
     CSID_input->setEnabled(false);
     gw_csc_cs_input->setEnabled(true);
     }
     
 }

void FindDialog::init()
{
//    m_bIs = 0; // default to unassgined propagation model
    unassigned_pm_ckbox->setChecked( TRUE );
    CSID_find->setChecked( FALSE );
    CSID_input->setEnabled(false);
    gw_csc_cs_find->setChecked( FALSE );
    gw_csc_cs_input->setEnabled(false);
  }

void FindDialog::changeDistValue1(const QString& text)
{
    QString t = text;
    Input_gw_csc_cs = text.toInt();

}

