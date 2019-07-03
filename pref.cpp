/******************************************************************************************/
/**** PROGRAM: pref.cpp                                                                ****/
/**** Michael Mandell 11/21/03                                                         ****/
/******************************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "cconst.h"
#include "WiSim.h"
#include "pref.h"
#include "cconst.h"

#if HAS_GUI
#include "gconst.h"
#include "WiSim_gui.h"
#endif

/*****************************************************************************************/
/****Function: DatabaseClass::DatabaseClass                                           ****/
/*****************************************************************************************/
DatabasePrefClass::DatabasePrefClass(){
    connection = (char *) NULL;
    name = (char*) NULL;
    port = (char *) NULL;
    ipaddr = (char*) NULL;
    sid = (char *)NULL;
    user = (char*) NULL;
    password = (char *)NULL;
}
/*****************************************************************************************/
/****Function: DatabaseClass::~DatabaseClass                                          ****/
/*****************************************************************************************/
DatabasePrefClass::~DatabasePrefClass(){
    if (connection)  free(connection);
    if (name)   free(name);
    if (port)   free(port);
    if (ipaddr)  free(ipaddr);
    if (sid)    free(sid);
    if (user)   free(user);
    if (password) free(password);
}
/******************************************************************************************/
/**** FUNCTION: PrefClass::PrefClass                                                   ****/
/******************************************************************************************/
PrefClass::PrefClass() {
    int i;

    language = CConst::en;
    num_db = 0;
    selected_db = -1;
    lv_dbnum = -1;
    filename = (char *) NULL;

    pwr_unit           = CConst::PWRdBuV_113;
    pwr_offset         = get_pwr_unit_offset(pwr_unit);

    char *str = CVECTOR(100);
    pwr_unit_to_name(pwr_unit, str);
    pwr_str_long = strdup(str);

    for (i=0; (str[i]) && (str[i] != '_'); i++) {}
    str[i] = (char) NULL;

    pwr_str_short = strdup(str);

    free(str);

    report_cell_name_pref     = CConst::CellIdxRef;

#if HAS_GUI
    cell_size_idx = 0;
    vw_cell_name_pref  = CConst::CellIdxRef;
    rtd_view_pref      = CConst::RTDbyLevel;

    GCellClass::setCellSize(cell_size_idx);
#endif
}
/******************************************************************************************/
/**** FUNCTION: PrefClass::~PrefClass                                                  ****/
/******************************************************************************************/
PrefClass::~PrefClass()
{
    int i;

    if (filename) { free(filename); }

    for (i=0; i<num_db; i++) {
        delete db_list[i];
    }

    if (num_db) {
        free(db_list);
    }

    free(pwr_str_long);
    free(pwr_str_short);
}
/******************************************************************************************/
/**** FUNCTION: PrefClass::readFile                                                   ****/
/******************************************************************************************/
int PrefClass::readFile(char *p_filename)
{
    FILE *fp;
    int i;
    int retval = 1;
    int cont;
    int index=0; 
    int ival, new_pwr_unit;
    char *str1, *str2;
    char *line = CVECTOR(MAX_LINE_SIZE);
    char *errmsg = CVECTOR(MAX_LINE_SIZE);
    filename = strdup(p_filename);

    if ( !(fp = fopen(filename, "rb")) ) {
        // sprintf(errmsg, "WARNING: Unable to open preferences file %s, using default preferences\n", filename);
        // PRMSG(stdout, errmsg);
        retval = 0;
        cont   = 0;
    } else {
        retval = 1;
        cont   = 1;
    }

    while ( cont && fgetline(fp, line) ) {
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            if (strcmp(str1, "LANGUAGE:") == 0) {
                str2 = strtok(NULL, CHDELIM);
                if (strcmp(str2, "en") == 0) {
                    language = CConst::en;
                } else if (strcmp(str2, "zh") == 0) {
                    language = CConst::zh;
                }
            }
            else if (strcmp(str1, "CELL_SIZE_IDX:") == 0) {
#if HAS_GUI
                str2 = strtok(NULL, CHDELIM);
                ival = atoi(str2);
                if ( (ival >= 0) && (ival <= GCellClass::num_sizes-1) ) {
                    cell_size_idx = ival;
                    GCellClass::setCellSize(cell_size_idx);
                }
#endif
            }
            else if (strcmp(str1, "REPORT_CELL_NAME_PREF:") == 0) {
                str2 = strtok(NULL, CHDELIM);
                report_cell_name_pref = atoi(str2);
            }
            else if (strcmp(str1, "PWR_UNIT:") == 0) {
                str2 = strtok(NULL, CHDELIM);
                new_pwr_unit = pwr_name_to_unit(str2);
                if (new_pwr_unit != -1) {
                    if (pwr_str_long ) { free(pwr_str_long ); }
                    if (pwr_str_short) { free(pwr_str_short); }
                    pwr_unit = new_pwr_unit;
                    pwr_offset = get_pwr_unit_offset(pwr_unit);
                    pwr_str_long = strdup(str2);

                    for (i=0; (str2[i]) && (str2[i] != '_'); i++) {}
                    str2[i] = (char) NULL;

                    pwr_str_short = strdup(str2);
                }
            }
            else if (strcmp(str1, "NUM_DB:") == 0)
            {
                str2 = strtok(NULL, CHDELIM);
                num_db = atoi(str2);
                db_list = (DatabasePrefClass**)malloc(num_db*sizeof(DatabasePrefClass*));
                for (int i=0; i<num_db; i++)
                    db_list[i] = new DatabasePrefClass();
            }
            else if (strcmp(str1, "SELECTED_DB:") == 0)
            {
                str2 = strtok(NULL, CHDELIM);
                selected_db = atoi(str2);
            }
            else if (strcmp(str1, "DATABASE:") == 0) {
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "CONNECTION:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->connection = strdup(str2);
                }
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "NAME:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->name = strdup(str2);
                }
                fgetline(fp, line); 
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "PORT:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->port = strdup(str2);
                }
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "IPADDRESS:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->ipaddr = strdup(str2);
                }
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "SID:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->sid = strdup(str2);
                }
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "USERNAME:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->user = strdup(str2);
                }
                fgetline(fp, line);
                str1 = strtok(line, CHDELIM);
                if (strcmp(str1, "PASSWORD:") == 0)
                {
                    str2 = strtok(NULL, CHDELIM);
                    db_list[index]->password = strdup(str2);
                }
                index++;
             }
        }         
    }
    if (cont) {
        fclose(fp);
    }

    if (line) free(line);
    free(errmsg);
    return(retval);
}
/******************************************************************************************/
