
/******************************************************************************************
**** PROGRAM: cvg_analysis_page.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef CVG_ANALYSIS_PAGE_H 
#define CVG_ANALYSIS_PAGE_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3Frame>

class Q3Frame;
class Q3GridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class Q3ScrollView;

class NetworkClass;
class LevelThresholdPage;

/******************************************************************************************/
/**** CLASS: CvgAnalysisPage                                                           ****/
/******************************************************************************************/
class CvgAnalysisPage : public QWidget
{
    Q_OBJECT
public:
    CvgAnalysisPage(NetworkClass *np_param, QWidget* parent, char *name);
    ~CvgAnalysisPage();
    bool advShown;

public slots:
    void adv_btn_clicked();

protected slots:
    virtual void run_btn_clicked();

protected:
    NetworkClass *np;
    char         *page_name;

    QPushButton *run_btn;

    QLabel *init_sample_res_lbl;
    QLabel *scan_fractional_area_lbl;

    QLineEdit *scan_fractional_area_val;
    QLineEdit *init_sample_res_val;

    Q3GridLayout *cvg_analysis_page_gridlayout;
};
/******************************************************************************************/

/******************************************************************************************/
/****                            CLASS: LayerPage                                      ****/
/******************************************************************************************/
class LayerPage : public CvgAnalysisPage
{
    Q_OBJECT
public:
    LayerPage(NetworkClass *np, int cvg_idx, QWidget* parent, char *name);
    ~LayerPage();

    QLineEdit *layer_thr_val;
    QSpinBox  *max_layer_spinbox;

private slots:
    void run_btn_clicked();

private:
    QLabel *layer_thr_lbl;
    QLabel *max_layer_lbl;

};
/******************************************************************************************/

/******************************************************************************************/
/****                            CLASS: LevelPage                                      ****/
/******************************************************************************************/
class LevelPage : public CvgAnalysisPage
{
    Q_OBJECT
public:
    LevelPage(NetworkClass *np, int cvg_idx, QWidget* parent, char *name);
    ~LevelPage();

    QSpinBox  *num_threshold_spinbox;
    QLineEdit *max_threshold_val;
    QLineEdit *min_threshold_val;

private slots:
    void evenly_spaced_btn_clicked();
    void manual_set_btn_clicked();
    void min_max_disable_set(int value);

    void run_btn_clicked();

    void thr_page_apply_btn_clicked();
    void thr_page_cancel_btn_clicked();

private:
    int  level_cvg_idx;

    QLabel      *num_threshold_lbl;
    QLabel      *max_threshold_lbl;
    QLabel      *min_threshold_lbl;

    QPushButton *evenly_spaced_btn;
    QPushButton *manually_set_btn;

    LevelThresholdPage *level_thr_page;
};
/******************************************************************************************/


/******************************************************************************************/
/***                                CLASS: LevelThresholdPage                           ***/
/******************************************************************************************/
class LevelThresholdPage : public QDialog
{
    Q_OBJECT

public:
    LevelThresholdPage( NetworkClass *param_np, int cvg_idx, int num_level, double min_thr, 
                        double max_thr, double space, bool man_set, QWidget* parent = 0,  char *name = 0);
    ~LevelThresholdPage();

    QLabel    **level_thr_lbl;
    QLineEdit **level_thr_val;

    QPushButton *apply_btn;
    QPushButton *cancel_btn;

private:
    NetworkClass   *np;
    Q3ScrollView    *thr_page_scrollview;
    Q3Frame         *thr_page_frame;
    Q3GridLayout    *thr_page_frameLayout;
    Q3GridLayout    *thr_page_Layout;
};
/******************************************************************************************/

/******************************************************************************************/
/****                            CLASS: SirLayerPage                                   ****/
/******************************************************************************************/
class SirLayerPage : public CvgAnalysisPage
{
    Q_OBJECT
public:
    SirLayerPage(NetworkClass *np, int cvg_idx, QWidget* parent, char *name);
    ~SirLayerPage();

    QLineEdit *sir_layer_thr_val;
    QSpinBox  *max_sir_layer_spinbox;

private slots:
    void run_btn_clicked();

private:
    QLabel *sir_layer_thr_lbl;
    QLabel *max_sir_layer_lbl;
};
/******************************************************************************************/
#endif
