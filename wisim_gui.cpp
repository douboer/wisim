/******************************************************************************************/
/**** PROGRAM: wisim_gui.cpp                                                         ****/
/**** Michael Mandell 2/19/03                                                          ****/
/******************************************************************************************/
/**** C++ GUI for wisim using qt                                                     ****/
/******************************************************************************************/

#include <string.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qevent.h>
#include <q3filedialog.h>
// #include <qgb18030codec.h>
// #include <qimage.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qprinter.h>
#include <q3progressdialog.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <Q3MemArray>

#include "cconst.h"
#include "wisim_gui.h"
#include "coverage.h"
#include "draw_polygon_dialog.h"
#include "gcall.h"
#include "icons.h"
#include "list.h"
#include "main_window.h"
#include "map_clutter.h"
#include "map_layer.h"
#include "pixmap_item.h"
#include "polygon.h"
#include "prop_model.h"
#include "read_mif_dia.h"
#include "road_test_data.h"
#include "sectorparamDia.h"
#include "shift_dialog.h"
#include "tooltip.h"
#include "traffic_type.h"
#include "utm_conversion.h"
#include "visibility_window.h"
#include "phs.h"

using namespace std;

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
QPixmap         ** GCellClass          :: pixmap_list            = (QPixmap         **) NULL;
QPixmap         ** GCellClass          :: selected_pixmap_list   = (QPixmap         **) NULL;
int                GCellClass          :: size_selected          = 0;
int                GCellClass          :: size                   = 0;
QBitmap **         GCellClass          :: bm_list                = (QBitmap         **) NULL;
QBitmap **         GCellClass          :: selected_bm_list       = (QBitmap         **) NULL;
VisibilityWindow * VisibilityList      :: visibility_window      = (VisibilityWindow *) NULL;
VisibilityWindow * VisibilityCheckItem :: visibility_window      = (VisibilityWindow *) NULL;
VisibilityWindow * VisibilityItem      :: visibility_window      = (VisibilityWindow *) NULL;
NetworkClass     * VisibilityWindow    :: np                     = (NetworkClass     *) NULL;
NetworkClass     * VisibilityList      :: np                     = (NetworkClass     *) NULL;
NetworkClass     * VisibilityItem      :: np                     = (NetworkClass     *) NULL;
NetworkClass     * VisibilityCheckItem :: np                     = (NetworkClass     *) NULL;
/******************************************************************************************/

extern QFont *fixed_width_font;

void skipLine( FILE* );
int skipFields( FILE*, int, char );
int readField( char*, FILE*, int, char );
char * trim(char * src);

/******************************************************************************************/
/**** FUNCTION: FigureEditor::FigureEditor                                             ****/
/******************************************************************************************/
FigureEditor::FigureEditor(
        Q3Canvas *c, NetworkClass *np_param, QWidget* parent,
        const char* name, Qt::WFlags f) :
    Q3CanvasView(c,parent,name,f)
{
    np = np_param;
    zoom = 1.0;
    // setMouseMode(GConst::selectMode);
    mouseMode = GConst::selectMode;

#if DEBUG_DRAW_POLYGON
    margin = 0.0;
#else 
    margin = 1.0;
#endif

    bgr_pm     = (PixmapItem *) NULL;

    selectCell = (GCellClass *) NULL;
    selectRegion = (Q3CanvasRectangle *) NULL;
    ruler = (RulerClass *) NULL;

    select_cell_list = new ListClass<int>(0);

    // Porting to QT4
    //dt = new DynamicTip( this );
    // gbc = new QGb18030Codec;

    /*  CG
    c_pmXL = 0;
    c_pmXH = 799;
    c_pmYL = -599;
    c_pmYH = 0;
    canvas()->resize( 800, 600);
    */

    viewport()->setMouseTracking( TRUE );
    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    setVScrollBarMode(Q3ScrollView::AlwaysOff);

    systemBoundaryOutlineColor  = QColor(0,0,0);
    systemBoundaryBrushStyle    = Qt::Dense5Pattern; // xxx Qt::NoBrush;

    mapLayerPolygonOutlineColor = QColor(0,0,0);
    mapLayerPolygonBrushStyle   = Qt::Dense5Pattern;

    coverageBrushStyle          = Qt::SolidPattern;

    printer = (QPrinter *) NULL;

    printrect = (PrintRect *) NULL;

    pcp = (PrintDialog *) NULL;

    excel_file = (char *) NULL;

    mousePosition = new PositionClass();

    geometry_filename = QString();

    if_show_handset_anomalies = 0;
    if_show_handset_noise = 0;
    if_show_3ho = 0;
    //if_show_new_location = 0;
    num_noise = 0;
    num_anomalies = 0;
    ho_show_index = 0;
    min_index = 0;
    max_index = 0;
    start_index = 0;
    end_index = 0;
    num_cs_new = 0;

    // clear vector data
    vec_anomaly_x.clear();
    vec_anomaly_y.clear();
    vec_source_cs.clear();
    vec_dest_cs.clear();
    vec_anomaly_index.clear();
    vec_noise_begin_x.clear();
    vec_noise_begin_y.clear();
    vec_noise_end_x.clear();
    vec_noise_end_y.clear();
    vec_noise_index.clear();
    vec_cs_new_x.clear();
    vec_cs_new_y.clear();
    vec_cs_index.clear();
    vec_dis.clear();
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::~FigureEditor                                            ****/
/******************************************************************************************/
FigureEditor::~FigureEditor()
{
    //if_show_new_location = 0;    
    clear();
    delete mousePosition;
    delete select_cell_list;

    //delete dt;

    if (excel_file) {
         free(excel_file);
         excel_file = (char *) NULL;
    }
}

void FigureEditor::get_handset_anomalies(char* filename)
{
    //sprintf(np->msg,"%s",filename);
    //PRMSG(stdout,np->msg);

    FILE* fp;
    if( NULL==(fp=fopen(filename,"r")) ) {
        sprintf(np->msg,"cannot open file %s",filename);
        PRMSG(stdout,np->msg);
        if_show_handset_anomalies = 0;
        return;
    }
    
    char str[20];
    int is_anomaly = 0;
    double lon, lat, lon1, lat1;
    double posn_x, posn_y;
    int idx_x, idx_y;
    char source_csid[20];
    char dest_csid[20];
    unsigned char* source_csid_hex;
    unsigned char* dest_csid_hex;
    int source_idx = -1;
    int dest_idx = -1;
    CellClass *cell;
    SectorClass *sector;
    PHSSectorClass *sector1;
    int num_line=0;

    while( !feof( fp ) ) {
        num_line++;
        is_anomaly = 0;

        if( readField( str, fp, 0, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
#ifdef __linux__
        if( strcasecmp(trim(str), "HO" ) != 0 && strcmp( trim(str), "ÇÐ»»" ) != 0 && strcasecmp(trim(str), "HANDOVER" ) != 0 ) {
#else        
        if( strcmp( strupr(trim(str)), "HO" ) != 0 && strcmp( trim(str), "ÇÐ»»" ) != 0 && strcmp( strupr(trim(str)), "HANDOVER" ) != 0 ) {
#endif
            skipLine(fp);
            continue;
        }
        if( readField( str, fp, 1, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
#ifdef __linux__
        if( strcasecmp(trim(str), "INDEX" ) == 0 || strcmp( trim(str), "ÐòºÅ" ) == 0 ) {
#else
        if( strcmp( strupr(trim(str)), "INDEX" ) == 0 || strcmp( trim(str), "ÐòºÅ" ) == 0 ) {
#endif
            skipLine(fp);
            continue;
        }
        if( readField( str, fp, 11, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        /*        
        if(atof(str)>4.0)
            is_anomaly = 1;
        else {
            if( readField( str, fp, 12, '\t' ) != 0 ) {
                skipLine(fp);
                continue;
            }
            if(atoi(str)==1)
                is_anomaly=1;
        }*/
        
        is_anomaly = 1;
        if(is_anomaly) {
            if( readField( str, fp, 14, '\t' ) != 0 ) {
                skipLine(fp);
                continue;
            }
            //--liutao,070419
            if(!strcmp(trim(str),"")) {
                skipLine(fp);
                continue;
            }
            lon = atof(trim(str));
            if( readField( str, fp, 15, '\t' ) != 0 ) {
                skipLine(fp);
                continue;
            }
            //--liutao,070419
            if(!strcmp(trim(str),"")) {
                skipLine(fp);
                continue;
            }
            lat = atof(trim(str));

            LLtoUTM( lon, lat, posn_x,  posn_y, np->utm_zone, np->utm_north, 
                np->utm_equatorial_radius, np->utm_eccentricity_sq);

            check_grid_val(posn_x, np->resolution, np->system_startx, &idx_x);
            check_grid_val(posn_y, np->resolution, np->system_starty, &idx_y);

            vec_anomaly_x.push_back(idx_x);
            vec_anomaly_y.push_back(idx_y);

            if( readField( str, fp, 4, '\t' ) != 0 ) 
                strcpy(source_csid,"");
            else {
                strncpy(source_csid, trim(str), 10);
            }
            if( readField( str, fp, 7, '\t' ) != 0 ) {
                strcpy(dest_csid,"");
            }
            else {
                strncpy(dest_csid, trim(str), 10);
            }
            source_csid[10] = 0;
            dest_csid[10] = 0;

            source_csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);
            dest_csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);    
            hexstr_to_hex(source_csid_hex, source_csid, PHSSectorClass::csid_byte_length);
            hexstr_to_hex(dest_csid_hex, dest_csid, PHSSectorClass::csid_byte_length);
			
            //find cs_idx
            for (int cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
                cell = np->cell_list[cell_idx];
                for (int sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                    sector1 = (PHSSectorClass*) cell->sector_list[sector_idx];
                    if (strncmp((char *) sector1->csid_hex, (char *) source_csid_hex, PHSSectorClass::csid_byte_length)==0) {
                        source_idx = cell_idx;
                    }
                    if (strncmp((char *) sector1->csid_hex, (char *) dest_csid_hex, PHSSectorClass::csid_byte_length)==0) {
                        dest_idx = cell_idx;
                    }
                }
            }
            vec_source_cs.push_back(source_idx);
            vec_dest_cs.push_back(dest_idx);

            vec_anomaly_index.push_back(num_line);

            if ( num_anomalies == 998 )
            {
                std::cout << "test" << std::endl;
            }

            num_anomalies++;
            
        }
        
        skipLine(fp);
    }
    min_index = 0;
    max_index = num_anomalies-1;
    fclose(fp);
    
    if( NULL==(fp=fopen(filename,"r")) ) {
        sprintf(np->msg,"cannot open file %s",filename);
        PRMSG(stdout,np->msg);
        if_show_handset_noise = 0;
        return;
    }


    num_line = 0;
    num_noise = 0;
    while( !feof( fp ) ) {
        num_line++;
        if( readField( str, fp, 0, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        if( strcmp( str, "Noise" ) != 0 ) {
            skipLine(fp);
            continue;
        }
        if( readField( str, fp, 1, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        if( strcmp( str, "BeginTime" ) == 0 ) {
            skipLine(fp);
            continue;
        }
        
        if( readField( str, fp, 4, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        lon = atof(str);
        if( readField( str, fp, 5, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        lat = atof(str);
        if( readField( str, fp, 6, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        lon1 = atof(str);
        if( readField( str, fp, 7, '\t' ) != 0 ) {
            skipLine(fp);
            continue;
        }
        lat1 = atof(str);
        
        LLtoUTM( lon, lat, posn_x,  posn_y, np->utm_zone, np->utm_north, 
            np->utm_equatorial_radius, np->utm_eccentricity_sq);

        check_grid_val(posn_x, np->resolution, np->system_startx, &idx_x);
        check_grid_val(posn_y, np->resolution, np->system_starty, &idx_y);

        vec_noise_begin_x.push_back(idx_x);
        vec_noise_begin_y.push_back(idx_y);

        LLtoUTM( lon1, lat1, posn_x,  posn_y, np->utm_zone, np->utm_north, 
            np->utm_equatorial_radius, np->utm_eccentricity_sq);

        check_grid_val(posn_x, np->resolution, np->system_startx, &idx_x);
        check_grid_val(posn_y, np->resolution, np->system_starty, &idx_y);

        vec_noise_end_x.push_back(idx_x);
        vec_noise_end_y.push_back(idx_y);
        
        vec_noise_index.push_back(num_line);
        
        num_noise++;
        
        skipLine(fp);
    }
    fclose(fp);       

}

void FigureEditor::show_handset_anomalies()
{ 
    int begin_index = 0;
    /*
    if (if_show_3ho) 
        begin_index = ho_show_index - 1;
    if (begin_index < min_index)
        begin_index = min_index;
    int end_index = 0;
    if (if_show_3ho)
        end_index = ho_show_index + 1;
    if (end_index > max_index )
        end_index = max_index;
     */

    /*
    sprintf(np->msg,"min_index: %d", min_index);
    PRMSG(stdout,np->msg);
    sprintf(np->msg,"max_index: %d", max_index);
    PRMSG(stdout,np->msg);
    sprintf(np->msg,"ho_show_index: %d", ho_show_index);
    PRMSG(stdout,np->msg);
    sprintf(np->msg,"begin_index: %d", begin_index);
    PRMSG(stdout,np->msg);
    sprintf(np->msg,"end_index: %d", end_index);
    PRMSG(stdout,np->msg);
    */

    if(np == NULL)
        return;
    if(np->cell_list == NULL)
        return;

    for(int i=start_index; i<=end_index; i++) {
        int event_canvas_x, event_canvas_y;
        int source_x, source_y, dest_x, dest_y;
        int source_canvas_x, source_canvas_y, dest_canvas_x, dest_canvas_y;

        //draw points to show the anomalies locations
        xy_to_canvas(event_canvas_x, event_canvas_y, vec_anomaly_x[i], vec_anomaly_y[i]);

        //
        // to avoid the crash caused by lon-lat error --liutao,070419
        // - CG 10/16/2008
        //
        /*
        if(event_canvas_x>canvas()->width()-3 || event_canvas_x<3 ||
            event_canvas_y>canvas()->height()-3 || event_canvas_y<3) {
            char msg[100];
            sprintf(msg, "LON-LAT error in HO index: %d Line: %d. Line ignored.\n",i,vec_anomaly_index[i]);
            PRMSG(stdout, msg);
            continue;
        }
        */
        Q3CanvasRectangle* rect = new Q3CanvasRectangle(event_canvas_x-3,event_canvas_y-3,6,6,canvas());
        rect->setBrush(Qt::red);
        rect->show();
        rect->setZ(100);

        //draw points to show the anomalies locations
        Q3CanvasText* text = new Q3CanvasText(canvas());
        QString str_index;
        str_index.sprintf("%d_%d",i,vec_anomaly_index[i]);
        text->setText(str_index);
        text->show();
        text->setX(event_canvas_x+2);
        text->setY(event_canvas_y+2);
        text->setZ(100);

        //draw lines to show the handover source cs and dest cs
        if(vec_source_cs[i]!=-1) {
            source_x = np->cell_list[vec_source_cs[i]]->posn_x;
            source_y = np->cell_list[vec_source_cs[i]]->posn_y;

            xy_to_canvas(source_canvas_x, source_canvas_y, source_x, source_y);

            Q3CanvasLine* line = new Q3CanvasLine(canvas());
            line->setPen(QPen(Qt::red));
            line->setPoints(event_canvas_x, event_canvas_y, source_canvas_x, source_canvas_y);
            line->show();
            line->setZ(99);
        }
        if(vec_dest_cs[i]!=-1) {
            if(np->cell_list[vec_dest_cs[i]] == NULL) 
                return;
            dest_x = np->cell_list[vec_dest_cs[i]]->posn_x;
            dest_y = np->cell_list[vec_dest_cs[i]]->posn_y;
            xy_to_canvas(dest_canvas_x, dest_canvas_y, dest_x, dest_y);
            Q3CanvasLine* line = new Q3CanvasLine(canvas());
            line->setPen(QPen(Qt::blue));
            line->setPoints(event_canvas_x, event_canvas_y, dest_canvas_x, dest_canvas_y);
            line->show();
            line->setZ(99);
        }
    }
}


void FigureEditor::show_handset_noise()
{ 
    if(np == NULL)
        return;
    if(np->cell_list == NULL)
        return;

    for(int i=0; i<num_noise; i++) {
        int begin_canvas_x, begin_canvas_y, end_canvas_x, end_canvas_y;

        //draw points to show the noise start locations
        xy_to_canvas(begin_canvas_x, begin_canvas_y, vec_noise_begin_x[i], vec_noise_begin_y[i]);
        Q3CanvasEllipse * circle = new Q3CanvasEllipse (7,7,canvas());
        circle->setBrush(Qt::red);
        circle->show();
        circle->setX(begin_canvas_x);
        circle->setY(begin_canvas_y);
        circle->setZ(100);

        //draw points to show the noise end locations
        xy_to_canvas(end_canvas_x, end_canvas_y, vec_noise_end_x[i], vec_noise_end_y[i]);
        Q3CanvasEllipse * circle2 = new Q3CanvasEllipse (7,7,canvas());
        circle2->setBrush(Qt::green);
        circle2->show();
        circle2->setX(end_canvas_x);
        circle2->setY(end_canvas_y);
        circle2->setZ(100);

        //draw lines to connect the noise begin and end points 
        Q3CanvasLine* line = new Q3CanvasLine(canvas());
        line->setPen(QPen(Qt::green));
        line->setPoints(begin_canvas_x, begin_canvas_y, end_canvas_x, end_canvas_y);
        line->show();
        line->setZ(99);

        Q3CanvasText* text = new Q3CanvasText(canvas());
        QString str_index;
        str_index.setNum(vec_noise_index[i]);
        text->setText(str_index);
        text->show();
        text->setX(begin_canvas_x+2);
        text->setY(begin_canvas_y+2);
        text->setZ(101);
    }
}


void FigureEditor::show_cs_new_location()
{
    if(np == NULL)
            return;
    if(np->cell_list == NULL) 
            return;

    for(int i=0; i<num_cs_new; i++) {

        int cs_old_x, cs_old_y, cs_old_canvas_x, cs_old_canvas_y,cs_new_canvas_x, cs_new_canvas_y;
        xy_to_canvas(cs_new_canvas_x, cs_new_canvas_y, vec_cs_new_x[i], vec_cs_new_y[i]);

        if((np->cell_list)[vec_cs_index[i]] == NULL)
            return;

        cs_old_x = (np->cell_list)[vec_cs_index[i]]->posn_x;
        cs_old_y = (np->cell_list)[vec_cs_index[i]]->posn_y;
        /*FILE* fp = fopen("c:\\aaa.txt", "a+");
        fprintf(fp, "****************************\n");
        fprintf(fp, "%d\t%d\t\t", cs_old_x, cs_old_y );
        fflush(fp);*/
        xy_to_canvas(cs_old_canvas_x, cs_old_canvas_y, cs_old_x, cs_old_y);
        
        //fprintf(fp, "%d\t%d\n", cs_old_canvas_x, cs_old_canvas_y );
        //fflush(fp);


        //draw the new position
        Q3CanvasEllipse* circle = new Q3CanvasEllipse (8,8,canvas());
        if(vec_dis[i]<50)
            circle->setBrush(Qt::yellow);
        else if(vec_dis[i]<100)
            circle->setBrush(Qt::green);
        else
            circle->setBrush(Qt::blue);
        circle->show();
        circle->setX(cs_new_canvas_x);
        circle->setY(cs_new_canvas_y);
        circle->setZ(100);
        
        //draw the line connecting the old position with the new postion
        Q3CanvasLine* line = new Q3CanvasLine(canvas());
        line->setPen(QPen(Qt::blue));
        line->setPoints(cs_new_canvas_x, cs_new_canvas_y, cs_old_canvas_x, cs_old_canvas_y);
        line->show();
        line->setZ(99);
        
        //draw text if distance >= 150
        if (vec_dis[i]>=150){
            Q3CanvasText* text = new Q3CanvasText(canvas());
            QString str_index;
            str_index.sprintf("%d: %d",vec_cs_index[i],vec_dis[i]);
            text->setText(str_index);
            text->show();
            text->setX((cs_new_canvas_x+cs_old_canvas_x)/2);
            text->setY((cs_new_canvas_y+cs_old_canvas_y)/2-2);
            text->setZ(101);
        }
        
    }
}

/******************************************************************************************/
/**** FUNCTION: FigureEditor:: Dialog create functions                                 ****/
/******************************************************************************************/
void FigureEditor::create_draw_polygon_dialog(int type) { new DrawPolygonDialog(this, 0, type); }
void FigureEditor::create_read_mif_dialog()             { new ReadMIFDia(np, 0); }
void FigureEditor::create_shift_dialog(int type)        { new ShiftDialog(np, type, 0); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: FigureEditor:: simple "set" and "get" functions                        ****/
/******************************************************************************************/
double FigureEditor::getZoom()                                 { return(zoom); }
NetworkClass *FigureEditor::get_np()                           { return(np); }
void   FigureEditor::setVisibilityWindow(VisibilityWindow *vw) { visibility_window = vw; };
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: FigureEditor:: Set Mouse Mode functions                                ****/
/******************************************************************************************/
void FigureEditor::setScrollMode()      { setMouseMode(GConst::scrollMode); };

void FigureEditor::setSelectCell(int c) { selectCell = (GCellClass *) c; }

/******************************************************************************************/
/**** FUNCTION: FigureEditor::deleteCell                                               ****/
/******************************************************************************************/
void FigureEditor::deleteCell()
{
    sprintf(np->line_buf, "delete_cell -cell_idx %d", selectCell->getCellIdx());
    np->process_command(np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::setGroupColor                                            ****/
/******************************************************************************************/
void FigureEditor::setGroupColor()
{
    int i, init_color;
    char *chptr;

    if (select_cell_list->getSize()) {
        init_color = np->cell_list[(*select_cell_list)[0]]->color;
    } else {
        init_color = 0;
    }
    QColor qcolor = QColorDialog::getColor(init_color);
    if (qcolor.isValid()) {
        int color = (qcolor.rgb())&(0xFFFFFF);
        chptr = np->line_buf;
        chptr += sprintf(chptr, "set_color -cell %d -cell_idx \'", color);
        for (i=0; i<=select_cell_list->getSize()-1; i++) {
            chptr += sprintf(chptr, "%d%c", (*select_cell_list)[i], ((i==select_cell_list->getSize()-1)?'\'':' '));
        }
        np->process_command(np->line_buf);
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::groupSectors                                             ****/
/******************************************************************************************/
void FigureEditor::groupSectors()
{
    int i, cell_idx, sector_idx;
    char *chptr;

    chptr = np->line_buf;
    chptr += sprintf(chptr, "group -sectors \'");
    for (i=0; i<=select_cell_list->getSize()-1; i++) {
        cell_idx = (*select_cell_list)[i];
        for (sector_idx=0; sector_idx<=np->cell_list[cell_idx]->num_sector-1; sector_idx++) {
            chptr += sprintf(chptr, "%d_%d%c", cell_idx, sector_idx,
                 ((i==select_cell_list->getSize()-1)&&(sector_idx==np->cell_list[cell_idx]->num_sector-1)?'\'':' '));
        }
    }
    np->process_command(np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::deleteGroup                                              ****/
/******************************************************************************************/
void FigureEditor::deleteGroup()
{
    int i;
    char *chptr;

    chptr = np->line_buf;
    chptr += sprintf(chptr, "delete_cell -cell_idx \'");
    for (i=0; i<=select_cell_list->getSize()-1; i++) {
        chptr += sprintf(chptr, "%d%c", (*select_cell_list)[i], ((i==select_cell_list->getSize()-1)?'\'':' '));
    }
    delete select_cell_list;
    select_cell_list = new ListClass<int>(0);
    np->process_command(np->line_buf);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::clear_select_list                                        ****/
/******************************************************************************************/
void FigureEditor::clear_select_list()
{
    if (select_cell_list->getSize()) {
        select_cell_list->reset();
        setVisibility(GConst::cellRTTI);
        emit selection_changed(select_cell_list);
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::toggleRoadTestData                                       ****/
/******************************************************************************************/
void FigureEditor::toggleRoadTestData()
{
    int cell_num, cell_idx, sector_idx;
    CellClass *cell;
    SectorClass *sector;
    VisibilityList *vlist        = (VisibilityList *) NULL;
    VisibilityItem *roadTestDataViewItem = (VisibilityItem *) NULL;
    VisibilityItem *roadTestDataCellViewItem = (VisibilityItem *) NULL;
    VisibilityCheckItem *vci = (VisibilityCheckItem *) NULL;

    cell_num = selectCell->getCellIdx();
    cell_idx = cell_num & ((1<<np->bit_cell)-1);
    sector_idx = cell_num >> np->bit_cell;
    cell = np->cell_list[cell_idx];
    sector = cell->sector_list[sector_idx];

    printf("TOGGLE RTD FOR (%d, %d)\n", cell_idx, sector_idx);

    vlist = visibility_window->visibility_list;
    roadTestDataViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);

    if (roadTestDataViewItem) {
        roadTestDataCellViewItem = (VisibilityItem *) VisibilityList::findItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 0);
    }

    if (roadTestDataCellViewItem) {
        vci = (VisibilityCheckItem *) VisibilityList::findItem(roadTestDataCellViewItem, GConst::roadTestDataRTTI, cell_idx, sector_idx);
    }

    if (vci) {
        vci->setOn(vci->isOn() ? false : true);
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::toggleRoadTestDataGroup                                  ****/
/******************************************************************************************/
void FigureEditor::toggleRoadTestDataGroup()
{
    int i, cell_num, cell_idx, sector_idx, found_off;
    CellClass *cell;
    SectorClass *sector;
    VisibilityList *vlist        = (VisibilityList *) NULL;
    VisibilityItem *roadTestDataViewItem = (VisibilityItem *) NULL;
    VisibilityItem *roadTestDataCellViewItem = (VisibilityItem *) NULL;
    VisibilityCheckItem *vci = (VisibilityCheckItem *) NULL;
    VisibilityCheckItem *first_vci = (VisibilityCheckItem *) NULL;

    vlist = visibility_window->visibility_list;
    roadTestDataViewItem = (VisibilityItem *) VisibilityList::findItem(vlist, GConst::roadTestDataRTTI, 0);

    if (roadTestDataViewItem) {
        roadTestDataCellViewItem = (VisibilityItem *) VisibilityList::findItem(roadTestDataViewItem, GConst::roadTestDataRTTI, 0);
    }

    if (roadTestDataCellViewItem) {
        found_off = 0;
        for (i=0; (i<=select_cell_list->getSize()-1)&&(!found_off); i++) {
            cell_num = (*select_cell_list)[i];
            cell_idx = cell_num & ((1<<np->bit_cell)-1);
            sector_idx = cell_num >> np->bit_cell;
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            vci = (VisibilityCheckItem *) VisibilityList::findItem(roadTestDataCellViewItem, GConst::roadTestDataRTTI, cell_idx, sector_idx);
            if (vci) {
                if (!vci->isOn()) { found_off = 1; }
            }
        }

        for (i=0; (i<=select_cell_list->getSize()-1); i++) {
            cell_num = (*select_cell_list)[i];
            cell_idx = cell_num & ((1<<np->bit_cell)-1);
            sector_idx = cell_num >> np->bit_cell;
            cell = np->cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            vci = (VisibilityCheckItem *) VisibilityList::findItem(roadTestDataCellViewItem, GConst::roadTestDataRTTI, cell_idx, sector_idx);
            if (vci) {
                if (    ( ( found_off) && (vci->isOn() == false) )
                     || ( (!found_off) && (vci->isOn() == true ) ) ) {
                    if (!first_vci) {
                        visibility_window->blockSignals(true);
                        first_vci = vci;
                    } else {
                        vci->setOn((found_off ? true : false));
                    }
                }
            }
        }
        if (first_vci) {
            visibility_window->blockSignals(false);
            first_vci->setOn((found_off ? true : false));
        }
    }
}


/******************************************************************************************/
/**** FUNCTION: FigureEditor::toggleNoise                                              ****/
/******************************************************************************************/
void FigureEditor::toggleNoise()
{
    sprintf(np->line_buf, "toggle_noise");
    np->process_command(np->line_buf);
}


/******************************************************************************************/
/**** FUNCTION: FigureEditor::testSlotA                                                ****/
/******************************************************************************************/
void FigureEditor::testSlotA()
{

    QVector<QPoint> seg;

#if 1
    int n = 26;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 85717; y[0] = 32886;
    x[1] = 85716; y[1] = 32665;
    x[2] = 85842; y[2] = 32667;
    x[3] = 85982; y[3] = 32663;
    x[4] = 86029; y[4] = 32663;
    x[5] = 86036; y[5] = 32813;
    x[6] = 86028; y[6] = 32813;
    x[7] = 86025; y[7] = 32795;
    x[8] = 86006; y[8] = 32794;
    x[9] = 86008; y[9] = 32832;
    x[10] = 86022; y[10] = 32832;
    x[11] = 86024; y[11] = 32826;
    x[12] = 86035; y[12] = 32822;
    x[13] = 86035; y[13] = 32849;
    x[14] = 85897; y[14] = 32861;
    x[15] = 85897; y[15] = 32840;
    x[16] = 85911; y[16] = 32840;
    x[17] = 85909; y[17] = 32811;
    x[18] = 85927; y[18] = 32810;
    x[19] = 85929; y[19] = 32795;
    x[20] = 85873; y[20] = 32800;
    x[21] = 85874; y[21] = 32838;
    x[22] = 85887; y[22] = 32838;
    x[23] = 85886; y[23] = 32861;
    x[24] = 85832; y[24] = 32862;
    x[25] = 85832; y[25] = 32884;

    int min_x = 85747;
    int max_x = 87708;
    int min_y = 32885;
    int max_y = 34332;
#elif 0
    int n = 10;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 82462; y[0] = 33445;
    x[1] = 82461; y[1] = 33388;
    x[2] = 82526; y[2] = 33386;
    x[3] = 82526; y[3] = 33397;
    x[4] = 82537; y[4] = 33397;
    x[5] = 82537; y[5] = 33417;
    x[6] = 82494; y[6] = 33418;
    x[7] = 82495; y[7] = 33458;
    x[8] = 82473; y[8] = 33458;
    x[9] = 82473; y[9] = 33445;

    int min_x = 82495;
    int max_x = 92166;
    int min_y = 29644;
    int max_y = 36780;

#elif 0
    int n = 10;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 86432; y[0] = 36317;
    x[1] = 86423; y[1] = 36229;
    x[2] = 86466; y[2] = 36228;
    x[3] = 86467; y[3] = 36268;
    x[4] = 86497; y[4] = 36267;
    x[5] = 86499; y[5] = 36282;
    x[6] = 86470; y[6] = 36286;
    x[7] = 86470; y[7] = 36297;
    x[8] = 86492; y[8] = 36294;
    x[9] = 86493; y[9] = 36305;

    int min_x = 83785;
    int max_x = 90523;
    int min_y = 31340;
    int max_y = 36267;
#elif 0
    int n = 4;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 89608; y[0] = 62070;
    x[1] = 89620; y[1] = 61877;
    x[2] = 89792; y[2] = 61873;
    x[3] = 89785; y[3] = 62067;

    int min_x = 73499;
    int max_x = 119961;
    int min_y = 15710;
    int max_y = 49994;
#elif 0
    int n = 10;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 30545; y[0] = 41466;
    x[1] = 30736; y[1] = 41444;
    x[2] = 30733; y[2] = 41310;
    x[3] = 30624; y[3] = 41313;
    x[4] = 30622; y[4] = 41263;
    x[5] = 30556; y[5] = 41274;
    x[6] = 30548; y[6] = 41299;
    x[7] = 30465; y[7] = 41310;
    x[8] = 30467; y[8] = 41385;
    x[9] = 30550; y[9] = 41374;

    int min_x = 71371;
    int max_x = 108695;
    int min_y = 19273;
    int max_y = 46814;

#else
    int n = 18;
    int *x = IVECTOR(n);
    int *y = IVECTOR(n);
    x[0] = 86751; y[0] = 33513;
    x[1] = 86763; y[1] = 33513;
    x[2] = 86763; y[2] = 33480;
    x[3] = 86751; y[3] = 33480;
    x[4] = 86749; y[4] = 33411;
    x[5] = 86901; y[5] = 33413;
    x[6] = 86899; y[6] = 33478;
    x[7] = 86863; y[7] = 33478;
    x[8] = 86864; y[8] = 33520;
    x[9] = 86880; y[9] = 33520;
    x[10] = 86880; y[10] = 33494;
    x[11] = 86899; y[11] = 33494;
    x[12] = 86896; y[12] = 33602;
    x[13] = 86903; y[13] = 33748;
    x[14] = 86907; y[14] = 33830;
    x[15] = 86856; y[15] = 33831;
    x[16] = 86841; y[16] = 33827;
    x[17] = 86758; y[17] = 33828;

    int min_x = 86756;
    int max_x = 87530;
    int min_y = 33186;
    int max_y = 33757;

#endif

    comp_effective_polygon_segment( seg, n, x, y, min_x, max_x, min_y, max_y);

    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::testSlotB                                                ****/
/******************************************************************************************/
void FigureEditor::testSlotB()
{

#if 1
    int c_pmXL = 7613;
    int c_pmXH = 9904;
    int c_pmYL = -5268;
    int c_pmYH = -3577;
    zoom = 7.2467621485;
#elif 0
    int c_pmXL = 683;
    int c_pmXH = 2974;
    int c_pmYL = -2380;
    int c_pmYH = -689;
    zoom = 1.3921382612;
#endif

    int cx = c_pmXL + (c_pmXH-c_pmXL)/2;
    int cy = c_pmYL + (c_pmYH-c_pmYL)/2;

    regenCanvas(cx, cy);

    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::testSlotC                                                ****/
/******************************************************************************************/
void FigureEditor::testSlotC()
{
    int i = 0;

    printf("num_select_cell = %d\n", select_cell_list->getSize());
    printf("CELL LIST:");
    for (i=0; i<=select_cell_list->getSize()-1; i++) {
        printf(" %d", (*select_cell_list)[i]);
    }
    printf("\n");

    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::testSlotD                                                ****/
/******************************************************************************************/
void FigureEditor::testSlotD()
{
#if 1
    int tt_idx;
    VisibilityWindow *vw = visibility_window;
    CoverageClass *cvg;

    printf("vis_system_boundary = %X\n", vw->vis_system_boundary);
    printf("vis_cell            = %X\n", vw->vis_cell);
    printf("vis_cell_text       = %X\n", vw->vis_cell_text);
    printf("vis_map_height      = %X\n", vw->vis_map_height);

#if HAS_MONTE_CARLO
    printf("vis_traffic         = %X\n", vw->vis_traffic);
    if (np->traffic_type_list) {
        for (tt_idx=0; tt_idx<=np->num_traffic_type-1; tt_idx++) {
            printf("vec_vis_traffic[%d] = %X\n", tt_idx, vw->vec_vis_traffic[tt_idx]);
        }
    }
#endif

    printf("vis_subnet          = %X\n", vw->vis_subnet);
    for (tt_idx=0; tt_idx<=np->num_traffic_type-1; tt_idx++) {
        for (int subnet_idx=0; subnet_idx<=np->num_subnet[tt_idx]-1; subnet_idx++) {
            printf("vec_vis_subnet[%d][%d] = %X\n", tt_idx, subnet_idx, vw->vec_vis_subnet[tt_idx][subnet_idx]);
        }
    }

    printf("vis_map_clutter     = %X\n", vw->vis_map_clutter);
    if (np->map_clutter) {
        for (int clutter_idx=0; clutter_idx<=np->map_clutter->num_clutter_type-1; clutter_idx++) {
            printf("vec_vis_map_clutter[%d] = %X\n", clutter_idx, vw->vec_vis_map_clutter[clutter_idx]);
        }
    }

    printf("vis_map_layer       = %X\n", vw->vis_map_layer);
    for (int map_layer_idx=0; map_layer_idx<=np->map_layer_list->getSize()-1; map_layer_idx++) {
        printf("vec_vis_map_layer[%d] = %X\n", map_layer_idx, vw->vec_vis_map_layer[map_layer_idx]);
    }

    printf("vis_road_test_data  = %d\n", vw->vis_road_test_data);

    printf("vis_coverage        = %X\n", vw->vis_coverage);
    for (int cvg_idx=0; cvg_idx<=np->num_coverage_analysis-1; cvg_idx++) {
        cvg = np->coverage_list[cvg_idx];
        for (int scan_type_idx=0; scan_type_idx<=cvg->scan_list->getSize()-1; scan_type_idx++) {
            printf("vec_vis_coverage[%d][%d] = %X\n", cvg_idx, scan_type_idx, vw->vec_vis_coverage[cvg_idx][scan_type_idx]);
        }
    }
    printf("==============================================================\n");
    printf("VIS WIN POSN = %d %d\n", visibility_window->geometry().topLeft().x(), visibility_window->geometry().topLeft().y());
    printf("VIS LIST Width  = %d\n", visibility_window->visibility_list->width());
    printf("VIS LIST Height = %d\n", visibility_window->visibility_list->height());
    printf("Visible Width = %d\n",   visibility_window->visibility_list->visibleWidth());
    printf("Visible Height = %d\n",  visibility_window->visibility_list->visibleHeight());
    printf("Contents X = %d\n",      visibility_window->visibility_list->contentsX());
    printf("Contents Y = %d\n",      visibility_window->visibility_list->contentsY());
    printf("Contents Width = %d\n",  visibility_window->visibility_list->contentsWidth());
    printf("Contents Height = %d\n", visibility_window->visibility_list->contentsHeight());
    printf("Updates Enables = %s\n", (visibility_window->isUpdatesEnabled()==TRUE ? "TRUE" : "FALSE"));
    printf("==============================================================\n");
#else
    this->get_np()->map_layer_list[0]->num_polygon--;

    int cx = c_pmXL + (c_pmXH - c_pmXL)/2;
    int cy = c_pmYL + (c_pmYH - c_pmYL)/2;

    regenCanvas(cx, cy);
#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::clear                                                    ****/
/******************************************************************************************/
void FigureEditor::clear()
{    
    if (ruler) {
        ruler->clear();
    }

    Q3CanvasItemList list = canvas()->allItems();
    Q3CanvasItemList::Iterator it;

#if CDEBUG
    printf("calling FigureEditor::clear() clearing all canvas items ... \n");
#endif

    it = list.begin();
    while (it != list.end()) {
        if (   ((*it)->rtti() == GConst::polygonBoundarySegmentRTTI)
            || ((*it)->rtti() == GConst::callConnectionRTTI) ) {
            it = list.erase(it);
        } else {
            it++;
        }
    }

    for (it = list.begin(); it != list.end(); it++) {
        delete *it;
    }

    power_meter_line = (Q3CanvasLine *) NULL;
#if CDEBUG
    printf("All canvas items cleared\n");
#endif

    bgr_pm     = (PixmapItem *) NULL;
    

}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::clear                                                    ****/
/******************************************************************************************/
void FigureEditor::clear(int rtti_val)
{
    Q3CanvasItemList list = canvas()->allItems();
    Q3CanvasItemList::Iterator it;

#if 0
    printf("calling clear with RTTI = %d\n", rtti_val);
    printf("BEGIN\n");
#endif

    it = list.begin();
    while (it != list.end()) {

#       if 0
        printf("RTTI value = %d\n", (*it)->rtti());
#       endif

        if ( (*it)->rtti() != rtti_val ) {
            it = list.erase(it);
        } else {
            it++;
        }
    }

    for (it = list.begin(); it != list.end(); it++) {
#       if 0
        printf("Deleting RTTI value = %d\n", (*it)->rtti());
#       endif
        delete *it;
    }

    if (rtti_val == GConst::backgroundPixmapRTTI) {
        bgr_pm = (PixmapItem *) NULL;
    }

#   if 0
    printf("END\n");
#   endif
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::regenCanvas                                              ****/
/**** INPUTS: cx x-corrdination of center                                              ****/
/**** INPUTS: cy y-corrdination of center                                              ****/
/******************************************************************************************/
void FigureEditor::regenCanvas(int cx, int cy)
{
    int traffic_type_idx;
#if CDEBUG
    printf("\n Begin regenCanvas()\n");
#endif

    /**************************************************************************************/
    /**** Clear the Canvas                                                             ****/
    /**************************************************************************************/
#if CDEBUG
    printf("regenCanvas(): calling clear()\n");
#endif

    clear();
    canvas()->update();

#if CDEBUG
    printf("regenCanvas(): clear() done\n");
#endif
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Compute corners of Canvas                                                    ****/
    /**** move to zoomToFit, only computer for initial, or the values are zero         ****/
    /**** - Chengan 20190627                                                           ****/
    /**************************************************************************************/
    c_pmXL = cx - (canvas()->width()-1)/2;
    c_pmXH = c_pmXL + canvas()->width()-1;
    c_pmYL = cy - (canvas()->height()-1)/2;
    c_pmYH = c_pmYL + canvas()->height()-1;

#if CDEBUG
    printf( "c_pmXL set to %d\n", c_pmXL);
    printf( "c_pmXH set to %d\n", c_pmXH);
    printf( "c_pmYL set to %d\n", c_pmYL);
    printf( "c_pmYH set to %d\n", c_pmYH);
#endif
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Center the Canvas                                                            ****/
    /**** center by redraw items not by center() function                              ****/
    /**************************************************************************************/
    /* Chengan xxx  20190627
#if CDEBUG
    printf("regenCanvas(): calling center()\n");
#endif

    // Chengan test xxxx
    //center(cx,cy);
    center((canvas()->width()-1)/2, (canvas()->height()-1)/2);

#if CDEBUG
    printf("regenCanvas(): center() done\n");
#endif
    */
    /**************************************************************************************/

    VisibilityWindow *vw = visibility_window;

    /**************************************************************************************/
    /**** Draw Background Pixmap                                                       ****/
    /**************************************************************************************/
    bgr_pm = new PixmapItem(this);

    if (np->mode == CConst::noGeomMode) {
        bgr_pm->fill(QColor(128,128,128));
    } else {
        bgr_pm->fill(Qt::white);
#if CDEBUG
        bgr_pm->drawResolutionGrid(this);
#endif
    }

    if (vw->vis_map_height) {
        bgr_pm->drawMapHeight(this);
    }
    if (vw->vis_map_background) {
        bgr_pm->drawMapBackground(this);
    }
    if (vw ->vis_map_clutter) {
        bgr_pm->drawMapClutter(this, vw->vec_vis_map_clutter);
    }
    if (vw ->vis_clutter_prop_model) {
        bgr_pm->drawClutterPropModel(this, vw->vec_vis_clutter_prop_model);
    }

    if ( vw->vis_map_layer ) {
        bgr_pm->drawMapLayer(this, vw->vec_vis_map_layer);
    }
    if ( vw->vis_system_boundary ) {
        bgr_pm->drawSystemBoundary(this);
    }
    if (vw->vis_subnet) {
        bgr_pm->drawSubnet(this, vw->vec_vis_subnet);
    }
    if ( vw->vis_coverage) {
        bgr_pm->drawCoverage(this, vw->vec_vis_coverage);
    }
    if ( vw->vis_road_test_data ) {
        bgr_pm->drawRoadTestData(this, vw->num_road_test_data_set, vw->vec_vis_road_test_data, vw->vec_rtd_cell_idx, vw->vec_rtd_sector_idx, vw->vec_vis_rtd_level);
    }
    if (vw->vis_antenna) {
        bgr_pm->drawDirAntenna(this);
    }
    
    if (np->mode == CConst::noGeomMode) {
        if_show_handset_anomalies = 0;
        if_show_handset_noise = 0;
        if_show_3ho = 0;
        num_noise = 0;
        num_anomalies = 0;
        ho_show_index = 0;
        min_index = 0;
        max_index = 0;
        start_index = 0;
        end_index = 0;
        num_cs_new = 0;
    } else {
        if(if_show_handset_anomalies)
            show_handset_anomalies();
        if(if_show_handset_noise)
            show_handset_noise();       
        show_cs_new_location();
    }

    bgr_pm->show();
    /**************************************************************************************/

    /**************************************************************************************/
    /**** BEGIN: Draw Items on Canvas                                                  ****/
    /**************************************************************************************/
    if ( vw->vis_cell ) {
        for (int cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
            if (vw->vec_vis_cell[cell_idx]) {
                new GCellClass(this, cell_idx, np->cell_list[cell_idx]);
            }
        }
    }

    if ( vw->vis_cell_text ) {
        for (int cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++) {
            new CellText(this, cell_idx, vw->vec_vis_cell_text);
        }
    }

#if HAS_MONTE_CARLO
    if ( vw->vis_traffic ) {
        for (int master_idx=0; master_idx<=np->master_call_list->getSize()-1; master_idx++) {
            traffic_type_idx = ((CallClass *) (*(np->master_call_list))[master_idx])->traffic_type_idx;
            if (vw->vec_vis_traffic[traffic_type_idx]) {
                new GCallClass(this, master_idx, traffic_type_idx);
            }
        }
    }
#endif
    /**************************************************************************************/
    /**** END: Draw Items on Canvas                                                    ****/
    /**************************************************************************************/

    if (ruler) {
        ruler->draw(this, canvas());
    }

    update();

#if CDEBUG
    printf("Done regenCanvas()\n");
#endif
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::resizeCanvas                                             ****/
/******************************************************************************************/
void FigureEditor::resizeCanvas()
{    
    clear();

    canvas()->resize( (int) floor(visibleWidth() *(1+2*margin)),
                      (int) floor(visibleHeight()*(1+2*margin)));

    int center_x = (int) floor(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0);
    int center_y = (int) floor(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0);

    regenCanvas(center_x, center_y);
    regenPrintRect();
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::scrollCanvas                                             ****/
/******************************************************************************************/
void FigureEditor::scrollCanvas(int x, int y)
{
    int center_x = (int) floor(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0 - zoom*x);
    int center_y = (int) floor(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0 + zoom*y);
    regenCanvas(center_x,center_y);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::scrollUp                                                 ****/
/******************************************************************************************/
void FigureEditor::scrollUp()
{
    int center_x = c_pmXL + (canvas()->width()-1)/2;
    int center_y = c_pmYL + (canvas()->height()-1)/2;

    regenCanvas(center_x, center_y-1);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::scrollDown                                               ****/
/******************************************************************************************/
void FigureEditor::scrollDown()
{
    int center_x = c_pmXL + (canvas()->width()-1)/2;
    int center_y = c_pmYL + (canvas()->height()-1)/2;

    regenCanvas(center_x, center_y+1);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::scrollLeft                                               ****/
/******************************************************************************************/
void FigureEditor::scrollLeft()
{
    int center_x = c_pmXL + (canvas()->width()-1)/2;
    int center_y = c_pmYL + (canvas()->height()-1)/2;

    regenCanvas(center_x-1, center_y);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::scrollRight                                              ****/
/******************************************************************************************/
void FigureEditor::scrollRight()
{
    int center_x = c_pmXL + (canvas()->width()-1)/2;
    int center_y = c_pmYL + (canvas()->height()-1)/2;

    regenCanvas(center_x+1, center_y);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::xy_to_canvas                                             ****/
/******************************************************************************************/
void FigureEditor::xy_to_canvas(int& x_canvas, int& y_canvas, int x, int y)
{
    x_canvas = (int) floor(x*zoom) - c_pmXL;
    y_canvas =  -c_pmYL - (int) floor(y*zoom);
    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::xy_to_canvas                                             ****/
/******************************************************************************************/
void FigureEditor::xy_to_canvas(int& x_canvas, int& y_canvas, double x, double y)
{
    x_canvas = (int) floor(x*zoom) - c_pmXL;
    y_canvas =  -c_pmYL - (int) floor(y*zoom);
    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::canvas_to_xy                                             ****/
/******************************************************************************************/
void FigureEditor::canvas_to_xy(int& x, int& y, int x_canvas, int y_canvas)
{
    x = (int) floor( (x_canvas+c_pmXL)/zoom);
    y = (int) floor(-(y_canvas+c_pmYL)/zoom);
    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::canvas_to_xy                                             ****/
/******************************************************************************************/
void FigureEditor::canvas_to_xy(double& x, double& y, int x_canvas, int y_canvas)
{
    x =  (x_canvas+c_pmXL)/zoom;
    y = -(y_canvas+c_pmYL)/zoom;
    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::printCanvasInfo                                          ****/
/******************************************************************************************/
void FigureEditor::printCanvasInfo()
{
    printf("\n==============================================================\n");
    printf("c_pmXL = %d\n", c_pmXL);
    printf("c_pmXH = %d\n", c_pmXH);
    printf("c_pmYL = %d\n", c_pmYL);
    printf("c_pmYH = %d\n", c_pmYH);
    printf("Canvas Width  = %d\n", canvas()->width());
    printf("Canvas Height = %d\n", canvas()->height());
    printf("Visible Width = %d\n", visibleWidth());
    printf("Visible Height = %d\n", visibleHeight());
    printf("Contents X = %d\n", contentsX());
    printf("Contents Y = %d\n", contentsY());
    printf("Zoom = %15.10f\n", zoom);
    printf("system_startx = %d\n", get_np()->system_startx);
    printf("system_starty = %d\n", get_np()->system_starty);

    int gx1, gy1, gx2, gy2, px1, py1;
    canvas_to_xy(gx1, gy1, 0, 0);
    canvas_to_xy(gx2, gy2, canvas()->width(), canvas()->height());
    xy_to_canvas(px1, py1, 0, 0);
    printf("canvas (0,0) to user(grid) = (%d, %d) \n", gx1, gy1);
    printf("canvas (0,0) to user(pixel) = (%d, %d) \n", (int)(gx1*zoom), (int)(gy1*zoom));
    printf("canvas (canvas_w, canvas_h) to user(grid) = (%d, %d)\n", gx2, gy2);
    printf("canvas (canvas_w, canvas_h) to user(pixel) = (%d, %d)\n", (int)(gx2*zoom), (int)(gy2*zoom));
    printf("user(0,0) to canvas(grid) = (%d, %d) \n", (int)(px1/zoom), (int)(py1/zoom));
    printf("user(0,0) to canvas(pixel) = (%d, %d) \n", px1, py1);

    printf("Canvas Height = %d\n", canvas()->height());
    printf("Updates Enables = %s\n", (isUpdatesEnabled()==TRUE ? "TRUE" : "FALSE"));
    printf("==============================================================\n");

    /*
    NetworkClass *np = get_np();
    sprintf(np->msg, "==============================================================\n");
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "c_pmXL = %d\n", c_pmXL);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "c_pmXH = %d\n", c_pmXH);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "c_pmYL = %d\n", c_pmYL);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "c_pmYH = %d\n", c_pmYH);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Canvas Width  = %d\n", canvas()->width());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Canvas Height = %d\n", canvas()->height());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Visible Width = %d\n", visibleWidth());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Visible Height = %d\n", visibleHeight());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Contents X = %d\n", contentsX());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Contents Y = %d\n", contentsY());
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "Zoom = %15.10f\n", zoom);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "system_startx = %d\n", get_np()->system_startx);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "system_starty = %d\n", get_np()->system_starty);
    PRMSG(stdout,np->msg);

    // the (posn_x,posn_y) of grid(0,0)
    sprintf(np->msg, "posn_x = %d\n", get_np()->idx_to_x(0));
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "posn_y = %d\n", get_np()->idx_to_y(0));
    PRMSG(stdout,np->msg);

    sprintf(np->msg, "npts_x = %d\n", get_np()->npts_x);
    PRMSG(stdout,np->msg);
    sprintf(np->msg, "npts_y = %d\n", get_np()->npts_y);
    PRMSG(stdout,np->msg);

    sprintf(np->msg, "Updates Enables = %s\n", (isUpdatesEnabled()==TRUE ? "TRUE" : "FALSE"));
    PRMSG(stdout,np->msg);

    sprintf(np->msg, "==============================================================\n");
    PRMSG(stdout,np->msg);
    */
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::printClutterInfo                                         ****/
/******************************************************************************************/
void FigureEditor::printClutterInfo()
{
    NetworkClass *np = get_np();
    MapClutterClass *mc = np->map_clutter;

    printf("==============================================================\n");
    printf("offset_x          = %d\n", mc->offset_x);
    printf("offset_y          = %d\n", mc->offset_y);
    printf("npts_x            = %d\n", mc->npts_x);
    printf("npts_y            = %d\n", mc->npts_y);
    printf("map_sim_res_ratio = %d\n", mc->map_sim_res_ratio);
    printf("num_clutter_type  = %d\n", mc->num_clutter_type);
    printf("==============================================================\n");
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::measPath                                                 ****/
/******************************************************************************************/
void FigureEditor::measPath()
{
    setMouseMode(GConst::measPathMode);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::setZoom                                                  ****/
/******************************************************************************************/
void FigureEditor::setZoom(double z)
{
    double zf = z / zoom;
    int center_x = (int) floor(zf*(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0));
    int center_y = (int) floor(zf*(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0));

    zoom = z;
    regenCanvas(center_x, center_y);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::zoomByFactor                                             ****/
/******************************************************************************************/
void FigureEditor::zoomByFactor(double zf)
{
    int center_x = (int) floor(zf*(c_pmXL + contentsX() + (visibleWidth()  - 1)/2.0));
    int center_y = (int) floor(zf*(c_pmYL + contentsY() + (visibleHeight() - 1)/2.0));

    zoom *= zf;
    regenCanvas(center_x, center_y);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::zoomIn                                                   ****/
/******************************************************************************************/
void FigureEditor::zoomIn()
{
    zoomByFactor(2.0);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::zoomOut                                                  ****/
/******************************************************************************************/
void FigureEditor::zoomOut()
{
    zoomByFactor(0.5);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::setZoomMode                                              ****/
/******************************************************************************************/
void FigureEditor::setZoomMode()
{
    setMouseMode(GConst::zoomMode);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::zoomToFit                                                ****/
/******************************************************************************************/
void FigureEditor::zoomToFit()
{
    double zx = (visibleWidth() - 1.0) / (np->npts_x-1);
    double zy = (visibleHeight() - 1.0) / (np->npts_y-1);
    zoom = ((zx < zy) ? zx : zy);
    int center_x = (int)  floor(zoom*(np->npts_x-1)/2.0);
    int center_y = (int) -floor(zoom*(np->npts_y-1)/2.0);

#if CDEBUG
    printf( "c_pmXL = %d\n", c_pmXL);
    printf( "c_pmXH = %d\n", c_pmXH);
    printf( "c_pmYL = %d\n", c_pmYL);
    printf( "c_pmYH = %d\n", c_pmYH);
#endif

    // compute the cornor of canvas, or the values are (0,0,0,0)
    c_pmXL = center_x - (canvas()->width()-1)/2;
    c_pmXH = c_pmXL + canvas()->width()-1;
    c_pmYL = center_y - (canvas()->height()-1)/2;
    c_pmYH = c_pmYL + canvas()->height()-1;

    regenCanvas(center_x, center_y);
    updateContents();
}
/******************************************************************************************/
/**** FUNCTION: CellText::CellText                                                     ****/
/******************************************************************************************/
CellText::CellText(FigureEditor *editor, int a_cell_idx, char *vec_vis) : Q3CanvasText( editor->canvas() )
{
    int x_canvas, y_canvas, sector_idx, pm;
    NetworkClass *np = editor->get_np();
    QString s("");

    SectorClass *sector;

    int flag = 0;

    cell_idx = a_cell_idx;
    CellClass *cell = np->cell_list[cell_idx];

    if (vec_vis[0]) {
        s += QString("%1").arg(cell_idx );
        flag = 1;
    }

    for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
        sector = cell->sector_list[sector_idx];
        if (vec_vis[1]) {
            if (flag) { s += "\n"; }
            s += QString("%1").arg(sector->antenna_height,4,'f',1);
            flag = 1;
        }
        if (vec_vis[2]) {
            if (flag) { s += "\n"; }
            pm = sector->prop_model;
            if (pm == -1) {
                s += "UNASSIGNED";
            } else {
                s += np->prop_model_list[pm]->get_strid();
            }
            flag = 1;
        }
        if (np->technology() == CConst::PHS) {
            if (vec_vis[3]) {
                if (flag) { s += "\n"; }
                s += cell->view_name(cell_idx, CConst::CellHexCSIDRef);
                flag = 1;
            }
            if (vec_vis[4]) {
                if (flag) { s += "\n"; }
                s += cell->view_name(cell_idx, CConst::CellCSNumberRef);
                flag = 1;
            }
            if (vec_vis[5]) {
                if (flag) { s += "\n"; }
                s += cell->view_name(cell_idx, CConst::CellPagingArea);
                flag = 1;
            }
        }
    }

    setFont(*fixed_width_font);
    setText(s);
    editor->xy_to_canvas(x_canvas, y_canvas, cell->posn_x, cell->posn_y);
    move(x_canvas + GCellClass::size/2, y_canvas);

    setColor( QColor(CellClass::text_color) );
    setZ(GConst::cellTextZ);
    show();
}
/******************************************************************************************/
/**** FUNCTION: CellText::~CellText                                                    ****/
/******************************************************************************************/
CellText::~CellText()
{
}
/******************************************************************************************/
/**** FUNCTION: CellText::rtti, getCellIdx                                             ****/
/******************************************************************************************/
int CellText::rtti() const {return GConst::cellTextRTTI;}
int CellText::getCellIdx() {return cell_idx;}
/******************************************************************************************/
/**** FUNCTION: GCellClass::GCellClass                                                 ****/
/******************************************************************************************/
GCellClass::GCellClass(FigureEditor *editor, int a_cell_idx, CellClass *a_cell) : Q3CanvasRectangle( editor->canvas() )
{
    int x_canvas, y_canvas;
    cell_idx = a_cell_idx;
    cell     = a_cell;

    editor->xy_to_canvas(x_canvas, y_canvas, cell->posn_x, cell->posn_y);

    if (editor->select_cell_list->contains(cell_idx)) {
        setSize(size_selected,size_selected);
        selected = 1;
    } else {
        setSize(size,size);
        selected = 0;
    }

    move(x_canvas-width()/2, y_canvas-height()/2);
    setZ(GConst::cellZ);
    show();
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::~GCellClass                                                ****/
/******************************************************************************************/
GCellClass::~GCellClass()
{
}

/******************************************************************************************/
/**** FUNCTION: GCellClass::rtti, getCellIdx                                           ****/
/******************************************************************************************/
int GCellClass::rtti() const {return GConst::cellRTTI;};
int GCellClass::getCellIdx() {return cell_idx;};
/******************************************************************************************/
/**** FUNCTION: GCellClass::setCellPixmap                                              ****/
/******************************************************************************************/
void GCellClass::setCellPixmapList()
{
    int i;

    pixmap_list = (QPixmap **) malloc(CellClass::num_bm*sizeof(QPixmap *));
    selected_pixmap_list = (QPixmap **) malloc(CellClass::num_bm*sizeof(QPixmap *));

    /**************************************************************************************/
    pixmap_list[0]          = new QPixmap(size, size);
    selected_pixmap_list[0] = new QPixmap(size_selected, size_selected);
    /**************************************************************************************/

    /**************************************************************************************/
    pixmap_list[1] = new QPixmap(size, size);
    selected_pixmap_list[1] = new QPixmap(size_selected, size_selected);

    QBitmap bm(size, size);
    QBitmap sel_bm(size_selected, size_selected);
    bm.fill(Qt::color0);
    sel_bm.fill(Qt::color0);

    QPainter painter(&bm);
    painter.setPen( Qt::color1 );
    painter.setBrush( Qt::color1 );
    painter.drawEllipse(0, 0, size, size);

    QPainter sel_painter(&sel_bm);
    sel_painter.setPen( Qt::color1 );
    sel_painter.setBrush( Qt::color1 );
    sel_painter.drawEllipse(0, 0, size_selected, size_selected);

    pixmap_list[1]->setMask(bm);
    selected_pixmap_list[1]->setMask(sel_bm);
    /**************************************************************************************/

    /**************************************************************************************/
    for (i=2; i<=CellClass::num_bm-1; i++) {
        pixmap_list[i] = new QPixmap(size, size);
        pixmap_list[i]->setMask(*GCellClass::bm_list[i]);

        selected_pixmap_list[i] = new QPixmap(size_selected, size_selected);
        selected_pixmap_list[i]->setMask(*GCellClass::selected_bm_list[i]);
    }
    /**************************************************************************************/

#if 0
xxxxxxxxxxxx
    QBitmap sel_bm(selected_pixmap->width(), selected_pixmap->height());
    sel_bm.fill(Qt::color0);

    QPainter sel_painter(&sel_bm);
    sel_painter.setPen( Qt::color1 );
    sel_painter.setBrush( Qt::color1 );
    sel_painter.drawEllipse(0, 0, selected_pixmap->width(), selected_pixmap->height());

    selected_pixmap->setMask(sel_bm);
#endif
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::getCellPixmap                                              ****/
/******************************************************************************************/
QPixmap *GCellClass::getCellPixmap(int i)
{
    return(pixmap_list[i]);
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::deleteCellPixmapList                                       ****/
/******************************************************************************************/
void GCellClass::deleteCellPixmapList()
{
    int i;

    for (i=0; i<=CellClass::num_bm-1; i++) {
        delete pixmap_list[i];
        delete selected_pixmap_list[i];
    }
    free(pixmap_list);
    free(selected_pixmap_list);

    pixmap_list          = (QPixmap **) NULL;
    selected_pixmap_list = (QPixmap **) NULL;
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::drawShape                                                  ****/
/******************************************************************************************/
void GCellClass::drawShape(QPainter &p)
{
    p.setBackgroundMode(Qt::TransparentMode);

    if (selected) {
        int u = (size_selected - size) / 2;
        selected_pixmap_list[cell->bm_idx]->fill(Qt::black);
        p.drawPixmap((int) x()-u, (int) y()-u, *(selected_pixmap_list[cell->bm_idx]));
        pixmap_list[cell->bm_idx]->fill(QColor(cell->color));

        // porting to QT 4.0
        p.drawPixmap((int) x(), (int) y(), *(pixmap_list[cell->bm_idx]));
    } else {
        pixmap_list[cell->bm_idx]->fill(QColor(cell->color));

        // CG DBG
        //QPixmap ppm(size, size);
        //ppm.setMask(*GCellClass::bm_list[cell->bm_idx]);
        //p.drawPixmap((int) x(), (int) y(), ppm);

        //printf("bm idx %d \n",cell->bm_idx);

        p.drawPixmap((int) x(), (int) y(), *(pixmap_list[cell->bm_idx]));
    }
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::view_label                                                 ****/
/******************************************************************************************/
QString GCellClass::view_label(NetworkClass *np, int cell_name_pref)
{
    QString s;

    s = qApp->translate("VisibilityWindow", "Cell");
    if (np->technology() == CConst::PHS) {
        switch(cell_name_pref) {
            case CConst::CellIdxRef:
                s += " (" + qApp->translate("VisibilityWindow", "Cell Index") + ")";
                break;
            case CConst::CellCSNumberRef:
                s += " (GW_CSC_CS)";
                break;
            case CConst::CellHexCSIDRef:
                s += " (CSID)";
                break;
            default:
                CORE_DUMP; break;
        }
    }

    return(s);
}
/******************************************************************************************/

#if (0 && HAS_MONTE_CARLO)
xxxxxxxx Moved to gcall.cpp DELETE
/******************************************************************************************/
/**** FUNCTION: GCallClass::GCallClass                                                 ****/
/******************************************************************************************/
GCallClass::GCallClass(FigureEditor *editor, int p_master_idx, int p_traffic_type_idx) : Q3CanvasRectangle( editor->canvas() )
{
    int cell_idx, call_x, call_y, cell_x, cell_y;
    CellClass   *cell;
    CallClass   *call;

    master_idx = p_master_idx;
    traffic_type_idx = p_traffic_type_idx;
    call = (CallClass *) (*(np->master_call_list))[master_idx];
    cell_idx = call->cell_idx;
    cell = np->cell_list[cell_idx];

    editor->xy_to_canvas(call_x, call_y, call->posn_x, call->posn_y);
    editor->xy_to_canvas(cell_x, cell_y, cell->posn_x, cell->posn_y);

    setSize(GConst::callSize,GConst::callSize);
    move(call_x-width()/2, call_y-height()/2);
    setZ(9);
    show();

    connection = new CallConnection(editor->canvas(), QPoint(call_x, call_y), QPoint(cell_x, cell_y));
    connection->setPen( QPen(QColor(100,100,100), 1) );
    connection->setZ(9);
    connection->show();
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::~GCallClassCall                                            ****/
/******************************************************************************************/
GCallClass::~GCallClass()
{
    delete connection;
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::rtti                                                       ****/
/******************************************************************************************/
int GCallClass::rtti() const { return GConst::trafficRTTI; }
/******************************************************************************************/
/**** FUNCTION: GCallClass::setNetworkStruct                                           ****/
/******************************************************************************************/
void GCallClass::setNetworkStruct( NetworkClass *p_np) { np = p_np; };
/******************************************************************************************/
/**** FUNCTION: GCallClass::setPixmap                                                  ****/
/******************************************************************************************/
void GCallClass::setPixmap()
{
    int tti;

    pixmap = (QPixmap **) malloc(np->num_traffic_type*sizeof(QPixmap *));
    for (tti=0; tti<=np->num_traffic_type-1; tti++) {
        pixmap[tti] = new QPixmap(GConst::callSize,GConst::callSize);
        pixmap[tti]->fill(QColor(np->traffic_type_list[tti]->get_color()));

        QPainter painter(pixmap[tti]);
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(0, 0, GConst::callSize, GConst::callSize);
    }
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::drawShape                                                  ****/
/******************************************************************************************/
void GCallClass::drawShape(QPainter &p)
{
    p.drawPixmap((int) x(), (int) y(), *(pixmap[traffic_type_idx]));
}
/******************************************************************************************/
/**** FUNCTION: GCallClass::deletePixmap                                               ****/
/******************************************************************************************/
void GCallClass::deletePixmap()
{
    int tti;

    if (pixmap) {
        for (tti=0; tti<=np->num_traffic_type-1; tti++) {
            delete pixmap[tti];
        }
        free(pixmap);
    }

    pixmap = (QPixmap **) NULL;
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: PixmapItem::drawShape                                                  ****/
/******************************************************************************************/
void PixmapItem::drawShape(QPainter &p)
{
    p.drawPixmap ((int) x(), (int) y(), pixmap);
}
/******************************************************************************************/
/**** FUNCTION: CallConnection::CallConnection                                         ****/
/******************************************************************************************/
CallConnection::CallConnection(Q3Canvas *canvas, QPoint pa, QPoint pb) : Q3CanvasLine( canvas )
{
    setPoints(pa.x(), pa.y(), pb.x(), pb.y());
}
/******************************************************************************************/
/**** FUNCTION: CallConnection::~CallConnection                                        ****/
/******************************************************************************************/
CallConnection::~CallConnection()
{
}
/******************************************************************************************/
/**** FUNCTION: CallConnection::rtti                                                   ****/
/******************************************************************************************/
int CallConnection::rtti() const {return GConst::callConnectionRTTI;}
/******************************************************************************************/
/**** FUNCTION: RulerPtClass::RulerPtClass                                             ****/
/******************************************************************************************/
RulerPtClass::RulerPtClass(Q3Canvas *canvas, int x, int y) : Q3CanvasRectangle( x-3, y-3, 7, 7, canvas )
{
}
/******************************************************************************************/
/**** FUNCTION: RulerPtClass::~RulerPtClass                                            ****/
/******************************************************************************************/
RulerPtClass::~RulerPtClass()
{
}
/******************************************************************************************/
/**** FUNCTION: RulerPtClass::rtti                                                     ****/
/******************************************************************************************/
int RulerPtClass::rtti() const {return GConst::rulerPtClassRTTI;}
/******************************************************************************************/
/**** FUNCTION: RulerLineClass::RulerLineClass                                         ****/
/******************************************************************************************/
RulerLineClass::RulerLineClass(Q3Canvas *canvas, int xa, int ya, int xb, int yb) : Q3CanvasLine( canvas )
{
    setPoints(xa, ya, xb, yb);
}
/******************************************************************************************/
/**** FUNCTION: RulerLineClass::~RulerLineClass                                        ****/
/******************************************************************************************/
RulerLineClass::~RulerLineClass()
{
}
/******************************************************************************************/
/**** FUNCTION: RulerLineClass::rtti                                                   ****/
/******************************************************************************************/
int RulerLineClass::rtti() const {return GConst::rulerLineClassRTTI;}
/******************************************************************************************/
/**** FUNCTION: RulerClass::RulerClass                                                 ****/
/******************************************************************************************/
RulerClass::RulerClass(int p_xa, int p_ya, int p_xb, int p_yb)
{
    xa = p_xa;
    ya = p_ya;
    xb = p_xb;
    yb = p_yb;

    pta  = (RulerPtClass   *) NULL;
    ptb  = (RulerPtClass   *) NULL;
    line = (RulerLineClass *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: RulerClass::~RulerClass                                                ****/
/******************************************************************************************/
RulerClass::~RulerClass()
{
    clear();
}
/******************************************************************************************/
/**** FUNCTION: RulerClass::clear                                                      ****/
/******************************************************************************************/
void RulerClass::clear()
{
    delete pta;
    delete ptb;
    delete line;

    pta  = (RulerPtClass   *) NULL;
    ptb  = (RulerPtClass   *) NULL;
    line = (RulerLineClass *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: RulerClass::draw                                                       ****/
/******************************************************************************************/
void RulerClass::draw(FigureEditor *editor, Q3Canvas *canvas)
{
    int p_xa, p_ya, p_xb, p_yb;

    editor->xy_to_canvas(p_xa, p_ya, xa, ya);
    editor->xy_to_canvas(p_xb, p_yb, xb, yb);

    line = new RulerLineClass(canvas, p_xa, p_ya, p_xb, p_yb);
    pta  = new RulerPtClass(canvas, p_xa, p_ya);
    ptb  = new RulerPtClass(canvas, p_xb, p_yb);

    line->setZ(GConst::selectRegionZ);
    pta->setZ(GConst::selectRegionZ);
    ptb->setZ(GConst::selectRegionZ);

    line->show();
    pta->show();
    ptb->show();
}
/******************************************************************************************/
/**** FUNCTION: RulerClass::setPt                                                      ****/
/******************************************************************************************/
void RulerClass::setPt(int idx, int x, int y)
{
    if (idx == 0) {
        xa = x;
        ya = y;
    } else {
        xb = x;
        yb = y;
    }
}
/******************************************************************************************/
inline int cross(const int xx1, const int yy1, const int xx2, const int yy2,
                 const int xx3, const int yy3, const int xx4, const int yy4)
{
    int ccc;
    LONGLONG_TYPE sss1, sss2, sss3, sss4;

    sss1 = (LONGLONG_TYPE) (yy3-yy1)*(xx2-xx1) - (LONGLONG_TYPE) (xx3-xx1)*(yy2-yy1);
    sss2 = (LONGLONG_TYPE) (yy4-yy1)*(xx2-xx1) - (LONGLONG_TYPE) (xx4-xx1)*(yy2-yy1);
    sss3 = (LONGLONG_TYPE) (yy1-yy3)*(xx4-xx3) - (LONGLONG_TYPE) (xx1-xx3)*(yy4-yy3);
    sss4 = (LONGLONG_TYPE) (yy2-yy3)*(xx4-xx3) - (LONGLONG_TYPE) (xx2-xx3)*(yy4-yy3);
    ccc = (  (    ( ( (sss1<=0)&&(sss2>=0) ) || ( (sss1>=0)&&(sss2<=0) ) )
               && ( ( (sss3<=0)&&(sss4>=0) ) || ( (sss3>=0)&&(sss4<=0) ) ) ) ? 1 : 0 );

    // printf("P1 = (%d, %d) P2 = (%d, %d) P3 = (%d, %d) P4 = (%d, %d) S1=%lld S2=%lld S3=%lld S4=%lld CROSS = %d\n",
    //         xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4, sss1, sss2, sss3, sss4, ccc);

    return(ccc);
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::drawLinePainter                                          ****/
/**** Draws multi-segment line on painter, appropriately clipping polygon at edges     ****/
/**** of canvas.                                                                       ****/
/******************************************************************************************/
void FigureEditor::drawLinePainter(LineClass *line, QPainter &painter,
                                      const int min_x, const int max_x, const int min_y, const int max_y)
{
    int found, num_found, use;
    int prev_inside, curr_inside, prev_i, curr_i;
    int prev_canvas_x, prev_canvas_y;
    int curr_canvas_x, curr_canvas_y;
    int x_out, y_out;
    int new_x, new_y;
    int cross_x[4], cross_y[4];
    int *x, *y;
    double u;
    double cross_u[4];

#define INSIDE(xxx) ( ( (x[xxx] >= min_x) && (x[xxx] <= max_x) && (y[xxx] >= min_y) && (y[xxx] <= max_y) ) ? 1 : 0 )

    x = line->pt_x;
    y = line->pt_y;

    prev_inside = INSIDE(0);
    if (prev_inside) {
        xy_to_canvas(prev_canvas_x, prev_canvas_y, x[0], y[0]);
    }
    for (curr_i=1; curr_i<=line->num_pt-1; curr_i++) {
        prev_i = curr_i-1;
        curr_inside = INSIDE(curr_i);
        if (curr_inside) {
            xy_to_canvas(curr_canvas_x, curr_canvas_y, x[curr_i], y[curr_i]);
        }

        if ( (prev_inside && (!curr_inside)) || ((!prev_inside) && curr_inside) ) {
            use = 1;
            if (prev_inside) {
                x_out = x[curr_i];
                y_out = y[curr_i];
            } else {
                x_out = x[prev_i];
                y_out = y[prev_i];
            }
            found = 0;
            if (x_out > max_x) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], max_x, min_y, max_x, max_y);
                if (found) {
                    u = (double) (max_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(new_x, new_y, (double) max_x, y[prev_i] + u*(y[curr_i]-y[prev_i]) );
                }
            } else if (x_out < min_x) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, min_x, max_y);
                if (found) {
                    u = (double) (min_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(new_x, new_y, (double) min_x, y[prev_i] + u*(y[curr_i]-y[prev_i]) );
                }
            }
            if (!found) {
            if (y_out > max_y) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, max_y, max_x, max_y);
                if (found) {
                    u = (double) (max_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(new_x, new_y, x[prev_i] + u*(x[curr_i]-x[prev_i]), (double) max_y);
                }
            } else if (y_out < min_y) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, max_x, min_y);
                if (found) {
                    u = (double) (min_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(new_x, new_y, x[prev_i] + u*(x[curr_i]-x[prev_i]), (double) min_y);
                }
            }
            }
            if (prev_inside) {
                curr_canvas_x = new_x;
                curr_canvas_y = new_y;
            } else {
                prev_canvas_x = new_x;
                prev_canvas_y = new_y;
            }
        } else if (!curr_inside) {
            num_found = 0;
            if (x[prev_i] != x[curr_i]) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], max_x, min_y, max_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (max_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], (double) max_x, y[prev_i] + cross_u[num_found]*(y[curr_i]-y[prev_i]) );
                    if ( (cross_y[num_found] != 0) && (cross_y[num_found] != canvas()->height()-1) ) {
                        num_found++;
                    }
                }
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, min_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (min_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], (double) min_x, y[prev_i] + cross_u[num_found]*(y[curr_i]-y[prev_i]) );
                    if ( (cross_y[num_found] != 0) && (cross_y[num_found] != canvas()->height()-1) ) {
                        num_found++;
                    }
                }
            }
            if (y[prev_i] != y[curr_i]) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, max_y, max_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (max_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], x[prev_i] + cross_u[num_found]*(x[curr_i]-x[prev_i]), (double) max_y);
                    if ( (cross_x[num_found] != 0) && (cross_x[num_found] != canvas()->width()-1) ) {
                        num_found++;
                    }
                }
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, max_x, min_y);
                if (found) {
                    cross_u[num_found] = (double) (min_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], x[prev_i] + cross_u[num_found]*(x[curr_i]-x[prev_i]), (double) min_y);
                    if ( (cross_x[num_found] != 0) && (cross_x[num_found] != canvas()->width()-1) ) {
                        num_found++;
                    }
                }
            }
            if (num_found == 2) {
                use = 1;
                prev_canvas_x = cross_x[0];
                prev_canvas_y = cross_y[0];
                curr_canvas_x = cross_x[1];
                curr_canvas_y = cross_y[1];
            } else {
                use = 0;
#if CDEBUG
                if (num_found) {
                    printf("NUM_FOUND = %d\n", num_found);
                    printf("MIN_X = %d MAX_X = %d MIN_Y = %d MAX_Y = %d\n", min_x, max_x, min_y, max_y);
                    printf("N = %d\n", line->num_pt);
                    for (int i=0; i<=line->num_pt-1; i++) {
                        printf("x[%d] = %d; y[%d] = %d;\n", i, x[i], i, y[i]);
                    }
                    fflush(stdout);
                    // CORE_DUMP;
                }
#endif
            }
        } else {
            use = 1;
        }

        if (use) {
            QPainterPath path;
            path.moveTo(prev_canvas_x, prev_canvas_y);
            path.lineTo(curr_canvas_x, curr_canvas_y);
            painter.drawPath(path);
        }
        if (curr_inside) {
            prev_canvas_x = curr_canvas_x;
            prev_canvas_y = curr_canvas_y;
        }
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::drawPolygonPainter                                       ****/
/**** Draws multi-segment polygon on painter, appropriately clipping polygon at edges  ****/
/**** of canvas.                                                                       ****/
/******************************************************************************************/
void FigureEditor::drawPolygonPainter(PolygonClass *p, QPainter &painter,
                                      const int min_x, const int max_x, const int min_y, const int max_y)
{
    int i, segment_idx, last_i, pt_idx, pt_idx_m, stop_i;
    QVector<QPoint> pa;
    QVector<QPoint> seg;
    Q3MemArray<int> p_seg_start(0);

    // porting to QT4
    QPainterPath path;

    i = 0;
    for (segment_idx=0; segment_idx<=p->num_segment-1; segment_idx++) {
        comp_effective_polygon_segment(
            seg, p->num_bdy_pt[segment_idx], p->bdy_pt_x[segment_idx], p->bdy_pt_y[segment_idx],
            min_x, max_x, min_y, max_y);
        if (seg.size()) {
            p_seg_start.resize(p_seg_start.size()+1);
            p_seg_start[(int) p_seg_start.size()-1] = i;
            pa.resize(pa.size()+seg.size()+1);
            for (pt_idx=0; pt_idx<= (int) seg.size(); pt_idx++) {
                pt_idx_m = ((pt_idx == (int) seg.size()) ? 0 : pt_idx);
                pa[i] = QPoint(seg[pt_idx_m].x(), seg[pt_idx_m].y());
                i++;
            }
        }
    }
    last_i = pa.size()-1;
    if (p_seg_start.size() >= 3) {
        for (segment_idx = (int) p_seg_start.size()-2; segment_idx>=1; segment_idx--) {
            pa.resize(pa.size()+1);
            pa[i] = pa[p_seg_start[segment_idx]];
            i++;
        }
    }
    if (pa.size()) {
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(pa);

        // CG no pen draw on boundray
        /*
        painter.setPen(Qt::SolidLine);
        for (segment_idx=0; segment_idx<= (int) p_seg_start.size()-1; segment_idx++) {
            i = p_seg_start[segment_idx];
            path.moveTo(pa[i]);
            stop_i = ( (segment_idx <= (int) p_seg_start.size()-2) ? p_seg_start[segment_idx+1]-1 : last_i );
            while(i < stop_i) {
                i++;
                path.lineTo(pa[i]);
            }
        }
        painter.drawPath(path);
        */
    }

    return;
}
/******************************************************************************************/

#define PT_GENERIC 0
#define PT_ENTER   1
#define PT_EXIT    2
#define PT_RETURN  3
#define PT_DELETE  4

void fix_metric_order(int num_x_pts, int *xpt_metric, int *xpt_idx, QVector<QPoint> &pa, Q3MemArray<int> &pa_type, int orient);

/******************************************************************************************/
/**** FUNCTION: FigureEditor::comp_effective_polygon_segment                           ****/
/**** Computes effective polygon segment given that polygon segment is clipped at      ****/
/**** edges of canvas.                                                                 ****/
/******************************************************************************************/
void FigureEditor::comp_effective_polygon_segment(QVector<QPoint> &pa, const int n, const int *x, const int *y,
                                                    const int min_x, const int max_x,
                                                    const int min_y, const int max_y)
{

#define INSIDE(xxx) ( ( (x[xxx] >= min_x) && (x[xxx] <= max_x) && (y[xxx] >= min_y) && (y[xxx] <= max_y) ) ? 1 : 0 )

    int prev_inside, curr_inside, cont, i;
    int numpts, num_x_pts, curr_i, found, num_found, new_x, new_y;
    int m, q, pt_idx, next_idx;
    int exit_seg, enter_seg, use, num_ins, ins_idx;
    int x_out, y_out;
    int cross_x[4], cross_y[4], met[4];
    int b_idx;
    int canvas_x, canvas_y;
    int metric = 0;
    int c_idx = -1;
    int a_idx = -1;
    int a_start_idx = -1;
    double a, u;
    double cross_u[4];

    int *cpt_x = IVECTOR(4);
    int *cpt_y = IVECTOR(4);
    int *d_pt  = IVECTOR(4);

    int c_left, c_right, c_bottom, c_top;
    xy_to_canvas(c_left,  c_bottom, min_x, min_y);
    xy_to_canvas(c_right, c_top,    max_x, max_y);

    cpt_x[0] = c_right; cpt_y[0] = c_top;
    cpt_x[1] = c_left;  cpt_y[1] = c_top;
    cpt_x[2] = c_left;  cpt_y[2] = c_bottom;
    cpt_x[3] = c_right; cpt_y[3] = c_bottom;

    int dist_x = c_right  - c_left;
    int dist_y = c_bottom - c_top;
    d_pt[0]  = 2*dist_x + dist_y + c_bottom;
    d_pt[1]  = c_right;
    d_pt[2]  = dist_x - c_top;
    d_pt[3]  = dist_x + dist_y - c_left;

    int ctr_x = canvas()->width()/2;
    int ctr_y = canvas()->height()/2;

    int orient = ( (PolygonClass::comp_bdy_area(n, x, y) > 0.0) ? 1 : 0 );
    int eff_orient;
    
    numpts = 0;
    num_x_pts = 0;
    pa.resize(numpts);
    Q3MemArray<int> pa_type(numpts);
    int *xpt_idx    = (int *) NULL;
    int *xpt_metric = (int *) NULL;
    int *subseg = (int *) NULL;
    int num_subseg;

    int prev_i = n-1;
    prev_inside = INSIDE(prev_i);
    for (curr_i=0; curr_i<=n-1; curr_i++) {
        curr_inside = INSIDE(curr_i);
        if ( (prev_inside && (!curr_inside)) || ((!prev_inside) && curr_inside) ) {
            if (prev_inside) {
                x_out = x[curr_i];
                y_out = y[curr_i];
            } else {
                x_out = x[prev_i];
                y_out = y[prev_i];
            }
            found = 0;
            if (x_out > max_x) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], max_x, min_y, max_x, max_y);
                if (found) {
                    u = (double) (max_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(new_x, new_y, (double) max_x, y[prev_i] + u*(y[curr_i]-y[prev_i]) );
                    metric = (d_pt[0] - new_y)*(orient?1:-1);
                }
            } else if (x_out < min_x) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, min_x, max_y);
                if (found) {
                    u = (double) (min_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(new_x, new_y, (double) min_x, y[prev_i] + u*(y[curr_i]-y[prev_i]) );
                    metric = (d_pt[2] + new_y)*(orient?1:-1);
                }
            }
            if (!found) { 
            if (y_out > max_y) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, max_y, max_x, max_y);
                if (found) {
                    u = (double) (max_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(new_x, new_y, x[prev_i] + u*(x[curr_i]-x[prev_i]), (double) max_y);
                    metric = (d_pt[1] - new_x)*(orient?1:-1);
                }
            } else if (y_out < min_y) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, max_x, min_y);
                if (found) {
                    u = (double) (min_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(new_x, new_y, x[prev_i] + u*(x[curr_i]-x[prev_i]), (double) min_y);
                    metric = (d_pt[3] + new_x)*(orient?1:-1);
                }
            }
            }
            numpts++;
            num_x_pts++;

            pa.resize(numpts);
            pa_type.resize(numpts, Q3GArray::SpeedOptim);
            xpt_idx    = (int *) realloc((void *) xpt_idx,    num_x_pts*sizeof(int));
            xpt_metric = (int *) realloc((void *) xpt_metric, num_x_pts*sizeof(int));

            pa[numpts-1] = QPoint(new_x, new_y);
            pa_type[numpts-1] = (curr_inside ? PT_ENTER : PT_EXIT);
            xpt_idx[num_x_pts-1] = numpts-1;
            xpt_metric[num_x_pts-1] = metric;
        } else if (!curr_inside) {
            num_found = 0;
            if (x[prev_i] != x[curr_i]) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], max_x, min_y, max_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (max_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], (double) max_x, y[prev_i] + cross_u[num_found]*(y[curr_i]-y[prev_i]) );
                    if ( (cross_y[num_found] != 0) && (cross_y[num_found] != canvas()->height()-1) ) {
                        met[num_found] = (d_pt[0] - cross_y[num_found])*(orient?1:-1);
                        num_found++;
                    }
                }
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, min_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (min_x - x[prev_i]) / (x[curr_i] - x[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], (double) min_x, y[prev_i] + cross_u[num_found]*(y[curr_i]-y[prev_i]) );
                    if ( (cross_y[num_found] != 0) && (cross_y[num_found] != canvas()->height()-1) ) {
                        met[num_found] = (d_pt[2] + cross_y[num_found])*(orient?1:-1);
                        num_found++;
                    }
                }
            }
            if (y[prev_i] != y[curr_i]) {
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, max_y, max_x, max_y);
                if (found) {
                    cross_u[num_found] = (double) (max_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], x[prev_i] + cross_u[num_found]*(x[curr_i]-x[prev_i]), (double) max_y);
                    if ( (cross_x[num_found] != 0) && (cross_x[num_found] != canvas()->width()-1) ) {
                        met[num_found] = (d_pt[1] - cross_x[num_found])*(orient?1:-1);
                        num_found++;
                    }
                }
                found = cross(x[prev_i], y[prev_i], x[curr_i], y[curr_i], min_x, min_y, max_x, min_y);
                if (found) {
                    cross_u[num_found] = (double) (min_y - y[prev_i]) / (y[curr_i] - y[prev_i]);
                    xy_to_canvas(cross_x[num_found], cross_y[num_found], x[prev_i] + cross_u[num_found]*(x[curr_i]-x[prev_i]), (double) min_y);
                    if ( (cross_x[num_found] != 0) && (cross_x[num_found] != canvas()->width()-1) ) {
                        met[num_found] = (d_pt[3] + cross_x[num_found])*(orient?1:-1);
                        num_found++;
                    }
                }
            }
            if (num_found == 2) {
                numpts += 2;
                num_x_pts += 2;
                pa.resize(numpts);
                pa_type.resize(numpts, Q3GArray::SpeedOptim);
                xpt_idx    = (int *) realloc((void *) xpt_idx,    num_x_pts*sizeof(int));
                xpt_metric = (int *) realloc((void *) xpt_metric, num_x_pts*sizeof(int));

                m = ( (cross_u[0] < cross_u[1]) ? 0 : 1 );
               
                // xxxxx pa[numpts-2] = QPoint((int) floor(cross_x[m]), (int) floor(cross_y[m]));
                pa[numpts-2] = QPoint(cross_x[m], cross_y[m]);
                pa_type[numpts-2] = PT_ENTER;
                xpt_idx[num_x_pts-2] = numpts-2;
                xpt_metric[num_x_pts-2] = met[m];

                // xxxxxx pa[numpts-1] = QPoint((int) floor(cross_x[1-m]), (int) floor(cross_y[1-m]));
                pa[numpts-1] = QPoint(cross_x[1-m], cross_y[1-m]);
                pa_type[numpts-1] = PT_EXIT;
                xpt_idx[num_x_pts-1] = numpts-1;
                xpt_metric[num_x_pts-1] = met[1-m];
            } else if (num_found) {
#if CDEBUG
                printf("NUM_FOUND = %d\n", num_found);
                printf("MIN_X = %d MAX_X = %d MIN_Y = %d MAX_Y = %d\n", min_x, max_x, min_y, max_y);
                printf("N = %d\n", n);
                for (int i=0; i<=n-1; i++) {
                    printf("x[%d] = %d; y[%d] = %d;\n", i, x[i], i, y[i]);
                }
                fflush(stdout);
                // CORE_DUMP;
#endif
            }
        }

        if (curr_inside) {
            xy_to_canvas(canvas_x, canvas_y, x[curr_i], y[curr_i]);
            if (!( (numpts) && (pa[numpts-1].x() == canvas_x) && (pa[numpts-1].y() == canvas_y) )) {
                numpts++;
                pa.resize(numpts);
                pa_type.resize(numpts, Q3GArray::SpeedOptim);

                pa[numpts-1] = QPoint(canvas_x, canvas_y);
                pa_type[numpts-1] = PT_GENERIC;
            }
        }

        prev_i = curr_i;
        prev_inside = curr_inside;
    }

    int i0, i1;
    int num_delete = 0;
    if (num_x_pts) {
        i = 0;
        while (i<= num_x_pts-1) {
            if (pa_type[xpt_idx[i]] == PT_ENTER) {
                i0 = xpt_idx[i];
                i1 = i0;
                while (pa_type[i1] != PT_EXIT) {
                    i1 = (i1==((int) pa.size())-1 ? 0 : i1+1);
                }
                if (pa[i0] == pa[i1]) {
                    int next_k;
                    int k = (i0==((int) pa.size())-1 ? 0 : i0+1);
                    int n = i1-i0;
                    if (n < 0) { n += pa.size(); }
                    n -= 2;

                    int area = 0;
                    for (int count=0; count<=n-1; count++) {
                        next_k = (k==((int) pa.size())-1 ? 0 : k+1);

                        int x1 = pa[k  ].x() - pa[i0].x();
                        int y1 = pa[k  ].y() - pa[i0].y();
                        int x2 = pa[next_k].x() - pa[i0].x();
                        int y2 = pa[next_k].y() - pa[i0].y();

                        area += (x1*y2 - x2*y1);

                        k = next_k;
                    }
                    if (area == 0) {
                        int min, max;
                        int j = 0;
                        while(xpt_idx[j] != i1) { j++; }
                        if (i < j) {
                            min=i;
                            max=j;
                            i--;
                        } else {
                            min=j;
                            max=i;
                            i-=2;
                        }

                        for (k=min+1; k<=max-1; k++) {
                            xpt_metric[k-1] = xpt_metric[k];
                            xpt_idx[k-1]    = xpt_idx[k];
                        }
                        for (k=max+1; k<=num_x_pts-1; k++) {
                            xpt_metric[k-2] = xpt_metric[k];
                            xpt_idx[k-2]    = xpt_idx[k];
                        }
                        num_x_pts -= 2;
                        k = i0;
                        pa_type[k] = PT_DELETE;
                        num_delete++;
                        do {
                            k = (k== ((int)pa.size())-1 ? 0 : k+1);
                            pa_type[k] = PT_DELETE;
                            num_delete++;
                        } while(k != i1);
                        xpt_idx    = (int *) realloc((void *) xpt_idx,    num_x_pts*sizeof(int));
                        xpt_metric = (int *) realloc((void *) xpt_metric, num_x_pts*sizeof(int));
                    }
                }
            }
            i++;
        }
    }

    if (num_x_pts) {
#if DEBUG_DRAW_POLYGON
        for (i=0; i<= (int) pa.size()-1; i++) {
            printf("pa[%d] = (%d, %d) TYPE = %s\n", i, pa[i].x(), pa[i].y(),
                ( (pa_type[i] == PT_GENERIC) ? "GENERIC" :
                  (pa_type[i] == PT_ENTER)   ? "ENTER"   :
                  (pa_type[i] == PT_EXIT)    ? "EXIT"    : "DELETE" ));
        }
#endif
        sort2z(num_x_pts, xpt_metric, xpt_idx);
        fix_metric_order(num_x_pts, xpt_metric, xpt_idx, pa, pa_type, orient);

#if DEBUG_DRAW_POLYGON
        for (i=0; i<= num_x_pts-1; i++) {
            printf("xpt_metric[%d] = %d  xpt_idx[%d] =  %d\n", i, xpt_metric[i], i, xpt_idx[i]);
        }
#endif

        QVector<QPoint> pb(pa.size()-num_delete);
        Q3MemArray<int> pb_type(pa.size()-num_delete);
        b_idx = 0;
        num_subseg = 0;
        do {
            cont = 0;
            for(i=0; (i<=num_x_pts-1)&&(!cont); i++) {
                if (pa_type[xpt_idx[i]] == PT_ENTER ) {
                    cont = 1;
                    a_start_idx = xpt_idx[i];
                    a_idx = a_start_idx;
                    num_subseg++;
                    subseg = (int *) realloc((void *) subseg, num_subseg*sizeof(int));
                    subseg[num_subseg-1] = a_start_idx;
                }
            }

            if (cont) {
                do {
                    pa_type[a_idx] += 16;
                    while(pa_type[a_idx] != PT_EXIT) {
                        pb[b_idx] = pa[a_idx];
                        pb_type[b_idx] = (pa_type[a_idx] & 0x0F);
                        b_idx++;
                        a_idx = ( (a_idx == (int) pa.size()-1) ? 0 : a_idx+1 );
                    }
                    pb[b_idx] = pa[a_idx];
                    pb_type[b_idx] = pa_type[a_idx];
                    b_idx++;
                    pa_type[a_idx] += 16;
                    i = 0;
                    while(xpt_idx[i] != a_idx) { i++; }
                    a_idx = xpt_idx[( (i == num_x_pts-1) ? 0 : i+1 )];
                } while (a_idx != a_start_idx);
                pb.resize(pb.size()+1);
                pb_type.resize(pb_type.size()+1, Q3GArray::SpeedOptim);
                pb[b_idx] = pa[a_start_idx];
                pb_type[b_idx] = pa_type[a_start_idx];
                b_idx++;
            }
        } while(cont);

        if (num_subseg >= 2) {
            pb_type[b_idx-1] = PT_RETURN;
        }

        if (num_subseg >= 3) {
            pb.resize(          pb.size()+num_subseg-2);
            pb_type.resize(pb_type.size()+num_subseg-2, Q3GArray::SpeedOptim);
            for (i = num_subseg-2; i>=1; i--) {
                pb[b_idx] = pa[subseg[i]];
                pb_type[b_idx] = PT_RETURN;
                b_idx++;
            }
        }
    
        pa.resize(pb.size());
        pa_type.resize(pb.size(), Q3GArray::SpeedOptim);
        for (i=0; i<=(int) pb.size()-1; i++) {
            pa[i] = pb[i];
            pa_type[i] = pb_type[i] & 0x0F;
        }
        numpts = pb.size();

        pt_idx = 0;
        while(pt_idx <= numpts-1) {
            next_idx = ( (pt_idx == numpts-1) ? 0 : pt_idx+1 );
            if (   (pa_type[pt_idx] == PT_EXIT) 
                || (    ( (pa_type[pt_idx]   == PT_ENTER) || (pa_type[pt_idx]   == PT_RETURN) )
                     && ( (pa_type[next_idx] == PT_ENTER) || (pa_type[next_idx] == PT_RETURN) ) ) ) {

                if      (pa[pt_idx].x()   == c_right ) { exit_seg = 0; }
                else if (pa[pt_idx].y()   == c_top   ) { exit_seg = 1; }
                else if (pa[pt_idx].x()   == c_left  ) { exit_seg = 2; }
                else                                { exit_seg = 3; }

                if      (pa[next_idx].x() == c_right ) { enter_seg = 0; }
                else if (pa[next_idx].y() == c_top   ) { enter_seg = 1; }
                else if (pa[next_idx].x() == c_left  ) { enter_seg = 2; }
                else                                { enter_seg = 3; }

                eff_orient = ( (pa_type[pt_idx] == PT_RETURN) ? 1-orient : orient );

                use = 1;
                if (enter_seg == exit_seg) {
                    a = (pa[pt_idx].x()-ctr_x)*(pa[next_idx].y()-ctr_y)-(pa[next_idx].x()-ctr_x)*(pa[pt_idx].y()-ctr_y);
                    if ( ((eff_orient) && (a <= 0)) || ((!eff_orient) && (a >= 0)) ) {
                        use = 0;
                    }
                }
                if (use) {
                    num_ins = ( eff_orient ? enter_seg - exit_seg : exit_seg - enter_seg);
                    if (num_ins <= 0) { num_ins += 4; }
                    numpts+=num_ins;
                    pa.resize(numpts);
                    pa_type.resize(numpts, Q3GArray::SpeedOptim);
                    for (q=numpts-1; q>=pt_idx+num_ins+1; q--) {
                        pa[q] = pa[q-num_ins];
                        pa_type[q] = pa_type[q-num_ins];
                    }
                    for (ins_idx=0; ins_idx<=num_ins-1; ins_idx++) {
                        if (ins_idx == 0) {
                            c_idx = (eff_orient ? exit_seg : exit_seg-1);
                            if (c_idx<0) { c_idx += 4; }
                        } else {
                            if (eff_orient) {
                                c_idx = ( (c_idx == 3) ? 0 : c_idx+1 );
                            } else {
                                c_idx = ( (c_idx == 0) ? 3 : c_idx-1 );
                            }
                        }

                        pa[pt_idx + 1 + ins_idx] = QPoint(cpt_x[c_idx], cpt_y[c_idx]);
                        pa_type[pt_idx + 1 + ins_idx] = PT_GENERIC;
                    }
                    pt_idx += num_ins;
                } else {
                    pt_idx++;
                }
            } else {
                pt_idx++;
            }
        }
#if DEBUG_DRAW_POLYGON
        printf("============================================================================\n");
        printf("FINAL POLYGON: orient = %d\n", orient);
        for (i=0; i<= (int) pa.size()-1; i++) {
            printf("pa[%d] = (%d, %d) TYPE = %s\n", i, pa[i].x(), pa[i].y(),
                ( (pa_type[i] == PT_GENERIC) ? "GENERIC" :
                  (pa_type[i] == PT_ENTER)   ? "ENTER"   :
                  (pa_type[i] == PT_RETURN)  ? "RETURN"  :
                  "EXIT" ));
        }
        printf("============================================================================\n");
#endif
    } else if (num_delete) {
        i0 = 0;
        while(pa_type[i0] != PT_DELETE) { i0++; }
        for (i1=i0+1; i1<=numpts-1; i1++) {
            if (pa_type[i1] != PT_DELETE) {
                pa[i0] = pa[i1];
                i0++;
            }
        }
        numpts -= num_delete;
        pa.resize(numpts);
    }

    if ( (numpts == 0) && (PolygonClass::in_bdy_area((min_x+max_x)/2, (min_y+max_y)/2, n, x, y)) ) {
        pa.resize(4);
        for (ins_idx=0; ins_idx<=3; ins_idx++) {
            if (ins_idx == 0) {
                c_idx = 0;
            } else {
                if (orient) {
                    c_idx = ( (c_idx == 3) ? 0 : c_idx+1 );
                } else {
                    c_idx = ( (c_idx == 0) ? 3 : c_idx-1 );
                }
            }

            pa[ins_idx] = QPoint(cpt_x[c_idx], cpt_y[c_idx]);
        }
    }

    free(cpt_x);
    free(cpt_y);
    free(d_pt);

    if (xpt_idx) {
        free(xpt_idx);
    }

    if (xpt_metric) {
        free(xpt_metric);
    }

    if (subseg) {
        free(subseg);
    }

    return;

    #undef INSIDE
}
/******************************************************************************************/
/**** FUNCTION: fix_metric_order                                                       ****/
/**** Checks for multiple points that have the same metric and appropriately fixes     ****/
/**** the order.                                                                       ****/
/******************************************************************************************/
void fix_metric_order(int num_x_pts, int *xpt_metric, int *xpt_idx, QVector<QPoint> &pa, Q3MemArray<int> &pa_type, int orient)
{
    int i0, i1, idx, i, m, tmp;
    int found, cont, num_enter, num_exit, type, start;
    int n0 = -1;

    i0 = 0;
    found = 0;

    do {
        i1 = i0+1;
        while((i1<=num_x_pts-1)&&(xpt_metric[i1] == xpt_metric[i0])) { i1++; }
        if (i0 == 0) { n0 = i1; }
        if ((i1-i0)&1) {
            found = 1;
        }
        if (found || (i1 == num_x_pts)) {
            cont = 0;
        } else {
            cont = 1;
            i0 = i1;
        }
    } while(cont);

    if (!found) {
        i0 = 0;
        i1 = n0;
    }

    num_enter = 0;
    num_exit  = 0;
    for (i=i0; i<=i1-1; i++) {
        if (pa_type[xpt_idx[i]] == PT_ENTER) {
            num_enter++;
        } else {
            num_exit++;
        }
    }

    if ( (!found) && (num_enter == num_exit) ) {
        int a_idx, min_a_idx;
        int x1, y1, x2, y2;

        int min_idx = i0;
        if (pa_type[xpt_idx[i0]] == PT_ENTER) {
            min_a_idx = (xpt_idx[i0] == ((int)pa.size())-1 ? 0 : xpt_idx[i0]+1);
        } else {
            min_a_idx = (xpt_idx[i0] == 0 ? ((int)pa.size())-1 : xpt_idx[i0]-1);
        }
        for (i=i0+1; i<=i1-1; i++) {
            if (pa_type[xpt_idx[i]] == PT_ENTER) {
                a_idx = (xpt_idx[i] == ((int)pa.size())-1 ? 0 : xpt_idx[i]+1);
            } else {
                a_idx = (xpt_idx[i] == 0 ? pa.size()-1 : xpt_idx[i]-1);
            }

            x1 = pa[min_a_idx].x() - pa[xpt_idx[min_idx]].x();
            y1 = pa[min_a_idx].y() - pa[xpt_idx[min_idx]].y();
            x2 = pa[    a_idx].x() - pa[xpt_idx[min_idx]].x();
            y2 = pa[    a_idx].y() - pa[xpt_idx[min_idx]].y();
            if ( (x1*y2-x2*y1)*(orient?1:-1) < 0) {
                min_idx = i;
                min_a_idx = a_idx;
            }
        }
        type = pa_type[xpt_idx[min_idx]];
    } else if ( (found) && (num_enter == num_exit  + 1) ) {
        type = PT_ENTER;
    } else if ( (found) && (num_exit  == num_enter + 1) ) {
        type = PT_EXIT;
    } else {
        type = -1;
        CORE_DUMP;
    }
    
    start = i0;
    idx = start;

    do {

        if (pa_type[xpt_idx[idx]] != type) {
            m = xpt_metric[idx];
            i = idx+1;
            while( (i<=num_x_pts-1) && (pa_type[xpt_idx[i]] != type) ) { i++; }
            if ( (i == num_x_pts) || (xpt_metric[i] != m) ) {
                CORE_DUMP;
            }
            tmp             = xpt_metric[idx];
            xpt_metric[idx] = xpt_metric[i];
            xpt_metric[i]   = tmp;

            tmp          = xpt_idx[idx];
            xpt_idx[idx] = xpt_idx[i];
            xpt_idx[i]   = tmp;
        }

        idx = ( (idx == num_x_pts-1) ? 0 : idx+1);
        type = ( (type == PT_ENTER) ? PT_EXIT : PT_ENTER );
    } while (idx != start);

    return;
}
/******************************************************************************************/

#undef INSIDE
#undef PT_GENERIC
#undef PT_ENTER
#undef PT_EXIT
#undef PT_DELETE

/******************************************************************************************/
PositionClass::PositionClass()
{
    pixel_x = 0;
    pixel_y = 0;
    grid_x  = 0;
    grid_y  = 0;
    posn_x  = 0.0;
    posn_y  = 0.0;
    lon_deg = 0.0;
    lat_deg = 0.0;
}
/******************************************************************************************/
PositionClass::~PositionClass()
{
}
/******************************************************************************************/
void PositionClass::setPosition(NetworkClass *np, FigureEditor *editor, QMouseEvent *e)
{
    pixel_x = e->pos().x();
    pixel_y = e->pos().y();

    editor->canvas_to_xy(grid_x, grid_y, pixel_x, pixel_y);

    posn_x = np->idx_to_x(grid_x);
    posn_y = np->idx_to_y(grid_y);

    if (np->coordinate_system == CConst::CoordUTM) {
        UTMtoLL( posn_x, posn_y, lon_deg,  lat_deg, np->utm_zone, np->utm_north,
                 np->utm_equatorial_radius, np->utm_eccentricity_sq);
    }
}
/******************************************************************************************/
int qstringcmp(QString s1, QString s2)
{
    int n1 = s1.length();
    int n2 = s2.length();
    int done = 0;
    int d1, d2;
    int nd1=0, nd2=0;
    int v1, v2;
    int retval = 0;
    QChar c1, c2;

    if ( (n1 == 0) && (n2 == 0) ) {
        return(0);
    } else if (n1 == 0) {
        return(-1);
    } else if (n2 == 0) {
        return(1);
    }

    int i1 = 0;
    int i2 = 0;

    while(!done) {
        c1 = s1.at(i1);
        d1 = c1.isDigit();
        if (d1) {
            nd1 = 1;
            while((i1+nd1<n1) && (s1.at(i1+nd1).isDigit())) {
                nd1++;
            }
        }

        c2 = s2.at(i2);
        d2 = c2.isDigit();
        if (d2) {
            nd2 = 1;
            while((i2+nd2<n2) && (s2.at(i2+nd2).isDigit())) {
                nd2++;
            }
        }

        if ((!d1) && (!d2)) {
            if (c1 < c2) {
                retval = -1;
                done = 1;
            } else if (c1 > c2) {
                retval = 1;
                done = 1;
            } else if ((i1 == n1-1) && (i2 < n2-1)) {
                retval = -1;
                done = 1;
            } else if ((i2 == n2-1) && (i1 < n1-1)) {
                retval = 1;
                done = 1;
            } else if ((i1 == n1-1) && (i2 == n2-1)) {
                retval = 0;
                done = 1;
            } else {
                i1++;
                i2++;
            }
        } else if ((!d1) && (d2)) {
            retval = 1;
            done = 1;
        } else if ((d1) && (!d2)) {
            retval = -1;
            done = 1;
        } else if ((d1) && (d2)) {
            while ((nd1 > nd2)&&(!done)) {
                if (s1.at(i1).digitValue()) {
                    retval = 1;
                    done = 1;
                } else {
                    i1++;
                    nd1--;
                }
            }
            while ((nd2 > nd1)&&(!done)) {
                if (s2.at(i2).digitValue()) {
                    retval = -1;
                    done = 1;
                } else {
                    i2++;
                    nd2--;
                }
            }
            while(!done) {
                if (nd1==0) {
                    retval = 0;
                    done = 1;
                } else {
                    v1 = s1.at(i1).digitValue();
                    v2 = s2.at(i2).digitValue();
                    if (v1 < v2) {
                        retval = -1;
                        done = 1;
                    } else if (v1 > v2) {
                        retval = 1;
                        done = 1;
                    } else {
                        i1++;
                        i2++;
                        nd1--;
                        nd2--;
                    }
                }
            }
        }
    }

    // printf("S1 = \"%s\"   S2 = \"%s\"   RETVAL = %d\n", s1.latin1(), s2.latin1(), retval);

    return(retval);
}
/******************************************************************************************/


//skip current line
void skipLine( FILE* file )
{
    char ch = fgetc( file );
    while( ch != '\n' && ch != EOF ) 
        ch = fgetc( file );
}


//skip num_field fields separated by token "token"
//return 0 if success, otherwise return 1
int skipFields( FILE* file, int num_field, char token )
{
    char ch;
    int i = 0;
    for( i=0; i<num_field; i++ ) {
        ch = fgetc( file );
        //printf( "%c", ch );
        while( ch != token && ch != '\n' && ch != EOF ) {
            ch = fgetc( file );
        }
        if( ch != token ) 
            break;
    }
    if( i == num_field ) return 0;
    else  return 1;
}


//read the content of filed filed_num into string str, rewind the pos pointer to the beginning of current line
//return 0 if success, 1 if error, -1 if encounter EOF
int readField( char* str, FILE* file, int field_num, char token )
{
    int curpos = ftell( file );
    if( skipFields( file, field_num, token ) == 1 ) {
        perror( "skipFields encounted an error\n" );
        return 1;
    }
    char ch = fgetc( file );
    if( ch == EOF ) 
        return -1;
    int i = 0;
    while( ch != token && ch != EOF && ch != '\n' ) {
        str[i] = ch;
        ch = fgetc( file );
        i++;
    }
    str[i] = '\0';
    if( (fseek( file, curpos, SEEK_SET ))!=0 ) {
        perror( "fseek error\nprogram aborted\n");
        exit(1);
    }
    return 0;
}

char * trim(char * src)
{
         int i = 0;
         char *begin = src;
         while(src[i] != '\0'){
                if(src[i] != ' '){
                      break;
                }else{
                      begin++;
                }
               i++;
         }
          for(i = strlen(src)-1; i >= 0;  i--){
                if(src[i] != ' '){
                      break;
                }else{
                      src[i] = '\0';
                }
         }
         return begin;
}

