/****************************************************************************
** mlistview.h 
** list view class that rewrite compare function 
****************************************************************************/
#ifndef MLISTVIEW_H
#define MLISTVIEW_H

class Q3ListView;
class Q3ListViewItem;

class mListViewItem : public Q3ListViewItem
{
public:    
    mListViewItem( Q3ListView *parent ) ;
    mListViewItem( mListViewItem *parent );

    ~mListViewItem();

    int compare( Q3ListViewItem *i, int , bool ) const;
};
#endif
