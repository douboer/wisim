
/******************************************************************************************
**** PROGRAM: select_csid_format_dialog.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <q3buttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>

#include "cconst.h"
#include "WiSim.h"
#include "select_csid_format_dialog.h"

/******************************************************************************************/
/**** SelectCSIDFormatDialog::SelectCSIDFormatDialog                                   ****/
/******************************************************************************************/
SelectCSIDFormatDialog::SelectCSIDFormatDialog(int* p_sel_var_ptr, QWidget* parent)
    : QDialog( parent, 0, true)
{
    setName( "SelectCSIDFormatDialog Dialog" );
    setCaption( tr("Select CSID Format") );
    sel_var_ptr = p_sel_var_ptr;

    Q3VBoxLayout *vlayout     = new Q3VBoxLayout(this, 6, 11);
    Q3HBoxLayout *btn_hlayout = new Q3HBoxLayout(0, 6, 11);
    Q3HBoxLayout *ok_hlayout  = new Q3HBoxLayout(0, 6, 11);

    button_grp = new Q3VButtonGroup( this );
    btn_hlayout->addWidget( button_grp, Qt::AlignHCenter );
    button_grp->setTitle( tr( "CSID Format" ) );

    ut_radio_btn        = new QRadioButton( tr("Format 1 (Np = 19)"), button_grp, "ut_radio_btn" );
    zhongxing_radio_btn = new QRadioButton( tr("Format 2 (Np = 16)"), button_grp, "zhongxing_radio_btn" );

    ut_radio_btn->setChecked(TRUE);
    zhongxing_radio_btn->setChecked(FALSE);
    *sel_var_ptr = 0;

    ok_btn = new QPushButton(this);
    ok_btn->setText( tr( "&Ok" ) );
    ok_hlayout->addWidget( ok_btn, Qt::AlignCenter);
    ok_btn->setMaximumWidth(80);

    vlayout->addLayout(btn_hlayout, Qt::AlignHCenter);
    vlayout->addLayout( ok_hlayout, Qt::AlignHCenter);

    connect( ok_btn,   SIGNAL( clicked() ), this, SLOT( ok_btn_clicked()   ) );

    ok_btn->setFocus();

    setGeometry( (parent->width()-300)/2, (parent->height()-150)/2, 300, 150);

    exec();
}
/******************************************************************************************/
/**** SelectCSIDFormatDialog::~SelectCSIDFormatDialog                                  ****/
/******************************************************************************************/
SelectCSIDFormatDialog::~SelectCSIDFormatDialog()
{
}
/******************************************************************************************/
/**** SelectCSIDFormatDialog::ok_btn_clicked                                           ****/
/******************************************************************************************/
void SelectCSIDFormatDialog::ok_btn_clicked()
{
    if (ut_radio_btn->isChecked()) {
        *sel_var_ptr = CConst::CSID19NP;
    } else if (zhongxing_radio_btn->isChecked()) {
        *sel_var_ptr = CConst::CSID16NP;
    }

    delete this;
}
/******************************************************************************************/
