/******************************************************************************************/
/**** FILE: hot_color.h                                                                ****/
/******************************************************************************************/

#ifndef HOT_COLOR_H
#define HOT_COLOR_H

#include <QColor>

/******************************************************************************************/
/**** CLASS: HotColorClass                                                             ****/
/******************************************************************************************/
class HotColorClass
{
public:
    HotColorClass();
    ~HotColorClass();
    int get_color(int i, int n);
    int get_color(int i);

    QColor cal_color(double f);
    // value = ((R*256)+G)*256+B
    int cal_color_value(double f);

    QColor cal_color(int i, int num);
    // value = ((R*256)+G)*256+B
    int cal_color_value(int i, int num);

private:
    int max_num_color;
    int **color_list;
};
/******************************************************************************************/

#endif
