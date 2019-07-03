/******************************************************************************************/
/**** FILE: info_window.cpp                                                            ****/
/******************************************************************************************/

#include <math.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qfont.h>
#include <qevent.h>
#include <q3textedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <QShowEvent>
#include <QHideEvent>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QCloseEvent>

#include "gconst.h"
#include "icons.h"
#include "WiSim.h"
#include "info_window.h"
#include "list.h"
#include "strint.h"

#include "set_language.h"

extern QFont *application_font;
extern QFont *fixed_width_font;

/******************************************************************************************/
/**** FUNCTION: InfoWindow::InfoWindow                                           ****/
/******************************************************************************************/
InfoWindow::InfoWindow(NetworkClass *np_param, QWidget* parent, const char* name) : QWidget(parent, name)
{
    np = np_param;

//    setPaletteBackgroundColor(QColor(192,192,255));

    command_text = new Q3TextEdit( this );
    command_text->setTextFormat(Qt::PlainText);
    command_text->setReadOnly(TRUE);
    command_text->setFont(*fixed_width_font);
    command_text->setWordWrap(Q3TextEdit::NoWrap);

    QIcon pop_out_iconset;
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    pop_out_iconset.setPixmap(QPixmap(XpmIcon::pop_out_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    QIcon win_close_iconset;
    win_close_iconset.setPixmap(QPixmap(XpmIcon::win_close_icon_normal),   QIcon::Small, QIcon::Normal,   QIcon::Off);
    win_close_iconset.setPixmap(QPixmap(XpmIcon::win_close_icon_disabled), QIcon::Small, QIcon::Disabled, QIcon::Off);

    header = new QWidget(this);
    pop_btn    = new QPushButton(QString::null, header);
    cancel_btn = new QPushButton(QString::null, header);
    pop_btn->setIconSet( pop_out_iconset );
    cancel_btn->setIconSet( win_close_iconset );
    pop_btn->setFixedSize(QSize(16,16));
    cancel_btn->setFixedSize(QSize(16,16));

    Q3VBoxLayout *vbox = new Q3VBoxLayout( this );
    header->setPaletteBackgroundColor(QColor(105,137,188));
    vbox->addWidget(header);
    Q3HBoxLayout *htop = new Q3HBoxLayout( header );
    vbox->addWidget(command_text);

    QLabel *win_lbl = new QLabel(tr("Information Window"), header);
    win_lbl->setPaletteForegroundColor(QColor(255,255,255));

    win_lbl->setFont(*application_font);
    htop->addWidget(win_lbl, 0, Qt::AlignLeft);
    htop->addStretch(100);
    htop->setMargin(2);
    htop->addWidget(pop_btn,         0, Qt::AlignRight);
    htop->addSpacing(2);
    htop->addWidget(cancel_btn,      0, Qt::AlignRight);

    connect(pop_btn,    SIGNAL(released()), SIGNAL(pop_signal()));
    connect(cancel_btn, SIGNAL(released()), SIGNAL(hide_signal()));

    setCaption(tr("Info Window"));

    setGeometry(200, 600, 800, 300);

    hide();
}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::~InfoWindow                                                ****/
/******************************************************************************************/
InfoWindow::~InfoWindow()
{
}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::setText                                                    ****/
/******************************************************************************************/
void InfoWindow::setText(QString s)
{
    command_text->setText(s);
}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::hideEvent                                                  ****/
/******************************************************************************************/
void InfoWindow::hideEvent(QHideEvent*)
{
    emit win_vis_changed(GConst::visHide);
}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::showEvent                                                  ****/
/******************************************************************************************/
void InfoWindow::showEvent(QShowEvent*)
{
    emit win_vis_changed(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::closeEvent                                                 ****/
/******************************************************************************************/
void InfoWindow::closeEvent( QCloseEvent* ce )
{
    ce->ignore();
    if (!np->error_state) {
        hide();
    }

}
/******************************************************************************************/
/**** FUNCTION: InfoWindow::update_selection_change                                    ****/
/******************************************************************************************/
void InfoWindow::update_selection_change( ListClass<int> *select_cell_list )
{
    int i, cell_idx, num_digit, cell_name_idx, cell_name_pref;
    CellClass *cell;
    SectorClass *sector;
    char *chptr;
    QString qstr = "";

    if (select_cell_list->getSize()) {
        qstr += QString("%1").arg(select_cell_list->getSize()) + " " + tr("cells selected") + "\n";
    }

    if (np->num_cell >= 2) {
        num_digit = (int) ceil( log((double) np->num_cell-1) / log(10.0) );
    } else {
        num_digit = 1;
    }

    for (i=0; i<=select_cell_list->getSize()-1; i++) {
        cell_idx = (*select_cell_list)[i];
        cell = np->cell_list[cell_idx];
        qstr += QString("(%1) %2 ").arg(i+1,num_digit).arg(cell_idx, num_digit);
        for (cell_name_idx=1; cell_name_idx<=np->report_cell_name_opt_list->getSize()-1; cell_name_idx++) {
            cell_name_pref = (*(np->report_cell_name_opt_list))[cell_name_idx].getInt();
            chptr = cell->view_name(cell_idx, cell_name_pref);
            qstr += QString(" ") + QString(chptr);
        }
        qstr += "\n";
    }

    setText(qstr);
}
/******************************************************************************************/
