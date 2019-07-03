
/******************************************************************************************
**** PROGRAM: cvg_maindia.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef CVG_ANA_DIA_H
#define CVG_ANA_DIA_H

#include <qpixmap.h>
#include <qdialog.h>
#include <q3valuevector.h> 
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3PtrList>

class NetworkClass;
class FigureEditor;

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;
class Q3ButtonGroup;
class QToolButton;
class Q3ListView;
class Q3ListViewItem;
class QLabel;
class Q3WidgetStack;

class CvgInterface;
class CvgAnalysis;
class CvgLevelWizard;
class CvgLevelPage;
class CvgLayerPage;
class CvgPAPage;
class CvgTypeNameDia;
class Cvg_Part_Wizard;

template<class T> class ListClass;

class CvgAnaDia : public QDialog
{
    Q_OBJECT

public:
    CvgAnaDia( FigureEditor* editor_param, QWidget* parent = 0, const char* name = 0 );
    ~CvgAnaDia();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    Q3ButtonGroup* buttonGroup1;

    QToolButton* create_toolButton;
    QToolButton* delete_toolButton;
    QLabel* head_lbl;

    Q3ListView* listView;
    Q3ListViewItem * top_level_item;

signals:

protected slots:
    virtual void languageChange();

    void create_toolButton_clicked();
    void delete_toolButton_clicked();

private slots:
    void listView_selectchanged( Q3ListViewItem* );
    void cvg_type_name_dia_ok_btn_clicked();
    void ok_btn_clicked();
    void cancel_btn_clicked();

    virtual void accept();             
    
    // added by Wei Ben on Nov. 10, 2004
    void pre_accept();

public slots:

private:
    FigureEditor *editor;
    NetworkClass *np;

    Q3GridLayout* CvgAnaDiaLayout;
    Q3HBoxLayout* ok_cancel_Layout;
    QSpacerItem* Horizontal_Spacing2;
    Q3VBoxLayout* toolbtn_listview_layout;
    Q3HBoxLayout* buttonGroup1Layout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    Q3VBoxLayout* cvg_info_layout;

    //when mouse on top level listview raise about_wid
    QWidget* about_wid;
    QLabel* about_lbl;
    Q3HBoxLayout* about_widLayout;

    QPixmap image1;
    QPixmap image2;
    QPixmap image3;

    CvgAnalysis*    cvg_layer_wizard;
    CvgLevelWizard* cvg_level_wizard;
    CvgTypeNameDia* cvg_type_name_dia;

    Cvg_Part_Wizard* cvg_part_wizard;

    CvgLayerPage* cvg_layer_page;
    CvgLevelPage* cvg_level_page;
    CvgPAPage* cvg_pa_page;

    Q3WidgetStack* cvg_widstack;

    Q3PtrList <CvgLayerPage> info_layer_wid_vector;
    Q3PtrList <CvgLevelPage> info_level_wid_vector;
    Q3PtrList <CvgPAPage> info_pa_wid_vector;

    //flag of new create coverage analysis type
    bool is_level_flag;

    // get coverage parameters
    QString m_cvg_type;
    int     m_cvg_type_idx;
    QString m_cvg_name;
    //--------------------
    //layer and sir-layer param
    double  m_sig_thr;
    int     m_max_layer;
    //level param
    int     m_num_thr;
    double  m_min_thr;
    double  m_max_thr;
    //--------------------
    double  m_scan_fra_area;
    int     m_sam_res;

    Q3ValueVector <double> m_thr_vector;

    int  m_item_count;
    //index of new create coverage analysis   |  = m_item_count - 1
    int  m_item_index;
    //index of selected coverage analysis
    int  m_select_index;
    //flag of select coverage analysis type
    enum selectType { isLayer, isLevel, isPA, undef };
    selectType type;

    // added by Wei Ben for cvg_part_wizard on Nov. 10, 2004
    bool m_bChoosePart;
    bool m_use_gpm;
    Q3ValueVector <int> m_vPartList;
    double m_dPartDistance;
    QStringList m_vDisplayList;

    void createCmd( char *);

    // added to easy maintain
    void doLevelCvg();
    void doLayerCvg();
    void doSirCvg();
    void doPACvg();

    bool checkParam();

    void initForm();

};

#endif // COV_ANA_DIA_H
