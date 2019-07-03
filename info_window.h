#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <qdialog.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QHideEvent>
#include <QCloseEvent>
#include <QLabel>

class Q3TextEdit;
class QLabel;
class QPushButton;

class NetworkClass;

template<class T> class ListClass;
/******************************************************************************************/
/**** CLASS: InfoWindow                                                                ****/
/******************************************************************************************/
class InfoWindow : public QWidget
{
    Q_OBJECT

public:
    InfoWindow(NetworkClass *np, QWidget* parent=0, const char* name=0);
    ~InfoWindow();
    void setText(QString s);
    friend class MainWindowClass;

public slots:
    void update_selection_change(ListClass<int> *select_cell_list);

private slots:

protected:
    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);
    void closeEvent( QCloseEvent* );

signals:
    void win_vis_changed(int a);
    void pop_signal();
    void hide_signal();

private:
    QWidget        *header;
    Q3TextEdit      *command_text;

    NetworkClass *np;
    QPushButton *pop_btn;
    QPushButton *cancel_btn;
};
/******************************************************************************************/

#endif
