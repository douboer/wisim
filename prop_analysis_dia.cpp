
/******************************************************************************************/
/**** PROGRAM: prop_analysis_dia.cpp                                                   ****/
/**** Implementation of Class PropAnalysisDia                                          ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/


#include <stdlib.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3VBoxLayout>
#include <iostream>

#include "phs.h"
#include "cconst.h"
#include "prop_analysis_dia.h"


PropAnalysisDia::PropAnalysisDia( NetworkClass* np_param, QWidget* parent)
    : QDialog( parent, 0, TRUE ), sector_number(0), np(np_param)
{
    int i;
    setName( "PropAnalysisDia" );
    setCaption( "Propagation Model Analysis" );

    int cell_idx        = 0;
    int sector_idx      = 0;
    CellClass *cell     = NULL;
    SectorClass *sector = NULL;

    /* Type of Prop Comp:
     *     PropSegment
     *     PropClutterSimp
     *     PropClutterFull
     *****************************************************************/

    prop_type = PropAnalysisDia::Seg;

    for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
            sector = cell->sector_list[sector_idx];
            sector_number = sector_number + 1;
        }
    }

    //allocate memory
    cell_sector_name         = ( char **)      malloc(sector_number*sizeof(char *));

    cell_sector_ckb_vec.clear();
    for( i=0; i<sector_number; i++) {
        cell_sector_name[i] = ( char *) malloc(10*sizeof( char ) );
    }

    prop_analysis_diaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "prop_analysis_diaLayout");

    /*
     ************************************
     *Scrview for sector groups
     ************************************
     */
    prop_analysis_scrollview = new Q3ScrollView( this, "prop_analysis_scrollview" );
    prop_analysis_scrollview->setPaletteBackgroundColor(backgroundColor());
    prop_analysis_scrollview->setVScrollBarMode(Q3ScrollView::Auto);
    prop_analysis_scrollview->setHScrollBarMode(Q3ScrollView::Auto);

    prop_analysis_frame = new Q3Frame( prop_analysis_scrollview->viewport(),"prop_analysis_frame");
    prop_analysis_frame->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0,
                                        prop_analysis_frame->sizePolicy().hasHeightForWidth() ) );
    prop_analysis_frameLayout = new Q3GridLayout( prop_analysis_frame, 1, 1, 11, 6, "prop_analysis_frameLayout");
    prop_analysis_scrollview->viewport()->resize(570,500);
    prop_analysis_frame->resize(570, 500);

    //sector for propagation analysis
    all_sector_checkbox = new QCheckBox( prop_analysis_frame, "all_sector_checkbox" );
    all_sector_checkbox->setMaximumWidth( 150 );
    all_sector_checkbox->setText( tr( "Select All Sectors" ) );
    prop_analysis_frameLayout->addWidget( all_sector_checkbox, 0, 0, Qt::AlignLeft );

    cell_sector_name_combobox = new QComboBox(prop_analysis_frame,"cell_sector_name_combobox");
    cell_sector_name_combobox->setFixedWidth( 220 );
    cell_sector_name_combobox->insertItem(tr("Cell Index"),0);
    cell_sector_name_combobox->insertItem(tr("GW_CSC_CS"),1);
    cell_sector_name_combobox->insertItem(tr("CSID"),2);
    prop_analysis_frameLayout->addWidget( cell_sector_name_combobox, 1, 0, Qt::AlignLeft );

    int sector_cnt = 0;
    for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];
        for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
            sector = cell->sector_list[sector_idx];

            QString namestr = QString("%1_%2").arg(cell_idx).arg(sector_idx);
            strcpy( cell_sector_name[sector_cnt], namestr); 
            QCheckBox* ckb = new QCheckBox( prop_analysis_frame, namestr );
            cell_sector_ckb_vec.push_back( ckb );
            ckb->setMaximumWidth( 120 );
            ckb->setText( namestr );
            prop_analysis_frameLayout->addWidget( ckb, sector_cnt+2, 0, Qt::AlignLeft );
            sector_cnt++;
        }
    }
    QSpacerItem* spacer1 = new QSpacerItem( 210, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    prop_analysis_frameLayout->addItem( spacer1, sector_cnt+3, 2 );

    prop_analysis_scrollview->addChild(prop_analysis_frame);
    prop_analysis_diaLayout->addMultiCellWidget( prop_analysis_scrollview, 0, 0, 0, 1 );

    /*
     * Select propagation model to simulaiton
     *****************************************************************************/
    prop_type_buttonGroup = new Q3ButtonGroup( this, "prop_type_buttonGroup" );
    prop_type_buttonGroup->setGeometry( QRect( 40, 20, 180, 72 ) );
    prop_type_buttonGroup->setColumnLayout(0, Qt::Vertical );
    prop_type_buttonGroup->layout()->setSpacing( 6 );
    prop_type_buttonGroup->layout()->setMargin( 11 );
    prop_type_buttonGroupLayout = new Q3VBoxLayout( prop_type_buttonGroup->layout() );
    prop_type_buttonGroupLayout->setAlignment( Qt::AlignTop );

    seg_type_radioButton = new QRadioButton( prop_type_buttonGroup, "seg_type_radioButton" );
    prop_type_buttonGroupLayout->addWidget( seg_type_radioButton );
    seg_type_radioButton->setChecked( TRUE );

    clt_type_radioButton = new QRadioButton( prop_type_buttonGroup, "clt_type_radioButton" );
    prop_type_buttonGroupLayout->addWidget( clt_type_radioButton );

    prop_type_buttonGroup->setTitle( tr( "Select Prop Type" ) );
    seg_type_radioButton->setText( tr( "Segment" ) );
    clt_type_radioButton->setText( tr( "Clutter" ) );

    prop_analysis_diaLayout->addMultiCellWidget( prop_type_buttonGroup, 1, 1, 0, 1, Qt::AlignTop );

   /*
    * Parameters of slope k and intercept b needed by Simple typle 
    * of clutter propagation model computation.
    *****************************************************************************/
    clutter_param_group = new Q3ButtonGroup(this);
    clutter_param_group->setColumnLayout(0, Qt::Vertical );
    clutter_param_group->layout()->setSpacing( 6 );
    clutter_param_group->layout()->setMargin( 11 );
    clutter_param_group->setTitle( tr( "Clutter Parameters" ) );
    clutter_param_groupLayout = new Q3GridLayout( clutter_param_group->layout() );
    clutter_param_groupLayout->setAlignment( Qt::AlignTop );

    init_clt_res_lbl = new QLabel(clutter_param_group);
    init_clt_res_lbl->setText( tr( "Initial clutter to simulation resolution ratio" ) );
    clutter_param_groupLayout->addWidget( init_clt_res_lbl, 0, 0 );
 
    init_clt_res_val = new QComboBox( clutter_param_group, "init_clt_res_val" );
    for (i=0; i<=10; i++) {
        init_clt_res_val->insertItem( QString("%1").arg(1<<i), i );
    }
    init_clt_res_val->setCurrentItem(7);
    // init_clt_res_val->setMaximumSize( QSize( 200, 32767 ) );
    // init_clt_res_val->setMaxValue( 2000 );
    // init_clt_res_val->setValue( 100 );
    clutter_param_groupLayout->addWidget( init_clt_res_val, 0, 1 );

    num_clt_split_lbl = new QLabel(clutter_param_group);
    num_clt_split_lbl->setMinimumSize( QSize( 150, 0 ) );
    num_clt_split_lbl->setText( tr( "Number of times to split clutter" ) );
    clutter_param_groupLayout->addWidget( num_clt_split_lbl, 1, 0 );

    num_clt_split_val = new QSpinBox(clutter_param_group);
    num_clt_split_val->setMaximumSize( QSize( 200, 32767 ) );
    clutter_param_groupLayout->addWidget( num_clt_split_val, 1, 1 );
    num_clt_split_val->setMinValue(0);
    num_clt_split_val->setValue(2);

    clt_res_lbl = new QLabel(clutter_param_group);
    clt_res_lbl->setText( tr( "Clutter to simulation resolution ratio" ) );
    clutter_param_groupLayout->addWidget( clt_res_lbl, 2, 0 );

    clt_res_val = new QLabel(clutter_param_group);
    clutter_param_groupLayout->addWidget( clt_res_val, 2, 1 );

    prop_analysis_diaLayout->addMultiCellWidget( clutter_param_group, 2, 2, 0, 1, Qt::AlignTop );

    /*
     ********************************************
     *scope option for propagation model analysis
     ********************************************
     */
    scope_option_btnGroup = new Q3ButtonGroup( tr("Scope Option") , this, "scope_option_btnGroup");
    scope_option_btnGroup->setColumnLayout(0, Qt::Vertical );
    scope_option_btnGroup->layout()->setSpacing( 6 );
    scope_option_btnGroup->layout()->setMargin( 11 );
    Q3GridLayout *scope_option_btnGroupLayout = new Q3GridLayout( scope_option_btnGroup->layout() );
    scope_option_btnGroupLayout->setAlignment( Qt::AlignTop );

    //global scope propagation analysis
    global_scope_radiobtn = new QRadioButton( scope_option_btnGroup, "global_scope_radiobtn" );
    scope_option_btnGroup->insert( global_scope_radiobtn, 0 );
    global_scope_radiobtn->setText( tr("Global Scope Analysis") );
    //global_scope_radiobtn->setChecked( TRUE );
    global_scope_radiobtn->setMaximumSize( QSize( 300, 32767 ) );
    scope_option_btnGroupLayout->addWidget( global_scope_radiobtn, 0, 0 );

    //individual scope propagation analysis
    individual_scope_radiobtn = new QRadioButton( scope_option_btnGroup, "individual_scope_radiobtn" );
    scope_option_btnGroup->insert( individual_scope_radiobtn, 1);
    individual_scope_radiobtn->setText( tr( "Individual Scope Analysis" ) );
    individual_scope_radiobtn->setChecked( TRUE );
    individual_scope_radiobtn->setMaximumSize( QSize( 300, 32767 ) );
    scope_option_btnGroupLayout->addWidget( individual_scope_radiobtn, 1, 0 );

    prop_analysis_diaLayout->addWidget( scope_option_btnGroup, 3, 0, Qt::AlignTop );

    /*
     ********************************************
     *other option for propagation model analysis
     ********************************************
     */
    other_option_btnGroup = new Q3ButtonGroup( tr( "Other Option" ), this, "other_option_btnGroup");
    other_option_btnGroup->setColumnLayout(0, Qt::Vertical );
    other_option_btnGroup->layout()->setSpacing( 6 );
    other_option_btnGroup->layout()->setMargin( 11 );
    Q3GridLayout *other_option_btnGroupLayout = new Q3GridLayout( other_option_btnGroup->layout() );
    scope_option_btnGroupLayout->setAlignment( Qt::AlignTop );

    //use height for propagation analysis
    useheight_checkbox = new QCheckBox( other_option_btnGroup, "useheight_checkbox" );
    other_option_btnGroup->insert( useheight_checkbox, 0 );
    useheight_checkbox->setChecked( TRUE );
    useheight_checkbox->setText( tr( "Use Height For Analysis" ) );
    useheight_checkbox->setMaximumSize( QSize( 300, 32767 ) );
    other_option_btnGroupLayout->addWidget( useheight_checkbox, 0, 0 );

    //adjust antenna angle for propagation analysis
    adjust_antenna_checkbox = new QCheckBox( other_option_btnGroup, "adjust_antenna_checkbox" );
    other_option_btnGroup->insert( adjust_antenna_checkbox, 1);
    adjust_antenna_checkbox->setChecked( TRUE );
    adjust_antenna_checkbox->setText( tr ("Adjust Antenna Angles") );
    adjust_antenna_checkbox->setMaximumSize( QSize( 300, 32767 ) );
    other_option_btnGroupLayout->addWidget( adjust_antenna_checkbox, 1, 0 );

    prop_analysis_diaLayout->addWidget( other_option_btnGroup, 3, 1, Qt::AlignTop );



    /*
     ********************************************
     *Ok and cancel pushbutton
     ********************************************
     */
    ok_cancel_hboxLayout = new Q3HBoxLayout( 0, 11, 6, "ok_cancel_hboxLayout");
    ok_btn = new QPushButton( tr("&Ok"), this, "ok_btn" );
    ok_btn->setMaximumWidth( 80 );
    ok_cancel_hboxLayout->addWidget( ok_btn, 0, Qt::AlignHCenter );

    cancel_btn = new QPushButton( tr("&Cancel"), this, "cancel_btn" );
    cancel_btn->setMaximumWidth( 80 );
    ok_cancel_hboxLayout->addWidget( cancel_btn, 0, Qt::AlignHCenter );
    prop_analysis_diaLayout->addMultiCellLayout( ok_cancel_hboxLayout, 4, 4, 0, 1 );


    init_clt_res_changed(init_clt_res_val->currentItem());
    prop_type_selected(prop_type_buttonGroup->selectedId());

    global = 0;

    //connect signal and slot
    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( cell_sector_name_combobox, SIGNAL( activated( int ) ),this, SLOT( cell_sector_name_combobox_changed(int) ) );
    connect( all_sector_checkbox, SIGNAL( stateChanged(int) ), this, SLOT( all_sector_selected(int) ) );
    connect( init_clt_res_val,      SIGNAL( activated(int) ), this, SLOT( init_clt_res_changed(int) ) );
    connect( num_clt_split_val,     SIGNAL( valueChanged(int) ), this, SLOT( num_clt_split_changed(int) ) );
    connect( prop_type_buttonGroup, SIGNAL( clicked(int) ), this, SLOT( prop_type_selected(int) ) );
    connect( scope_option_btnGroup, SIGNAL( clicked(int) ), this, SLOT( scope_option_selected(int) ) );

    resize( 600, 500 );

    exec();
}

PropAnalysisDia::~PropAnalysisDia()
{
    for( int idx=0; idx<sector_number; idx++ ) {
        free(cell_sector_name[idx]);
    }
    free(cell_sector_name);

    cell_sector_ckb_vec.clear();
}

void PropAnalysisDia::init_clt_res_changed(int i)
{
    num_clt_split_val->setMaxValue(i);
    clt_res_val->setText(QString("%1").arg(1<<(init_clt_res_val->currentItem() - num_clt_split_val->value())) );
}

void PropAnalysisDia::num_clt_split_changed(int)
{
    clt_res_val->setText(QString("%1").arg(1<<(init_clt_res_val->currentItem() - num_clt_split_val->value())) );
}

void PropAnalysisDia::prop_type_selected(int i)
{
    switch (i) {
        case 0: 
            prop_type = PropAnalysisDia::Seg;

            clutter_param_group->setEnabled(false);
            scope_option_btnGroup->setEnabled(true);
            other_option_btnGroup->setEnabled(true);
            break;
        case 1:
            prop_type = PropAnalysisDia::Clt;

            clutter_param_group->setEnabled(true);
            scope_option_btnGroup->setEnabled(false);
            other_option_btnGroup->setEnabled(false);
            break;
    }
}


void PropAnalysisDia::scope_option_selected(int i)
{
    switch (i) {
        case 0: 
            global = 1;
            break;
        case 1:
            global = 0; 
            break;
    }
}


void PropAnalysisDia::ok_btn_clicked()
{
    QString sectorstr = NULL;

    if ( useheight_checkbox->isChecked() ) {
        useheight = 1;
    } else {
        useheight = 0;
    }

    if ( adjust_antenna_checkbox->isChecked() ) {
        adjust_angles = 1;
    } else {
        adjust_angles = 0;
    }


    if ( all_sector_checkbox->isChecked() ) {
        sectorstr = QString("all");
    } else {
        for( int i=0; i<sector_number; i++ ) {
            if ( cell_sector_ckb_vec[i]->isChecked() ) {
                sectorstr += cell_sector_name[i];
                sectorstr += " ";
            }
        }

        sectorstr.truncate( sectorstr.length() - 1 );
        sectorstr.insert( 0, "'");
        sectorstr.insert( sectorstr.length(), "'");
    }

    if ( !sectorstr.isNull() || !sectorstr.isEmpty() ) {
        if ( prop_type == Seg )
        {
            if ( global == 1 ) {
                sprintf(np->line_buf, "comp_prop_model -scope global -sectors %s -useheight %d -adjust_angles %d -min_dist 15.0", 
                        sectorstr.latin1(), useheight, adjust_angles );
                np->process_command(np->line_buf);
            } else if ( global == 0 ) {
                sprintf(np->line_buf, "comp_prop_model -scope individual -sectors %s -useheight %d -adjust_angles %d -min_dist 15.0", 
                        sectorstr.latin1(), useheight, adjust_angles );
                np->process_command(np->line_buf);
            }
        } else if ( prop_type == Clt ) {
            sprintf(np->line_buf, "gen_clutter -scope global -sectors %s -useheight %d -type expo_linear -map_sim_res_ratio %d", 
                    sectorstr.latin1(), 0 /* useheight */, 1<<init_clt_res_val->currentItem() );
            np->process_command(np->line_buf);

            sprintf(np->line_buf, "refine_clutter_model -n %d -pm_idx %d", 
                    num_clt_split_val->value(), np->num_prop_model-1);
            np->process_command(np->line_buf);
        }
    }

    delete this;
}

void PropAnalysisDia::cancel_btn_clicked()
{
    delete this;
}

void PropAnalysisDia::all_sector_selected(int)
{
    if ( all_sector_checkbox->isChecked() ) {
        for ( int i=0; i<sector_number; i++ ) {
            cell_sector_ckb_vec[i]->setChecked( TRUE );
        }
    } else {
        for ( int i=0; i<sector_number; i++ ) {
            cell_sector_ckb_vec[i]->setChecked( FALSE );
        }
    }
}

void PropAnalysisDia::cell_sector_name_combobox_changed( int i )
{
    int         sector_cnt = 0;
    int         cell_idx   = 0;
    int         sector_idx = 0;
    CellClass   *cell      = NULL;         //pointer of cell struct
    SectorClass *sector    = NULL;         //pointer of sector struct
    char        namestr[50]= "";

    switch ( i ) {
        case 0:
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = cell->sector_list[sector_idx];
                    sprintf( namestr, "%d_%d", cell_idx, sector_idx );
                    cell_sector_ckb_vec[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            break;
        case 1:
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = cell->sector_list[sector_idx];
                    sprintf( namestr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs );
                    cell_sector_ckb_vec[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            break;
        case 2:
            char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
            for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                    sector = cell->sector_list[sector_idx];
                    if ( np->technology() == CConst::PHS )
                        hex_to_hexstr(hexstr,((PHSSectorClass *) sector)->csid_hex,PHSSectorClass::csid_byte_length);
                    sprintf( namestr, "%s", hexstr );
                    cell_sector_ckb_vec[sector_cnt]->setText( namestr );
                    sector_cnt++;
                }
            }
            free(hexstr);
            break;
    }
}


void PropAnalysisDia::resizeEvent( QResizeEvent* )
{
    prop_analysis_frame->setMinimumSize(prop_analysis_scrollview->viewport()->width(), height());
}

