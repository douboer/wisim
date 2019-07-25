
/******************************************************************************************
**** PROGRAM: subnet_traffic_dia.cpp 
**** AUTHOR:  Chengan 4/01/04
****          douboer@gmail.com
******************************************************************************************/
/*
   rewrited on 2004.5
 */
#include <iostream>

#include <qcheckbox.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <q3frame.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>

#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "phs.h"
#include "polygon.h"
#include "subnet_traffic_dia.h"
#include "sector_prop_table.h"

enum SortType{ SortFloat, SortString };
SortType mtype;

#define SN_DEBUG 0

MTable::MTable(QWidget* parent, const char* name) : Q3Table( parent, name )
{
    mtype = SortFloat;
}

struct MSortableTableItem
{
    Q3TableItem *item;
};

static int cmpMTableItems( const void *n1, const void *n2 )
{
    int n(0);

    if ( !n1 || !n2 )
        return 0;

    MSortableTableItem *i1 = (MSortableTableItem *)n1;
    MSortableTableItem *i2 = (MSortableTableItem *)n2;

    if ( mtype == SortFloat ) {
        if (i1->item->col() == 0) {
            n = qstringcmp(i1->item->key(), i2->item->key());
        } else {
            double v1 = i1->item->key().toDouble();
            double v2 = i2->item->key().toDouble();
            if (v1 < v2) {
                n = -1;
            } else if (v1 > v2) {
                n = 1;
            } else {
                n = 0;
            }
        }
    } else if ( mtype == SortString ) {
        QString v1 = i1->item->key();
        QString v2 = i2->item->key();
        n = v1.localeAwareCompare( v2 );
    }

    return(n);
}

void MTable::sortColumn( int col, bool ascending, bool)
{
    bool wholeRows = TRUE;

    int filledRows = 0, i;
    for ( i = 0; i < numRows(); ++i ) {
        Q3TableItem *itm = item( i, col );
        if ( itm )
            filledRows++;
    }

    if ( !filledRows )
        return;

    MSortableTableItem *items = new MSortableTableItem[ filledRows ];
    int j = 0;
    for ( i = 0; i < numRows(); ++i ) {
        Q3TableItem *itm = item( i, col );
        if ( !itm )
            continue;
        items[ j++ ].item = itm;
    }

    qsort( items, filledRows, sizeof( MSortableTableItem ), cmpMTableItems );

    bool updatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled( FALSE );
    for ( i = 0; i < numRows(); ++i ) {
        if ( i < filledRows ) {
            if ( ascending ) {
                if ( items[ i ].item->row() == i )
                    continue;
                if ( wholeRows )
                    swapRows( items[ i ].item->row(), i );
                else
                    swapCells( items[ i ].item->row(), col, i, col );
            } else {
                if ( items[ i ].item->row() == filledRows - i - 1 )
                    continue;
                if ( wholeRows )
                    swapRows( items[ i ].item->row(), filledRows - i - 1 );
                else
                    swapCells( items[ i ].item->row(), col,
                               filledRows - i - 1, col );
            }
        }
    }
    setUpdatesEnabled( updatesEnabled );
    if ( horizontalHeader() )
        horizontalHeader()->setSortIndicator( col, ascending ? Qt::AscendingOrder : Qt::DescendingOrder );

    if ( !wholeRows )
        repaintContents( columnPos( col ), contentsY(),
                         columnWidth( col ), visibleHeight(), FALSE );
    else
        repaintContents( contentsX(), contentsY(),
                         visibleWidth(), visibleHeight(), FALSE );

    delete [] items;
}

/*
 *   construct function : SubnetTraffic::SubnetTraffic(NetworkClass* np_param, QWidget* parent)
 */
SubnetTraffic::SubnetTraffic(NetworkClass* np_param, QWidget* parent)
    : QDialog( parent, 0, TRUE )
{
    setName( "SubnetTraffic" );
    np = np_param;
    traffic_type_idx = 0;
    setSizeGripEnabled( TRUE );
    SubnetTrafficLayout = new Q3GridLayout( this, 1, 1, 11, 15, "SubnetTrafficLayout"); 

    //-----------------------------------------------------------------------------------------------------------
    typeLayout = new Q3GridLayout( 0, 1, 1, 0, 15, "typeLayout"); 

    traffic_type_textLabel = new QLabel( this, "traffic_type_textLabel" );
    traffic_type_textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 5, 0, 
                traffic_type_textLabel->sizePolicy().hasHeightForWidth() ) );
    typeLayout->addWidget( traffic_type_textLabel, 0, 0 );

    traffic_comboBox = new QComboBox( FALSE, this, "traffic_comboBox" );
    traffic_comboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 8, 0, traffic_comboBox->sizePolicy().hasHeightForWidth() ) );
    typeLayout->addWidget( traffic_comboBox, 0, 1 );


    QLabel* space_lbl2 = new QLabel( this, "space_lbl2" ); 
    space_lbl2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 13, 0, space_lbl2->sizePolicy().hasHeightForWidth() ) );
    typeLayout->addWidget( space_lbl2, 0, 2 );

    SubnetTrafficLayout->addLayout( typeLayout, 0, 0 );

    //-----------------------------------------------------------------------------------------------------------
    if ( np->technology() == CConst::PHS ) {
        traffic_buttonGroup = new Q3ButtonGroup( this, "traffic_buttonGroup" );
        traffic_buttonGroup->setAlignment( int( Qt::AlignVCenter ) );
        traffic_buttonGroup->setColumnLayout(0, Qt::Vertical );
        traffic_buttonGroup->layout()->setSpacing( 6 );
        traffic_buttonGroup->layout()->setMargin( 11 );

        traffic_buttonGroupLayout = new Q3GridLayout( traffic_buttonGroup->layout() );
        traffic_buttonGroupLayout->setAlignment( Qt::AlignTop );

        exp_radioButton = new QRadioButton( traffic_buttonGroup, "exp_radioButton" );
        traffic_buttonGroupLayout->addWidget( exp_radioButton, 0, 0 );

        line = new Q3Frame( traffic_buttonGroup, "line" );
        line->setMinimumSize( QSize( 20, 0 ) );
        line->setFrameShape( Q3Frame::VLine );
        line->setFrameShadow( Q3Frame::Sunken );
        line->setFrameShape( Q3Frame::VLine );
        traffic_buttonGroupLayout->addMultiCellWidget( line, 0, 1, 1, 1 );

        meanLayout = new Q3HBoxLayout( 0, 0, 6, "meanLayout"); 

        spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
        meanLayout->addItem( spacer3 );

        mean_time_lbl = new QLabel( traffic_buttonGroup, "mean_time_lbl" );
        mean_time_lbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 5, 0, mean_time_lbl->sizePolicy().hasHeightForWidth() ) );
        meanLayout->addWidget( mean_time_lbl );

        mean_time_val = new QLineEdit( traffic_buttonGroup, "mean_time_val" );
        mean_time_val->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 10, 0, mean_time_val->sizePolicy().hasHeightForWidth() ) );
        mean_time_val->setMinimumSize( QSize( 80, 0 ) );
        meanLayout->addWidget( mean_time_val );

        traffic_buttonGroupLayout->addLayout( meanLayout, 1, 0 );

        unif_radioButton = new QRadioButton( traffic_buttonGroup, "unif_radioButton" );
        traffic_buttonGroupLayout->addWidget( unif_radioButton, 0, 2 );

        minMaxLayout = new Q3GridLayout( 0, 1, 1, 0, 6, "minMaxLayout"); 
        spacer4 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
        minMaxLayout->addMultiCell( spacer4, 0, 1, 0, 0 );

        min_time_lbl = new QLabel( traffic_buttonGroup, "min_time_lbl" );
        min_time_lbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 5, 0, min_time_lbl->sizePolicy().hasHeightForWidth() ) );
        minMaxLayout->addWidget( min_time_lbl, 0, 1 );

        min_time_val = new QLineEdit( traffic_buttonGroup, "min_time_val" );
        min_time_val->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 10, 0, min_time_val->sizePolicy().hasHeightForWidth() ) );
        min_time_val->setMinimumSize( QSize( 80, 0 ) );
        minMaxLayout->addWidget( min_time_val, 0, 2 );

        max_time_lbl = new QLabel( traffic_buttonGroup, "max_time_lbl" );
        minMaxLayout->addWidget( max_time_lbl, 1, 1 );

        max_time_val = new QLineEdit( traffic_buttonGroup, "max_time_val" );
        max_time_val->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, max_time_val->sizePolicy().hasHeightForWidth() ) );
        max_time_val->setMinimumSize( QSize( 80, 0 ) );
        minMaxLayout->addWidget( max_time_val, 1, 2 );

        traffic_buttonGroupLayout->addLayout( minMaxLayout, 1, 2 );

        SubnetTrafficLayout->addWidget( traffic_buttonGroup, 1, 0 );

    //-----------------------------------------------------------------------------------------------------------
        numAttemp_buttonGroup = new Q3ButtonGroup( this, "numAttemp_buttonGroup" );
        numAttemp_buttonGroup->setColumnLayout(0, Qt::Vertical );
        numAttemp_buttonGroup->layout()->setSpacing( 6 );
        numAttemp_buttonGroup->layout()->setMargin( 11 );
        numAttemp_buttonGroupLayout = new Q3HBoxLayout( numAttemp_buttonGroup->layout() );
        numAttemp_buttonGroupLayout->setAlignment( Qt::AlignTop );
        
        num_attemp_request_lbl = new QLabel( numAttemp_buttonGroup, "num_attemp_request_lbl" );
        numAttemp_buttonGroupLayout->addWidget( num_attemp_request_lbl );
        
        num_attemp_request_val = new QLineEdit( numAttemp_buttonGroup, "num_attemp_request_val" );
        num_attemp_request_val->setMaximumSize( QSize( 120, 32767 ) );
        numAttemp_buttonGroupLayout->addWidget( num_attemp_request_val );
        
        line_2 = new Q3Frame( numAttemp_buttonGroup, "line_2" );
        line_2->setMinimumSize( QSize( 20, 0 ) );
        line_2->setFrameShape( Q3Frame::VLine );
        line_2->setFrameShadow( Q3Frame::Sunken );
        line_2->setFrameShape( Q3Frame::VLine );
        numAttemp_buttonGroupLayout->addWidget( line_2 );
        
        num_attemp_handover_lbl = new QLabel( numAttemp_buttonGroup, "num_attemp_handover_lbl" );
        numAttemp_buttonGroupLayout->addWidget( num_attemp_handover_lbl );
        
        num_attemp_handover_val = new QLineEdit( numAttemp_buttonGroup, "num_attemp_handover_val" );
        num_attemp_handover_val->setMaximumSize( QSize( 120, 32767 ) );
        numAttemp_buttonGroupLayout->addWidget( num_attemp_handover_val );

        SubnetTrafficLayout->addWidget( numAttemp_buttonGroup, 2, 0 );
    }

    if ( np->technology() == CConst::WLAN ) {
        // to be implement
    }

    //-----------------------------------------------------------------------------------------------------------
     ckb = new QCheckBox( this, "ckb" );
     SubnetTrafficLayout->addWidget( ckb, 3, 0 );

    //-----------------------------------------------------------------------------------------------------------
    trafficTableLayout = new Q3GridLayout( 0, 1, 1, 0, 0, "trafficTableLayout"); 

    subnet_traffic_table = new MTable( this, "subnet_traffic_table" );
    //subnet_traffic_table->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ); 
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1, 
            qApp->translate("SubnetTraffic", "Select to Delete" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Subnet ID" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Area(m^2)" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Arrival Rate(calls/sec)" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Offered Traffic(Erl)" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Density(Erl/m^2)" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "Fraction" ) );
    subnet_traffic_table->setNumCols( subnet_traffic_table->numCols() + 1 );
    subnet_traffic_table->horizontalHeader()->setLabel( subnet_traffic_table->numCols() - 1,
            qApp->translate("SubnetTraffic", "For Index" ) );
    //subnet_traffic_table->hideColumn( 1 );
    subnet_traffic_table->hideColumn( 7 );
    subnet_traffic_table->setHScrollBarMode( Q3ScrollView::AlwaysOff );
    subnet_traffic_table->verticalHeader()->hide();
    subnet_traffic_table->setLeftMargin( 0 );
    subnet_traffic_table->verticalHeader()->setPaletteBackgroundColor(Qt::gray);
    subnet_traffic_table->setNumCols( 8 );
    subnet_traffic_table->setSorting  ( true );
    trafficTableLayout->addWidget( subnet_traffic_table, 0, 0 );

    totalTrafficLayout = new Q3HBoxLayout( 0, 0, 0, "totalTrafficLayout"); 

    total_textLabel = new QLabel( this, "total_textLabel" );
    totalTrafficLayout->addWidget( total_textLabel );


    total_area_val = new QLineEdit( this, "total_area_val" );
    totalTrafficLayout->addWidget( total_area_val );
    total_area_val->setAlignment( Qt::AlignRight );

    total_arrival_rate_val = new QLineEdit( this, "total_arrival_rate_val" );
    totalTrafficLayout->addWidget( total_arrival_rate_val );
    total_arrival_rate_val->setAlignment( Qt::AlignRight );

    total_traffic_val = new QLineEdit( this, "total_traffic_val" );
    totalTrafficLayout->addWidget( total_traffic_val );
    total_traffic_val->setAlignment( Qt::AlignRight );

    total_density_val = new QLineEdit( this, "total_density_val" );
    totalTrafficLayout->addWidget( total_density_val );
    total_density_val->setAlignment( Qt::AlignRight );

    total_faction_val = new QLineEdit( this, "total_faction_val" );
    totalTrafficLayout->addWidget( total_faction_val );
    total_faction_val->setAlignment( Qt::AlignRight );

    QSpacerItem* sp = new QSpacerItem( 100, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    totalTrafficLayout->addItem( sp );

    trafficTableLayout->addLayout( totalTrafficLayout, 1, 0 );

    SubnetTrafficLayout->addLayout( trafficTableLayout, 4, 0 );

    //-----------------------------------------------------------------------------------------------------------
    okCancelLayout = new Q3HBoxLayout( 0, 0, 6, "okCancelLayout"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( FALSE );
    buttonHelp->setDefault( FALSE );
    okCancelLayout->addWidget( buttonHelp );
    spacer6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    okCancelLayout->addItem( spacer6 );

    buttonApply = new QPushButton( this, "buttonApply" );
    buttonApply->setAutoDefault( FALSE );
    buttonApply->setDefault( FALSE );
    okCancelLayout->addWidget( buttonApply );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( FALSE );
    buttonOk->setDefault( FALSE );
    okCancelLayout->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( FALSE );
    okCancelLayout->addWidget( buttonCancel );

    SubnetTrafficLayout->addLayout( okCancelLayout, 5, 0 );

    //-----------------------------------------------------------------------------------------------------------
    //initial radio buttonGroup
    if ( np->technology() == CConst::PHS ) { 
        min_time_val->setDisabled(TRUE);
        max_time_val->setDisabled(TRUE);
    }

    //initial the column size of subnet traffic table
    subnet_traffic_table->setColumnWidth( 0, 40 );
    subnet_traffic_table->setColumnWidth( 1, 50 );
    subnet_traffic_table->setColumnWidth( 2, 105 );
    subnet_traffic_table->setColumnWidth( 3, 105 );
    subnet_traffic_table->setColumnWidth( 4, 105 );
    subnet_traffic_table->setColumnWidth( 5, 100 );
    subnet_traffic_table->setColumnWidth( 6, 100);
    subnet_traffic_table->setColumnWidth( 7, 105 );


    //initial the basic parameters of a traffic 
    number_traffic_type = np->num_traffic_type;
    if (number_traffic_type != 0) {
#if SN_DEBUG
        std::cout << "traffic_type_idx = " << traffic_type_idx << std::endl;
        std::cout << " np->traffic_type_list[traffic_type_idx]->mean_time = " <<  np->traffic_type_list[traffic_type_idx]->mean_time << std::endl;
#endif

        if ( np->technology() == CConst::PHS ) {
            str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->mean_time);
            mean_time_val->setText(str);

            str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->min_time);
            min_time_val->setText(str);

            str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->max_time);
            max_time_val->setText(str);

            str.sprintf("%d", ((PHSTrafficTypeClass *) np->traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent));
            num_attemp_request_val->setText(str);

            str.sprintf("%d", ((PHSTrafficTypeClass *) np->traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent));
            num_attemp_handover_val->setText(str);
        }

        traffic_comboBox->clear();
        for(int i = 0; i<number_traffic_type; i++) {
            //printf("test np->traffic_type_list[i] %d\n", np->traffic_type_list[i]);
            traffic_comboBox->insertItem(np->traffic_type_list[i]->name(), i );
        }

#if SN_DEBUG
        std::cout << "np->traffic_type_list[traffic_type_idx]->duration_dist = " << np->traffic_type_list[traffic_type_idx]->duration_dist << std::endl;
#endif
        if ( np->technology() == CConst::PHS ) {
            if ( np->traffic_type_list[traffic_type_idx]->duration_dist == 0 ) {
                exp_radioButton->setChecked(TRUE);
                min_time_val->setDisabled(TRUE);
                max_time_val->setDisabled(TRUE);
                mean_time_val->setDisabled(FALSE);
            } 
            else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == 1 ) {
                exp_radioButton->setChecked(FALSE);
                unif_radioButton->setChecked(TRUE);
                min_time_val->setDisabled(FALSE);
                max_time_val->setDisabled(FALSE);
                mean_time_val->setDisabled(TRUE);
            }
        }
    }

    number_subnet.clear();
    for (int tr_idx=0; tr_idx<number_traffic_type; tr_idx++) {
        number_subnet.push_back( np->num_subnet[tr_idx] );
#if SN_DEBUG 
        std::cout << "debug number_subner : " << number_subnet[tr_idx] << std::endl;
        std::cout << std::endl << "====================================" << std::endl;
        std::cout << "type list of traffic : " << np->traffic_type_list[tr_idx]->strid << std::endl;
        for ( int m_subnet_idx = 0; m_subnet_idx < np->num_subnet[tr_idx]; m_subnet_idx++ )
            {
                subnet_class = np->subnet_list[tr_idx][m_subnet_idx];
                std::cout << "subnet ID : " << subnet_class->strid << std::endl;
            }
        std::cout << "====================================" << std::endl;
#endif
    }

    recreate_table();


    //initial the size of total traffic lineEdit width.
    total_area_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 2 ).width() );
    total_arrival_rate_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 3 ).width() );
    total_traffic_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 4 ).width() );
    total_density_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 5 ).width() );
    total_faction_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 6 ).width() );


    int width;
    width = subnet_traffic_table->verticalHeader()->sectionRect( 0 ).width() 
        + subnet_traffic_table->horizontalHeader()->sectionRect( 0 ).width()
        + subnet_traffic_table->horizontalHeader()->sectionRect( 1 ).width();
    total_textLabel->setMinimumWidth( width );



    //-----------------------------------------------------------------------------------------------------------
    languageChange();
    resize( QSize(665, 450).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply_btn_clicked() ) );
    connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( help_btn_clicked() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );
    connect( subnet_traffic_table->horizontalHeader(), SIGNAL( sectionSizeChanged( int ) ), 
            this, SLOT( hHeadSizeChanged( int ) ) );
    //connect( subnet_traffic_table, SIGNAL( valueChanged( int, int )), this, SLOT( tab_valueChanged( int, int )));
    connect( ckb, SIGNAL( stateChanged(int) ), this, SLOT( all_ckb_selected(int) ) );

    connect( total_arrival_rate_val,  SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
    connect( total_traffic_val,       SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
    connect( total_density_val,       SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
    if ( np->technology() == CConst::PHS ) {
        connect( traffic_comboBox, SIGNAL( activated(int)), this, SLOT( traffic_comboBox_select(int) ));
        connect( num_attemp_request_val,  SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
        connect( num_attemp_handover_val, SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
        connect( traffic_buttonGroup, SIGNAL( clicked( int )), this, SLOT( traffic_buttonGroup_clicked( int )));

        connect( mean_time_val,           SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
        connect( min_time_val,            SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
        connect( max_time_val,            SIGNAL( returnPressed( ) ), this, SLOT( total_val_changed( ) ) ); 
    }

    connect ( subnet_traffic_table->horizontalHeader(), SIGNAL( clicked( int ) ), this, SLOT( headClicked( int ) ) );

    // tab order
    if ( np->technology() == CConst::PHS ) {
        setTabOrder( traffic_comboBox, exp_radioButton);
        setTabOrder( exp_radioButton, mean_time_val );
        setTabOrder( mean_time_val, min_time_val );
        setTabOrder( min_time_val, max_time_val );
        setTabOrder( max_time_val, total_area_val );
    }

    setTabOrder( total_area_val, total_arrival_rate_val );
    setTabOrder( total_arrival_rate_val, total_traffic_val );
    setTabOrder( total_traffic_val, total_density_val );
    setTabOrder( total_density_val, total_faction_val );
    setTabOrder( total_faction_val, subnet_traffic_table );
    setTabOrder( subnet_traffic_table, buttonHelp );
    setTabOrder( buttonHelp, buttonApply );
    setTabOrder( buttonApply, buttonOk );
    setTabOrder( buttonOk, buttonCancel );



    exec();
}

/*
 *  Destroys the object and frees any allocated resources
 */
SubnetTraffic::~SubnetTraffic()
{

}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SubnetTraffic::languageChange()
{
    setCaption( qApp->translate("SubnetTraffic", "Subnet Traffic" ) );

    ckb->setText(tr( "Select All Subnet to Delete" ));
    buttonHelp->setText( qApp->translate("SubnetTraffic", "&Help" ) );
    buttonHelp->setAccel( QKeySequence( "F1" ) );
    buttonApply->setText( qApp->translate("SubnetTraffic", "&Apply" ) );
    buttonApply->setAccel( QKeySequence( QString::null ) );
    buttonOk->setText( qApp->translate("SubnetTraffic", "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( qApp->translate("SubnetTraffic", "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );

    subnet_traffic_table->horizontalHeader()->setLabel( 0, qApp->translate("SubnetTraffic", "Select to Delete" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 1, qApp->translate("SubnetTraffic", "Subnet ID" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 2, qApp->translate("SubnetTraffic", "Area(m^2)" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 3, qApp->translate("SubnetTraffic", "Arrival Rate(calls/sec)" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 4, qApp->translate("SubnetTraffic", "Offered Traffic(Erl)" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 5, qApp->translate("SubnetTraffic", "Density(Erl/m^2)" ) );
    subnet_traffic_table->horizontalHeader()->setLabel( 6, qApp->translate("SubnetTraffic", "Fraction" ) );
    total_textLabel->setText( qApp->translate("SubnetTraffic", "System Boundary" ) );

    if ( np->technology() == CConst::PHS ) { 
        traffic_buttonGroup->setTitle( qApp->translate("SubnetTraffic", "Distribution" ) );
        mean_time_lbl->setText( qApp->translate("SubnetTraffic", "Mean Time" ) );
        unif_radioButton->setText( qApp->translate("SubnetTraffic", "Uniform" ) );
        min_time_lbl->setText( qApp->translate("SubnetTraffic", "Min Time" ) );
        max_time_lbl->setText( qApp->translate("SubnetTraffic", "Max Time" ) );
        exp_radioButton->setText( qApp->translate("SubnetTraffic", "Exponent" ) );

        numAttemp_buttonGroup->setTitle( tr( "Number of Attempts to Assign Channel" ) );
        num_attemp_request_lbl->setText( tr( "Call Request" ) );
        num_attemp_handover_lbl->setText( tr( "Handover" ) );
    }

    //traffic_comboBox->clear();
    //traffic_comboBox->insertItem( qApp->translate("SubnetTraffic", "COMM" ) );
    //traffic_comboBox->insertItem( qApp->translate("SubnetTraffic", "LREG" ) );
    //arrival_rate_textLabel->setText( qApp->translate("SubnetTraffic", "Arrival Rate" ) );
    traffic_type_textLabel->setText( qApp->translate("SubnetTraffic", "Traffic Type" ) );
}


void SubnetTraffic::headClicked( int section ) {
    std::cout << "Sorting vertical header section " << section << std::endl;

    switch ( section )  {
        case 1: 
            mtype = SortString;
            break;
        default:
            mtype = SortFloat;
            break;
    }
    subnet_traffic_table->sortColumn( section, true, true);
}

/*
 *    function : SubnetTraffic::apply_btn_clicked()
 */
void SubnetTraffic::apply_btn_clicked()
{
    setFocus();
    delete_subnet_func();
    total_val_changed();

    ckb ->setChecked( false );
}

void SubnetTraffic::help_btn_clicked()
{
    QMessageBox::information( this, tr("Help"),
            tr("You can press 'Enter' key to verify the setting."),
            QMessageBox::Cancel,
            0,
            0
            );

}


/*
 *   function : SubnetTraffic::ok_btn_clicked()
 */
void SubnetTraffic::ok_btn_clicked()
{
    accept ();
    delete_subnet_func();
    total_val_changed();
    ckb ->setChecked( false );

    delete this;
}


/*
 *  function : SubnetTraffic::hHeadSizeChanged( int section )
 */
void SubnetTraffic::hHeadSizeChanged( int section )
{
    int   width;
    switch ( section ) {
        case 0:
            width = subnet_traffic_table->verticalHeader()->sectionRect( 0 ).width() 
                + subnet_traffic_table->horizontalHeader()->sectionRect( 0 ).width()
                + subnet_traffic_table->horizontalHeader()->sectionRect( 1 ).width();
            total_textLabel->setMinimumWidth( width );
#if SN_DEBUG 
            std::cout << " width : " << width << std::endl; 
            std::cout << "lbl width : " << total_textLabel->width() << std::endl;
#endif
            break;
        case 1:
            width = subnet_traffic_table->verticalHeader()->sectionRect( 0 ).width() 
                + subnet_traffic_table->horizontalHeader()->sectionRect( 0 ).width()
                + subnet_traffic_table->horizontalHeader()->sectionRect( 1 ).width();
            total_textLabel->setMinimumWidth( width );
            break;
        case 2:
            total_area_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 2 ).width() ); 
            break;
        case 3:
            total_arrival_rate_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 3 ).width() ); 
            break;
        case 4:
            total_traffic_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 4 ).width() ); 
            break;
        case 5:
            total_density_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 5 ).width() ); 
            break;
        case 6:
            total_faction_val->setFixedWidth( subnet_traffic_table->horizontalHeader()->sectionRect( 6 ).width() ); 
            break;
    }
    total_area_val->update();
    total_arrival_rate_val->update();
    total_traffic_val->update();
    total_density_val->update();
    total_faction_val->update();
}


/*
 *  function: SubnetTraffic::traffic_comboBox_select(int)
 */
void SubnetTraffic::traffic_comboBox_select(int j)
{
    traffic_type_idx = j;

    QString str;
    traffic_comboBox->setCurrentItem(traffic_type_idx);
    ckb->setChecked( false );


    if ( np->technology() == CConst::PHS ) {
        str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->mean_time);
        mean_time_val->setText(str);

        str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->min_time);
        min_time_val->setText(str);

        str.sprintf("%f", np->traffic_type_list[traffic_type_idx]->max_time);
        max_time_val->setText(str);

        str.sprintf("%d", ((PHSTrafficTypeClass *) np->traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::RequestEvent));
        num_attemp_request_val->setText(str);

        str.sprintf("%d", ((PHSTrafficTypeClass *) np->traffic_type_list[traffic_type_idx])->get_num_attempt(CConst::HandoverEvent));
        num_attemp_handover_val->setText(str);
    }


    if ( np->traffic_type_list[traffic_type_idx]->duration_dist == 0 ) {
        exp_radioButton->setChecked(TRUE);
        unif_radioButton->setChecked(FALSE);
        if ( np->technology() == CConst::PHS ) {
            min_time_val->setDisabled(TRUE);
            max_time_val->setDisabled(TRUE);
            mean_time_val->setDisabled(FALSE);
        }
    } 
    else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == 1 ) {
        exp_radioButton->setChecked(FALSE);
        unif_radioButton->setChecked(TRUE);
        if ( np->technology() == CConst::PHS ) {
            min_time_val->setDisabled(FALSE);
            max_time_val->setDisabled(FALSE);
            mean_time_val->setDisabled(TRUE);
        }
    }

     recreate_table();
}


/*
 *  function : SubnetTraffic::traffic_buttonGroup_clicked( int i )
 */
void SubnetTraffic::traffic_buttonGroup_clicked( int i )
{
    switch (i) {
        case 0:
            min_time_val->setDisabled(TRUE);
            max_time_val->setDisabled(TRUE);
            mean_time_val->setDisabled(FALSE);

            sprintf(np->line_buf, "set_traffic -traffic_type %s -duration_dist expo", np->traffic_type_list[traffic_type_idx]->name() );
            np->process_command(np->line_buf);
            
            unif_radioButton->setChecked(FALSE);
            break;
        case 1:
            mean_time_val->setDisabled(TRUE);
            min_time_val->setDisabled(FALSE);
            max_time_val->setDisabled(FALSE);

            sprintf(np->line_buf, "set_traffic -traffic_type %s -duration_dist unif", np->traffic_type_list[traffic_type_idx]->name() );
            np->process_command(np->line_buf);
            
            exp_radioButton->setChecked(FALSE);
            break;
    }

    update_all_value();
}

/*
 *  function: SubnetTraffic::update_all_value()   
 */
//XXXXXXXXX  use two function : update_all_value() 
//                              recreate_table()

void SubnetTraffic::recreate_table()
{
    int m_subnet_idx;
    //table_tmp->hideColumn( 1 );


    table_tmp = new MTable( this, "table_tmp" );
    //table_tmp->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ); 
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Select to Delete" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Subnet ID" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Area(m^2)" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Arrival Rate(calls/sec)" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Offered Traffic(Erl)" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Density(Erl/m^2)" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "Fraction" ) );
    table_tmp->setNumCols( table_tmp->numCols() + 1 );
    table_tmp->horizontalHeader()->setLabel( table_tmp->numCols() - 1,
            qApp->translate("SubnetTraffic", "For Index" ) );
    //table_tmp->hideColumn( 1 );
    table_tmp->hideColumn( 7 );
    table_tmp->setHScrollBarMode( Q3ScrollView::AlwaysOff );
    table_tmp->verticalHeader()->hide();
    table_tmp->setLeftMargin( 0 );
    table_tmp->verticalHeader()->setPaletteBackgroundColor(Qt::gray);
    table_tmp->setNumCols( 8 );
    table_tmp->setSorting  ( true );
    
    table_tmp->hideColumn( 7 );

    single_area                = 0;
    single_arrival_rate        = 0;
    single_traffic             = 0;
    single_density             = 0;
    single_faction             = 0;
    //total_traffic_proportion   = 0;
    total_area                 = 0;
    total_arrival_rate         = 0;
    total_traffic              = 0;
    total_density              = 0;
    total_faction              = 0;

    /*
        Read the list of the subnet and write them to the table.
     */
    if ( number_traffic_type!=0 ) {
        if ( number_subnet[traffic_type_idx] != 0 ) {

             table_tmp->setDisabled( FALSE );

             total_arrival_rate_val->setReadOnly( FALSE );
             total_traffic_val->setReadOnly( FALSE);
             total_density_val->setReadOnly( FALSE );

             //compute the total traffic, % 
             for( m_subnet_idx=0; m_subnet_idx<number_subnet[traffic_type_idx]; m_subnet_idx++ )
             {
                 subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];
                 total_arrival_rate += subnet_class->arrival_rate;
#if SN_DEBUG
                 std::cout << "subnet_idx " << m_subnet_idx << " subnet_class->arrival_rate " << subnet_class->arrival_rate << std::endl; 
                 std::cout << "total_arrival_rate " << total_arrival_rate << std::endl;
#endif
             }

             //fill the table with param and compute the total value of subnet param
             //std::cout << "debug number_subnet[traffic_type_idx]: " << number_subnet[traffic_type_idx] << std::endl; 
             //for ( int i=0; i<table_tmp->numRows(); i++ )        mistake!! table_tmp->numRows() is changing
             while ( table_tmp->numRows() > 0 )
             {
                 table_tmp->removeRow( table_tmp->numRows()-1 );
             }

             ckb_item_vec.clear();
             for( m_subnet_idx=0; m_subnet_idx<number_subnet[traffic_type_idx]; m_subnet_idx++ )
             {
                 subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];

                 table_tmp->insertRows( m_subnet_idx );

                 ckb_item = new Q3CheckTableItem( table_tmp, "    ");
                 table_tmp->setItem( m_subnet_idx, 0, ckb_item );
                 ckb_item_vec.push_back( ckb_item );
                 
                 //property QTableItem::Never shows that the table items are not editable. 
                 tab_item = new TableItem( table_tmp, Q3TableItem::Never, subnet_class->strid );
                 table_tmp->setItem( m_subnet_idx, 1, tab_item );
                 
                 //fill area of each subnet
                 single_area = subnet_class->p->comp_bdy_area();

                 total_area += single_area;
                 str.sprintf( "%f", single_area );
                 tab_item = new TableItem( table_tmp, Q3TableItem::Never, str );
                 table_tmp->setItem( m_subnet_idx, 2, tab_item );

                 single_arrival_rate = subnet_class->arrival_rate;
#if 0
                 total_arrival_rate += single_arrival_rate;
#endif 
                 str.sprintf( "%f", single_arrival_rate ); 
                 //tab_item = new TableItem( table_tmp, QTableItem::Never, str );
                 //table_tmp->setItem( m_subnet_idx, 2, tab_item );
                 table_tmp->setText( m_subnet_idx, 3, str );

                 if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                     single_traffic = single_arrival_rate*(np->traffic_type_list[traffic_type_idx]->mean_time);
                 }
                 else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                     single_traffic = single_arrival_rate*((np->traffic_type_list[traffic_type_idx]->min_time
                                 + np->traffic_type_list[traffic_type_idx]->max_time)/2);
                 }
                 total_traffic += single_traffic;
                 str.sprintf( "%f", single_traffic );
                 table_tmp->setText( m_subnet_idx, 4, str ); 

                 single_density = single_traffic/single_area;
                 str.sprintf( "%f", single_density );
                 //tab_item = new TableItem( table_tmp, QTableItem::Never, str );
                 //table_tmp->setItem( m_subnet_idx, 4, tab_item );
                 table_tmp->setText( m_subnet_idx, 5, str );

                 single_faction = (subnet_class->arrival_rate)/total_arrival_rate ;
                 str.sprintf( "%f", single_faction );
                 tab_item = new TableItem( table_tmp, Q3TableItem::Never, str ); 
                 table_tmp->setItem( m_subnet_idx, 6, tab_item );

                 str.sprintf( "%d", m_subnet_idx );
                 table_tmp->setText( m_subnet_idx, 7, str );
#if SN_DEBUG
                 // xxx std::cout << "proportion traffic " << subnet_class->traffic << std::endl;
                 std::cout << "single_area  " << single_area << std::endl;
                 std::cout << "subnet_idx   " << m_subnet_idx << " single_faction      " << single_faction << std::endl; 
                 // xxx std::cout << "arrival_rate " << np->traffic_type_list[traffic_type_idx]->arrival_rate << std::endl; 
                 std::cout << "subnet_idx   " << m_subnet_idx << " single_arrival_rate " << single_arrival_rate << std::endl; 
                 std::cout << "subnet_idx   " << m_subnet_idx << " single_density      " << single_density << std::endl; 
#endif
             }
             total_faction = 1;
             total_faction_val->setText( "1" );
             total_faction_val->setReadOnly( TRUE );
             total_faction_val->setPaletteBackgroundColor( Qt::gray );

             total_area_val->setText( str.sprintf("%f", total_area) );
             total_area_val->setReadOnly( TRUE );
             total_area_val->setPaletteBackgroundColor( Qt::gray );

             total_arrival_rate_val->setText(str.sprintf("%f", total_arrival_rate));
#if  SN_DEBUG
             total_arrival_rate_val->setReadOnly( TRUE );
             total_arrival_rate_val->setPaletteBackgroundColor( Qt::gray );
#endif

             total_traffic_val->setText( str.sprintf("%f", total_traffic ));
#if  SN_DEBUG
             total_traffic_val->setReadOnly( TRUE );
             total_traffic_val->setPaletteBackgroundColor( Qt::gray );
#endif

             total_density = total_traffic/total_area;
             total_density_val->setText( str.sprintf( "%f", total_density ) );
#if  SN_DEBUG
             total_density_val->setReadOnly( TRUE );
             total_density_val->setPaletteBackgroundColor( Qt::gray );
#endif
        }
        else {
             while ( table_tmp->numRows() > 0 )
             {
                 table_tmp->removeRow( table_tmp->numRows()-1 );
             }

            for ( int i=0; i < 10; i++ ) {
                table_tmp->insertRows( 0 );
            }

            table_tmp->setDisabled( TRUE );

            total_faction = 1;
            total_faction_val->setText( "1" );

            total_area = np->system_bdy->comp_bdy_area();
            total_area_val->setText( str.sprintf("%f", total_area) );

            total_arrival_rate = np->total_arrival_rate;
            total_arrival_rate_val->setText(str.sprintf("%f", total_arrival_rate));

            if( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                total_traffic = total_arrival_rate*(np->traffic_type_list[traffic_type_idx]->mean_time);
            }
            else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                total_traffic = total_arrival_rate*((np->traffic_type_list[traffic_type_idx]->min_time
                            + np->traffic_type_list[traffic_type_idx]->max_time)/2);
            }
            total_traffic_val->setText( str.sprintf("%f", total_traffic ));

            total_density = total_traffic/total_area;
            total_density_val->setText( str.sprintf( "%f", total_density ) );

            total_faction_val->setReadOnly( TRUE );
            total_faction_val->setPaletteBackgroundColor( Qt::gray );
            total_area_val->setReadOnly( TRUE );
            total_area_val->setPaletteBackgroundColor( Qt::gray );
             total_arrival_rate_val->setReadOnly( TRUE );
             total_arrival_rate_val->setPaletteBackgroundColor( Qt::gray );
             total_traffic_val->setReadOnly( TRUE );
             total_traffic_val->setPaletteBackgroundColor( Qt::gray );
             total_density_val->setReadOnly( TRUE );
             total_density_val->setPaletteBackgroundColor( Qt::gray );

#if SN_DEBUG
            std::cout << "total_arrival_rate " << total_arrival_rate << std::endl;
            std::cout << "total_traffic " << total_traffic << std::endl;
            std::cout << "total_density " << total_density << std::endl;
#endif
        }
    }

    delete subnet_traffic_table;
    subnet_traffic_table = table_tmp;
    connect( subnet_traffic_table, SIGNAL( valueChanged( int, int )), this, SLOT( tab_valueChanged( int, int )));
    table_tmp = NULL;

    trafficTableLayout->addWidget( subnet_traffic_table, 0, 0 );

    subnet_traffic_table->show();
}

void SubnetTraffic::update_all_value()
{
    int m_row;
    int m_subnet_idx; 

    single_area                = 0;
    single_arrival_rate        = 0;
    single_traffic             = 0;
    single_density             = 0;
    single_faction             = 0;
    total_area                 = 0;
    total_arrival_rate         = 0;
    total_traffic              = 0;
    total_density              = 0;
    total_faction              = 0;


    /*
        Read the list of the subnet and write them to the table.
     */
    if ( number_traffic_type!=0 ) {
        if ( number_subnet[traffic_type_idx] != 0 ) {
             subnet_traffic_table->setDisabled( FALSE );

             total_arrival_rate_val->setReadOnly( FALSE );
             total_traffic_val->setReadOnly( FALSE);
             total_density_val->setReadOnly( FALSE );

             //compute the total traffic, % 
             for( m_row=0; m_row<number_subnet[traffic_type_idx]; m_row++ )
             {
                 m_subnet_idx = subnet_traffic_table->text( m_row, 7).toInt();
                 subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];
                 total_arrival_rate += subnet_class->arrival_rate;
             }

             //fill the table with param and compute the total value of subnet param
             for( m_row=0; m_row<number_subnet[traffic_type_idx]; m_row++ )
             {
                 m_subnet_idx = subnet_traffic_table->text( m_row, 7).toInt();
                 subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];

                 single_area = subnet_class->p->comp_bdy_area();
                 total_area += single_area;

                 single_arrival_rate = subnet_class->arrival_rate;
                 str.sprintf( "%f", single_arrival_rate ); 
                 subnet_traffic_table->setText( m_row, 3, str );

                 if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                     single_traffic = single_arrival_rate*(np->traffic_type_list[traffic_type_idx]->mean_time);
                 }
                 else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                     single_traffic = single_arrival_rate*((np->traffic_type_list[traffic_type_idx]->min_time
                                 + np->traffic_type_list[traffic_type_idx]->max_time)/2);
                 }
                 total_traffic += single_traffic;
                 str.sprintf( "%f", single_traffic );
                 subnet_traffic_table->setText( m_row, 4, str ); 

                 single_density = single_traffic/single_area;
                 str.sprintf( "%f", single_density );
                 subnet_traffic_table->setText( m_row, 5, str );

                 single_faction = (subnet_class->arrival_rate)/total_arrival_rate ;
             }
             total_faction = 1;
             total_faction_val->setText( "1" );
             total_faction_val->setReadOnly( TRUE );
             total_faction_val->setPaletteBackgroundColor( Qt::gray );

             total_area_val->setText( str.sprintf("%f", total_area) );
             total_area_val->setReadOnly( TRUE );
             total_area_val->setPaletteBackgroundColor( Qt::gray );

             total_arrival_rate_val->setText(str.sprintf("%f", total_arrival_rate));

             total_traffic_val->setText( str.sprintf("%f", total_traffic ));

             total_density = total_traffic/total_area;
             total_density_val->setText( str.sprintf( "%f", total_density ) );
        }
        else {
            subnet_traffic_table->setDisabled( TRUE );

            if ( subnet_traffic_table->numRows() == 0 ) {
                for ( int i=0; i < 10; i++ ) {
                    subnet_traffic_table->insertRows( 0 );
                }
            }

            total_faction = 1;
            total_faction_val->setText( "1" );

            total_area = np->system_bdy->comp_bdy_area();
            total_area_val->setText( str.sprintf("%f", total_area) );

            total_arrival_rate = np->total_arrival_rate;
            total_arrival_rate_val->setText(str.sprintf("%f", total_arrival_rate));

            if( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                total_traffic = total_arrival_rate*(np->traffic_type_list[traffic_type_idx]->mean_time);
            }
            else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                total_traffic = total_arrival_rate*((np->traffic_type_list[traffic_type_idx]->min_time
                            + np->traffic_type_list[traffic_type_idx]->max_time)/2);
            }
            total_traffic_val->setText( str.sprintf("%f", total_traffic ));

            total_density = total_traffic/total_area;
            total_density_val->setText( str.sprintf( "%f", total_density ) );

            total_faction_val->setReadOnly( TRUE );
            total_faction_val->setPaletteBackgroundColor( Qt::gray );
            total_area_val->setReadOnly( TRUE );
            total_area_val->setPaletteBackgroundColor( Qt::gray );
            total_arrival_rate_val->setReadOnly( TRUE );
            total_arrival_rate_val->setPaletteBackgroundColor( Qt::gray );
            total_traffic_val->setReadOnly( TRUE );
            total_traffic_val->setPaletteBackgroundColor( Qt::gray );
            total_density_val->setReadOnly( TRUE );
            total_density_val->setPaletteBackgroundColor( Qt::gray );
        }
    }
}


/*
 *   function : SubnetTraffic::process_cmd()
 */
void SubnetTraffic::process_cmd()
{
    if ( np->technology() == CConst::PHS ) {
        if ( mean_time_val->edited() ) {
            sprintf(np->line_buf, "set_traffic -traffic_type %s -mean_time %s", np->traffic_type_list[traffic_type_idx]->name(), mean_time_val->text().latin1() );
            np->process_command(np->line_buf);
        }
        if ( min_time_val->edited() ) { 
            sprintf(np->line_buf, "set_traffic -traffic_type %s -min_time %s", np->traffic_type_list[traffic_type_idx]->name(), min_time_val->text().latin1() );
            np->process_command(np->line_buf);
        }
        if ( max_time_val->edited() ) {
            sprintf(np->line_buf, "set_traffic -traffic_type %s -max_time %s", np->traffic_type_list[traffic_type_idx]->name(), max_time_val->text().latin1() );
            np->process_command(np->line_buf);
        }
    }
    if ( total_arrival_rate_val->edited() ) {
        sprintf(np->line_buf, "set_traffic -traffic_type %s -arrival_rate %s", np->traffic_type_list[traffic_type_idx]->name(), total_arrival_rate_val->text().latin1() );
        np->process_command(np->line_buf);
    }
}


void SubnetTraffic::delete_subnet_func()
{
    int i = 0;
    int j = 0;
    QString str;
    std::vector<Q3CheckTableItem*>::iterator it;

    if ( ckb->isChecked() && np->num_subnet[traffic_type_idx] != 0 ) {
        for ( i=0; i<number_subnet[traffic_type_idx]; i++ ) {
            sprintf(np->line_buf, "delete_subnet -traffic_type %s -subnet %s", np->traffic_type_list[traffic_type_idx]->name(), 
                    subnet_traffic_table->text( number_subnet[traffic_type_idx] - i - 1, 1).latin1() );
            np->process_command(np->line_buf);

            subnet_traffic_table->removeRow( number_subnet[traffic_type_idx] - i - 1 );
        }
        number_subnet[traffic_type_idx] = 0;
        ckb_item_vec.clear();

    } else if ( !(ckb->isChecked()) && np->num_subnet[traffic_type_idx] != 0 ) {
        
        it = ckb_item_vec.begin();
        do {
            if ( i > subnet_traffic_table->numRows() - 1 )  break;
            
            if (  ((Q3CheckTableItem*) (subnet_traffic_table->item(i, 0)))->isChecked() ) {
                sprintf(np->line_buf, "delete_subnet -traffic_type %s -subnet %s", np->traffic_type_list[traffic_type_idx]->name(), 
                        subnet_traffic_table->text( i, 1 ).latin1() );
                np->process_command(np->line_buf);
                
                ckb_item_vec.erase( ckb_item_vec.begin() + subnet_traffic_table->text( i, 7).toInt() );
                
                for ( j=i+1; j<number_subnet[traffic_type_idx]; j++ ) {
                    str.sprintf( "%d", j-1 );
                    subnet_traffic_table->setText( j, 7, str );
                }
                
                subnet_traffic_table->removeRow( i );
                number_subnet[traffic_type_idx] --;
                
                i  --;
                it --;
            }
            
            it ++;
            i  ++;
        } 
        while ( it != ckb_item_vec.end() );
    }
}


/*
 *  function : SubnetTraffic::tab_valueChanged( int row, int col )
 */
void SubnetTraffic::tab_valueChanged( int row, int col )
{
    std::cout << " SubnetTraffic::tab_valueChanged " << std::endl;
    
    int      subnet_idx = subnet_traffic_table->text( row, 7 ).toInt(); 
    subnet_class = np->subnet_list[traffic_type_idx][subnet_idx];
    
    double   modify_value = subnet_traffic_table->text( row, col ).toDouble();
    double   orignal_arrival_rate             = 0;
    //double   orignal_proportion_traffic       = 0;
    //double   orignal_total_proportion_traffic = 0;
    double   orignal_total_arrival_rate = 0;
    double   orignal_absolution_traffic       = 0;
    double   orignal_density                  = 0;
    double   orignal_faction                  = 1;
    double   differ                           = 1;   
    double   differ_arrival_rate              = 1;   
    //double   differ_proportion_traffic      = 0;
    
    //double   modify_proportion_traffic      = 0;
    double   modify_total_arrival_rate        = 0;
    double   modify_arrival_rate              = 0;

#if 0
    double   modify_absolution_traffic        = 0;
    double   modify_density                   = 0;
    double   modify_faction                   = 1;
#endif
    
    orignal_arrival_rate = subnet_class->arrival_rate;
    for( int m_subnet_idx=0; m_subnet_idx<number_subnet[traffic_type_idx]; m_subnet_idx++ )
    {
        subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];
        orignal_total_arrival_rate += subnet_class->arrival_rate;
    }
    orignal_faction = orignal_arrival_rate/orignal_total_arrival_rate ;
    
    if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
        orignal_absolution_traffic = orignal_arrival_rate*(np->traffic_type_list[traffic_type_idx]->mean_time);
    }
    else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
        orignal_absolution_traffic = orignal_arrival_rate*((np->traffic_type_list[traffic_type_idx]->min_time
                    + np->traffic_type_list[traffic_type_idx]->max_time)/2);
    }
    orignal_density = orignal_absolution_traffic/subnet_class->p->comp_bdy_area(); 
    
    switch (col) {
        case 3:
            std::cout << "arrival rate edited..." << std::endl; 
            differ = modify_value - orignal_arrival_rate; 
            differ_arrival_rate = differ;
            modify_total_arrival_rate = differ + total_arrival_rate; 

#if 0
            differ_proportion_traffic = differ*orignal_proportion_traffic/orignal_arrival_rate;
            modify_proportion_traffic = orignal_proportion_traffic + differ_proportion_traffic;
#endif

            sprintf(np->line_buf, "set_subnet -traffic_type %s -subnet %s  -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), np->subnet_list[traffic_type_idx][subnet_idx]->strid,  modify_value );
            np->process_command(np->line_buf);
#if SN_DEBUG
            std::cout << "differ_proportion_traffic " << differ_proportion_traffic << std::endl;
            std::cout << "orignal_proportion_traffic " << orignal_proportion_traffic << std::endl;
            std::cout << "modify_proportion_traffic " << modify_proportion_traffic << std::endl;
            // xxxxx std::cout << "np->traffic_type_list[traffic_type_idx]->arrival_rate " << np->traffic_type_list[traffic_type_idx]->arrival_rate << std::endl;

            sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), modify_total_arrival_rate );
            np->process_command(np->line_buf);
#endif

            update_all_value();
#if 0
            for( int m_subnet_idx=0; m_subnet_idx<number_subnet[traffic_type_idx]; m_subnet_idx++ )
            {
                subnet_class = np->subnet_list[traffic_type_idx][m_subnet_idx];
                orignal_total_arrival_rate += subnet_class->traffic;
            }
#endif
            break;
        case 4:
            std::cout << "offered traffic edited..." << std::endl; 
#if SN_DEBUG
            differ = modify_value - orignal_absolution_traffic;
            differ_arrival_rate = differ*orignal_arrival_rate/orignal_absolution_traffic;
            modify_arrival_rate = differ_arrival_rate + orignal_arrival_rate;
            modify_total_arrival_rate = differ_arrival_rate + total_arrival_rate; 
#endif

            if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                modify_arrival_rate = modify_value/(np->traffic_type_list[traffic_type_idx]->mean_time);
            }
            else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                modify_arrival_rate = modify_value/((np->traffic_type_list[traffic_type_idx]->min_time
                            + np->traffic_type_list[traffic_type_idx]->max_time)/2);
            }

            sprintf(np->line_buf, "set_subnet -traffic_type %s -subnet %s  -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), np->subnet_list[traffic_type_idx][subnet_idx]->strid,  modify_arrival_rate );
            np->process_command(np->line_buf);
#if SN_DEBUG
            std::cout << "differ_proportion_traffic " << differ_proportion_traffic << std::endl;
            std::cout << "orignal_proportion_traffic " << orignal_proportion_traffic << std::endl;
            std::cout << "modify_proportion_traffic " << modify_proportion_traffic << std::endl;
            std::cout << "modify_arrival_rate " << modify_arrival_rate << std::endl;
            std::cout << "total_arrival_rate " << total_arrival_rate << std::endl;
            std::cout << "differ " << differ << std::endl;
            std::cout << "modify_total_arrival_rate " << modify_total_arrival_rate << std::endl;
            // xxxx std::cout << "np->traffic_type_list[traffic_type_idx]->arrival_rate " << np->traffic_type_list[traffic_type_idx]->arrival_rate << std::endl;

            sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), modify_total_arrival_rate );
            np->process_command(np->line_buf);
#endif

            update_all_value();
            
            break;
        case 5:
            std::cout << "traffic density edited..." << std::endl; 
#if SN_DEBUG
            differ = modify_value - orignal_density;
            differ_arrival_rate = differ*orignal_arrival_rate/orignal_density;
            modify_arrival_rate = differ_arrival_rate + orignal_arrival_rate;
            modify_total_arrival_rate = differ_arrival_rate + total_arrival_rate; 
#endif
            subnet_class = np->subnet_list[traffic_type_idx][subnet_idx];
            if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::ExpoDist ) {
                modify_arrival_rate = (modify_value * subnet_class->p->comp_bdy_area() )/(np->traffic_type_list[traffic_type_idx]->mean_time);
            }
            else if ( np->traffic_type_list[traffic_type_idx]->duration_dist == CConst::UnifDist ) {
                modify_arrival_rate = (modify_value * subnet_class->p->comp_bdy_area() )/((np->traffic_type_list[traffic_type_idx]->min_time
                            + np->traffic_type_list[traffic_type_idx]->max_time)/2);
            }


            sprintf(np->line_buf, "set_subnet -traffic_type %s -subnet %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), np->subnet_list[traffic_type_idx][subnet_idx]->strid, modify_arrival_rate );
            np->process_command(np->line_buf);

#if 0
            sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), modify_total_arrival_rate );
            np->process_command(np->line_buf);
#endif
#if SN_DEBUG
            std::cout << "differ_proportion_traffic " << differ_proportion_traffic << std::endl;
            std::cout << "orignal_proportion_traffic " << orignal_proportion_traffic << std::endl;
            std::cout << "modify_proportion_traffic " << modify_proportion_traffic << std::endl;
            // xxxx std::cout << "np->traffic_type_list[traffic_type_idx]->arrival_rate " << np->traffic_type_list[traffic_type_idx]->arrival_rate << std::endl;
#endif

            update_all_value();
            break;
        case 6:
            std::cout << "traffic faction edited..." << std::endl; 
            differ = modify_value - orignal_faction;
            differ_arrival_rate = differ*orignal_arrival_rate/orignal_faction;
            modify_arrival_rate = differ_arrival_rate + orignal_arrival_rate;
            modify_total_arrival_rate = differ_arrival_rate + total_arrival_rate; 


            sprintf(np->line_buf, "set_subnet -traffic_type %s -subnet %s  -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), np->subnet_list[traffic_type_idx][subnet_idx]->strid,  modify_arrival_rate );
            np->process_command(np->line_buf);

#if SN_DEBUG
            std::cout << "differ_proportion_traffic "  << differ_proportion_traffic  << std::endl;
            std::cout << "orignal_proportion_traffic " << orignal_proportion_traffic << std::endl;
            std::cout << "modify_proportion_traffic "  << modify_proportion_traffic  << std::endl;

            sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), modify_total_arrival_rate );
            np->process_command(np->line_buf);
#endif

            update_all_value();
            break;
    }
}

/*
 *  function :  SubnetTraffic::total_val_changed( )
 */
void  SubnetTraffic::total_val_changed( )
{
    double  differ;
    double  differ_arrival_rate;

#if 0
    if ( total_arrival_rate_val->edited() ) {
        sprintf(np->line_buf, "set_traffic -traffic_type %s -arrival_rate %s", np->traffic_type_list[traffic_type_idx]->name(), total_arrival_rate_val->text().latin1() );
        np->process_command(np->line_buf);
    }
#endif

    if (total_arrival_rate_val->edited()) {
        sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), total_arrival_rate_val->text().toDouble());
        np->process_command(np->line_buf);

        update_all_value();
    }

    if (total_traffic_val->edited()) {
        differ =  total_traffic_val->text().toDouble() - total_traffic ; 
        differ_arrival_rate = differ*(total_arrival_rate/total_traffic);
        total_arrival_rate += differ_arrival_rate;
#if SN_DEBUG
        std::cout << "differ " << differ << std::endl; 
        std::cout << "differ_arrival_rate  " << differ_arrival_rate  << std::endl; 
        std::cout << "total_arrival_rate " << total_arrival_rate << std::endl;
#endif

        sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), total_arrival_rate );
        np->process_command(np->line_buf);

        update_all_value();
    }

    if (total_density_val->edited()) {
        differ =  total_density_val->text().toDouble() - total_density ; 
        differ_arrival_rate = differ*(total_arrival_rate/total_density);
        total_arrival_rate += differ_arrival_rate;
#if SN_DEBUG
        std::cout << "differ " << differ << std::endl; 
        std::cout << "differ_arrival_rate  " << differ_arrival_rate  << std::endl; 
        std::cout << "total_arrival_rate " << total_arrival_rate << std::endl;
#endif
        sprintf(np->line_buf, "set_total_traffic -traffic_type %s -arrival_rate %f", np->traffic_type_list[traffic_type_idx]->name(), total_arrival_rate);
        np->process_command(np->line_buf);

        update_all_value();
    }

    if ( np->technology() == CConst::PHS ) {
        if (num_attemp_request_val->edited()) {
            int num_attempt_request  = num_attemp_request_val->text().toInt();
            sprintf(np->line_buf, "set_traffic -traffic_type %s -num_attempt_request %d", np->traffic_type_list[traffic_type_idx]->name(), num_attempt_request);
            np->process_command(np->line_buf);
        }

        if (num_attemp_handover_val->edited()) {
            int num_attempt_handover = num_attemp_handover_val->text().toInt();
            sprintf(np->line_buf, "set_traffic -traffic_type %s -num_attempt_handover %d", np->traffic_type_list[traffic_type_idx]->name(), num_attempt_handover);
            np->process_command(np->line_buf);
        }

        if ( mean_time_val->edited() ) {
            sprintf(np->line_buf, "set_traffic -traffic_type %s -mean_time %s", np->traffic_type_list[traffic_type_idx]->name(), mean_time_val->text().latin1() ); 
            np->process_command(np->line_buf);
        }
        if ( min_time_val->edited() ) {
            sprintf(np->line_buf, "set_traffic -traffic_type %s -min_time %s", np->traffic_type_list[traffic_type_idx]->name(), min_time_val->text().latin1() ); 
            np->process_command(np->line_buf);
        }
        if ( max_time_val->edited() ) {
            sprintf(np->line_buf, "set_traffic -traffic_type %s -max_time %s", np->traffic_type_list[traffic_type_idx]->name(), max_time_val->text().latin1() );
            np->process_command(np->line_buf);
        }
    }

    update_all_value();
}

void SubnetTraffic::all_ckb_selected(int)
{
    if ( ckb->isChecked() ) {
        for ( int i=0; i<number_subnet[traffic_type_idx]; i++ ) {
            ckb_item_vec[i]->setChecked( true );
        }
    } else {
        for ( int i=0; i<number_subnet[traffic_type_idx]; i++ ) {
            ckb_item_vec[i]->setChecked( false );
        }
    }
}



/* 
 *  function : SubnetTraffic::cancel_btn_clicked()
 */
void SubnetTraffic::cancel_btn_clicked()
{
    delete this;
}

#undef SN_DEBUG
