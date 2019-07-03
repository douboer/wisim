
/******************************************************************************************
**** PROGRAM: cvg_info_wid.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef CVG_INFO_WID_H
#define CVG_INFO_WID_H

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
class Q3ButtonGroup;
class QLabel;
class QLineEdit;
class QPushButton;
class NetworkClass;
class QCheckBox;

class disp_cells;

class CvgInfoLayerWid : public QWidget
{

    Q_OBJECT
public:
    CvgInfoLayerWid( NetworkClass*,  QWidget* parent = 0, const char* name = 0 );
    ~CvgInfoLayerWid();

    Q3ButtonGroup* buttonGroup1;
    QLabel* info_cvg_type_textLabel;
    QLabel* info_cvg_name_textLabel;
    QLineEdit* info_cvg_type_lineEdit;
    QLineEdit* info_cvg_name_lineEdit;
    
    Q3ButtonGroup* buttonGroup2;
    QLabel* info_cvg_sig_thr_textLabel;
    QLabel* info_max_layer_textLabel;
    QLineEdit* info_cvg_sig_thr_lineEdit;
    QLineEdit* info_max_layer_lineEdit;

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

    disp_cells *m_oDispDialog;

    void setParam
        ( const QString&, const QString&, double, int, double, int );

    // added by Wei Ben
    void setPartParam( const double, const QStringList&, bool); 

protected:
    Q3GridLayout* CvgInfoLayerWidLayout;

    Q3GridLayout* buttonGroup1Layout;
    Q3GridLayout* buttonGroup2Layout;
    Q3GridLayout* buttonGroup3Layout;
    QSpacerItem* info_spacer;

    // added by Wei Ben
    Q3GridLayout* layout14;
    
    virtual void languageChange();

protected slots:  
  void part_btn_clicked();
 

private:
    NetworkClass* np;

};

/*
 * A new class for PA infomation
 *
 */

class CvgInfoPAWid : public QWidget
{

    Q_OBJECT
public:
    CvgInfoPAWid( NetworkClass*,  bool bYesThrd, QWidget* parent = 0, const char* name = 0 );
    ~CvgInfoPAWid();

    Q3ButtonGroup* buttonGroup1;
    QLabel* info_cvg_type_textLabel;
    QLabel* info_cvg_name_textLabel;
    QLineEdit* info_cvg_type_lineEdit;
    QLineEdit* info_cvg_name_lineEdit;
    
    Q3ButtonGroup* buttonGroup2;
    QLabel* info_cvg_sig_thr_textLabel;
    QLineEdit* info_cvg_sig_thr_lineEdit;

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

    disp_cells *m_oDispDialog;

    void setParam
        ( const QString&, const QString&, double, double, int );
    void setParam
        ( const QString&, const QString&, double, int );

    // added by Wei Ben
    void setPartParam( const double, const QStringList&, bool); 

protected:
    Q3GridLayout* CvgInfoPAWidLayout;

    Q3GridLayout* buttonGroup1Layout;
    Q3GridLayout* buttonGroup2Layout;
    Q3GridLayout* buttonGroup3Layout;
    QSpacerItem* info_spacer;

    // added by Wei Ben
    Q3GridLayout* layout14;
    
    virtual void languageChange();

protected slots:  
  void part_btn_clicked();
 

private:
    NetworkClass* np;

};   



#endif // CVG_INFO_WID_H
