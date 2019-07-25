#include <qvariant.h>
#include <q3table.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <q3buttongroup.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <iostream>

#include "sector_prop_table.h"
#include "wisim.h"
#include "phs.h"
#include "cconst.h"

void TableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
    QColorGroup g( cg );

    //if ( row() == table()->numRows() - 1 )
    g.setColor( QColorGroup::Base, Qt::gray );
    Q3TableItem::paint( p, g, cr, selected );
}


QPushButtonTableItem::QPushButtonTableItem( Q3Table *table, const QString &txt )
    : Q3TableItem( table, WhenCurrent, txt )
{
}

void QPushButtonTableItem::paint( QPainter *p, const QColorGroup &cg,
        const QRect &cr, bool selected )
{
    /*
     * how to porting to QT4 ?
     */
    /*
    selected = false;

    p->fillRect( 0, 0, cr.width(), cr.height(),
            selected ? cg.brush( QColorGroup::Highlight )
            : cg.brush( QColorGroup::Base ) );

    int w = cr.width();
    int h = cr.height();

    QSize sz = QSize( table()->style().pixelMetric(QStyle::PM_DefaultFrameWidth),
            table()->style().pixelMetric(QStyle::PM_ButtonShiftVertical) );
    QColorGroup c( cg );
    c.setBrush( QColorGroup::Background, c.brush( QColorGroup::Base ) );
    QStyle::State flags = QStyle::State_None;
    if(isEnabled())
        flags |= QStyle::State_Enabled;
    else
        flags |= QStyle::State_Off;
    if ( isEnabled() && table()->isEnabled() )
        flags |= QStyle::State_Enabled;
    
    table()->style().drawPrimitive( QStyle::PE_ButtonCommand, p,
            QRect( 0, ( cr.height() - sz.height() ) / 2, sz.width(), sz.height() ), c, flags );
    int x = sz.width() + 6;
    w = w - x;
    if ( selected )
        p->setPen( cg.highlightedText() );
    else
        p->setPen( cg.text() );

    p->drawText( x, 0, w, h, wordWrap() ? ( alignment() | Qt::TextWordWrap ) : alignment(), text() );
    */
}


// reimprement
void QPushButtonTableItem::setText( const QString &t )
{
    /*
     * how to porting to QT4?
     *
    Q3TableItem::setText( t );
    QWidget *w = table()->cellWidget( row(), col() );
    QPushButton *pbt = ::qt_cast<QPushButton*>(w);
    if ( pbt )
        pbt->setText( t );
     */
}

QPushButton *QPushButtonTableItem::getWidget()
{
    return (QPushButton*)table()->cellWidget( row(), col() );
    //return pb;
}

// reimprement
QWidget *QPushButtonTableItem::createEditor() const
{
    ( (QPushButtonTableItem*)this )->pb = new QPushButton( table()->viewport(), "qt_editor_pushbutton" );
    pb->setText( text() );
    pb->setBackgroundColor( table()->viewport()->backgroundColor() );
    QObject::connect( pb, SIGNAL( toggled(bool) ), table(), SLOT( doValueChanged() ) );
    return pb;
}

/*
   structor function
 */
SectorPropTable::SectorPropTable( NetworkClass* np_param, QWidget* parent, const char* name ) 
    : QDialog( parent, name, true ), np( np_param )
{
    if ( !name )
    setName( "SectorPropTable" );

    find_unassigned_prop = false;
    have_assigned_prop   = false;
    no_assigned_prop     = true;
    all_unassigned       = false;

    combo_list.setAutoDelete( TRUE );

    SectorPropTableLayout = new Q3GridLayout( this, 1, 1, 11, 15, "SectorPropTableLayout");

    table = new Q3Table( this, "table" );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "Cell Index" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "GW_CSC_CS" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "CSID" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "Choose Propagation Model" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "View Parameters" ) );
    table->setNumCols( 5 );
    table->setColumnWidth( 0, 80  );
    table->setColumnWidth( 1, 80  );
    table->setColumnWidth( 2, 100 );
    table->setColumnWidth( 3, 150 );
    table->setColumnWidth( 4, 60  );
    table->setColumnReadOnly( 0, TRUE );
    table->setColumnReadOnly( 1, TRUE );
    table->setColumnReadOnly( 2, TRUE );
    table->hideColumn( 4 );
    SectorPropTableLayout->addMultiCellWidget( table, 0, 0, 0, 4 );

    //----------------------------------------------------------------------------------
    set_unassigned_prop_model_buttonGroup = new Q3ButtonGroup( this, "set_unassigned_prop_model_buttonGroup" );
    set_unassigned_prop_model_buttonGroup->setColumnLayout(0, Qt::Vertical );
    set_unassigned_prop_model_buttonGroup->layout()->setSpacing( 6 );
    set_unassigned_prop_model_buttonGroup->layout()->setMargin( 11 );
    set_unassigned_prop_model_buttonGroupLayout = new Q3HBoxLayout( set_unassigned_prop_model_buttonGroup->layout() );
    set_unassigned_prop_model_buttonGroupLayout->setAlignment( Qt::AlignTop );

    set_unassigned_prop_model_lbl = new QLabel( set_unassigned_prop_model_buttonGroup, "set_unassigned_prop_model_lbl" );
    set_unassigned_prop_model_buttonGroupLayout->addWidget( set_unassigned_prop_model_lbl );

    QSpacerItem* spacer4 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    set_unassigned_prop_model_buttonGroupLayout->addItem( spacer4 );

    set_unassigned_prop_model_btn = new QPushButton( set_unassigned_prop_model_buttonGroup, "set_unassigned_prop_model_btn" );
    set_unassigned_prop_model_btn->setMaximumSize( QSize( 50, 32767 ) );
    set_unassigned_prop_model_buttonGroupLayout->addWidget( set_unassigned_prop_model_btn );

#if 0
    QSpacerItem* spacer5 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    set_unassigned_prop_model_buttonGroupLayout->addItem( spacer5 );
#endif

    SectorPropTableLayout->addMultiCellWidget( set_unassigned_prop_model_buttonGroup, 1, 1, 0, 4 );

    //----------------------------------------------------------------------------------
    set_all_unassigned = new Q3ButtonGroup( this, "set_all_unassigned" );
    set_all_unassigned->setColumnLayout(0, Qt::Vertical );
    set_all_unassigned->layout()->setSpacing( 6 );
    set_all_unassigned->layout()->setMargin( 11 );
    set_all_unassignedLayout = new Q3HBoxLayout( set_all_unassigned->layout() );
    set_all_unassignedLayout->setAlignment( Qt::AlignTop );

    set_all_unassigned_lbl = new QLabel( set_all_unassigned, "set_all_unassigned_lbl" );
    set_all_unassignedLayout->addWidget( set_all_unassigned_lbl );

    QSpacerItem* spacer6 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    set_all_unassignedLayout->addItem( spacer6 );

    set_all_unassigned_btn = new QPushButton( set_all_unassigned, "set_all_unassigned_btn" );
    set_all_unassigned_btn->setMaximumSize( QSize( 50, 32767 ) );
    set_all_unassignedLayout->addWidget( set_all_unassigned_btn );

#if 0
    QSpacerItem* spacer7 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    set_all_unassignedLayout->addItem( spacer7 );
#endif

    SectorPropTableLayout->addMultiCellWidget( set_all_unassigned, 2, 2, 0, 4 );

    //----------------------------------------------------------------------------------
    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    SectorPropTableLayout->addItem( spacer1, 3, 0 );
    ok_btn = new QPushButton( this, "close_btn" );
    SectorPropTableLayout->addWidget( ok_btn, 3, 1 );
    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    SectorPropTableLayout->addItem( spacer2, 3, 2 );
    cancel_btn = new QPushButton( this, "close_btn" );
    SectorPropTableLayout->addWidget( cancel_btn, 3, 3 );
    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    SectorPropTableLayout->addItem( spacer3, 3, 4 );

    //----------------------------------------------------------------------------------
    combo_list.clear();
    raw_pm_vec.clear();

    QString        namestr;
    //QStringList    text_list;
    CellClass      *cell;
    PHSSectorClass *sector;
    int sector_cnt = 0;

    for( int pm_idx=0; pm_idx<np->num_prop_model; pm_idx++) {
        namestr = np->prop_model_list[pm_idx]->get_strid();
        text_list.append( namestr );
        //std::cout << " debug text_list : " << text_list.count() << std::endl;
    }

    text_list.append( "UNASSIGNED" );

    for ( int  cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
        cell = np->cell_list[cell_idx];

        for ( int sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];

            //insert rows and set item
            table->insertRows( sector_cnt );

            namestr.sprintf( "  %d_%d", cell_idx, sector_idx ); 
            tab_item = new TableItem( table, Q3TableItem::Never, namestr );
            table->setItem( sector_cnt, 0, tab_item );
            //table->setText( sector_cnt, 0, namestr );

            namestr.sprintf( "  %.6d    ", sector->gw_csc_cs );
            tab_item = new TableItem( table, Q3TableItem::Never, namestr );
            table->setItem( sector_cnt, 1, tab_item );
            //table->setText( sector_cnt, 1, namestr );

            char *hexstr = CVECTOR( 2*sector->csid_byte_length );
            if ( np->technology() == CConst::PHS )
                hex_to_hexstr( hexstr, sector->csid_hex, sector->csid_byte_length );
            namestr.sprintf( "  %s ", hexstr );
            tab_item = new TableItem( table, Q3TableItem::Never, namestr );
            table->setItem( sector_cnt, 2, tab_item );
            //table->setText( sector_cnt, 2, namestr );
            free(hexstr);

#if CGDEBUG
            std::cout << "debug sector->prop_model : " << sector->prop_model << std::endl;
#endif

#if CGDEBUG
            if ( sector->prop_model < 0 ) {                
                find_unassigned_prop = true;
                
                combo_item = new Q3ComboTableItem( table, text_list, false );

                combo_item->setCurrentItem ( text_list.count() - 1 );
                combo_list.append( combo_item );
                raw_pm_vec.push_back( text_list.count() - 1 );
            } else {
                
                combo_item = new Q3ComboTableItem( table, text_list, false );

                combo_item->setCurrentItem ( sector->prop_model );
                combo_list.append( combo_item );
                raw_pm_vec.push_back(sector->prop_model);
            }
#endif

            //text_list.append( "UNASSIGNED" );
            combo_item = new Q3ComboTableItem( table, text_list, false );
            combo_list.append( combo_item );

            if ( sector->prop_model < 0 ) {                
                find_unassigned_prop = true;

                combo_item->setCurrentItem ( text_list.count() - 1 );
                raw_pm_vec.push_back( text_list.count() - 1 );
            } else {
                no_assigned_prop = false;

                combo_item->setCurrentItem ( sector->prop_model );
                raw_pm_vec.push_back(sector->prop_model);
            }
            table->setItem( sector_cnt, 3, combo_item ); 

            btn_item = new QPushButtonTableItem( table, "" );
            btn_item->setText( tr(" Click") );
            //btn_item->getWidget()->setText( tr(" Click") );
            table->setItem( sector_cnt, 4, btn_item ); 

            sector_cnt++;
        }
    }
    //-------------------------------------------------------------------------------------------------

    //signal and slot
    connect( table,      SIGNAL(valueChanged( int, int )), this, SLOT( combo_selected( int, int ) ) );
    connect( cancel_btn, SIGNAL( clicked() ),              this, SLOT( cancel_btn_clicked() ) );
    connect( ok_btn,     SIGNAL( clicked() ),              this, SLOT( ok_btn_clicked() ) );
    connect( set_unassigned_prop_model_btn, SIGNAL( clicked() ), this, SLOT( set_unassigned_prop_model_btn_clicked() ) );
    connect( set_all_unassigned_btn,        SIGNAL( clicked() ), this, SLOT( set_all_unassigned_btn_clicked() ) );

    languageChange();
    resize( QSize(526, 304).expandedTo(minimumSizeHint()) );

    exec();
}


SectorPropTable::~SectorPropTable()
{
}

void SectorPropTable::languageChange()
{
    setCaption( tr( "Assign Propagation Model" ) );
    table->horizontalHeader()->setLabel( 0, tr( "Cell Index" ) );
    table->horizontalHeader()->setLabel( 1, tr( "GW_CSC_CS" ) );
    table->horizontalHeader()->setLabel( 2, tr( "CSID" ) );
    table->horizontalHeader()->setLabel( 3, tr( "Propagation Model" ) );

    set_unassigned_prop_model_lbl->setText( tr( "Set Unassigned Propagation Model") );
    set_all_unassigned_lbl->setText( tr("Let all sectors unassigned") ); 
    set_unassigned_prop_model_btn->setText( tr( "&Apply") );
    set_all_unassigned_btn->setText( tr("A&pply") ); 
    set_unassigned_prop_model_btn->setAccel( QKeySequence( "Alt+K" ) );

    cancel_btn->setText( tr( "&Cancel" ) );
    cancel_btn->setAccel( QKeySequence( "Alt+C" ) );

    ok_btn->setText( tr( "&Ok" ) );
    ok_btn->setAccel( QKeySequence( "Alt+O" ) );
}

void SectorPropTable::ok_btn_clicked()
{
        int         itemIndex;
        int         sector_cnt = 0;
        int         cell_idx = 0;
        int         sector_idx = 0;
        CellClass   *cell;           
        SectorClass *sector;        

        for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
            cell = np->cell_list[cell_idx];
            for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                sector = cell->sector_list[sector_idx];
                itemIndex = combo_list.at(sector_cnt)->currentItem();

                if( raw_pm_vec[sector_cnt] != itemIndex ) {
                    if ( itemIndex != combo_list.at(sector_cnt)->count() - 1 ) {
                        sprintf(np->line_buf,"set_sector -sector %d_%d -prop_model %d",cell_idx,sector_idx,itemIndex );
                        np->process_command(np->line_buf);
                    } else {
                        sprintf(np->line_buf,"set_sector -sector %d_%d -prop_model -1",cell_idx,sector_idx );
                        np->process_command(np->line_buf);
                    }
                }
                sector_cnt++;
            }
        }

        combo_list.clear();
        raw_pm_vec.clear();
        text_list.clear();

        delete this;
}

void SectorPropTable::cancel_btn_clicked()
{
    combo_list.clear();
    raw_pm_vec.clear();
    text_list.clear();

    delete this;
}


void SectorPropTable::combo_selected(int sector_idx, int col )
{
    int m_sector_idx = 0;
    int m_col        = 0;
    m_sector_idx = sector_idx;
    m_col        = col;

#if 0
    //process_command
    switch ( col ) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
    }
#endif

}

void SectorPropTable::set_unassigned_prop_model_btn_clicked()
{
    sprintf(np->line_buf, "set_unassigned_prop_model" );
    np->process_command(np->line_buf);

    //if there are more than one unassigned propagation models
    //and if didn't execute above command yet
    //and if not all cells without propagation
    if ( find_unassigned_prop && !have_assigned_prop && !no_assigned_prop ) { 
        //int         itemIndex  = 0;
        int         sector_cnt = 0;
        int         cell_idx   = 0;
        int         sector_idx = 0;
        CellClass   *cell;
        SectorClass *sector;

        for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
            cell = np->cell_list[cell_idx];
            for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                sector = cell->sector_list[sector_idx];
                combo_list.at(sector_cnt)->setCurrentItem( sector->prop_model );
                //combo_list.at(sector_cnt)->setCurrentItem( 0 );
                sector_cnt++;
            }
        }
        
        have_assigned_prop = true;
    } else {
        QMessageBox::warning( this, "Warning",
                QString( tr("No sector with unassigned propagation model found")),
                QMessageBox::Warning,
                0
                );
    }
}

void SectorPropTable::set_all_unassigned_btn_clicked()
{
    if ( !all_unassigned ) { 
        int sector_cnt = 0;
        int cell_idx   = 0;
        int sector_idx = 0;
        CellClass   *cell;

        for ( cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
            cell = np->cell_list[cell_idx];
            for ( sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                combo_list.at(sector_cnt)->setCurrentItem( np->num_prop_model );
                sector_cnt++;
            }
        }
        all_unassigned = true;
    } else 
        QMessageBox::warning( this, "Warning",
                QString( tr("All sectors have unassigned propagation model")),
                QMessageBox::Warning, 0 );
}
