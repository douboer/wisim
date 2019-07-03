/****************************************************************************
**
****************************************************************************/

#ifndef CSDCAPARAM_H
#define CSDCAPARAM_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class Q3ButtonGroup;
class QRadioButton;
class NetworkClass;

class csDcaParam : public QWidget
{
    Q_OBJECT

public:
    csDcaParam( NetworkClass *np_param, QWidget* parent = 0, const char* name = 0 );
    ~csDcaParam();

    QLabel*     sir_threshold_call_request_cs_db_lbl;
    QLineEdit*  sir_threshold_call_request_cs_db_val;
    QLabel*     sir_threshold_call_drop_cs_db_lbl;
    QLineEdit*  sir_threshold_call_drop_cs_db_val;

    QLabel*     int_threshold_call_request_cs_db_lbl;
    QLineEdit*  int_threshold_call_request_cs_db_val;
    QLabel*     int_threshold_call_drop_cs_db_lbl;
    QLineEdit*  int_threshold_call_drop_cs_db_val;

    Q3ButtonGroup* cs_dca_algorithm;
    QRadioButton* cs_sir;
    QRadioButton* cs_int;
    QRadioButton* cs_melco;

protected:
    Q3GridLayout* csDcaParamLayout;
    Q3GridLayout* cs_dca_algorithmLayout;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;

private:
    void init_cs_dca_alg(int);

    NetworkClass *np;

protected slots:
    virtual void languageChange();

    void set_cs_dca_alg( int );

public slots:
    virtual void ok_btn_clicked();
//    virtual void cancel_btn_clicked();

};

#endif // CSDCAPARAM_H
