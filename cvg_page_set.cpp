
/******************************************************************************************
**** PROGRAM: cvg_page_set.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
#include "cvg_page_set.h"
#include "icons_test.h"
#include "cvg_info_wid.h"
#include "cvg_maindia.h"
#include "cvg_info_level_wid.h"

#include "wisim.h"

CvgLayerPage::CvgLayerPage( NetworkClass* np_param, CvgAnaDia* v_cvg_dia )
    : np( np_param )
{
  extension = false; //TRUE;
    
    sub_item_name = QString( "xxx" );
    create( np_param, v_cvg_dia );
}

CvgLayerPage::~CvgLayerPage()
{
     if (m_cvg_wid) 
         delete m_cvg_wid;
     if (m_cvg_item)
         delete m_cvg_item;
}

void CvgLayerPage::setNameSubItem( QString& sb )
{
    sub_item_name = sb;
}

QString CvgLayerPage::nameSubItem( ) const 
{
    return sub_item_name;
}

QString CvgLayerPage::typeSubItem( ) const
{
    return QString( "Layer");
}
/*
QIconSet CvgLayerPage::cvgIcon(int i ) const
{
    return QPixmap( QString::null );
}
*/
void CvgLayerPage::create( NetworkClass* np_param, CvgAnaDia* v_cvg_dia )
{
    m_cvg_wid = new CvgInfoLayerWid( np );
    Q3ListViewItem* xxx = v_cvg_dia->top_level_item;
    m_cvg_item = new Q3ListViewItem(xxx , 0);                     
    v_cvg_dia->top_level_item->insertItem( m_cvg_item );        
}

/*
   level type of coverage
 */
CvgLevelPage::CvgLevelPage( NetworkClass* np_param, CvgAnaDia* v_cvg_dia )
    : np( np_param )
{
  extension = false; //TRUE;
    
    sub_item_name = QString( "xxx" );
    create( np_param, v_cvg_dia );
}

CvgLevelPage::~CvgLevelPage()
{
     if (m_cvg_wid) 
         delete m_cvg_wid;
     if (m_cvg_item)
         delete m_cvg_item;
}

void CvgLevelPage::setNameSubItem( QString& sb )
{
    sub_item_name = sb;
} 

QString CvgLevelPage::nameSubItem( ) const 
{
    return sub_item_name;
}

QString CvgLevelPage::typeSubItem( ) const
{
    return QString( "level");
}
/*
QIconSet CvgLevelPage::cvgIcon(int i ) const
{
    return QPixmap( QString::null );
}
*/
void CvgLevelPage::create( NetworkClass* np_param, CvgAnaDia* v_cvg_dia )
{
    m_cvg_wid = new CvgInfoLevelWid(np);
    Q3ListViewItem* xxx = v_cvg_dia->top_level_item;
    m_cvg_item = new Q3ListViewItem(xxx , 0);                   //define in interface head file
//    v_cvg_dia->top_level_item->insertItem( m_cvg_item );     //modifing
}


// ----------------------------
// Memebers of CvgPAPage class
// ----------------------------
CvgPAPage::CvgPAPage( NetworkClass* np_param, bool bHasThrd,  CvgAnaDia* v_cvg_dia )
    : np( np_param )
{
  extension = false; //TRUE;
    
    sub_item_name = QString( "xxx" );
    create( np_param, bHasThrd, v_cvg_dia );
}

CvgPAPage::~CvgPAPage()
{
     if (m_cvg_wid) 
         delete m_cvg_wid;
     if (m_cvg_item)
         delete m_cvg_item;
}

void CvgPAPage::setNameSubItem( QString& sb )
{
    sub_item_name = sb;
}

QString CvgPAPage::nameSubItem( ) const 
{
    return sub_item_name;
}

QString CvgPAPage::typeSubItem( ) const
{
    return QString( "PA");
}
/*
QIconSet CvgPAPage::cvgIcon(int i ) const
{
    return QPixmap( QString::null );
}
*/
void CvgPAPage::create( NetworkClass* np_param, bool bHasThrd, CvgAnaDia* v_cvg_dia )
{
    m_cvg_wid = new CvgInfoPAWid( np, bHasThrd );
    Q3ListViewItem* xxx = v_cvg_dia->top_level_item;
    m_cvg_item = new Q3ListViewItem(xxx , 0);                     
    v_cvg_dia->top_level_item->insertItem( m_cvg_item );        
}
