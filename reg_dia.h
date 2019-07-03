
/******************************************************************************************
**** PROGRAM: reg_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef REG_DIA_H
#define REG_DIA_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QPushButton;
class QTabWidget;
class QWidget;
class QLabel;
class QLineEdit;
class QRadioButton;
class Q3HButtonGroup;

class PrefClass;

class RegistrationErrorDialog : public QDialog
{
    Q_OBJECT

public:
    RegistrationErrorDialog(int registered, PrefClass *preferences, unsigned char *reg_info, int ris, QWidget* parent = 0, const char* name = 0 );

    void launchDia();

    void set_usb_sn(char *&usb_sn);
    const char *get_usb_sn();

    void set_path(char *&path);
    const char *get_path();

protected slots:
    void language_btn_clicked(int btn);
    void register_btn_clicked();
    void install_btn_clicked();
    void cancel_btn_clicked();

private:
    void set_text();

    unsigned char *reg_info;
    int ris, en_id, zh_id;
    int registered;

    char *usb_sn;
    char *path;

    QLabel *reg_lbl;
    Q3HButtonGroup *bgrp2;
    QRadioButton *English_lgn;
    QRadioButton *Chinese_lgn;
    QPushButton *register_btn;
    QPushButton *install_btn;
    QPushButton *cancel_btn;
};

#endif // REG_DIA_H
