/****************************************************************************
** prop_mod_widget.h
****************************************************************************/

#ifndef PROP_MOD_WIDGET_H
#define PROP_MOD_WIDGET_H

#include <qpushbutton.h>
#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include "wisim.h"
#include "prop_model.h"

class Q3GridLayout;
class QRadioButton;
class Q3ButtonGroup;
class QPushButton;
class QLabel;
class QLineEdit;

class PropModWidget: public QWidget
{
    Q_OBJECT

public:
    PropModWidget(NetworkClass* param_np, int prop_model_idx = -1, QWidget* parent = 0, const char* name = 0);
    ~PropModWidget();

    Q3ButtonGroup* type_buttongroup;
    QRadioButton* expo_type_radiobutton;
    QRadioButton* segment_type_radiobutton;

    Q3ButtonGroup* param_buttongroup;
    QLabel* expo_lbl;
    QLabel* expo_coefficient_lbl;
    QLineEdit* expo_val;
    QLineEdit* expo_coefficient_val;

    QPushButton *apply_btn;
    QLabel*      inflexion_number_lbl;
    QLabel*      start_slope_lbl;
    QLabel*      final_slope_lbl;
    QLabel**     inflexion_x_lbl;
    QLabel**     inflexion_y_lbl;
    QLineEdit*   inflexion_number_val;
    QLineEdit*   start_slope_val;
    QLineEdit*   final_slope_val;
    QLineEdit**  inflexion_x_val;
    QLineEdit**  inflexion_y_val;
    
signals:
    void valueChanged( int );

protected:
    Q3GridLayout* prop_mod_widgetLayout;
    Q3GridLayout* type_buttongroupLayout;
    Q3GridLayout* param_buttongroupLayout;

public slots:
    void apply_btn_clicked();
    void type_radiobutton_clicked( int );
    void languageChange();
    void radioChanged( int );
    void lineeditChanged( const QString& );

private:
    int inflexion_num;
    NetworkClass* np;
    SegmentPropModelClass* seg_pm;
    ExpoPropModelClass* expo_pm;
};

#endif // PROP_MOD_WIDGET_H
