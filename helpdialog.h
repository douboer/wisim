/******************************************************************************************/
/**** PROGRAM: helpdialog.cpp                                                          ****/
/**** Chengan                                                                          ****/
/******************************************************************************************/


#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3VBoxLayout>

class QPushButton;
class Q3TextEdit;
class Q3VBoxLayout;
class Q3GridLayout;

class helpDialog : public QDialog {
  
  Q_OBJECT
    
public:
  
    
  helpDialog(QString s, QWidget* parent = 0, \
             const char* name = "help dialog", bool modal = TRUE, Qt::WFlags f = 0);

  ~helpDialog();

  void getContent() const;
  void setContent();
  void setParent(QWidget* parent);

  QPushButton *oYes;

protected:

  Q3VBoxLayout *oLayout;
  Q3GridLayout *oGrid;

protected slots:
  
  void chooseYesButton();
  

private:

  QString sContent;
  Q3TextEdit *oContent;
  //QPushButton *oYes;
  QWidget* popupParent;

};

#endif
