/******************************************************************************************/
/***                           FILE: num_sector_apply.h                                ****/
/******************************************************************************************/
#include "qdialog.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
//Added by qt3to4:
#include <Q3GridLayout>

/******************************************************************************************/
/****                            CLASS: NumSectorApply                                 ****/
/******************************************************************************************/
class NumSectorApply: public QDialog
{
public:
    NumSectorApply(QWidget* parent, const char* name);		
    ~NumSectorApply(){};		

    QLineEdit *cell_num_val[20];
    QLineEdit *sector_num_val[20];
    Q3GridLayout *num_sector_grid;
    QLabel *num_sector_head_lbl;	
    QLabel *cell_num_lbl[20];
    QLabel *sector_num_lbl[20];
    QPushButton *num_sector_ok_btn;
};
