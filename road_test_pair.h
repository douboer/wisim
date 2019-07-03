/******************************************/
/* This class creates a dialog for user to*/
/* input the requirment data format C for */
/* road test data conversion.             */
/* Auther:                                */
/*         Wei Ben                        */
/* Data  :                                */
/*         Nov. 15, 2004                  */        
/******************************************/

#ifndef ROAD_TEST_PAIR_H
#define ROAD_TEST_PAIR_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>
#include <QLabel>
#include "filechooser.h"

class Q3HBoxLayout;
class QLabel;
class QPushButton;
class Q3VBoxLayout;
class QLineEdit;

class SpecialFormatDialog: public QDialog {

  Q_OBJECT

public:

  SpecialFormatDialog(QWidget* parent = 0, const char* name = "Road Test Points Convert", 
                bool modal = TRUE, Qt::WFlags f = 0);

  ~SpecialFormatDialog();

  FileChooser *oGPSChooser;
  FileChooser *oPHSChooser;

  // added on Nov. 15, 2004
  QString& getGpsName();
  QString& getPhsName();

protected:

  QPushButton *okBtn;
  QPushButton *cancelBtn;
  QLabel *oGpsLabel;
  QLabel *oPhsLabel;

  

  Q3VBoxLayout *FormLayout;
  Q3HBoxLayout *GpsLayout;
  Q3HBoxLayout *PhsLayout;  
  Q3HBoxLayout *BtnsLayout;

protected slots:

   void chooseGpsFile( const QString&);
   void choosePhsFile( const QString&);
   
   // added on Nov. 15, 2004
private:
   
   QString m_GpsFileName;
   QString m_PhsFileName;

   
};

#endif


