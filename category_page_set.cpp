#include <qtoolbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <iostream>
#include <fstream>

#include "category_wid.h"
#include "category_page_set.h"
#include "icons_test.h"
#include "psdcaparam.h"
#include "csdcaparam.h"
#include "generalparam.h"
#include "wlan_generalparam.h"

#include "WiSim.h"
#include "wlan.h"
#include "cconst.h"

#define CGDEBUG 0

static void set_caption( CategoryWid* xx, CategoryInterface *c, int i )
{
//    QWidget *w = qApp->mainWidget();
    QWidget *w = xx; 
    if ( !w )
        return;
    QString title = QString("Parameters" );
    title += " - " + c->categoryName( i - c->categoryOffset() );
    w->setCaption( title );
}


CategoryPage::CategoryPage( NetworkClass *np_param, Q3WidgetStack *s, CategoryWid* v_category_wid ) : CategoryInterface( s ), 
    created( FALSE ), np( np_param ), category_wid( v_category_wid )
{
//   category_wid = new CategoryWid; 
//   m_stack = category_wid->stack;
}

CategoryPage::~CategoryPage( )
{
}

QString CategoryPage::categoryName( int i ) const 
{
    switch ( i ) {
        case 0:
            return QString(qApp->translate( "CategoryPage", "General - PHS" ));
            break;
        case 1:
            return QString(qApp->translate( "CategoryPage", "General - WLAN" ));
            break;
        case 2:
            return QString(qApp->translate( "CategoryPage", "PS DCA" ));
            break;
        case 3:
            return QString(qApp->translate( "CategoryPage", "CS DCA" ));
            break;
    }
    return QString::null;
}

QIcon CategoryPage::icon(int i) const
{
    switch (i ) {
        case 0:
            return QPixmap( TestIcon::icon_general );
            break;
        case 1:
            return QPixmap( TestIcon::icon_general);
            break;
        case 2:
            return QPixmap( TestIcon::icon_ps);
            break;
        case 3:
            return QPixmap( TestIcon::icon_cs);
            break;
        default:
            return  QPixmap( QString::null ); 
            break;
    }
}

void CategoryPage::setCurrentCategory( int i)
{
    create();
    m_stack->raiseWidget( i );
    set_caption( category_wid, this, i );
    
    connect( category_wid->g, SIGNAL( clicked( int ) ), category_wid, SLOT( setBkColor(int) ) );

#if CGDEBUG
    for ( int i=0; i<category_wid->g->count(); i++ )
    {    
        printf("test category_wid->g->count()  %d  \n", category_wid->g->count() );

        //category_wid->g->selected()->setPaletteBackgroundColor( green );

        if ( i == category_wid->g->selectedId() )
        {
            printf(" test \n ");
            category_wid->g->selected()->setPaletteBackgroundColor( Qt::green );
        } else {
            category_wid->g->find(i)->setBackgroundMode(Qt::PaletteBase);
        }
    }
#endif

}

#if 0
void CategoryPage::setBkColor( int i )
{
    for ( int j=0; j<category_wid->g->count(); j++ )
    {    
        printf("test category_wid->g->count()  %d  \n", category_wid->g->count() );

        //category_wid->g->selected()->setPaletteBackgroundColor( green );

        if ( j == i )
        {
            printf(" test \n ");
            category_wid->g->find(j)->setPaletteBackgroundColor( Qt::green );
        } else {
            category_wid->g->find(j)->setBackgroundMode(Qt::PaletteBase);
        }
    }
}
#endif

void CategoryPage::create( )
{
#if 0
    std::cout << "CategoryPage::create() \n";
#endif

    if ( created )
        return;
    created = TRUE;

    gp = new generalParam( np, m_stack );
    m_stack->addWidget( gp, categoryOffset() + 0 );

    wgp = new WLANGeneralParam( np, m_stack );
    m_stack->addWidget( wgp, categoryOffset() + 1 );

    pp = new psDcaParam( np, m_stack );
    m_stack->addWidget( pp, categoryOffset() + 2 );

    cp = new csDcaParam( np, m_stack );
    m_stack->addWidget( cp, categoryOffset() + 3 );

    if ( np->technology() == CConst::PHS )
    {
        wgp->setDisabled (true);
    }
    else if ( np->technology() == CConst::WLAN ) {
        gp->setDisabled (true);
        pp->setDisabled (true);
        cp->setDisabled (true);
    }

#if CGDEBUG
    // xxxxxxx
    int cell_idx = 0;
    int ap_idx   = 0;

    bool    flag      = true;
    int     num       = 0;
    double  radius    = 120.0;
    double  threshold = 2000.0;
    double* sir_list  = ( double* ) NULL;

    WLANCellClass*   cell = (WLANCellClass*)   NULL;
    WLANSectorClass* ap   = (WLANSectorClass*) NULL;

    if ( flag ) {
        std::ofstream sir_aps_output;
        sir_aps_output.open("sir_aps.txt");
        if( !sir_aps_output )
            std::cout << "File sir_aps.txt not exist.\n";

        // Calculate SIR
        num = 0;
        for (cell_idx=0; cell_idx<np->num_cell; cell_idx++) {
            cell = (WLANCellClass*) np->cell_list[cell_idx];
            for (ap_idx=0; ap_idx<cell->num_sector; ap_idx++) num ++;
        }

        sir_list = (double*) malloc ( (num+1) * sizeof(double) );
        ((WLANNetworkClass* ) np)->comp_sir_aps( sir_list, radius, threshold );

        // Print results to file.
        for ( int i=0; i<num; i++ ) {
            sir_aps_output << "AP " << i << " SIR " << sir_list[i] << std::endl;
            std::cout      << "AP " << i << " SIR " << sir_list[i] << std::endl;
        }

        sir_aps_output.close();

        flag = false;
    }

    free( sir_list ); sir_list = (double*) NULL;
#endif
}
