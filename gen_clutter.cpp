
/******************************************************************************************
**** PROGRAM: gen_clutter.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

/*******************************************************************************************
**** PROGRAM: gen_clutter.cpp 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/
/******************************************************************************************/
/**** FILE: gen_clutter.cpp                                                            ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "wisim.h"
#include "cconst.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "pref.h"
#include "clutter_data_analysis.h"
#include "road_test_data.h"
#include "polygon.h"
#include "antenna.h"

enum SimScope {
    Global,
    Individual
};

extern char *wisim_home;

static SimScope scope = Global;

#if 0
// Move to road_test_data.cpp  xxxxxxxxxx
/******************************************************************************************/
/**** FUNCTION: get_angle( const int, const int, const int ,const int)                 ****/ 
/**** Return angle which range from -180 degree to 180 degree                          ****/ 
/******************************************************************************************/
double get_angle( const int start_x, const int start_y, const int end_x, const int end_y)
{
    int heading;
    double angle;
    double slope;
    
    if ( start_x > end_x ) heading = 1;
    else heading = 0;
    
    if ( start_x==end_x ) {
        if ( start_y<end_y ) angle = PI/2.0;
        else angle = -PI/2.0;
    } else {
        slope = (double)(end_y-start_y)/(end_x-start_x);
        angle = atan( slope);
        if ( slope > 0 ) angle = angle - heading*PI;
        if ( slope < 0 ) angle = angle + heading*PI;
    }

    return(angle);
}
#endif

#if CLUTTER_DEBUG
static std::ofstream clutter_sim_result;
static std::ofstream clutter_coeff;
static std::ofstream stat_dis_pow;
static std::ofstream matrix_test;
static std::ofstream a_posn_test;
#endif
static int idx = 0;

/******************************************************************************************/
/**** FUNCTION: NetworkClass::gen_clutter                                              ****/
/******************************************************************************************/
void NetworkClass::gen_clutter( ListClass<int> *scan_index_list, int useheight, int scope,
    int type, double slope, double intercept, int clutter_sim_res_ratio )
{
#if (DEMO == 0)
    int m_type = type;

    char fn_str[100];
    char ostr[35];

    if ( road_test_data_list->getSize() == 0 ) {
        sprintf(msg, "ERROR: No road test data defined, unable to generate clutter\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    idx++;
    sprintf( ostr, "clutter_coeff_%d.txt", idx ); 

#if 0
    printf("WISIM HOME: %s\n", wisim_home);
    printf("PWR OFFSET: %12.10f\n", preferences->pwr_offset);
#endif

#if CLUTTER_DEBUG
    //Open log file to write into.
    sprintf( fn_str, "%s%cclt_sim_result.txt", wisim_home, FPATH_SEPARATOR );
    clutter_sim_result.open( fn_str );
    if( !clutter_sim_result ) {
        sprintf( msg, "File \"%s\" not exist.\n", fn_str );
        PRMSG(stdout, msg);
        error_state = 1;
    }

    sprintf( fn_str, "%s%c%s", wisim_home, FPATH_SEPARATOR, ostr );
    clutter_coeff.open( fn_str );
    if( !clutter_coeff ) {
        sprintf( msg, "File \"%s\" not exist.\n", fn_str );
        PRMSG(stdout, msg);
        error_state = 1;
    }
#endif

    //Statistics the distance and the recieve power (and angle) of road test points of a cell
    sprintf( fn_str, "%s%cdist_power.txt", wisim_home, FPATH_SEPARATOR );
#if CLUTTER_DEBUG
    stat_dis_pow.open( fn_str );
    if( !stat_dis_pow) {
        sprintf( msg, "File \"%s\" not exist.\n", fn_str );
        PRMSG(stdout, msg);
        error_state = 1;
    }

    //Save the value of matrix mmx_a. 
    sprintf( fn_str, "%s%ctest_matrix.txt", wisim_home, FPATH_SEPARATOR );
    matrix_test.open( fn_str );
    if( !matrix_test) {
        sprintf( msg, "File \"%s\" not exist.\n", fn_str );
        PRMSG(stdout, msg);
        error_state = 1;
    }

    sprintf( fn_str, "%s%ctest_aposn.txt", wisim_home, FPATH_SEPARATOR );
    a_posn_test.open( fn_str );
    if( !a_posn_test ) {
        sprintf( msg, "File \"%s\" not exist.\n", fn_str );
        PRMSG(stdout, msg);
        error_state = 1;
    }
#endif

    /*
       This section is writed for debug on linux operating system. 
       We can input parameters from shell.
       Now these parameters specify by wisim process command.
       So, the section code will delete in the future.
     */
    int   scan_idx          = 0;
    if ( scope == Individual )
    {
        scope = Individual;
    } else if ( scope == Global )
    {
        scope = Global;
    }

    if ( scope == Individual ) {
        ListClass<int> *tmplist = new ListClass<int>(1);
        tmplist->append(0);
        for ( scan_idx=0; scan_idx<=scan_index_list->getSize()-1; scan_idx++ ) {
            (*tmplist)[0] = (*scan_index_list)[scan_idx];
            gen_clutter( tmplist, useheight, m_type, slope, intercept, clutter_sim_res_ratio );
        }
        delete tmplist;
    } else if ( scope == Global ) {
        gen_clutter( scan_index_list, useheight, m_type, slope, intercept, clutter_sim_res_ratio );
    }

#if CLUTTER_DEBUG
    //Finished write files, so close them.
    clutter_sim_result.close();
    clutter_coeff.close();
    matrix_test.close();
    stat_dis_pow.close();
    a_posn_test.close();
#endif

#endif
}


/******************************************************************************************/
/**** FUNCTION: NetworkClass::gen_clutter                                              ****/
/******************************************************************************************/
void NetworkClass::gen_clutter( ListClass<int> *scan_index_list, int useheight, int type, double slope, double intercept, int clutter_sim_res_ratio )
{
    int i;
    int num_var;

    int m_type = type;
    //double m_slope, m_intercept;

    int    cell_idx;
    int    sector_idx;
    int    scan_idx;
    int    rtp_idx;
    int    num_insys_rtp;      //Save the number of road test points in system boundry;
    double error;
    double rms;      //save rms error

#if OUTPUT_TIME_INFOR
    time_t td;
#endif

    MapClutterClass* map_clutter = NULL;

    /* CConst class defines five types of Clutter propagation models:
     *        1.  PropClutterSimp
     *        2.  PropClutterFull
     *        3.  PropClutterSymFull
     *        4.  PropClutterWtExpo
     *        5.  PropClutterWtExpoSlope
     *        6.  PropClutterGlobal
     *        7.  PropClutterExpoLinear
     **********************************************************************************/
    switch(m_type) {
        case CConst::PropClutterSimp:        printf("RUNNING SIMPLIFIED    CLUTTER PROPAGATION MODEL ALGORITHM ...\n"); break;
        case CConst::PropClutterFull:        printf("RUNNING FULL          CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        case CConst::PropClutterSymFull:     printf("RUNNING SYMETRIC FULL CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        case CConst::PropClutterWtExpo:      printf("RUNNING WT EXPO       CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        case CConst::PropClutterWtExpoSlope: printf("RUNNING WT EXPO SLOPE CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        case CConst::PropClutterGlobal:      printf("RUNNING WT GLOBAL     CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        case CConst::PropClutterExpoLinear:  printf("RUNNING EXPO LINEAR   CLUTTER PROPAGATION MODEL ALGROITHM ...\n"); break;
        default: CORE_DUMP; break;
    }

#if OUTPUT_TIME_INFOR
    time(&td);
    printf("\n---- Time ---- %s", ctime(&td));

    sprintf(msg, "---- Time ---- %s \n", ctime(&td));
    PRMSG(stdout, msg);
#endif

    double** mmx_a          = NULL;
    int*     a_posn         = NULL;   //Save the value of the clutter type vector of CELL layers in.

    double *vec_b;
    
    CellClass       *cell          = NULL;    
    SectorClass     *sector        = NULL;   
    RoadTestPtClass *road_test_pt  = NULL;

    //Allocate a propagation model.  
    char    strr[30];
    char*   string;
    int     index = 0;
    for ( i=0; i<num_prop_model; i++ ) {
        string = prop_model_list[i]->get_strid();
        if ( strncmp(string,"CLT_MD_SIMP", 11) == 0 ) {
            index++;
        } 
        else if ( strncmp(string,"CLT_MD_FULL", 11) == 0 ) {
            index++;
        }
        else if ( strncmp(string,"CLT_MD_SYMFULL", 14) == 0 ) {
            index++;
        }
        else if ( strncmp(string,"CLT_MD_WTEXPO", 13) == 0 ) {
            index++;
        }
        else if ( strncmp(string,"CLT_MD_WTEXPOSLOPE", 18) == 0 ) {
            index++;
        }
        else if ( strncmp(string,"CLT_MD_WTGLOBAL", 15) == 0 ) {
            index++;
        }
        else if ( strncmp(string,"CLT_MD_EXPOLINEAR", 17) == 0 ) {
            index++;
        }
    }

         if ( m_type == CConst::PropClutterSimp        ) { sprintf(strr, "CLT_MD_SIMP_%d",        index); }
    else if ( m_type == CConst::PropClutterFull        ) { sprintf(strr, "CLT_MD_FULL_%d",        index); }
    else if ( m_type == CConst::PropClutterSymFull     ) { sprintf(strr, "CLT_MD_SYMFULL_%d",     index); }
    else if ( m_type == CConst::PropClutterWtExpo      ) { sprintf(strr, "CLT_MD_WTEXPO_%d",      index); }
    else if ( m_type == CConst::PropClutterWtExpoSlope ) { sprintf(strr, "CLT_MD_WTEXPOSLOPE_%d", index); }
    else if ( m_type == CConst::PropClutterGlobal      ) { sprintf(strr, "CLT_MD_WTGLOBAL_%d",    index); }
    else if ( m_type == CConst::PropClutterExpoLinear  ) { sprintf(strr, "CLT_MD_EXPOLINEAR_%d",  index); }

    GenericClutterPropModelClass *pm = NULL;
    ClutterGlobalPropModelClass *tpm = NULL;

    switch(m_type) {
        case CConst::PropClutterSimp:
            pm =  (GenericClutterPropModelClass *) new ClutterPropModelClass(strr);
            ((ClutterPropModelClass*) pm)->k = slope;
            ((ClutterPropModelClass*) pm)->b = intercept;
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = pm->num_clutter_type+useheight+1;
            break;
        case  CConst::PropClutterFull:
            pm =  (GenericClutterPropModelClass *) new ClutterPropModelFullClass(strr);
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = 2*pm->num_clutter_type+useheight;
            break;
        case CConst::PropClutterSymFull:
            pm =  (GenericClutterPropModelClass *) new ClutterSymFullPropModelClass(strr);
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = 2*pm->num_clutter_type+useheight;
            break;
        case CConst::PropClutterWtExpo:
            pm =  (GenericClutterPropModelClass *) new ClutterWtExpoPropModelClass(strr);
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = 2*pm->num_clutter_type+useheight;
            break;
        case CConst::PropClutterWtExpoSlope:
            pm =  (GenericClutterPropModelClass *) new ClutterWtExpoSlopePropModelClass(strr);
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = pm->num_clutter_type+useheight+1;
            break;
        case CConst::PropClutterGlobal:
            pm =  (GenericClutterPropModelClass *) new ClutterGlobalPropModelClass(strr);
            tpm = (ClutterGlobalPropModelClass *) pm;
            // need to assign PM's useheight value, this will be used in comp_global_model()
            tpm->useheight = useheight;
            // create global segment PM
            tpm->globPm = new SegmentPropModelClass(map_clutter, (char*) NULL);
            // calcualte parameters for global segment PM
            tpm->comp_global_model(this, scan_index_list->getSize(),  &((*scan_index_list)[0]), log(tpm->r0)/log(10.0));
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = pm->num_clutter_type+useheight+1;
            break;
        case CConst::PropClutterExpoLinear:
            pm =  (GenericClutterPropModelClass *) new ClutterExpoLinearPropModelClass(strr);
            pm->define_clutter(this, clutter_sim_res_ratio);
            num_var = pm->num_clutter_type+useheight+1;
            break;
        default:
            CORE_DUMP; break;
    }

    pm->useheight = useheight;

    /*
       Function of this loop 
       1.  Compute Path Loss of all RTPs which distribue in system boundary, and fill them to vector vec_b.
       2.  Statistics the number of all RTPs and RTPs in SYS-BDY.
     */
    PolygonClass *map_bdy = pm->create_map_bdy();

    num_insys_rtp = 0; 
    for (rtp_idx=0; rtp_idx<=road_test_data_list->getSize()-1; rtp_idx++) {
        road_test_pt = &( (*road_test_data_list)[rtp_idx] );         
        cell_idx = road_test_pt->cell_idx;        
        sector_idx = road_test_pt->sector_idx;
        scan_idx = (sector_idx << bit_cell) | cell_idx;
        //std::cout << " Individual CS Propagation Model 1 " << std::endl;

        if (scan_index_list->contains(scan_idx)) {
            cell = cell_list[cell_idx];
            sector = cell->sector_list[sector_idx];
            
#if CLUTTER_DEBUG
            stat_dis_pow << "#Cell Index : " << cell_idx << std::endl;
            stat_dis_pow << "#" << std::setw(10) << "DISTANCE" 
                << std::setw(14) << "ANGLE"
                << std::setw(15) << "RECIEVE_POWER" 
                << std::setw(15) << "TX_POWER"
                << std::setw(15) << "ANNTA_GAIN" 
                << std::setw(15) << "PATH_GAIN" 
                << std::endl;
#endif
            if (map_bdy->in_bdy_area(road_test_pt->posn_x, road_test_pt->posn_y)) {
                num_insys_rtp ++; 
            }
        }
    }
    //std::cout << "Number of RTD points in SYS-BDY " << num_insys_rtp << std::endl; 


    /*
       Judge the number of RTPs in SYS-BDY, if it is less than 30, return.
     */
    if ( scope == Individual ) {
        if ( num_insys_rtp<30 ) {
#if CLUTTER_DEBUG
            clutter_coeff << "SCANNING CS INDEX " << (*scan_index_list)[0] << " ... " << "   Number of Road Test Points in SYS-BOUNDRY "
                << num_insys_rtp << " < 30 " << std::endl;

            for ( clutter_idx=0; clutter_idx<pm->num_clutter_type; clutter_idx++ ) {
                clutter_coeff << "Clutter " << std::setw(3) << clutter_idx << " Coeffecient: " << std::setprecision(5) << std::setw(12) << "null"<< std::endl;
            }

            clutter_coeff << "TOTAL SQUARE ERROR = null" << std::endl;
            clutter_coeff << "RMS ERROR = null " << std::endl << std::endl;
#endif

            sprintf(msg, "ERROR LEVEL 1: CELL %d num_insys_rtp = %d < 30.\n", (*scan_index_list)[0], num_insys_rtp);
            PRMSG(stdout, msg);
            //error_state = 1;
            return;
        } else {
#if CLUTTER_DEBUG
            clutter_coeff << "SCANNING CS INDEX " << (*scan_index_list)[0] << " ... " << "   Number of RTD points in SYS-BDY "<< num_insys_rtp << std::endl;
#endif
        }
    }

    pm->create_svd_matrices(this, scan_index_list, map_bdy, num_insys_rtp, num_var, mmx_a, vec_b);

#if CLUTTER_DEBUG
    if ( scope == Individual ) { 
        clutter_sim_result << "SCANNING CS INDEX " << (*scan_index_list)[0] << " ... " << std::endl;
    }
    clutter_sim_result << "num of clutter type " << pm->num_clutter_type << std::endl;
    clutter_sim_result << "number of road test points in system boundary " << num_insys_rtp << std::endl << std::endl;
#endif

   error = pm->solve_clutter_coe( num_insys_rtp, num_var, mmx_a, vec_b );

#if OUTPUT_TIME_INFOR
    time(&td);
    printf("\n---- Time ---- %s", ctime(&td));

    sprintf(msg, "---- Time ---- %s \n", ctime(&td));
    PRMSG(stdout, msg);
#endif

    
#if 0
// xxxxxxxxxxxxx MM
        printf("Checking for unused variables ...\n");
        int clt_idx, found;
        for ( clt_idx=0; clt_idx<=num_var-1; clt_idx++ ) {
            found = 0;
            for ( rtp_idx=0; (rtp_idx<num_insys_rtp)&&(!found); rtp_idx++ ) {
                if (mmx_a[rtp_idx][clt_idx] != 0.0) {
                    found = 1;
                }
            }

            if (!found) {
                printf("UNUSED VARIABLE %d\n", clt_idx);
            }
        }
#endif

#if 0
        char fn_str[100];
        std::ofstream dbg_clt_reg;
        sprintf( fn_str, "%s%cdbg_clt_reg.txt", wisim_home, FPATH_SEPARATOR );
        dbg_clt_reg.open( fn_str );
        if( !dbg_clt_reg)
            printf( "File \"%s\" not exist.\n", fn_str );

        ((ClutterPropModelFullClass*) pm)->clt_fit_line( m_slope, m_intercept );
        for ( clutter_idx=0; clutter_idx<pm->num_clutter_type; clutter_idx++ ) {
           dbg_clt_reg //<< "Clutter " << std::setw(3) << clutter_idx << " Coeffecient & Constant : "
                       << std::setprecision(5) << std::setw(12)
                       << ((ClutterPropModelFullClass*) pm)->mvec_x[2*clutter_idx+((ClutterPropModelFullClass*) pm)->useheight]
                       << std::setprecision(5) << std::setw(12)
                       << ((ClutterPropModelFullClass*) pm)->mvec_x[2*clutter_idx+((ClutterPropModelFullClass*) pm)->useheight+1] << std::endl;
        }
        dbg_clt_reg << std::endl
                    << std::setprecision(5) << std::setw(12)
                    << m_slope
                    << std::setprecision(5) << std::setw(12)
                    << m_intercept << std::endl;

        dbg_clt_reg.close();
#endif

#if 0
// xxxxxxxxxxxxx MM
        int clt_idx;
        double product;
        for ( rtp_idx=0; rtp_idx<=num_insys_rtp-1; rtp_idx++ ) {
            product = 0.0;
            printf("MATRIX VALUES FOR RTP %d\n", rtp_idx);
            for ( clt_idx=0; clt_idx<=num_var-1; clt_idx++ ) {
                printf("A[%d][%d] = %15.12f\n", rtp_idx, clt_idx, mmx_a[rtp_idx][clt_idx]);
                if ((clt_idx==0) && (useheight)) {
                    product += 0.0*mmx_a[rtp_idx][clt_idx];
                } else if ((clt_idx - useheight) % 2 == 0) {
                    product += -30.0*mmx_a[rtp_idx][clt_idx];
                } else {
                    product += 41.9382*mmx_a[rtp_idx][clt_idx];
                }
            }
            printf("B[%d] = %15.12f\n", rtp_idx, vec_b[rtp_idx]);
            printf("PRODUCT = %15.12f\n", product);
            printf("ERROR   = %15.12f\n", product - vec_b[rtp_idx]);
            printf("\n");
        }
#endif

    rms = sqrt(error/num_insys_rtp);

#if CLUTTER_DEBUG
    clutter_sim_result << "TOTAL SQUARE ERROR : " << error << std::endl;
    clutter_sim_result << "RMS ERROR : " << rms << std::endl << std::endl;
#endif

    if ( scope == Individual ) { 
        sprintf(msg, "SCANNING CS INDEX %d  ... \n", (*scan_index_list)[0]);
        PRMSG(stdout, msg);
    }
    sprintf(msg, "TOTAL SQUARE ERROR = %15.10f \n", error);
    PRMSG(stdout, msg);
    sprintf(msg, "RMS ERROR = %15.10f \n \n", sqrt(error/num_insys_rtp));
    PRMSG(stdout, msg);
    
#if 0
xxxxxxxxx DELETE MFK
    if ( m_type == CConst::PropClutterSimp ) {
        int uu(0), zz(0), dd(0);
        ((ClutterPropModelClass*) pm)->clt_statistic( uu, zz, dd );

        // Signed clutters with special color by coefficient levels.
        ((ClutterPropModelClass*) pm)->clt_distribute( this );
        

#if CLUTTER_DEBUG
        if( ((ClutterPropModelClass*) pm)->useheight ) {
            clutter_coeff << "Antenna Height Coeffecient:   " << std::setprecision(5) << std::setw(12) 
                          << ((ClutterPropModelClass*) pm)->mvec_x[0] << std::endl;
        }
        
        for ( clutter_idx=0; clutter_idx<pm->num_clutter_type; clutter_idx++ ) {
            clutter_coeff << "Clutter " << std::setw(3) << clutter_idx << " Coeffecient: " 
                << std::setprecision(5) << std::setw(12) 
                << ((ClutterPropModelClass*) pm)->mvec_x[clutter_idx+((ClutterPropModelClass*) pm)->useheight] << std::endl;
        }
#endif
    }
    else if ( m_type == CConst::PropClutterFull ) {

        // Signed clutters with special color by coefficient levels.
        ((ClutterPropModelFullClass*) pm)->clt_distribute( this );


#if CLUTTER_DEBUG
        if (((ClutterPropModelFullClass*) pm)->useheight) {
            clutter_coeff << "Antenna Height Coeffecient:   " << std::setprecision(5) << std::setw(12) 
                          << ((ClutterPropModelFullClass*) pm)->mvec_x[0] << std::endl;
        }

        for ( clutter_idx=0; clutter_idx<pm->num_clutter_type; clutter_idx++ ) {
            clutter_coeff 
                //                    << "Clutter " << std::setw(3) << clutter_idx << " Coeffecient & Constant : " 
                << std::setprecision(5) << std::setw(12) 
                << ((ClutterPropModelFullClass*) pm)->mvec_x[2*clutter_idx+((ClutterPropModelFullClass*) pm)->useheight] 
                << std::setprecision(5) << std::setw(12) 
                << ((ClutterPropModelFullClass*) pm)->mvec_x[2*clutter_idx+((ClutterPropModelFullClass*) pm)->useheight+1] << std::endl;
        }
#endif
        
        // Fit a line 'y = slope*x + intercept' with clutter coeffecient(X-asix) and constant data(Y-asix).
        ((ClutterPropModelFullClass*) pm)->clt_fit_line( m_slope, m_intercept );
        ((ClutterPropModelFullClass*) pm)->fit_k = m_slope;
        ((ClutterPropModelFullClass*) pm)->fit_b = m_intercept;
        
#if CLUTTER_DEBUG
        clutter_coeff << std::setw(10) << m_slope << std::setw(10) << m_intercept << std::endl;
#endif
    }
    else if ( m_type == CConst::PropClutterSymFull ) {

        // Signed clutters with special color by coefficient levels.
        ((ClutterSymFullPropModelClass*) pm)->clt_distribute( this );

        // Fit a line 'y = slope*x + intercept' with clutter coeffecient(X-asix) and constant data(Y-asix).
        ((ClutterSymFullPropModelClass*) pm)->clt_fit_line( m_slope, m_intercept );
        ((ClutterSymFullPropModelClass*) pm)->fit_k = m_slope;
        ((ClutterSymFullPropModelClass*) pm)->fit_b = m_intercept;
    }
    else if ( m_type == CConst::PropClutterWtExpo ) {

        // Signed clutters with special color by coefficient levels.
        ((ClutterWtExpoPropModelClass*) pm)->clt_distribute( this );

        // Fit a line 'y = slope*x + intercept' with clutter coeffecient(X-asix) and constant data(Y-asix).
        ((ClutterWtExpoPropModelClass*) pm)->clt_fit_line( m_slope, m_intercept );
        ((ClutterWtExpoPropModelClass*) pm)->fit_k = m_slope;
        ((ClutterWtExpoPropModelClass*) pm)->fit_b = m_intercept;
    }
#endif

    if ( scope == Individual ) { 
        sprintf(msg, "SCANNING CS INDEX %d  ... \n", (*scan_index_list)[0]);
        PRMSG(stdout, msg);
    }

#if CLUTTER_DEBUG
    clutter_coeff << "TOTAL SQUARE ERROR = " << error << std::endl;
    clutter_coeff << "RMS ERROR = " << sqrt(error/num_insys_rtp) << std::endl;
    clutter_coeff << std::endl;
#endif

#if 0
    clutter_sim_result << "Number of clutters that coeffecient larger than zero  " << uu << std::endl;
    clutter_sim_result << "Number of clutters that coeffecient equal to zero  "    << zz << std::endl;
    clutter_sim_result << "Number of clutters that coeffecient less than zero  "   << dd << std::endl;
#endif

    num_prop_model++;
    prop_model_list = (PropModelClass **) realloc((void *) prop_model_list, (num_prop_model)*sizeof(PropModelClass *));
    prop_model_list[num_prop_model-1] = (PropModelClass *) pm;


    //if type equal Global allocate the computed propagation model for each cell, and otherwise .
    if ( scope == Global ) {
        int prop_model_idx = 0;
        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                sector->prop_model = num_prop_model-1;
            }
            prop_model_idx ++;
        }
    } else if ( scope == Individual ) {
        for ( i=0; i<=scan_index_list->getSize()-1; i++ ) {
            scan_idx = (*scan_index_list)[i];
            cell_idx = scan_idx & ((1<<bit_cell)-1);
            sector_idx = scan_idx >> bit_cell;

            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                sector->prop_model = num_prop_model-1;
            }
        }
    } 

#if OUTPUT_TIME_INFOR
    time(&td);
    printf("\n---- Time ---- %s", ctime(&td));

    sprintf(msg, "---- Time ---- %s \n", ctime(&td));
    PRMSG(stdout, msg);
#endif

    for ( rtp_idx=0; rtp_idx<num_insys_rtp; rtp_idx++ ) {
        free (mmx_a[rtp_idx]); 
    }
    free( mmx_a );
    free( a_posn );
    free( vec_b );

    delete map_bdy;
}
/******************************************************************************************/
