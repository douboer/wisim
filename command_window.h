#ifndef COMMAND_WINDOW_H
#define COMMAND_WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qstring.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QCloseEvent>
#include <QLabel>

class Q3TextEdit;
class QLabel;
class QPushButton;
class NetworkClass;
/******************************************************************************************/
/**** CLASS: CommandWindow                                                             ****/
/******************************************************************************************/
class CommandWindow : public QWidget
{
    Q_OBJECT

public:
    CommandWindow(NetworkClass *np, QWidget* parent=0, const char* name=0);
    ~CommandWindow();
    friend class MainWindowClass;
    friend void print_gui_message(FILE *fp, char *line);

public slots:
    void save_gui_command(char *);
    void setEnabledButtons(bool enable);

private slots:
    void get_gui_command();
    void clear_error_state();

protected:
    void keyPressEvent(QKeyEvent *);
    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);
    void closeEvent( QCloseEvent* );

signals:
    void win_vis_changed(int a);
    void pop_signal();

private:
    QWidget        *header;
    Q3TextEdit      *command_text;
    QLabel         *cle_prompt;
    QPushButton    *clear_button;
    QLineEdit      *command_line;
    QString        command_line_string[10];
    int            command_number;
    int            command_index;
    int            past_command_index;
    NetworkClass *np;
    QPushButton *pop_btn;
    QPushButton *cancel_btn;
    FILE           *fl;

    void           get_last_command( bool );
};
/******************************************************************************************/

#endif
