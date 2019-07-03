/******************************************************************************************/
/**** PROGRAM: find_dia.h                                                              ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef FIND_DIA_H
#define FIND_DIA_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class QPushButton;
class QCheckBox;
class QRadioButton;
class Q3ButtonGroup;
class QWidget;
class NetworkClass;
class FigureEditor;
class QLabel;
class QLineEdit;
class QSpacerItem;

class FigureEditor;
class NetworkClass;


template<class T> class ListClass;

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(NetworkClass* np_param, FigureEditor *p_editor, QWidget* parent = 0);
    ~FindDialog();

private slots:
    void ok_btn_clicked();
    void cancel_clicked();
    void CheckToJump( int i );
    void init();
    void changeDistValue1( const QString & );
signals:
    void selection_changed(ListClass<int> *select_cell_list);

private:
    NetworkClass* np;
    FigureEditor* editor;
    
    unsigned char* Input_csid;
    int Input_gw_csc_cs;
        
    QWidget* chooseWizardPage;
    Q3ButtonGroup* buttonGroup1;
    QRadioButton* unassigned_pm_ckbox;
    QRadioButton* CSID_find;
    QRadioButton* gw_csc_cs_find;    
   // QLabel* textLabel1;
    QLineEdit* CSID_input;
    QLineEdit* gw_csc_cs_input;    
    
    Q3VBoxLayout* layout6;
    Q3VBoxLayout* layout7;
    QSpacerItem* spacer1;
    
    Q3HBoxLayout*  okCancelLayout;
    QPushButton*  ok_btn;
    QPushButton*  cancel_btn;
};

#endif // FIND_DIA_H
