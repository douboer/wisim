/******************************************************************************************/
/****                           FILE: num_sector_apply.cpp                             ****/
/******************************************************************************************/

#include <stdlib.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include "num_sector_apply.h"

/******************************************************************************************/
/****                          FUNCTION:  NumSectorApply                               ****/
/******************************************************************************************/
NumSectorApply::NumSectorApply(QWidget* parent, const char* name) : QDialog(parent, name, true)
{
    setCaption(name);

    num_sector_grid = new Q3GridLayout(this, 15, 4, 5, 5);
    num_sector_head_lbl = new QLabel(qApp->translate("NumSectorApply", "------ Sector Group ------"), this);
    num_sector_grid->addMultiCellWidget(num_sector_head_lbl, 0, 0, 0, 3, Qt::AlignHCenter);
}
//-------------------------------------------------------------------------------------------
