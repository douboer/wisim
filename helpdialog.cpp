/*********************************************/
/* This class creates a dialog for user to  **/
/* to display the helpful infomation for     */
/* choosing the input file type              */
/* Auther:  Wei Ben                          */
/* Data  :  Sept. 20, 2004                   */
/*********************************************/


#include <qapplication.h>
#include <qfile.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3TextEdit>

#include "helpdialog.h"

helpDialog::helpDialog(QString s, QWidget* parent , const char* name, \
                       bool modal, Qt::WFlags f)
      : QDialog(parent, name, modal, f)
{
  sContent = s;
  popupParent = parent;

  setCaption( tr("Convert Road Test Data Help") );
  resize( 600, 400 );
  
  oLayout = new Q3VBoxLayout(this, 11, 6, "Form");
  oGrid = new Q3GridLayout(oLayout, 0, 0, 10);

  int iRow = 0;
  int iCol = 0;
  int iButtonPos = 2;

  oContent = new Q3TextEdit(this, "help_display");

  QFile file( sContent ); // Read the text from a file
  if ( file.open( QIODevice::ReadOnly ) ) {
    Q3TextStream stream( &file );
    oContent->setText( stream.read() );
  }

  oContent->setTextFormat(Qt::PlainText);
  oContent->setReadOnly(TRUE);
  
  oGrid->addMultiCellWidget(oContent, iRow, iRow + iButtonPos, 
                           iCol, iCol + iButtonPos);
  
  oYes = new QPushButton("OK", this);
  oYes->setDefault (TRUE);
  oGrid->addWidget(oYes, iRow+iButtonPos + 1, iButtonPos/2);

  connect( oYes, SIGNAL(clicked()), this, 
           SLOT(chooseYesButton()));


}

helpDialog::~helpDialog() {
  
  //qDebug("help dialog desconstructor() is called");
}

void helpDialog::setParent(QWidget* parent) {

  popupParent = parent;

}

void helpDialog::chooseYesButton() {

  //qDebug("helpOK is called");

  accept();
}
