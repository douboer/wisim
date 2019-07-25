#include <qvariant.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <iostream>

#include "wlan_generalparam.h"
#include "wisim.h"
#include "cconst.h"
#include "phs.h"


WLANGeneralParam::WLANGeneralParam( NetworkClass* np_param, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    np = np_param;
    
    if ( !name )
    setName( "WLANGeneralParam" );

    Q3VBoxLayout* WLANGeneralParamLayout = new Q3VBoxLayout( this );

    info_lbl = new QLabel( this, "info_lbl" );
    info_lbl->setText( "NOT SUPPORTING IN THIS DEMO VERSION YET! " );
    info_lbl->setAlignment( Qt::AlignCenter );

    WLANGeneralParamLayout->insertWidget( 0, info_lbl );
}

/*
 */
WLANGeneralParam::~WLANGeneralParam()
{
}

void WLANGeneralParam::ok_btn_clicked()
{
    std::cout << "WLAN GUI, Not implement yet!\n";
}
