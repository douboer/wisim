
/******************************************************************************************
**** PROGRAM: set_strid_dia.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#include <stdlib.h>
#include <qvariant.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3CString>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "WiSim.h"
#include "coverage.h"
#include "gconst.h"
#include "set_strid_dia.h"
#include "traffic_type.h"

/******************************************************************************************/
/**** FUNCTION: SetStridDiaClass::SetStridDiaClass                                               ****/
/******************************************************************************************/
SetStridDiaClass::SetStridDiaClass( int s_rtti, int s_index, int s_p_idx, NetworkClass *s_np, QWidget* parent, const char* name) : QDialog( parent, name, true), np(s_np), rtti(s_rtti), index(s_index), p_idx(s_p_idx)
{
    setCaption( name );
    // resize( 460, 100 );

    Q3VBoxLayout *set_strid_dia_vlayout = new Q3VBoxLayout(this, 6, 11); 
    Q3GridLayout *grid_layout     = new Q3GridLayout((QWidget *) 0, 1, 2, 10);
    Q3HBoxLayout *set_strid_dia_hlayout = new Q3HBoxLayout(0, 6, 11); 

    QLabel *set_strid_dia_name_lbl = new QLabel( tr("New Name") + " : ", this );     
    grid_layout->addWidget(set_strid_dia_name_lbl, 0, 0);
    set_strid_dia_name_val = new QLineEdit(this);
    grid_layout->addWidget(set_strid_dia_name_val, 0, 1);

    QPushButton *ok_btn = new QPushButton(tr("&Ok"), this);
    set_strid_dia_hlayout->addWidget(ok_btn, Qt::AlignCenter);
    ok_btn->setMaximumWidth(80);

    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), this);
    set_strid_dia_hlayout->addWidget(cancel_btn, Qt::AlignCenter);
    cancel_btn->setMaximumWidth(80);

    set_strid_dia_vlayout->addLayout(grid_layout,     Qt::AlignHCenter);
    set_strid_dia_vlayout->addLayout(set_strid_dia_hlayout, Qt::AlignHCenter);

    connect( ok_btn,     SIGNAL( clicked() ), this, SLOT( ok_btn_clicked() ) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT( cancel_btn_clicked() ) );

    switch(rtti) {
        case GConst::subnetRTTI:
            set_strid_dia_name_val->setText(np->subnet_list[p_idx][index]->strid);
            break;
        case GConst::coverageRTTI:
            set_strid_dia_name_val->setText(np->coverage_list[index]->strid);
            break;
        default:
            CORE_DUMP;
            break;
    }

    set_strid_dia_name_val->selectAll();

    exec();
}  
/******************************************************************************************/
/**** FUNCTION: SetStridDiaClass::cancel_btn_clicked                                   ****/
/******************************************************************************************/
void SetStridDiaClass::cancel_btn_clicked()
{
    delete this;
}
/******************************************************************************************/
/**** FUNCTION: SetStridDiaClass::ok_btn_clicked                                       ****/
/******************************************************************************************/
void SetStridDiaClass::ok_btn_clicked()
{
    char *chptr;

    chptr = np->line_buf;

    switch(rtti) {
        case GConst::subnetRTTI:
            chptr += sprintf(chptr, "set_subnet -traffic_type '%s' -subnet '%s' -strid ",
                             np->traffic_type_list[p_idx]->name(), np->subnet_list[p_idx][index]->strid);
            break;
        case GConst::coverageRTTI:
            chptr += sprintf(chptr, "set_coverage_analysis -name '%s' -strid ", np->coverage_list[index]->strid);
            break;
        default:
            CORE_DUMP;
            break;
    }

    if (!set_strid_dia_name_val->text().isEmpty()) {
        Q3CString qcs(2*set_strid_dia_name_val->text().length());
        qcs = set_strid_dia_name_val->text().local8Bit();

        chptr += sprintf(chptr, "'%s'", (const char *) qcs);
        np->process_command(np->line_buf);
    }

    delete this;
}
/******************************************************************************************/
