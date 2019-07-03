#ifndef INFO_WID_H
#define INFO_WID_H

#include <qwidget.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3ButtonGroup;
class QCheckBox;
class Q3GridLayout;
class Q3HBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpacerItem;
class QSpinBox;
class Q3Table;
class Q3VBoxLayout;

class NetworkClass;
class ExpoPropModelClass;
class SegmentPropModelClass;
class ClutterExpoLinearPropModelClass;

class SegViewTable : public QDialog
{
    Q_OBJECT

public:
    SegViewTable(  NetworkClass*, QWidget* parent = 0, const char* name = 0 );
    ~SegViewTable();

    Q3Table* table;

    QPushButton* ok_btn;

protected:
    Q3VBoxLayout* SegViewTableLayout;
    Q3HBoxLayout* layout1;

private:
    NetworkClass* np;
    
protected slots:
    virtual void languageChange();

    virtual void ok_btn_clicked();

    void set_num_row(int num_row);
};


/******************************************************************************************/
/**** CLASS: GenericInfoWidClass                                                       ****/
/******************************************************************************************/
class GenericInfoWidClass : public QWidget
{
    Q_OBJECT

public:
    GenericInfoWidClass( NetworkClass*, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    virtual ~GenericInfoWidClass();
    virtual void applyParamEdit(int);

    Q3GridLayout*    topLayout;

    Q3ButtonGroup*   prop_type_buttonGroup;
    Q3GridLayout*    prop_type_buttonGroupLayout;
    QLabel*         type_textLabel;
    QLineEdit*      type_lineEdit;
    QLabel*         name_textLabel;
    QLineEdit*      name_lineEdit;

    Q3ButtonGroup*   param_buttonGroup;
    Q3GridLayout*    param_buttonGroupLayout;

    int             prop_idx;

protected:
    NetworkClass*   np;
    
    // QGridLayout*    SegInfoWidLayout;
    // QHBoxLayout*    name_layout;

signals:
    void modified();

protected slots:
    void languageChange();
    virtual void clutter_view_btn_clicked();
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: ExponentialInfoWidClass                                                   ****/
/******************************************************************************************/
class ExponentialInfoWidClass : public GenericInfoWidClass
{
    Q_OBJECT

public:
    ExponentialInfoWidClass( NetworkClass*, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~ExponentialInfoWidClass();
    void applyParamEdit(int pm_idx);

    QLabel*         exponent_textLabel;
    QLineEdit*      exponent_lineEdit;
    QLabel*         coefficient_textLabel;
    QLineEdit*      coefficient_lineEdit;

protected slots:
    void languageChange();

public slots:
    void setParam( ExpoPropModelClass *epm );
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: SegmentInfoWidClass                                                       ****/
/******************************************************************************************/
class SegmentInfoWidClass : public GenericInfoWidClass
{
    Q_OBJECT

public:
    SegmentInfoWidClass( NetworkClass*, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~SegmentInfoWidClass();
    void applyParamEdit(int pm_idx);

    QLabel*         num_clutter_textLabel;
    QSpinBox*       num_clutter_spinBox;
    QLabel*         num_pt_textLabel;
    QSpinBox*       num_pt_spinBox;
    QLabel*         start_slope_textLabel;
    QLineEdit*      start_slope_lineEdit;
    QLabel*         final_slope_textLabel;
    QLineEdit*      final_slope_lineEdit;

    QCheckBox*      useheight_checkBox;
    QLabel*         coeff_logh_textLabel;
    QLineEdit*      coeff_logh_lineEdit;
    QLabel*         coeff_loghd_textLabel;
    QLineEdit*      coeff_loghd_lineEdit;

    Q3ButtonGroup*   view_buttonGroup;
    Q3GridLayout*    view_buttonGroupLayout;
    QLabel*         view_textLabel;
    QPushButton*    view_pushButton;
    QLabel*         clutter_view_textLabel;
    QPushButton*    clutter_view_pushButton;

    SegViewTable*   view_table;
    SegViewTable*   clutter_view_table;

protected slots:
    void languageChange();

    void useheight_cb_clicked();
    void view_btn_clicked();
    void clutter_view_btn_clicked();

public slots:
    void setParam( SegmentPropModelClass *spm );
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: ClutterInfoWidClass                                                       ****/
/******************************************************************************************/
class ClutterInfoWidClass : public GenericInfoWidClass
{
    Q_OBJECT

public:
    ClutterInfoWidClass( NetworkClass*, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~ClutterInfoWidClass();
    void applyParamEdit(int pm_idx);

    QLabel*         clutter_res_textLabel;
    QLineEdit*      clutter_res_lineEdit;

    QLabel*         startx_textLabel;
    QLineEdit*      startx_lineEdit;
    QLabel*         starty_textLabel;
    QLineEdit*      starty_lineEdit;

    QLabel*         numpts_x_textLabel;
    QSpinBox*       numpts_x_spinBox;
    QLabel*         numpts_y_textLabel;
    QSpinBox*       numpts_y_spinBox;

    QCheckBox*      useheight_checkBox;
    QLabel*         coeff_logh_textLabel;
    QLineEdit*      coeff_logh_lineEdit;

    QLabel*         num_clutter_type_textLabel;
    QLabel*         num_clutter_type_textValue;

    Q3ButtonGroup*   view_buttonGroup;
    Q3GridLayout*    view_buttonGroupLayout;
    QLabel*         clutter_view_textLabel;
    QPushButton*    clutter_view_pushButton;

    SegViewTable*   clutter_view_table;

protected slots:
    void languageChange();
    void clutter_view_btn_clicked();

public slots:
    void setParam( ClutterExpoLinearPropModelClass *cpm );
};
/******************************************************************************************/

#endif // INFO_WID_H
