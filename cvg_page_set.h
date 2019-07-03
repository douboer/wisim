
/******************************************************************************************
**** PROGRAM: cvg_page_set.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#ifndef CVG_PAGE_SET_H
#define CVG_PAGE_SET_H

#include <stdlib.h>
#include <qapplication.h>
#include <qimage.h>
#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>
#include <q3widgetstack.h>
#include <qwidget.h>

#include <qstring.h>
#include <qicon.h>
#include <qobject.h>
#include <q3listview.h>

#include "cvg_info_wid.h"
#include "cvg_info_level_wid.h"

class NetworkClass;
class CvgAnaDia;
class CvgThrTable;
//class CvgInfoLayerWid;
//class CvgInfoLevelWid;

class CvgInterface : public QObject
{
public:
    CvgInterface(){ }             
    virtual ~CvgInterface() {}

    virtual QString name() const = 0;
    virtual QString nameSubItem( ) const = 0;
    virtual QString typeSubItem() const = 0;
    //    virtual QIconSet cvgIcon( int i ) const = 0;

    int item_index;
    QString sub_item_name;
    //if advance item is visibiable, this value is TRUE
    bool extension;

    Q3ListViewItem* m_cvg_item;

};

/* 
    new name 
    class CvgLayerPage : public CvgInterface
*/
class CvgLayerPage : public CvgInterface
{
public:
    CvgLayerPage( NetworkClass*,  CvgAnaDia*  );
    ~CvgLayerPage();

    QString name() const { return "Coverage analysis : Layer and SIR Layer Type"; }
    QIcon icon() const;

    QString nameSubItem( ) const;
    void setNameSubItem( QString& ); 
    QString typeSubItem( ) const;
        
    //    QIconSet cvgIcon( int ) const; // { return QIconSet(); }

//    void itemCount() { return m_item_count; }
//    void itemIndex() { return m_item_index; }
    
    void create( NetworkClass*, CvgAnaDia* );

    int layer_item_index;
    CvgInfoLayerWid* m_cvg_wid;    
private:
    NetworkClass* np;

    QString sub_item_name;

};

class CvgLevelPage : public CvgInterface
{
public:
    CvgLevelPage( NetworkClass*,  CvgAnaDia*  );
    ~CvgLevelPage();

    QString name() const { return "Coverage analysis : Level Type"; }
    QIcon icon() const;

    QString nameSubItem( ) const;
    void setNameSubItem( QString& ); 
    QString typeSubItem( ) const;
        
    //    QIconSet cvgIcon( int ) const; // { return QIconSet(); }
    
    void create( NetworkClass*, CvgAnaDia* );

    int level_item_index;

    CvgInfoLevelWid* m_cvg_wid;       //CvgInfoWid* 
private:
    NetworkClass* np;
};

// A new class as an interface for display PA infomation

class CvgPAPage : public CvgInterface
{
public:
    CvgPAPage( NetworkClass*, bool bHasThrd,  CvgAnaDia*  );
    ~CvgPAPage();

    QString name() const { return "Coverage analysis : Level Type"; }
    QIcon icon() const;

    QString nameSubItem( ) const;
    void setNameSubItem( QString& ); 
    QString typeSubItem( ) const;
        
    //    QIconSet cvgIcon( int ) const; // { return QIconSet(); }
    
    void create( NetworkClass*, bool,  CvgAnaDia* );

    int level_item_index;

    bool bHasThrd;

    CvgInfoPAWid* m_cvg_wid;       //CvgInfoWid* 


private:
    NetworkClass* np;
};
#endif
