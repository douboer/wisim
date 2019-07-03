#include "category_wid.h"

#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3accel.h>
#include <qtoolbox.h>
#include <qpainter.h>
#include <q3dockwindow.h>

#include <qaction.h>
#include <qsignalmapper.h>
#include <qdir.h>
#include <q3buttongroup.h>
#include <qtoolbutton.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3Frame>
#include <Q3PtrList>


CategoryWid::CategoryWid( NetworkClass *np,  QWidget *parent, const char *name )
    : QDialog( parent, name )
{
    MyDialogLayout = new Q3GridLayout( this, 1, 1, 11, 6, "MyDialogLayout"); 

    /*   
         begin ok & cancel button layout
     */
    OkCancelLayout = new Q3HBoxLayout( 0, 0, 6, "OkCancelLayout"); 

    spacer1 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    OkCancelLayout->addItem( spacer1 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    OkCancelLayout->addWidget( buttonOk );

    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    OkCancelLayout->addItem( spacer2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    OkCancelLayout->addWidget( buttonCancel );

    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    OkCancelLayout->addItem( spacer3 );

    MyDialogLayout->addMultiCellLayout( OkCancelLayout, 1, 1, 0, 1 );

    /*   
         begin dock window layout which contain toolbox
     */
    toolBox = new QToolBox( this );

    toolBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 3, 0, toolBox->sizePolicy().hasHeightForWidth() ) );
    toolBox->setPaletteBackgroundColor( paletteBackgroundColor() );
    toolBox->setFrameShape( QToolBox::StyledPanel );
    toolBox->setFrameShadow( QToolBox::Sunken );

    MyDialogLayout->addWidget( toolBox, 0, 0 );

    /*     
         begin windowStack item
     */
    stack = new Q3WidgetStack( this );
    stack->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 9, 0, stack->sizePolicy().hasHeightForWidth() ) );

    category_page = new CategoryPage( np, stack, this );

    category_list( category_page );

    MyDialogLayout->addWidget( stack, 0, 1 );
    languageChange();
    resize( QSize(600, 382).expandedTo(minimumSizeHint()) );

    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );

    exec();
}

CategoryWid::~CategoryWid()
{
    if( category_page ) {  
         delete category_page; }  
}

void CategoryWid::languageChange()
{
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
}

void CategoryWid::setCategories( const Q3PtrList <CategoryInterface> &v_categories )
{
    categories = v_categories;

    for ( int i = 0; i < (int)categories.count(); ++i )
        toolBox->addItem( createCategoryPage( categories.at(i) ),
                          //categories.at(i)->icon(),
                          categories.at(i)->name() );

    categories.first()->setCurrentCategory( 0 );
}

QWidget *CategoryWid::createCategoryPage( CategoryInterface *v_category )
{
    g = new Q3ButtonGroup( 1, Qt::Horizontal, toolBox );
    g->setFrameStyle( Q3Frame::NoFrame );
    g->setEraseColor(Qt::green);
    g->setBackgroundMode(Qt::PaletteBase);
    g->setPaletteBackgroundColor( Qt::white );    //paletteBackgroundColor() );
    for ( int i = 0; i < v_category->numCategories(); ++i ) {
        QToolButton *b = new QToolButton( g );
        b->setBackgroundMode(Qt::PaletteBase);
        b->setTextLabel( v_category->categoryName( i ) );
        b->setIconSet( v_category->icon(i));  //c->categoryIcon( i ) );
        b->setAutoRaise( TRUE );
        b->setTextPosition( QToolButton::BesideIcon );
        b->setUsesTextLabel( TRUE );

        // 2007-3-28 - hide wlan tool button
        if ( i==1)
            b->hide();

        g->insert( b, i + v_category->categoryOffset() );
        connect( g, SIGNAL( clicked( int ) ), v_category, SLOT( setCurrentCategory( int ) ) );
    }

    //initial
    g->find(0)->setPaletteBackgroundColor( Qt::blue );

    return g;
}


void CategoryWid::category_list( CategoryPage* v_widget_category )
{
//    QPtrList <CategoryInterface> categories;
    categories.append( v_widget_category );
    setCategories( categories );
}

void CategoryWid::setBkColor(int i)
{
    for ( int j=0; j<g->count(); j++ )
    {
        //printf("test g->count()  %d  \n", g->count() );
        //category_wid->g->selected()->setPaletteBackgroundColor( green );
        if ( j == i )
        {
            g->find(j)->setPaletteBackgroundColor( Qt::blue );
        } else {
            //g->find(j)->setBackgroundMode(PaletteBase);
            g->find(j)->setPaletteBackgroundColor( Qt::white );
        }
    }
}

void CategoryWid::ok_btn_clicked()
{
    hide();
    
    //call slot function
    category_page->gp ->ok_btn_clicked();
    category_page->wgp->ok_btn_clicked();
    category_page->pp ->ok_btn_clicked();
    category_page->cp ->ok_btn_clicked();

    delete this;
}

void CategoryWid::cancel_btn_clicked()
{
    hide();
    delete this;
}
