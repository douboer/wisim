/******************************************************************************************/
/**** FILE: command_window.cpp                                                         ****/
/******************************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qfont.h>
#include <qevent.h>
#include <q3textedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <QShowEvent>
#include <QPixmap>
#include <QHideEvent>
#include <QKeyEvent>
#include <Q3VBoxLayout>

#include "gconst.h"
#include "icons.h"
#include "wisim.h"
#include "command_window.h"
#include "set_language.h"

extern QFont *application_font;
extern QFont *fixed_width_font;

/******************************************************************************************/
/**** FUNCTION: CommandWindow::CommandWindow                                           ****/
/******************************************************************************************/
CommandWindow::CommandWindow(NetworkClass *np_param, QWidget* parent, const char* name) : QWidget(parent, name)
{
    np = np_param;
    past_command_index = command_index = 0;
    command_number=0;

    command_text = new Q3TextEdit( this );
    command_text->setTextFormat(Qt::PlainText);
    command_text->setReadOnly(TRUE);
    command_text->setFont(*fixed_width_font);
    command_text->setWordWrap(Q3TextEdit::NoWrap);
    // command_text->setMargin(5);

    cle_prompt = new QLabel( this );
    cle_prompt->setText(np->prompt);
    cle_prompt->setFont(*fixed_width_font);
    
    command_line = new QLineEdit( this );
    command_line->setFont(*fixed_width_font);
    command_line->setFocus();

    clear_button = new QPushButton( tr("&Clear Error State"), this, "clear error" );
    clear_button->setEnabled( FALSE ); 

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

    connect(command_line, SIGNAL( returnPressed() ), this, SLOT( get_gui_command() ));
    connect(clear_button, SIGNAL( clicked() ),       this, SLOT( clear_error_state() ));

    Q3VBoxLayout *vbox = new Q3VBoxLayout( this );
    header->setPaletteBackgroundColor(QColor(105,137,188));
    vbox->addWidget(header);
    Q3HBoxLayout *htop = new Q3HBoxLayout( header );
    vbox->addWidget(command_text);
    Q3HBoxLayout *hbot = new Q3HBoxLayout( vbox );

    QLabel *win_lbl = new QLabel(tr("Command Window"), header);
    win_lbl->setPaletteForegroundColor(QColor(255,255,255));
    win_lbl->setFont(*application_font);
    htop->addWidget(win_lbl, 0, Qt::AlignLeft);
    htop->addStretch(100);
    htop->setMargin(2);
    htop->addWidget(pop_btn,         0, Qt::AlignRight);
    htop->addSpacing(2);
    htop->addWidget(cancel_btn,      0, Qt::AlignRight);

    hbot->addWidget(cle_prompt);
    hbot->addWidget(command_line);
    hbot->addWidget(clear_button);

    connect(pop_btn,    SIGNAL(released()), SIGNAL(pop_signal()));
    connect(cancel_btn, SIGNAL(released()), this, SLOT(hide()));

    setCaption(tr("Command Window"));

    setGeometry(200, 600, 800, 300);

    fl = fopen("wisim_cmdwin.log", "w");

    hide();
}
/******************************************************************************************/
/**** FUNCTION: CommandWindow::~CommandWindow                                          ****/
/******************************************************************************************/
CommandWindow::~CommandWindow()
{
    if (fl) {
        fclose(fl);
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CommandWindow::setEnabledButtons                                       ****/
/******************************************************************************************/
void CommandWindow::setEnabledButtons(bool enable)
{
    pop_btn->setEnabled(enable);
    cancel_btn->setEnabled(enable);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CommandWindow::clear_error_state                                       ****/
/******************************************************************************************/
void CommandWindow::clear_error_state()
{
     sprintf(np->line_buf, "clear_error_state");
     np->process_command(np->line_buf);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CommandWindow::save_gui_command                                        ****/
/******************************************************************************************/
void CommandWindow::save_gui_command(char *cmd)
{
     if ( command_number<10 )
     {
         command_line_string[command_index].sprintf("%s",cmd);
         command_index++;
         past_command_index=command_index-1;
         command_number++;
     }
     else  {
         for ( int i=1; i<=9; i++ )
             command_line_string[i-1]=command_line_string[i];
         command_line_string[9].sprintf("%s",cmd);
         command_index=10;
         past_command_index = 9;
     }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CommandWindow::keyPressEvent                                           ****/
/******************************************************************************************/
void CommandWindow::keyPressEvent(QKeyEvent *q)
{
     bool key_flag;
     switch ( q->key() ) {
       case Qt::Key_Up : 
           key_flag = TRUE; 
           get_last_command(key_flag);
           break;
       case Qt::Key_Down : 
	   key_flag = FALSE; 
	   get_last_command(key_flag);
            break;
       default : break;
    }  
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CommandWindow::get_last_command                                        ****/
/******************************************************************************************/
void CommandWindow::get_last_command( bool Up )
{
     static int up_flag=0;
     if ( Up == TRUE )
     {
        if ( ( up_flag == 1 ) && ( past_command_index <=8 ) && ( past_command_index < command_index-1 ) )
            past_command_index--;
        up_flag = 0;
        if (past_command_index >= 0 )
        {
           command_line->setText(command_line_string[past_command_index]);
           if (past_command_index>0) past_command_index--;
        }
     }
     else if ( Up == FALSE )
     {
        if ( ( up_flag == 0 ) && ( past_command_index >= 1 ) )
            past_command_index++;
        up_flag = 1;
        if ((past_command_index <= (command_index-1))&&(past_command_index<=9))
        {
           if (past_command_index < (command_index-1)) {
              past_command_index++;
              command_line->setText(command_line_string[past_command_index]);
       } 
	   else {
	      command_line->setText("");
	   }
        }
     }
}
/******************************************************************************************/
/**** FUNCTION: CommandWindow::get_gui_command                                         ****/
/******************************************************************************************/
void CommandWindow::get_gui_command()
{
    strcpy(np->line_buf, command_line->text().latin1());

#if CDEBUG
    printf("COMMAND = \"%s\"\n", np->line_buf);
#endif

    command_line->setText("");
    np->process_command(np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: CommandWindow::hideEvent                                               ****/
/******************************************************************************************/
void CommandWindow::hideEvent(QHideEvent*)
{
    emit win_vis_changed(GConst::visHide);
}
/******************************************************************************************/
/**** FUNCTION: CommandWindow::showEvent                                               ****/
/******************************************************************************************/
void CommandWindow::showEvent(QShowEvent*)
{
    emit win_vis_changed(GConst::visShow);
}
/******************************************************************************************/
/**** FUNCTION: CommandWindow::closeEvent                                              ****/
/******************************************************************************************/
void CommandWindow::closeEvent( QCloseEvent* ce )
{
    ce->ignore();
    if (!np->error_state) {
        hide();
    }

}
/******************************************************************************************/
