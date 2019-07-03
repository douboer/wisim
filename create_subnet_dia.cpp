/******************************************************************************************/
/**** PROGRAM: create_subnet_dia.cpp                                                   ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <q3buttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <q3scrollview.h>
#include <qvariant.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3Frame>

#include "WiSim.h"
#include "create_subnet_dia.h"
#include "list.h"
#include "map_layer.h"
#include "strint.h"

extern QFont *fixed_width_font;

CreSubnetDia::CreSubnetDia(NetworkClass* np_param, QWidget* parent)
    : QDialog( parent, 0, true)
{
    np = np_param;    
    setName( "Create Subnets" );

    cre_subnet_diaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "cre_subnet_diaLayout"); 

    map_layer_chkbox = NULL;
    map_layer_chkbox = ( QCheckBox ** ) malloc( np->map_layer_list->getSize()*sizeof(QCheckBox * ));

    //map layer select
    map_layer_btngroup = new Q3ButtonGroup( this, "map_layer_btngroup" );
    map_layer_btngroup->setColumnLayout(0, Qt::Vertical );
    map_layer_btngroup->layout()->setSpacing( 6 );
    map_layer_btngroup->layout()->setMargin( 11 );
    map_layer_btngroupLayout = new Q3GridLayout( map_layer_btngroup->layout() );
    map_layer_btngroupLayout->setAlignment( Qt::AlignTop );

    if ( np->map_layer_list->getSize() > 0 ) {
        for( int ml_idx=0; ml_idx<np->map_layer_list->getSize(); ml_idx++ ) {
            QString mlNamestr;
            mlNamestr = (*(np->map_layer_list))[ml_idx]->name;
            map_layer_chkbox[ml_idx] = new QCheckBox( map_layer_btngroup, "map_layer_chkbox" );
            map_layer_chkbox[ml_idx]->setText(mlNamestr);
            map_layer_btngroupLayout->addWidget( map_layer_chkbox[ml_idx], ml_idx, 0, Qt::AlignLeft );
        }
    } else {
        QLabel *no_map_layer_lbl = new QLabel( tr("Current design has no map layers defined"), map_layer_btngroup, "no_map_layer_lbl" );
        map_layer_btngroupLayout->addWidget( no_map_layer_lbl, 0, 0 );
    }

    cre_subnet_diaLayout->addMultiCellWidget( map_layer_btngroup, 0, 0, 0, 3 );

    //advanced, group sector, ok and cancel push button
    adv_btn = new QPushButton( this, "adv_bnt" );
    adv_btn->setMaximumWidth( 100 );
    cre_subnet_diaLayout->addWidget( adv_btn, 1, 0 );

    group_sector_btn = new QPushButton( this, "group_sector_btn" );
    group_sector_btn->setMaximumWidth( 100 );
    cre_subnet_diaLayout->addWidget( group_sector_btn, 1, 1 );

    ok_btn = new QPushButton( this, "ok_btn" );
    ok_btn->setMaximumWidth( 80 );
    cre_subnet_diaLayout->addWidget( ok_btn, 1, 2 );

    cancel_btn = new QPushButton( this, "cancel_btn" );
    cancel_btn->setMaximumWidth( 80 );
    cre_subnet_diaLayout->addWidget( cancel_btn, 1, 3 );

    languageChange();
    resize( QSize(400, 200).expandedTo(minimumSizeHint()) );

    //signals and slots connections
    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( adv_btn, SIGNAL( clicked() ), this, SLOT( adv_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( group_sector_btn, SIGNAL( clicked() ), this, SLOT( group_sector_btn_clicked() ) );

    //tab order
    ok_btn->setFocus();
    setTabOrder( map_layer_btngroup, adv_btn );
    setTabOrder( adv_btn, group_sector_btn );
    setTabOrder( group_sector_btn, ok_btn );
    setTabOrder( ok_btn, cancel_btn );
    setTabOrder( cancel_btn, map_layer_btngroup );

    //setup the extention of the Advanced widget
    advancedShown = FALSE;
    setExtension( new AdvCreSubnetDia( np, this) );
    setOrientation( Qt::Vertical );
    advancedDialog = ( AdvCreSubnetDia * ) extension();

    exec();  
}

/*
 *  Destroys the object and frees any allocated resources
 */
CreSubnetDia::~CreSubnetDia()
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CreSubnetDia::languageChange()
{
    setCaption( tr( "Create Subnet" ) );
    adv_btn->setText( tr( "&Advanced" ) );
    group_sector_btn->setText( tr( "&Group Sector" ) );
    ok_btn->setText( tr( "&Ok" ) );
    cancel_btn->setText( tr( "&Cancel" ) );
    map_layer_btngroup->setTitle( tr( "Map Layers" ) );
}

void CreSubnetDia::ok_btn_clicked()
{
    QString str1 = advancedDialog->scan_fractional_area_val->text().latin1();
    QString str2 = advancedDialog->init_sample_res_val->text().latin1();
    QString str3 = "";

    for( int ml_idx=0; ml_idx<np->map_layer_list->getSize(); ml_idx++ ) {
        if( map_layer_chkbox[ml_idx]->isChecked() ) {
            str3 += QString("m%1").arg(ml_idx);
        }
    }

    if ( str3 == "" ) {
        sprintf(np->line_buf, "create_subnets -scan_fractional_area %s -init_sample_res %s", str1.latin1(), str2.latin1());
        np->process_command(np->line_buf);
    } else {
        sprintf(np->line_buf, "create_subnets -scan_fractional_area %s -init_sample_res %s -exclude_ml %s",
                str1.latin1(), str2.latin1(), str3.latin1());
        np->process_command(np->line_buf);
    }

    delete this;
}


void CreSubnetDia::cancel_btn_clicked()
{
    delete this;
}

void CreSubnetDia::adv_btn_clicked()
{
    advancedShown = !advancedShown;
    showExtension( advancedShown );
    if ( !advancedShown ){
        adv_btn->setText( tr("&Advanced") );
    } else {
        adv_btn->setText( tr("&Advanced") + " >>>" );
    }
}

void CreSubnetDia::group_sector_btn_clicked()
{
    sectorgroup_dia = new SectorgroupDia(np,0);

}

/*
 ****************************************************************
 * Advanced Create Subnet Class
 ****************************************************************
 */
AdvCreSubnetDia::AdvCreSubnetDia(NetworkClass* np_param, QWidget* parent) : QWidget( parent, 0 )
{
    setName( "AdvCreSubnet" );
    np = np_param;

    adv_cre_subnet_diaLayout = new Q3GridLayout( this, 2, 2, 11, 6, "adv_cre_subnet_diaLayout" );

    //fractional area label and lineedit
    scan_fractional_area_lbl = new QLabel( tr("Scan Fractional Area"), this, "scan_fractional_area_lbl");
    scan_fractional_area_lbl->setMaximumWidth( 150 );
    adv_cre_subnet_diaLayout->addWidget( scan_fractional_area_lbl, 0, 0, Qt::AlignHCenter );

    scan_fractional_area_val = new QLineEdit(this, "scan_fractional_area_val");
    scan_fractional_area_val->setMaximumSize( QSize( 120, 32767 ) );
    scan_fractional_area_lbl->setBuddy( scan_fractional_area_val );
    adv_cre_subnet_diaLayout->addWidget( scan_fractional_area_val, 0, 1, Qt::AlignHCenter );

    //initial sample resolution label and lineedit
    init_sample_res_lbl = new QLabel( tr("Initial Sample Resolution"), this, "init_sample_res_lbl");
    init_sample_res_lbl->setMaximumWidth( 150 );
    adv_cre_subnet_diaLayout->addWidget( init_sample_res_lbl, 1, 0, Qt::AlignHCenter );

    init_sample_res_val = new QLineEdit(this, "init_sample_res_val");
    init_sample_res_val->setMaximumSize( QSize( 120, 32767 ) );
    init_sample_res_lbl->setBuddy( init_sample_res_val );
    adv_cre_subnet_diaLayout->addWidget( init_sample_res_val, 1, 1, Qt::AlignHCenter );

    //set initial value
    scan_fractional_area_val->setText("0.995");
    init_sample_res_val->setText("16");
}

AdvCreSubnetDia::~AdvCreSubnetDia()
{
}


/******************************************************************************************/
/****                          FUNCTION:  SectorgroupDia                               ****/
/******************************************************************************************/
SectorgroupDia::SectorgroupDia( NetworkClass *np_param, QWidget* parent)
               : QDialog(parent, 0, true)
{

    std::cout << "SectorgroupDia::SectorgroupDia ... \n";

    createflg = false;

    int         i          = 0;
    int         j          = 0;
    int         k          = 0;
    int         m          = 0;
    int         cell_idx   = 0;
    int         sector_idx = 0;
    int         cell_name_idx;
    CellClass   *cell      = NULL;  //pointer of cell struct
    SectorClass *sector    = NULL;  //pointer of sector struct
    ListClass<int> *lc;

    setName( "SectorgroupDia" );
    setCaption(tr("Sector Group"));
    np = np_param;
    newgroup_number = 0;

    sector_number = 0;
    for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
            sector = cell->sector_list[sector_idx];
            sector_number = sector_number + 1;
        }
    }

    //allocate memory
    cell_sector_name         = NULL;
    cell_sector_name_lbl     = NULL;
    sector_group_name_lbl    = NULL;
    cell_sector_checkbox     = NULL;
    cell_sector_name         = ( char      **) malloc(sector_number*sizeof(char *));
    cell_sector_name_lbl     = ( QLabel    **) malloc(sector_number*sizeof(QLabel *));
    sector_group_name_lbl    = ( QLabel    **) malloc(INIT_GRP_NUM*sizeof(QLabel *));
    cell_sector_checkbox     = ( QCheckBox **) malloc(INIT_GRP_NUM*sector_number*sizeof(QCheckBox *));

    for( i=0; i<sector_number; i++) {
        cell_sector_name[i] = ( char *) malloc(10*sizeof( char ) );
    }

    //the gridlayout of the dialog
    sector_group_diaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "sector_group_diaLayout");

    //Scrview for sector groups
    sector_group_scrollview = new Q3ScrollView( this, "sector_group_scrollview" );
    sector_group_scrollview->setPaletteBackgroundColor(backgroundColor());
    sector_group_scrollview->setVScrollBarMode(Q3ScrollView::Auto);
    sector_group_scrollview->setHScrollBarMode(Q3ScrollView::Auto);

    sector_group_frame = new Q3Frame( sector_group_scrollview->viewport(),"sector_group_frame");
    sector_group_frame->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0,
                                       sector_group_frame->sizePolicy().hasHeightForWidth() ) );
    sector_group_frameLayout = new Q3GridLayout( sector_group_frame, 1, 1, 11, 6, "sector_group_frameLayout");
    sector_group_scrollview->viewport()->resize(400,500);
    sector_group_frame->resize(400, 500);

    //first column of the scrollview, they are name of the cells and sectors
    cell_sector_name_combobox = new QComboBox(sector_group_frame,"cell_sector_name_combobox");
    cell_sector_name_combobox->setMaximumWidth( 120 );

    for (cell_name_idx=0; cell_name_idx<=np->report_cell_name_opt_list->getSize()-1; cell_name_idx++) {
        cell_sector_name_combobox->insertItem((*(np->report_cell_name_opt_list))[cell_name_idx].getStr(), cell_name_idx);
    }

    // xxxxx cell_sector_name_combobox->insertItem(tr("Cell Index"),0);
    // xxxxx cell_sector_name_combobox->insertItem(tr("GW_CSC_CS"),1);
    // xxxxx cell_sector_name_combobox->insertItem(tr("CSID"),2);

    sector_group_frameLayout->addWidget( cell_sector_name_combobox, 0, 0, Qt::AlignCenter );

    int sector_cnt = 0;
    for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
            sector = cell->sector_list[sector_idx];

            QString namestr;
            namestr = QString("%1_%2").arg(cell_idx).arg(sector_idx);
            strcpy(cell_sector_name[sector_cnt],namestr);

            cell_sector_name_lbl[sector_cnt] = new QLabel( sector_group_frame );
            cell_sector_name_lbl[sector_cnt]->setText( namestr );
            cell_sector_name_lbl[sector_cnt]->setMaximumWidth( 120 );
            cell_sector_name_lbl[sector_cnt]->setTextFormat(Qt::PlainText);
            cell_sector_name_lbl[sector_cnt]->setFont(*fixed_width_font);

            sector_group_frameLayout->addWidget( cell_sector_name_lbl[sector_cnt],sector_cnt+1,0,Qt::AlignCenter );
            sector_cnt++;
        }
    }

    //seconde column of the scrollview, they are sector group
    for( i=0; i<INIT_GRP_NUM; i++) {
        QString namestr = QString("                           ");
        sector_group_name_lbl[i] = new QLabel( sector_group_frame, namestr);
        sector_group_name_lbl[i]->setText(tr(namestr));
        sector_group_name_lbl[i]->setMaximumWidth( 120 );
        sector_group_name_lbl[i]->setTextFormat(Qt::PlainText);
        sector_group_frameLayout->addWidget(sector_group_name_lbl[i], 0, i+1, Qt::AlignCenter );
    }

    //check box for each sector used for group
    sector_group_btnGroup = new Q3ButtonGroup(sector_number, Qt::Vertical, sector_group_frame, "sector_group_btnGroup");
    sector_group_btnGroup->hide();

    for( i=0; i<INIT_GRP_NUM; i++ ) {
        for( j=0; j<sector_number; j++ ) {
            cell_sector_checkbox[i*sector_number+j] = new QCheckBox( sector_group_frame, "cell_sector_checkbox" );
            cell_sector_checkbox[i*sector_number+j]->setMaximumWidth( 120 );
            sector_group_btnGroup->insert( cell_sector_checkbox[i*sector_number+j], i*sector_number+j );
            sector_group_frameLayout->addWidget(cell_sector_checkbox[i*sector_number+j], j+1, i+1, Qt::AlignCenter );
        }
    }
    sector_group_frameLayout->addMultiCellWidget( sector_group_btnGroup, 1, sector_number, 1,
                                                  INIT_GRP_NUM, Qt::AlignCenter );
    sector_group_scrollview->addChild(sector_group_frame);
    sector_group_diaLayout->addWidget( sector_group_scrollview, 0, 0);

    //read the sector group information from np to this dialog
    for ( i=0; i<=np->sector_group_list->getSize()-1; i++ ) {
        lc = (ListClass<int> *) (*(np->sector_group_list))[i];
        for ( j=0; j<=lc->getSize()-1; j++ ) {
            cell_idx   = (*lc)[j] & ((1<<np->bit_cell)-1);
            sector_idx = (*lc)[j] >> np->bit_cell;
            QString namestr = QString("%1_%2").arg(cell_idx).arg(sector_idx);
            for( k=0; k<sector_number; k++ ) {
                if ( strcmp(cell_sector_name[k],namestr)==0 ) {
                    cell_sector_checkbox[i*sector_number+k]->setChecked( true );
                    for( m=0; m<INIT_GRP_NUM; m++ ) {
                        if( m!= i ) {
                            cell_sector_checkbox[m*sector_number+k]->setEnabled(false);
                        }
                    }
                }
            }
        }
    }

    //if the responding sector group not defined, then hide them
    for ( i=np->sector_group_list->getSize(); i<INIT_GRP_NUM; i++ ) {
        sector_group_name_lbl[i]->hide();
        for ( j=0; j<sector_number; j++ ) {
            cell_sector_checkbox[i*sector_number+j]->hide();
        }
    }
 
    //if no sector group defined at all, then give the warning
    if ( np->sector_group_list->getSize()==0 ) {
        sector_group_name_lbl[0]->show();
        sector_group_name_lbl[0]->setText( tr("No sector group defined") );
        sector_group_name_lbl[0]->setMaximumWidth( 400 );
    }

    //Ok and Cancel button layout
    create_ok_cancel_hboxLayout = new Q3HBoxLayout( 0, 11, 6, "create_ok_cancel_hboxLayout");
    create_btn = new QPushButton( tr("C&reate"), this, "cancel_btn" );
    create_btn->setMaximumWidth( 80 );
    create_ok_cancel_hboxLayout->addWidget( create_btn, 0, Qt::AlignHCenter );

    ok_btn = new QPushButton( tr("&Ok"), this, "ok_btn" );
    ok_btn->setMaximumWidth( 80 );
    create_ok_cancel_hboxLayout->addWidget( ok_btn, 0, Qt::AlignHCenter );

    cancel_btn = new QPushButton( tr("&Cancel"), this, "cancel_btn" );
    cancel_btn->setMaximumWidth( 80 );
    create_ok_cancel_hboxLayout->addWidget( cancel_btn, 0, Qt::AlignHCenter );
    sector_group_diaLayout->addLayout( create_ok_cancel_hboxLayout, 1, 0 );

    // show first list of group
    // create_btn_clicked();
    if ( newgroup_number > 0 )
        ok_btn->setDisabled(false);
    else
        ok_btn->setDisabled(true);

    //signal and slot connection
    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( create_btn, SIGNAL( clicked() ), this, SLOT( create_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( cell_sector_name_combobox, SIGNAL( activated( int ) ),this, SLOT( cell_sector_name_combobox_changed(int) ) );
    connect( sector_group_btnGroup, SIGNAL( clicked(int) ), this, SLOT( sector_group_clicked(int) ) );

    resize( 400, 500 );

    exec();
}

SectorgroupDia::~SectorgroupDia()
{
    for( int idx=0; idx<sector_number; idx++ ) {
        free(cell_sector_name[idx]);
    }
    free(cell_sector_name);
}

void SectorgroupDia::create_btn_clicked()
{
    std::cout << "SectorgroupDia::create_btn_clicked() .... \n";

    int i = 0;
    int j = 0;

    if( newgroup_number < INIT_GRP_NUM ) {
        newgroup_number = newgroup_number + 1;
        QString namestr = QString("Group %1").arg(np->sector_group_list->getSize()+newgroup_number-1);
        sector_group_name_lbl[np->sector_group_list->getSize()+newgroup_number-1]->setText(tr(namestr));
        sector_group_name_lbl[np->sector_group_list->getSize()+newgroup_number-1]->show();
        for ( i=0; i<sector_number; i++ ) {
            cell_sector_checkbox[(np->sector_group_list->getSize()+newgroup_number-1)*sector_number+i]->show();
        }

        //if the cell sector have been grouped then disabled them
        for ( i=0; i<np->sector_group_list->getSize()+newgroup_number-1; i++ ) {
            for ( j=0; j<sector_number; j++ ) {
                 if ( cell_sector_checkbox[i*sector_number+j]->isChecked() ) {
                      cell_sector_checkbox[(np->sector_group_list->getSize()+newgroup_number-1)*sector_number+j]->setEnabled( false );
                 }
            }
        }
    }

    if ( newgroup_number > 0 )
        ok_btn->setDisabled(false);
    else
        ok_btn->setDisabled(true);
}

void SectorgroupDia::ok_btn_clicked()
{
    int grp_idx = 0;
    int sct_idx = 0;
    int grp_number = 0;
    grp_number = np->sector_group_list->getSize();

    for ( grp_idx=grp_number; grp_idx<grp_number+newgroup_number; grp_idx++ ) {
        QString grp_str;
        for( sct_idx=0; sct_idx<sector_number; sct_idx++ ) {
            if ( cell_sector_checkbox[grp_idx*sector_number+sct_idx]->isChecked() ) { 
                grp_str += cell_sector_name[sct_idx];
                grp_str += " ";
            }
        }

        if ( !grp_str.isNull() || !grp_str.isEmpty() ) {
            grp_str.truncate( grp_str.length() -1 );

            sprintf(np->line_buf, "group -sectors '%s'", grp_str.latin1());
            np->process_command(np->line_buf);
        }
    }
    delete this;
}

void SectorgroupDia::cancel_btn_clicked()
{
    delete this;
}

void SectorgroupDia::sector_group_clicked(int i)
{
    int idx     = 0;
    int sct_idx = 0;
    int grp_idx = 0;

    grp_idx = i/sector_number;
    sct_idx = i - grp_idx*sector_number;

    if( cell_sector_checkbox[i]->isChecked() ) {
        for( idx=0; idx<np->sector_group_list->getSize()+newgroup_number; idx++ ) {
            if( idx!= grp_idx ) {
                cell_sector_checkbox[idx*sector_number+sct_idx]->setEnabled(false);
            }
        } 
    } else {
        for( idx=0; idx<np->sector_group_list->getSize()+newgroup_number; idx++ ) {
            if( idx!= grp_idx ) {
                cell_sector_checkbox[idx*sector_number+sct_idx]->setEnabled(true);
            }
        } 
    }
}

void SectorgroupDia::cell_sector_name_combobox_changed(int i)
{
    int         sector_cnt = 0;
    int         cell_idx   = 0;
    CellClass   *cell      = NULL;         //pointer of cell struct

    int name_pref = (*(np->report_cell_name_opt_list))[i].getInt();
    for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        cell_sector_name_lbl[sector_cnt]->setText( cell->view_name(cell_idx, name_pref));
        sector_cnt++;
    }

#if 0
xxxxxxxx
    switch ( i ) {
        case 0:
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    sprintf( namestr, "%d_%d", cell_idx, sector_idx );
                    cell_sector_name_lbl[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            break;
        case 1:
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    sprintf( namestr, "%.6d", sector->gw_csc_cs );
                    cell_sector_name_lbl[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            break;
        case 2:
            char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                    hex_to_hexstr(hexstr,sector->csid_hex,sector->csid_byte_length);
                    sprintf( namestr, "%s", hexstr );
                    cell_sector_name_lbl[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            free(hexstr);
            break;
    }
#endif
}

void SectorgroupDia::resizeEvent( QResizeEvent* )
{
    sector_group_frame->setMinimumSize(sector_group_scrollview->viewport()->width(), height());
}

