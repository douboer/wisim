#ifndef CATEGORY_WIN_H
#define CATEGORY_WID_H

#include <stdlib.h>
#include <qapplication.h>
#include <qimage.h>
#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>
#include <q3widgetstack.h>
#include <qdialog.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3PtrList>
#include <Q3HBoxLayout>

#include "psdcaparam.h"
#include "csdcaparam.h"
#include "generalparam.h"
#include "wlan_generalparam.h"

#include "category_page_set.h"

#define CGDEBUG 0

class QToolBox;
class QStyle;
class Q3WidgetStack;
class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;
class QTabWidget;
class NetworkClass;

#if 0 
void category_list(/*const QPtrList <CategoryInterface> &categories,*/ CategoryWid* v_category_wid, CategoryPage* v_widget_category )
{
    Q3PtrList <CategoryInterface> categories;
    categories.append( v_widget_category );
    v_category_wid->setCategories( categories );
}
#endif

class CategoryWid : public QDialog
{
    Q_OBJECT

public:
//    CategoryWid( QWidget *parent=0, const char *name=0 );
    CategoryWid( NetworkClass *, QWidget *parent=0, const char *name=0 );
    ~CategoryWid();

    void setCategories( const Q3PtrList <CategoryInterface> &v_categories );


    Q3WidgetStack *widgetStack() const { return stack; }

    void category_list(  CategoryPage* v_widget_category );

     Q3ButtonGroup *g;

private slots:

protected:
    Q3GridLayout* MyDialogLayout;
    Q3HBoxLayout* OkCancelLayout;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;

private:
    QWidget *createCategoryPage( CategoryInterface *c );

    QPushButton* buttonOk;
    QPushButton* buttonCancel;

    QToolBox *toolBox;
    Q3WidgetStack *stack;
    Q3PtrList <CategoryInterface> categories;

    CategoryPage* category_page;

    virtual void languageChange();

private slots:
    void ok_btn_clicked();
    void cancel_btn_clicked();
    void setBkColor(int );

};
#endif
