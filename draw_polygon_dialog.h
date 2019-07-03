/******************************************************************************************/
/**** PROGRAM: draw_polygon_dialog.h                                                   ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef DRAW_POLYGON_H 
#define DRAW_POLYGON_H

#include <qdialog.h>

class FigureEditor;
class Q3CanvasPolygon;
class QComboBox;
class QCheckBox;
class PHSNetworkClass;
/******************************************************************************************/
/**** Class DrawPolygonDialog                                                          ****/
/******************************************************************************************/
class DrawPolygonDialog : public QDialog
{
    Q_OBJECT
public:
    DrawPolygonDialog(FigureEditor *s_editor, QWidget* parent, int rtti_val);
    ~DrawPolygonDialog();

private slots:
    void draw_btn_clicked();
    void cancel_btn_clicked();
    void done_drawing(Q3CanvasPolygon* polygon);

private:
    int polygon_type;
    FigureEditor *editor;
    QComboBox *traffic_comboBox;
    QCheckBox *prune_checkBox;
};

#endif
