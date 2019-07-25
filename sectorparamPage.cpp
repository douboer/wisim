
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>

#include "cconst.h"
#include "sectorparamPage.h"
#include "phs.h"
#include "wisim.h"
#include "antenna.h"
#include "unusedfreq.h"
#include "prop_model.h"

#include <iostream>

/******************************************************************************************/
/**** FUNCTION: sectorparamPage::sectorparamPage                                       ****/
/******************************************************************************************/
sectorparamPage::sectorparamPage(NetworkClass *np_param, CellClass *cell, int m_cell_idx, int m_sector_idx,
    QWidget *parent,const char *name) : QWidget(parent,name)
{
    np = np_param;
    cell_idx = m_cell_idx;
    sector_idx = m_sector_idx;
    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
    AntennaClass *antenna;
    PropModelClass *pm;
    char str[100];
    int ant_idx, pm_idx;

    Q3GridLayout *sectorParamGrid = new Q3GridLayout( this, 7, 4, 15, 15 );

#if CGDEBUG 
    num_unused_freq_lbl = new QLabel(tr("number of unused frequencies"),this);
    sectorParamGrid->addWidget(num_unused_freq_lbl,5,2);
    num_unused_freq_val = new QLineEdit(this);
    num_unused_freq_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(num_unused_freq_val,5,3);
    sprintf(str, "%d", sector->num_unused_freq);
    num_unused_freq_val->setText(str);
    num_unused_freq_val->setDisabled(true);

    comm_traffic_lbl = new QLabel(tr("communication traffic"),this);
    sectorParamGrid->addWidget(comm_traffic_lbl,0,0);
    comm_traffic_val = new QLineEdit(this);
    comm_traffic_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(comm_traffic_val,0,1);
    sprintf(str, "%01.3f", sector->comm_traffic);
    comm_traffic_val->setText(str);

    lreg_traffic_lbl = new QLabel(tr("location legistation traffic"),this);
    sectorParamGrid->addWidget(lreg_traffic_lbl,1,0);
    lreg_traffic_val = new QLineEdit(this);
    lreg_traffic_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(lreg_traffic_val,1,1);
    sprintf(str, "%01.3f", sector->lreg_traffic);
    lreg_traffic_val->setText(str);
#endif

    angle_deg_lbl = new QLabel(tr("Azimuth Angle (deg)\nN=0, E=90, S=180, W=270"),this);
    sectorParamGrid->addWidget( angle_deg_lbl,0,0);
    angle_deg_val = new QLineEdit(this);
    angle_deg_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(angle_deg_val,0,1);
    double degree = 90.0 - (sector->antenna_angle_rad)*180/PI;
    if (degree < 0.0) { degree += 360.0; }
    sprintf(str, "%01.3f", degree);
    angle_deg_val->setText(str);

    num_physical_tx_lbl = new QLabel(tr("number of physical transmiters"),this);
    sectorParamGrid->addWidget(num_physical_tx_lbl,0,2);
    num_physical_tx_val = new QLineEdit(this);
    num_physical_tx_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(num_physical_tx_val,0,3);
    sprintf(str, "%d", sector->num_physical_tx);
    num_physical_tx_val->setText(str);

    antenna_type_lbl = new QLabel(tr("antenna type"),this);
    sectorParamGrid->addWidget(antenna_type_lbl,1,0);
    antenna_type_combobox = new QComboBox(false,this);
    sectorParamGrid->addWidget(antenna_type_combobox,1,1);
    for (ant_idx=0; ant_idx<=np->num_antenna_type-1; ant_idx++) {
        antenna = np->antenna_type_list[ant_idx];
        antenna_type_combobox->insertItem(antenna->get_strid(), ant_idx);
    }
    antenna_type_combobox->setCurrentItem(sector->antenna_type);

    has_access_control_lbl = new QLabel(tr("set access control"),this);
    sectorParamGrid->addWidget(has_access_control_lbl,1,2);
    has_access_control_combobox = new QComboBox(false,this);
    sectorParamGrid->addWidget(has_access_control_combobox,1,3);
    has_access_control_combobox->insertItem(tr("no access control"),0);
    has_access_control_combobox->insertItem(tr("has access control"),1);
    has_access_control_combobox->setCurrentItem(sector->has_access_control);

    antenna_height_lbl = new QLabel(tr("antenna height"),this);
    sectorParamGrid->addWidget(antenna_height_lbl,2,0);
    antenna_height_val = new QLineEdit(this);
    antenna_height_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(antenna_height_val,2,1);
    sprintf(str, "%01.3f", sector->antenna_height);
    antenna_height_val->setText(str);


    cntl_chan_slot_lbl = new QLabel(tr("control channel slot"),this);
    sectorParamGrid->addWidget(cntl_chan_slot_lbl,2,2);
    cntl_chan_slot_val = new QLineEdit(this);
    cntl_chan_slot_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(cntl_chan_slot_val,2,3);
    sprintf(str, "%d", sector->cntl_chan_slot);
    cntl_chan_slot_val->setText(str);

#if CGDEBUG
    sync_level_lbl = new QLabel(tr("synchronization level"),this);
    sectorParamGrid->addWidget(sync_level_lbl,3,2);
    sync_level_val = new QLineEdit(this);
    sync_level_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget(sync_level_val,3,3);
    sprintf(str, "%d", sector->sync_level);
    sync_level_val->setText(str);
#endif

    tx_power_lbl = new QLabel(tr("transmit power"),this);
    sectorParamGrid->addWidget(tx_power_lbl,3,0);
    tx_power_val = new QLineEdit(this);
    tx_power_val->setGeometry(10,10, 130, 30);
    sectorParamGrid->addWidget( tx_power_val,3,1);
    sprintf(str, "%01.3f", sector->tx_pwr);
    tx_power_val->setText(str);

    //-------------------------------- CSID --------------------------------------------
    csid_lbl = new QLabel(tr("CSID"), this);
    sectorParamGrid->addWidget(csid_lbl,3,2);

    csid_val = new QLineEdit(this);
    sectorParamGrid->addWidget(csid_val,3,3);

    char *csid_hexstr = (char*) NULL;
    csid_hexstr = (char *)malloc(2*sector->csid_byte_length + 1);
    if ( np->technology() == CConst::PHS )
        hex_to_hexstr(csid_hexstr, sector->csid_hex, sector->csid_byte_length);
    csid_val->setText(csid_hexstr);
    free(csid_hexstr);
    //const char *csidtext = csid_val->text();

    //---- Propagation Model -----------------------------------------------------------
    prop_model_lbl = new QLabel(tr("Propagation Model"),this);
    sectorParamGrid->addWidget(prop_model_lbl,4,0);
    prop_model_combobox = new QComboBox(false,this);
    sectorParamGrid->addWidget(prop_model_combobox,4,1);
    for (pm_idx=0; pm_idx<=np->num_prop_model-1; pm_idx++) {
        pm = np->prop_model_list[pm_idx];
        prop_model_combobox->insertItem(pm->get_strid(), pm_idx);
    }
    prop_model_combobox->insertItem("UNASSIGNED");
    pm_idx = sector->prop_model;
    if (pm_idx == -1) { pm_idx = np->num_prop_model; }
    prop_model_combobox->setCurrentItem(pm_idx);

    //--------------------------------GW_CSC_CS ----------------------------------------
    gw_csc_cs_lbl = new QLabel(tr("GW_CSC_CS"), this);
    sectorParamGrid->addWidget(gw_csc_cs_lbl,4,2);

    gw_csc_cs_val = new QLineEdit(this);
    sectorParamGrid->addWidget(gw_csc_cs_val,4,3);

    char *gw_csc_cs_str = CVECTOR(7);
    sprintf(gw_csc_cs_str, "%.6d", sector->gw_csc_cs);
    gw_csc_cs_val->setText(gw_csc_cs_str);
    free(gw_csc_cs_str);

    //---------------------------------------------------------------------------------

#if CGDEBUG
    int num_traffic;
    num_traffic = sector->num_traffic; 
    std::cout << " num_traffic " << num_traffic << std::endl;
    for ( int i=0; i<num_traffic; i++ ) {
        std::cout << " meas_ctr " << sector->meas_ctr_list[i] << std::endl;
    }
#endif
    QString str1;

    Q3ButtonGroup* meas_ctr_buttonGroup = new Q3ButtonGroup( this, "meas_ctr_buttonGroup" );
    meas_ctr_buttonGroup->setColumnLayout(0, Qt::Vertical );
    meas_ctr_buttonGroup->layout()->setSpacing( 6 );
    meas_ctr_buttonGroup->layout()->setMargin( 11 );
    Q3GridLayout* meas_ctr_buttonGroupLayout = new Q3GridLayout( meas_ctr_buttonGroup->layout() );
    meas_ctr_buttonGroupLayout->setAlignment( Qt::AlignTop );
    meas_ctr_buttonGroup->setTitle( tr("Carried Traffic Rate ") );

    for(int i = 0; i<np->num_traffic_type; i++) {
        str1.sprintf( "%s", np->traffic_type_list[i]->name() );
        QLabel* comm_ctr_lbl = new QLabel( meas_ctr_buttonGroup, "comm_ctr_lbl" );
        meas_ctr_buttonGroupLayout->addWidget( comm_ctr_lbl, i, 0 );
        comm_ctr_lbl->setText( str1 );

        comm_ctr_val = new QLineEdit( meas_ctr_buttonGroup, "comm_ctr_val" );
        comm_ctr_val->setMaximumSize( QSize( 120, 32767 ) );
        meas_ctr_buttonGroupLayout->addWidget( comm_ctr_val, i, 1 );
        str1.sprintf( "%f", sector->meas_ctr_list[i] );
        comm_ctr_val->setText( str1 );

        ctr_vec.push_back( comm_ctr_val );
    }

    sectorParamGrid->addMultiCellWidget( meas_ctr_buttonGroup, 5, 5, 0, 4 );

    //---------------------------------------------------------------------------------

    QLabel* unused_frequency_lbl = new QLabel(this);
    sectorParamGrid->addMultiCellWidget(unused_frequency_lbl, 6, 6, 0, 1);
    unused_frequency_lbl->setText( tr("Edit Unused Frequency") );

    unused_frequency_btn = new QPushButton(tr("Edit"),this);
    unused_frequency_btn->setGeometry(180,80,130,30);
    sectorParamGrid->addMultiCellWidget(unused_frequency_btn,6,6,2,4,Qt::AlignCenter);

    //////////////////////////////////////////////////////////////////////////////////////////

#if CGDEBUG
    int num_rows = sector->num_unused_freq;
    unusedFreq *unused_freq = new unusedFreq(np, cell, sector_idx, num_rows, 0, "unused_freq_wid");
#endif

    connect( unused_frequency_btn, SIGNAL(clicked()), this, SLOT( unused_frequency_btn_clicked() ) ); 

    if (np->mode != CConst::editGeomMode) {
        angle_deg_val->setEnabled(false);
        antenna_type_combobox->setEnabled(false);
        antenna_height_val->setEnabled(false);
        tx_power_val->setEnabled(false);
        num_physical_tx_val->setEnabled(false);
        has_access_control_combobox->setEnabled(false);
        cntl_chan_slot_val->setEnabled(false);
        csid_val->setEnabled(false);
        gw_csc_cs_val->setEnabled(false);
    }
}
/******************************************************************************************/
/**** FUNCTION: sectorparamPage::unused_frequency_btn_clicked                         ****/
/******************************************************************************************/
void sectorparamPage::unused_frequency_btn_clicked()
{
    //int num_rows = atoi( num_unused_freq_val->text() );
    int num_rows = sector->num_unused_freq;
    unusedFreq *unused_freq = new unusedFreq(np, sector, num_rows, cell_idx, sector_idx, this, "unused_freq_wid");
    unused_freq->show();
}
