#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <iostream>

#include "qpainter.h"
#include "unusedfreq.h"
#include "phs.h"
#include "wlan.h"
#include "cconst.h"

QWidget *MyCheckTableItem::createEditor() const
{
    ( (MyCheckTableItem*)this )->cb = new QCheckBox( table()->viewport(), "qt_editor_checkbox" );
    cb->setChecked( false );
    cb->setText( text() );
    cb->setTristate ( TRUE );
    cb->setBackgroundColor( table()->viewport()->backgroundColor() );
    QObject::connect( cb, SIGNAL( toggled(bool) ), table(), SLOT( doValueChanged() ) );
    return cb;
}

unusedFreq::unusedFreq(NetworkClass *np_param, SectorClass *sector_param, int mc_num_rows,
        int m_cell_idx, int m_sector_idx, QWidget *parent,const char *name) :
        QDialog( parent, name, true )
{
    int i;
    int mfreq_idx;
    cell_idx = m_cell_idx;
    sector_idx = m_sector_idx;

    resize(390,400);
    setCaption(tr("Unused Frequencies"));

    np       =  np_param;
    sector   = sector_param;
    num_rows = np->num_freq;

    unused_freq_layout = new Q3VBoxLayout(this,10);

    table = new Q3Table( this, "table" );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "Frequency Number" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "Unused" ) );
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, tr( "Used" ) );
    table->setNumCols( 3 );
    table->setColumnWidth( 0, 120);
    if ( np->technology() == CConst::PHS )
        table->setColumnWidth( 1, 100);
    else if ( np->technology() == CConst::WLAN )
        table->setColumnWidth( 1, 200);
    table->setColumnWidth( 2, 100);
    table->setColumnReadOnly( 0, TRUE );
    table->verticalHeader()->hide();
    table->setSelectionMode( Q3Table::Single );
    unused_freq_layout->addWidget( table );

    Q3HBoxLayout *unused_freq_okcancel_layout = new Q3HBoxLayout(this,10);
    unused_freq_okbtn = new QPushButton(tr("Ok"), this);
    unused_freq_okbtn->setGeometry(0,0,50,30);
    unused_freq_okcancel_layout->addWidget(unused_freq_okbtn,0,Qt::AlignBottom);
    unused_freq_okbtn->setMaximumWidth(80);
    unused_freq_cancelbtn = new QPushButton(tr("Cancel"), this);
    unused_freq_cancelbtn->setGeometry(0,0,50,30);
    unused_freq_okcancel_layout->addWidget(unused_freq_cancelbtn,0,Qt::AlignBottom);
    unused_freq_cancelbtn->setMaximumWidth(80);
    unused_freq_layout->addLayout(unused_freq_okcancel_layout);
    

    //------------------------------------------------------------------------- 
    combo_list.clear();
    raw_pm_vec.clear();

    QString        str;
    QStringList    text_list;
    int            unused_freq = 0;
    int            status = 0;

    QColorGroup g;
    QPainter* p = new QPainter(this);
    g.setColor( QColorGroup::Background, Qt::red );
    p->setPen( Qt::red );
    p->setBrush( Qt::red );

    if ( np->technology() == CConst::WLAN )
    {
        WLANSectorClass* m_sector = (WLANSectorClass*) sector;
        table->hideColumn( 2 );
        
        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
            //insert rows and set item
            table->insertRows( mfreq_idx );

            str.sprintf( "%d        ", mfreq_idx+1 ); 
            tab_item = new MyTableItem( table, Q3TableItem::Never, str );
            table->setItem( mfreq_idx, 0, tab_item );
            tab_list.append( tab_item );

            check_item = new QRadioButton( this, "UNUSED FREQUENCY" ) ;
            check_item->setPaletteBackgroundColor( Qt::white );
            check_item->setChecked ( false );
            check_item->setText( "        " );
            check_list.append( check_item );
            table->setCellWidget( mfreq_idx, 1, check_item );

            connect( check_item, SIGNAL( toggled( bool ) ), this, SLOT( update_C( bool ) ) );
        }
        flag = true; // first clicked

        m_chan_idx = m_sector->chan_idx;
        raw_select = m_sector->chan_idx-1;

#if 0
        std ::cout << "read channel index " << m_sector->chan_idx << std::endl;
#endif

        check_list.at( raw_select )->setChecked( true );
        check_list.at( raw_select )->setPaletteBackgroundColor( Qt::red );
        tab_list.  at( raw_select )->setBkgColor( Qt::red );

    }
    else if ( np->technology() == CConst::PHS )
    {
        PHSSectorClass* m_sector = (PHSSectorClass*) sector;

        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
            //insert rows and set item
            table->insertRows( mfreq_idx );

            str.sprintf( "%d        ", mfreq_idx ); 
            tab_item = new MyTableItem( table, Q3TableItem::Never, str );
            table->setItem( mfreq_idx, 0, tab_item );
            tab_list.append( tab_item );

            check_item = new QRadioButton( this, "UNUSED FREQUENCY" ) ;
            check_item->setPaletteBackgroundColor( Qt::white );
            check_item->setChecked ( false );
            check_item->setText( "        " );
            check_list.append( check_item );
            table->setCellWidget( mfreq_idx, 1, check_item );

            check_item1 = new QRadioButton( this, "USED FREQUENCY" ) ;
            check_item1->setPaletteBackgroundColor( Qt::white );
            check_item1->setChecked ( true );
            check_item1->setText( "        " );
            check_list1.append( check_item1 );
            table->setCellWidget( mfreq_idx, 2, check_item1 );

            connect( check_item, SIGNAL( toggled( bool ) ), this, SLOT( update_C( bool ) ) );
            connect( check_item1, SIGNAL( toggled( bool ) ), this, SLOT( update_C1( bool ) ) );
        }

        QRect cr = table->cellRect( 0, 1 );

        for ( i=0; i<m_sector->num_unused_freq; i++ ) {
            unused_freq = m_sector->unused_freq[i];
            if ( np->num_freq >= unused_freq ) {
                check_list.at( unused_freq )->setChecked( true );
            }
        }

        for ( i=0; i<np->num_freq; i++ ) {
            status = check_list.at(i)->isChecked(); 
            raw_pm_vec.push_back( status ); 
        }

        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
            table->paintCell ( p, 0, mfreq_idx, cr, true );
        }
    }
    delete p;

    //------------------------------------------------------------------------- 
    connect( unused_freq_okbtn,     SIGNAL( clicked() ), this, SLOT( unused_freq_okbtn_clicked() ) );
    connect( unused_freq_cancelbtn, SIGNAL( clicked() ), this, SLOT( unused_freq_cancelbtn_clicked() ) );
    connect( table,                 SIGNAL( clicked(int,int,int,const QPoint&) ), 
            this, SLOT( table_valueChanged(int, int, int)));

}

unusedFreq::~unusedFreq()
{
}

/******************************************************************************************/
/**** FUNCTION: unusedFreq::unused_freq_okbtn_clicked()                                ****/
/******************************************************************************************/
void unusedFreq::unused_freq_okbtn_clicked()
{

    if ( np->technology() == CConst::PHS ) {
        PHSSectorClass* m_sector = (PHSSectorClass*) sector ;

        int mfreq_idx, n;

        n = 0;
        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) { 
            if (check_list.at( mfreq_idx )->isChecked()) {
                n++;
            }
        }

        if (m_sector->num_unused_freq) {
            free(m_sector->unused_freq);
        }
        m_sector->num_unused_freq = n;

        m_sector->unused_freq = IVECTOR(m_sector->num_unused_freq);

        n = 0;
        QString str = NULL;
        QString freqlist = NULL;
        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) { 
            if (check_list.at( mfreq_idx )->isChecked()) {
                m_sector->unused_freq[n] = mfreq_idx;
                
                str.sprintf("%d ", mfreq_idx);
                freqlist.append(str);

                n++;
            }
        }

#if 0
        std::cout << "cell "      << cell_idx 
                  << " sector "   << sector_idx
                  << " freqlist " << freqlist.latin1() << "\n" ;
#endif

        sprintf(np->line_buf, "set_unused_freq -sector %d_%d -freq_list \'%s\'",
                cell_idx, sector_idx, freqlist.latin1());
        np->process_command( np->line_buf);
    }
    else if ( np->technology() == CConst::WLAN ) {
        // To be implemented.
        WLANSectorClass* m_sector = (WLANSectorClass*) sector ;

        std::cout << "set channel index \n";
        m_sector->chan_idx = m_chan_idx;
    }

    delete this;
}

void unusedFreq::unused_freq_cancelbtn_clicked()
{
    delete this;
}

void unusedFreq::table_valueChanged(int row, int col, int button)
{
    std::cout << " row " << row << " col " << col << " button " << button << std::endl;
    if ( button == Qt::LeftButton ) {
        switch( col ) {
            case 0 :
                break;

            case 1:
                if ( !check_list.at(row)->isChecked() ) {
                    check_list.at(row)->setPaletteBackgroundColor( Qt::red );
                    tab_list.at(row)->setBkgColor( Qt::red );
                    table->updateCell( row, 0);
                    table->updateCell( row, 1);
                    table->updateCell( row, 2);
                }
                else {
                    check_list.at(row)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
                    tab_list.at(row)->setBkgColor( table->paletteBackgroundColor() );
                    table->updateCell( row, 0);
                    table->updateCell( row, 1);
                    table->updateCell( row, 2);
                }
                break;

            default :
                break;
        }
    }
}

void unusedFreq::update_C( bool state ) {
    std::cout << "Call Function : unusedFreq::update_C() " << state <<  std::endl;
    bool ischecked;
    int  mfreq_idx;
    if ( np->technology() == CConst::PHS ) {
        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
            ischecked = check_list.at( mfreq_idx )->isChecked();
            if ( ischecked == true ) {
                check_list.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red );
                check_list1.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red );
                check_list1.at(mfreq_idx)->setChecked( false );
                tab_list.at(mfreq_idx)->setBkgColor( Qt::red );
                table->updateCell( mfreq_idx, 0);
                table->updateCell( mfreq_idx, 1);
                table->updateCell( mfreq_idx, 2);
            }
            else {
                check_list.at(mfreq_idx)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
                check_list1.at(mfreq_idx)->setPaletteBackgroundColor(  table->paletteBackgroundColor());
                check_list1.at(mfreq_idx)->setChecked( true );
                tab_list.at(mfreq_idx)->setBkgColor( table->paletteBackgroundColor() );
                table->updateCell( mfreq_idx, 0);
                table->updateCell( mfreq_idx, 1);
                table->updateCell( mfreq_idx, 2);
            }
        }
    }
    else if ( np->technology() == CConst::WLAN )
    {
        if ( check_list.at( raw_select )->isChecked() == false ) {
            check_list.at(raw_select)->setPaletteBackgroundColor( Qt::red );
            tab_list.at(raw_select)->setBkgColor( Qt::red );
            table->updateCell( raw_select, 0);
            table->updateCell( raw_select, 1);

            std::cout << " clicked raw \n";
        }
        else
        {
            std::cout << " clicked non raw \n";

            for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
                if ( check_list.at( mfreq_idx )->isChecked() == true ) {
                    std::cout << mfreq_idx << " check_list isChecked() == true \n";

                    if ( flag == true ) {
                        flag = false;
                    }
                    else { 
                        check_list.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red );
                        tab_list.at(mfreq_idx)->setBkgColor( Qt::red );
                        table->updateCell( mfreq_idx, 0);
                        table->updateCell( mfreq_idx, 1);

                        check_list.at(raw_select)->setChecked(false);

                        check_list.at(raw_select)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
                        tab_list.at(raw_select)->setBkgColor( table->paletteBackgroundColor() );
                        table->updateCell( raw_select, 0);
                        table->updateCell( raw_select, 1);

                        raw_select = mfreq_idx;
                        m_chan_idx = mfreq_idx + 1;
                    }
                }
            }
        }

        std::cout << "raw_select " << raw_select << std::endl;
        std::cout << "chan_idx   " << m_chan_idx << std::endl;
#if 0
        for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
            ischecked = check_list.at( mfreq_idx )->isChecked();
            if ( ischecked == true ) {
                check_list.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red );
                tab_list.at(mfreq_idx)->setBkgColor( Qt::red );
                table->updateCell( mfreq_idx, 0);
                table->updateCell( mfreq_idx, 1);
            }
            else {
                check_list.at(mfreq_idx)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
                tab_list.at(mfreq_idx)->setBkgColor( table->paletteBackgroundColor() );
                table->updateCell( mfreq_idx, 0);
                table->updateCell( mfreq_idx, 1);
            }
        }
#endif
    }
}

// Only for PHS service
void unusedFreq::update_C1( bool state ) {
    std::cout << "Call Function : unusedFreq::update_C1() " << state <<  std::endl;
    bool ischecked;
    int  mfreq_idx;
    for ( mfreq_idx=0; mfreq_idx<np->num_freq; mfreq_idx++ ) {
        ischecked = check_list1.at( mfreq_idx )->isChecked();
        if ( ischecked == true ) {
            check_list.at(mfreq_idx)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
            check_list1.at(mfreq_idx)->setPaletteBackgroundColor( table->paletteBackgroundColor() );
            tab_list.at(mfreq_idx)->setBkgColor( table->paletteBackgroundColor());
            check_list.at(mfreq_idx)->setChecked( false );
            table->updateCell( mfreq_idx, 0);
            table->updateCell( mfreq_idx, 1);
            table->updateCell( mfreq_idx, 2);
        }
        else {
            check_list1.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red );
            check_list.at(mfreq_idx)->setPaletteBackgroundColor( Qt::red);
            tab_list.at(mfreq_idx)->setBkgColor( Qt::red );
            check_list.at(mfreq_idx)->setChecked( true );
            table->updateCell( mfreq_idx, 0);
            table->updateCell( mfreq_idx, 1);
            table->updateCell( mfreq_idx, 2);
        }
    }
}

