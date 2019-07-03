/******************************************************************************************/
/**** FILE: report_prop_model_param.cpp                                                ****/
/******************************************************************************************/
#include <stdio.h>
#include <math.h>
#include <qstring.h>

#include "WiSim.h"
#include "prop_model.h"
#include "doubleintint.h"
#include "list.h"
#include "cconst.h"
#include "strint.h"

#include <iostream>

/******************************************************************************************/
/**** FUNCTION: NetworkClass::report_prop_model_param                                  ****/
/****                                                                                  ****/
/**** param = 0 : inflexion                                                            ****/
/****         1 : slope                                                                ****/
/****         2 : all parameters                                                       ****/
/****                                                                                  ****/
/**** model = 0 : all prop models                                                      ****/
/****         1 : used prop models                                                     ****/
/******************************************************************************************/
void NetworkClass::report_prop_model_param(int param, int model, char *filename)
{
    int pm_idx, i, seg;
    double slope;
    char *chptr;
    SegmentPropModelClass *pm;
    ListClass<DoubleIntIntClass> *param_list = new ListClass<DoubleIntIntClass>(10);
    FILE *fp;

    if (model == 0) {
        for (pm_idx=0; pm_idx<=num_prop_model-1; pm_idx++) {
            if (prop_model_list[pm_idx]->type() == CConst::PropSegment) {
                pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                if ( param == 2 || param == 0 ) { // inflexion and all parameters types output
                    for( seg=0; seg<=pm->num_inflexion-1; seg++ ) {
                        param_list->ins_elem(DoubleIntIntClass(pm->x[seg], pm_idx, seg), 1);
                    }
                //} else if ( (param == 0) || (param == 1) ) {
                } else if ( param == 1 ) { // slope type of output
                    for( seg=0; seg<=pm->num_inflexion; seg++ ) {
                        if (seg == 0) {
                            slope = pm->start_slope;
                        } else if (seg == pm->num_inflexion) {
                            slope = pm->final_slope;
                        } else {
                            slope = (pm->y[seg] - pm->y[seg-1])/(pm->x[seg] - pm->x[seg-1]);
                        }
                        param_list->ins_elem(DoubleIntIntClass(slope, pm_idx, seg), 1);
                    }
                }
            }
        }

        param_list->sort();
    }

    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "#########################################################################\n");
    if (param == 2 && model == 0) {
    chptr += sprintf(chptr, "#### Propagation Model Param Report (ALL)                            ####\n");
    } else if (param == 0 && model == 0) {
    chptr += sprintf(chptr, "#### Propagation Model Param Report (INFLEXION)                      ####\n");
    } else if ( param == 1  && model == 0) {
    chptr += sprintf(chptr, "#### Propagation Model Param Report (SLOPE)                          ####\n");
    } else if ( param == 2 && model == 1 ) {
    chptr += sprintf(chptr, "#### Propagation Model Param Report (CS LIST)                        ####\n");
    }
    chptr += sprintf(chptr, "#########################################################################\n");
    chptr += sprintf(chptr, "\n");
    PRMSG(fp, msg);


    if (param == 2 && model == 0) {
        sprintf(msg, "%-15s %-12s %-50s %-32s %-13s\n",
                "PM_NAME", "NUM_INFLEX", "INFLEXS(DIST,  PWR)",
                "SLOPS", "INTERCEPT_PWR" );
    } else if (param == 2 && model == 1) {
        sprintf(msg, "%-8s %-10s %-14s %-15s %-12s %-50s %-32s %-13s\n", 
                "INDEX", "GW_CSC_CS", "CSID", "PM_NAME", "NUM_INFLEX",
                "INFLEXS(DIST,  PWR)", "SLOPS", "INTERCEPT_PWR" );
    } else if (param == 0 && model == 0) {
        sprintf(msg, "%-15s %-12s %-50s\n", "PROP_MODEL_NAME", "NUM_INFLEX", "INFLEXS(DIST, PWR)");
    } else if (param == 1 && model == 0) { 
        sprintf(msg, "%15s %30s %15s\n", "SLOPE",     "PROP_MODEL_NAME", "SEGMENT_NUM");
    }
    PRMSG(fp, msg);

    int num_inflex(1), j(0);
    double intercept_pwr(100);
    double *dv, *pv, *sv;
    QString str, str1, str2;
    if ( ( param == 0 || param == 2 ) && model == 0 ) {
        for ( pm_idx=0; pm_idx<num_prop_model; pm_idx++ ) { 
            if ( prop_model_list[pm_idx]->type() == CConst::PropSegment ) {
                pm = (SegmentPropModelClass *) prop_model_list[pm_idx];
                num_inflex = pm->num_inflexion;
                dv = (double*) malloc ( num_inflex     * sizeof (double) );
                pv = (double*) malloc ( num_inflex     * sizeof (double) );
                sv = (double*) malloc ( (num_inflex+1) * sizeof (double) );
                
                // initilization
                str  = "";
                str1 = "";
                str2 = "";
                for ( j=0; j<num_inflex; j++ ) {
                    dv[j] = exp( pm->x[j]*log(10.) ); 
                    pv[j] = pm->y[j];
                    if (j==0) intercept_pwr = pv[j]-(pm->start_slope)*(pm->x[j]); 

                    /*
                    if ( j <  num_inflex - 1 )
                        std::cout << "param      = " << param       << std::endl
                                  << "pm_idx     = " << pm_idx      << std::endl
                                  << "num_inflex = " << num_inflex  << std::endl
                                  << "j          = " << j           << std::endl
                                  << "pm->y[j]   = " << pm->y[j]    << std::endl
                                  << "pm->y[j+1] = " << pm->y[j+1]  << std::endl
                                  << "pm->x[j]   = " << pm->x[j]    << std::endl
                                  << "pm->x[j+1] = " << pm->x[j+1]  << std::endl
                                  << std::endl;
                      */

                    // get string of inflexion coodination
                    str1 += str.sprintf( "(%-8.3f %-8.3f) ", dv[j], pv[j] );

                    if ( j < num_inflex-1 )
                    {
                        sv[j+1] = (pm->y[j+1]-pm->y[j])/(pm->x[j+1]-pm->x[j]);
                        str2 += str.sprintf( "%-8.2f ", sv[j+1] );
                    }
                }

                sv[0] = pm->start_slope;
                sv[num_inflex] = pm->final_slope;

                if ( param == 2 )
                {
                    sprintf(msg, "%-15s %-12d %-32s %-8.2f %-24s %-8.2f %-13.2f\n", 
                            pm->get_strid(), num_inflex, str1.latin1(), sv[0], str2.latin1(), sv[num_inflex], intercept_pwr);
                    PRMSG(fp, msg);
                }
                else if ( param == 0 )
                {
                    sprintf(msg, "%-15s %-12d %-50s \n", pm->get_strid(), num_inflex, str1.latin1() );
                    PRMSG(fp, msg);
                }

                free (dv); free (pv); free (sv);
            }
        }
    }

    int cell_name_idx, cell_name_pref;
    CellClass      *cell;
    SectorClass *sector;
    if ( param == 2 && model == 1 ) {
        for ( int  cell_idx=0; cell_idx<num_cell; cell_idx++) {
            cell = cell_list[cell_idx];

            for ( int sector_idx=0; sector_idx<cell->num_sector; sector_idx++ ) {
                sector = cell->sector_list[sector_idx];

                int prop_idx = sector->prop_model;
                if ( prop_idx >= 0 ) pm = (SegmentPropModelClass *) prop_model_list[prop_idx];
                if ( prop_idx >= 0 && prop_model_list[prop_idx]->type() == CConst::PropSegment ) {
                    chptr = msg;

                    chptr += sprintf( chptr, "%d_%d", cell_idx, sector_idx ); 

                    for (cell_name_idx=1; cell_name_idx<=report_cell_name_opt_list->getSize()-1; cell_name_idx++) {
                        cell_name_pref = (*(report_cell_name_opt_list))[cell_name_idx].getInt();
                        chptr += sprintf(chptr, "    %s", cell->view_name(cell_idx, cell_name_pref));
                    }

                    num_inflex = pm->num_inflexion;
                    dv = (double*) malloc ( num_inflex     * sizeof (double) );
                    pv = (double*) malloc ( num_inflex     * sizeof (double) );
                    sv = (double*) malloc ( (num_inflex+1) * sizeof (double) );
                    
                    // initilization
                    str  = "";
                    str1 = "";
                    str2 = "";
                    for ( j=0; j<num_inflex; j++ ) {
                        dv[j] = exp( pm->x[j]*log(10.) ); 
                        pv[j] = pm->y[j];
                        if (j==0) intercept_pwr = pv[j]-(pm->start_slope)*(pm->x[j]); 

                        if ( j != num_inflex )
                            sv[j+1] = (pm->y[j+1]-pm->y[j])/(pm->x[j+1]-pm->x[j]);

                        // get string of inflexion coodination
                        str1 += str.sprintf( "(%-8.3f %-8.3f) ", dv[j], pv[j] );
                        str2 += str.sprintf( "%-8.2f ", sv[j+1] );
                    }
                    sv[0] = pm->start_slope;
                    if ( num_inflex == 2 ) sv[1] = (pm->y[1]-pm->y[0])/(pm->x[1]-pm->x[0]);
                    sv[num_inflex] = pm->final_slope;

                    chptr += sprintf(chptr, " %-12s %-12d %-32s %-8.2f %-24s %-13.2f\n", 
                            pm->get_strid(), num_inflex, str1.latin1(), sv[0], str2.latin1(), intercept_pwr);
                    PRMSG(fp, msg);

                    free (dv); free (pv); free (sv);
                }
            }
        }
    }

    sprintf(msg, "%s", "");
    for (i=0; i<=param_list->getSize()-1; i++) {
        pm_idx = (*param_list)[i].getInt(0);
        seg = (*param_list)[i].getInt(1);
        pm = (SegmentPropModelClass *) prop_model_list[pm_idx];

        if ( param == 2 )
        {
            // do nothing
        } 
        else if ( param == 0 )
        {
        }
        else if ( param == 1 )
        {
            sprintf(msg, "%15.6f %30s %15d\n", (*param_list)[i].getDouble(), pm->get_strid(), seg);
        }
        PRMSG(fp, msg);
    }

    if ( param == 0 || param == 1 )
        sprintf(msg, "#########################################################################\n");
    else
        sprintf(msg, "#########################################################################%s",
                "####################################################\n");

    PRMSG(fp, msg);

    if ( fp != stdout ) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
