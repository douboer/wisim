/******************************************************************************************/
/**** PROGRAM: pref_dia_db.cpp                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef PREF_DIA_DB_H 
#define PREF_DIA_DB_H

#include <qdialog.h>
#include <qwidget.h>
#include <q3listview.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include "wisim.h"

class NetworkClass;
class QRadioButton;
class QLineEdit;
class Q3GridLayout;
class Q3ButtonGroup;
class Q3GroupBox;
class QLabel;

/******************************************************************************************/
/**** CLASS: ConnectionNameDia                                                         ****/
/******************************************************************************************/
class ConnectionNameDia : public QDialog
{
    Q_OBJECT
public:
    ConnectionNameDia(NetworkClass *np_param, QWidget* parent, const char* name);
    ~ConnectionNameDia();
    QLineEdit  *addLineEdit;
    QLineEdit  *nameLineEdit;
    QLineEdit  *portLineEdit;
    QLineEdit  *ipLineEdit;
    QLineEdit  *sidLineEdit;
    QLineEdit  *usr_nameLineEdit;
    QLineEdit  *passwdLineEdit;
    int repetition_flag;
private slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();
private:
    NetworkClass *np;
};

class DbStDia: public QWidget
{   
    Q_OBJECT
public:
    DbStDia(NetworkClass *np_param, QWidget* parent);
    ~DbStDia();

    QLabel     *nameLineEdit;
    QLabel     *ipLineEdit;
    QLabel     *sidLineEdit;
    QLabel     *usr_nameLineEdit;
    QLabel     *passwdLineEdit;
    QLabel     *portLineEdit;
    Q3ListView     *dbListView;
    Q3ListViewItem **dbnListView;
    char          *lv_text;
    int           lv_num;
    QPushButton   *connect_btn;
    QPushButton   *delete_btn;
    Q3ButtonGroup  *dbButtonGroup;
    Q3GridLayout   *dbsetting_diaLayout;
    ConnectionNameDia   *conn_name_dia;
private slots:
    void add_btn_clicked();
    void delete_btn_clicked();
    void listview_selected(Q3ListViewItem *lview_item );
    void connect_btn_clicked();
    void export_btn_clicked();
private:
    NetworkClass *np;
};

#endif
