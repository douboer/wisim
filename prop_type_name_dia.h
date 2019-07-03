/******************************************************************************************/
/**** PROGRAM: prop_type_name_dia.h                                                    ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef PROP_TYPE_NAME_DIA_H
#define PROP_TYPE_NAME_DIA_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3ButtonGroup;
class QRadioButton;
class QPushButton;
class QLabel;
class QLineEdit;
class NetworkClass;

class PropTypeNameDia : public QDialog
{
    Q_OBJECT

public:
    PropTypeNameDia( NetworkClass*, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~PropTypeNameDia();

    Q3ButtonGroup* prop_type_buttonGroup;
    QRadioButton* expo_radioButton;
    QRadioButton* seg_radioButton;
    QRadioButton* clt_radioButton;
    QPushButton*  cancel_pushButton;
    QPushButton*  ok_pushButton;
    QLabel*       name_textLabel;
    QLineEdit*    name_lineEdit;

    QString prop_name;
    int     prop_type;
protected:
    NetworkClass* np;

    Q3GridLayout* PropTypeNameDiaLayout;
    Q3VBoxLayout* prop_type_buttonGroupLayout;
    Q3HBoxLayout* name_layout;

protected slots:
    void prop_type_btngroup_clicked(int);
    void prop_cancel_btn_clicked();

    virtual void languageChange();

private:
    void set_default_name();
};

#endif // PROP_TYPE_NAME_DIA_H
