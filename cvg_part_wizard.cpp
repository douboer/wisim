/**********************************************************/
/* cvg_part_wizard.cpp file implements choosing part/all  */
/* cells to do simulation                                 */
/**********************************************************/
#include "cvg_part_wizard.h"

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvariant.h>
#include <q3whatsthis.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "gconst.h"
#include "cconst.h"
#include "WiSim.h"
#include "WiSim_gui.h"
#include "list.h"

#include <q3valuelist.h>

#define Debug 0


Cvg_Part_Wizard::Cvg_Part_Wizard( FigureEditor* editor_para, NetworkClass* np_param, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : Q3Wizard( parent, name, modal, fl )
{

  editor = editor_para;
  np = np_param;
  setModal(true);

  if ( !name )
    setName( "Cvg_Part_Wizard" );
  
  chooseWizardPage = new QWidget( this, "chooseWizardPage" );

  Q3VBoxLayout *vlayout = new Q3VBoxLayout( chooseWizardPage, 11, 6); 
  
  buttonGroup1 = new Q3ButtonGroup( chooseWizardPage, "buttonGroup1" );
  buttonGroup1->setGeometry( QRect( 40, 20, 370, 210 ) );
  vlayout->addWidget( buttonGroup1 );
  
  chooseAll = new QRadioButton( buttonGroup1, "chooseAll" );
  chooseAll->setGeometry( QRect( 20, 40, 71, 21 ) );
  chooseAll->setChecked( TRUE );
  
  choosePartBtn = new QRadioButton( buttonGroup1, "choosePartBtn" );
  choosePartBtn->setEnabled( TRUE );
  choosePartBtn->setGeometry( QRect( 20, 80, 96, 21 ) );

  QWidget* privateLayoutWidget = new QWidget( buttonGroup1, "layout6" );
  privateLayoutWidget->setGeometry( QRect( 60, 140, 260, 40 ) );
  layout6 = new Q3HBoxLayout( privateLayoutWidget, 11, 6, "layout6"); 
  
  textLabel1 = new QLabel( privateLayoutWidget, "textLabel1" );
  layout6->addWidget( textLabel1 );
  spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout6->addItem( spacer1 );
  
  maxDist = new QLineEdit( privateLayoutWidget, "maxDist" );
  layout6->addWidget( maxDist );

  useGPMCheckBox = new QCheckBox(tr("Use Global Propagation Models"), chooseWizardPage);
  useGPMCheckBox->setChecked( TRUE );
  vlayout->addWidget( useGPMCheckBox );

  addPage( chooseWizardPage, QString("") );
  
  partWizardPage = new QWidget( this, "partWizardPage" );
  
  textLabel1_2 = new QLabel( partWizardPage, "textLabel1_2" );
  textLabel1_2->setGeometry( QRect( 63, 31, 60, 25 ) );
  
  DeleteToolBtn = new QToolButton( partWizardPage, "DeleteToolBtn" );
  DeleteToolBtn->setGeometry( QRect( 196, 237, 48, 42 ) );
  DeleteToolBtn->setEnabled ( false );
  
  Category_comboBox = new QComboBox( FALSE, partWizardPage, "Category_comboBox" );
  Category_comboBox->setGeometry( QRect( 210, 30, 141, 23 ) );
  
  ChooseToolBtn = new QToolButton( partWizardPage, "ChooseToolBtn" );
  ChooseToolBtn->setGeometry( QRect( 196, 132, 48, 42 ) );
  
  Source_listBox = new Q3ListBox( partWizardPage, "Source_listBox" );
  Source_listBox->setGeometry( QRect( 30, 90, 140, 240 ) );
  Source_listBox->setSelectionMode( Q3ListBox::Extended);
  
  Dest_listBox = new Q3ListBox( partWizardPage, "Dest_listBox" );
  Dest_listBox->setGeometry( QRect( 280, 90, 140, 240 ) );
  Dest_listBox->setSelectionMode( Q3ListBox::Extended );
  addPage( partWizardPage, QString("") );
  
  languageChange();
  resize( QSize(442, 440).expandedTo(minimumSizeHint()) ); // (442, 363)
  
    // signals and slots connections
  connect( buttonGroup1, SIGNAL( clicked(int) ), this, SLOT( checkToJump(int) ) );
  connect( Category_comboBox, SIGNAL( activated(const QString&) ), this, SLOT( changeOptions(const QString&) ) );
  connect( ChooseToolBtn, SIGNAL( clicked() ), this, SLOT( selectCells() ) );
  connect( DeleteToolBtn, SIGNAL( clicked() ), this, SLOT( deleteCells() ) );
  connect( Dest_listBox, SIGNAL( selectionChanged()), this, SLOT( setBtnEnable() ) );
  connect( maxDist, SIGNAL( textChanged(const QString&) ), this, SLOT( changeDistValue(const QString&) ) );
  
  // tab order
  setTabOrder( Category_comboBox, Source_listBox );
  setTabOrder( Source_listBox, Dest_listBox );
  setTabOrder( Dest_listBox, chooseAll );
  setTabOrder( chooseAll, maxDist );
  init();

}

/*
 *  Destroys the object and frees any allocated resources
 */
Cvg_Part_Wizard::~Cvg_Part_Wizard()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Cvg_Part_Wizard::languageChange()
{
  setCaption( tr( "Part_or_All" ) );
  buttonGroup1->setTitle( tr( "Scope Choice" ) );
  chooseAll->setText( tr( "All Cells" ) );
  choosePartBtn->setText( tr( "Cell Group" ) );
  textLabel1->setText( tr( "Max. Distance :" ) );
  maxDist->setInputMask( QString::null );
  setTitle( chooseWizardPage, tr( "Choose single or multiple cells" ) );
  textLabel1_2->setText( tr("Show Cells By") + ": " );
  DeleteToolBtn->setText( "..." );
  Category_comboBox->clear();
  Category_comboBox->insertItem( tr( "Cell Index" ) );
  Category_comboBox->insertItem( tr( "GW_CSC_CS" ) );
  Category_comboBox->insertItem( tr( "CSID" ) );
  ChooseToolBtn->setText( "..." );
  setTitle( partWizardPage, tr( "Choose cells" ) );
  //setTitle( WizardPage, tr( "Page_3" ) );

  DeleteToolBtn->setText("<==");
  ChooseToolBtn->setText("==>");

    backButton()->setText(tr("&Back") + " <");
    nextButton()->setText(tr("&Next") + " >");
    finishButton()->setText(tr("&Finish"));
    cancelButton()->setText(tr("&Cancel"));
    helpButton()->setText(tr("&Help"));

}




void Cvg_Part_Wizard::checkToJump(int i)
{

  if (Debug == 1)
    qDebug( "the radiobutton's output = %d", i);

    if ( i == 0 ) {
      m_bIsPart = false;
      setAppropriate(partWizardPage, false);
      maxDist->setEnabled(false);
    }
    if ( i == 1) {
      m_bIsPart = true;
      setAppropriate(partWizardPage, true);
      maxDist->setEnabled(true);
    }	

}




void Cvg_Part_Wizard::init()
{
  setHelpEnabled( chooseWizardPage, FALSE );
  setHelpEnabled( partWizardPage, FALSE );
  m_CellList = new QStringList();
  if (!m_CellList->isEmpty() )
    m_CellList->clear();

  maxDist->setValidator( new QDoubleValidator( maxDist ) );
  // when m_bIsPart = true,  m_dMaxlength is valid

  // added on Dec. 3, 2003 for right-click
  if(editor->select_cell_list->getSize() == 0) {
    m_bIsPart = false;  // default to choose all
    chooseAll->setChecked( TRUE );
    choosePartBtn->setChecked( FALSE );
    maxDist->setEnabled(false);
  }
  else {
    choosePartBtn->setChecked( TRUE );
    maxDist->setEnabled(true);
    chooseAll->setChecked( FALSE );
    m_bIsPart = true;
    setAppropriate(partWizardPage, true);
    setFinishEnabled(partWizardPage, true);
  }

  // initial the ListBox -- source Listbox
  // modified on Dec. 3, 2004

  //if ( editor->select_cell_list->getSize() == 0) {
  int cell_idx, name_pref = -1;
  CellClass *cell1;
  QString s;
  Source_listBox->clear();
  name_pref = CConst::CellIdxRef;
  for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
    cell1 = np->cell_list[cell_idx];
    s = QString("%1").arg(cell1->view_name(cell_idx, name_pref));
    // added on Nov. 23, 2004
    // modified on Dec. 3, 2004 for right-click function
    if (editor->select_cell_list->contains( cell_idx )) {
      m_CellList->append(s);
      m_Map[s] = 1;
    }
    else {
      m_osList.append(s);
      m_Map[s] = -1;
    }
    m_oMapIndex[s] = cell_idx;
  }

  Source_listBox->insertStringList(m_osList);
  Dest_listBox->insertStringList(*m_CellList);
}


void Cvg_Part_Wizard::changeOptions( const QString & )
{
  enum userInput {
    CSID,
    CS_Number,
    CS_Index
  };
  
  QString sDispName;
  QStringList source, dest;

  source = m_osList;
  dest = *m_CellList;

  int cell_idx, name_pref = -1;
  CellClass *cell;
  
  Source_listBox->clear();
  m_osList.clear();
  m_CellList->clear();
  m_CellVector.clear();
  Dest_listBox->clear();

  switch ( Category_comboBox->currentItem() ) {
  case CSID:	
    name_pref = CConst::CellIdxRef;
    break;
  case CS_Number:
    name_pref = CConst::CellCSNumberRef;
    break;
  case CS_Index:
    name_pref = CConst::CellHexCSIDRef;
    break;
  }    

  /*
  // init. the source listbox and the map
  for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
    cell = np->cell_list[cell_idx];
    sDispName = QString("%1\n").arg(cell->view_name(cell_idx, name_pref));
    m_osList.append(sDispName);
    m_Map[sDispName] = -1;
    m_oMapIndex[sDispName] = cell_idx;
  }
  */

  int idx;
  QStringList::Iterator it;
  // change the strings in destination listbox
  for ( it = dest.begin(); it != dest.end(); ++it ) {
    idx = m_oMapIndex[*it];
    cell = np->cell_list[idx];
    sDispName = QString("%1").arg(cell->view_name(idx, name_pref));
    m_Map.remove(*it);
    m_Map.insert( sDispName, 1);
    m_CellList->append(sDispName);
  }

  // change the strings in source listbox
  for ( it = source.begin(); it != source.end(); ++it ) {
    idx = m_oMapIndex[*it];
    cell = np->cell_list[idx];
    sDispName = QString("%1").arg(cell->view_name(idx, name_pref));
    m_Map.remove(*it);
    m_Map.insert( sDispName, -1);
    m_osList.append(sDispName);
  }

  // change the m_oMapIndex contents
  m_oMapIndex.clear();
  for (cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
    cell = np->cell_list[cell_idx];
    sDispName = QString("%1").arg(cell->view_name(cell_idx, name_pref));
     m_oMapIndex[sDispName] = cell_idx;
  }

  printf("change --------------\n");

  CellIndexMap::Iterator itt;
  for ( itt = m_oMapIndex.begin(); itt != m_oMapIndex.end(); ++itt ) {
    printf("%s  %d\n", itt.key().latin1(), itt.data());
  }

  for ( itt = m_Map.begin(); itt != m_Map.end(); ++itt ) {
    printf("%s  %d\n", itt.key().latin1(), itt.data());
  }

  Source_listBox->insertStringList(m_osList);
  Dest_listBox->insertStringList(*m_CellList);
}


void Cvg_Part_Wizard::selectCells()
{  

  Q3ListBoxItem *item;

  // Go through all items of the first ListBox
  for ( unsigned int i = 0; i < Source_listBox->count(); i++ ) {
    if ( Source_listBox->isSelected(i) ) {
      item = Source_listBox->item( i );
      Dest_listBox->insertItem(item->text(), -1);
      //m_CellVector.push_back(i);      // for cmd
      m_CellList->append(item->text()); // for display
      
      // added on Nov. 22
      m_Map[item->text()] = 1;
    }
  }

  Source_listBox->clearSelection();
  setFinishEnabled(partWizardPage, true);

  if (Debug == 1) {
    //for( unsigned int k = 0; k < m_CellVector.size(); k++)
    //  printf("the index = %d\n", m_CellVector[k]);
    qDebug(m_CellList->join("#"));
  }


  // added on Nov. 22, 2004
  for ( QStringList::Iterator it = m_CellList->begin(); it != m_CellList->end(); ++it ) {
      //printf("the target = %s\n", (*it).latin1());
      item = Source_listBox->findItem((*it).latin1(), Q3ListBox::ExactMatch);
      if( item > 0) { 
        m_osList.remove(item->text());
        //printf("find the one to match = %s\n", (item->text()).latin1());
        //Source_listBox->takeItem(item);
        delete(item); 
      }
      //else
      // qDebug("can't find the string");
  }
  Source_listBox->clear();
  Source_listBox->insertStringList(m_osList);

}



void Cvg_Part_Wizard::deleteCells()
{
  // Go through all items of the destination ListBox

    QStringList finalList;
    //printf("the size = %d\n", Dest_listBox->count());
    for ( unsigned int i = 0; i <  Dest_listBox->count(); i++ ) {
        Q3ListBoxItem *item = Dest_listBox->item( i );
        // if the item is selected...
        if ( item->isSelected() ) {
          m_Map[item->text()] = -1;
          //printf("delete %s", item->text().latin1());
          m_CellList->remove(item->text());
          m_osList.append(item->text());
        }
        else {
	    finalList.append(item->text());
        }
    }

    Dest_listBox->clear();
    if (Debug == 1) {
      qDebug(finalList.join(": "));
      qDebug(m_CellList->join("*"));
    }
    Dest_listBox->insertStringList(finalList);


    Source_listBox->clear();
    // sort the m_osList and displayed in Source_listBox
    sortStringList(m_osList);
    Source_listBox->insertStringList(m_osList);

    if (Dest_listBox->count() == 0)
      setFinishEnabled(partWizardPage, false);

}



void Cvg_Part_Wizard::next() {
  if (!m_bIsPart) { // all

    Q3Wizard::accept();
    emit chooseAllSig(); 
  }
  else {  // part
    if ( maxDist->text().isEmpty () ) {
      QMessageBox::warning( this, "Validation of the user's input", 
                                   QString( "Please input max. distance value!\n"),
                                   "&OK", QString::null, 0,1 );   
    }
    else {
      Q3Wizard::next ();
    }
  }
}


void Cvg_Part_Wizard::accept() {

  /*
  if ( m_bIsPart) {

    //m_CellVector = m_Map.values();
   
    if (m_CellVector.size() == 0) 
      QMessageBox::warning( this, "Validation of the user's input", 
                                   QString( "Please choose the research cell!\n"),
                                "&OK", QString::null, 0,1 ); 
  }
  else 

*/
    Q3Wizard::accept();
}


void Cvg_Part_Wizard::changeDistValue(const QString& text)
{
    QString t = text;
    m_dMaxDistance = text.toDouble();
    if (Debug == 1)
      qDebug("the max length = %f\n", m_dMaxDistance);
}


void Cvg_Part_Wizard::setBtnEnable() {
     DeleteToolBtn->setEnabled ( true ); 
}


/***********************************************************/
/* Logical functions.                                      */
/***********************************************************/
bool Cvg_Part_Wizard::getChoice() {

  return m_bIsPart;

}

QStringList& Cvg_Part_Wizard::getPartCellList() {

  return (*m_CellList);

}

Q3ValueVector <int>& Cvg_Part_Wizard::getPartCellVector() {

  int kk;
  CellIndexMap::Iterator itt;
  for ( itt = m_Map.begin(); itt != m_Map.end(); ++itt ) {
    //printf( "%s, %d\n", itt.key().latin1(),itt.data());
    if( itt.data() == 1) {
      kk = (m_oMapIndex.find(itt.key())).data();
      m_CellVector.push_back(kk);
    }
      
  }
  
  /*
  //  qDebug(oCellIndexList);
  QValueVector<int> ::iterator it;
  QString ss;
  for( it = m_CellVector.begin(); it != m_CellVector.end(); ++it ) {
    ss = QString("the value of index = %1 \n").arg((*it));
    qDebug(ss);
  }

  */



  return m_CellVector;
}

double Cvg_Part_Wizard::getMaxDist() {
  return m_dMaxDistance;
}


//---------------------------------------------------
// added on Nov. 23, 2004
// Descprition: 
//     sort a QStringList in ascending sequence, 
//     but not dictionary.
//---------------------------------------------------

void Cvg_Part_Wizard::sortStringList(QStringList &sl)
{
  QString s0, s2;
  QStringList sList;
  bool bFound = false;
  
  s0 = sl.join("*");

  sList = QStringList::split("*", s0);
  //qDebug(sList.join("#"));
  
  sl.clear();
  
  for ( QStringList::Iterator it = sList.begin(); it != sList.end(); ++it ) {
    s0 = (*it);
    if ( sl.isEmpty() ) 
      sl.append(s0);
    else {
      for ( QStringList::Iterator itt = sl.begin(); itt != sl.end(); ++itt ) {
        
        s2 = (*itt);
        if (!s2.isEmpty() || !s2.isNull() ) {
          //printf("the compared s0 = %s, s2 =  %s", s0.latin1(), s2.latin1());
          if ( qstringcmp(s0, s2) <  0) {
            bFound = true;
            sl.insert(itt, s0);
            break;
          }
        } // end if
      } // end for
      
      if( ! bFound) 
        sl.append(s0);
    }
  } // end for
}
