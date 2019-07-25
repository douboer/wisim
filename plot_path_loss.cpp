/******************************************************************************************/
/**** FILE: plot_path_loss.cpp                                                         ****/
/******************************************************************************************/

#include <math.h>
#include <string.h>
#include <time.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

#include "antenna.h"
#include "cconst.h"
#include "wisim_gui.h"
#include "wisim.h"
#include "coverage.h"
#include "datachart.h"
#include "list.h"
#include "phs.h"
#include "polygon.h"
#include "pref.h"
#include "prop_model.h"
#include "road_test_data.h"

/******************************************************************************************/
/**** FUNCTION: FigureEditor::plotPathLoss                                             ****/
/******************************************************************************************/
void FigureEditor::plotPathLoss()
{
    int i;
    int     rtd_idx  = 0;
    int     cell_num = 0;
    int     cell_idx = 0;
    int     sector_idx = 0;
    int     point_num= 0;

    int found = 0;
    double  min_pwr  = 0.0;
    double  max_pwr  = 0.0;
    double  min_logd = 0.0;
    double  max_logd = 0.0;
    double  distsq   = 0.0;

    double          *log_dis = NULL;
    double          *pow_db  = NULL;
    double          *point_x = NULL;
    double          *point_y = NULL;
    //xyGraph         *pdg     = NULL;
    CellClass       *cell    = (CellClass       *) NULL;
    SectorClass     *sector  = (SectorClass     *) NULL;
    RoadTestPtClass *rtp     = (RoadTestPtClass *) NULL;

    cell_num = selectCell->getCellIdx();
    cell_idx = cell_num & ((1<<np->bit_cell)-1);
    sector_idx = cell_num >> np->bit_cell;
    cell = np->cell_list[cell_idx];
    sector = cell->sector_list[sector_idx];

    double angle_deg = 0.0;
    if ( sector->prop_model >=0 ) {
        int pm_type = np->prop_model_list[sector->prop_model]->type();
        if (   ((pm_type == CConst::PropSegment) && (((SegmentPropModelClass *) np->prop_model_list[sector->prop_model])->num_clutter_type))
            || (pm_type == CConst::PropSegmentWithTheta)
            || (pm_type == CConst::PropSegmentAngle)
            || (pm_type == CConst::PropClutterSimp) ) {
            bool ok;
            angle_deg = QInputDialog::getDouble("WiSim", "Enter Angle (degrees):", 0.0, -180.0, 360.0, 3, &ok, this);
            if ( ok ) {
                // user entered something and pressed OK
            } else {
                return;
            }
        }
    }

    /***************************************/
    /*     plot road test data             */
    /***************************************/
    int num_rtp = 0;
    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        if ( (rtp->cell_idx == cell_idx) && (rtp->sector_idx == sector_idx) ) {
            num_rtp++;
        }
    }

    log_dis = DVECTOR(num_rtp);
    pow_db  = DVECTOR(num_rtp);

    int idx = 0;
    for (rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*(np->road_test_data_list))[rtd_idx] );
        if ( (rtp->cell_idx == cell_idx) && (rtp->sector_idx == sector_idx) ) {
            double dx = (rtp->posn_x - cell->posn_x)*np->resolution;
            double dy = (rtp->posn_y - cell->posn_y)*np->resolution;
            double dz = - sector->antenna_height;
            double logd = 0.5*log( dx*dx + dy*dy + dz*dz ) / log(10.0);
            AntennaClass *antenna = np->antenna_type_list[sector->antenna_type];
            double gain_db = antenna->gainDB(dx, dy, dz, sector->antenna_angle_rad);
            double tx_pwr_db = 10.0*log(sector->tx_pwr)/log(10.0);
            log_dis[idx] = logd;
            pow_db[idx]  = rtp->pwr_db - gain_db - tx_pwr_db;

            if (!found) {
                max_logd = min_logd = log_dis[idx];
                max_pwr  = min_pwr  = pow_db[idx];
                found = 1;
            } else {
                if ( log_dis[idx] < min_logd ) {
                    min_logd = log_dis[idx];
                }

                if ( log_dis[idx] > max_logd ) {
                    max_logd = log_dis[idx];
                }

                if ( pow_db[idx] < min_pwr ) {
                    min_pwr = pow_db[idx];
                }

                if ( pow_db[idx] > max_pwr ) {
                    max_pwr = pow_db[idx];
                }
            }
            idx++;
        }
    }

#if 0
    FILE *fprtd = fopen("roadtest.txt","w");
    for (idx=0; idx<=num_rtp-1; idx++) {
        fprintf(fprtd,"%f %f\n", log_dis[idx], pow_db[idx]);
    }
    fclose(fprtd);
#endif

    /***************************************/
    /*     plot propagation model          */
    /***************************************/
    if ( sector->prop_model >=0 ) {
#if 1
        double log_r, dist;
        int delta_x;
        point_num = 100;
        PropModelClass *pm = np->prop_model_list[sector->prop_model];
        double log_r_start = log(np->resolution) / log(10.0);
        double log_r_stop  = 4.0;  // xxxxxxxxxxxxxxx Make this an adjustable parameter
        point_x = DVECTOR(point_num);
        point_y = DVECTOR(point_num);
        for (i=0; i<=point_num-1; i++) {
            log_r = log_r_start + (log_r_stop - log_r_start)*i/(point_num-1);
            dist = exp(log(10.0)*log_r);
            check_grid_val(dist, np->resolution, 0, &delta_x);
            distsq = (np->resolution*delta_x)*(np->resolution*delta_x) + (sector->antenna_height)*(sector->antenna_height);
            point_x[i] = 0.5*log(distsq)/log(10.0);
            point_y[i] = 10.0 * log(pm->prop_power_loss(np, sector, delta_x, 0, -sector->antenna_height, angle_deg))/log(10.0);

            if (!found) {
                max_logd = min_logd = point_x[i];
                max_pwr  = min_pwr  = point_y[i];
                found = 1;
            } else {
                if ( point_x[i] < min_logd ) {
                    min_logd = point_x[i];
                }

                if ( point_x[i] > max_logd ) {
                    max_logd = point_x[i];
                }

                if ( point_y[i] < min_pwr ) {
                    min_pwr = point_y[i];
                }

                if ( point_y[i] > max_pwr ) {
                    max_pwr = point_y[i];
                }
            }
        }
#else
        SegmentPropModelClass *pm = (SegmentPropModelClass *) np->prop_model_list[sector->prop_model];

        if ( pm->num_inflexion>=1 ) {
            point_num = pm->num_inflexion+2;
        } else {
            point_num = 0;
        }
        point_x = DVECTOR(point_num);
        point_y = DVECTOR(point_num);

        if ( point_num > 0 ) {
            if ( min_logd < pm->x[0] ) {
                point_x[0] = min_logd - 0.15;
            } else {
                point_x[0] = pm->x[0] - 0.15;
            }

            point_y[0] = pm->y[0] + pm->start_slope*(point_x[0]-pm->x[0]);

            for ( int idx=0; idx<point_num-2; idx++) {
                point_x[idx+1] = pm->x[idx];
                point_y[idx+1] = pm->y[idx];
            }

            point_x[point_num-1] = max_logd + 0.001;
            if ( point_x[point_num-1]<=pm->x[0] ) {
                point_y[point_num-1] = pm->y[0] + pm->start_slope*(point_x[point_num-1]-pm->x[0]);
            } else if ( point_x[point_num-1]>pm->x[pm->num_inflexion-1] ) {
                point_y[point_num-1] = pm->y[pm->num_inflexion-1] + pm->final_slope*(point_x[point_num-1]-pm->x[pm->num_inflexion-1]);
            } else {
                for ( int idx=0;idx<pm->num_inflexion-1; idx++ ) {
                    if (point_x[point_num-1]>pm->x[idx] && point_x[point_num-1]<=pm->x[idx+1] ) {
                        point_y[point_num-1] = pm->y[idx] + ((pm->y[idx]-pm->y[idx+1])/(pm->x[idx]-pm->x[idx+1]))*(point_x[point_num-1]-pm->x[idx]);
                        break;
                    }
                }
            }
        }
#endif
    }

    /***************************************/
    /*     display the data with xyGraph   */
    /***************************************/
    char *hexstr = CVECTOR(2*((PHSSectorClass *) sector)->csid_byte_length);
    hex_to_hexstr(hexstr,((PHSSectorClass *) sector)->csid_hex,((PHSSectorClass *) sector)->csid_byte_length);

    if (  (point_num > 0)  || (num_rtp > 0) ) {
        pdg = new xyGraph( min_logd-0.1, max_logd+0.1, min_pwr-1.0, max_pwr+1.0,
                   QString("Cell_%1:").arg(cell_num)+QString(hexstr), POINTCHART, 0 );
    }

    if (  point_num > 0 ) {
        pdg->addXY( point_x, point_y, point_num );
    }

    if (  num_rtp > 0 ) {
        pdg->addXY( log_dis, pow_db, num_rtp );
    }

    if ( point_num >0 || num_rtp > 0 ) {
        pdg->setCoordDesc("Log(distance(m))","Path Loss(dB)"); //liutao,041210
        pdg->setReference( 1 );//liutao,041210
        pdg->show();
    }

    if ( point_num<=0 && num_rtp<=0 ) {
        QMessageBox::information( this, "Plot Path Loss ",
        "There are no road test points or propagation model exist.");
    }
}
/******************************************************************************************/
/**** FUNCTION: FigureEditor::plotCoverage                                             ****/
/******************************************************************************************/
void FigureEditor::plotCoverage(int cvg_idx_0, int cvg_idx_1)
{
    printf("plotCoverage: cvg_idx_0 = %d cvg_idx_1 = %d\n", cvg_idx_0, cvg_idx_1);

    NetworkClass *np = get_np();

    if( cvg_idx_0 >= np->num_coverage_analysis || cvg_idx_1 >= np->num_coverage_analysis ) {
         QMessageBox::warning( this, "Error", "coverage_idx_num out of range!\n" );
         return;
    }
    CoverageClass *c0 = np->coverage_list[cvg_idx_0];
    
    double *area_list = DVECTOR(c0->scan_list->getSize());
    char**   label;


    label = (char**) malloc( c0->scan_list->getSize()*sizeof( char*) );
    for( int k=0; k<=c0->scan_list->getSize()-1; k++ ) {
        label[k] = (char*)malloc( 100*sizeof(char) );
    }

    int scan_type_idx;
    double pwr_offset = np->preferences->pwr_offset;

    double total_area = 0.0;
//    for (scan_type_idx=0; scan_type_idx<=c0->scan_list->getSize()-1; scan_type_idx++) {
//        area_list[scan_type_idx] = 0.0;
//    }

    for( int i=0; i<=c0->scan_list->getSize()-1; i++ ) {
        // area_list[c0->polygon_scan_type[i]] += (c0->polygon_list[i])->comp_bdy_area();
        if (c0->polygon_list[i]) {
            area_list[i] = c0->polygon_list[i]->comp_bdy_area();
        } else {
            area_list[i] = 0.0;
        }
    }

    for (scan_type_idx=0; scan_type_idx<=c0->scan_list->getSize()-1; scan_type_idx++) {
        c0->scan_label(np, label[scan_type_idx], pwr_offset, scan_type_idx);
        total_area += area_list[scan_type_idx];
    }

    QColor *color = (QColor *) malloc(c0->scan_list->getSize()*sizeof(QColor));
    for (scan_type_idx=0; scan_type_idx<=c0->scan_list->getSize()-1; scan_type_idx++) {
        color[scan_type_idx].setRgb( (c0->color_list)[scan_type_idx] );
    }

    if( cvg_idx_1 == -1 ) {        
           datagraph = new xyGraph( "Coverage Report", PIECHART, 0 );
           datagraph->addXY( label, area_list, c0->scan_list->getSize() );
           datagraph->setTitle( c0->strid, Qt::magenta );
           datagraph->setPieDataColor( color );
           datagraph->show();
    }

    else {
        CoverageClass *c1 = np->coverage_list[cvg_idx_1];
        if( c0->type!=c1->type || c0->scan_list->getSize()!=c1->scan_list->getSize() ) {
            QMessageBox::warning( this, "Error", "Cannot compare coverages of different types!\n" );
            return;
        }

        double *area_list1 = DVECTOR(c1->scan_list->getSize());
        char**   label1;
        label1 = (char**) malloc( c0->scan_list->getSize()*sizeof( char*) );
        for( int k=0; k<=c0->scan_list->getSize()-1; k++ ) {
            label1[k] = (char*)malloc( 100*sizeof(char) );
        }
        int scan_type_idx1;

        double total_area1 = 0.0;
    //    for (scan_type_idx1=0; scan_type_idx1<=c1->scan_list->getSize()-1; scan_type_idx1++) {
    //        area_list1[scan_type_idx1] = 0.0;
    //    }

        for( int i=0; i<=c1->scan_list->getSize()-1; i++ ) {
            // area_list1[c1->polygon_scan_type[i]] += (c1->polygon_list[i])->comp_bdy_area();
            if (c1->polygon_list[i]) {
                area_list1[i] = c1->polygon_list[i]->comp_bdy_area();
            } else {
                area_list1[i] = 0.0;
            }
        }

        for (scan_type_idx1=0; scan_type_idx1<=c1->scan_list->getSize()-1; scan_type_idx1++) {
            c1->scan_label(np, label1[scan_type_idx1], pwr_offset, scan_type_idx1);
            total_area1 += area_list1[scan_type_idx1];
            //printf("area%d: %f\n", scan_type_idx1, area_list1[scan_type_idx1] );
        }
        
        datagraph = new xyGraph( "Coverage Report", PIECHART2, 0 );
        datagraph->addXY( label, area_list, c0->scan_list->getSize() );
        datagraph->addXY( label1, area_list1, c1->scan_list->getSize() );
        datagraph->setTitle( c0->strid, c1->strid, Qt::magenta ); 
        datagraph->setPieDataColor( color );
        datagraph->show();
    }

    free(color);

    return;
}



/******************************************************************************************/
