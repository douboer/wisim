#ifndef PSDCAPARAM_H
#define PSDCAPARAM_H

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
class QComboBox;
class QLabel;
class QLineEdit;
class Q3ButtonGroup;
class QRadioButton;
class NetworkClass;

class psDcaParam : public QWidget
{
    Q_OBJECT

public:
    psDcaParam( NetworkClass* np, QWidget* parent = 0, const char* name = 0 );
    ~psDcaParam();

    QLabel*     sir_threshold_call_request_ps_db_lbl;
    QLineEdit*  sir_threshold_call_request_ps_db_val;
    QLabel*     sir_threshold_call_drop_ps_db_lbl;
    QLineEdit*  sir_threshold_call_drop_ps_db_val;

    QLabel*     int_threshold_call_request_ps_db_lbl;
    QLineEdit*  int_threshold_call_request_ps_db_val;
    QLabel*     int_threshold_call_drop_ps_db_lbl;
    QLineEdit*  int_threshold_call_drop_ps_db_val;

    Q3ButtonGroup* ps_dca_algorithm;
    QRadioButton* ps_sir;
    QRadioButton* ps_int;
    QRadioButton* ps_int_sir;

    Q3ButtonGroup* ps_mem_buttonGp;
    Q3GridLayout*  ps_mem_buttonGpLayout;
    QLabel*       memoryless_ps_lbl;
    QComboBox*    memoryless_ps_combobox;

protected:
    Q3GridLayout* psDcaParamLayout;
    Q3GridLayout* ps_dca_algorithmLayout;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;

private:
    void init_ps_dca_alg(int);

    NetworkClass* np;
    
public slots:
    virtual void ok_btn_clicked();
    void ps_type_select( int i );
    
protected slots:
    virtual void languageChange();
    void set_ps_dca_alg( int );

};

#endif // PSDCAPARAM_H
