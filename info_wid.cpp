#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <q3table.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "info_wid.h"
#include "WiSim.h"
#include "prop_model.h"
#include "clutter_data_analysis.h"

/******************************************************************************************/
/**** FUNCTION: Virtual Functions                                                      ****/
/******************************************************************************************/
void        GenericInfoWidClass      :: clutter_view_btn_clicked()          { CORE_DUMP; return; }
void        GenericInfoWidClass      :: applyParamEdit(int)                 { CORE_DUMP; return; }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: SegViewTable::SegViewTable                                             ****/
/******************************************************************************************/
SegViewTable::SegViewTable( NetworkClass* np_param, QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    np = np_param;
    
    if ( !name )
	setName( "SegViewTable" );

    SegViewTableLayout = new Q3VBoxLayout( this, 11, 6, "SegViewTableLayout"); 

    table = new Q3Table( this, "table" );
    SegViewTableLayout->addWidget( table );

    layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 

    layout1->addStretch();

    ok_btn = new QPushButton( this );
    layout1->addWidget( ok_btn );
    ok_btn->setText( tr("&Ok") );

    layout1->addStretch();

    SegViewTableLayout->addLayout( layout1 );

    //signals and slots
    connect( ok_btn, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );

    resize( QSize(321, 348).expandedTo(minimumSizeHint()) );
}
/******************************************************************************************/
/**** FUNCTION: SegViewTable::~SegViewTable                                            ****/
/******************************************************************************************/
SegViewTable::~SegViewTable()
{

}
/******************************************************************************************/
/**** FUNCTION: SegViewTable::languageChange                                           ****/
/******************************************************************************************/
void SegViewTable::languageChange()
{

#if 0
    setCaption( tr( "Segment Points" ) );
    table->horizontalHeader()->setLabel( 0, tr( "X Coordinate" ) );
    table->horizontalHeader()->setLabel( 1, tr( "Y Coordinate" ) );
#endif
    ok_btn->setText( tr( "&Ok" ) );
    ok_btn->setAccel( QKeySequence( "Alt+O" ) );
}

void SegViewTable::ok_btn_clicked()
{
    hide();
}

/******************************************************************************************/
/**** FUNCTION: SegViewTable::set_num_row                                              ****/
/******************************************************************************************/
void SegViewTable::set_num_row(int num_row)
{
    int i, k, num_col;
    int prev_num_row = table->numRows();
    Q3Header *th;

    num_col = table->numCols();

    if (num_row < prev_num_row) {
        for ( i=prev_num_row-1; i>=num_row; i-- ) {
            table->removeRow(i);
        }
        table->setNumRows(num_row);
    } else if (num_row > prev_num_row) {
        th = table->verticalHeader();
        table->setNumRows(num_row);
        for( i=prev_num_row; i<=num_row-1; i++ ) {
            for (k=0; k<=num_col-1; k++) {
                table->setItem( i, k, new Q3TableItem(table, Q3TableItem::WhenCurrent, ""));
            }

            if (num_col == 1) {
                th->setLabel( i, QString("Clutter Type %1").arg(i));
            } else {
                th->setLabel( i, QString("Point %1").arg(i));
            }
        }
    }
    /**************************************************************************************/
}

/******************************************************************************************/
/**** FUNCTION: GenericInfoWidClass::GenericInfoWidClass                               ****/
/******************************************************************************************/
GenericInfoWidClass::GenericInfoWidClass( NetworkClass* np_param,  QWidget* parent, const char* name, Qt::WFlags fl )
    : QWidget( parent, name, fl ), np( np_param )
{
    if ( !name )
	setName( "Information Widget" );

    topLayout = new Q3GridLayout( this, 1, 1, 11, 6);

    //------------------------------------------------------------------------------------------
    prop_type_buttonGroup = new Q3ButtonGroup( this, "prop_type_buttonGroup" );
    prop_type_buttonGroup->setColumnLayout(0, Qt::Vertical );
    prop_type_buttonGroup->layout()->setSpacing( 15 );
    prop_type_buttonGroup->layout()->setMargin( 11 );

    prop_type_buttonGroupLayout = new Q3GridLayout( prop_type_buttonGroup->layout() );
    prop_type_buttonGroupLayout->setAlignment( Qt::AlignTop );

    type_textLabel = new QLabel( prop_type_buttonGroup, "type_textLabel" );
    type_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, type_textLabel->sizePolicy().hasHeightForWidth() ) );
    prop_type_buttonGroupLayout->addWidget( type_textLabel, 0, 0 );

    name_textLabel = new QLabel( prop_type_buttonGroup, "name_textLabel" );
    name_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, name_textLabel->sizePolicy().hasHeightForWidth() ) );
    prop_type_buttonGroupLayout->addWidget( name_textLabel, 1, 0 );

    type_lineEdit = new QLineEdit( prop_type_buttonGroup, "type_lineEdit" );
    type_lineEdit->setEnabled( FALSE );
    type_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, type_lineEdit->sizePolicy().hasHeightForWidth() ) );
    prop_type_buttonGroupLayout->addWidget( type_lineEdit, 0, 1 );

    name_lineEdit = new QLineEdit( prop_type_buttonGroup, "name_lineEdit" );
    name_lineEdit->setEnabled( FALSE );
    name_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, name_lineEdit->sizePolicy().hasHeightForWidth() ) );
    prop_type_buttonGroupLayout->addWidget( name_lineEdit, 1, 1 );

    topLayout->addWidget( prop_type_buttonGroup, 0, 0 );

    //-------------------------------------------------------------------------
    param_buttonGroup = new Q3ButtonGroup( this, "param_buttonGroup" );
    param_buttonGroup->setColumnLayout(0, Qt::Vertical );
    param_buttonGroup->layout()->setSpacing( 15 );
    param_buttonGroup->layout()->setMargin( 11 );

    param_buttonGroupLayout = new Q3GridLayout( param_buttonGroup->layout() );
    param_buttonGroupLayout->setAlignment( Qt::AlignTop );


#if 0
    num_clutter_textLabel = new QLabel( param_buttonGroup, "num_clutter_textLabel" );
    num_clutter_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, 
                num_clutter_textLabel->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( num_clutter_textLabel, 0, 0 );

    num_clutter_lineEdit = new QLineEdit( param_buttonGroup, "num_clutter_lineEdit" );
    num_clutter_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, 
                num_clutter_lineEdit->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( num_clutter_lineEdit, 0, 1 );



    num_pt_textLabel = new QLabel( param_buttonGroup, "num_pt_textLabel" );
    num_pt_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 5, 0, num_pt_textLabel->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( num_pt_textLabel, 1, 0 );

    num_pt_lineEdit = new QLineEdit( param_buttonGroup, "num_pt_lineEdit" );
    num_pt_lineEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 6, 0, num_pt_lineEdit->sizePolicy().hasHeightForWidth() ) );
    param_buttonGroupLayout->addWidget( num_pt_lineEdit, 1, 1 );

    start_slope_textLabel = new QLabel( param_buttonGroup);
    param_buttonGroupLayout->addWidget( start_slope_textLabel, 2, 0 );

    start_slope_lineEdit = new QLineEdit( param_buttonGroup);
    param_buttonGroupLayout->addWidget( start_slope_lineEdit, 2, 1 );

    final_slope_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(final_slope_lineEdit, 3, 1 );

    final_slope_textLabel = new QLabel( param_buttonGroup, "final_slope_textLabel" );
    param_buttonGroupLayout->addWidget( final_slope_textLabel, 3, 0 );
#endif

    topLayout->addWidget( param_buttonGroup, 1, 0 );

#if 0
    //-------------------------------------------------------------------------
    view_buttonGroup = new Q3ButtonGroup( this, "view_buttonGroup" );
    view_buttonGroup->setColumnLayout(0, Qt::Vertical );
    view_buttonGroup->layout()->setSpacing( 15 );
    view_buttonGroup->layout()->setMargin( 11 );

    view_buttonGroupLayout = new Q3GridLayout( view_buttonGroup->layout() );
    view_buttonGroupLayout->setAlignment( Qt::AlignTop );

    view_textLabel = new QLabel( view_buttonGroup, "view_textLabel" );
    view_buttonGroupLayout->addWidget( view_textLabel, 1, 0 );

    view_pushButton = new QPushButton( view_buttonGroup, "view_pushButton" );
    view_pushButton->setMaximumSize( QSize( 120, 32767 ) );
    view_buttonGroupLayout->addWidget( view_pushButton, 1, 1 );


    clutter_view_textLabel = new QLabel( view_buttonGroup, "clutter_view_textLabel" );
    view_buttonGroupLayout->addWidget( clutter_view_textLabel, 0, 0 );

    clutter_view_pushButton = new QPushButton( view_buttonGroup, "clutter_view_pushButton" );
    clutter_view_pushButton->setMaximumSize( QSize( 120, 32767 ) );
    view_buttonGroupLayout->addWidget( clutter_view_pushButton, 0, 1 );


    topLayout->addWidget( view_buttonGroup, 2, 0 );
#endif

    //initial
    //num_pt_lineEdit->setReadOnly( true );
    //start_slope_lineEdit->setReadOnly( true );
    //final_slope_lineEdit->setReadOnly( true );

    //connect

    //-------------------------------------------------------------------------
    resize( QSize(509, 334).expandedTo(minimumSizeHint()) );

    // tab order
    // setTabOrder( num_clutter_lineEdit, num_pt_lineEdit );
    // setTabOrder( num_pt_lineEdit, start_slope_lineEdit );
    // setTabOrder( start_slope_lineEdit, final_slope_lineEdit );
    // setTabOrder( final_slope_lineEdit, view_pushButton );
    // setTabOrder( view_pushButton, clutter_view_pushButton );
}
/******************************************************************************************/
/**** FUNCTION: GenericInfoWidClass::~GenericInfoWidClass                              ****/
/******************************************************************************************/
GenericInfoWidClass::~GenericInfoWidClass()
{
}
/******************************************************************************************/
/**** FUNCTION: GenericInfoWidClass::languageChange                                    ****/
/******************************************************************************************/
void GenericInfoWidClass::languageChange()
{
    prop_type_buttonGroup->setTitle( tr( "Propagation Model Type" ) );
    type_textLabel->setText( tr( "Type" ) );
    name_textLabel->setText( tr( "Name" ) );

    param_buttonGroup->setTitle( tr( "Propagation Model Parameters" ) );
}
/******************************************************************************************/
/**** FUNCTION: ExponentialInfoWidClass::ExponentialInfoWidClass                       ****/
/******************************************************************************************/
ExponentialInfoWidClass::ExponentialInfoWidClass( NetworkClass* np_param, QWidget* parent, const char* name, Qt::WFlags fl )
    : GenericInfoWidClass( np_param, parent, name, fl )
{

    /**************************************************************************************/
    /**** Parameter Section                                                            ****/
    /**************************************************************************************/
    exponent_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( exponent_textLabel, 0, 0 );

    exponent_lineEdit = new QLineEdit( param_buttonGroup );
    param_buttonGroupLayout->addWidget( exponent_lineEdit, 0, 1 );
    connect(exponent_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));

    coefficient_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( coefficient_textLabel, 1, 0 );

    coefficient_lineEdit = new QLineEdit( param_buttonGroup );
    param_buttonGroupLayout->addWidget( coefficient_lineEdit, 1, 1 );
    connect(coefficient_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    /**************************************************************************************/

    languageChange();
}
/******************************************************************************************/
/**** FUNCTION: ExponentialInfoWidClass::~ExponentialInfoWidClass                      ****/
/******************************************************************************************/
ExponentialInfoWidClass::~ExponentialInfoWidClass()
{
}
/******************************************************************************************/
/**** FUNCTION: ExponentialInfoWidClass::languageChange                                ****/
/******************************************************************************************/
void ExponentialInfoWidClass::languageChange()
{
    GenericInfoWidClass::languageChange();

    exponent_textLabel->setText( tr( "Exponent" ) );
    coefficient_textLabel->setText( tr( "Coefficient" ) );
}
/******************************************************************************************/
/**** FUNCTION: ExponentialInfoWidClass::setParam                                      ****/
/******************************************************************************************/
void ExponentialInfoWidClass::setParam( ExpoPropModelClass *epm )
{
    blockSignals(true);

           type_lineEdit->setText( tr("exponential") );
           name_lineEdit->setText( epm->get_strid() );

       exponent_lineEdit->setText( QString("%1").arg(epm->exponent) );
    coefficient_lineEdit->setText( QString("%1").arg(epm->coefficient) );

    blockSignals(false);
}
/******************************************************************************************/
/**** FUNCTION: ExponentialInfoWidClass::applyParamEdit                                ****/
/******************************************************************************************/
void ExponentialInfoWidClass::applyParamEdit(int pm_idx)
{
    double dval;
    ExpoPropModelClass *pm = (ExpoPropModelClass *) np->prop_model_list[pm_idx];

    dval = exponent_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->exponent) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -exponent %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }

    dval = coefficient_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->coefficient) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -coefficient %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::SegmentInfoWidClass                               ****/
/******************************************************************************************/
SegmentInfoWidClass::SegmentInfoWidClass( NetworkClass* np_param, QWidget* parent, const char* name, Qt::WFlags fl )
    : GenericInfoWidClass( np_param, parent, name, fl )
{

    /**************************************************************************************/
    /**** Parameter Section                                                            ****/
    /**************************************************************************************/
    num_clutter_textLabel = new QLabel( param_buttonGroup, "num_clutter_textLabel" );
    param_buttonGroupLayout->addWidget( num_clutter_textLabel, 0, 0 );

    num_clutter_spinBox = new QSpinBox( param_buttonGroup);
    param_buttonGroupLayout->addWidget( num_clutter_spinBox, 0, 1 );
    connect(num_clutter_spinBox, SIGNAL(valueChanged(int)), SIGNAL(modified()));

    num_pt_textLabel = new QLabel( param_buttonGroup, "num_pt_textLabel" );
    param_buttonGroupLayout->addWidget( num_pt_textLabel, 1, 0 );

    num_pt_spinBox = new QSpinBox(1, 100, 1, param_buttonGroup);
    param_buttonGroupLayout->addWidget( num_pt_spinBox, 1, 1 );
    connect(num_pt_spinBox, SIGNAL(valueChanged(int)), SIGNAL(modified()));

    start_slope_textLabel = new QLabel(param_buttonGroup);
    param_buttonGroupLayout->addWidget(start_slope_textLabel, 2, 0 );

    start_slope_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(start_slope_lineEdit, 2, 1 );
    connect(start_slope_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));

    final_slope_textLabel = new QLabel(param_buttonGroup);
    param_buttonGroupLayout->addWidget(final_slope_textLabel, 3, 0 );

    final_slope_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(final_slope_lineEdit, 3, 1 );
    connect(final_slope_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));

    useheight_checkBox= new QCheckBox(param_buttonGroup);
    param_buttonGroupLayout->addWidget( useheight_checkBox, 4, 0 );
    connect(useheight_checkBox, SIGNAL(toggled(bool)), SIGNAL(modified()));
    connect(useheight_checkBox, SIGNAL(toggled(bool)), this, SLOT(useheight_cb_clicked()));

    coeff_logh_textLabel = new QLabel(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_logh_textLabel, 5, 0 );

    coeff_logh_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_logh_lineEdit, 5, 1 );
    connect(coeff_logh_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));

    coeff_loghd_textLabel = new QLabel(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_loghd_textLabel, 6, 0 );

    coeff_loghd_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_loghd_lineEdit, 6, 1 );
    connect(coeff_loghd_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    /**************************************************************************************/

    /**************************************************************************************/
    /**** View Section                                                                 ****/
    /**************************************************************************************/
    view_buttonGroup = new Q3ButtonGroup( this, "view_buttonGroup" );
    view_buttonGroup->setColumnLayout(0, Qt::Vertical );
    view_buttonGroup->layout()->setSpacing( 15 );
    view_buttonGroup->layout()->setMargin( 11 );

    view_buttonGroupLayout = new Q3GridLayout( view_buttonGroup->layout() );
    view_buttonGroupLayout->setAlignment( Qt::AlignTop );

    view_textLabel = new QLabel( view_buttonGroup );
    view_buttonGroupLayout->addWidget( view_textLabel, 0, 0 );

    view_pushButton = new QPushButton( view_buttonGroup );
    view_pushButton->setMaximumSize( QSize( 120, 32767 ) );
    view_buttonGroupLayout->addWidget( view_pushButton, 0, 1 );

    clutter_view_textLabel = new QLabel( view_buttonGroup );
    view_buttonGroupLayout->addWidget( clutter_view_textLabel, 1, 0 );

    clutter_view_pushButton = new QPushButton( view_buttonGroup );
    clutter_view_pushButton->setMaximumSize( QSize( 120, 32767 ) );
    view_buttonGroupLayout->addWidget( clutter_view_pushButton, 1, 1 );

    topLayout->addWidget( view_buttonGroup, 2, 0 );
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Inflexion Point Table                                                        ****/
    /**************************************************************************************/
    view_table = new SegViewTable( np ); 
    view_table->setCaption( tr( "Segment Points" ) );
    view_table->table->setNumCols( 2 );
    view_table->table->horizontalHeader()->setLabel( 0, tr("LOG(distance)") );
    view_table->table->horizontalHeader()->setLabel( 1, tr("Path Loss (dB)") );

    connect(view_table->table, SIGNAL(valueChanged(int, int)), SIGNAL(modified()));

    connect(num_pt_spinBox, SIGNAL(valueChanged(int)), view_table, SLOT(set_num_row(int)));
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Clutter Coefficient Table                                                    ****/
    /**************************************************************************************/
    clutter_view_table = new SegViewTable( np ); 
    clutter_view_table->setCaption( tr( "clutter coeffecient" ) );
    clutter_view_table->table->setNumCols( 1 );
    clutter_view_table->table->horizontalHeader()->setLabel( 0, tr( "coefficient" ) );

    connect(clutter_view_table->table, SIGNAL(valueChanged(int, int)), SIGNAL(modified()));

    connect(num_clutter_spinBox, SIGNAL(valueChanged(int)), clutter_view_table, SLOT(set_num_row(int)));
    /**************************************************************************************/

    connect( view_pushButton, SIGNAL( clicked () ), this, SLOT( view_btn_clicked() ) );

    connect( clutter_view_pushButton, SIGNAL( clicked () ), this, SLOT( clutter_view_btn_clicked() ) );

    languageChange();
}
/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::~SegmentInfoWidClass                              ****/
/******************************************************************************************/
SegmentInfoWidClass::~SegmentInfoWidClass()
{
    if ( view_table )
        delete view_table;
    if ( clutter_view_table )
        delete clutter_view_table;
}

/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::languageChange                                    ****/
/******************************************************************************************/
void SegmentInfoWidClass::languageChange()
{
    GenericInfoWidClass::languageChange();

    num_clutter_textLabel->setText( tr( "Number of Clutter Types" ) );
    num_pt_textLabel->setText( tr( "Number of Inflexion Points" ) );
    start_slope_textLabel->setText( tr( "Initial Slope" ) );
    final_slope_textLabel->setText( tr( "Final Slope" ) );
    useheight_checkBox->setText( tr( "Use Height" ) );
    coeff_logh_textLabel->setText( tr( "Coefficient of log(h)" ) );
    coeff_loghd_textLabel->setText( tr( "Coefficient of log(h)*d" ) );

    view_buttonGroup->setTitle( QString::null );
    view_textLabel->setText( tr( "Inflexion points" ) );
    view_pushButton->setText( tr( "&View" ) );
    view_pushButton->setAccel( QKeySequence( "Alt+V" ) );
    clutter_view_textLabel->setText( tr( "Clutter Coeffecients" ) );
    clutter_view_pushButton->setText( tr( "V&iew" ) );
    clutter_view_pushButton->setAccel( QKeySequence( "Alt+i" ) );
}

/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::setParam                                          ****/
/******************************************************************************************/
void SegmentInfoWidClass::setParam( SegmentPropModelClass *spm )
{
    int i, num_row;
    Q3Header *th;

    blockSignals(true);

           type_lineEdit->setText( tr("segment") );
           name_lineEdit->setText( spm->get_strid() );

         num_pt_spinBox->setValue(spm->num_inflexion);
    num_clutter_spinBox->setValue(spm->num_clutter_type);
     start_slope_lineEdit->setText( QString("%1").arg(spm->start_slope) );
     final_slope_lineEdit->setText( QString("%1").arg(spm->final_slope) );

     useheight_checkBox->setChecked(spm->useheight);
     if (spm->useheight) {
         coeff_logh_lineEdit->setText( QString("%1").arg(spm->vec_k[0]) );
         coeff_loghd_lineEdit->setText( QString("%1").arg(spm->vec_k[1]) );
     } else {
         coeff_logh_lineEdit->setText( "0.0" );
         coeff_loghd_lineEdit->setText( "0.0" );
     }
     useheight_cb_clicked();

    /**************************************************************************************/
    /**** Inflexion Point Table                                                        ****/
    /**************************************************************************************/
    num_row = view_table->table->numRows();
    for ( i=num_row-1; i>=0; i-- ) {
        view_table->table->removeRow(i);
    }
    view_table->table->setNumRows(spm->num_inflexion);

    th = view_table->table->verticalHeader();
    for( i=0; i<=spm->num_inflexion-1; i++ ) {

        view_table->table->setItem( i, 0,
            new Q3TableItem( view_table->table, Q3TableItem::WhenCurrent, QString("%1").arg(spm->x[i])));

        view_table->table->setItem( i, 1,
            new Q3TableItem( view_table->table, Q3TableItem::WhenCurrent, QString("%1").arg(spm->y[i])));

        th->setLabel( i, QString("Point %1").arg(i));
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Clutter Coefficient Table                                                    ****/
    /**************************************************************************************/
    num_row = clutter_view_table->table->numRows();
    for ( i=num_row-1; i>=0; i-- ) {
        clutter_view_table->table->removeRow(i);
    }
    clutter_view_table->table->setNumRows(spm->num_clutter_type);

    th = clutter_view_table->table->verticalHeader();
    for (i=0; i<=spm->num_clutter_type-1; i++ ) {

        clutter_view_table->table->setItem( i, 0,
            new Q3TableItem( clutter_view_table->table, Q3TableItem::WhenCurrent, QString("%1").arg(spm->vec_k[2*spm->useheight+i])));

        th->setLabel( i, QString("Clutter Type %1").arg(i) );
    }
    /**************************************************************************************/

    blockSignals(false);
}
/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::applyParamEdit                                    ****/
/******************************************************************************************/
void SegmentInfoWidClass::applyParamEdit(int pm_idx)
{
    int ival, flag, new_useheight, i;
    double dval, xval, yval;
    SegmentPropModelClass *pm = (SegmentPropModelClass *) np->prop_model_list[pm_idx];

    dval = start_slope_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->start_slope) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -start_slope %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }

    dval = final_slope_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->final_slope) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -final_slope %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }

    flag = 0;
    ival = num_pt_spinBox->value();
    if (ival != pm->num_inflexion) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -num_pts %d", pm_idx, ival);
        np->process_command(np->line_buf);
    }

    for (i=0; i<=ival-1; i++) {
        xval = view_table->table->text(i, 0).stripWhiteSpace().toDouble();
        yval = view_table->table->text(i, 1).stripWhiteSpace().toDouble();
        if ( (flag) || (xval != pm->x[i]) || (yval != pm->y[i]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -pt_idx %d -x %f -y %f", pm_idx, i, xval, yval);
            np->process_command(np->line_buf);
        }
    }

    flag = 0;
    new_useheight = (useheight_checkBox->isChecked() ? 1 : 0);
    if (new_useheight != pm->useheight) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -useheight %d", pm_idx, new_useheight);
        np->process_command(np->line_buf);
    }

    if (new_useheight) {
        dval = coeff_logh_lineEdit->text().stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[0]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -coeff_logh %f", pm_idx, dval);
            np->process_command(np->line_buf);
        }
        dval = coeff_loghd_lineEdit->text().stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[1]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -coeff_loghd %f", pm_idx, dval);
            np->process_command(np->line_buf);
        }
    }

    flag = 0;
    ival = num_clutter_spinBox->value();
    if (ival != pm->num_clutter_type) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -num_clutter_type %d", pm_idx, ival);
        np->process_command(np->line_buf);
    }

    for (i=0; i<=ival-1; i++) {
        dval = clutter_view_table->table->text(i, 0).stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[2*pm->useheight+i]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -c_idx %d -c_coeff %f", pm_idx, i, dval);
            np->process_command(np->line_buf);
        }
    }
}
/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::useheight_cb_clicked                              ****/
/******************************************************************************************/
void SegmentInfoWidClass::useheight_cb_clicked()
{
    int checked = useheight_checkBox->isChecked();

    coeff_logh_textLabel->setEnabled(checked);
    coeff_logh_lineEdit->setEnabled(checked);
    coeff_loghd_textLabel->setEnabled(checked);
    coeff_loghd_lineEdit->setEnabled(checked);
}
/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::view_btn_clicked                                  ****/
/******************************************************************************************/
void SegmentInfoWidClass::view_btn_clicked()
{
    view_table->exec();
}
/******************************************************************************************/
/**** FUNCTION: SegmentInfoWidClass::clutter_view_btn_clicked                          ****/
/******************************************************************************************/
void SegmentInfoWidClass::clutter_view_btn_clicked()
{
    clutter_view_table->exec();
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::ClutterInfoWidClass                               ****/
/******************************************************************************************/
ClutterInfoWidClass::ClutterInfoWidClass( NetworkClass* np_param, QWidget* parent, const char* name, Qt::WFlags fl )
    : GenericInfoWidClass( np_param, parent, name, fl )
{
    /**************************************************************************************/
    /**** Parameter Section                                                            ****/
    /**************************************************************************************/
    clutter_res_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( clutter_res_textLabel, 0, 0 );
    clutter_res_lineEdit = new QLineEdit( param_buttonGroup );
    param_buttonGroupLayout->addWidget( clutter_res_lineEdit, 0, 1 );
    connect(clutter_res_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    clutter_res_lineEdit->setEnabled(false);

    startx_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( startx_textLabel, 1, 0 );
    startx_lineEdit = new QLineEdit( param_buttonGroup );
    param_buttonGroupLayout->addWidget( startx_lineEdit, 1, 1 );
    connect(startx_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    startx_lineEdit->setEnabled(false);

    starty_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( starty_textLabel, 2, 0 );
    starty_lineEdit = new QLineEdit( param_buttonGroup );
    param_buttonGroupLayout->addWidget( starty_lineEdit, 2, 1 );
    connect(starty_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    starty_lineEdit->setEnabled(false);

    numpts_x_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( numpts_x_textLabel, 3, 0 );
    numpts_x_spinBox = new QSpinBox( param_buttonGroup );
    param_buttonGroupLayout->addWidget( numpts_x_spinBox, 3, 1 );
    connect(numpts_x_spinBox, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    numpts_x_spinBox->setEnabled(false);

    numpts_y_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( numpts_y_textLabel, 4, 0 );
    numpts_y_spinBox = new QSpinBox( param_buttonGroup );
    param_buttonGroupLayout->addWidget( numpts_y_spinBox, 4, 1 );
    connect(numpts_y_spinBox, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    numpts_y_spinBox->setEnabled(false);

    useheight_checkBox= new QCheckBox(param_buttonGroup);
    param_buttonGroupLayout->addWidget( useheight_checkBox, 5, 0 );
    connect(useheight_checkBox, SIGNAL(toggled(bool)), SIGNAL(modified()));
    // connect(useheight_checkBox, SIGNAL(toggled(bool)), this, SLOT(useheight_cb_clicked()));
    useheight_checkBox->setEnabled(false);

    coeff_logh_textLabel = new QLabel(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_logh_textLabel, 6, 0 );
    coeff_logh_lineEdit = new QLineEdit(param_buttonGroup);
    param_buttonGroupLayout->addWidget(coeff_logh_lineEdit, 6, 1 );
    connect(coeff_logh_lineEdit, SIGNAL(textChanged(const QString&)), SIGNAL(modified()));
    coeff_logh_lineEdit->setEnabled(false);

    num_clutter_type_textLabel = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( num_clutter_type_textLabel, 7, 0 );
    num_clutter_type_textValue = new QLabel( param_buttonGroup );
    param_buttonGroupLayout->addWidget( num_clutter_type_textValue, 7, 1 );
    /**************************************************************************************/

    /**************************************************************************************/
    /**** View Section                                                                 ****/
    /**************************************************************************************/
    view_buttonGroup = new Q3ButtonGroup( this, "view_buttonGroup" );
    view_buttonGroup->setColumnLayout(0, Qt::Vertical );
    view_buttonGroup->layout()->setSpacing( 15 );
    view_buttonGroup->layout()->setMargin( 11 );

    view_buttonGroupLayout = new Q3GridLayout( view_buttonGroup->layout() );
    view_buttonGroupLayout->setAlignment( Qt::AlignTop );

    clutter_view_textLabel = new QLabel( view_buttonGroup, "clutter_view_textLabel" );
    view_buttonGroupLayout->addWidget( clutter_view_textLabel, 0, 0 );

    clutter_view_pushButton = new QPushButton( view_buttonGroup, "clutter_view_pushButton" );
    clutter_view_pushButton->setMaximumSize( QSize( 120, 32767 ) );
    view_buttonGroupLayout->addWidget( clutter_view_pushButton, 0, 1 );

    topLayout->addWidget( view_buttonGroup, 2, 0 );
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Clutter Coefficient Table                                                    ****/
    /**************************************************************************************/
    clutter_view_table = new SegViewTable( np ); 
    clutter_view_table->setCaption( tr( "clutter coeffecient" ) );
    clutter_view_table->table->setNumCols( 1 );
    clutter_view_table->table->horizontalHeader()->setLabel( 0, tr( "coefficient" ) );

    connect(clutter_view_table->table, SIGNAL(valueChanged(int, int)), SIGNAL(modified()));

    connect(numpts_x_spinBox, SIGNAL(valueChanged(int)), clutter_view_table, SLOT(set_num_row(int)));
    connect(numpts_y_spinBox, SIGNAL(valueChanged(int)), clutter_view_table, SLOT(set_num_row(int)));
    /**************************************************************************************/

    connect( clutter_view_pushButton, SIGNAL( clicked () ), this, SLOT( clutter_view_btn_clicked() ) );

    languageChange();
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::~ClutterInfoWidClass                              ****/
/******************************************************************************************/
ClutterInfoWidClass::~ClutterInfoWidClass()
{
    if ( clutter_view_table )
        delete clutter_view_table;
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::languageChange                                    ****/
/******************************************************************************************/
void ClutterInfoWidClass::languageChange()
{
    GenericInfoWidClass::languageChange();

    setCaption( tr("clutter") );

    clutter_res_textLabel->setText( tr( "Clutter / Geometry Resolution Ratio" ) );

    startx_textLabel->setText( tr( "Start X" ) );
    starty_textLabel->setText( tr( "Start Y" ) );
    numpts_x_textLabel->setText( tr( "Num Pts X" ) );
    numpts_y_textLabel->setText( tr( "Num Pts Y" ) );
    useheight_checkBox->setText( tr( "Use Height" ) );
    coeff_logh_textLabel->setText( tr( "Coefficient of log(h)" ) );
    num_clutter_type_textLabel->setText( tr( "Num Clutter Type" ) );

    view_buttonGroup->setTitle( QString::null );
    clutter_view_textLabel->setText( tr( "Clutter Coeffecients" ) );
    clutter_view_pushButton->setText( tr( "V&iew" ) );
    clutter_view_pushButton->setAccel( QKeySequence( "Alt+i" ) );
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::setParam                                          ****/
/******************************************************************************************/
void ClutterInfoWidClass::setParam( ClutterExpoLinearPropModelClass *cpm )
{
    int i, num_row;
    Q3Header *th;

    blockSignals(true);

                type_lineEdit->setText( "clutter" );
                name_lineEdit->setText( cpm->get_strid() );

         clutter_res_lineEdit->setText( QString("%1").arg(cpm->clutter_sim_res_ratio) );
              startx_lineEdit->setText( QString("%1").arg(cpm->offset_x) );
              starty_lineEdit->setText( QString("%1").arg(cpm->offset_y) );
             numpts_x_spinBox->setValue(cpm->npts_x);
             numpts_y_spinBox->setValue(cpm->npts_y);

    useheight_checkBox->setChecked(cpm->useheight);
    if (cpm->useheight) {
        coeff_logh_lineEdit->setText( QString("%1").arg(cpm->mvec_x[0]) );
    } else {
        coeff_logh_lineEdit->setText( "0.0" );
    }

   num_clutter_type_textValue->setText( QString("%1").arg(cpm->num_clutter_type) );

    /**************************************************************************************/
    /**** Clutter Coefficient Table                                                    ****/
    /**************************************************************************************/
    num_row = clutter_view_table->table->numRows();
    for ( i=num_row-1; i>=0; i-- ) {
        clutter_view_table->table->removeRow(i);
    }
    clutter_view_table->table->setNumRows(cpm->num_clutter_type);

    th = clutter_view_table->table->verticalHeader();
    for (i=0; i<=cpm->num_clutter_type-1; i++ ) {

        clutter_view_table->table->setItem( i, 0,
            new Q3TableItem( clutter_view_table->table, Q3TableItem::WhenCurrent, QString("%1").arg(cpm->mvec_x[cpm->useheight+i])));

        clutter_view_table->table->item(i, 0)->setEnabled(false);

        th->setLabel( i, QString("Clutter Type %1").arg(i) );
    }
    /**************************************************************************************/

    blockSignals(false);
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::applyParamEdit                                    ****/
/******************************************************************************************/
void ClutterInfoWidClass::applyParamEdit(int pm_idx)
{
#if 0
// 050914 MM : This functionality has not been added to the command line.  After it is
//             added to the command line, GUI can be updated.

    int ival, flag, new_useheight, i;
    double dval, xval, yval;
    SegmentPropModelClass *pm = (SegmentPropModelClass *) np->prop_model_list[pm_idx];

    dval = start_slope_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->start_slope) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -start_slope %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }

    dval = final_slope_lineEdit->text().stripWhiteSpace().toDouble();
    if (dval != pm->final_slope) {
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -final_slope %f", pm_idx, dval);
        np->process_command(np->line_buf);
    }

    flag = 0;
    ival = num_pt_spinBox->value();
    if (ival != pm->num_inflexion) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -num_pts %d", pm_idx, ival);
        np->process_command(np->line_buf);
    }

    for (i=0; i<=ival-1; i++) {
        xval = view_table->table->text(i, 0).stripWhiteSpace().toDouble();
        yval = view_table->table->text(i, 1).stripWhiteSpace().toDouble();
        if ( (flag) || (xval != pm->x[i]) || (yval != pm->y[i]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -pt_idx %d -x %f -y %f", pm_idx, i, xval, yval);
            np->process_command(np->line_buf);
        }
    }

    flag = 0;
    new_useheight = (useheight_checkBox->isChecked() ? 1 : 0);
    if (new_useheight != pm->useheight) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -useheight %d", pm_idx, new_useheight);
        np->process_command(np->line_buf);
    }

    if (new_useheight) {
        dval = coeff_logh_lineEdit->text().stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[0]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -coeff_logh %f", pm_idx, dval);
            np->process_command(np->line_buf);
        }
        dval = coeff_loghd_lineEdit->text().stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[1]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -coeff_loghd %f", pm_idx, dval);
            np->process_command(np->line_buf);
        }
    }

    flag = 0;
    ival = num_clutter_spinBox->value();
    if (ival != pm->num_clutter_type) {
        flag = 1;
        sprintf(np->line_buf, "set_prop_model -pm_idx %d -num_clutter_type %d", pm_idx, ival);
        np->process_command(np->line_buf);
    }

    for (i=0; i<=ival-1; i++) {
        dval = clutter_view_table->table->text(i, 0).stripWhiteSpace().toDouble();
        if ( (flag) || (dval != pm->vec_k[2*pm->useheight+i]) ) {
            sprintf(np->line_buf, "set_prop_model -pm_idx %d -c_idx %d -c_coeff %f", pm_idx, i, dval);
            np->process_command(np->line_buf);
        }
    }
#endif
}
/******************************************************************************************/
/**** FUNCTION: ClutterInfoWidClass::clutter_view_btn_clicked                          ****/
/******************************************************************************************/
void ClutterInfoWidClass::clutter_view_btn_clicked()
{
    clutter_view_table->exec();
}
/******************************************************************************************/
