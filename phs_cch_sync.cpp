/******************************************************************************************/
/**** PROGRAM: phs_cch_sync.cpp                                                        ****/
/**** Michael Mandell                                                                  ****/
/******************************************************************************************/
/**** Simulate CCH assignment and synchronization in PHS system.                       ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "phs.h"
#include "antenna.h"
#include "prop_model.h"
#include "road_test_data.h"
#include "randomc.h"

#if DEMO
    void PHSNetworkClass::read_cch_rssi_table(char *filename) { }
    void PHSNetworkClass::run_system_sync() {}
    void PHSNetworkClass::comp_sync_order(int *sector_list, int num_sector) {}
    void PHSNetworkClass::expand_cch_rssi_table(double threshold_db) {}
    void PHSNetworkClass::print_sync_state(char *filename) {}
    void PHSNetworkClass::write_cch_rssi_table(char *filename) {}
#else
/******************************************************************************************/
/**** FUNCTION: read_cch_rssi_table                                                    ****/
/**** Open specified file and read CCH RSSI table.                                     ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void PHSNetworkClass::read_cch_rssi_table(char *filename)
{
    int cell_idx, sector_idx;
    int table_idx = -1;
    char *line, *str1;
    double tmpd;
    FILE *fp;

    line = CVECTOR(MAX_LINE_SIZE);

    if (cch_rssi_table) {
        sprintf(msg, "ERROR: CCH RSSI table already defined\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file 7 %s\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    sprintf(msg, "Reading CCH RSSI table file: %s\n", filename);
    PRMSG(stdout, msg);

#define STATE_TABLE_SIZE              0
#define STATE_TABLE_ENTRY             1
#define STATE_DONE                    2

    int state = STATE_TABLE_SIZE;

    cch_rssi_table = (CchRssiTableClass *) malloc(sizeof(CchRssiTableClass));
    CchRssiTableClass *tbl = cch_rssi_table;
    unsigned char *csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length);

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_TABLE_SIZE:
                    if (strcmp(str1, "TABLE_SIZE:") != 0) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "Expecting \"TABLE_SIZE:\" NOT \"%s\"\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        tbl->table_size = atoi(str1);
                    } else {
                        tbl->table_size = 0;
                    }
                    if (tbl->table_size <= 0) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "table_size = %d must be > 0\n", filename, tbl->table_size);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    table_idx = 0;
                    tbl->sector_rx = IVECTOR(tbl->table_size);
                    tbl->sector_tx = IVECTOR(tbl->table_size);
                    tbl->rssi      = DVECTOR(tbl->table_size);
                    state = STATE_TABLE_ENTRY;
                    break;
                case STATE_TABLE_ENTRY:
                    if ( strlen(str1) != 2*PHSSectorClass::csid_byte_length ) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "CSID: %s has improper length\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    uid_to_sector(str1, cell_idx, sector_idx);
                    tbl->sector_rx[table_idx] = (sector_idx << bit_cell) | cell_idx;
                    if (tbl->sector_rx[table_idx] == -1) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "CSID: %s not defined\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    if ( strlen(str1) != 2*PHSSectorClass::csid_byte_length ) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "CSID: %s has improper length\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    uid_to_sector(str1, cell_idx, sector_idx);
                    tbl->sector_tx[table_idx] = (sector_idx << bit_cell) | cell_idx;
                    if (tbl->sector_tx[table_idx] == -1) {
                        sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n"
                                         "CSID: %s not defined\n", filename, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, CHDELIM);
                    tmpd = atof(str1);
                    tbl->rssi[table_idx] = exp(tmpd * log(10.0)/10.0);

                    table_idx++;
                    if (table_idx == tbl->table_size) {
                        state = STATE_DONE;
                    } else {
                        state = STATE_TABLE_ENTRY;
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid CCH RSSI file \"%s\"\n", filename);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
                default:
                    sprintf(msg, "ERROR in routine read_cch_rssi_table()\n"
                                     "Invalid state (%d) encountered\n", state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }
#undef STATE_TABLE_SIZE
#undef STATE_TABLE_ENTRY
#undef STATE_DONE

    free(line);
    free(csid_hex);

    fclose(fp);
    return;
}


/*
**************************************************************************************
* FUNCTION: run_system_sync
* Run PHS system synchronization algoththm to assign sync_level and CCH to all
* sectors in the system.
**************************************************************************************
*/
void PHSNetworkClass::run_system_sync()
{
    int        i;                      //loop variable
    char       *chptr;                 //error report string

    int        num_sector;             //total sector number in network
    int        *sector_list;           //sector list in network

    int        cell_idx;               //cell index for synchronizing cell
    int        sector_idx;             //sector index for synchronizing cell
    int        i_cell_idx;             //cell index for synchronizied cell
    int        i_sector_idx;           //sector index for synchronizied cell

    int        sync_level;             //cell synchronization level
    int        cch_slot;               //the cch_slot number of a synchronized cell
    int        *visible_sector;        //for a synchronizing cell,visible synchronizied sector

    CellClass        *cell;            //synchronizing cell pointer
    PHSSectorClass   *sector;          //synchronizing sector pointer
    PHSSectorClass   *i_sector;        //synchronized sector pointer

    CchRssiTableClass *tbl = cch_rssi_table;

//    cch_allocation_mode= 1;
//    cch_selection_mode = 1;

    if (!tbl) {
        chptr = msg;
        chptr += sprintf(chptr, "ERROR in routine run_system_sync()\n");
        chptr += sprintf(chptr, "RSSI table not defined\n");
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    //Count the sector number in the network need to synchronization
    num_sector = 0;
    for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        num_sector += cell->num_sector;
    }
    sector_list = IVECTOR(num_sector);

    //synchronization level variable
    visible_sector = IVECTOR(max_sync_level+1);

    //sort the sector of the network need to be synchronization right now
    comp_sync_order(sector_list, num_sector);


    /*
    **********************************************************************************
    * synchronization the network and allcoate CCH slot sector by sector
    **********************************************************************************
    */
    for (int idx=0; idx<=num_sector-1; idx++) {
        //extract cell_idx and sector_idx from sector_list entry
        //then get responding sector information
        cell_idx   = sector_list[idx] & ((1<<bit_cell)-1);
        sector_idx = sector_list[idx] >> bit_cell;
        sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];

        /*
        ******************************************************************************
        * Assign sync level
        ******************************************************************************
        */
        if (sector->sync_level != 0) {
            //for every cell need to be synchronizied,set the visible sector to -1 as initilization
            for (sync_level=0; sync_level < max_sync_level+1; sync_level++) {
                visible_sector[sync_level] = -1;
            }

            //search the rssi table,found the synchronizied cell and get their synchronization level
            //for here only need to assign the synchronization level for the sector,so it does not
            //matter the rssi for different transmit cell.
            for (int table_idx = 0; table_idx <= tbl->table_size-1; table_idx++) {
                if (tbl->sector_rx[table_idx] == sector_list[idx]) {
                    if (tbl->rssi[table_idx] > sync_level_allocation_threshold) {
                        i_cell_idx   = tbl->sector_tx[table_idx] & ((1<<bit_cell)-1);
                        i_sector_idx = tbl->sector_tx[table_idx] >> bit_cell;
                        i_sector = (PHSSectorClass *) cell_list[i_cell_idx]->sector_list[i_sector_idx];
                        sync_level = i_sector->sync_level;

                        if (sync_level != -1) {
                            visible_sector[sync_level] = tbl->sector_tx[table_idx];
                        }
                    }
                }
            }

            //according to the visible sector,set the sector's synchronization level next to
            //the most propriety synchronization level in the visible sector variable.
            int found = 0;
            for (sync_level=0;sync_level< max_sync_level; sync_level++) {
                if (visible_sector[sync_level] != -1) {
                    sector->sync_level = sync_level + 1;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                sector->sync_level = max_sync_level;
            }
        }


        /*
        ******************************************************************************
        * After assign the synchronization level for the sector, allocate the CCH slot
        * to the sector
        ******************************************************************************
        */
        //First according to the RSSI table, get the slot witch the CCH signal is minimal
        //and the slot should divide to group for slot priority mode in allocating CCH.
        int       slot_a;          //minimal rssi signal slot in group A slot
        int       slot_b;          //minimal rssi signal slot in group B slot
        int       abs_min_slot;    //minimal rssi signal slot in all slot
        double    min_a;           //minimal rssi signal value in group A slot
        double    min_b;           //minimal rssi signal value in group B slot
        double    abs_min;         //minimal rssi signal value in all slot

        double    *slot_scan = DVECTOR(num_cntl_chan_slot);
        for (i=0; i<=num_cntl_chan_slot-1; i++) {
            slot_scan[i] = 0.0;
        }

        min_a = 0.0;
        min_b = 0.0;
        slot_a = -1;
        slot_b = -1;

        //statistic the rssi signal for each cch slot according to rssi table
        for (int table_idx = 0; table_idx <= tbl->table_size-1; table_idx++) {
            if (tbl->sector_rx[table_idx] == sector_list[idx]) {
                i_cell_idx   = tbl->sector_tx[table_idx] & ((1<<bit_cell)-1);
                i_sector_idx = tbl->sector_tx[table_idx] >> bit_cell;
                i_sector = (PHSSectorClass *) cell_list[i_cell_idx]->sector_list[i_sector_idx];
                cch_slot = i_sector->cntl_chan_slot;
                slot_scan[cch_slot] += tbl->rssi[table_idx];
            }
        }

        //get the minimal value as following
        for (i=0; i<=num_cntl_chan_slot-1; i++) {
            //Group A slot,that is slot 1 and slot 4 in the four slot 1,2,3,4
            if ( i%4 == 0 || i%4 == 3 ) {
                if ( (slot_a==-1) || (slot_scan[i] < min_a) ) {
                    slot_a = i;
                    min_a = slot_scan[i];
                }
            } else {//Group B slot,that is slot 2 and slot 3 in the four slot 1,2,3,4
                /* group B */
                if ( (slot_b==-1) || (slot_scan[i] < min_b) ) {
                    slot_b = i;
                    min_b = slot_scan[i];
                }
            }
        }

        if (min_a <= min_b) {
            abs_min_slot = slot_a;
            abs_min = min_a;
        } else {
            abs_min_slot = slot_b;
            abs_min = min_b;
        }

        //According to the allocation mode and the cch slot signal information,
        //allocate the cch slot to the synchronizing cell.
        if ( (cch_allocation_mode == NOT_FORCED) && (abs_min > cch_allocation_threshold) ) {
            sector->cntl_chan_slot = -1;
        } else if (cch_selection_mode == LEVEL_PRIORITY) {
            sector->cntl_chan_slot = abs_min_slot;
        } else {
            if (min_a <= cch_allocation_threshold) {
                sector->cntl_chan_slot = slot_a;
            } else {
                sector->cntl_chan_slot = slot_b;
            }
        }

        free(slot_scan);
    }

    free(sector_list);
    free(visible_sector);
    return;
}


/*
**************************************************************************************
* FUNCTION: comp_sync_order
* According to the synchronization and cs_num compute the sector synchronization order
* If the sector's sync_level and cs_num is same, the order of these sectors are random
**************************************************************************************
*/
void PHSNetworkClass::comp_sync_order(int *sector_list, int num_sector)
{
    int cell_idx, sector_idx;
    int cs_num;
    int *index_list;
    CellClass *cell;
    PHSSectorClass *sector;

    index_list = IVECTOR(num_sector);

    int j = 0;
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            cs_num = sector->gw_csc_cs%100;
            sector_list[j] = (sector_idx << bit_cell) | cell_idx;

            //if sector sync level is unsigned,set it to max_sync_level,
            //and index_list could order by sync_level,master is first
            if( sector->sync_level == -1 ) {
                index_list[j] = (((max_sync_level+1)<<5) | cs_num);
            } else {
                index_list[j] = ((sector->sync_level<<5) | cs_num);
            }
            j++;
        }
    }

    //sort index_list and sector_list according to index_list content
    sort2z(num_sector, index_list, sector_list);


    /* Randomize sectors with same sync_level and cs_num */
    int tmpi;
    int i0 = 0;
    int i1 = 0;
    while(i0 <= num_sector-1) {
        while( (i1<=num_sector-1) && (index_list[i1]==index_list[i0]) ) {
            i1++;
        }

        int n = i1-i0;
        if (n >= 2) {
            for (int i=0; i<=n-2; i++) {
                // int r = (int) floor(ran3(&seed, 0)*(n-i));
                int r = (int) floor(rg->Random()*(n-i));
                /* swap indices (i0 + i) and  (i0 + i + r) */
                tmpi                    = sector_list[i0 + i];
                sector_list[i0 + i]     = sector_list[i0 + i + r];
                sector_list[i0 + i + r] = tmpi;
            }
        }

        i0 = i1;
    }

    free(index_list);

    return;
}


/*
**************************************************************************************
* According to the RSSI table file, compute the propagate model of the CCH.
* and generate a expand RSSI table file according to a given threshold for WiSim.
**************************************************************************************
*/
void PHSNetworkClass::expand_cch_rssi_table(double threshold_db)
{
#if 0
    int i;                 //loop variable
    int j;                 //loop variable
    int m;                 //loop variable
    int n;                 //loop variable
    int table_idx;         //loop variable
    int table_size;        //table size for new rssi file

    int rx_cell_found;     //flag of rx cell found in old rssi file
    int tx_cell_found;     //flag of tx cell found in old rssi file

    int num_rssi_table;    //number of rssi table
    int rx_cell_idx;       //index for receive cell
    int rx_sector_idx;     //index for receive cell's sector
    int tx_cell_idx;       //index for transmit cell
    int tx_sector_idx;     //index for transmit cell's sector

    double dx,dy,dz;       //distance between tx and rx cell in x orientation and y orientation
    double logd;           //geometry distance between rx and tx cell
    double power;          //power according to the propagation model
    double gtx_db,grx_db;  //antenna gain of tx and rx cell
    double min_height;     //min height of Rx and Tx cell antenna

    double *pwr_db;        //power vector for rssi
    double *vec_logd;      //distance vector for rssi
    double **mx_a;         //matrix A for the first step, Mx3
    double *vec_x;         //vector x for the first step, 3x1:val_py,s1,s2

    double cch_max_gain;   //max power gain of CCH propagation model
    double cch_threshold;  //threshold of CCH propagation model

    int pre_rx_cell_idx;             //previous idx of rx cell
    int num_rx_cell;                 //number of rx cell in rssi table
    int num_tx_cell;                 //number of tx cell in rssi table
    int rx_cell_num;                 //number of rx cell in rssi table
    RxCellTableClass  **rctable; //all the rx cell idx in the rssi table

    CchRssiTableClass *tbl;          //original rssi table
    CchRssiTableClass *extbl;        //expand rssi table
    // TerrainPropModelClass *tpmodel;      //terrain prop model structure.

    PHSSectorClass *sector;

    /*
    *********************************************************************************
    * set value to some variable
    *********************************************************************************
    */
    tbl             = cch_rssi_table;
    num_rssi_table  = tbl->table_size;
    rx_cell_idx     = 0;
    rx_sector_idx   = 0;
    tx_cell_idx     = 0;
    tx_sector_idx   = 0;

    /*
    *********************************************************************************
    * Allocate the vector variable according to the rssi table size
    *********************************************************************************
    */
    // tpmodel = new TerrainPropModelClass(map_clutter);

    pwr_db   = DVECTOR(num_rssi_table);
    vec_logd = DVECTOR(num_rssi_table);

    mx_a = (double **) malloc(num_rssi_table*sizeof(double *));
    for (i=0; i<num_rssi_table; i++) {
        mx_a[i] = DVECTOR(3);
    }

    vec_x = DVECTOR(3);

    for (i=0; i<num_rssi_table; i++) {
        mx_a[i][0]  = 1.0;  //first column always is one.
        mx_a[i][1]  = 0.0;  //second column initial to zero.
        mx_a[i][2]  = 0.0;  //third column initial to zero.
        vec_logd[i] = 0.0;  //the vector is initial to zero too.
        pwr_db[i]   = 0.0;  //the vector is initial to zero too.
    }


    /*
    *********************************************************************************
    * According to the rssi table read by WiSim, get the power attenuation with
    * the distance. Through singular value discomposition get the propagation model
    * of CCH.
    *********************************************************************************
    */
    min_height = -1.0;
    for(table_idx=0; table_idx<num_rssi_table; table_idx++) {
        rx_cell_idx   = tbl->sector_rx[table_idx] & ((1<<bit_cell)-1);
        rx_sector_idx = tbl->sector_rx[table_idx] >> bit_cell;

        tx_cell_idx   = tbl->sector_tx[table_idx] & ((1<<bit_cell)-1);
        tx_sector_idx = tbl->sector_tx[table_idx] >> bit_cell;

        dx = (double) cell_list[rx_cell_idx]->posn_x - cell_list[tx_cell_idx]->posn_x;
        dy = (double) cell_list[rx_cell_idx]->posn_y - cell_list[tx_cell_idx]->posn_y;
        dz = (double) cell_list[rx_cell_idx]->sector_list[rx_sector_idx]->antenna_height
                    - cell_list[tx_cell_idx]->sector_list[tx_sector_idx]->antenna_height;

        if ( (min_height == -1.0) || (fabs(dz)<min_height) ) {
            min_height = fabs(dz);
        }

        double tx_antenna_angle = cell_list[tx_cell_idx]->sector_list[tx_sector_idx]->antenna_angle_rad;
        double rx_antenna_angle = cell_list[rx_cell_idx]->sector_list[rx_sector_idx]->antenna_angle_rad;

        AntennaClass *tx_antenna = antenna_type_list[cell_list[tx_cell_idx]->sector_list[tx_sector_idx]->antenna_type];
        AntennaClass *rx_antenna = antenna_type_list[cell_list[rx_cell_idx]->sector_list[rx_sector_idx]->antenna_type];

        gtx_db = tx_antenna->gainDB( dx,  dy,  dz, tx_antenna_angle);
        grx_db = rx_antenna->gainDB(-dx, -dy, -dz, rx_antenna_angle);

        pwr_db[table_idx] = 10*log(tbl->rssi[table_idx])/log(10.0) - gtx_db - grx_db;

        vec_logd[table_idx] = log(sqrt(dx*dx+dy*dy+dz*dz))/log(10.0);
    }

    if ( min_height<=0 ) {
        sprintf(msg, "ERROR: Antenna height less than 0\n");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    /*
    **********************************************************************************
    * Use singluar value decomposition get the propagation model of CCH
    **********************************************************************************
    */
    //fit_data(tpmodel,num_rssi_table,mx_a,pwr_db,vec_x,vec_logd);
    // tpmodel->fit_data(num_rssi_table,mx_a,pwr_db,vec_x,vec_logd,min_height);

    // cch_max_gain      = tpmodel->val_y - tpmodel->val_y*tpmodel->val_s1;
    cch_threshold     = cch_max_gain - threshold_db;

//     printf("The propagation model of CCH is :\n");
//     printf("tpmodel->y  = %f:\n",tpmodel->val_y);
//     printf("tpmodel->py = %f:\n",tpmodel->val_py);
//     printf("tpmodel->s1 = %f:\n",tpmodel->val_s1);
//     printf("tpmodel->s2 = %f:\n",tpmodel->val_s2);
    printf("CCH max gain= %f:\n",cch_max_gain);
    printf("Threshold   = %f:\n",cch_threshold);

    free(vec_logd);
    free(vec_x);

    //clear memory
    for (i=0; i<num_rssi_table; i++) {
         free(mx_a[i]);
    }
    free(mx_a);

    /*
    **********************************************************************************
    * Construct a data structure of tx cell and rx cell, prepare for the complete
    * RSSI file generation.
    **********************************************************************************
    */
    //count number of rx cell in rssi table and put them into a rx cell table
    num_rx_cell = 0;
    pre_rx_cell_idx = -1;

    for ( table_idx=0; table_idx<num_rssi_table; table_idx++) {

        rx_cell_idx   = tbl->sector_rx[table_idx] & ((1<<bit_cell)-1);

        if ( rx_cell_idx != pre_rx_cell_idx ) {
             num_rx_cell = num_rx_cell + 1;
        }

        pre_rx_cell_idx = rx_cell_idx;
    }

    //according to the number of the rx cell,allocate the memory to the rx_cell_table
    rx_cell_num = num_rx_cell;
    rctable = (RxCellTableClass **) malloc(rx_cell_num*sizeof(RxCellTableClass *));
    for(i=0;i<rx_cell_num;i++) {
       rctable[i] = (RxCellTableClass *) malloc(sizeof(RxCellTableClass));
    }


    //count the tx cell number for each rx cell,and allocate memory for it's tx cell table
    num_rx_cell = 0;
    num_tx_cell = 1;
    pre_rx_cell_idx = -1;
    for ( table_idx=0; table_idx<num_rssi_table; table_idx++) {

        rx_cell_idx   = tbl->sector_rx[table_idx] & ((1<<bit_cell)-1);

        if ( rx_cell_idx == pre_rx_cell_idx ) {
            num_tx_cell = num_tx_cell + 1;
        } else {
            if( num_rx_cell > 0 ) {
                rctable[num_rx_cell-1]->rx_cell_idx   = pre_rx_cell_idx;
                rctable[num_rx_cell-1]->num_tx_cell   = num_tx_cell;
                num_tx_cell = 1;
            }
            num_rx_cell = num_rx_cell + 1;
        }

        //last cell
        if(table_idx==num_rssi_table-1) {
            rctable[num_rx_cell-1]->rx_cell_idx   = rx_cell_idx;
            rctable[num_rx_cell-1]->num_tx_cell   = num_tx_cell;
        }

        pre_rx_cell_idx = rx_cell_idx;
    }

    //allocate memory for tx_cell of each rx cell
    for( i=0;i<rx_cell_num;i++) {
        rctable[i]->tx_cell_table = (int *) malloc(rctable[i]->num_tx_cell*sizeof(int));
        rctable[i]->pwr_db        = (double *) malloc(rctable[i]->num_tx_cell*sizeof(double));
    }

    //create the tx cell table for each rx cell
    num_rx_cell = 0;
    num_tx_cell = 1;
    pre_rx_cell_idx = -1;
    for ( table_idx=0; table_idx<num_rssi_table; table_idx++) {

        rx_cell_idx   = tbl->sector_rx[table_idx] & ((1<<bit_cell)-1);
        tx_cell_idx   = tbl->sector_tx[table_idx] & ((1<<bit_cell)-1);

        if ( rx_cell_idx == pre_rx_cell_idx ) {
            num_tx_cell = num_tx_cell + 1;
        } else {
            num_rx_cell = num_rx_cell + 1;
            num_tx_cell = 1;
        }

        if( num_rx_cell > 0 ) {
            rctable[num_rx_cell-1]->tx_cell_table[num_tx_cell-1] = tx_cell_idx;
            rctable[num_rx_cell-1]->pwr_db[num_tx_cell-1]        = pwr_db[table_idx];
        }

        pre_rx_cell_idx = rx_cell_idx;

    }

    //clear memory
    free(pwr_db);

    /*
    **********************************************************************************
    * Use the cch propagation model, compute the power attenuation from every cell
    * every sector to every other cell every other sector.
    **********************************************************************************
    */

    /*
    **********************************************************************************
    * Count the size of new rssi file
    **********************************************************************************
    */
    table_size = 0;
    for(i=0;i<num_cell;i++) {  //rx cell loop for new RSSI file
        //judge if there are same transmit cell and receive cell but different transmit
        //direction in old RSSI file.
        for(j=0;j<i;j++) {//tx cell loop for new RSSI file
            //if tx cell "j" in rx_cell_list of old RSSI file
            rx_cell_found = 0;
            for(m=0;m<rx_cell_num;m++) {
                if( rctable[m]->rx_cell_idx == j ) {
                     num_tx_cell   = rctable[m]->num_tx_cell;
                     rx_cell_idx   = m;
                     rx_cell_found = 1;
                     break;
                }
            }

            //if rx cell "i" in tx_cell_list of old RSSI file
            if(rx_cell_found == 1) {
                 tx_cell_found = 0;
                 for( n=0;n<num_tx_cell;n++) {
                     if( rctable[rx_cell_idx]->tx_cell_table[n] == i ) {
                         tx_cell_idx    = n;
                         tx_cell_found  = 1;
                         break;
                     }
                 }
             }

             if(rx_cell_found==1 && tx_cell_found==1) {
                 if( rctable[rx_cell_idx]->pwr_db[tx_cell_idx] > cch_threshold ) {
                     table_size = table_size + 1;
                 }
             } else {
                 dx = cell_list[i]->posn_x - cell_list[j]->posn_x;
                 dy = cell_list[i]->posn_y - cell_list[j]->posn_y;
                 logd = log(sqrt(dx*dx+dy*dy))/log(10.0);

#if 0
                 if( logd>=tpmodel->val_y ) {
                     power = tpmodel->val_py + tpmodel->val_s2*(logd - tpmodel->val_y);
                 } else {
                     power = tpmodel->val_py + tpmodel->val_s1*(logd - tpmodel->val_y);
                 }
#endif

                 if( power > cch_threshold )
                 {
                     table_size = table_size + 1;
                 }

             }
        }


        //judge if there are same transmit cell and receive cell and same transmit
        //direction in old RSSI file.
        for(j=i+1;j<num_cell;j++) {//tx cell loop for new RSSI file
            //if rx cell "i" in rx_cell_list of old RSSI file
            rx_cell_found = 0;
            for(m=0;m<rx_cell_num;m++) {
                if(rctable[m]->rx_cell_idx == i ) {
                    num_tx_cell   = rctable[m]->num_tx_cell;
                    rx_cell_idx   = m;
                    rx_cell_found = 1;
                    break;
                }
            }

            //if tx cell "j" in tx_cell_list of old RSSI file
            if(rx_cell_found==1) {
                tx_cell_found = 0;
                for(n=0;n<num_tx_cell;n++) {
                    if( rctable[rx_cell_idx]->tx_cell_table[n] == j ) {
                        tx_cell_idx   = n;
                        tx_cell_found = 1;
                        break;
                    }
                }
            }

            if(rx_cell_found==1 && tx_cell_found==1) {
                if( rctable[rx_cell_idx]->pwr_db[tx_cell_idx] > cch_threshold ) {
                     table_size = table_size + 1;
                }
            } else {
                dx = cell_list[i]->posn_x - cell_list[j]->posn_x;
                dy = cell_list[i]->posn_y - cell_list[j]->posn_y;
                logd = log(sqrt(dx*dx+dy*dy))/log(10.0);

#if 0
                if( logd>=tpmodel->val_y ) {
                    power = tpmodel->val_py + tpmodel->val_s2*(logd - tpmodel->val_y);
                } else {
                    power = tpmodel->val_py + tpmodel->val_s1*(logd - tpmodel->val_y);
                }
#endif

                if( power > cch_threshold ) {
                     table_size = table_size + 1;
                }
            }
        }
    }

    //clear memory of old cch rssi table
    clear_memory(cch_rssi_table);

    /*
    **********************************************************************************
    * Allocate memory for new rssi data structure
    **********************************************************************************
    */
    cch_rssi_table = (CchRssiTableClass *) malloc(sizeof(CchRssiTableClass));
    extbl              = cch_rssi_table;
    extbl->table_size  = table_size;
    extbl->sector_rx   = IVECTOR(extbl->table_size);
    extbl->sector_tx   = IVECTOR(extbl->table_size);
    extbl->rssi        = DVECTOR(extbl->table_size);

    /*
    **********************************************************************************
    * According the cch propagation model, create each new rssi file entry
    **********************************************************************************
    */
    table_idx = 0;
    for(i=0;i<num_cell;i++) {//rx cell loop for new RSSI file
        //judge if there are same transmit cell and receive cell but different transmit
        //direction in old RSSI file.
        for(j=0;j<i;j++) {//tx cell loop for new RSSI file
            //if tx cell "j" in rx_cell_list of old RSSI file
            rx_cell_found = 0;
            for(m=0;m<rx_cell_num;m++) {
                if( rctable[m]->rx_cell_idx == j ) {
                     num_tx_cell   = rctable[m]->num_tx_cell;
                     rx_cell_idx   = m;
                     rx_cell_found = 1;
                     break;
                }
            }

            //if rx cell "i" in tx_cell_list of old RSSI file
            if(rx_cell_found == 1) {
                 tx_cell_found = 0;
                 for( n=0;n<num_tx_cell;n++) {
                     if( rctable[rx_cell_idx]->tx_cell_table[n] == i ) {
                         tx_cell_idx    = n;
                         tx_cell_found  = 1;
                         break;
                     }
                 }
             }

             if(rx_cell_found==1 && tx_cell_found==1) {
                 if( rctable[rx_cell_idx]->pwr_db[tx_cell_idx] > cch_threshold ) {
                     extbl->sector_rx[table_idx] = i;
                     extbl->sector_tx[table_idx] = j;
                     extbl->rssi[table_idx]      = rctable[rx_cell_idx]->pwr_db[tx_cell_idx];

                     table_idx = table_idx + 1;
                 }
             } else {
                 dx = cell_list[i]->posn_x - cell_list[j]->posn_x;
                 dy = cell_list[i]->posn_y - cell_list[j]->posn_y;
                 logd = log(sqrt(dx*dx+dy*dy))/log(10.0);

#if 0
                 if( logd>=tpmodel->val_y ) {
                     power = tpmodel->val_py + tpmodel->val_s2*(logd - tpmodel->val_y);
                 } else {
                     power = tpmodel->val_py + tpmodel->val_s1*(logd - tpmodel->val_y);
                 }
#endif

                 if( power > cch_threshold ) {
                     extbl->sector_rx[table_idx] = i;
                     extbl->sector_tx[table_idx] = j;
                     extbl->rssi[table_idx]      = power;

                     table_idx = table_idx + 1;
                 }

             }
        }

        //judge if there are same transmit cell and receive cell and same transmit
        //direction in old RSSI file.
        for(j=i+1;j<num_cell;j++)   //tx cell loop for new RSSI file
        {
            //if rx cell "i" in rx_cell_list of old RSSI file
            rx_cell_found = 0;
            for(m=0;m<rx_cell_num;m++) {
                if(rctable[m]->rx_cell_idx == i ) {
                    num_tx_cell   = rctable[m]->num_tx_cell;
                    rx_cell_idx   = m;
                    rx_cell_found = 1;
                    break;
                }
            }

            //if tx cell "j" in tx_cell_list of old RSSI file
            if(rx_cell_found==1) {
                tx_cell_found = 0;
                for(n=0;n<num_tx_cell;n++) {
                    if( rctable[rx_cell_idx]->tx_cell_table[n] == j ) {
                        tx_cell_idx   = n;
                        tx_cell_found = 1;
                        break;
                    }
                }
            }

            if(rx_cell_found==1 && tx_cell_found==1) {
                if( rctable[rx_cell_idx]->pwr_db[tx_cell_idx] > cch_threshold ) {
                     extbl->sector_rx[table_idx] = i;
                     extbl->sector_tx[table_idx] = j;
                     extbl->rssi[table_idx]      = rctable[rx_cell_idx]->pwr_db[tx_cell_idx];

                     table_idx = table_idx + 1;
                }
            } else {
                dx = cell_list[i]->posn_x - cell_list[j]->posn_x;
                dy = cell_list[i]->posn_y - cell_list[j]->posn_y;
                logd = log(sqrt(dx*dx+dy*dy))/log(10.0);

#if 0
                if( logd>=tpmodel->val_y ) {
                    power = tpmodel->val_py + tpmodel->val_s2*(logd - tpmodel->val_y);
                } else {
                    power = tpmodel->val_py + tpmodel->val_s1*(logd - tpmodel->val_y);
                }
#endif

                if( power > cch_threshold ) {
                     extbl->sector_rx[table_idx] = i;
                     extbl->sector_tx[table_idx] = j;
                     extbl->rssi[table_idx]      = power;

                     table_idx = table_idx + 1;
                }
            }
        }
    }


    /*
    **********************************************************************************
    * Free memory space of used data structure
    **********************************************************************************
    */
//    free(tpmodel);

    for(i=0;i<rx_cell_num;i++) {
       free(rctable[i]->pwr_db);
       free(rctable[i]->tx_cell_table);
       free(rctable[i]);
    }
    free(rctable);

#endif
}


/*
**************************************************************************************
* Write expand rssi table to file for resue
**************************************************************************************
*/
void PHSNetworkClass::write_cch_rssi_table(char *filename)
{
    int i;
    int cell_idx;
    int sector_idx;
    int table_size;

    PHSSectorClass *sector;

    char *hexstr;                 //csid hex string
    unsigned char *hex = NULL;    //csid hex
    unsigned int byte_len;        //byte length of the csid hex

    FILE *fprssi  = NULL;
    fprssi  = fopen(filename,"w");

    table_size = cch_rssi_table->table_size;
    CchRssiTableClass *extbl;
    extbl = cch_rssi_table;

    byte_len = PHSSectorClass::csid_byte_length;
    hexstr   = CVECTOR(2*PHSSectorClass::csid_byte_length);

    fprintf(fprssi,"#LINE FORMAT: csid_rx csid_tx pwr_dbuv\n\n");
    fprintf(fprssi,"TABLE_SIZE: %d\n",extbl->table_size);
    for(i=0;i<extbl->table_size;i++) {
        cell_idx   = extbl->sector_rx[i] & ((1<<bit_cell)-1);
        sector_idx = extbl->sector_rx[i] >> bit_cell;
        sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];
        hex = sector->csid_hex;
        hex_to_hexstr(hexstr,hex,byte_len);
        fprintf(fprssi,"%s ",hexstr);

        cell_idx   = extbl->sector_tx[i] & ((1<<bit_cell)-1);
        sector_idx = extbl->sector_tx[i] >> bit_cell;
        sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];
        hex = sector->csid_hex;
        hex_to_hexstr(hexstr,hex,byte_len);
        fprintf(fprssi,"%s ",hexstr);

        fprintf(fprssi,"%f\n",extbl->rssi[i]);
    }

    free(hexstr);
    fclose(fprssi);
}
/******************************************************************************************/
/**** FUNCTION: print_sync_state                                                       ****/
/**** Print system sync state: CCH and sync level for all sectors in the system        ****/
/******************************************************************************************/
void PHSNetworkClass::print_sync_state(char *filename)
{
    int cell_idx, sector_idx;
    char tmpstr[100], *chptr;
    CellClass *cell;
    PHSSectorClass *sector;
    FILE *fp;

    if (!filename) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    chptr = msg;
    chptr += sprintf(chptr, "SYNCHROZINATION STATE:\n");
    chptr += sprintf(chptr, "%15s%15s%15s\n", "SECTOR", "SYNC_LEVEL", "CCH");
    PRMSG(fp, msg);

    for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            chptr = msg;

            sprintf(tmpstr, "%d_%d", cell_idx, sector_idx);
            chptr += sprintf(chptr, "%15s", tmpstr);

            if (sector->sync_level == -1) {
                sprintf(tmpstr, "UNASSIGNED");
            } else {
                sprintf(tmpstr, "%d", sector->sync_level);
            }
            chptr += sprintf(chptr, "%15s", tmpstr);

            if (sector->cntl_chan_slot == -1) {
                sprintf(tmpstr, "UNASSIGNED");
            } else {
                sprintf(tmpstr, "%d", sector->cntl_chan_slot);
            }
            chptr += sprintf(chptr, "%15s", tmpstr);

            chptr += sprintf(chptr, "\n");
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
#endif
