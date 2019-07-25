/******************************************************************************************/
/**** FILE: check_road_test_data.cpp                                                   ****/
/******************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <vector>

#include "road_test_data.h"
#include "wisim.h"
#include "list.h"
#include "pref.h"
#include "phs.h"
#include "antenna.h"

#if HAS_GUI
#include "cs_lonlat_error_dialog.h"
#include "cs_ll_err_option_dialog.h"
#include <q3ptrlist.h>
#include <q3strlist.h>
#include <q3progressdialog.h>
#include <qapplication.h>
#include <qpushbutton.h>
#endif

using namespace std;

typedef vector<int> VEC_INT; 
typedef vector<long> VEC_LONG;

/******************************************************************************************/
/**** FUNCTION: NetworkClass::check_road_test_data                                     ****/
/******************************************************************************************/

void angle_range( ListClass<double>* angle, double rate, double& start_a, double& end_a );
int in_angle_range( double angle, double start_a, double end_a );
void add_error_cell_vector( VEC_LONG& new_num_vec, VEC_LONG& new_desc_vec,
                            vector<VEC_LONG>& dest_vec, unsigned& num_elem, int p );



#if HAS_GUI
void NetworkClass::check_road_test_data()
{
    CsLonlatErrorOptionDialog *pDlg = new CsLonlatErrorOptionDialog();
    pDlg->np = this;
    //pDlg->show();
    pDlg->accept();

    return;
}
#endif




          
/*******************example*****************/
/*
    int rtd_idx, cell_idx, sector_idx;
    double pwr_db;
    SectorClass *sector;
    RoadTestPtClass *rtp;

    char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
    char *gwcsccsstr = CVECTOR(6);

    printf("Check road test data\n");

    for (rtd_idx=0; rtd_idx<=road_test_data_list->getSize()-1; rtd_idx++) {
        rtp = &( (*road_test_data_list)[rtd_idx] );
        cell_idx = rtp->cell_idx;
        sector_idx = rtp->sector_idx;
        sector = cell_list[cell_idx]->sector_list[sector_idx];
        pwr_db = rtp->pwr_db;

        hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
        sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);

        fprintf(stdout, "(%d) %d %s %s %7.3f\n", rtd_idx, cell_idx, hexstr, gwcsccsstr, pwr_db - preferences->pwr_offset);
    }

    free(hexstr);
    free(gwcsccsstr);
*/

/******************************************************************************************/
