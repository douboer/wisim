/****************************************************************************
**   mlistview.cpp
****************************************************************************/
#include <q3listview.h>

#include "WiSim_gui.h"
#include "mlistviewitem.h"

mListViewItem::mListViewItem(Q3ListView *parent)
    : Q3ListViewItem( parent )
{
}

mListViewItem::mListViewItem(mListViewItem *parent)
    : Q3ListViewItem( parent )
{
}

mListViewItem::~mListViewItem()
{
}

int mListViewItem::compare( Q3ListViewItem *i, int col , bool ascending ) const
{
        QString k1 = key(col, ascending);
        QString k2 = i->key(col, ascending);

        return(qstringcmp(k1, k2));
}
//end mlistview.cpp
