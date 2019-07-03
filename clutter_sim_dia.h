
/*******************************************************************************************
**** PROGRAM: clutter_sim_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
/******************************************************************************************/
/**** PROGRAM: clutter_sim_dia.h                                                       ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef CLUTTER_SIM_DIA_H
#define CLUTTER_SIM_DIA_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class QLabel;
class QLineEdit;
class QComboBox;
class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;

class NetworkClass;

class ClutterSim : public QDialog
{
    Q_OBJECT

public:
    ClutterSim( NetworkClass* , QWidget* parent , const char* name ); 
    ~ClutterSim();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QLabel*      sim_type_textLabel;
    QComboBox*   sim_type_comboBox;
    QLabel*      from_scan_textLabel;
    QLineEdit*   from_scan_lineEdit;
    QLabel*      num_scan_textLabel;
    QLineEdit*   num_scan_lineEdit;
    QLabel*      useheight_textLabel;
    QComboBox*   useheight_comboBox;

protected:
    Q3GridLayout* ClutterSimLayout;
    Q3HBoxLayout* layout2;
    QSpacerItem* spacer2;

private:
    NetworkClass* np;

    bool useheight;
    //SimType type; 

protected slots:
    virtual void languageChange();

    void ok_btn_clicked();
    void cancel_btn_clicked();
    void sim_type_comboBox_select( int );
    void useheight_comboBox_select( int );
};

#endif // CLUTTER_SIM_DIA_H
