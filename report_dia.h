
/******************************************************************************************
**** PROGRAM: report_dia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef REPORT_DIA_H
#define REPORT_DIA_H

#include <qvariant.h>
#include <qdialog.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class Q3ButtonGroup;
class QSpacerItem;
class QRadioButton;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class NumSectorApply;
class SubnetClass;
class NetworkClass;
class CoverageClass;
class FileChooser;

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    ReportDialog(NetworkClass* np_param, int type_param, QWidget* parent = 0);
    ~ReportDialog();

private slots:
    void ok_btn_clicked();
    void cancel_clicked();

    void sta_btngroup1_clicked( int );
    void sta_apply_clicked();

    void btngroup_clicked( int );

    void fileChooser_textChanged( const QString & );
    void check_box_toggled( bool );

private:
    NetworkClass* np;
    int type;
    QString extension;

    QLabel*       combo_box_lbl;
    QLabel*       combo_box2_lbl;
    QComboBox*    combo_box;
    QComboBox*    combo_box2;
    QCheckBox*    check_box;

    Q3ButtonGroup* button_grp;
    FileChooser*  fileChooser;

    QRadioButton* cmdline_radio_btn;
    QRadioButton* file_radio_btn;
    Q3GridLayout*  button_grp_layout;

    Q3HBoxLayout*  okCancelLayout;
    QPushButton*  ok_btn;
    QPushButton*  cancel_btn;

    Q3GridLayout*  layout;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;
    QSpacerItem* spacer6;
    QSpacerItem* spacer7;

    // xxxxx 
    Q3ButtonGroup* sta_btngroup1;
    QRadioButton* sta_entire_sys_radiobtn;
    QRadioButton* sta_sector_group_radiobtn;
    QLabel*       sta_num_sector_lbl;
    QSpinBox*     sta_num_sector_spinbox;
    QPushButton*  sta_apply_btn;

    Q3GridLayout* sta_btngroup1Layout;

    int pre_num_sector;
    int num_sector;
    NumSectorApply* sector_group_dia;

    bool to_file;
};

#endif // REPORT_DIA_H
