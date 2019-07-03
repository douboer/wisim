/******************************************************************************************/
/**** PROGRAM: prop_mod_mgr_dia.h                                                      ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef PROP_MOD_MGR_H
#define PROP_MOD_MGR_H

#include <qdialog.h>
#include <q3listview.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3ButtonGroup;
class Q3GridLayout;
class Q3HBoxLayout;
class QLabel;
class QPushButton;
class QSpacerItem;
class QToolButton;
class Q3VBoxLayout;
class Q3WidgetStack;

class ExponentialInfoWidClass;
class SegmentInfoWidClass;
class ClutterInfoWidClass;
class ExpoPropWizard;
class NetworkClass;
class PropExpoPage;
class PropInterface;
class PropSegPage;
class PropTypeNameDia;
class PropWizard;
class SectorPropTable;
class SegPropWizard;

/******************************************************************************************/
/**** CLASS: PropModelListItem                                                         ****/
/******************************************************************************************/
class PropModelListItem : public Q3ListViewItem
{
public:
    PropModelListItem( Q3ListView *parent, int m_pm_idx);
    ~PropModelListItem();
    int getIndex();
    void setIndex(int m_pm_idx);
    int compare( Q3ListViewItem *i, int col, bool ascending ) const;

private:
    int pm_idx;
};
/******************************************************************************************/

class ToolButtonWithText : public QWidget
{
    Q_OBJECT
public:
    ToolButtonWithText( QWidget *parent, const char *name );
    ~ToolButtonWithText();

    void setIconSet( const QIcon & );
    void setText( const QString & );

    QToolButton* toolButton;

private:
    Q3VBoxLayout* vlayout;
    // QHBoxLayout* hlayout;
    QLabel*      textLabel;

};


class PropModMgrDia : public QDialog
{
    Q_OBJECT

public:
    PropModMgrDia( NetworkClass *np_param, QWidget* parent = 0 );
    ~PropModMgrDia();

    void generate_list(int selected_idx);

    QPushButton* buttonClose;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;

    Q3ButtonGroup* buttonGroup1;

    ToolButtonWithText* create_toolButton;
    ToolButtonWithText* delete_toolButton;
    ToolButtonWithText* assign_toolButton;

    QLabel* head_lbl;

    Q3ListView*      listView;

private slots:
    void languageChange();
    void updateStateChange();

    void create_toolButton_clicked();
    void delete_selected_model();
    void assign_toolButton_clicked();

    void listView_selectchanged( Q3ListViewItem* );
    void prop_type_name_dia_ok_btn_clicked();

    void close_btn_clicked();
    void cancel_btn_clicked();
    void apply_btn_clicked();

    void entryEdited();

private:
    Q3GridLayout* PropAnaDiaLayout;
    Q3HBoxLayout* ok_cancel_Layout;
    QSpacerItem* Horizontal_Spacing2;
    Q3VBoxLayout* toolbtn_listview_layout;
    Q3HBoxLayout* buttonGroup1Layout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    Q3VBoxLayout* prop_info_layout;

    QWidget*     about_wid;
    QLabel*      about_lbl;
    Q3HBoxLayout* about_widLayout;

    ExpoPropWizard*    expo_prop_wizard;
    PropWizard*        prop_wizard;
    SectorPropTable*   sector_prop_table;

    PropTypeNameDia*   prop_type_name_dia;
    PropInterface*     prop_page;
    Q3WidgetStack*      prop_widstack;

    ExponentialInfoWidClass* expo_info_wid;
    SegmentInfoWidClass*     seg_info_wid;
    ClutterInfoWidClass*     clutter_info_wid;

    // common parameters
    int            m_prop_type;
    QString        m_prop_name;
    int            pm_idx;
    int            state;

    NetworkClass*  np;
};

#endif
