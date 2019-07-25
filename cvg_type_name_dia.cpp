/******************************************************************************************/
/**** PROGRAM: cvg_type_name_dia.cpp                                                   ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>

#include "cconst.h"
#include "wisim.h"
#include "coverage.h"
#include "cvg_type_name_dia.h"

//*****************************************************************************
//    FUNCTION:CvgTypeNameDia::CvgTypeNameDia 
//*****************************************************************************
CvgTypeNameDia::CvgTypeNameDia(NetworkClass *np_param, QWidget* parent) : QDialog(parent, 0, true)
{
    QString cvg_layer_str;
    QString cvg_level_str;
    QString cvg_sir_str;

    setFixedWidth( 300 );
    setCaption("Coverage Parameters");
    np = np_param;

    cvg_type_name_grid = new Q3GridLayout(this, 4, 2, 4, 2, "cvg_type_name_grid");

    //buttongroup setting
    cvg_type_btngroup = new Q3ButtonGroup(1, Qt::Horizontal,
                                         QString(np->technology_str()) + " " + tr("Coverage Analysis"), this);
    cvg_type_btngroup->setMinimumWidth( width() ); 

    if (np->technology() == CConst::CDMA2000) {
        cvg_layer_str = tr("Pilot Layer");
        cvg_level_str = tr("Pilot Level");
        cvg_sir_str   = tr("Pilot Pollution");
    } else {
        cvg_layer_str = tr("Layer");
        cvg_level_str = tr("Level");
        cvg_sir_str   = tr("SIR");
    }

    layer_radiobtn = new QRadioButton(cvg_layer_str, cvg_type_btngroup);
    cvg_type_btngroup->insert(layer_radiobtn, 0);
    layer_radiobtn->setChecked(true);

    level_radiobtn = new QRadioButton(cvg_level_str, cvg_type_btngroup);
    cvg_type_btngroup->insert(level_radiobtn, 1);

    sir_layer_radiobtn = new QRadioButton(cvg_sir_str, cvg_type_btngroup);
    cvg_type_btngroup->insert(sir_layer_radiobtn, 2);
    if (np->technology() == CConst::CDMA2000) {
        sir_layer_radiobtn->setEnabled(false);
    }

    if (np->technology() == CConst::PHS) {
    pa_layer_radiobtn = new QRadioButton(tr("Paging Area"), cvg_type_btngroup);
    cvg_type_btngroup->insert(pa_layer_radiobtn, 3);
    }

    cvg_type_name_grid->addMultiCellWidget(cvg_type_btngroup, 0, 0, 0, 1, Qt::AlignHCenter);

    //coverage name label and lineedit
    cvg_name_lbl = new QLabel(tr("Name"), this, "cvg_name_lbl");
    cvg_name_lbl->setMaximumWidth( 100 );
    cvg_type_name_grid->addWidget(cvg_name_lbl, 1, 0, Qt::AlignHCenter);	

    cvg_name_val = new QLineEdit(this, "cvg_name_val");
    cvg_name_val->setMaximumWidth( 120 );
    cvg_name_val->setMinimumWidth( 120 );
    cvg_type_name_grid->addWidget(cvg_name_val, 1, 1, Qt::AlignHCenter);	

    //advance, ok and cancel button setting
    ok_cancel_hlayout = new Q3HBoxLayout( 0, 4, 2, "ok_cancel_hlayout");

    QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_hlayout->addItem( sp1 );

    ok_btn = new QPushButton(tr("&Ok"), this, "ok_btn");
    ok_btn->setMaximumWidth( 100 );
    ok_btn->setMinimumWidth( 100 );
    ok_cancel_hlayout->addWidget( ok_btn, 0, Qt::AlignRight );

    QSpacerItem* sp2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_hlayout->addItem( sp2 );
    
    cancel_btn = new QPushButton(tr("&Cancel"), this, "cancel_btn");
    cancel_btn->setMaximumWidth( 100 );
    cancel_btn->setMinimumWidth( 100 );
    ok_cancel_hlayout->addWidget( cancel_btn, 0, Qt::AlignRight );

    QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ok_cancel_hlayout->addItem( sp3 );

    cvg_type_name_grid->addMultiCellLayout(ok_cancel_hlayout, 2, 2, 0, 1, Qt::AlignHCenter);

    //slot and signal connection
    connect(cvg_type_btngroup, SIGNAL(clicked(int)), this, SLOT(cvg_type_btngroup_clicked(int)));
    connect(ok_btn, SIGNAL(clicked()), this, SLOT(cvg_ok_btn_clicked()));
    connect(cancel_btn, SIGNAL(clicked()), this, SLOT(cvg_cancel_btn_clicked()));

    //initial
   cvg_type = QString( tr("Layer") );
   cvg_type_idx = CConst::layerCoverage;
 
    //set name of new coverage
    int  repeat_num = 0;
    char string[100];

    if ( layer_radiobtn->isChecked() ) {
        for( int cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
            sprintf(string,np->coverage_list[cvg_idx]->strid);
            if ( strncmp(string,"layer_type",10)==0 ) {
                repeat_num = repeat_num + 1;
            }
        }
        sprintf( string, "layer_type_%d", repeat_num );
        cvg_name_val->setText(string);
    }


    if ( level_radiobtn->isChecked() ) {
        for( int cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
            sprintf(string,np->coverage_list[cvg_idx]->strid);
            if ( strncmp(string,"level_type",10)==0 ) {
                repeat_num = repeat_num + 1;
            }
        }
        sprintf( string, "level_type_%d", repeat_num );
        cvg_name_val->setText(string);
    }

    if ( sir_layer_radiobtn->isChecked() ) {
        for( int cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
            sprintf(string,np->coverage_list[cvg_idx]->strid);
            if ( strncmp(string,"sir_layer_type",14)==0 ) {
                repeat_num = repeat_num + 1;
            }
        }
        sprintf( string, "sir_layer_type_%d", repeat_num );
        cvg_name_val->setText(string);
    }

    if (np->technology() == CConst::PHS) {
    if ( pa_layer_radiobtn->isChecked() ) {
        for( int cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
            sprintf(string,np->coverage_list[cvg_idx]->strid);
            if ( strncmp(string,"pa_type",7)==0 ) {
                repeat_num = repeat_num + 1;
            }
        }
        sprintf( string, "pa_layer_type_%d", repeat_num );
        cvg_name_val->setText(string);
    }
    }
}

CvgTypeNameDia::~CvgTypeNameDia()
{

}

//*****************************************************************************
//    FUNCTION:CvgTypeNameDia::cvg_type_btngroup_clicked()
//*****************************************************************************
void CvgTypeNameDia::cvg_type_btngroup_clicked(int i)
{
    int cvg_idx;
    int  repeat_num = 0;
    char string[100];

    switch (i) {
        case 0:
            for( cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
                sprintf(string,np->coverage_list[cvg_idx]->strid);
                if ( strncmp(string,"layer_type",10)==0 ) {
                    repeat_num = repeat_num + 1;
                }
            }
            sprintf( string, "layer_type_%d", repeat_num );
            cvg_name_val->setText(string);

            cvg_type = QString( tr("Layer") );
            cvg_type_idx = CConst::layerCoverage ; 
            break;

        case 1:
            for( cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
                sprintf(string,np->coverage_list[cvg_idx]->strid);
                if ( strncmp(string,"level_type",10)==0 ) {
                    repeat_num = repeat_num + 1;
                }
            }
            sprintf( string, "level_type_%d", repeat_num );
            cvg_name_val->setText(string);

            cvg_type = QString( tr("Level") );
            cvg_type_idx = CConst::levelCoverage; 
            break;

        case 2:
            for( cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
                sprintf(string,np->coverage_list[cvg_idx]->strid);
                if ( strncmp(string,"sir_layer_type",14)==0 ) {
                    repeat_num = repeat_num + 1;
                }
            }
            sprintf( string, "sir_layer_type_%d", repeat_num );
            cvg_name_val->setText(string);

            cvg_type = QString( tr("SIR") );
            cvg_type_idx = CConst::sirLayerCoverage; 
            break;

        case 3:
            for( cvg_idx=0; cvg_idx<np->num_coverage_analysis; cvg_idx++) {
                sprintf(string,np->coverage_list[cvg_idx]->strid);
                if ( strncmp(string,"pa_type",7)==0 ) {
                    repeat_num = repeat_num + 1;
                }
            }
            sprintf( string, "pa_type_%d", repeat_num );
            cvg_name_val->setText(string);

            cvg_type = QString( tr("PA") );
            cvg_type_idx = CConst::pagingAreaCoverage; 
            break;

        default:
            break;
    }
}

void CvgTypeNameDia::cvg_ok_btn_clicked()
{
    cvg_name = cvg_name_val->text();
}

void CvgTypeNameDia::cvg_cancel_btn_clicked()
{
    delete this;
}
