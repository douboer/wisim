#include <qvariant.h>
#include <q3header.h>
#include <q3listview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include <qstringlist.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "cvg_part_disp.h"

/*
 *  Constructs a disp_cells as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
disp_cells::disp_cells( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
  setModal(true);
    if ( !name )
	setName( "disp_cells" );
    disp_cellsLayout = new Q3VBoxLayout( this, 11, 6, "disp_cellsLayout"); 

    listView4 = new Q3ListView( this, "listView4" );
    listView4->addColumn( tr( "Choosed Cells" ) );
    listView4->header()->setClickEnabled( FALSE, listView4->header()->count() - 1 );
    listView4->setResizePolicy( Q3ListView::AutoOne );
    listView4->setResizeMode( Q3ListView::NoColumn );
    disp_cellsLayout->addWidget( listView4 );
    spacer5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    disp_cellsLayout->addItem( spacer5 );

    layout22 = new Q3HBoxLayout( 0, 0, 6, "layout22"); 
    spacer6 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout22->addItem( spacer6 );

    pushButton6 = new QPushButton( this, "pushButton6" );
    layout22->addWidget( pushButton6 );
    disp_cellsLayout->addLayout( layout22 );
    languageChange();
    resize( QSize(387, 464).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( pushButton6, SIGNAL( clicked() ), this, SLOT( close() ) );

}

/*
 *  Destroys the object and frees any allocated resources
 */
disp_cells::~disp_cells()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void disp_cells::languageChange()
{
    setCaption( tr( "Display Choosed Cells" ) );
    listView4->header()->setLabel( 0, tr( "Choosed Cells" ) );
    pushButton6->setText( tr( "&OK" ) );
    pushButton6->setAccel( QKeySequence( tr( "Alt+O" ) ) );
}

void disp_cells::init() {
  //QString s = "cell0*cell1*cell2";
  //QStringList content  = QStringList::split( "*", s );
    for( unsigned int i = 0; i < m_Display.size(); i++)
       (void) new Q3ListViewItem( listView4, m_Display[i]);
    
}


void disp_cells::setCellList(const QStringList& clist) {

  m_Display = clist;
  init();

}
