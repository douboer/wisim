#ifndef SECTORPAGE_H
#define SECTORPAGE_H

#include <qapplication.h>
#include <qwidget.h>

#include <q3popupmenu.h>
#include <q3mainwindow.h>
#include <qpushbutton.h>
#include <q3tabdialog.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <vector>

//class QLabel;
//class QLineedit;

class NetworkClass;
class PHSSectorClass;
class CellClass;
/******************************************************************************************/
/**** CLASS: sectorparamPage.h                                                         ****/
/******************************************************************************************/

class sectorparamPage : public QWidget
{
    Q_OBJECT

public:
    sectorparamPage(NetworkClass *np_param, CellClass *cell, int m_cell_idx, int m_sector_idx, QWidget *parent=0,const char *name=0);

    QPushButton *unused_frequency_btn;

    QLineEdit* comm_ctr_val;
    //QLineEdit* lreg_ctr_val;

    std::vector <QLineEdit*> ctr_vec; 

    QLabel *comm_traffic_lbl;
    QLineEdit *comm_traffic_val;

    QLabel *lreg_traffic_lbl;
    QLineEdit *lreg_traffic_val;

    QLabel *angle_deg_lbl;
    QLineEdit *angle_deg_val;

    QLabel *antenna_type_lbl;
    QComboBox *antenna_type_combobox;
    // QLineEdit *antenna_type_val;

    QLabel *antenna_height_lbl;
    QLineEdit *antenna_height_val;

    QLabel *tx_power_lbl;
    QLineEdit *tx_power_val;

    QLabel *num_physical_tx_lbl;
    QLineEdit *num_physical_tx_val;

    QLabel *has_access_control_lbl;
    QComboBox *has_access_control_combobox;

    QLabel *cntl_chan_slot_lbl;
    QLineEdit *cntl_chan_slot_val;
#if CGDEBUG
    QLabel *num_unused_freq_lbl;
    QLineEdit *num_unused_freq_val;
#endif
    QLabel *unused_freq_0_lbl;
    QLineEdit *unused_freq_0_val;

    QLabel *sync_level_lbl;
    QLineEdit *sync_level_val;
    //-----------------------------------------------------------------------------------
    QLabel *csid_lbl;
    QLineEdit *csid_val;

    QLabel *gw_csc_cs_lbl;
    QLineEdit *gw_csc_cs_val;

    QLabel *prop_model_lbl;
    QComboBox *prop_model_combobox;


public slots:
    void unused_frequency_btn_clicked(); 

private slots:

private:
    NetworkClass *np;
    PHSSectorClass *sector;
    int sector_idx;
    int cell_idx;

};	
/******************************************************************************************/
#endif


