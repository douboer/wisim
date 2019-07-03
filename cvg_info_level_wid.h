
/******************************************************************************************
**** PROGRAM: cvg_info_level_wid.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef CVG_INFO_LEVEL_WID_H
#define CVG_INFO_LEVEL_WID_H

#include <qvariant.h>
#include <qwidget.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3ButtonGroup;
class QLabel;
class QLineEdit;
class QPushButton;
class Q3Table;
class NetworkClass;
class QCheckBox;
class disp_cells;


class CvgThrTable : public QDialog
{
    Q_OBJECT

public:
    CvgThrTable(  NetworkClass*, QWidget* parent = 0, const char* name = 0 );
    ~CvgThrTable();

    Q3Table* table;

    QPushButton* cancel_btn;

protected:
    Q3VBoxLayout* CvgThrTableLayout;
    Q3HBoxLayout* layout1;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;

private:
    NetworkClass* np;
    
protected slots:
    virtual void languageChange();

    virtual void cancel_btn_clicked();

};

class CvgInfoLevelWid : public QWidget
{
    Q_OBJECT

public:
    CvgInfoLevelWid( NetworkClass*, QWidget* parent = 0, const char* name = 0 );
    ~CvgInfoLevelWid();

    CvgThrTable* thr_tab_dia; 
    
    Q3ButtonGroup* buttonGroup1;
    QLabel* info_cvg_type_textLabel;
    QLabel* info_cvg_name_textLabel;
    QLineEdit* info_cvg_type_lineEdit;
    QLineEdit* info_cvg_name_lineEdit;

    Q3ButtonGroup* buttonGroup2;
    QLineEdit* info_min_thr_lineEdit;
    QLabel* info_max_thr_textLabel;
    QLineEdit* info_max_thr_lineEdit;
    QLabel* info_min_thr_textLabel;
    QLabel* view_thr_textLabel;
    QPushButton* view_pushButton;
    QLabel* num_thr_textLabel;
    QLineEdit* num_thr_lineEdit;

    Q3ButtonGroup* buttonGroup3;
    QLabel* info_scan_fraction_area_textLabel;
    QLabel* info_init_sample_resolution_textLabel;
    QLineEdit* info_scan_fraction_area_lineEdit;
    QLineEdit* info_init_sample_resolution_lineEdit;

    // added by Wei Ben on Nov. 11, 2004
    QPushButton* viewPartList;
    QCheckBox* chooseAll;
    QCheckBox* choosePart;    
    QLabel* maxDistLabel;
    QLineEdit* maxDistlineEdit;

protected:
    Q3VBoxLayout* CvgInfoLevelWidLayout;

    Q3GridLayout* buttonGroup1Layout;
    Q3GridLayout* buttonGroup2Layout;
    Q3GridLayout* buttonGroup3Layout;
//    QHBoxLayout* buttonGroup4Layout;

    Q3HBoxLayout* layout2;
    QSpacerItem* info_spacer1;
    QSpacerItem* info_spacer2;

    // added by Wei Ben
    Q3GridLayout* layout14;
    disp_cells *m_oDispDialog;

public slots:
    void setParam( const QString&, const QString&, 
                            int, double, double, double, int );

// addede by Wei Ben
 void setPartParam( const double, const QStringList&, bool); 
 void part_btn_clicked();

protected slots:
    virtual void languageChange();
    void view_btn_clicked();
    
//    void run_btn_clicked();
    
private:
    NetworkClass* np;
    

};

#endif // CVG_INFO_LEVEL_WID_H
