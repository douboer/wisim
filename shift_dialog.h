
/******************************************************************************************
**** PROGRAM: shift_dialog.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef SHIFT_DIALOG_H
#define SHIFT_DIALOG_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>
#include "wisim.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class QLabel;
class QComboBox;
class Q3GridLayout;
class Q3ButtonGroup;
class QSpacerItem;
class QPushButton;
class NetworkClass;

class ShiftDialog : public QDialog
{
    Q_OBJECT

public:
    ShiftDialog( NetworkClass *np_param, int rtti_val, QWidget* parent = 0 );
    ~ShiftDialog();

    Q3ButtonGroup* adjust_buttonGroup;

    QLabel*      mapLayerLabel; 
    QComboBox*   mapLayerComboBox;

    QPushButton* left_pushButton;
    QPushButton* right_pushButton;
    QPushButton* down_pushButton;
    QPushButton* up_pushButton;

    QPushButton* ok_button;
    QPushButton* cancel_button;

protected:
    Q3GridLayout* ShiftDialogLayout;
    Q3HBoxLayout* layout1;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;

    Q3GridLayout* adjust_buttonGroupLayout;
    QSpacerItem* spacer5;
    QSpacerItem* spacer6;
    QSpacerItem* spacer7;
    QSpacerItem* spacer8;
    QSpacerItem* spacer9;
    QSpacerItem* spacer10;
    QSpacerItem* spacer11;

public slots:
    void left_btn_clicked();
    void right_btn_clicked();
    void up_btn_clicked();
    void down_btn_clicked();

    void ok_btn_clicked();
    void cancel_btn_clicked();

    void mapLayerComboBox_select( int );

protected slots:
    virtual void languageChange();

private:
    int map_layer_idx;
    int rtti_value;
    class NetworkClass *np;
};

#endif // SHIFT_DIALOG_H
