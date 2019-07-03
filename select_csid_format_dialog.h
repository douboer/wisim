
/******************************************************************************************
**** PROGRAM: select_csid_format_dialog.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef SELECT_CSID_FORMAT_DIALOG_H
#define SELECT_CSID_FORMAT_DIALOG_H

#include <qdialog.h>

class NetworkClass;
class Q3ButtonGroup;
class QRadioButton;
class QPushButton;

class SelectCSIDFormatDialog : public QDialog
{
    Q_OBJECT

public:
    SelectCSIDFormatDialog(int *p_sel_var_ptr, QWidget* parent = 0);
    ~SelectCSIDFormatDialog();

protected slots:
    void ok_btn_clicked();

private:
    int *sel_var_ptr;
    Q3ButtonGroup *button_grp;
    QRadioButton *ut_radio_btn;
    QRadioButton *zhongxing_radio_btn;
    QPushButton  *ok_btn;
};

#endif // SELECT_CSID_FORMAT_DIALOG_H
