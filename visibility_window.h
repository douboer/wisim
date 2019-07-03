#ifndef VISIBILITY_WINDOW_H
#define VISIBILITY_WINDOW_H

#include <stdio.h>
#include <stdlib.h>

#include <q3listview.h>
#include <q3mainwindow.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QResizeEvent>
#include <Q3Frame>
#include <QMouseEvent>
#include <QHideEvent>

#include "WiSim.h"
#include "WiSim_gui.h"

class QPushButton;
class VisibilityList;
class VisibilityItem;
class VisibilityCheckItem;
/******************************************************************************************/
/**** CLASS: VisibilityList                                                            ****/
/******************************************************************************************/
class VisibilityList : public Q3ListView
{
    Q_OBJECT

public:
    VisibilityList( QWidget *parent = 0, const char *name = 0 );
    static void setNetworkStruct( NetworkClass *m_np) { np = m_np; };
    static void setVisibilityWindow( VisibilityWindow *vw) { visibility_window = vw; };
    static Q3ListViewItem *findItem( VisibilityList *parent, int rtti_val, int index);
    static Q3ListViewItem *findItem( VisibilityItem *parent, int rtti_val, int index);
    static Q3ListViewItem *findItem( VisibilityItem *parent, int rtti_val, int cell_idx, int sector_idx);
    void update_rtd(int create);
    void update_cpm();
    void update_cvg_analysis(int cvg_idx);

protected:
    virtual void contentsMouseDoubleClickEvent(QMouseEvent*);
    virtual void contentsMousePressEvent(QMouseEvent*);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);

private:
    static NetworkClass *np;
    static VisibilityWindow *visibility_window;
};
/******************************************************************************************/
/**** CLASS: VisibilityWindow                                                          ****/
/******************************************************************************************/
class VisibilityWindow : public Q3Frame
{
    Q_OBJECT

public:
    VisibilityWindow( QWidget *parent = 0, const char *name = 0 );
    ~VisibilityWindow();
    static void setNetworkStruct( NetworkClass *m_np) { np = m_np; };
    friend class MainWindowClass;
    friend class VisibilityList;
    friend class VisibilityItem;
    friend class VisibilityCheckItem;
    friend class MeshClass;
    friend int NetworkClass::process_command(char *line);
    friend void NetworkClass::delete_cell(int *ptr_s, int n);
    friend void NetworkClass::delete_cell(ListClass<int> *int_list);
    friend void NetworkClass::delete_prop_model(ListClass<int> *int_list);
    friend void FigureEditor::regenCanvas(int cx, int cy);
    friend void FigureEditor::setVisibility(int rtti_val);
    friend void FigureEditor::toggleRoadTestData();
    friend void FigureEditor::toggleRoadTestDataGroup();
    friend void FigureEditor::testSlotD();
    friend void FigureEditor::trackRoadTestData();
    friend void FigureEditor::trackSignalLevel();
    friend QRect FigureEditor::tip( const QPoint& p, QString& s );
    void resize();

protected:
    virtual void resizeEvent(QResizeEvent*);
#if 0
    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);
#endif
    virtual void mouseReleaseEvent ( QMouseEvent * event ) ;

private:
    QWidget *header;
    void visibility_changed(int rtti_val);
    void delete_cell(int cell_idx);
    void change_cell_idx(int old_idx, int new_idx);

    VisibilityList *visibility_list;
    static NetworkClass *np;

    QPushButton *pop_btn;
    QPushButton *cancel_btn;

    //////////////////////////////////////////////////////////////////////////
    // Visibility State                                                     //
    //////////////////////////////////////////////////////////////////////////
    char vis_system_boundary;
    char vis_antenna;
    int  vis_cell;
    char vis_cell_text;
    char vis_map_height;
    int  vis_map_clutter;
    int  vis_clutter_prop_model;
    char vis_map_background;
    char vis_map_layer;
    char vis_coverage;
    int  vis_road_test_data;
    char vis_subnet;
    int  vis_rtd_level;

    char *vec_vis_cell;
    char **vec_vis_subnet;
    char *vec_vis_map_clutter;
    char *vec_vis_clutter_prop_model;
    char *vec_vis_map_layer;
    char **vec_vis_coverage;

    char *vec_vis_road_test_data;

    char *vec_vis_rtd_level;
    int num_road_test_data_set;
    int num_rtd_level;
    int *vec_rtd_cell_idx;
    int *vec_rtd_sector_idx;

    char *vec_vis_cell_text;

#if HAS_MONTE_CARLO
    char vis_traffic;
    char *vec_vis_traffic;
#endif
    //////////////////////////////////////////////////////////////////////////

    int select_rtti;
    int select_index;
    int select_p_idx;

private slots:
    void setCellNamePref(int cell_view_pref);
    void setRTDPref(int rtd_view_pref);
    void setStrid();

signals:
    void visibility_state_change(int rtti_val);
    void win_vis_changed(int a);
    void pop_signal();
    void hide_signal();
};
/******************************************************************************************/
/**** CLASS: VisibilityItem                                                            ****/
/******************************************************************************************/
class VisibilityItem : public Q3ListViewItem
{
public:
    VisibilityItem( Q3ListView     *parent, const int m_rtti_val, const int m_index, const QString & text, int color = -1);
    VisibilityItem( Q3ListViewItem *parent, const int m_rtti_val, const int m_index, const QString & text, int color = -1);
    int rtti() const;
    virtual void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );
    void setIndex(int m_index) { index = m_index; };
    void setItemColor(int color);
    void toggleChildren();
    static void setNetworkStruct( NetworkClass *m_np) { np = m_np; };
    static void setVisibilityWindow( VisibilityWindow *vw) { visibility_window = vw; };
    friend class VisibilityList;
    friend class VisibilityCheckItem;
    friend class VisibilityWindow;

private:
    void allocateItemVisibility();
    int rtti_val;
    int index;
    int color;
    static VisibilityWindow *visibility_window;
    static NetworkClass *np;
};
/******************************************************************************************/
/**** CLASS: VisibilityCheckItem                                                       ****/
/******************************************************************************************/
class VisibilityCheckItem : public Q3CheckListItem
{
public:
    VisibilityCheckItem( Q3ListView     *parent, const int m_rtti_val, const int m_index,
                         const QString & text, int color = -1, int cell_idx = -1, int sector_idx = -1);
    VisibilityCheckItem( Q3ListViewItem *parent, const int m_rtti_val, const int m_index,
                         const QString & text, int color = -1, int cell_idx = -1, int sector_idx = -1);
    int rtti() const;
    virtual void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );
    int getIndex() {return index; };
    void setIndex(int m_index) {index = m_index; };
    void setItemColor(int color);
    void setItemVisibility(int init = 0, int cell_idx = -1, int sector_idx = -1);
    void setItemID(int cell_idx, int sector_idx);
    static void setNetworkStruct( NetworkClass *m_np) { np = m_np; };
    static void setVisibilityWindow( VisibilityWindow *vw) { visibility_window = vw; };
    int compare( Q3ListViewItem *i, int col, bool ascending ) const;
    friend class FigureEditor;
    friend class VisibilityList;

protected:
    void stateChange ( bool );

private:
    int rtti_val;
    int index;
    int color;
    static VisibilityWindow *visibility_window;
    static NetworkClass *np;
};
/******************************************************************************************/

#endif
