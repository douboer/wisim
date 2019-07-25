/******************************************************************************************/
/**** PROGRAM: cs_ll_err_option_dialog.h                                               ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <qdialog.h>
#include <q3table.h>
#include <vector>
#include <qpushbutton.h>
#include <q3progressdialog.h>
#include <qlineedit.h>
#include <q3groupbox.h>
#include <q3wizard.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qradiobutton.h>

#include "wisim.h"

class CsLonlatErrorOptionDialog : public Q3Wizard
{
    Q_OBJECT

public:
    CsLonlatErrorOptionDialog( QWidget * parent = 0, const char * name = 0 );
    ~CsLonlatErrorOptionDialog();

	void check_cs_error();

    void setupPage1();
    void setupPage2();
    void setupPage3();
        
    NetworkClass* np;

private:
	int c1_thre_RSSI;
	double c1_thre_distance;
  double c2_thre_percentage;
  int c3_thre_dis_diff;

	Q3GroupBox* group1;
	Q3GroupBox* group2;
	Q3GroupBox* group3;

	QLineEdit* edit_distance1;
	QLineEdit* edit_RSSI1;
	QLineEdit* edit_percentage2;
	QLineEdit* edit_distance3;

  Q3VBox* page1;
  Q3VBox* page2;
  Q3VBox* page3;
  
  QRadioButton *rb1;
  QRadioButton *rb2;
  
public slots:
    virtual void next();
    virtual void back();
    virtual void accept();

};
