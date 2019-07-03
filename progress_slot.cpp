/*************************************************************************************************/
/****                                 FILE:progress_slot.cpp                                  ****/
/*************************************************************************************************/

#include <math.h>
#include "progress_slot.h"
#include <qapplication.h>

/*************************************************************************************************/
/****                              FUNCTION:ProgressSlot::ProgressSlot                        ****/
/*************************************************************************************************/

ProgressSlot::ProgressSlot(QWidget *parent, const char *name, const QString labeltext) : Q3ProgressDialog(labeltext, 0, 100, parent, name)
{
    offset = 0;
    weight = 100;
    setCaption(tr("Progress Bar"));
    resize(400,60);
    show();
}
/*************************************************************************************************/
/****                              FUNCTION:ProgressSlot::set_prog_percent                    ****/
/*************************************************************************************************/
void ProgressSlot::setOffsetWeight(int offset_val, int weight_val)
{
    offset = offset_val;
    weight = weight_val;
}
/*************************************************************************************************/
/****                              FUNCTION:ProgressSlot::set_prog_percent                    ****/
/*************************************************************************************************/
void ProgressSlot::set_prog_percent(int n)
{
    int psteps = (int) floor(offset + weight*n/100.0);

    setProgress(psteps);
    qApp->processEvents();
//    if (psteps > 100) {
//        cancel();
//    }
}
/*************************************************************************************************/
