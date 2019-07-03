/******************************************************************************************/
/**** PROGRAM: create_subnet_dia.cpp                                                   ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef CRE_SUBNET_DIA_H
#define CRE_SUBNET_DIA_H

#include <qdialog.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3Frame>

const int INIT_GRP_NUM = 10;

class QLabel;
class Q3HBoxLayout;
class Q3GridLayout;
class QCheckBox;
class Q3ButtonGroup;
class QLineEdit;
class Q3ScrollView;
class Q3Frame;
class QComboBox;

class AdvCreSubnetDia;
class SectorgroupDia;
class NetworkClass;

class CreSubnetDia : public QDialog
{
    Q_OBJECT

public:
    CreSubnetDia(NetworkClass* np_param, QWidget* parent = 0);
    ~CreSubnetDia();

public slots:
    void ok_btn_clicked();
    void adv_btn_clicked();
    void cancel_btn_clicked();
    void group_sector_btn_clicked();

protected:
    Q3GridLayout* cre_subnet_diaLayout;
    Q3GridLayout* map_layer_btngroupLayout;

protected slots:
    virtual void languageChange();

private:
    NetworkClass*     np;
    bool              advancedShown;
    SectorgroupDia    *sectorgroup_dia;

    QCheckBox    **map_layer_chkbox;

    QPushButton  *ok_btn;
    QPushButton  *adv_btn;
    QPushButton  *cancel_btn;
    QPushButton  *group_sector_btn;

    Q3ButtonGroup *map_layer_btngroup;
    AdvCreSubnetDia *advancedDialog;
};

class AdvCreSubnetDia : public QWidget
{
    Q_OBJECT

public:
    AdvCreSubnetDia(NetworkClass* np_param, QWidget* parent = 0);
    ~AdvCreSubnetDia();

    friend class CreSubnetDia;

private:
    NetworkClass *np;

    Q3GridLayout *adv_cre_subnet_diaLayout;
    QLabel *init_sample_res_lbl;
    QLabel *scan_fractional_area_lbl;
    QLineEdit *scan_fractional_area_val;
    QLineEdit *init_sample_res_val;
};


/******************************************************************************************/
/****                            CLASS: SectorgroupDia                                 ****/
/******************************************************************************************/
class SectorgroupDia: public QDialog
{
    Q_OBJECT
public:
    SectorgroupDia( NetworkClass *np_param, QWidget* parent = 0);
    ~SectorgroupDia();

protected:
    QPushButton *ok_btn;
    QPushButton *create_btn;
    QPushButton *cancel_btn;

    Q3ScrollView *sector_group_scrollview;
    Q3Frame      *sector_group_frame;

    Q3HBoxLayout *create_ok_cancel_hboxLayout;
    Q3GridLayout *sector_group_diaLayout;
    Q3GridLayout *sector_group_frameLayout;
    Q3GridLayout *sector_group_scrviewLayout;

    QComboBox   *cell_sector_name_combobox;
    QLabel      **sector_group_name_lbl;
    QLabel      **cell_sector_name_lbl;

    Q3ButtonGroup *sector_group_btnGroup;
    QCheckBox    **cell_sector_checkbox;

public slots:
    void create_btn_clicked();

private slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();

    void sector_group_clicked(int);
    void cell_sector_name_combobox_changed(int);

    virtual void resizeEvent( QResizeEvent *e );

private:
    bool           createflg;
    int            sector_number;
    int            newgroup_number;
    char           **cell_sector_name;
    NetworkClass   *np;
};

#endif // CRE_SUBNET_DIA_H

