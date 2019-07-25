
/******************************************************************************************
**** PROGRAM: sectorparamDia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <iostream>

#include <q3buttongroup.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <QLabel>

#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "icons_test.h"
#include "map_clutter.h"
#include "phs.h"
#include "sectorparamDia.h"
#include "sectorparamPage.h"
#include "utm_conversion.h"
#include "wcdma.h"
#include "wlan.h"

/******************************************************************************************/
/**** FUNCTION: sectorparamDia::sectorparamDia                                         ****/
/******************************************************************************************/
sectorparamDia::sectorparamDia(NetworkClass *np_param, int m_cell_idx, QWidget *parent,
        const char *name, int has_posn) : QDialog(parent, name, true)
{
    setName( "sectorparamDia" );

    np = np_param;
    count         = 0;
    posn_edt      = false;
    x_edt         = false;
    y_edt         = false;
    lon_edt       = false;
    lat_edt       = false;
    int    sector_idx = 0;
    double posn_x     = 0;
    double posn_y     = 0;
    double lon_deg    = 0;
    double lat_deg    = 0;

    cell_idx = m_cell_idx;

    QString param_val;

    QString caption;
    if (cell_idx == -1) {
        switch(np->technology()) {
            case CConst::PHS:   cell = (CellClass *) new PHSCellClass(1);   break;
            case CConst::WCDMA: cell = (CellClass *) new WCDMACellClass(1); break;
            case CConst::WLAN:  cell = (CellClass *) new WLANCellClass(1);  break;
            default: CORE_DUMP; break;
        }
        caption = tr("Add Cell Parameters");
    } else {
        cell     = np->cell_list[cell_idx]->duplicate(np->cell_list[cell_idx]->posn_x, np->cell_list[cell_idx]->posn_y, 1);
        caption = tr("Cell") + QString(" %1 ").arg(cell_idx) + tr("Parameters");
    }
    setCaption(caption);

    Q3GridLayout *topLayout = new Q3GridLayout( this,  1, 1, 11, 15, "topLayout" );
    //--------------------------------------------------------------------------------------------------------------------
    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setTitle( tr("Create or Delete Sector") );
    buttonGroup1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, buttonGroup1->sizePolicy().hasHeightForWidth() ) );
    buttonGroup1->setMaximumSize( QSize( 200, 32767 ) );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 6 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new Q3HBoxLayout( buttonGroup1->layout(), 15, "buttonGroup1Layout" );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );
    addButton = new QToolButton( buttonGroup1, "addButton" );
    addButton->setIconSet( QIcon( TestIcon::icon_create ) );
    buttonGroup1Layout->addWidget( addButton );
    spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer1 );
    deleteButton = new QToolButton( buttonGroup1, "deleteButton" );
    deleteButton->setIconSet( QIcon( TestIcon::icon_delete ) );
    buttonGroup1Layout->addWidget( deleteButton );
    spacer2 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer2 );
    choose_toolButton = new QToolButton( buttonGroup1, "deleteButton" );
    choose_toolButton->setIconSet( QIcon( TestIcon::icon_choose ) );
    choose_toolButton->hide();
    buttonGroup1Layout->addWidget( choose_toolButton );
    QSpacerItem* spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer3 );
    topLayout->addMultiCellWidget( buttonGroup1, 0, 0, 0, 1 );
    //--------------------------------------------------------------------------------------------------------------------
    cellParam = new Q3ButtonGroup(2, Qt::Horizontal, this);
    sector_num_lbl = new QLabel(tr("number of sectors"), cellParam);
    sectorSpinBox = new SectorSpinBox(cellParam);
    sectorSpinBox->setGeometry(10,10, 80, 30);
    sectorSpinBox->setMaximumSize(150,45);
    sectorSpinBox->setDisabled(true);
    topLayout->addMultiCellWidget( cellParam, 0, 0, 2, 3 );
    cellParam->hide();


    //--------------------------------------------------------------------------------------------------------------------
    buttonGroup2 = new Q3ButtonGroup( this, "buttonGroup2" );
    buttonGroup2->setTitle( tr("POSITION") );
    //buttonGroup2->setGeometry( QRect( 20, 19, 270, 94 ) );
    buttonGroup2->setColumnLayout(0, Qt::Vertical );
    buttonGroup2->layout()->setSpacing( 5 );
    buttonGroup2->layout()->setMargin( 11 );
    buttonGroup2Layout = new Q3GridLayout( buttonGroup2->layout(), 1, 1, -1, "buttonGroup2Layout" );
    buttonGroup2Layout->setAlignment( Qt::AlignTop );

    utm_x_lbl = new QLabel( buttonGroup2, "utm_x_lbl" );
    buttonGroup2Layout->addWidget( utm_x_lbl, 0, 0 );
    utm_x_val = new QLineEdit( buttonGroup2, "utm_x_val" );
    utm_x_val->setMinimumSize(150,22);
    buttonGroup2Layout->addWidget( utm_x_val, 0, 1 );

    utm_y_lbl = new QLabel( buttonGroup2, "utm_y_lbl" );
    buttonGroup2Layout->addWidget( utm_y_lbl, 0, 2 );
    utm_y_val = new QLineEdit( buttonGroup2, "utm_y_val" );
    utm_y_val->setMinimumSize(150,22);
    buttonGroup2Layout->addWidget( utm_y_val, 0, 3 );

    if (has_posn) {
        posn_x = np->idx_to_x(cell->posn_x);
        posn_y = np->idx_to_y(cell->posn_y);
        param_val = QString("%1").arg(posn_x, 9, 'f', 7);
        utm_x_val->setText(param_val);
        param_val = QString("%1").arg(posn_y, 9, 'f', 7);
        utm_y_val->setText(param_val);
    }

    if (np->coordinate_system == CConst::CoordUTM) {
        utm_lon_lbl = new QLabel( buttonGroup2, "utm_lon_lbl" );
        buttonGroup2Layout->addWidget( utm_lon_lbl, 1, 0 );
        utm_lon_val = new QLineEdit( buttonGroup2, "utm_lon_val" );
        utm_lon_val->setMinimumSize(150,22);
        buttonGroup2Layout->addWidget( utm_lon_val, 1, 1 );

        utm_lat_lbl = new QLabel( buttonGroup2, "utm_lat_lbl" );
        buttonGroup2Layout->addWidget( utm_lat_lbl, 1, 2 );
        utm_lat_val = new QLineEdit( buttonGroup2, "utm_lat_val" );
        utm_lat_val->setMinimumSize(150,22);
        buttonGroup2Layout->addWidget( utm_lat_val, 1, 3 );
        topLayout->addMultiCellWidget( buttonGroup2, 0, 0, 2, 3 );
        if (has_posn) {
            UTMtoLL( posn_x, posn_y, lon_deg,  lat_deg, np->utm_zone, np->utm_north,
                 np->utm_equatorial_radius, np->utm_eccentricity_sq);
            param_val = QString("%1").arg(lon_deg, 9, 'f', 7);
            utm_lon_val->setText(param_val);
            param_val = QString("%1").arg(lat_deg, 9, 'f', 7);
            utm_lat_val->setText(param_val);
        }
    }

    //--------------------------------------------------------------------------------------------------------------------

    if (np->coordinate_system == CConst::CoordLONLAT) {
        utm_x_lbl->setText( tr("LON") );
        utm_y_lbl->setText( tr("LAT") );
    } else {
        utm_x_lbl->setText( "X" );
        utm_y_lbl->setText( "Y" );
    }

    if (np->coordinate_system == CConst::CoordUTM) {
        utm_lon_lbl->setText( tr("LON") );
        utm_lat_lbl->setText( tr("LAT") );
    }


    addButton->setTextLabel( tr("Create Sector") );
    addButton->setAutoRaise( true );
    deleteButton->setTextLabel( tr("Delete Sector") );
    deleteButton->setAutoRaise( true );
    choose_toolButton->setTextLabel( tr("unused yet!!") );
    choose_toolButton->setAutoRaise( true );
    //--------------------------------------------------------------------------------------------------------------------

    connect(addButton,SIGNAL(clicked()),this,SLOT(add_sector_clicked()));
    connect(deleteButton,SIGNAL(clicked()),this,SLOT(delete_sector_clicked()));

    tdialog = new QTabWidget(this);                                           //change
    for (sector_idx = 0; sector_idx <= cell->num_sector-1; sector_idx++) {
        page[sector_idx] = new sectorparamPage(np, cell, cell_idx, sector_idx, this,"sectorpage");
        sectorStr = "sector";
        sectorSpinBox->setValue(cell->num_sector);
        tdialog->addTab(page[sector_idx],sectorStr);
    }
    topLayout->addMultiCellWidget( tdialog, 2, 2, 0, 3 );

    Q3HBoxLayout *okcancelLayout = new Q3HBoxLayout(0, 10, -1, "okcancelLayout" );

    ok_btn = new QPushButton(tr("OK"), this);
    ok_btn->setGeometry(0,0,50,30);
    okcancelLayout->addWidget(ok_btn);
    ok_btn->setMaximumWidth(80);
    ok_btn->setEnabled( has_posn ? true : false );
    ok_btn->setFocus();

    cancel_btn = new QPushButton(tr("Cancel"), this);
    cancel_btn->setGeometry(0,0,50,30);
    okcancelLayout->addWidget(cancel_btn);
    cancel_btn->setMaximumWidth(80);

    topLayout->addMultiCellLayout(okcancelLayout, 3, 3, 0, 3);

    connect( ok_btn,     SIGNAL( clicked() ), this, SLOT( ok_button_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_button_clicked() ) );

    if (np->mode != CConst::editGeomMode) {
        ok_btn->setEnabled(false);
        addButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }


    connect(utm_x_val, SIGNAL(textChanged(const QString&)), this, SLOT(x_edited()));
    connect(utm_y_val, SIGNAL(textChanged(const QString&)), this, SLOT(y_edited()));
    connect(utm_x_val, SIGNAL(lostFocus()), this, SLOT(x_changed()));
    connect(utm_y_val, SIGNAL(lostFocus()), this, SLOT(y_changed()));

    if (np->coordinate_system == CConst::CoordUTM) {
        connect(utm_lon_val, SIGNAL(textChanged(const QString&)), this, SLOT(lon_edited()));
        connect(utm_lat_val, SIGNAL(textChanged(const QString&)), this, SLOT(lat_edited()));
        connect(utm_lon_val, SIGNAL(lostFocus()), this, SLOT(lon_changed()));
        connect(utm_lat_val, SIGNAL(lostFocus()), this, SLOT(lat_changed()));
    }
}

sectorparamDia::~sectorparamDia()
{
}

/******************************************************************************************/
/**** FUNCTION: sectorparamDia::add_sector_clicked                                     ****/
/******************************************************************************************/
void sectorparamDia::add_sector_clicked()
{
    int sector_idx;
    if ( cell->num_sector < 5 )
    {
        cell->sector_list = (SectorClass **) realloc((void *) cell->sector_list, (cell->num_sector+1)*sizeof(SectorClass *));
        cell->num_sector++;
        sector_idx = cell->num_sector-1;
        cell->sector_list[sector_idx] = cell->sector_list[0]->duplicate(0);
        cell->sector_list[sector_idx]->parent_cell = cell;
        page[sector_idx] = new sectorparamPage(np, cell, cell_idx, sector_idx, this,"sectorpage");
        printf("\n%s\n","add a sector");
        tdialog->addTab(page[sector_idx],sectorStr);
        tdialog->showPage(page[sector_idx]);
        sectorSpinBox->setValue(sector_idx);
    }
    else
    {
        QMessageBox::warning( this, "Warning",
                QString( tr("Number of sector must less than 5 !")),
                QMessageBox::Warning,
                0
                );
    }
}

/******************************************************************************************/
/**** FUNCTION: sectorparamDia::delete_sector_clicked                                  ****/
/******************************************************************************************/
void sectorparamDia::delete_sector_clicked()
{
    int sector_idx;

    if(cell->num_sector == 1) {
        QMessageBox::warning( this, "Warning",
                QString( tr("Number of sector must more than 1 !")),
                QMessageBox::Warning,
                0
                );
    }
    if(cell->num_sector > 1) {
        sector_idx = cell->num_sector-1;
        tdialog->removePage(page[sector_idx]);
        sectorSpinBox->setValue(sector_idx-1);

        cell->num_sector--;
        delete cell->sector_list[sector_idx];
        cell->sector_list = (SectorClass **) realloc((void *) cell->sector_list, cell->num_sector*sizeof(SectorClass *));
    }
}
/******************************************************************************************/
/**** function: sectorparamdia::x_changed                                              ****/
/******************************************************************************************/
void sectorparamDia::x_changed()
{
    int grid_x, grid_y;
    double posn_x, posn_y;
    double lon_deg, lat_deg;
    QString param_val;
    bool focus = utm_y_val->hasFocus();
    bool vali_x,vali_y;

    if (utm_x_val->text().isEmpty() || utm_y_val->text().isEmpty()) {
        utm_lon_val->setText("");
        utm_lat_val->setText("");
        ok_btn->setEnabled( false );
    }
    else if ( x_edt == true && !focus)
    {
        utm_x_val->text().toDouble(&vali_x);
        utm_y_val->text().toDouble(&vali_y);
        if ( !vali_x || !vali_y ) {
            QMessageBox::warning( this, "Warning",
                                  QString( tr("UTM Coordination is invalid!")),
                                  QMessageBox::Warning,
                                  0
                                );
        }
        else {
            posn_x = atof(utm_x_val->text());
            check_grid_val(posn_x, np->resolution, np->system_startx, &grid_x);
            posn_y = atof(utm_y_val->text());
            check_grid_val(posn_y, np->resolution, np->system_starty, &grid_y);

            if (np->coordinate_system == CConst::CoordUTM) {
                UTMtoLL( posn_x, posn_y, lon_deg,  lat_deg, np->utm_zone, np->utm_north,
                        np->utm_equatorial_radius, np->utm_eccentricity_sq);
                param_val = QString("%1").arg(lon_deg, 9, 'f', 7);
                utm_lon_val->setText(param_val);
                param_val = QString("%1").arg(lat_deg, 9, 'f', 7);
                utm_lat_val->setText(param_val);
            }

            ok_btn->setEnabled( true );
        }
    }
    posn_edt = true;
    x_edt = false;
    y_edt = false;
    lon_edt = false;
    lat_edt = false;
    if ( x_edt == true && focus )
        y_edt = true;
}

/******************************************************************************************/
/**** function: edFreQEctorparamdia::y_changed                                              ****/
/******************************************************************************************/
void sectorparamDia::y_changed()
{
    int grid_x, grid_y;
    double posn_x, posn_y;
    double lon_deg, lat_deg;
    QString param_val;
    bool focus = utm_x_val->hasFocus();
    bool vali_x,vali_y;

    if (utm_x_val->text().isEmpty() || utm_y_val->text().isEmpty()) {
        utm_lon_val->setText("");
        utm_lat_val->setText("");
        ok_btn->setEnabled( false );
    } else if ( y_edt == true && !focus)  {
        utm_x_val->text().toDouble(&vali_x);
        utm_y_val->text().toDouble(&vali_y);
        if ( !vali_x || !vali_y ) {
            QMessageBox::warning( this, "Warning",
                                  QString( tr("UTM Coordination is invalid!")),
                                  QMessageBox::Warning,
                                  0
                                );
        }
        else {
            posn_x = atof(utm_x_val->text());
            check_grid_val(posn_x, np->resolution, np->system_startx, &grid_x);
            posn_y = atof(utm_y_val->text());
            check_grid_val(posn_y, np->resolution, np->system_starty, &grid_y);

            if (np->coordinate_system == CConst::CoordUTM) {
                UTMtoLL( posn_x, posn_y, lon_deg,  lat_deg, np->utm_zone, np->utm_north,
                        np->utm_equatorial_radius, np->utm_eccentricity_sq);
                param_val = QString("%1").arg(lon_deg, 9, 'f', 7);
                utm_lon_val->setText(param_val);
                param_val = QString("%1").arg(lat_deg, 9, 'f', 7);
                utm_lat_val->setText(param_val);
            }

            ok_btn->setEnabled( true );
        }
    }
    x_edt = false;
    posn_edt = true;
    y_edt = false;
    lon_edt = false;
    lat_edt = false;
    if (  y_edt == true && focus )
        x_edt = true;
}
/******************************************************************************************/
/**** function: sectorparamdia::lon_changed                                            ****/
/******************************************************************************************/
void sectorparamDia::lon_changed()
{
    int grid_x, grid_y;
    double posn_x, posn_y;
    double lon_deg, lat_deg;
    QString param_val;
    bool focus = utm_lat_val->hasFocus();
    bool vali_lon,vali_lat;

    if (utm_lon_val->text().isEmpty() || utm_lat_val->text().isEmpty()) {
        utm_x_val->setText("");
        utm_y_val->setText("");
        ok_btn->setEnabled( false );
    } else if ( lon_edt == true && !focus ) {
        utm_lon_val->text().toDouble(&vali_lon);
        utm_lat_val->text().toDouble(&vali_lat);
        if ( !vali_lon || !vali_lat ) {
            QMessageBox::warning( this, "Warning",
                                  QString( tr("LON-LAT is invalid!")),
                                  QMessageBox::Warning,
                                  0
                                );
        }
        else {
            lon_deg = atof(utm_lon_val->text());
            lat_deg = atof(utm_lat_val->text());

            LLtoUTM( lon_deg, lat_deg, posn_x, posn_y, np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);

            check_grid_val(posn_x, np->resolution, np->system_startx, &grid_x);
            check_grid_val(posn_y, np->resolution, np->system_starty, &grid_y);

            posn_x = np->idx_to_x(grid_x);
            param_val = QString("%1").arg(posn_x, 9, 'f', 7);
            utm_x_val->setText(param_val);

            posn_y = np->idx_to_y(grid_y);
            param_val = QString("%1").arg(posn_y, 9, 'f', 7);
            utm_y_val->setText(param_val);
            ok_btn->setEnabled( true );
        }
    }
    posn_edt = true;
    x_edt = false;
    y_edt = false;
    lon_edt = false;
    lat_edt = false;
    if ( lon_edt == true && focus )
        lat_edt = true;
}

/******************************************************************************************/
/**** function: sectorparamdia::lat_changed                                            ****/
/******************************************************************************************/
void sectorparamDia::lat_changed()
{
    int grid_x, grid_y;
    double posn_x, posn_y;
    double lon_deg, lat_deg;
    QString param_val;
    bool focus = utm_lon_val->hasFocus();
    bool vali_lon,vali_lat;

    if (utm_lon_val->text().isEmpty() || utm_lat_val->text().isEmpty()) {
        utm_x_val->setText("");
        utm_y_val->setText("");
        ok_btn->setEnabled( false );
    } else if ( lat_edt == true && !focus ) {
        utm_lon_val->text().toDouble(&vali_lon);
        utm_lat_val->text().toDouble(&vali_lat);

        if ( !vali_lon || !vali_lat ) {
            QMessageBox::warning( this, "Warning",
                                  QString( tr("LON-LAT is invalid!")),
                                  QMessageBox::Warning,
                                  0
                                );
        }
        else {
            lon_deg = atof(utm_lon_val->text());
            lat_deg = atof(utm_lat_val->text());

            LLtoUTM( lon_deg, lat_deg, posn_x, posn_y, np->utm_zone, np->utm_north, np->utm_equatorial_radius, np->utm_eccentricity_sq);

            check_grid_val(posn_x, np->resolution, np->system_startx, &grid_x);
            check_grid_val(posn_y, np->resolution, np->system_starty, &grid_y);

            posn_x = np->idx_to_x(grid_x);
            param_val = QString("%1").arg(posn_x, 9, 'f', 7);
            utm_x_val->setText(param_val);

            posn_y = np->idx_to_y(grid_y);
            param_val = QString("%1").arg(posn_y, 9, 'f', 7);
            utm_y_val->setText(param_val);
            ok_btn->setEnabled( true );
        }
    }
    posn_edt = true;
    x_edt = false;
    y_edt = false;
    lon_edt = false;
    lat_edt = false;
    if (  lat_edt == true && focus )
        lon_edt = true;
}

void sectorparamDia::x_edited()
{
    x_edt = true;
    y_edt = false;
    lon_edt = false;
    lat_edt = false;
    ok_btn->setEnabled( false );
}

void sectorparamDia::y_edited()
{
    x_edt = false;
    y_edt = true;
    lon_edt = false;
    lat_edt = false;
    ok_btn->setEnabled( false );
}

void sectorparamDia::lon_edited()
{
    x_edt = false;
    y_edt = false;
    lon_edt = true;
    lat_edt = false;
    ok_btn->setEnabled( false );
}

void sectorparamDia::lat_edited()
{
    x_edt = false;
    y_edt = false;
    lon_edt = false;
    lat_edt = true;
    ok_btn->setEnabled( false );
}
/******************************************************************************************/
/**** function: sectorparamdia::sector_num_spb_changed                                 ****/
/******************************************************************************************/
void sectorparamDia::sector_num_spb_changed( int )
{
}
/******************************************************************************************/
/**** FUNCTION: sectorparamDia::display                                                ****/
/******************************************************************************************/
void sectorparamDia::display( ) {
    show();
}

/******************************************************************************************/
/**** FUNCTION: sectorparamDia::ok_button_clicked                                      ****/
/******************************************************************************************/
void sectorparamDia::ok_button_clicked( )
{
    int i, sector_idx, pm_idx, unused_freq_edited;
    char *chptr;
    PHSSectorClass *sector;

    if (cell_idx == -1) {
        sprintf(np->line_buf, "add_cell -posn_x %s -posn_y %s",
                utm_x_val->text().latin1(), utm_y_val->text().latin1());
        np->process_command( np->line_buf);
        cell_idx = np->num_cell-1;
    } else {
        if ( posn_edt ) {
            sprintf(np->line_buf, "move_cell -cell_idx %d -posn_x %s -posn_y %s",
                    cell_idx, utm_x_val->text().latin1(), utm_y_val->text().latin1());
            np->process_command( np->line_buf);
        }
    }

    if (cell->num_sector != np->cell_list[cell_idx]->num_sector) {
        sprintf(np->line_buf, "set_cell -cell %d -num_sector %d", cell_idx, cell->num_sector);
        np->process_command( np->line_buf);
    }

    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {

        sector = (PHSSectorClass *) cell->sector_list[sector_idx];

        for ( i = 0; i<np->num_traffic_type; i++) {
            if (page[sector_idx]->ctr_vec[i]->edited()) {
                sprintf(np->line_buf, "set_sector_meas_ctr -sector %d_%d -traffic_type %s -meas_ctr %s",
                        cell_idx, sector_idx,
                        np->traffic_type_list[i]->name(),
                        page[sector_idx]->ctr_vec[i]->text().latin1());
                np->process_command( np->line_buf);
            }
        }

        if (page[sector_idx]->antenna_type_combobox->currentItem() != np->cell_list[cell_idx]->sector_list[sector_idx]->antenna_type) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -antenna_type %d",
                    cell_idx, sector_idx, page[sector_idx]->antenna_type_combobox->currentItem());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->antenna_height_val->edited()) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -antenna_height %s",
                    cell_idx, sector_idx, page[sector_idx]->antenna_height_val->text().latin1());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->angle_deg_val->edited()) {
            double angle_deg = page[sector_idx]->angle_deg_val->text().toDouble();
            angle_deg = 90.0 - angle_deg;
            sprintf(np->line_buf, "set_sector -sector %d_%d -antenna_angle_deg %13.8f",
                    cell_idx, sector_idx, angle_deg);
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->tx_power_val->edited()) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -tx_pwr %s",
                    cell_idx, sector_idx, page[sector_idx]->tx_power_val->text().latin1());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->num_physical_tx_val->edited()) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -num_physical_tx %s",
                    cell_idx, sector_idx, page[sector_idx]->num_physical_tx_val->text().latin1());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->cntl_chan_slot_val->edited()) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -cntl_chan_slot %s",
                    cell_idx, sector_idx, page[sector_idx]->cntl_chan_slot_val->text().latin1());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->has_access_control_combobox->currentItem() != ((PHSSectorClass *) np->cell_list[cell_idx]->sector_list[sector_idx])->has_access_control) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -has_access_control %d",
                    cell_idx, sector_idx, page[sector_idx]->has_access_control_combobox->currentItem());
            np->process_command( np->line_buf);
        }

        if (page[sector_idx]->csid_val->edited()) {
            csid_tmp = page[sector_idx]->csid_val->text();
            if ( csid_tmp.length() == 2*PHSSectorClass::csid_byte_length  ) {
                sprintf(np->line_buf, "set_sector -sector %d_%d -csid_hex %s",
                        cell_idx, sector_idx, csid_tmp.latin1());
                np->process_command( np->line_buf);
            } else {
                QMessageBox::warning( this, "Warning",
                        tr(" CSID %1 has improper length ").arg( csid_tmp.latin1() ),
                        QMessageBox::Warning,
                        0
                        );
                return;
            }
        }

        if (page[sector_idx]->gw_csc_cs_val->edited()) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -gw_csc_cs %d",
                    cell_idx, sector_idx, page[sector_idx]->gw_csc_cs_val->text().toInt());
            np->process_command( np->line_buf);
        }

        pm_idx = page[sector_idx]->prop_model_combobox->currentItem();
        if (pm_idx == np->num_prop_model) { pm_idx = -1; }

        if (pm_idx != np->cell_list[cell_idx]->sector_list[sector_idx]->prop_model) {
            sprintf(np->line_buf, "set_sector -sector %d_%d -prop_model %d", cell_idx, sector_idx, pm_idx);
            np->process_command( np->line_buf);
        }

        unused_freq_edited = 0;

        if (np->technology() == CConst::PHS && unused_freq_edited ) {
            if (sector->num_unused_freq != ((PHSSectorClass *) np->cell_list[cell_idx]->
                        sector_list[sector_idx])->num_unused_freq) {
                unused_freq_edited = 1;
            }
            for (i=0; (i<=sector->num_unused_freq-1)&&(!unused_freq_edited); i++) {
                if (sector->unused_freq[i] != ((PHSSectorClass *) np->cell_list[cell_idx]->
                            sector_list[sector_idx])->unused_freq[i]) {
                    unused_freq_edited = 1;
                }
            }
        }

        if (unused_freq_edited) {
            if (np->technology() == CConst::PHS && unused_freq_edited ) {
                chptr = np->line_buf;
                chptr += sprintf(chptr,"set_unused_freq -sector %d_%d -freq_list \'",cell_idx, sector_idx);

                for (i=0; i<=sector->num_unused_freq-1; i++) {
                    chptr += sprintf(chptr,"%d ",sector->unused_freq[i]);
                }
                chptr += sprintf(chptr,"\'");
                np->process_command(np->line_buf);
            }
        }
        if (np->technology() == CConst::WLAN)
        {
            std::cout << "To be impremented.\n";
            std::cout << "sector_idx " << cell_idx << "_" <<  sector_idx << std::endl;
            std::cout << "chan_idx   " << ((WLANSectorClass*) sector)->chan_idx << std::endl;

            sprintf(np->line_buf, "set_sector -sector %d_%d -chan_idx %d",
                cell_idx, sector_idx, ((WLANSectorClass*) sector)->chan_idx );
            np->process_command( np->line_buf);
        }
    }

    delete cell;
    delete this;
}
/******************************************************************************************/
/**** FUNCTION: sectorparamDia::cancel_button_clicked                                  ****/
/******************************************************************************************/
void sectorparamDia::cancel_button_clicked( )
{
    delete cell;
    delete this;
}

/******************************************************************************************/
/**** FUNCTION: SectorSpinBox                                                          ****/
/******************************************************************************************/
SectorSpinBox::SectorSpinBox(QWidget * parent, const char * name) : QSpinBox(parent, name)
{
}
/******************************************************************************************/
