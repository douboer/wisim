/**********************************************************************************/
/***                               FILE:progress_slot.h                         ***/
/**********************************************************************************/

#ifndef PROGRESS_SLOT_H
#define PROGRESS_SLOT_H

#include <q3progressdialog.h>

/**********************************************************************************/
/***                          class:ProgressSlot                                ***/
/**********************************************************************************/
class ProgressSlot : public Q3ProgressDialog
{
    Q_OBJECT
public:
    ProgressSlot(QWidget *parent, const char *name, const QString label_text = "");
    void setOffsetWeight(int offset_val, int weight_val);

public slots:
    void set_prog_percent(int psteps);

private slots:
//    void abort_clicked();

private:
    int offset, weight;
};
/**********************************************************************************/

#endif
