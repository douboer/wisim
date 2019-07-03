/****************************************************************************
** prop_mod_widget.cpp
****************************************************************************/

#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvariant.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "cconst.h"
#include "WiSim.h"
#include "prop_model.h"
#include "prop_mod_widget.h"

PropModWidget::PropModWidget( NetworkClass* param_np, int prop_model_idx, QWidget* parent, const char* name )
             : QWidget( parent, name )
{
    if ( !name ) {
        setName( "PropModWidget" );
    } else {
        setName( name );
    }

    np = param_np;
    if ( prop_model_idx>=0 ) {
        if ( np->prop_model_list[prop_model_idx]->type() == CConst::PropExpo ) {
            expo_pm = (ExpoPropModelClass *) np->prop_model_list[prop_model_idx];
            inflexion_num = 1;
        } else if ( np->prop_model_list[prop_model_idx]->type() == CConst::PropSegment ) {
            seg_pm = (SegmentPropModelClass *) np->prop_model_list[prop_model_idx];
            inflexion_num = seg_pm->num_inflexion;
        } else {
            inflexion_num = 1;
        }
    } else {
        inflexion_num = 1;
    }

    prop_mod_widgetLayout = new Q3GridLayout( this, 1, 1, 11, 6, "prop_mod_widgetLayout"); 
    prop_mod_widgetLayout->setAlignment( Qt::AlignTop );

    // Type of Propagation Model
    type_buttongroup = new Q3ButtonGroup( this, "type_buttongroup" );
    type_buttongroup->setColumnLayout(0, Qt::Vertical );
    type_buttongroup->layout()->setSpacing( 6 );
    type_buttongroup->layout()->setMargin( 11 );
    type_buttongroupLayout = new Q3GridLayout( type_buttongroup->layout() );
    type_buttongroupLayout->setAlignment( Qt::AlignTop );

    // Exponential Propagation Model
    expo_type_radiobutton = new QRadioButton( type_buttongroup, "expo_type_radiobutton" );
    type_buttongroup->insert( expo_type_radiobutton, 0 );
    expo_type_radiobutton->setEnabled( true );
    expo_type_radiobutton->setMaximumSize( QSize( 300, 32767 ) );
    type_buttongroupLayout->addWidget( expo_type_radiobutton, 0, 0 );

    // Terrain Popagation Model
    segment_type_radiobutton = new QRadioButton( type_buttongroup, "terrain_type_radiobutton" );
    type_buttongroup->insert( segment_type_radiobutton, 1);
    segment_type_radiobutton->setMaximumSize( QSize( 300, 32767 ) );
    type_buttongroupLayout->addWidget( segment_type_radiobutton, 1, 0 );

    prop_mod_widgetLayout->addWidget( type_buttongroup, 0, 0, Qt::AlignTop );

    // Parameters of Propagation Model
    param_buttongroup = new Q3ButtonGroup( this, "param_buttongroup" );
    param_buttongroup->setColumnLayout(0, Qt::Vertical );
    param_buttongroup->layout()->setSpacing( 6 );
    param_buttongroup->layout()->setMargin( 11 );
    param_buttongroupLayout = new Q3GridLayout( param_buttongroup->layout() );
    param_buttongroupLayout->setAlignment( Qt::AlignTop );

    // Parameters of Exponential Propagation Model
    expo_lbl = new QLabel( param_buttongroup, "expo_lbl" );
    param_buttongroupLayout->addWidget( expo_lbl, 0, 0, Qt::AlignCenter );

    expo_coefficient_lbl = new QLabel( param_buttongroup, "expo_coefficient_lbl" );
    param_buttongroupLayout->addWidget( expo_coefficient_lbl, 1, 0, Qt::AlignCenter );

    expo_val = new QLineEdit( param_buttongroup, "expo_val" );
    expo_val->setMaximumSize( QSize( 120, 32767 ) );
    param_buttongroupLayout->addWidget( expo_val, 0, 1, Qt::AlignCenter );

    expo_coefficient_val = new QLineEdit( param_buttongroup, "expo_coefficient_val" );
    expo_coefficient_val->setMaximumSize( QSize( 120, 32767 ) );
    param_buttongroupLayout->addWidget( expo_coefficient_val, 1, 1, Qt::AlignCenter );

    // Parameters of Segment Propagation Model
    inflexion_number_lbl = new QLabel( param_buttongroup, "inflexion_number_lbl" );
    param_buttongroupLayout->addWidget( inflexion_number_lbl, 2, 0, Qt::AlignCenter );

    inflexion_number_val = new QLineEdit( param_buttongroup, "inflexion_number_val" );
    inflexion_number_val->setMaximumSize( QSize( 120, 32767 ) );
    param_buttongroupLayout->addWidget( inflexion_number_val, 2, 1, Qt::AlignCenter );

    apply_btn = new QPushButton( param_buttongroup, "apply_btn" );
    apply_btn->setMaximumWidth( 120 );
    param_buttongroupLayout->addWidget( apply_btn, 2, 2, Qt::AlignCenter );

    start_slope_lbl = new QLabel( param_buttongroup, "start_slope_lbl" );
    param_buttongroupLayout->addWidget( start_slope_lbl, 3, 0, Qt::AlignCenter );

    start_slope_val = new QLineEdit( param_buttongroup, "start_slope_val" );
    start_slope_val->setMaximumSize( QSize( 120, 32767 ) );
    param_buttongroupLayout->addWidget( start_slope_val, 3, 1, Qt::AlignCenter );

    final_slope_lbl = new QLabel( param_buttongroup, "final_slope_lbl" );
    param_buttongroupLayout->addWidget( final_slope_lbl, 3, 2, Qt::AlignCenter );

    final_slope_val = new QLineEdit( param_buttongroup, "final_slope_val" );
    final_slope_val->setMaximumSize( QSize( 120, 32767 ) );
    param_buttongroupLayout->addWidget( final_slope_val, 3, 3, Qt::AlignCenter );

    inflexion_x_lbl = ( QLabel ** ) malloc(inflexion_num*sizeof(QLabel *));
    inflexion_x_val = ( QLineEdit ** ) malloc(inflexion_num*sizeof(QLineEdit *));
    inflexion_y_lbl = ( QLabel ** ) malloc(inflexion_num*sizeof(QLabel *));
    inflexion_y_val = ( QLineEdit ** ) malloc(inflexion_num*sizeof(QLineEdit *));

    for( int idx=0; idx<inflexion_num; idx++ ) {
        inflexion_x_lbl[idx] = new QLabel( param_buttongroup, "inflexion_val_x_lbl");
        param_buttongroupLayout->addWidget( inflexion_x_lbl[idx], idx+4, 0, Qt::AlignCenter );

        inflexion_x_val[idx] = new QLineEdit( param_buttongroup, "inflexion_val_x_lbl");
        inflexion_x_val[idx]->setMaximumSize( QSize( 120, 32767 ) );
        param_buttongroupLayout->addWidget( inflexion_x_val[idx], idx+4, 1, Qt::AlignCenter );

        inflexion_y_lbl[idx] = new QLabel( param_buttongroup, "inflexion_val_y_lbl");
        param_buttongroupLayout->addWidget( inflexion_y_lbl[idx], idx+4, 2, Qt::AlignCenter );

        inflexion_y_val[idx] = new QLineEdit( param_buttongroup, "inflexion_val_y_lbl");
        inflexion_y_val[idx]->setMaximumSize( QSize( 120, 32767 ) );
        param_buttongroupLayout->addWidget( inflexion_y_val[idx], idx+4, 3, Qt::AlignCenter );            

        connect(inflexion_x_val[idx], SIGNAL(textChanged(const QString&)), this, SLOT(lineeditChanged(const QString&)));
        connect(inflexion_y_val[idx], SIGNAL(textChanged(const QString&)), this, SLOT(lineeditChanged(const QString&)));
    }
    prop_mod_widgetLayout->addWidget( param_buttongroup, 2, 0, Qt::AlignTop );

    //input the display data
    if ( prop_model_idx == -1 ) {
        expo_type_radiobutton->setChecked( TRUE );
        segment_type_radiobutton->setChecked( FALSE );
    } else if ( np->prop_model_list[prop_model_idx]->type() == CConst::PropExpo ) {
        expo_type_radiobutton->setChecked( TRUE );
        segment_type_radiobutton->setChecked( FALSE );

        expo_pm = (ExpoPropModelClass *) np->prop_model_list[prop_model_idx];
        expo_val->setText( QString("%1").arg(expo_pm->exponent) );
        expo_coefficient_val->setText( QString("%1").arg(expo_pm->coefficient) );
    } else if ( np->prop_model_list[prop_model_idx]->type() == CConst::PropSegment ) {
        expo_type_radiobutton->setChecked( FALSE );
        segment_type_radiobutton->setChecked( TRUE );

        inflexion_number_val->setText(QString("%1").arg(seg_pm->num_inflexion));
        start_slope_val->setText(QString("%1").arg(seg_pm->start_slope));
        final_slope_val->setText(QString("%1").arg(seg_pm->final_slope));
        for( int idx=0; idx<inflexion_num; idx++ ) {
            inflexion_x_val[idx]->setText(QString("%1").arg(seg_pm->x[idx]));
            inflexion_y_val[idx]->setText(QString("%1").arg(seg_pm->y[idx]));
        }
    } else {
        expo_type_radiobutton->setChecked( TRUE );
        segment_type_radiobutton->setChecked( FALSE );
    }

    // Signals and slots connections
    connect( type_buttongroup, SIGNAL( clicked(int) ), this, SLOT(type_radiobutton_clicked(int)) );
    connect( expo_type_radiobutton, SIGNAL( stateChanged(int) ), this, SLOT(radioChanged(int)) );
    connect( segment_type_radiobutton, SIGNAL( stateChanged(int) ), this, SLOT(radioChanged(int)) );
    connect( expo_val, SIGNAL( textChanged(const QString&) ), this, SLOT(lineeditChanged(const QString &)) );
    connect( expo_coefficient_val, SIGNAL( textChanged(const QString&) ),this, SLOT(lineeditChanged(const QString&)) );
    connect( start_slope_val, SIGNAL(textChanged(const QString &)),this, SLOT(lineeditChanged(const QString&)) );
    connect( final_slope_val, SIGNAL(textChanged(const QString &)),this, SLOT(lineeditChanged(const QString&)) );
    connect( apply_btn, SIGNAL(clicked()), this, SLOT(apply_btn_clicked()));

    // Control the widget display property
    if ( expo_type_radiobutton->isChecked() ) {
        expo_lbl->show();
        expo_coefficient_lbl->show();
        expo_val->show();
        expo_val->setDisabled( false );
        expo_coefficient_val->show();
        expo_coefficient_val->setDisabled( false );
    } else {
        expo_lbl->hide();
        expo_coefficient_lbl->hide();
        expo_val->hide();
        expo_val->setDisabled( true );
        expo_coefficient_val->hide();
        expo_coefficient_val->setDisabled( true );
    }

    if ( segment_type_radiobutton->isChecked() ) {
        apply_btn->setDisabled( false );
        apply_btn->show();
        inflexion_number_val->setDisabled( false );
        inflexion_number_val->show();
        start_slope_val->setDisabled( false );
        start_slope_val->show();
        final_slope_val->setDisabled( false );
        final_slope_val->show();
        for( int idx=0; idx<inflexion_num; idx++ ) {
            inflexion_x_val[idx]->setDisabled( false );
            inflexion_x_val[idx]->show();
            inflexion_y_val[idx]->setDisabled( false );
            inflexion_y_val[idx]->show();
            inflexion_x_lbl[idx]->show();
            inflexion_y_lbl[idx]->show();
        }
        inflexion_number_lbl->show();
        start_slope_lbl->show();
        final_slope_lbl->show();
    } else {
        apply_btn->setDisabled( true );
        apply_btn->hide();
        inflexion_number_val->setDisabled( true );
        inflexion_number_val->hide();
        start_slope_val->setDisabled( true );
        start_slope_val->hide();
        final_slope_val->setDisabled( true );
        final_slope_val->hide();
        for( int idx=0; idx<inflexion_num; idx++ ) {
            inflexion_x_val[idx]->setDisabled( true );
            inflexion_x_val[idx]->hide();
            inflexion_y_val[idx]->setDisabled( true );
            inflexion_y_val[idx]->hide();
            inflexion_x_lbl[idx]->hide();
            inflexion_y_lbl[idx]->hide();
        }
        inflexion_number_lbl->hide();
        start_slope_lbl->hide();
        final_slope_lbl->hide();
    }

    languageChange();
    resize( QSize(673, 365).expandedTo(minimumSizeHint()) );
}

PropModWidget::~PropModWidget()
{
}

void PropModWidget::languageChange()
{
    setCaption( tr( "Propagation Model Widget" ) );
    type_buttongroup->setTitle( tr( "Type of Propagation Models" ) );
    expo_type_radiobutton->setText( tr( "Exponential Propagation Model" ) );
    segment_type_radiobutton->setText( tr( "Segment Propagation Model" ) );

    param_buttongroup->setTitle( tr( "Parameters of Propagation Models" ) );
    expo_lbl->setText( tr( "Exponent" ) );
    expo_coefficient_lbl->setText( tr( "Coefficient" ) );

    inflexion_number_lbl->setText( tr( "Number of Inflexion" ) );
    apply_btn->setText( tr("&Apply") );
    start_slope_lbl->setText( tr( "Start Slope" ) );
    final_slope_lbl->setText( tr( "Final Slope" ) );

    for ( int idx=0; idx<inflexion_num; idx++ ) {
        inflexion_x_lbl[idx]->setText( tr(QString("X Value of point %1").arg(idx) ) );
        inflexion_y_lbl[idx]->setText( tr(QString("Y Value of point %1").arg(idx) ) );
    }
}

void PropModWidget::apply_btn_clicked()
{
    int idx;
    int old_inflexion   = inflexion_num;
    inflexion_num   = atoi( inflexion_number_val->text() );

    for( idx=0; idx<old_inflexion; idx++ ) {
        disconnect(inflexion_x_val[idx],SIGNAL(textChanged(const QString&)), this, SLOT(lineeditChanged(const QString&)));
        disconnect(inflexion_y_val[idx],SIGNAL(textChanged(const QString&)), this, SLOT(lineeditChanged(const QString&)));
        inflexion_x_lbl[idx]->hide();
        inflexion_y_lbl[idx]->hide();
        inflexion_x_val[idx]->hide();
        inflexion_y_val[idx]->hide();
        delete inflexion_x_lbl[idx];
        delete inflexion_y_lbl[idx];
        delete inflexion_x_val[idx];
        delete inflexion_y_val[idx];
    }
    free(inflexion_x_lbl);
    free(inflexion_y_lbl);
    free(inflexion_x_val);
    free(inflexion_y_val);

    inflexion_x_lbl = NULL;
    inflexion_y_lbl = NULL;
    inflexion_x_val = NULL;
    inflexion_y_val = NULL;

    inflexion_x_lbl = ( QLabel ** ) malloc(inflexion_num*sizeof(QLabel *));
    inflexion_y_lbl = ( QLabel ** ) malloc(inflexion_num*sizeof(QLabel *));
    inflexion_x_val = ( QLineEdit ** ) malloc(inflexion_num*sizeof(QLineEdit *));
    inflexion_y_val = ( QLineEdit ** ) malloc(inflexion_num*sizeof(QLineEdit *));

    for( idx=0; idx<inflexion_num; idx++ ) {
        inflexion_x_lbl[idx] = new QLabel( param_buttongroup, "inflexion_val_x_lbl");
        inflexion_x_lbl[idx]->show();
        param_buttongroupLayout->addWidget( inflexion_x_lbl[idx], idx+4, 0, Qt::AlignCenter );

        inflexion_x_val[idx] = new QLineEdit( param_buttongroup, "inflexion_val_x_lbl");
        inflexion_x_val[idx]->setMaximumSize( QSize( 120, 32767 ) );
        inflexion_x_val[idx]->show();
        param_buttongroupLayout->addWidget( inflexion_x_val[idx], idx+4, 1, Qt::AlignCenter );

        inflexion_y_lbl[idx] = new QLabel( param_buttongroup, "inflexion_val_y_lbl");
        inflexion_y_lbl[idx]->show();
        param_buttongroupLayout->addWidget( inflexion_y_lbl[idx], idx+4, 2, Qt::AlignCenter );

        inflexion_y_val[idx] = new QLineEdit( param_buttongroup, "inflexion_val_y_lbl");
        inflexion_y_val[idx]->setMaximumSize( QSize( 120, 32767 ) );
        inflexion_y_val[idx]->show();
        param_buttongroupLayout->addWidget( inflexion_y_val[idx], idx+4, 3, Qt::AlignCenter );            

        connect(inflexion_x_val[idx], SIGNAL(textChanged(const QString&)), SLOT(lineeditChanged(const QString&)));
        connect(inflexion_y_val[idx], SIGNAL(textChanged(const QString&)), SLOT(lineeditChanged(const QString&)));
    }

    for( idx=0; (idx<old_inflexion)&&(idx<inflexion_num); idx++ ) {
        inflexion_x_val[idx]->setText(QString("%1").arg(seg_pm->x[idx]));
        inflexion_y_val[idx]->setText(QString("%1").arg(seg_pm->y[idx]));
    }

    for ( idx=0; idx<inflexion_num; idx++ ) {
        inflexion_x_lbl[idx]->setText( tr(QString("X Value of point %1").arg(idx) ) );
        inflexion_y_lbl[idx]->setText( tr(QString("Y Value of point %1").arg(idx) ) );
    }
}

void PropModWidget::type_radiobutton_clicked(int i)
{
    int idx;

    switch( i ) {
        case 0:
            expo_type_radiobutton->setChecked( true );
            segment_type_radiobutton->setChecked( false );

            expo_lbl->show();
            expo_coefficient_lbl->show();
            expo_val->show();
            expo_val->setDisabled( false );
            expo_coefficient_val->show();
            expo_coefficient_val->setDisabled( false );

            apply_btn->setDisabled( true );
            apply_btn->hide();
            inflexion_number_val->setDisabled( true );
            inflexion_number_val->hide();
            start_slope_val->setDisabled( true );
            start_slope_val->hide();
            final_slope_val->setDisabled( true );
            final_slope_val->hide();
            for( idx=0; idx<inflexion_num; idx++ ) {
                inflexion_x_val[idx]->setDisabled( true );
                inflexion_x_val[idx]->hide();
                inflexion_y_val[idx]->setDisabled( true );
                inflexion_y_val[idx]->hide();
                inflexion_x_lbl[idx]->hide();
                inflexion_y_lbl[idx]->hide();
            }
            inflexion_number_lbl->hide();
            start_slope_lbl->hide();
            final_slope_lbl->hide();
            break;
        default:
            expo_type_radiobutton->setChecked( false );
            segment_type_radiobutton->setChecked( true );

            expo_lbl->hide();
            expo_coefficient_lbl->hide();
            expo_val->hide();
            expo_val->setDisabled( true );
            expo_coefficient_val->hide();
            expo_coefficient_val->setDisabled( true );

            apply_btn->setDisabled( false );
            apply_btn->show();
            inflexion_number_val->setDisabled( false );
            inflexion_number_val->show();
            start_slope_val->setDisabled( false );
            start_slope_val->show();
            final_slope_val->setDisabled( false );
            final_slope_val->show();
            for( idx=0; idx<inflexion_num; idx++ ) {
                inflexion_x_val[idx]->setDisabled( false );
                inflexion_x_val[idx]->show();
                inflexion_y_val[idx]->setDisabled( false );
                inflexion_y_val[idx]->show();
                inflexion_x_lbl[idx]->show();
                inflexion_y_lbl[idx]->show();
            }
            inflexion_number_lbl->show();
            start_slope_lbl->show();
            final_slope_lbl->show();
            break;
    }
}

void PropModWidget::radioChanged( int )
{
    emit valueChanged(1);
}

void PropModWidget::lineeditChanged( const QString& )
{
    emit valueChanged(2);
}

