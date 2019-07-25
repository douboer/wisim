/******************************************************************************************
**** PROGRAM: set_strid_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <math.h>
#include <stdlib.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <q3scrollview.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>

#include "cvg_analysis_page.h"
#include "coverage.h"
#include "pref.h"
#include "list.h"
#include "wisim.h"

#define PRECISION  0.00001 

/******************************************************************************************/
/**** FUNCTION: Virtual Functions                                                      ****/
/******************************************************************************************/
void         CvgAnalysisPage::run_btn_clicked()                      { CORE_DUMP; return; }
/******************************************************************************************/

/******************************************************************************************/
/****                          FUNCTION: CvgAnalysisPage                               ****/
/******************************************************************************************/
CvgAnalysisPage::CvgAnalysisPage(NetworkClass *np_param, QWidget* parent, char *name)
    : QWidget(parent, name)
{
    np = np_param;
    page_name = strdup(name);
}


CvgAnalysisPage::~CvgAnalysisPage()
{
    free(page_name);
}


/******************************************************************************************/
/**** FUNCTION: CvgAnalysisPage::advnaced_btn_clicked                                  ****/
/******************************************************************************************/
void CvgAnalysisPage::adv_btn_clicked()
{
    advShown = !advShown;

    if ( advShown ) {
        scan_fractional_area_lbl->show();
        scan_fractional_area_val->show();
        init_sample_res_lbl->show();
        init_sample_res_val->show();
    } else {
        scan_fractional_area_lbl->hide();
        scan_fractional_area_val->hide();
        init_sample_res_lbl->hide();
        init_sample_res_val->hide();
    }

}

/******************************************************************************************/
/**** FUNCTION: LayerPage::LayerPage                                                   ****/
/******************************************************************************************/
LayerPage::LayerPage(NetworkClass *np, int cvg_idx, QWidget* parent, char* name)
    : CvgAnalysisPage(np, parent, name)
{
    QString s;
    cvg_analysis_page_gridlayout = new Q3GridLayout(this, 3, 2, 5, 5, "layer_cvg_analysis_page_gridlayout") ;

    //layer threshold setting
    s.append(tr("&Signal Threshold")+" (" + QString(np->preferences->pwr_str_short)+")");
    layer_thr_lbl = new QLabel(s, this, "layer_thr_lbl");
    layer_thr_lbl->setMaximumWidth(120);
    cvg_analysis_page_gridlayout->addWidget(layer_thr_lbl, 0, 0, Qt::AlignHCenter);

    layer_thr_val = new QLineEdit(this, "layer_thr_val");
    layer_thr_val->setMaximumSize( QSize( 120, 32767 ) );
    layer_thr_lbl->setBuddy(layer_thr_val);
    cvg_analysis_page_gridlayout->addWidget(layer_thr_val, 0, 1, Qt::AlignHCenter);

    //layer number setting
    max_layer_lbl = new QLabel( tr("&Max Layer"), this, "max_layer_lbl");
    max_layer_lbl->setMaximumWidth(120);
    cvg_analysis_page_gridlayout->addWidget(max_layer_lbl, 1, 0, Qt::AlignHCenter);

    max_layer_spinbox = new QSpinBox( 1, 40, 1, this, "max_layer_spinbox");
    max_layer_spinbox->setMaximumWidth( 120 );
    max_layer_spinbox->setMinimumWidth( 120 );
    max_layer_lbl->setBuddy(max_layer_spinbox);
    cvg_analysis_page_gridlayout->addWidget(max_layer_spinbox, 1, 1, Qt::AlignHCenter);

    //run button
    run_btn = new QPushButton( tr("&Run"), this, "run_btn");
    run_btn->setMaximumWidth(80);
    cvg_analysis_page_gridlayout->addWidget(run_btn, 2, 1, Qt::AlignHCenter);

    //fractional area label and lineedit
    scan_fractional_area_lbl = new QLabel( tr("Scan Fraction Area"), this, "scan_fractional_area_lbl");
    scan_fractional_area_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addWidget( scan_fractional_area_lbl, 3, 0, Qt::AlignHCenter );

    scan_fractional_area_val = new QLineEdit(this, "scan_fractional_area_val");
    scan_fractional_area_val->setMaximumSize( QSize( 120, 32767 ) );
    scan_fractional_area_lbl->setBuddy( scan_fractional_area_val );
    cvg_analysis_page_gridlayout->addWidget( scan_fractional_area_val, 3, 1, Qt::AlignHCenter );

    //initial sample resolution label and lineedit
    init_sample_res_lbl = new QLabel( tr("Initial Sample resolution"), this, "init_sample_res_lbl");
    init_sample_res_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addWidget( init_sample_res_lbl, 4, 0, Qt::AlignHCenter );

    init_sample_res_val = new QLineEdit(this, "init_sample_res_val");
    init_sample_res_val->setMaximumSize( QSize( 120, 32767 ) );
    init_sample_res_lbl->setBuddy( init_sample_res_val );
    cvg_analysis_page_gridlayout->addWidget( init_sample_res_val, 4, 1, Qt::AlignHCenter );

    //set value
    QString str1 = str1.number( 10*log(np->coverage_list[cvg_idx]->threshold)/log(10.0) );
    QString str2 = str2.number( np->coverage_list[cvg_idx]->scan_fractional_area );
    QString str3 = str3.number( np->coverage_list[cvg_idx]->init_sample_res );

    layer_thr_val->setText(str1);
    scan_fractional_area_val->setText(str2);
    init_sample_res_val->setText(str3);
    max_layer_spinbox->setValue( np->coverage_list[cvg_idx]->scan_list->getSize()-1 );

    connect(run_btn, SIGNAL(clicked()), this, SLOT(run_btn_clicked()));

    advShown = true;
    adv_btn_clicked();

    resize( QSize(673, 365).expandedTo(minimumSizeHint()) );
}

LayerPage::~LayerPage()
{

}

/******************************************************************************************/
/**** FUNCTION: LayerPage::run_btn_clicked                                             ****/
/******************************************************************************************/
void LayerPage::run_btn_clicked()
{

    int     max_layer  = max_layer_spinbox->value();
    int     init_sample_res = atoi(init_sample_res_val->text().latin1());
    double  signal_thr = layer_thr_val->text().toDouble() + np->preferences->pwr_offset;
    double  scan_fractional_area = atof(scan_fractional_area_val->text().latin1());

    sprintf(np->line_buf, "set_coverage_analysis -name %s -threshold_db %f -num_scan_type %d",
            page_name, signal_thr, max_layer + 1);
    np->process_command(np->line_buf);

    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            page_name, scan_fractional_area, init_sample_res);
    np->process_command(np->line_buf);

    sprintf(np->line_buf, "run_coverage -name %s", page_name);
    np->process_command(np->line_buf);
}

/******************************************************************************************/
/**** FUNCTION: LevelPage::LevelPage                                                   ****/
/******************************************************************************************/
LevelPage::LevelPage(NetworkClass *np, int cvg_idx, QWidget* parent, char* name)
    : CvgAnalysisPage(np, parent, name)
{
    level_cvg_idx = cvg_idx;
    QString s;

    cvg_analysis_page_gridlayout = new Q3GridLayout(this, 5, 4, 5, 5, "level_cvg_analysis_page_gridlayout") ;

    //threshold setting
    num_threshold_lbl = new QLabel(tr("Number of &Thresholds"), this, "num_threshold_lbl");
    num_threshold_lbl->setMaximumWidth(120);
    cvg_analysis_page_gridlayout->addWidget(num_threshold_lbl, 0, 0, Qt::AlignLeft);

    num_threshold_spinbox = new QSpinBox( 1, 40, 1, this, "num_threshold_val");
    num_threshold_spinbox->setMaximumWidth( 120 );
    num_threshold_spinbox->setMinimumWidth( 120 );
    num_threshold_lbl->setBuddy(num_threshold_spinbox);
    cvg_analysis_page_gridlayout->addWidget(num_threshold_spinbox, 0, 1, Qt::AlignLeft);

    manually_set_btn = new QPushButton( tr("&Manually Set Thresholds"), this, "manually set" );
    manually_set_btn->setMaximumWidth( 160 );
    cvg_analysis_page_gridlayout->addWidget(manually_set_btn, 0, 2, Qt::AlignLeft);

    evenly_spaced_btn = new QPushButton(tr("&Evenly Spaced Thresholds"), this, "evenly_spaced_btn" );
    evenly_spaced_btn->setMaximumWidth( 160 );
    cvg_analysis_page_gridlayout->addWidget(evenly_spaced_btn, 0, 3, Qt::AlignLeft);

    s = tr("Minimum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    min_threshold_lbl = new QLabel(s, this, "min_threshold_lbl");
    min_threshold_lbl->setMaximumWidth( 120 );
    cvg_analysis_page_gridlayout->addWidget( min_threshold_lbl, 1, 0, Qt::AlignLeft );

    min_threshold_val = new QLineEdit( this, "min_threshold_val");
    min_threshold_val->setMaximumSize( QSize( 120, 32767 ) );
    min_threshold_lbl->setBuddy( min_threshold_val );
    cvg_analysis_page_gridlayout->addWidget( min_threshold_val, 1, 1, Qt::AlignLeft );

    s = tr("Maximum Threshold") + " (" + QString(np->preferences->pwr_str_short) + ")";
    max_threshold_lbl = new QLabel(s, this, "max_threshold_lbl");
    max_threshold_lbl->setMaximumWidth( 120 );
    cvg_analysis_page_gridlayout->addWidget( max_threshold_lbl, 1, 2, Qt::AlignLeft );

    max_threshold_val = new QLineEdit( this, "max_threshold_val");
    max_threshold_val->setMaximumSize( QSize( 120, 32767 ) );
    max_threshold_lbl->setBuddy( max_threshold_val );
    cvg_analysis_page_gridlayout->addWidget( max_threshold_val, 1, 3, Qt::AlignLeft );

    //run and advanced button
    run_btn = new QPushButton(tr("&Run"), this, "run_btn");
    run_btn->setMaximumWidth(80);
    cvg_analysis_page_gridlayout->addWidget(run_btn, 2, 3, Qt::AlignHCenter);

    //fractional area label and lineedit
    scan_fractional_area_lbl = new QLabel(tr("Scan Fraction Area"), this, "scan_fractional_area_lbl");
    scan_fractional_area_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addMultiCellWidget( scan_fractional_area_lbl, 3, 3, 0, 1, Qt::AlignHCenter );

    scan_fractional_area_val = new QLineEdit(this, "scan_fractional_area_val");
    scan_fractional_area_val->setMaximumSize( QSize( 120, 32767 ) );
    scan_fractional_area_lbl->setBuddy( scan_fractional_area_val );
    cvg_analysis_page_gridlayout->addMultiCellWidget( scan_fractional_area_val, 3, 3, 2, 3, Qt::AlignHCenter );

    //initial sample resolution label and lineedit
    init_sample_res_lbl = new QLabel(tr("Initial Sample resolution"), this, "init_sample_res_lbl");
    init_sample_res_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addMultiCellWidget( init_sample_res_lbl, 4, 4, 0, 1, Qt::AlignHCenter );

    init_sample_res_val = new QLineEdit(this, "init_sample_res_val");
    init_sample_res_val->setMaximumSize( QSize( 120, 32767 ) );
    init_sample_res_lbl->setBuddy( init_sample_res_val );
    cvg_analysis_page_gridlayout->addMultiCellWidget( init_sample_res_val, 4, 4, 2, 3, Qt::AlignHCenter );

    //set value
    int max_idx = np->coverage_list[cvg_idx]->scan_list->getSize() - 2;
    QString str1 = str1.number( 10*log(np->coverage_list[cvg_idx]->level_list[0])/log(10.0) );
    QString str2 = str2.number( 10*log(np->coverage_list[cvg_idx]->level_list[max_idx])/log(10.0) );
    QString str3 = str3.number( np->coverage_list[cvg_idx]->scan_fractional_area );
    QString str4 = str4.number( np->coverage_list[cvg_idx]->init_sample_res );

    num_threshold_spinbox->setValue(np->coverage_list[cvg_idx]->scan_list->getSize() - 1);
    min_threshold_val->setText( str1 );
    max_threshold_val->setText( str2 );
    scan_fractional_area_val->setText( str3 );
    init_sample_res_val->setText( str4 );

    //set the minimum and maximum threshold properity
    if ( num_threshold_spinbox->value() == 1 ) {
        min_threshold_val->setDisabled( false );
        max_threshold_val->setDisabled( true );
    } else {
        min_threshold_val->setDisabled( false );
        max_threshold_val->setDisabled( false );
    }

    connect(num_threshold_spinbox, SIGNAL(valueChanged( int )), this, SLOT(min_max_disable_set( int )));
    connect(evenly_spaced_btn, SIGNAL(clicked()), this, SLOT(evenly_spaced_btn_clicked()));
    connect(manually_set_btn,  SIGNAL(clicked()), this, SLOT(manual_set_btn_clicked()));
    connect(run_btn, SIGNAL(clicked()), this, SLOT(run_btn_clicked()));

    advShown = true;
    adv_btn_clicked();

    resize( QSize(673, 365).expandedTo(minimumSizeHint()) );
}

LevelPage::~LevelPage()
{

}

/******************************************************************************************/
/**** FUNCTION: LevelPage::run_btn_clicked                                             ****/
/******************************************************************************************/
void LevelPage::run_btn_clicked()
{
    sprintf(np->line_buf, "run_coverage -name %s", page_name);
    np->process_command(np->line_buf);
}

/******************************************************************************************/
/**** FUNCTION: LevelPage::apply_btn_clicked                                           ****/
/******************************************************************************************/
void LevelPage::min_max_disable_set(int value)
{
    if ( value == 1 ) {
        min_threshold_val->setDisabled( false );
        max_threshold_val->setDisabled( true );
    } else {
        min_threshold_val->setDisabled( false );
        max_threshold_val->setDisabled( false );
    }
}

/******************************************************************************************/
/**** FUNCTION: LevelPage::apply_btn_clicked                                           ****/
/******************************************************************************************/
void LevelPage::evenly_spaced_btn_clicked()
{
    bool    man_set = false;
    int     num_thr = num_threshold_spinbox->value();
    double  min_thr = atof(min_threshold_val->text().latin1());
    double  max_thr = atof(max_threshold_val->text().latin1());
    double  space   = 0.0;

    int     init_sample_res = atoi(init_sample_res_val->text().latin1());
    double  scan_fractional_area = atof(scan_fractional_area_val->text().latin1());

    sprintf(np->line_buf, "set_coverage_analysis -name %s -num_scan_type %d",
            page_name, num_thr + 1);
    np->process_command(np->line_buf);

    if ( num_thr > 2 ) {
        space = ( max_thr - min_thr )/( num_thr - 1);
    }

    level_thr_page = new LevelThresholdPage(np, level_cvg_idx, num_thr, min_thr, max_thr, space, man_set, this, page_name);
    level_thr_page->setCaption(tr("Level Threshold Page Setting"));
    level_thr_page->show();

    connect( level_thr_page->apply_btn, SIGNAL( clicked() ), this, SLOT( thr_page_apply_btn_clicked() ) );
    connect( level_thr_page->cancel_btn,SIGNAL( clicked() ), this, SLOT( thr_page_cancel_btn_clicked() ) );

    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            page_name, scan_fractional_area, init_sample_res);
    np->process_command(np->line_buf);
}

/******************************************************************************************/
/**** FUNCTION: LevelPage::apply_btn_clicked                                           ****/
/******************************************************************************************/
void LevelPage::manual_set_btn_clicked()
{
    bool    man_set = true;
    int     num_thr = num_threshold_spinbox->value();
    double  min_thr = atof(min_threshold_val->text().latin1());
    double  max_thr = atof(max_threshold_val->text().latin1());
    double  space   = 0.0;

    int     init_sample_res = atoi(init_sample_res_val->text().latin1());
    double  scan_fractional_area = atof(scan_fractional_area_val->text().latin1());

    sprintf(np->line_buf, "set_coverage_analysis -name %s -num_scan_type %d",
            page_name, num_thr + 1);
    np->process_command(np->line_buf);

    if ( num_thr > 2 ) {
        space = ( max_thr - min_thr )/( num_thr - 1);
    }

    level_thr_page = new LevelThresholdPage(np, level_cvg_idx, num_thr, min_thr, max_thr, space, man_set, this, page_name);
    level_thr_page->setCaption(tr("Level Threshold Page Setting"));
    level_thr_page->show();

    connect( level_thr_page->apply_btn, SIGNAL( clicked() ), this, SLOT( thr_page_apply_btn_clicked() ) );
    connect( level_thr_page->cancel_btn,SIGNAL( clicked() ), this, SLOT( thr_page_cancel_btn_clicked() ) );

    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            page_name, scan_fractional_area, init_sample_res);
    np->process_command(np->line_buf);
}

void LevelPage::thr_page_apply_btn_clicked()
{
    int     num_thr = num_threshold_spinbox->value();
    level_thr_page->hide();

    for( int i=0; i<num_thr; i++) {
        sprintf(np->line_buf, "set_coverage_analysis -name %s -thr_idx %d -threshold_db %f",
                page_name, i, level_thr_page->level_thr_val[i]->text().toDouble()+np->preferences->pwr_offset);
        np->process_command(np->line_buf);
    }

    delete level_thr_page;
}

void LevelPage::thr_page_cancel_btn_clicked()
{
    delete level_thr_page;
}

/******************************************************************************************/
/**** FUNCTION: LevelThresholdPage::LevelThresholdPage                                 ****/
/******************************************************************************************/
LevelThresholdPage::LevelThresholdPage( NetworkClass *param_np, int cvg_idx, int num_level,
    double min_thr, double max_thr, double space, bool man_set, QWidget* parent,  char *name)
    : QDialog( parent, name, true) 
{
    int    i; //loop variable
    int    max_idx;
    int    np_num_level;
    double np_min_thr;
    double np_max_thr;

    np = param_np;

    thr_page_Layout = new Q3GridLayout( this, 1, 1, 11, 6, "thr_page_Layout");
    thr_page_Layout->setColStretch(0,1);

    thr_page_scrollview = new Q3ScrollView( this, "thr_page_scrollview");
    thr_page_scrollview->setPaletteBackgroundColor(backgroundColor());
    thr_page_scrollview->setVScrollBarMode(Q3ScrollView::Auto);
    thr_page_scrollview->setHScrollBarMode(Q3ScrollView::Auto);

    thr_page_frame = new Q3Frame( thr_page_scrollview->viewport(), "thr_page_frame" );
    thr_page_frameLayout = new Q3GridLayout( thr_page_frame, 1, 1, 11, 6, "thr_page_frameLayout");
    thr_page_frameLayout->setColStretch(1,1);

    //apply and cancel button
    apply_btn = new QPushButton(tr("&Apply"),thr_page_frame,"apply_btn");
    apply_btn->setMaximumWidth(80);
    thr_page_frameLayout->addWidget(apply_btn, 0, 0, Qt::AlignCenter);

    cancel_btn = new QPushButton(tr("&Cancel"),thr_page_frame,"cancel_btn");
    cancel_btn->setMaximumWidth(80);
    thr_page_frameLayout->addWidget(cancel_btn, 0, 1, Qt::AlignCenter);

    //threshold value setting
    level_thr_lbl = ( QLabel ** ) malloc((num_level)*sizeof( QLabel * ) );
    level_thr_val = ( QLineEdit ** ) malloc((num_level)*sizeof( QLineEdit * ) );

    for( i=0; i<num_level; i++ ) {
        QString str1;
        str1 = QString("Threshold_%1 ( dB )").arg(i);

        level_thr_lbl[i] = new QLabel(thr_page_frame, "level_thr_lbl");
        level_thr_lbl[i]->setText( str1 );
        level_thr_lbl[i]->setMaximumWidth( 120 );
        thr_page_frameLayout->addWidget( level_thr_lbl[i], i+1, 0, Qt::AlignCenter );

        level_thr_val[i] = new QLineEdit( thr_page_frame, "level_thr_val" );
        level_thr_val[i]->setMaximumSize( QSize( 120, 32767 ) );
        level_thr_val[i]->setReadOnly( !man_set );
        level_thr_lbl[i]->setBuddy(level_thr_val[i]);
        thr_page_frameLayout->addWidget( level_thr_val[i], i+1, 1, Qt::AlignCenter );
    }

    thr_page_scrollview->addChild(thr_page_frame);
    thr_page_Layout->addWidget(thr_page_scrollview, 0, 0, Qt::AlignCenter);

    //set value. if num_level, min_thr or max_thr is not equal to the value in np,
    //then should not read data from np. 
////////////////////AAAAAAAAAAAAAAAAAAAAAAAAA//////////////////////
    max_idx      = np->coverage_list[cvg_idx]->scan_list->getSize() - 2;
    np_num_level = np->coverage_list[cvg_idx]->scan_list->getSize() - 1;
    np_min_thr   = 10*log(np->coverage_list[cvg_idx]->level_list[0])/log(10.0);
    np_max_thr   = 10*log(np->coverage_list[cvg_idx]->level_list[max_idx])/log(10.0);

    if( man_set && (num_level==np_num_level) && (min_thr<np_min_thr+PRECISION) && (max_thr<np_max_thr+PRECISION)
        && (min_thr>np_min_thr-PRECISION) && (max_thr>np_max_thr-PRECISION) ) {
        for( i=0; i<num_level; i++ ) {
            QString str2 = str2.number( 10*log(np->coverage_list[cvg_idx]->level_list[i])/log(10.0) );
            level_thr_val[i]->setText(str2);
        }
    } else {
        for( i=0; i<num_level - 1; i++ ) {
            QString str2 = str2.number( min_thr + i*space );
            level_thr_val[i]->setText(str2);
        }

        if ( num_level == 1 ) {
            QString str2 = str2.number( min_thr );
            level_thr_val[0]->setText(str2);
        }

        if ( num_level >=2 ) {
            QString str2 = str2.number( max_thr );
            level_thr_val[num_level-1]->setText(str2);
        }
    }
////////////////////AAAAAAAAAAAAAAAAAAAAAAAAA//////////////////////


    setFixedWidth( 300 );
    setFixedHeight( 90+num_level*30 );
}

LevelThresholdPage::~LevelThresholdPage()
{
}

/******************************************************************************************/
/**** FUNCTION: SirLayerPage::SirLayerPage                                             ****/
/******************************************************************************************/
SirLayerPage::SirLayerPage(NetworkClass *np, int cvg_idx, QWidget* parent, char* name) 
    : CvgAnalysisPage(np, parent, name)
{
    cvg_analysis_page_gridlayout = new Q3GridLayout(this, 3, 2, 5, 5, "sirlayer_cvg_analysis_page_gridlayout") ;

    //sir layer threshold and max sir layer
    sir_layer_thr_lbl = new QLabel(tr("SIR &Threshold"), this, "sir_layer_thr_lbl");
    sir_layer_thr_lbl->setMaximumWidth(120);
    cvg_analysis_page_gridlayout->addWidget(sir_layer_thr_lbl, 0, 0, Qt::AlignHCenter);

    sir_layer_thr_val = new QLineEdit(this,"sir_layer_thr_val");
    sir_layer_thr_val->setMaximumSize( QSize( 120, 32767 ) );
    sir_layer_thr_lbl->setBuddy(sir_layer_thr_val);
    cvg_analysis_page_gridlayout->addWidget(sir_layer_thr_val, 0, 1, Qt::AlignHCenter);

    max_sir_layer_lbl = new QLabel(tr("&Max Layer"), this, "max_sir_layer_lbl");
    max_sir_layer_lbl->setMaximumWidth(120);
    cvg_analysis_page_gridlayout->addWidget(max_sir_layer_lbl, 1, 0, Qt::AlignHCenter);

    max_sir_layer_spinbox = new QSpinBox( 1, 40, 1, this, "max_sir_layer_spinbox");
    max_sir_layer_spinbox->setMaximumWidth( 120 );
    max_sir_layer_spinbox->setMinimumWidth( 120 );
    max_sir_layer_lbl->setBuddy(max_sir_layer_spinbox);
    cvg_analysis_page_gridlayout->addWidget(max_sir_layer_spinbox, 1, 1, Qt::AlignHCenter);

    //run button
    run_btn = new QPushButton(tr("&Run"), this, "run_btn");
    run_btn->setMaximumWidth(80);
    cvg_analysis_page_gridlayout->addWidget(run_btn, 2, 1, Qt::AlignHCenter);

    //fractional area label and lineedit
    scan_fractional_area_lbl = new QLabel(tr("Scan Fraction Area"), this, "scan_fractional_area_lbl");
    scan_fractional_area_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addWidget( scan_fractional_area_lbl, 3, 0, Qt::AlignHCenter );

    scan_fractional_area_val = new QLineEdit(this, "scan_fractional_area_val");
    scan_fractional_area_val->setMaximumSize( QSize( 120, 32767 ) );
    scan_fractional_area_lbl->setBuddy( scan_fractional_area_val );
    cvg_analysis_page_gridlayout->addWidget( scan_fractional_area_val, 3, 1, Qt::AlignHCenter );

    //initial sample resolution label and lineedit
    init_sample_res_lbl = new QLabel(tr("Initial Sample resolution"), this, "init_sample_res_lbl");
    init_sample_res_lbl->setMaximumWidth( 150 );
    cvg_analysis_page_gridlayout->addWidget( init_sample_res_lbl, 4, 0, Qt::AlignHCenter );

    init_sample_res_val = new QLineEdit(this, "init_sample_res_val");
    init_sample_res_val->setMaximumSize( QSize( 120, 32767 ) );
    init_sample_res_lbl->setBuddy( init_sample_res_val );
    cvg_analysis_page_gridlayout->addWidget( init_sample_res_val, 4, 1, Qt::AlignHCenter );

    //set value
    QString str1 = str1.number( 10*log(np->coverage_list[cvg_idx]->threshold)/log(10.0) );
    QString str2 = str2.number( np->coverage_list[cvg_idx]->scan_fractional_area );
    QString str3 = str3.number( np->coverage_list[cvg_idx]->init_sample_res );

    sir_layer_thr_val->setText(str1);
    scan_fractional_area_val->setText( str2 );
    init_sample_res_val->setText( str3 );
    max_sir_layer_spinbox->setValue( np->coverage_list[cvg_idx]->scan_list->getSize()-1 );

    connect(run_btn, SIGNAL(clicked()), this, SLOT(run_btn_clicked()));

    advShown = true;
    adv_btn_clicked();

    resize( QSize(673, 365).expandedTo(minimumSizeHint()) );
}

SirLayerPage::~SirLayerPage() 
{

}


/******************************************************************************************/
/**** FUNCTION: SirLayerPage::run_btn_clicked                                          ****/
/******************************************************************************************/
void SirLayerPage::run_btn_clicked()
{
    int     max_sir_layer = max_sir_layer_spinbox->value();
    int     init_sample_res = init_sample_res_val->text().toInt();
    double  sir_thr = sir_layer_thr_val->text().toInt();
    double  scan_fractional_area = scan_fractional_area_val->text().toDouble();

    sprintf(np->line_buf, "set_coverage_analysis -name %s -threshold_db %f -num_scan_type %d",
            page_name, sir_thr, max_sir_layer+1);
    np->process_command(np->line_buf);

    sprintf(np->line_buf, "set_coverage_analysis -name %s -scan_fractional_area %f -init_sample_res %d",
            page_name, scan_fractional_area, init_sample_res);
    np->process_command(np->line_buf);

    sprintf(np->line_buf, "run_coverage -name %s", page_name);
    np->process_command(np->line_buf);
}
