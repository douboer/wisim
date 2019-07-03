
/******************************************************************************************
**** PROGRAM: report_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3cstring.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>

#include "cconst.h"
#include "WiSim.h"
#include "coverage.h"
#include "filechooser.h"
#include "gconst.h"
#include "list.h"
#include "map_layer.h"
#include "num_sector_apply.h"
#include "prop_model.h"
#include "report_dia.h"
#include "traffic_type.h"

ReportDialog::ReportDialog(NetworkClass*np_param, int type_param, QWidget* parent)
    : QDialog( parent, 0, true)
{

    int i, idx;
    np = np_param;
    type = type_param;
    QString filt;

    layout = new Q3GridLayout( this, 1, 1, 11, 15, "layout"); 

    if (type != GConst::coveragePlot) {
        button_grp = new Q3ButtonGroup( this, "button_grp" );
        button_grp->setColumnLayout(0, Qt::Vertical );
        button_grp->layout()->setSpacing( 15 );
        button_grp->layout()->setMargin( 11 );
        button_grp->setTitle( tr( "Output" ) );
        button_grp_layout = new Q3GridLayout( button_grp->layout() );
        button_grp_layout->setAlignment( Qt::AlignTop );

        file_radio_btn = new QRadioButton( tr("File"), button_grp, "file_radio_btn" );

        cmdline_radio_btn = new QRadioButton( tr("Command Window"), button_grp, "cmdline_radio_btn" );

        button_grp_layout->addWidget( file_radio_btn, 1, 0 );
        button_grp_layout->addMultiCellWidget( cmdline_radio_btn, 0,0, 0,1 );

        switch (type) {
            case GConst::coverageAnalysisFile:
                extension = "ccvg";
                filt = tr("Coverage Analysis Files") + " (*.ccvg);;" + tr("All Files") + " (*)";
                break;
            case GConst::mapLayerFile:
                extension = "cmpl";
                filt = tr("Map Layer Files") + " (*.cmpl);;" + tr("All Files") + " (*)";
                break;
            case GConst::clutterPropModelFile:
                extension = "ccpm";
                filt = tr("Clutter Propagation Model Files") + " (*.ccpm);;" + tr("All Files") + " (*)";
                break;
            case GConst::statisticsReport:
                extension = "cstt";
                filt = tr("Call Statistics Files") + " (*.cstt);;" + tr("All Files") + " (*)";
                break;
            default:
                extension = "txt";
                filt = tr("Text Files") + " (*.txt);;" + tr("All Files") + " (*)";
                break;
        }

        fileChooser = new FileChooser( button_grp, "fileChooser ", CConst::saveFileMode );
        fileChooser ->setFileFilter( filt );
        fileChooser ->setDialogCaption("Choose File...");
        button_grp_layout->addWidget( fileChooser , 1, 1 );
    }

    okCancelLayout = new Q3HBoxLayout( 0, 0, 6, "okCancelLayout");
    spacer5 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    okCancelLayout->addItem( spacer5);
    ok_btn = new QPushButton( this, "ok_btn" );
    okCancelLayout->addWidget( ok_btn );
    spacer6 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    okCancelLayout->addItem( spacer6 );
    cancel_btn = new QPushButton( this, "cancel_btn" );
    okCancelLayout->addWidget( cancel_btn );
    spacer7 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    okCancelLayout->addItem( spacer7 );

    cancel_btn->setText( tr( "&Cancel" ) );
    cancel_btn->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    ok_btn->setText( tr( "&Ok" ) );
    ok_btn->setAccel( QKeySequence( tr( "Alt+O" ) ) );

    switch (type) {
        case GConst::statisticsReport:
            pre_num_sector = 0;
            sector_group_dia = new NumSectorApply(this, tr("Sector Group"));
            sector_group_dia->num_sector_ok_btn = NULL;
            
            layout->addMultiCellWidget( button_grp, 1, 1, 0, 4 );
            
            sta_btngroup1 = new Q3ButtonGroup( this, "sta_btngroup1" );
            sta_btngroup1->setColumnLayout(0, Qt::Vertical );
            sta_btngroup1->layout()->setSpacing( 10 );
            sta_btngroup1->layout()->setMargin( 11 );
            sta_btngroup1Layout = new Q3GridLayout( sta_btngroup1->layout() );
            sta_btngroup1Layout->setAlignment( Qt::AlignTop );
            spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
            sta_btngroup1Layout->addItem( spacer1, 0, 0 );
            
            sta_apply_btn = new QPushButton( tr( "&Apply" ), sta_btngroup1, "sta_apply_btn" );
            sta_btngroup1Layout->addMultiCellWidget( sta_apply_btn, 3, 3, 2, 3 );
            sta_apply_btn->setAccel( QKeySequence( tr( "Alt+A" ) ) );
            
            sta_num_sector_spinbox = new QSpinBox( sta_btngroup1, "sta_num_sector_spinbox" );
            sta_num_sector_spinbox->setEnabled( FALSE );
            sta_num_sector_spinbox->setMaxValue( 19 );
            sta_num_sector_spinbox->setMinValue( 1 );
            sta_num_sector_spinbox->setValue( 1 );
            sta_btngroup1Layout->addMultiCellWidget( sta_num_sector_spinbox, 2, 2, 3, 4 );

            sta_num_sector_lbl = new QLabel( tr( "Number of Sectors" ), sta_btngroup1, "sta_num_sector_lbl" );
            sta_btngroup1Layout->addMultiCellWidget( sta_num_sector_lbl, 2, 2, 0, 2 );
            spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
            sta_btngroup1Layout->addItem( spacer3, 3, 4 );
            spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
            sta_btngroup1Layout->addMultiCell( spacer2, 3, 3, 0, 1 );
            
            sta_entire_sys_radiobtn = new QRadioButton( tr( "Entire System" ), sta_btngroup1, "sta_entire_sys_radiobtn" );
            sta_entire_sys_radiobtn->setChecked( TRUE );
            sta_btngroup1Layout->addWidget( sta_entire_sys_radiobtn, 0, 1 );
            
            sta_sector_group_radiobtn = new QRadioButton( tr( "Sector Group" ), sta_btngroup1, "sta_sector_group_radiobtn" );
            sta_btngroup1Layout->addWidget( sta_sector_group_radiobtn, 1, 1 );
            
            layout->addMultiCellWidget( sta_btngroup1, 0, 0, 0, 4 );
            //------------------------------------------------------------------------------------
            
            layout->addMultiCellLayout( okCancelLayout, 2, 2,0, 4 );

            //------------------------------------------------------------------------------------
            
            // signals and slots connections
            connect( sta_btngroup1, SIGNAL( clicked(int) ), this, SLOT( sta_btngroup1_clicked(int) ) );
            connect( sta_apply_btn, SIGNAL( clicked() ), this, SLOT( sta_apply_clicked() ) );

#if 0
            setTabOrder( sta_entire_sys_radiobtn, sta_num_sector_spinbox );
            setTabOrder( sta_num_sector_spinbox, sta_apply_btn );
            setTabOrder( sta_apply_btn, sta_file_radiobtn );
            setTabOrder( sta_file_radiobtn, sta_ok_btn );
            setTabOrder( sta_ok_btn, sta_cancel_btn );
#endif

            cmdline_radio_btn->setChecked( TRUE );
            break;

        case GConst::coverageReport:
            setCaption( tr( "Coverage Report" ) );

            combo_box_lbl = new QLabel(  tr("Name of Coverage Analysis"), this, "cvg_report_textLabel" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0, 0, 1 );
            
            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0, 0, 2,4 );

            layout->addMultiCellWidget( button_grp, 1, 1, 0, 4 );
            
            layout->addMultiCellLayout( okCancelLayout, 2, 2,0, 4 );

            for( i=0; i<=np->num_coverage_analysis-1; i++ ) {
                combo_box->insertItem( np->coverage_list[i]->strid, i );
            }
            
            cmdline_radio_btn->setChecked( TRUE );
            break;
        case GConst::coveragePlot:
            setCaption( tr( "Coverage Plot" ) );
            to_file = false;

            combo_box_lbl = new QLabel(  tr("Name of Coverage Analysis"), this, "cvg_report_textLabel" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0, 0, 1 );
            
            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0, 0, 2,4 );
            for( i=0; i<=np->num_coverage_analysis-1; i++ ) {
                combo_box->insertItem( np->coverage_list[i]->strid, i );
            }

            layout->addMultiCellLayout( okCancelLayout, 2, 2,0, 4 );
            
            sta_btngroup1 = new Q3ButtonGroup( this, "sta_btngroup1" );
            sta_btngroup1->setColumnLayout(0, Qt::Vertical );
            sta_btngroup1->layout()->setSpacing( 10 );
            sta_btngroup1->layout()->setMargin( 11 );
            sta_btngroup1Layout = new Q3GridLayout( sta_btngroup1->layout() );
            sta_btngroup1Layout->setAlignment( Qt::AlignTop );
            spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
            sta_btngroup1Layout->addItem( spacer1, 0, 0 );

            check_box = new QCheckBox(  tr( "Compare with Coverage Analysis" ), sta_btngroup1, "check_box" );
            sta_btngroup1Layout->addMultiCellWidget( check_box, 0, 0, 0, 2 );
            connect( check_box, SIGNAL( toggled(bool) ), this, SLOT( check_box_toggled(bool) ));

            combo_box2_lbl = new QLabel(  tr("Name of Coverage Analysis"), sta_btngroup1, "cvg_report_textLabel" );
            sta_btngroup1Layout->addMultiCellWidget( combo_box2_lbl, 1,1, 0, 1 );

            combo_box2 = new QComboBox( sta_btngroup1, "combo_box2" );
            sta_btngroup1Layout->addMultiCellWidget( combo_box2, 1, 1, 2,4 );
            for( i=0; i<=np->num_coverage_analysis-1; i++ ) {
                combo_box2->insertItem( np->coverage_list[i]->strid, i );
            }
            combo_box2->setEnabled(false);

            layout->addMultiCellWidget( sta_btngroup1, 1, 1, 0, 4 );

            break;
        case GConst::subnetReport:
            setCaption( tr( "Subnet Report" ) );
            
            combo_box_lbl = new QLabel(tr( "Traffic Type" ), this, "combo_box_lbl" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0,0,1 );
            
            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0,0,2,4 );
            
            check_box = new QCheckBox(  tr( "Report cells contained in each subnet" ), this, "check_box" );
            layout->addMultiCellWidget( check_box, 1, 1, 0, 2 );
            
            layout->addMultiCellWidget( button_grp, 2,2,0,4 );
            
            layout->addMultiCellLayout( okCancelLayout, 3,3, 0,4 );

            for(i = 0; i<=np->num_traffic_type-1; i++) {
                combo_box->insertItem( np->traffic_type_list[i]->name(), i );
            }
            
            cmdline_radio_btn->setChecked( TRUE );
            break;

        case GConst::propagationParamReport:
            setCaption( tr( "Propation Model Parameter Report" ) );
            
            combo_box_lbl = new QLabel(tr( "Parameter" ), this, "combo_box_lbl" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0,0,1 );
            
            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0,0,2,4 );
            
            layout->addMultiCellWidget( button_grp, 1,1,0,4 );
            
            layout->addMultiCellLayout( okCancelLayout, 2,2, 0,4 );

            combo_box->insertItem( "INFLEXION", 0 );
            combo_box->insertItem( "SLOPE",     1 );

            combo_box->insertItem( "ALL",       2 );
            combo_box->insertItem( "PROP USED", 3 );
            
            cmdline_radio_btn->setChecked( TRUE );
            break;
        case GConst::propagationErrorReport:
            setCaption( tr( "Propagation Error Report" ) );

            layout->addMultiCellWidget( button_grp, 0, 0, 0, 4 );

            layout->addMultiCellLayout( okCancelLayout, 1,1, 0,4 );

            cmdline_radio_btn->setChecked( TRUE );
            break;
        case GConst::settingsReport:
            setCaption( tr( "Setting Report" ) );

            layout->addMultiCellWidget( button_grp, 0, 0, 0, 4 );

            layout->addMultiCellLayout( okCancelLayout, 1,1, 0,4 );

            cmdline_radio_btn->setChecked( TRUE );
            break;
        case GConst::coverageAnalysisFile :
            setCaption( tr( "Save Coverage Analysis" ) );

            combo_box_lbl = new QLabel( tr ( "Name of Coverage Analysis" ), this, "combo_box_lbl" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0,0,1 );
            
            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0,0,2,4 );

            layout->addMultiCellWidget( button_grp, 1,1,0,4 );
            
            layout->addMultiCellLayout( okCancelLayout, 2,2, 0,4 );

            for( i=0; i<=np->num_coverage_analysis-1; i++ ) {
                combo_box->insertItem( np->coverage_list[i]->strid, i );
            }
            
            cmdline_radio_btn->setEnabled(false);
            file_radio_btn->setChecked( TRUE );
            break;
        case GConst::mapLayerFile :
            setCaption( tr( "Save Map Layer" ) );

            combo_box_lbl = new QLabel( tr ( "Name of Map Layer" ), this, "combo_box_lbl" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0,0,1 );

            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0,0,2,4 );

            layout->addMultiCellWidget( button_grp, 1,1,0,4 );

            layout->addMultiCellLayout( okCancelLayout, 2,2, 0,4 );

            for( i=0; i<=np->map_layer_list->getSize()-1; i++ ) {
                combo_box->insertItem((*(np->map_layer_list))[i]->name, i );
            }

            cmdline_radio_btn->setEnabled(false);
            file_radio_btn->setChecked( TRUE );
            break;
        case GConst::clutterPropModelFile :
            setCaption( tr( "Export Clutter Propagation Model" ) );

            combo_box_lbl = new QLabel( tr ( "Name of Clutter Propagation Model" ), this, "combo_box_lbl" );
            layout->addMultiCellWidget( combo_box_lbl, 0,0,0,1 );

            combo_box = new QComboBox( this, "combo_box" );
            layout->addMultiCellWidget( combo_box, 0,0,2,4 );

            layout->addMultiCellWidget( button_grp, 1,1,0,4 );

            layout->addMultiCellLayout( okCancelLayout, 2,2, 0,4 );

            idx = 0;
            for( i=0; i<=np->num_prop_model-1; i++ ) {
                if (np->prop_model_list[i]->is_clutter_model()) {
                    combo_box->insertItem(np->prop_model_list[i]->get_strid(), idx++);
                }
            }

            cmdline_radio_btn->setEnabled(false);
            file_radio_btn->setChecked( TRUE );
            break;
        default:
            break;
    }

    resize( QSize(491, 50).expandedTo(minimumSizeHint()) );

    if (type != GConst::coveragePlot) {
        btngroup_clicked( cmdline_radio_btn->isChecked() ? 1 : 0);

        connect( fileChooser->lineEdit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( fileChooser_textChanged( const QString & ) ));
        connect( button_grp, SIGNAL( clicked(int) ), this, SLOT( btngroup_clicked(int) ) );
    }

    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_clicked() ) );
        
    exec();
}
    
/*
 *  Destroys the object and frees any allocated resources
 */
ReportDialog::~ReportDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void ReportDialog::check_box_toggled(bool s)
{
    if (type == GConst::coveragePlot) {
        combo_box2->setEnabled(s);
    }
}

/****************************************************************************
**    DEFINE SLOT
*****************************************************************************/
void ReportDialog::ok_btn_clicked()
{
    int i, idx, found;
    char *chptr;

    chptr = np->line_buf;
    
    switch (type) {
        case GConst::statisticsReport:
            chptr += sprintf(chptr, "print_call_statistics");

            if ( sta_sector_group_radiobtn->isChecked() ) {
                chptr += sprintf(chptr, " -sectors '");
                for (i=0; i<=num_sector-1; i++) {
                    chptr += sprintf(chptr, "%d_%d ",
                                        sector_group_dia->cell_num_val[i]->text().toInt(),
                                        sector_group_dia->sector_num_val[i]->text().toInt());
                }
                chptr += sprintf(chptr, "'");
            }
            break;
        case GConst::coverageReport:
            chptr += sprintf(chptr, "report_coverage_analysis -name %s", np->coverage_list[combo_box->currentItem()]->strid);
            break;
        case GConst::subnetReport:
            chptr += sprintf(chptr, "report_subnets -traffic_type %s", np->traffic_type_list[combo_box->currentItem()]->name());

            if (check_box->isChecked()) {
                chptr += sprintf(chptr, " -contained_cells");
            }
            break;
        case GConst::propagationErrorReport:
            chptr += sprintf(chptr, "comp_prop_error");
            break;
        case GConst::settingsReport:
            chptr += sprintf(chptr, "display_settings");
            break;
        case GConst::coverageAnalysisFile :
            chptr += sprintf(chptr, "write_coverage_analysis -name %s", np->coverage_list[combo_box->currentItem()]->strid);
            break;
        case GConst::coveragePlot:
            chptr += sprintf(chptr, "plot_coverage_analysis -name %s", np->coverage_list[combo_box->currentItem()]->strid);
            if (check_box->isChecked()) {
                chptr += sprintf(chptr, " -name2 %s", np->coverage_list[combo_box2->currentItem()]->strid);
            }
            break;
        case GConst::mapLayerFile :
            chptr += sprintf(chptr, "save_map_layer -map_layer_idx %d", combo_box->currentItem());
            break;
        case GConst::clutterPropModelFile :
            found = 0;
            i = 0;
            idx = 0;
            do {
                if (np->prop_model_list[i]->is_clutter_model()) {
                    if (idx == combo_box->currentItem()) {
                        found = 1;
                    }
                    idx++;
                }
                if (!found) {
                    i++;
                }
            } while((!found) && (i<=np->num_prop_model-1));

            chptr += sprintf(chptr, "export_clutter_model -pm_idx %d", i);
            break;
        case GConst::propagationParamReport:
            chptr += sprintf(chptr, "report_prop_model_param -param %s",
                ((combo_box->currentItem() == 0) ? "inflexion -model all" :
                 (combo_box->currentItem() == 1) ? "slope -model all"     : 
                 (combo_box->currentItem() == 2) ? "all -model all"       : 
                 "all -model used" ));
            break;
    }

    if (to_file) {
        QString fn = fileChooser ->lineEdit->text();

        if (!fn.isEmpty()) {
            QRegExp rx = QRegExp("\\." + extension + "$");
            if (rx.search(fn) == -1) {
                fn += "." + extension;
            }

            Q3CString qcs(2*fn.length());
            qcs = fn.local8Bit();

            chptr += sprintf(chptr, " -f \'%s\'", (const char *) qcs);
        } else {
            return;
        }
    } 
    np->process_command( np->line_buf);
    
    delete this;
}


void ReportDialog::cancel_clicked()
{
    delete this;
}

void ReportDialog::sta_apply_clicked()
{
    num_sector = sta_num_sector_spinbox->value(); 
    
    if (sta_entire_sys_radiobtn->isChecked()) {
    }
    
    if (sta_sector_group_radiobtn->isChecked()) {

        if ( sector_group_dia->num_sector_ok_btn ) delete sector_group_dia->num_sector_ok_btn;
        if ( num_sector <= pre_num_sector)
        {
            for (int i=num_sector; i<pre_num_sector; i++)
            {
                delete sector_group_dia->cell_num_lbl[i];
                delete sector_group_dia->cell_num_val[i];
                delete sector_group_dia->sector_num_lbl[i];
                delete sector_group_dia->sector_num_val[i];
                //delete sector_group_dia->num_sector_ok_btn;
            }
        }
        else
        {
            const int cell_label_col   = 0;
            const int cell_line_col    = 1;
            const int sector_label_col = 2;
            const int sector_line_col  = 3;
            int row;
            
            for (row=pre_num_sector; row<num_sector; row++)
            {
                sector_group_dia->sector_num_lbl[row] = new QLabel(sector_group_dia);
                sector_group_dia->cell_num_lbl[row] = new QLabel(sector_group_dia);
                sector_group_dia->cell_num_lbl[row]->setText( tr("Cell Index") + QString("_%1").arg(row) );
                sector_group_dia->sector_num_lbl[row]->setText( tr("Sector Index") + QString("_%1").arg(row) );
                sector_group_dia->num_sector_grid->addWidget( sector_group_dia->cell_num_lbl[row], row+1, cell_label_col );
                sector_group_dia->num_sector_grid->addWidget( sector_group_dia->sector_num_lbl[row], row+1, sector_label_col );
                sector_group_dia->sector_num_lbl[row]->show();
                sector_group_dia->cell_num_lbl[row]->show();
                
                sector_group_dia->sector_num_val[row] = new QLineEdit( sector_group_dia);
                sector_group_dia->cell_num_val[row] = new QLineEdit( sector_group_dia);
                sector_group_dia->sector_num_val[row]->setMaximumWidth(80);
                sector_group_dia->cell_num_val[row]->setMaximumWidth(80);
                sector_group_dia->num_sector_grid->addWidget( sector_group_dia->sector_num_val[row], row+1, sector_line_col );
                sector_group_dia->num_sector_grid->addWidget( sector_group_dia->cell_num_val[row], row+1, cell_line_col );
                sector_group_dia->sector_num_val[row]->show();
                sector_group_dia->cell_num_val[row]->show();
            }
            
        }

        sector_group_dia->num_sector_ok_btn = new QPushButton(tr("Ok"), sector_group_dia);
        sector_group_dia->num_sector_grid->addMultiCellWidget(sector_group_dia->num_sector_ok_btn, num_sector+1, num_sector+1, 0, 3, Qt::AlignHCenter);
        connect(sector_group_dia->num_sector_ok_btn, SIGNAL(clicked()), sector_group_dia, SLOT(hide()));

        sector_group_dia->resize( 490, 100 ); 
        sector_group_dia->show();
    }
    pre_num_sector = num_sector;
}

void ReportDialog::sta_btngroup1_clicked( int idx )
{
    switch ( idx ) 
    {
        case 1:
            sta_num_sector_spinbox->setDisabled(true);
            sta_apply_btn->setDisabled(true);
            break;
        case 2:
            sta_num_sector_spinbox->setDisabled(false);
            sta_apply_btn->setDisabled(false);
            break;
    }
}

void ReportDialog::btngroup_clicked( int idx )
{
    switch( idx ) {
        case 0:
            if ( fileChooser->lineEdit->text() == "" ){
                ok_btn->setDisabled( true );
            }
            fileChooser ->setDisabled( false );
            to_file = true;
            break;
        case 1:
            fileChooser ->setDisabled( true );
            ok_btn->setDisabled( false );
            to_file = false ;
            break;
    }
}

void ReportDialog::fileChooser_textChanged(const  QString& cc )
{
    if ( cc != "" && to_file )
    {
        ok_btn->setDisabled( false );
    }
    else if  ( cc == "" && to_file )
    {
        ok_btn->setDisabled( true );
    }
    else if ( !to_file )
    {
        ok_btn->setDisabled( false );
    }
}
