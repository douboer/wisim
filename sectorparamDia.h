
/******************************************************************************************
**** PROGRAM: sectorparamDia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef SECTORPARAMDIA_H
#define SECTORPARAMDIA_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qdialog.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class QPushButton;
class Q3ButtonGroup;
class Q3GridLayout;
class Q3HBoxLayout;
class QLabel;
class QLineEdit;
class QMessageBox;
class QSpacerItem;
class Q3TabDialog;
class QTabWidget;
class QToolButton;

class CellClass;
class NetworkClass;
class sectorparamPage;
class SectorSpinBox;
/******************************************************************************************/
/**** CLASS: SectorparamDia                                                            ****/
/******************************************************************************************/
class sectorparamDia: public QDialog
{
    Q_OBJECT
public:
    sectorparamDia(NetworkClass *np_param, int m_cell_idx, QWidget *parent=0,const char *name=0, int has_posn=1);
    ~sectorparamDia( );

    Q3ButtonGroup *sectorParam;

    Q3ButtonGroup *cellParam;
    QLabel *position_lbl;
    QLineEdit *position_val;
    QLabel *sector_num_lbl;
    SectorSpinBox *sectorSpinBox;
    Q3GridLayout *topLayout;

    QLabel*  utm_x_lbl;
    QLabel*  utm_y_lbl;
    QLabel*  utm_lon_lbl;
    QLabel*  utm_lat_lbl;
//    QLabel*  lon_lat_lon_lbl;
//    QLabel*  lon_lat_lat_lbl;
//    QLabel*  generic_x_lbl;
//    QLabel*  generic_y_lbl;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;

    QToolButton* addButton;
    QToolButton* deleteButton;
    QToolButton* choose_toolButton;

    QLineEdit*  utm_x_val;
    QLineEdit*  utm_y_val;
    QLineEdit*  utm_lon_val;
    QLineEdit*  utm_lat_val;
    QLineEdit*  lon_lat_lon_val;
    QLineEdit*  lon_lat_lat_val;
    QLineEdit*  generic_x_val;
    QLineEdit*  generic_y_val;

    Q3ButtonGroup* buttonGroup1;
    Q3ButtonGroup* buttonGroup2;
    Q3HBoxLayout*  buttonGroup1Layout;
    Q3GridLayout*  buttonGroup2Layout;

    QTabWidget *tdialog;
    QLabel *comm_traffic_lbl;
    QLineEdit *comm_traffic_val;

    QLabel *lreg_traffic_lbl;
    QLineEdit *lreg_traffic_val;

    QLabel *angle_deg_lbl;
    QLineEdit *angle_deg_val;

    QLabel *antenna_type_lbl;
    QLineEdit *antenna_type_val;

    QLabel *antenna_height_lbl;
    QLineEdit *antenna_height_val;

    QLabel *tx_power_lbl;
    QLineEdit *tx_power_val;

    QLabel *num_physical_tx_lbl;
    QLineEdit *num_physical_tx_val;

    QLabel *has_access_control_lbl;
    QLineEdit *has_access_control_val;

    QLabel *cntl_chan_slot_lbl;
    QLineEdit *cntl_chan_slot_val;

    QLabel *num_unused_freq_lbl;
    QLineEdit *num_unused_freq_val;

    QLabel *unused_freq_0_lbl;
    QLineEdit *unused_freq_0_val;

    Q3TabDialog *sector_param_tbd;	

    sectorparamPage *page[5];

    char *countstr;
    char *sectorStr;
/////////////////////////////////////////////////////////////////////////////////////////////////////
    QPushButton *ok_btn;
    QPushButton *cancel_btn;

    int count;	
    int sectorNumber;
    int prevNumber;
    int diffValue;

public slots:
    void add_sector_clicked();
    void delete_sector_clicked();
    //void show_sector_param_dia();
    void sector_num_spb_changed( int );

private slots:
    void ok_button_clicked();
    void cancel_button_clicked();
    void x_changed();
    void y_changed();
    void lat_changed();
    void lon_changed();
    void x_edited();
    void y_edited();
    void lon_edited();
    void lat_edited();
    void display();

private:
    NetworkClass *np;
    CellClass *cell;
    int cell_idx;
//	int sector_idx;
    int currentCellIdx;
//	char str[100];
    char string[100];
    char string1[100];
    QString csid_tmp;
    bool posn_edt;
    bool x_edt, y_edt, lon_edt, lat_edt;
};
/******************************************************************************************/
/**** CLASS: SectorSpinBox                                                             ****/
/******************************************************************************************/
class SectorSpinBox: public QSpinBox
{
public:
    SectorSpinBox( QWidget * parent = 0, const char * name = 0 );
    // void setSpinBoxValue(int val) { directSetValue(val); }
};
/******************************************************************************************/
#endif
