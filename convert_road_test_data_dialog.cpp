/****************************************/
/* This class creates dialog for user to*/
/* input the requirment data for road   */
/* test data conversion.                */
/* Auther:                              */
/*         Wei Ben                      */
/* Data  :                              */
/*         Sept. 07, 2004               */        
/* Revise: Oct. 11, 2004                */
/*         Nov, 16, 2004                */
/****************************************/


#include "cconst.h"
#include "wisim.h"
#include "convert_road_test_data_dialog.h"
#include "helpdialog.h"

// added by Wei on Nov. 15, 2004
#include "road_test_pair.h"
#include <qstringlist.h>
#include <q3whatsthis.h>
#include <q3header.h>
#include <qtooltip.h>
#include <qvariant.h>


#include <q3buttongroup.h>
#include <q3frame.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <q3listbox.h>
#include <qlayout.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <process.h>
#endif

// added on Oct. 11, 2004
#include "server.h"

#include <iostream>

#define CGDEBUG 0

extern char *wisim_home;

ConvertRoadTestDataDialog::ConvertRoadTestDataDialog(QWidget* parent, const char* name, \
                           bool modal,  Qt::WFlags f) 
  : QDialog(parent, name, modal, f)

{
  data = new Element();
  help = new helpDialog(data->getHelpFile(), this, tr ("help"), TRUE );

  // added on Nov. 16, 2004
  m_bTypeC = false;
  oTypeSpecialDlg = new SpecialFormatDialog(this);

  sExtension = QString("crtd");

  setCaption( tr("Convert Road Test Data") );
  resize( 400, 380);

  roadtestFormLayout = new Q3VBoxLayout(this, 11, 6, "whole_Form");
  Q3GridLayout *grid = new Q3GridLayout(roadtestFormLayout, 0, 0, 10);
  
  int row = 0;
  int labelCol = 0;
  int linedCol = 1;

  oFileTypeComboBox = new QComboBox( FALSE, this );
  oFileTypeComboBox->insertItem("a" );
  oFileTypeComboBox->insertItem("b" );
  oFileTypeComboBox->insertItem("c" );
  oFileTypeComboBox->insertItem("d" );
  oFileTypeComboBox->insertItem("e" );
  oFileTypeComboBox->insertItem("f" );
  oFileTypeComboBox->insertItem("g" );
  oFileTypeComboBox->insertItem("h" );
  oFileTypeComboBox->insertItem("i" );
  oFileTypeComboBox->insertItem("j" );

  grid->addWidget(oFileTypeComboBox, row, linedCol);

  oDataFormatLabel = new QLabel(oFileTypeComboBox, tr( "Input File &Type") ,this, "TypeLabe");
  grid->addWidget(oDataFormatLabel, row, labelCol);

  oTypeHelp = new QPushButton( tr ("Help"), this);
  grid->addWidget(oTypeHelp, row, linedCol+1);
 
  // modified on Nov. 16, 2004
  oInputPushButton = new QPushButton(tr ("Input &Files ..." ), this);
  grid->addWidget(oInputPushButton, row+1, labelCol);
  oDisplayInputFiles = new Q3ListBox(this, "only display", 0);
  //oDisplayInputFiles->setSelectionMode(QListBox::Extended);
  grid->addMultiCellWidget(oDisplayInputFiles, row+1, row+3, linedCol, linedCol+1);//linedCol, linedCol);
  
  // added on Nov. 16, 2004
  removeBtn = new QPushButton(tr("&Remove"), this);
  grid->addWidget(removeBtn, row+2, labelCol);


  // added on Nov. 25, 2004
  oUnitComboBox = new QComboBox( FALSE, this, "comboBox1" );
  oUnitComboBox->clear();
  oUnitComboBox->insertItem( "dBm" );
  oUnitComboBox->insertItem( "dBuV_107" );
  oUnitComboBox->insertItem( "dBuV_113" );
  oUnitComboBox->setCurrentItem( 2 );  // added by Liu Tao, 05-03-10
  grid->addWidget(oUnitComboBox, row+3, labelCol);

  // the output file format
  OutputLayout = new Q3HBoxLayout( 0, 0, 6);
  oOutputTextLabel = new QLabel( tr ("&Out file..." ), this );
  OutputLayout->addWidget( oOutputTextLabel );

  fileChooser = new FileChooser( this, "fileChooser ", CConst::saveFileMode );
  fileChooser ->setFileFilter( tr("Road Test Data Files") + " (*.crtd);;" + tr("All Files") + " (*)" );
  fileChooser ->setDialogCaption( tr ("Choose File..."));
  OutputLayout->addWidget( fileChooser );  

  roadtestFormLayout->addLayout( OutputLayout );
  oOutputTextLabel->setBuddy(fileChooser);  
  

  // the threshold and resolution items
  thresholdLayout = new Q3HBoxLayout( 0, 0, 6 );
  oThresholdLabel = new QLabel( tr ("Threshold") + " (dBuV_113)", this );
  thresholdLayout->addWidget(oThresholdLabel );
  
  oThresholdSpinBox = new QSpinBox( this );
  oThresholdSpinBox->setMinValue( -200);
  oThresholdSpinBox->setMaxValue( 200 );
  oThresholdSpinBox->setValue(80);
  thresholdLayout->addWidget( oThresholdSpinBox );
  roadtestFormLayout->addLayout( thresholdLayout );
  oThresholdLabel->setBuddy(oThresholdSpinBox);
  
  resolutionLayout = new Q3HBoxLayout( 0, 0, 6);
  oResolutionLabel = new QLabel(tr ("Resolution "), this);
  resolutionLayout->addWidget(oResolutionLabel);

  oResolutionSpinBox = new QSpinBox(this);
  oResolutionSpinBox->setMinValue(1);
  oResolutionSpinBox->setMaxValue(32);
  resolutionLayout->addWidget(oResolutionSpinBox);
  oResolutionLabel->setBuddy(oResolutionSpinBox);

  roadtestFormLayout->addLayout( resolutionLayout );



  buttonsLayout = new Q3HBoxLayout( 0, 0, 6 );
  QSpacerItem *spacer = new QSpacerItem( 0, 0,
                            QSizePolicy::Expanding, QSizePolicy::Minimum );
  buttonsLayout->addItem( spacer );

  
  
  okPushButton = new QPushButton(tr("OK"), this );
  okPushButton->setDefault(false);
  okPushButton->setEnabled(false);
  buttonsLayout->addWidget( okPushButton );
  
  cancelPushButton = new QPushButton( tr("Cancel"), this );
  okPushButton->setDefault(true);
  buttonsLayout->addWidget( cancelPushButton );
  roadtestFormLayout->addLayout( buttonsLayout );

  connect( oTypeHelp, SIGNAL( clicked() ), this,
           SLOT( openHelp() ) );

  connect( oInputPushButton, SIGNAL( clicked() ), 
           this, SLOT( chooseInputFiles() ) );

  connect( fileChooser->lineEdit, SIGNAL( textChanged(const QString &) ), 
           this, SLOT( chooseOutputFile(const QString&) ) );

  connect( okPushButton, SIGNAL( clicked() ), 
           this, SLOT( runPerl() ) );
 
  connect( cancelPushButton, SIGNAL( clicked() ), 
           this, SLOT( cancelResponse() ) );

  // added on Nov. 16, 2004
  connect( oFileTypeComboBox, SIGNAL( activated(const QString&) ), \
           this, SLOT( changeOptions(const QString&) ) );

  connect( oUnitComboBox, SIGNAL( activated(const QString&) ), \
         this, SLOT( changeUnitOpt(const QString&) ) );

  connect( removeBtn, SIGNAL( clicked()), this, SLOT( removeItem()) );
  //------------------

  // user defined signal/slot
  connect( this, SIGNAL( listboxchanged(bool )) , \
           this, SLOT( updateOkEnabled(bool ))  );

  exec();
}


/**********************************/
/*  the deconstructor             */
/**********************************/
ConvertRoadTestDataDialog::~ConvertRoadTestDataDialog() {

  delete data;
  // added on Nov. 16, 2004
  delete oTypeSpecialDlg;

}

/**********************************/
/* the help button slot           */
/**********************************/
void ConvertRoadTestDataDialog::openHelp() {
  
  help->exec();

}


/**********************************/
/* the cancel button slot         */
/**********************************/
void ConvertRoadTestDataDialog::cancelResponse() {

  delete this;
}

/**********************************/
/* the input file slot            */
/**********************************/
void ConvertRoadTestDataDialog::chooseInputFiles() {
  
  static int iPairCount = 1;
  

  // added on Nov. 16, 2004
  if (m_bTypeC) {
    // popup the oTypeSpecialDlg to input files
    if (oTypeSpecialDlg->exec() == QDialog::Accepted ) {
      oDisplayInputFiles->insertItem(QString("%1 %2").arg(oTypeSpecialDlg->getGpsName(),oTypeSpecialDlg->getPhsName()));
      iPairCount++;

      list.append(oTypeSpecialDlg->getGpsName());
      list.append(oTypeSpecialDlg->getPhsName());
      //qDebug(list.join("*"));
    } 
  }
  else {
    QStringList tlst;
    QString filt = tr("Files") + " (*.txt);;" + tr("All Files") + " (*)";
    iPairCount = 1;
    QStringList files = Q3FileDialog::getOpenFileNames(filt,
                                                      QString::null,
                                                      this,
                                                      "open files dialog"
                                                      "Select one or more files to open" );
    tlst = files;

    if (! tlst.isEmpty())
    {
      oDisplayInputFiles->insertStringList(tlst, -1);

      list += tlst;
    }

#if CGDEBUG
    std::cout << " input file, list.size() " << list.size() << std::endl;
#endif

  }

  emit listboxchanged(true);

}

/**********************************/
/* choose output file slot        */
/**********************************/

void ConvertRoadTestDataDialog::chooseOutputFile() {

  int answer;
    
  QString filename;

  
  Q3FileDialog* fd = new Q3FileDialog(QString::null,
                                    tr("Road Test Data Files") + " (*.crtd);;" + tr("All Files") + " (*)",
                                    this, 
                                    "file create", "Road Test Convert -- File Output" );
  fd->setMode( Q3FileDialog::AnyFile );
  if ( fd->exec() == QDialog::Accepted ) {
    filename = fd->selectedFile();  
    if ( !filename.isEmpty() ) {
      
      if ( QFile::exists( filename ) ) {
        answer = QMessageBox::warning(
                                      this, "Road Test Convert -- Overwrite File",
                                      QString( "Overwrite\n\'%1\'?" ).
                                      arg( filename ),
                                      "&Yes", "&No", QString::null, 1, 1 );
        if ( answer == 0 ) {
          setOutputFile(filename);
        }
      }
      else {
        setOutputFile(filename);
      }
    }

  }
  
  data->setOutputFile(filename);

}

/********************************************/
/* new output file button response          */
/* Modified: Oct. 19, 2004                  */
/********************************************/

void  ConvertRoadTestDataDialog::chooseOutputFile(const QString& filename ) {

  QString fn = filename;
  qDebug(fn);
  if (filename != "") {
    QRegExp rx = QRegExp("\\." + sExtension + "$");
    if (rx.search(filename) == -1) {
      fn += "." + sExtension;
    }
    data->setOutputFile(fn);
    emit listboxchanged(true);
  }
  else
    emit listboxchanged(false);
}
 
/********************************************/
/* The dialog's ok button slot              */
/* get the input, run convert_rtd.pl        */
/* using socket technique to comm. with GUI */
/* Modified: Oct. 14, 2004: using fork()    */
/*           Oct. 26, 2004  not using fork()*/
/********************************************/
void ConvertRoadTestDataDialog::runPerl() { 
  
  getInput();

  if ( validInput()) {
        
#ifdef __linux__

    // -----------------------------------------------------------------
    // Modified on Nov. 29, 2004.
    // changed to use CVECTOR() to adapt to no-GUI, no QT support --------
    char *exefile;
    exefile = CVECTOR(strlen(wisim_home) + 1 + strlen("convert_rtd.pl"));
    sprintf(exefile, "%s%cconvert_rtd.pl", wisim_home, FPATH_SEPARATOR);

    //printf("%s\n", exefile);

    char *txtfile;
    txtfile = CVECTOR(strlen(wisim_home) + 3 + strlen("rtd_input.txt"));
    sprintf(txtfile, "%s%crtd_input.txt", wisim_home, FPATH_SEPARATOR);

    //printf("%s\n", txtfile);

    char *cmd;

    //cmd = CVECTOR(strlen(exefile) + strlen(txtfile) + 11);
    // sprintf(cmd, "%s %s %s %c", exefile, "-socket", txtfile, '&');
    // convert_rtd.pl lies at wisim_home dir.

    cmd = CVECTOR(strlen("convert_rtd.pl") + strlen(txtfile) + 11);
    sprintf(cmd, "%s %s %s %c", "convert_rtd.pl", "-socket", txtfile, '&');
    // convert_rtd.pl lies at current directory

    system(cmd);
    ServerInfo *server = new ServerInfo();

    free(exefile);
    free(txtfile);
    free (cmd);
    delete this;
    // ------------------------------------------------------------------

#else
    //_spawnl( _P_WAIT, "convert_rtd.exe -socket < file.txt &", NULL ); // under current directory.
    // modified on Oct. 14, 2004
    // under current directory
    //_spawnl( _P_DETACH, "convert_rtd.exe", "convert_rtd.exe","-socket", "file.txt", NULL );


    /*
    // ----------------------------------------------------------------
    // modified on Nov. 25, 2004 by Wei Ben -- use the path --
    QString sParaName = QString(wisim_home) + QString(QChar(FPATH_SEPARATOR)) + "file.txt";
    QString sExeName = QString(wisim_home) + QString(QChar(FPATH_SEPARATOR)) + "convert_rtd.exe";
    QString sCmd = QString("_P_DETACH, \"%1\", \"convert_rtd.exe\",\"-socket\", \"%2\"").arg(sExeName.latin1(), sParaName.latin1());
    _spawnl( sCmd.latin1(), NULL);

    ServerInfo *server = new ServerInfo();
    delete this;
    // -----------------------------------------------------------------
    */

    // -----------------------------------------------------------------
    // Modified on Nov. 29, 2004.
    // changed to use CVECTOR() to adapt to no-GUI, no QT support --------
    char *exefile;
    exefile = CVECTOR(strlen(wisim_home) + 1 + strlen("convert_rtd.exe"));
    sprintf(exefile, "%s%cconvert_rtd.exe", wisim_home, FPATH_SEPARATOR);

    char *txtfile;
    txtfile = CVECTOR(strlen(wisim_home) + 3 + strlen("rtd_input.txt"));
    sprintf(txtfile, "\"%s%crtd_input.txt\"", wisim_home, FPATH_SEPARATOR);

    _spawnl( _P_DETACH, exefile, "convert_rtd.exe", "-socket", txtfile, NULL );  
    ServerInfo *server = new ServerInfo();

    free(exefile);
    free(txtfile);
    delete this;
    // ------------------------------------------------------------------
#endif
  }
  else {
    
    QMessageBox::warning( this, "Alert", 
                                   tr("Input Data Incomplete!"),
                                   tr ("OK"), QString::null, 0,1 );           
  }

}


/*******************************************/
/* check the user's input data validation  */
/*******************************************/
void ConvertRoadTestDataDialog::validNum(const QString &text) {
  QString fileNum;
  bool ok;

  fileNum = text; 
  int dec = fileNum.toInt( &ok, 10 );    
  
  if ( ( ok) && ( dec == 0)) {
    QMessageBox::warning( this, "Valadition of the user's input", 
                                   QString( "Input File Number can't be zero!\n" ), 
                                   "&OK", QString::null, 0,1 );
  }  
  else if ( ! ok) {
    QMessageBox::warning( this, "Valadition of the user's input", 
                                   QString( "Input File Number will be digit!\n" ), 
                                   "&OK", QString::null, 0,1 );    
          
  }
}

/*******************************************/
/*  display the user's choosed output file */
/*******************************************/
void ConvertRoadTestDataDialog::setOutputFile(QString filename) {

  oOutputTextLabel->setText( filename);
}


/******************************************/
/*  fill the data object: element         */
/*  and save the input in a specific file */
/******************************************/
void ConvertRoadTestDataDialog::getInput() {

  int threshold, resolution;
  QString tmp, fileNum, sUnit;

  tmp = oFileTypeComboBox->currentText();
  sUnit = oUnitComboBox->currentText();
  threshold  = oThresholdSpinBox->value();
  resolution = oResolutionSpinBox->value();
  data->setResolution((double) resolution);
  data->setFormat(tmp);
  data->setThreshold(threshold);
  
  // added on Nov. 16, 2004
  if (m_bTypeC)
    data->setNumInput(list.count() / 2 );
  else 
    data->setNumInput(list.count());
  data->setInputFiles(list);

#if CGDEBUG
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
      std::cout << "size " << list.size() << "  " << *it << "\n";
  }
  std::cout << std::endl;
#endif


  // added on Nov. 25, 2004
  data->setUnitName(sUnit);
  
  data->writeToFile();
                   
}

bool ConvertRoadTestDataDialog::validInput() {
  
  if (data->validation())
    return true;
  else 
    return false;

}

/*********************************/
/* slot for choosing type        */
/* specially handle the type c   */
/* Date:  Nov. 16, 2004          */
/*********************************/
void ConvertRoadTestDataDialog::changeOptions(const QString & s)
{
    if (s.compare("c") == 0) {
        //qDebug("choose the type c");
        m_bTypeC = true;
    } else {
        m_bTypeC = false;
    }
}

void ConvertRoadTestDataDialog::removeItem() {

  QString sItemText;
  QStringList tmp;
  if( oDisplayInputFiles->currentItem() > -1 ) {
    sItemText = oDisplayInputFiles->currentText();
    oDisplayInputFiles->removeItem( oDisplayInputFiles->currentItem() );

    if (m_bTypeC) {
      tmp = QStringList::split(" ", sItemText, false);

      for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it ) {
        list.remove(*it);
      }
    }
    else
      list.remove(sItemText);

    //qDebug(sItemText);
    //qDebug(list.join("*"));

  }
  if (oDisplayInputFiles->count() == 0)
    emit listboxchanged(false);
  else
    emit listboxchanged(true);

}

/*****************************************/
/* slot for unit option change           */
/* change the "oThresholdLabel" text     */
/*****************************************/

void ConvertRoadTestDataDialog::changeUnitOpt( const QString & stmp)
{

  QString s_tmp;
  s_tmp = QString("Threshold") + " (" + stmp + ")";

//added by Liu Tao, 050310
  QString text_old = oThresholdLabel->text();  
  int base_old, base_new;
  if( text_old == "Threshold (dBm)" )             base_old = 0;
  else if( text_old == "Threshold (dBuV_107)" )   base_old = 107;
  else                                            base_old = 113;
  oThresholdLabel->setText(s_tmp);
  if( s_tmp == "Threshold (dBm)" )                base_new = 0;
  else if( s_tmp == "Threshold (dBuV_107)" )      base_new = 107;
  else                                            base_new = 113;
  int change = base_new - base_old;

  int newThreshold = oThresholdSpinBox->value() + change ;
  oThresholdSpinBox->setValue( newThreshold );
}


/****************************************/
/* slot for enabling OK button          */
/*                                      */
/****************************************/

void ConvertRoadTestDataDialog::updateOkEnabled(bool bYes) 
{

  if ( bYes ){
       getInput();
       if ( validInput() ) 
         okPushButton->setEnabled(true);
  }
  else 
    okPushButton->setEnabled(false);
}

#undef CGDEBUG
