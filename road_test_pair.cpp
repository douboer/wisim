#include "road_test_pair.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>

#include "cconst.h"
#include "filechooser.h"

SpecialFormatDialog::SpecialFormatDialog (QWidget* parent, const char* name, bool modal, Qt::WFlags f ) 
  : QDialog(parent, name, modal, f) {

  FormLayout = new Q3VBoxLayout(this, 11, 6, "form");

 
  // first line
  GpsLayout = new Q3HBoxLayout( 0, 0 ,6);
  
  oGpsLabel = new QLabel("Choose GPS File", this );
  GpsLayout->addWidget( oGpsLabel );

  oGPSChooser = new FileChooser( this, "GPS_fileChooser ", CConst::saveFileMode );
  oGPSChooser->setFileFilter( tr("All Files") + " (*)" );
  oGPSChooser->setDialogCaption("Choose GPS File...");
  GpsLayout->addWidget(oGPSChooser);

  FormLayout->addLayout( GpsLayout );
  oGpsLabel->setBuddy(oGPSChooser);  

  
  // second line
  PhsLayout = new Q3HBoxLayout( 0, 0 ,6);

  oPhsLabel = new QLabel("Choose PHS File", this);
  PhsLayout->addWidget(oPhsLabel);

  oPHSChooser = new FileChooser( this, "PHS35_fileChooser ", CConst::saveFileMode );
  oPHSChooser->setFileFilter( tr("All Files") + " (*)" );
  oPHSChooser->setDialogCaption("Choose PHS35 File...");
  PhsLayout->addWidget(oPHSChooser);

  FormLayout->addLayout( PhsLayout );
  oPhsLabel->setBuddy(oPHSChooser);  
  
  
  // third line
  BtnsLayout = new Q3HBoxLayout( 0, 0, 6 );
  okBtn = new QPushButton( "OK", this );
  okBtn->setDefault(TRUE);
  BtnsLayout->addWidget( okBtn );
  cancelBtn = new QPushButton( "Cancel", this );
  BtnsLayout->addWidget( cancelBtn );

  FormLayout->addLayout( BtnsLayout );
    
  connect( oGPSChooser, SIGNAL( fileNameChanged(const QString &) ), 
           this, SLOT( chooseGpsFile(const QString&) ) );

  connect( oPHSChooser, SIGNAL( fileNameChanged(const QString &) ), 
           this, SLOT( choosePhsFile(const QString&) ) );
  
  connect( okBtn, SIGNAL( clicked() ), 
           this, SLOT( accept() ));
 
  connect( cancelBtn, SIGNAL( clicked() ), 
           this, SLOT( reject() ) );

  // exec();

}


// the deconstructor
SpecialFormatDialog::~SpecialFormatDialog() {


}

/*
void SpecialFormatDialog::cancel() {

}

void SpecialFormatDialog::accept() {
}
*/
void SpecialFormatDialog::chooseGpsFile(const QString& s) {
  
  m_GpsFileName = s;

}


void SpecialFormatDialog::choosePhsFile(const QString& ss) {

  m_PhsFileName = ss;
}

QString& SpecialFormatDialog::getGpsName() {

  return m_GpsFileName;

}

QString& SpecialFormatDialog::getPhsName() {

  return m_PhsFileName;

}
