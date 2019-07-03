/******************************************************************************************/
/**** FILE: print_gui_message.cpp                                                      ****/
/******************************************************************************************/

// #include <qgb18030codec.h>
#include <q3textedit.h>
#include "WiSim.h"
#include "main_window.h"
#include "command_window.h"

extern MainWindowClass *main_window;

extern int use_gui;

/******************************************************************************************/
void print_gui_message(FILE *fp, char *line)
{
    int n = strlen(line);
    QString s;
    // static QGb18030Codec gbc;

    if ((use_gui) && (fp == stdout)) {
        if ( (n==1)&&(line[0]=='\n') ) {
            main_window->command_window->command_text->append(" ");
            if (main_window->command_window->fl) {
                fprintf(main_window->command_window->fl, "\n");
            }
        } else if (n > 1) {
//            if (line[n-1] == '\n') {
//                line[n-1] = (char) NULL;
//                n--;
//            }
            // s = gbc.toUnicode(line, n);
            s = QString::fromLocal8Bit(line);
            main_window->command_window->command_text->append(s);
            if (main_window->command_window->fl) {
//                fprintf(main_window->command_window->fl, "%s\n", line);
                fprintf(main_window->command_window->fl, "%s", line);
                if (line[n-1] != '\n') {
                    fprintf(main_window->command_window->fl, "\n");
                }
                fflush(main_window->command_window->fl);
            }
        }
        main_window->command_window->command_text->scrollToBottom();
    } else {
        fprintf(fp, "%s", line);
    }


    return;
}
/******************************************************************************************/
