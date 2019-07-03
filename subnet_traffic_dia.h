
/******************************************************************************************
**** PROGRAM: subnet_traffic_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef SUBNET_TRAFFIC_DIA_H
#define SUBNET_TRAFFIC_DIA_H

#include <qvariant.h>
#include <qdialog.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include <vector>
#include <q3table.h>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;
class Q3ButtonGroup;
class QRadioButton;
class Q3Frame;
class QComboBox;
class TableItem;
class NetworkClass;
class SubnetClass;

class MTable : public Q3Table
{
public:
    MTable( QWidget* parent=0, const char* name=0 );
    void sortColumn( int col, bool ascending, bool);
};

class SubnetTraffic : public QDialog
{
    Q_OBJECT

public:
    SubnetTraffic( NetworkClass *np_param, QWidget *parent );
    ~SubnetTraffic();

    QPushButton*  buttonHelp;
    QPushButton*  buttonApply;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;
    MTable*       subnet_traffic_table;
    MTable*       table_tmp;
    QLabel*       total_textLabel;

    QLineEdit*    total_area_val;
    QLineEdit*    total_arrival_rate_val;
    QLineEdit*    total_traffic_val;
    QLineEdit*    total_density_val;
    QLineEdit*    total_faction_val;

    Q3ButtonGroup* traffic_buttonGroup;
    QLabel*       mean_time_lbl;
    QLineEdit*    mean_time_val;
    QRadioButton* unif_radioButton;
    QLabel*       min_time_lbl;
    QLineEdit*    min_time_val;
    QLabel*       max_time_lbl;
    QLineEdit*    max_time_val;
    QRadioButton* exp_radioButton;
    Q3Frame*       line;
    QLineEdit*    arrival_rate_val;
    QComboBox*    traffic_comboBox;
    QLabel*       arrival_rate_textLabel;
    QLabel*       traffic_type_textLabel;


    Q3ButtonGroup* numAttemp_buttonGroup;
    Q3HBoxLayout* numAttemp_buttonGroupLayout;
    QLabel* num_attemp_request_lbl;
    QLineEdit* num_attemp_request_val;
    Q3Frame* line_2;
    QLabel* num_attemp_handover_lbl;
    QLineEdit* num_attemp_handover_val;

    TableItem*    tab_item;

    QCheckBox*          ckb;
    Q3CheckTableItem*    ckb_item;
    std::vector<Q3CheckTableItem*> ckb_item_vec;


protected:
    Q3GridLayout*  SubnetTrafficLayout;

    Q3GridLayout*  typeLayout;
    QSpacerItem*  spacer1;
    QSpacerItem*  spacer2;

    Q3GridLayout*  traffic_buttonGroupLayout;
    Q3HBoxLayout*  meanLayout;
    QSpacerItem*  spacer3;
    Q3GridLayout*  minMaxLayout;
    QSpacerItem*  spacer4;

    Q3GridLayout*  trafficTableLayout;
    Q3HBoxLayout*  totalTrafficLayout;
    //QSpacerItem*  spacer5;
    QLabel*       lbl;

    Q3HBoxLayout*  okCancelLayout;
    QSpacerItem*  spacer6;

    SubnetClass*  subnet_class;
    NetworkClass* np;

    std::vector <int> number_subnet;
    QString str;
    int number_traffic_type;
    int traffic_type_idx;

    double   single_area             ;
    double   single_arrival_rate     ;
    double   single_traffic          ;
    double   single_density          ;
    double   single_faction          ;
    double   total_traffic_proportion;
    double   total_area              ;
    double   total_arrival_rate      ;
    double   total_traffic           ;
    double   total_density           ;
    double   total_faction           ;

    int      num_attempt_request     ;
    int      num_attempt_handover    ;

    int editRow;
    int editCol;

    void update_all_value();
    void recreate_table();
    void process_cmd();

protected slots:
    virtual void languageChange();

    void apply_btn_clicked();
    void help_btn_clicked();
    void ok_btn_clicked();
    void cancel_btn_clicked();
    void hHeadSizeChanged( int );
    void traffic_comboBox_select(int);
    void traffic_buttonGroup_clicked( int );
    void tab_valueChanged(int, int );
    void total_val_changed();
    void all_ckb_selected(int);
    void delete_subnet_func();

    void headClicked( int );
//    void exp_radioButton_clicked();
//    void uniform_radioButton_clicked();
};

#endif // SUBNET_TRAFFIC_DIA_H
