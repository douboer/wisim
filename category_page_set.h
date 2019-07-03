#ifndef CATEGRORY_PAGE_SET_H
#define CATEGRORY_PAGE_SET_H

#include <stdlib.h>
#include <qapplication.h>
#include <qimage.h>
#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>
#include <q3widgetstack.h>
#include <qwidget.h>

class generalParam;
class WLANGeneralParam;
class psDcaParam;
class csDcaParam;
class CategoryWid;
class Q3WidgetStack;
class NetworkClass;

class CategoryInterface : public QObject
{
    Q_OBJECT
public:
    CategoryInterface( Q3WidgetStack *s ) : m_stack( s ) {}
    virtual ~CategoryInterface() {}
    virtual QString name() const = 0;
    virtual QIcon icon(int ) const = 0;
    virtual int numCategories() const = 0;
    virtual QString categoryName( int i ) const = 0;
    virtual QIcon categoryIcon( int i ) const = 0;
    virtual int categoryOffset() const = 0;

public slots:
   virtual void setCurrentCategory( int i ) = 0;

protected:
   Q3WidgetStack *m_stack;
};


class CategoryPage : public CategoryInterface
{
public:
    CategoryPage( NetworkClass *np, Q3WidgetStack *s, CategoryWid* ) ;//: CategoryInterface( s ), created( FALSE ); 
    ~CategoryPage();

    QString name() const { return "Parameters setting"; }
    QIcon icon(int) const;// { return QPixmap( widicon ); }
    int numCategories() const { return 4; }   //the windown's number of every toolbox page
    QString categoryName( int ) const;

    QIcon categoryIcon( int ) const { return QIcon(); }
    void setCurrentCategory( int );

    void create();
    int categoryOffset() const { return 0; }

    generalParam* gp;
    WLANGeneralParam* wgp;
    psDcaParam* pp;
    csDcaParam* cp;


private:
    bool created;

    NetworkClass* np;

    CategoryWid* category_wid;

private slots:
//    void setBkColor(int );
};
#endif
