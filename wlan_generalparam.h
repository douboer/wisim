#ifndef WLAN_GENERALPARAM_H
#define WLAN_GENERALPARAM_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>

class QLabel;
class NetworkClass;

class WLANGeneralParam : public QWidget
{
    Q_OBJECT

public:
    WLANGeneralParam( NetworkClass* np, QWidget* parent = 0, const char* name = 0 );
    ~WLANGeneralParam();

    QLabel* info_lbl;

private:
    NetworkClass* np;

public slots:
    virtual void ok_btn_clicked();

};
#endif // WLAN_GENERALPARAM_H
