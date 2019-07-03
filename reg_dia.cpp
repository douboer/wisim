/******************************************************************************************
**** PROGRAM: reg_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <stdlib.h>
#include <string>
#include <list>
#include <iostream>

#include <q3filedialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <q3whatsthis.h>
#include <qwidget.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "cconst.h"
#include "license.h"
#include "WiSim.h"
#include "WiSim_gui.h"
#include "pref.h"
#include "reg_dia.h"
#include "reg_form.h"
#include "randNumStr.h"

extern char *WiSim_home;

using namespace std;

/*
void create_regerror_dialog(int registered, PrefClass *preferences, unsigned char *reg_info, int ris) {
    new RegistrationErrorDialog(registered, preferences, reg_info, ris);
}
*/

/*
 * Constructor()
 */

RegistrationErrorDialog::RegistrationErrorDialog(int p_registered, PrefClass *preferences,
unsigned char *s_reg_info, int s_ris, QWidget* parent, const char* name)
: QDialog(parent, name, true), reg_info(s_reg_info), ris(s_ris)
{
    resize( 550, 250);
    usb_sn = (char *) NULL;
    path   = (char *) NULL;

    registered = p_registered;

    Q3VBoxLayout *reg_err_dia_vlayout  = new Q3VBoxLayout(this, 6, 11);
    Q3GridLayout *grid_layout          = new Q3GridLayout((QWidget *) 0, 1, 1, 10);
    Q3HBoxLayout *reg_err_dia_hlayout  = new Q3HBoxLayout(0, 6, 11);
    Q3HBoxLayout *language_dia_hlayout = new Q3HBoxLayout(0, 6, 11);

    reg_lbl = new QLabel(this);
    grid_layout->addWidget(reg_lbl, 0, 0);

    bgrp2 = new Q3HButtonGroup(this);
    language_dia_hlayout->addWidget( bgrp2, Qt::AlignHCenter );
    bgrp2->setExclusive( FALSE );

    English_lgn = new QRadioButton(bgrp2);
    Chinese_lgn = new QRadioButton(bgrp2);

    en_id = bgrp2->id(English_lgn);
    zh_id = bgrp2->id(Chinese_lgn);

    if (preferences->language == CConst::en) {
        English_lgn->setChecked( TRUE );
        Chinese_lgn->setChecked( FALSE );
    } else if (preferences->language == CConst::zh) {
        English_lgn->setChecked( FALSE );
        Chinese_lgn->setChecked( TRUE  );
    } else {
        CORE_DUMP;
    }

    register_btn = new QPushButton(this);
    reg_err_dia_hlayout->addWidget(register_btn, Qt::AlignCenter);
    register_btn->setMaximumWidth(80);

    install_btn = new QPushButton(this);
    reg_err_dia_hlayout->addWidget(install_btn, Qt::AlignCenter);
    install_btn->setMaximumWidth(120);

    cancel_btn = new QPushButton(this);
    reg_err_dia_hlayout->addWidget(cancel_btn, Qt::AlignCenter);
    cancel_btn->setMaximumWidth(80);

    reg_err_dia_vlayout->addLayout(language_dia_hlayout,     Qt::AlignHCenter);
    reg_err_dia_vlayout->addLayout(grid_layout,     Qt::AlignHCenter);
    reg_err_dia_vlayout->addLayout(reg_err_dia_hlayout, Qt::AlignHCenter);

    connect( bgrp2,        SIGNAL( clicked(int) ), this, SLOT( language_btn_clicked(int) ) );
    connect( register_btn, SIGNAL( clicked()    ), this, SLOT( register_btn_clicked()    ) );
    connect( install_btn,  SIGNAL( clicked()    ), this, SLOT( install_btn_clicked()     ) );
    connect( cancel_btn,   SIGNAL( clicked() ),    this, SLOT( cancel_btn_clicked()   ) );
    register_btn->setFocus();

    set_text();

    //exec();
}

/*
 * Slot for language_btn_clicked
 */
void RegistrationErrorDialog::language_btn_clicked(int btn_id)
{
    int lang;

    if (btn_id == en_id) {
        lang = CConst::en;
    } else if (btn_id == zh_id) {
        lang = CConst::zh;
    } else {
        CORE_DUMP;
    }

    set_language(lang);
    set_text();
}

void RegistrationErrorDialog::launchDia()
{
    set_text();
    this->exec();
}

void RegistrationErrorDialog::set_text()
{
    setCaption( tr("WiSim")+" "+WISIM_RELEASE );

    QString qstr;

    qstr = "<h3>Registration Error: </h3>";
    qstr += "<ul>";

    if (registered == LIC_FILE_NOT_FOUND) {
        qstr += "<li> " + tr(" Unable to read license file !");
    } else if (registered == LIC_EXPIRED) {
        qstr += "<li> " + tr(" License file Expired !");
    } else if (registered == LIC_INVALID) {
        qstr += "<li> " + tr(" License file Invalid !");
    } else if (registered == LIC_NO_NET_CONN) {
        qstr += "<li> " + tr(" No Network Connection Available !");
    } else if (registered == LIC_USB_INVALID) {
        qstr += "<li> " + tr(" Do not insert usb key or usb key is not invalid");
        if ( get_usb_sn() != NULL )
        {
            //qstr += "<li> " + tr(" Register Code : ") + "<br>";
            //qstr += get_usb_sn();
        }
    }

    qstr += "</ul>";

    reg_lbl->setText(qstr);

    bgrp2->setTitle( tr("Language") );

    English_lgn->setText(tr("English"));
    Chinese_lgn->setText(tr("Chinese"));

    register_btn->setText(tr("Register"));
    install_btn->setText(tr("Install From File"));
    cancel_btn->setText(tr("&Cancel"));
}

void RegistrationErrorDialog::set_usb_sn(char *&usb_sn)
{
    this->usb_sn = usb_sn;
}

const char* RegistrationErrorDialog::get_usb_sn()
{
    return usb_sn;
}

void RegistrationErrorDialog::set_path(char *&path)
{
    this->path = path;
}

const char* RegistrationErrorDialog::get_path()
{
    return path;
}

/*
 * Slot for register_btn_clicked
 * call reg_form wizard for user's input
 */
void RegistrationErrorDialog::register_btn_clicked()
{
    int show_reg_error_box;

    regForm *reg_form = new regForm(reg_info, ris, this);

    int exec_reg_dialog = reg_form->exec();
    if (exec_reg_dialog == QDialog::Accepted) {
        show_reg_error_box = 0;
    } else {
        show_reg_error_box = 1;
    }

    if ( get_usb_sn() != NULL )
    {
        //
        // temp code for chile usb key
        // - CG 9/10/2008
        //
        string fn  = "";
        string str = "";

        fn += get_path();
        fn += "\\usb_key.dat";
        str += get_usb_sn();

        std::cout << fn << std::endl;

        list<string> lst;
        lst = encryptStr(str);
        ExportToFile(lst, fn);
    }

    if (!show_reg_error_box) {
        delete this;
    }
}

/*
 * Slot for install_btn_clicked
 */
void RegistrationErrorDialog::install_btn_clicked()
{

    int show_reg_error_box = 1;

    QString fn = Q3FileDialog::getOpenFileName(
        QString::null,
        tr("License Files") + " (*.dat);;" +
        tr("All Files") + " (*)",
        this,
        tr("Install License From File"),
        tr("Select File to Read License Data From") );

    if (!fn.isEmpty()) {
        char *install_file = strdup((const char *) fn.local8Bit());
        show_reg_error_box = install_license_file(install_file, WiSim_home, reg_info, ris);
    }

    if (!show_reg_error_box) {
        delete this;
    }
}

/*
 * Slot for cancel_btn_clicked
 */

void RegistrationErrorDialog::cancel_btn_clicked()
{
    delete this;
}
