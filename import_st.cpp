/******************************************************************************************/
/**** FILE: import_st.cpp                                                              ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>

#include "antenna.h"
#include "charstr.h"
#include "cconst.h"
#include "wisim.h"
#include "clutter_data_analysis.h"
#include "intint.h"
#include "list.h"
#include "map_clutter.h"
#include "phs.h"
#include "polygon.h"
#include "prop_model.h"
#include "road_test_data.h"
#include "utm_conversion.h"
#include "st_param.h"
#include "global_fn.h"

/******************************************************************************************/
/**** FUNCTION: NetworkClass::import_st_data                                           ****/
/******************************************************************************************/
void NetworkClass::import_st_data(int format, char *cscfile, char *filename, int period)
{
    int gwCscIdx, cell_idx, sector_idx;
    int Sum_St22, Sum_St21, Sum_St10, Sum_St18, Sum_St20;
    ListClass<CharStrClass> *ipStrList = new ListClass<CharStrClass>(0);
    ListClass<int>          *gwCscList = new ListClass<int>(0);
    CellClass *cell;
    PHSSectorClass *sector;
    PHSTrafficTypeClass *traffic_type;

    /**************************************************************************************/
    /**** Read CSC file                                                                ****/
    /**************************************************************************************/
    readCscFile(cscfile, ipStrList, gwCscList);

    if (error_state) { return; }
    /**************************************************************************************/

    ipStrList->printlist();
    gwCscList->printlist();

    /**************************************************************************************/
    /**** Read ST data                                                                 ****/
    /**************************************************************************************/
    readSTDataFile(filename, format, period, ipStrList, gwCscList);
    /**************************************************************************************/
    /**** For Sanyo CS, use 3*ST23 - 2*ST25 as equivalent of melco's ST21.             ****/
    /**************************************************************************************/
    if (format == 1) {
        int paramIdxA = PHSSectorClass::getSTParamIdx(0, 21);
        int paramIdxB = PHSSectorClass::getSTParamIdx(1, 23);
        int paramIdxC = PHSSectorClass::getSTParamIdx(1, 25);

        for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                if (sector->st_data) {
                    sector->st_data[paramIdxA] = 3*sector->st_data[paramIdxB] - 2*sector->st_data[paramIdxC];
                }
            }
        }
    }

    /**************************************************************************************/

    /**************************************************************************************/
    /**** Check ST data for each sector, if data not valid delete all ST data for the  ****/
    /**** sector and issue a warning                                                   ****/
    /**************************************************************************************/
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            if (sector->st_data) {
                if (sector->checkSTData(msg) == 0) {
                    PRMSG(stdout, msg);
                    warning_state = 1;

                    free(sector->st_data);
                    sector->st_data = (int *) NULL;
                }
            }
        }
    }
    /**************************************************************************************/

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            if (sector->st_data) {
				        if (sector->st_data[11]+sector->st_data[12]+sector->st_data[14]+sector->st_data[21] == 0) {
		                sector->meas_ctr_list[0] = (double) sector->st_data[5]+sector->st_data[6]+sector->st_data[7];
		                sector->meas_ctr_list[1] = 0.0;
				        } else {
				            sector->meas_ctr_list[0] = (double) (sector->st_data[5]+sector->st_data[6]+sector->st_data[7])*(sector->st_data[12]+sector->st_data[14])/(sector->st_data[11]+sector->st_data[12]+sector->st_data[14]+sector->st_data[21]);
				        		sector->meas_ctr_list[1] = (double) (sector->st_data[5]+sector->st_data[6]+sector->st_data[7])*(sector->st_data[11]+sector->st_data[21])/(sector->st_data[11]+sector->st_data[12]+sector->st_data[14]+sector->st_data[21]);
				        }
				    } else {
				        sector->meas_ctr_list[0]=0.0;
				        sector->meas_ctr_list[1]=0.0;
				    }
				}
    }

    Sum_St22 = Sum_st(PHSSectorClass::getSTParamIdx(0, 22));
    Sum_St21 = Sum_st(PHSSectorClass::getSTParamIdx(0, 21));
    Sum_St10 = Sum_st(PHSSectorClass::getSTParamIdx(0, 10));
    Sum_St18 = Sum_st(PHSSectorClass::getSTParamIdx(0, 18));
    Sum_St20 = Sum_st(PHSSectorClass::getSTParamIdx(0, 20));

    traffic_type = (PHSTrafficTypeClass *) traffic_type_list[1];
    traffic_type->min_time=(Sum_St10==0 ? 0.0 : (double) (Sum_St22-Sum_St21)/Sum_St10);

    char tmp_msg[100];
    sprintf(tmp_msg, "%d----Sum_St22---- %d", period, Sum_St22);
    PRMSG(stdout, tmp_msg);
    //sprintf(tmp_msg, "----Sum_St21---- %d", Sum_St21);
    //PRMSG(stdout, tmp_msg);
    //sprintf(tmp_msg, "----traffic_type->min_time---- %f", traffic_type->min_time);
    //PRMSG(stdout, tmp_msg);
    traffic_type->max_time=traffic_type->min_time;

    traffic_type = (PHSTrafficTypeClass *) traffic_type_list[0];
    traffic_type->mean_time=(Sum_St18+Sum_St20==0 ? 0.0 : (double) Sum_St21/(Sum_St18+Sum_St20)+traffic_type->min_time);

    /**************************************************************************************/
    /**** Free memory                                                                  ****/
    /**************************************************************************************/
    for (gwCscIdx=0; gwCscIdx<=ipStrList->getSize()-1; gwCscIdx++) {
        (*ipStrList)[gwCscIdx].setStr((char *) NULL);
    }
    delete ipStrList;
    delete gwCscList;
    /**************************************************************************************/
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::readCscFile                                              ****/
/******************************************************************************************/
void NetworkClass::readCscFile(char *cscfile, ListClass<CharStrClass> *ipStrList, ListClass<int> *gwCscList)
{
    int i, j, linenum, file_size, bytes_read, num_bytes, gwCsc;
    char *str1, *str2, *chptr;
    FILE *fp;

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(cscfile, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file cscfile \"%s\"\n", cscfile);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    file_size = get_file_size(cscfile);
    sprintf(msg, "Reading cscnodefile: %s (TOTAL %d bytes)\n", cscfile, file_size);
    PRMSG(stdout, msg);

    linenum = 0;

    while (fgetline(fp, line)) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM",");
        if ( str1 && (str1[0] != '#') ) {
            str2 = strtok(NULL, CHDELIM",");

            if ( (strcmp(str1, "CSCID")==0) && (strcmp(str2, "IP")==0) ) {
                if (ipStrList->getSize()) {
                    sprintf(msg, "ERROR: invalid cscnode file \"%s(%d)\" CSCID,IP not at beginning of dataset\n", cscfile, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                }
            } else {
                chptr = str1;
                while(*chptr) {
                    if ( ((*chptr) < '0') || ((*chptr) > '9')) {
                        sprintf(msg, "ERROR: cscnode file \"%s(%d)\" CSCID = \"%s\" must be numeric\n", cscfile, linenum, str1);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    chptr++;
                }
                gwCsc = atoi(str1);
                if ((gwCsc < 0) || (gwCsc > 9999)) {
                    sprintf(msg, "ERROR: cscnode file \"%s(%d)\" CSCID = %d out of range\n", cscfile, linenum, gwCsc);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                }
                ipStrList->append(CharStrClass());
                gwCscList->append(gwCsc);
                (*ipStrList)[ipStrList->getSize()-1].setStr(str2);
            }
        }
    }
    fclose(fp);

    sort2(ipStrList, gwCscList);

    for (i=0; i<=ipStrList->getSize()-2; i++) {
        if (strcmp( (*ipStrList)[i].getStr(), (*ipStrList)[i+1].getStr() ) == 0) {
            sprintf(msg, "ERROR: cscnode file \"%s\" contains multiple instances of IP = \"%s\"\n", cscfile, (*ipStrList)[i].getStr());
            PRMSG(stdout, msg);
            error_state = 1;
            return;
        }
        for (j=i+1; j<=ipStrList->getSize()-1; j++) {
            if ((*gwCscList)[i] == (*gwCscList)[j]) {
                sprintf(msg, "ERROR: cscnode file \"%s\" contains multiple instances of CSCID = \"%.4d\"\n", cscfile, (*gwCscList)[i]);
                PRMSG(stdout, msg);
                error_state = 1;
                return;
            }
        }
    }

    free(line);

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::readSTDataFile                                           ****/
/******************************************************************************************/
void NetworkClass::readSTDataFile(char *filename, int format, int period, ListClass<CharStrClass> *ipStrList, ListClass<int> *gwCscList)
{
    int i, j, linenum, file_size, bytes_read, num_bytes, gwCsc, gwCscCs, flag, periodVal;
    int column, cont, ipStrIdx, csIdx, cell_idx, sector_idx, stIdx, warn_ipstr_not_found;
    int maxSTIdx;
    char *str1, *str2, *chptr, *n_str1;
    CharStrClass ipStr;
    PHSSectorClass *sector;
    FILE *fp;

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file stfile \"%s\"\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    file_size = get_file_size(filename);
    sprintf(msg, "Reading stfile: %s (TOTAL %d bytes)\n", filename, file_size);
    PRMSG(stdout, msg);

    maxSTIdx = PHSSectorClass::getMaxSTIdx(format);

    PHSSectorClass::st_data_period = period;

    linenum = 0;
    flag = 0;
    warn_ipstr_not_found = 0;

    // liutao,060827 -- begin
    // FILE* fp_csid;
    int* cell_has_st=(int*)malloc(num_cell*sizeof(int));
    for( int i=0; i<num_cell; i++) 
				cell_has_st[i]=0;
				
    char** st_extra_csid = (char**)malloc(sizeof(char*)); 
    int num_st_extra_csid = 0;
        
    while (fgetline(fp, line)) {
        linenum++;
        str1 = line;
        chptr = str1;
        if (chptr) {
            while((*chptr)&&(*chptr!=',')) { chptr++; }
            if ((*chptr) == ',') {
                (*chptr) = (char) NULL;
                n_str1 = chptr+1;
            } else {
                n_str1 = (char *) NULL;
            }
        }
        str1 = remove_quotes(str1);
        column = 0;
        if ( str1 && (str1[0] != '#') ) {
            if ( (strcmp(str1, "MO_NAME")==0) ) {
                if (flag) {
                    sprintf(msg, "ERROR: invalid stfile \"%s(%d)\" MO_NAME not at beginning of dataset\n", filename, linenum);
                    PRMSG(stdout, msg); error_state = 1;
                    return;
                }
            } else {
                flag = 1;
                cont = 1;
                do {
                    if (column == 1) {
                        ipStr.setStr(str1);
                        ipStrIdx = ipStrList->get_index_sorted(ipStr, 0);
                        if ((ipStrIdx == -1) && (warn_ipstr_not_found <= 10)) {
                            chptr = msg;
                            chptr += sprintf(chptr, "WARNING: stfile \"%s(%d)\" column = 1 has node address \"%s\" which is not in cscnode file\n",
                                filename, linenum, ipStr.getStr());
                            if (warn_ipstr_not_found == 10) {
                                chptr += sprintf(chptr, "Further WARNINGS suppressed.");
                            }
                            PRMSG(stdout, msg); warning_state = 1;
                            warn_ipstr_not_found++;
                            cont = 0;
                        } else {
                        		if (ipStrIdx != -1) {
                            		gwCsc = (*gwCscList)[ipStrIdx];
				                    } else {
				                    		cont = 0;
				                    }
                        }
                    } else if (column == 2) {
                        csIdx = atoi(str1);
                        gwCscCs = 100*gwCsc + csIdx;
                        uid_to_sector(gwCscCs, cell_idx, sector_idx);
                        if (cell_idx == -1) {
                            cont = 0;
                            //liutao,060827 -- begin
                            if( 0== num_st_extra_csid ) {
                            	st_extra_csid[0] = (char*)malloc(7*sizeof(char)); 
                            	sprintf(st_extra_csid[0],"%.6d",gwCscCs);
                            	num_st_extra_csid++;
                            }
                            else {
                            	//find if the cs has already been recorded
                            	int find = 0;
                                for(int i=0; i<num_st_extra_csid; i++) {
                                    if(gwCscCs == atoi(st_extra_csid[i])) {
                                    	find = 1;
                                        break;
									}
                                }
                            	if(!find) {
                             	    st_extra_csid = (char**)realloc(st_extra_csid, (num_st_extra_csid+1)*sizeof(char*));
                            	    st_extra_csid[num_st_extra_csid] = (char*)malloc(7*sizeof(char)); 
                           	    sprintf(st_extra_csid[num_st_extra_csid],"%.6d",gwCscCs);
                            	    num_st_extra_csid++;
                            	}
                            }
                        }

                        //liutao,060827 -- begin
                        else {
                            cell_has_st[cell_idx] = 1;
                        }         
                        //liutao,060827 -- end               	
                    } else if (column == 5) {
                        periodVal = atoi(str1);
                        if (periodVal != period) {
                            cont = 0;
                        }
                    } else if (column >= 7) {
                        stIdx = column-7 + 1;
                        sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];
                        if (!sector->st_data) {
                            sector->st_data = IVECTOR(sector->st_param_list->getSize());
                            for (i=0; i<=sector->st_param_list->getSize()-1; i++) {
                                sector->st_data[i] = 0;
                            }
                        }
                        stIdx = sector->getSTParamIdx(format, stIdx);
                        if (stIdx != -1) {
                            sector->st_data[stIdx] = atoi(str1);
                        }
                    }

                    str1 = n_str1;
                    chptr = str1;
                    if (chptr) {
                        while((*chptr)&&(*chptr!=',')) { chptr++; }
                        if ((*chptr) == ',') {
                            (*chptr) = (char) NULL;
                            n_str1 = chptr+1;
                        } else {
                            n_str1 = (char *) NULL;
                        }
                    }
                    str1 = remove_quotes(str1);
                    column++;
                    if (!str1) {
                        if (column <= 7 + maxSTIdx - 1) {
                            sprintf(msg, "ERROR: invalid stfile \"%s(%d)\" line contains too few columns of data", filename, linenum);
                            PRMSG(stdout, msg); error_state = 1;
                            return;
                        }
                        cont = 0;
                    }
                } while(cont);
            }
        }
    }
    fclose(fp);

    free(line);

    ipStr.setStr((char *) NULL);

    //liutao,060827 -- begin
    sprintf(msg, "----------------------------------------------------------------");
    PRMSG(stdout, msg);
    sprintf(msg, "List of the gw-csc-cs in traffic data but not in the cell table:");
    PRMSG(stdout, msg);

    for(int i=0; i<num_st_extra_csid; i++ ) {
    	//fprintf(fp_csid, "%s\n", st_extra_csid[i] );
    	sprintf(msg, "%s", st_extra_csid[i]);
        PRMSG(stdout, msg);
    }
    sprintf(msg, "----------------------------------------------------------------");
    PRMSG(stdout, msg);

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::delete_st_data                                           ****/
/******************************************************************************************/
void NetworkClass::delete_st_data()
{
    int cell_idx, sector_idx;
    CellClass *cell;
    PHSSectorClass *sector;

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            if (sector->st_data) {
                 free(sector->st_data);
                 sector->st_data = (int *) NULL;
            }
        }
    }
}
/******************************************************************************************/
