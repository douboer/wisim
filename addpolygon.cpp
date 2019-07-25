/******************************************************************************************/
/**** FILE: addpolygon.cpp                                                             ****/
/******************************************************************************************/

#include "wisim_gui.h"
#include "wisim.h"
#include "gconst.h"
#include "qapplication.h"
#include "qmessagebox.h"
//Added by qt3to4:
#include <Q3PointArray>
#include <QMouseEvent>

#define CGDEBUG 0

/******************************************************************************************/
/**** FUNCTION: FigureEditor::addPolygon                                               ****/
/******************************************************************************************/

void FigureEditor::addPolygon(int mouseAction, QMouseEvent* e)
{
    static int count = 0;
    static Q3CanvasEllipse* ellipse[200];
    static Q3CanvasLine* edge[200];
    static Q3PointArray pa(0);
    Q3CanvasPolygon* polygon;

    double s_one = 0;
    double s_two = 0;
    double s_three = 0;
    double s_four = 0;
    int *crossline_value = new int [200];
    int sign=1;
    static int flag = 1;

    //QCanvasLine* line = new QCanvasLine(canvas());
    static Q3CanvasLine* line;
    int canvas_x;
    int canvas_y;

    for (int i=0;i<=199;i++) {crossline_value[i]=1;}
    switch(mouseAction) {
        case GConst::mousePress:

#if CGDEBUG
            printf("Mouse Pressed \n");
#endif

            if (e->button() == Qt::LeftButton) {
                ellipse[count] = new Q3CanvasEllipse(5,5,canvas());
                ellipse[count]->setBrush( QColor(Qt::red) );
                ellipse[count]->move(e->x(),e->y());
                ellipse[count]->setZ(GConst::selectRegionZ);
                ellipse[count]->show();

                line = new Q3CanvasLine(canvas());
                line->setZ(GConst::selectRegionZ);

                pa.resize(count+1);
                pa[count] = e->pos();

                if (count <= 2) {
                    edge[count] = line;
                }

                if (count > 2) {
                    edge[count] = line;
                    for (int i=0;i<count-2;i++) {
                        s_one=(pa.point(count).y()-pa.point(i).y())*(pa.point(i+1).x()-pa.point(i).x())-(pa.point(count).x()-pa.point(i).x())*(pa.point(i+1).y()-pa.point(i).y());
                        s_two=(pa[count-1].y()-pa[i].y())*(pa[i+1].x()-pa[i].x())-(pa[count-1].x()-pa[i].x())*(pa[i+1].y()-pa[i].y());
                        s_three=(pa[i].y()-pa[count].y())*(pa[count-1].x()-pa[count].x())-(pa[i].x()-pa[count].x())*(pa[count-1].y()-pa[count].y());
                        s_four=(pa[i+1].y()-pa[count].y())*(pa[count-1].x()-pa[count].x())-(pa[i+1].x()-pa[count].x())*(pa[count-1].y()-pa[count].y());

#if CGDEBUG
                        printf("\n%f",s_one);
                        printf("\n%f",s_two);
                        printf("\n%f",s_three);
                        printf("\n%f",s_four);
#endif
                        if (s_one*s_two<=0 && s_three*s_four<=0) {
                            crossline_value[i] = 0;
                        }  else {
                            crossline_value[i] = 1;
                        }

#if CGDEBUG
                        printf("\n linecross occur? line [%d] &line [%d]  %d\n",i+1,count,crossline_value[i]);
#endif

                        sign=sign*crossline_value[i];

#if CGDEBUG
                        printf("\ntext sign:sign = %d\n",sign);
#endif
                    }

#if CGDEBUG
                    printf("\nsign = %d\n",sign);
#endif
                    flag = sign;

                    if (sign == 0) {
#if CGDEBUG
                        printf("lines crossing,please draw the line again!");
#endif
                        //edge[count-1]->setVisible(false);
                        line->setVisible(false);
                        ellipse[count]->setVisible(false);
                        edge[count]->setVisible(false);
                        count = count-1;
                        //edge[count] = line;
                        edge[count]->setVisible(false);
                        edge[count] = line;
                    }
                }

#if CGDEBUG
                printf("Setting pa[%d]\n", count);
#endif
                canvas_to_xy(canvas_x,canvas_y,e->x(),e->y());

#if CGDEBUG
                printf("x = %d\n",canvas_x);
                printf("y = %d\n",canvas_y);
#endif
                count++;
            } else if (e->button() == Qt::RightButton) {
                if (count<=2) {
                    polygon = (Q3CanvasPolygon *) NULL;
                } else {
                    polygon = new Q3CanvasPolygon(canvas());
                    polygon->setPoints(pa);
                    polygon->setBrush( QColor(Qt::green) );
                    polygon->setZ(GConst::selectRegionZ);
                    polygon->show();
                }

                for (int i=0;i<count;i++) {
                    delete ellipse[i];
                    delete edge[i];
                }
                setMouseMode(GConst::selectMode);
                count = 0;
                emit done_drawing_polygon(polygon);

#if CGDEBUG
                printf("Right Button\n");
#endif
            }
            break;

        case GConst::mouseMove:
            //printf("Mouse Moved \n");

            if (count >= 1) {
                line->setPoints(pa.point(count-1).x(),pa.point(count-1).y(),e->x(),e->y());
                line->show();
            }
            break;
        case GConst::mouseRelease:
#if CGDEBUG
            printf("Mouse Released\n");
#endif
            break;
        case GConst::mouseDoubleClick:
#if CGDEBUG
            printf("Mouse Double-Clicked \n");
#endif
            break;
    }
}
/******************************************************************************************/

#undef CGDEBUG
