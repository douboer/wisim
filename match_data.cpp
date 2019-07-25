/******************************************************************************************/
/**** FILE: match_data.cpp                                                             ****/
/**** Michael Mandell 6/22/06                                                          ****/
/******************************************************************************************/

#include <string.h>

#include "cconst.h"
#include "wisim.h"
#include "match_data.h"
#include "phs.h"
#include "statistics.h"
#include "phs_statistics.h"
#include "randomc.h"

/******************************************************************************************/
/**** FUNCTION: MatchDataClass::MatchDataClass                                         ****/
/******************************************************************************************/
MatchDataClass::MatchDataClass()
{
    num_iteration_parameter           = 0;
    iteration_parameter_name          = (char **) NULL;
    iteration_parameter_initial_value = (double *) NULL;
    iteration_parameter_step_value    = (double *) NULL;

    num_measured_parameter            = 0;
    measured_parameter_name           = (char **) NULL;
    measured_parameter_value          = (double *) NULL;
    measured_parameter_weight         = (double *) NULL;

    num_set_parameter                 = 0;
    set_parameter_name                = (char **) NULL;
    set_parameter_value               = (double *) NULL;

    num_sets = 2;
    initTime = 360.0;
    simulationTime = 3600.0;

    thresholdDifference = 2.0;
    hasThresholdDifference = 0;
    comm_lreg_arrival_ratio = 10.0;
    has_comm_lreg_arrival_ratio = 0;
    has_seed = 0;

    checkPointFile                    = (char *) NULL;
}
/******************************************************************************************/
/**** FUNCTION: MatchDataClass::~MatchDataClass                                        ****/
/******************************************************************************************/
MatchDataClass::~MatchDataClass()
{
    int idx;

    for (idx=0; idx<=num_iteration_parameter-1; idx++) {
        free(iteration_parameter_name[idx]);
    }
    free(iteration_parameter_name);
    free(iteration_parameter_initial_value);
    free(iteration_parameter_step_value);

    for (idx=0; idx<=num_measured_parameter-1; idx++) {
        free(measured_parameter_name[idx]);
    }
    free(measured_parameter_name);
    free(measured_parameter_value);
    free(measured_parameter_weight);

    for (idx=0; idx<=num_set_parameter-1; idx++) {
        free(set_parameter_name[idx]);
    }
    free(set_parameter_name);
    free(set_parameter_value);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::read_match_data                                          ****/
/**** Open specified file and read match data.                                         ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void NetworkClass::read_match_data(char *filename, char *force_fmt, char *checkPointFile)
{
    int linenum;
    char *str1;
    char *format_str;
    FILE *fp;

    if (num_cell) {
        BITWIDTH(bit_cell, num_cell-1);
    } else {
        sprintf(msg, "WARNING: num_cell = 0 no data read from file \"%s\"\n", filename);
        PRMSG(stdout, msg); warning_state = 1;
        return;
    }

    char *line = CVECTOR(MAX_LINE_SIZE);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: Unable to read from file 6 %s\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    enum state_enum {
        STATE_FORMAT,
        STATE_READ_VERSION
    };

    state_enum state;

    state = STATE_FORMAT;
    linenum = 0;

    if (!force_fmt) {
    while ( (state != STATE_READ_VERSION) && (fgetline(fp, line)) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_FORMAT:
                    if (strcmp(str1, "FORMAT:") != 0) {
                        sprintf(msg, "WARNING: road test data file \"%s\"\n"
                                         "Assuming format 1.1\n", filename);
                        PRMSG(stdout, msg); warning_state = 1;

                        fclose(fp);
                        if ( !(fp = fopen(filename, "rb")) ) {
                            sprintf(msg, "ERROR: cannot open match data file %s\n", filename);
                            PRMSG(stdout, msg);
                            error_state = 1;
                            return;
                        }
                        format_str = strdup("1.1");
                        state = STATE_READ_VERSION;
                    } else {
                        str1 = strtok(NULL, CHDELIM);
                        format_str = strdup(str1);
                        state = STATE_READ_VERSION;
                    }
                    break;
                default:
                    sprintf(msg, "ERROR: invalid match data file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_READ_VERSION) {
        sprintf(msg, "ERROR: invalid match data file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }
    } else {
        format_str = strdup(force_fmt);
    }

    if (strcmp(format_str,"1.1")==0) {
        read_match_data_1_1(fp, line, filename, linenum);
    } else {
        sprintf(msg, "ERROR: match data file \"%s\" has invalid format \"%s\"\n", filename, format_str);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

    if (error_state == 1) { return; }

    free(line);
    free(format_str);

    fclose(fp);

    if ((!error_state) && (matchData)) {
        if (matchData->checkPointFile) {
            free(matchData->checkPointFile);
            matchData->checkPointFile = (char *) NULL;
        }
        if (checkPointFile) {
            matchData->checkPointFile = strdup(checkPointFile);
        }
    }

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::read_match_data_1_1                                      ****/
/**** Open specified file and read match data.                                         ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void NetworkClass::read_match_data_1_1(FILE *fp, char *line, char *filename, int linenum)
{
#if (DEMO == 0)

    int param_idx;
    int num_iteration_parameter;
    int num_measured_parameter;
    int num_set_parameter;

    char *str1, str[1000];

    if (matchData) {
        delete matchData;
    }
    matchData = new MatchDataClass();

    enum state_enum {
        STATE_NUM_ITERATION_PARAMETER,
        STATE_ITERATION_PARAMETER,
        STATE_ITERATION_PARAMETER_NAME,
        STATE_ITERATION_PARAMETER_INITIAL_VALUE,
        STATE_ITERATION_PARAMETER_STEP_VALUE,

        STATE_NUM_MEASURED_PARAMETER,
        STATE_MEASURED_PARAMETER,
        STATE_MEASURED_PARAMETER_NAME,
        STATE_MEASURED_PARAMETER_VALUE,
        STATE_MEASURED_PARAMETER_WEIGHT,

        STATE_NUM_SET_PARAMETER,
        STATE_SET_PARAMETER,
        STATE_SET_PARAMETER_NAME,
        STATE_SET_PARAMETER_VALUE,

        STATE_DONE
    };

    state_enum state;

    /**************************************************************************************/

    state = STATE_NUM_ITERATION_PARAMETER;

    while ( fgetline(fp, line) ) {
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            switch(state) {
                case STATE_NUM_ITERATION_PARAMETER:
                    if (strcmp(str1, "NUM_ITERATION_PARAMETER:") != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"NUM_ITERATION_PARAMETER:\" NOT \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);

                    if (str1) {
                        num_iteration_parameter = atoi(str1);
                    } else {
                        num_iteration_parameter = -1;
                    }

                    if (num_iteration_parameter < 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s\"\n"
                                         "num_iteration_parameter = %d must be >= 0\n",
                                filename, num_iteration_parameter);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (num_iteration_parameter) {
                        matchData->num_iteration_parameter           = num_iteration_parameter;
                        matchData->iteration_parameter_name          = (char **) malloc(num_iteration_parameter*sizeof(char *));
                        matchData->iteration_parameter_initial_value = DVECTOR(num_iteration_parameter);
                        matchData->iteration_parameter_step_value    = DVECTOR(num_iteration_parameter);

                        param_idx = 0;
                        state = STATE_ITERATION_PARAMETER;
                    } else {
                        state = STATE_NUM_MEASURED_PARAMETER;
                    }
                    break;
                case STATE_ITERATION_PARAMETER:
                    sprintf(str, "ITERATION_PARAMETER_%d:", param_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    state = STATE_ITERATION_PARAMETER_NAME;
                    break;
                case STATE_ITERATION_PARAMETER_NAME:
                    sprintf(str, "NAME:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, "");
                    matchData->iteration_parameter_name[param_idx] = get_strid(str1);
                    if (!matchData->iteration_parameter_name[param_idx]) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "ITERATION_PARAMETER_%d has no name\n", filename, linenum, param_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_ITERATION_PARAMETER_INITIAL_VALUE;
                    break;
                case STATE_ITERATION_PARAMETER_INITIAL_VALUE:
                    sprintf(str, "INITIAL_VALUE:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        matchData->iteration_parameter_initial_value[param_idx] = atof(str1);
                        
                    } else {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "No \"%s\" specified\n", filename, linenum, str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_ITERATION_PARAMETER_STEP_VALUE;
                    break;
                case STATE_ITERATION_PARAMETER_STEP_VALUE:
                    sprintf(str, "STEP_VALUE:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        matchData->iteration_parameter_step_value[param_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "No \"%s\" specified\n", filename, linenum, str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    param_idx++;
                    if (param_idx >= num_iteration_parameter) {
                        state = STATE_NUM_MEASURED_PARAMETER;
                    } else {
                        state = STATE_ITERATION_PARAMETER;
                    }
                    break;

                case STATE_NUM_MEASURED_PARAMETER:
                    sprintf(str, "NUM_MEASURED_PARAMETER:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);

                    if (str1) {
                        num_measured_parameter = atoi(str1);
                    } else {
                        num_measured_parameter = -1;
                    }

                    if (num_measured_parameter < 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s\"\n"
                                         "num_measured_parameter = %d must be >= 0\n",
                                filename, num_measured_parameter);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (num_measured_parameter) {
                        matchData->num_measured_parameter           = num_measured_parameter;
                        matchData->measured_parameter_name          = (char **) malloc(num_measured_parameter*sizeof(char *));
                        matchData->measured_parameter_value         = DVECTOR(num_measured_parameter);
                        matchData->measured_parameter_weight        = DVECTOR(num_measured_parameter);

                        param_idx = 0;
                        state = STATE_MEASURED_PARAMETER;
                    } else {
                        state = STATE_NUM_SET_PARAMETER;
                    }
                    break;
                case STATE_MEASURED_PARAMETER:
                    sprintf(str, "MEASURED_PARAMETER_%d:", param_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    state = STATE_MEASURED_PARAMETER_NAME;
                    break;
                case STATE_MEASURED_PARAMETER_NAME:
                    sprintf(str, "NAME:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, "");
                    matchData->measured_parameter_name[param_idx] = get_strid(str1);
                    if (!matchData->measured_parameter_name[param_idx]) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "MEASURED_PARAMETER_%d has no name\n", filename, linenum, param_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_MEASURED_PARAMETER_VALUE;
                    break;
                case STATE_MEASURED_PARAMETER_VALUE:
                    sprintf(str, "VALUE:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        matchData->measured_parameter_value[param_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "No \"%s\" specified\n", filename, linenum, str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }
                    state = STATE_MEASURED_PARAMETER_WEIGHT;
                    break;
                case STATE_MEASURED_PARAMETER_WEIGHT:
                    sprintf(str, "WEIGHT:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        matchData->measured_parameter_weight[param_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "No \"%s\" specified\n", filename, linenum, str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    param_idx++;
                    if (param_idx >= num_measured_parameter) {
                        state = STATE_NUM_SET_PARAMETER;
                    } else {
                        state = STATE_MEASURED_PARAMETER;
                    }
                    break;

                case STATE_NUM_SET_PARAMETER:
                    sprintf(str, "NUM_SET_PARAMETER:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename, linenum, str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);

                    if (str1) {
                        num_set_parameter = atoi(str1);
                    } else {
                        num_set_parameter = -1;
                    }

                    if (num_set_parameter < 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s\"\n"
                                         "num_set_parameter = %d must be >= 0\n",
                                filename, num_set_parameter);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    if (num_set_parameter) {
                        matchData->num_set_parameter           = num_set_parameter;
                        matchData->set_parameter_name          = (char **) malloc(num_set_parameter*sizeof(char *));
                        matchData->set_parameter_value         = DVECTOR(num_set_parameter);

                        param_idx = 0;
                        state = STATE_SET_PARAMETER;
                    } else {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_SET_PARAMETER:
                    sprintf(str, "SET_PARAMETER_%d:", param_idx);
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    state = STATE_SET_PARAMETER_NAME;
                    break;
                case STATE_SET_PARAMETER_NAME:
                    sprintf(str, "NAME:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }

                    str1 = strtok(NULL, "");
                    matchData->set_parameter_name[param_idx] = get_strid(str1);
                    if (!matchData->set_parameter_name[param_idx]) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "SET_PARAMETER_%d has no name\n", filename, linenum, param_idx);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    state = STATE_SET_PARAMETER_VALUE;
                    break;
                case STATE_SET_PARAMETER_VALUE:
                    sprintf(str, "VALUE:");
                    if (strcmp(str1, str) != 0) {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "Expecting \"%s\" NOT \"%s\"\n", filename,linenum,
                                str, str1);
                        PRMSG(stdout, msg); error_state = 1;
                        return;
                    }
                    str1 = strtok(NULL, CHDELIM);
                    if (str1) {
                        matchData->set_parameter_value[param_idx] = atof(str1);
                    } else {
                        sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                         "No \"%s\" specified\n", filename, linenum, str);
                        PRMSG(stdout, msg);
                        error_state = 1;
                        return;
                    }

                    param_idx++;
                    if (param_idx >= num_set_parameter) {
                        state = STATE_DONE;
                    } else {
                        state = STATE_SET_PARAMETER;
                    }
                    break;
                case STATE_DONE:
                    sprintf(msg, "ERROR: invalid match_data file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, msg);
                    error_state = 1;
                    return;
                    break;

                default:
                    sprintf(msg, "ERROR: invalid match_data file \"%s\"\n"
                                     "Invalid state (%d) encountered\n", filename, state);
                    PRMSG(stdout, msg); error_state = 1;
                    return;
                    break;
            }
        }
    }

    if (state != STATE_DONE) {
        sprintf(msg, "ERROR: invalid match_data file \"%s\"\n"
            "Premature end of file encountered\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

#endif
    return;
}
/******************************************************************************************/
/**** FUNCTION: NetworkClass::set_match_data                                           ****/
/**** Open specified file and set match data.                                          ****/
/**** Blank lines are ignored.                                                         ****/
/**** Lines beginning with "#" are treated as comments.                                ****/
/******************************************************************************************/
void NetworkClass::set_match_data(int it, int rt)
{
#if (DEMO == 0)

    if (matchData) {
        delete matchData;
    }
    matchData = new MatchDataClass();

    int Sum_st6=Sum_st(PHSSectorClass::getSTParamIdx(0, 6));
    int Sum_st7=Sum_st(PHSSectorClass::getSTParamIdx(0, 7));
    int Sum_st8=Sum_st(PHSSectorClass::getSTParamIdx(0, 8));
    int Sum_st9=Sum_st(PHSSectorClass::getSTParamIdx(0, 9));
    int Sum_st10=Sum_st(PHSSectorClass::getSTParamIdx(0, 10));
    int Sum_st17=Sum_st(PHSSectorClass::getSTParamIdx(0, 17));
    int Sum_st19=Sum_st(PHSSectorClass::getSTParamIdx(0, 19));
    int Sum_st14=Sum_st(PHSSectorClass::getSTParamIdx(0, 14));
    int Sum_st28=Sum_st(PHSSectorClass::getSTParamIdx(0, 28));
	int Sum_st8_9_10= Sum_st8+Sum_st9+Sum_st10;

    char tmp_msg[100];

	//sprintf(tmp_msg, "PHSSectorClass::getSTParamIdx(0, 8)=%d\n", PHSSectorClass::getSTParamIdx(0, 8));
	//PRMSG(stdout, tmp_msg);

	/*sprintf(tmp_msg, "Sum_st8=%d\n", Sum_st8);
	PRMSG(stdout, tmp_msg);
	sprintf(tmp_msg, "Sum_st9=%d\n", Sum_st9);
	PRMSG(stdout, tmp_msg);
	sprintf(tmp_msg, "Sum_st10=%d\n", Sum_st10);
	PRMSG(stdout, tmp_msg);
	sprintf(tmp_msg, "Sum_st8_9_10=%d\n", Sum_st8_9_10);
	PRMSG(stdout, tmp_msg);
	*/
    matchData->num_set_parameter           = 5;
    matchData->set_parameter_name          = (char **) malloc(5*sizeof(char *));
    matchData->set_parameter_value    = DVECTOR(5);

    matchData->set_parameter_name[0] = strdup("threshold_difference");
    matchData->set_parameter_value[0] = 2;

    matchData->set_parameter_name[1] = strdup("comm_lreg_arrival_ratio");
    matchData->set_parameter_value[1] = ((Sum_st14+Sum_st28) ? (double) (Sum_st17+Sum_st19)/(Sum_st14+Sum_st28) : 100.0);

    matchData->set_parameter_name[2] = strdup("init_time");
    matchData->set_parameter_value[2] = it;

    matchData->set_parameter_name[3] = strdup("run_time");
    matchData->set_parameter_value[3] = rt;
    
    matchData->set_parameter_name[4] = strdup("seed");
    matchData->set_parameter_value[4] = 2244344;

	//sprintf(tmp_msg, "comm_lreg_arrival_ratio=%f\n", matchData->set_parameter_value[1]);
	//PRMSG(stdout, tmp_msg);
		
    matchData->num_iteration_parameter           = 3;
    matchData->iteration_parameter_name          = (char **) malloc(3*sizeof(char *));
    matchData->iteration_parameter_initial_value = DVECTOR(3);
    matchData->iteration_parameter_step_value    = DVECTOR(3);

    matchData->iteration_parameter_name[0] = strdup("comm_arrival_rate");
    matchData->iteration_parameter_initial_value[0] = ((Sum_st6+Sum_st7) ? Sum_st8_9_10/3600.0/(1.0+ (double) Sum_st7/(Sum_st6+Sum_st7))/(1.0+1/matchData->set_parameter_value[1]) : Sum_st8_9_10/3600.0);
    matchData->iteration_parameter_step_value[0] = matchData->iteration_parameter_initial_value[0]/4;

    matchData->iteration_parameter_name[1] = strdup("int_threshold_call_request_cs_db");
    matchData->iteration_parameter_initial_value[1] = -100;//30.0;
    matchData->iteration_parameter_step_value[1] = 5;//matchData->iteration_parameter_initial_value[1]*3;

    matchData->iteration_parameter_name[2] = strdup("int_threshold_call_request_ps_db");
    matchData->iteration_parameter_initial_value[2] = -92;//30.0;
    matchData->iteration_parameter_step_value[2] = 5;//matchData->iteration_parameter_initial_value[2]*3;

    /*sprintf(tmp_msg, "set_total_traffic -traffic_type COMM -arrival_rate %25.20e\n", matchData->iteration_parameter_initial_value[0]);
	PRMSG(stdout, tmp_msg);
    sprintf(tmp_msg, "set_total_traffic -traffic_type LREG -arrival_rate %25.20e\n", matchData->iteration_parameter_initial_value[0]/matchData->set_parameter_value[1]);
	PRMSG(stdout, tmp_msg);
    sprintf(tmp_msg, "set -int_threshold_call_request_cs_db %f\n", matchData->iteration_parameter_initial_value[1]);
	PRMSG(stdout, tmp_msg);
    sprintf(tmp_msg, "set -int_threshold_call_drop_cs_db %f\n", matchData->iteration_parameter_initial_value[1]+matchData->set_parameter_value[0]);
	PRMSG(stdout, tmp_msg);
    sprintf(tmp_msg, "set -int_threshold_call_request_ps_db %f\n", matchData->iteration_parameter_initial_value[2]);
	PRMSG(stdout, tmp_msg);
    sprintf(tmp_msg, "set -int_threshold_call_drop_ps_db %f\n", matchData->iteration_parameter_initial_value[2]+matchData->set_parameter_value[0]);
	PRMSG(stdout, tmp_msg);
    */
    
    matchData->num_measured_parameter           = 4;
    matchData->measured_parameter_name          = (char **) malloc(4*sizeof(char *));
    matchData->measured_parameter_value = DVECTOR(4);
    matchData->measured_parameter_weight    = DVECTOR(4);

    matchData->measured_parameter_name[0] = strdup("phys_block_rate");
    matchData->measured_parameter_value[0] = (Sum_st8_9_10 ?  (double) Sum_st8/Sum_st8_9_10 : 0.0);
    matchData->measured_parameter_weight[0] = 0.25;

    matchData->measured_parameter_name[1] = strdup("cs_block_rate");
    matchData->measured_parameter_value[1] = (Sum_st8_9_10 ? (double) Sum_st9/Sum_st8_9_10 : 0.0);
    matchData->measured_parameter_weight[1] = 0.25;

    matchData->measured_parameter_name[2] = strdup("rereq_rate");
    matchData->measured_parameter_value[2] = ((Sum_st6+Sum_st7) ? (double) Sum_st7/(Sum_st6+Sum_st7) : 0.0);
    matchData->measured_parameter_weight[2] = 0.25;

    matchData->measured_parameter_name[3] = strdup("request_rate");
    matchData->measured_parameter_value[3] = Sum_st8_9_10/3600.0; //matchData->iteration_parameter_initial_value[0];
    matchData->measured_parameter_weight[3] = 0.25;

	sprintf(tmp_msg, "KPI: %15.10e %15.10e %15.10e %15.10e\n", 
	    matchData->measured_parameter_value[0],
	    matchData->measured_parameter_value[1],
	    matchData->measured_parameter_value[2],
	    matchData->measured_parameter_value[3]);
	PRMSG(stdout, tmp_msg);

/*    matchData->num_set_parameter           = 5;
    matchData->set_parameter_name          = (char **) malloc(5*sizeof(char *));
    matchData->set_parameter_value    = DVECTOR(5);

    matchData->set_parameter_name[0] = strdup("threshold_difference");
    matchData->set_parameter_value[0] = 2;

    matchData->set_parameter_name[1] = strdup("comm_lreg_arrival_ratio");
    matchData->set_parameter_value[1] = ((Sum_st14+Sum_st28) ? (double) (Sum_st17+Sum_st19)/(Sum_st14+Sum_st28) : 100.0);

    matchData->set_parameter_name[2] = strdup("init_time");
    matchData->set_parameter_value[2] = it;

    matchData->set_parameter_name[3] = strdup("run_time");
    matchData->set_parameter_value[3] = rt;
    
    matchData->set_parameter_name[4] = strdup("seed");
    matchData->set_parameter_value[4] = 2244344;
*/

    int i, q, m, done, done1, done2, min_idx, min_idx_single;
    int start_from_check_point_file = 0; // add this later ??
    int setParamIdx, measParamIdx;
    int comm_traffic_idx = get_traffic_type_idx("COMM", 1);
    int lreg_traffic_idx = get_traffic_type_idx("LREG", 1);
    double simVal, measVal;
    double param_val, min_metric, min_metric_single;
    PHSNetworkClass *phsnp = (PHSNetworkClass *) this;

    int num_attempt, attempt;
    int comm_total_num_request;
    int comm_total_num_request_blocked_phys_chan;
    int comm_total_num_request_blocked_sir_cs;
    int comm_total_num_handover;
    int comm_total_num_handover_blocked_phys_chan;
    int comm_total_num_handover_blocked_sir_cs;
    int lreg_total_num_request;
    int lreg_total_num_handover;
    int lreg_total_num_request_blocked_phys_chan;
    int lreg_total_num_request_blocked_sir_cs;
    int lreg_total_num_handover_blocked_phys_chan;
    int lreg_total_num_handover_blocked_sir_cs;
    int comm_num_rereq;
    int lreg_num_rereq;

    int total_num_requests;
    int total_phys_block;
    int total_cs_block;
    int total_num_rereq;

    double phys_block_rate;
    double cs_block_rate;
    double rereq_rate;
    double request_rate;

    PHSStatCountClass *sc;
    
    matchData->numCases = 30;
    
    int max_step = 30;
    double** set_param;
    set_param = (double**)malloc((matchData->numCases+max_step)*sizeof(double*));
    for(int j=0;j<matchData->numCases+max_step;j++)
        set_param[j] = DVECTOR(matchData->num_iteration_parameter);

    double** sim_param;
    sim_param = (double**)malloc((matchData->numCases+max_step)*sizeof(double*));
    for(int j=0;j<matchData->numCases+max_step;j++)
        sim_param[j] = DVECTOR(matchData->num_measured_parameter);


    matchData->p = DVECTOR(matchData->num_iteration_parameter);
    matchData->metric = DVECTOR(matchData->numCases+max_step);
    double* metric_single = DVECTOR(matchData->numCases+max_step);

    if (start_from_check_point_file) {
        // read_check_point_file($check_point_file);
    } else {
        for (i=0; i<=matchData->num_iteration_parameter-1; i++) {
            matchData->p[i] = matchData->iteration_parameter_initial_value[i];
        }

        for (i=0; i<matchData->numCases+max_step; i++) {
            matchData->metric[i] = 0.0;
        }
        matchData->setIdx = 0;
        matchData->caseIdx = 0;
        matchData->runCount = 0;
    }

    if (matchData->setIdx == matchData->num_sets) {
        done = 1;
    } else {
        done = 0;
        done1 = 0;
        done2 = 0;
    }

    sprintf(tmp_msg, "match starting...\n");
    PRMSG(stdout, tmp_msg);

    double* set_param_min=DVECTOR(matchData->numCases+max_step);
    double* set_param_max=DVECTOR(matchData->numCases+max_step);

    int last;
    int bound_found = 0;

    double best_int_threshold_call_request_ps_db, best_arrive_rate;
    best_int_threshold_call_request_ps_db = matchData->iteration_parameter_initial_value[2];
    best_arrive_rate = matchData->iteration_parameter_initial_value[0];

    int turn = 0; //0:initial state  1: match rerequest  2:match request
    int turn_done = 0;
    int step_idx1 = 0;
    int step_idx2 = 0;

    double* e_rerequest = DVECTOR(matchData->numCases+max_step);
    double* e_request = DVECTOR(matchData->numCases+max_step);

        
    while (!done) {
        
        //use initial values
        if(0 == turn) {
        
            /*sprintf(msg, "--------Use initial value--------\n");
            PRMSG(stdout, msg);
            sprintf(msg, "case_idx: %d\n", matchData->caseIdx);
            PRMSG(stdout, msg);*/
            
            
            setParam(matchData->iteration_parameter_name[2], matchData->iteration_parameter_initial_value[2]);
            set_param[matchData->caseIdx][2] = matchData->iteration_parameter_initial_value[2];

            setParam(matchData->iteration_parameter_name[0], matchData->iteration_parameter_initial_value[0]);
            set_param[matchData->caseIdx][0] = matchData->iteration_parameter_initial_value[0];

            setParam(matchData->iteration_parameter_name[1], matchData->iteration_parameter_initial_value[2]-17.0);
            set_param[matchData->caseIdx][1] = matchData->iteration_parameter_initial_value[2]-17.0;

        }

        if(1==turn) {
            /*sprintf(msg, "--------Adjust int_threshold_call_request_ps_db--------\n");
            PRMSG(stdout, msg);
            sprintf(msg, "case_idx: %d\tstep_idx: %d\n", matchData->caseIdx, step_idx1);
            PRMSG(stdout, msg);
            */
            
            if(0 == step_idx1) {
                if(sim_param[matchData->caseIdx-1][2] <= matchData->measured_parameter_value[2]) {
                    param_val = set_param[matchData->caseIdx-1][2]-10;
                    set_param_min[2] = param_val;
                    set_param_max[2] = set_param[matchData->caseIdx-1][2];
                    last = 1;//max
                }
                else {
                    param_val = set_param[matchData->caseIdx-1][2]+10;
                    set_param_max[2] = param_val;
                    set_param_min[2] = set_param[matchData->caseIdx-1][2];
                    last = 0;//min
                }
                param_val = (set_param_min[2]+set_param_max[2])/2.0;
            }
            else {
                if(sim_param[matchData->caseIdx-1][2] <= matchData->measured_parameter_value[2]) 
                    set_param_max[2] = set_param[matchData->caseIdx-1][2];
                else
                    set_param_min[2] = set_param[matchData->caseIdx-1][2];
                
                param_val = (set_param_min[2]+set_param_max[2])/2.0;

            }
            
            //set parameters and save settings
            setParam(matchData->iteration_parameter_name[2], param_val);
            set_param[matchData->caseIdx][2] = param_val;

            setParam(matchData->iteration_parameter_name[0], set_param[matchData->caseIdx-1][0]);
            set_param[matchData->caseIdx][0] = set_param[matchData->caseIdx-1][0];

            setParam(matchData->iteration_parameter_name[1], set_param[matchData->caseIdx][2]-17.0);
            set_param[matchData->caseIdx][1] = set_param[matchData->caseIdx][2]-17.0;

        }

        if(2==turn) {
            /*sprintf(msg, "--------Adjust arrive_rate--------\n");
            PRMSG(stdout, msg);
            sprintf(msg, "case_idx: %d\tstep_idx: %d\n", matchData->caseIdx, step_idx2);
            PRMSG(stdout, msg);
            */
            //adjust comm_arrival_rate according to previous simulation results(request_rate)
            if(0 == step_idx2) {
                param_val = set_param[matchData->caseIdx-1][0] * matchData->measured_parameter_value[3] / 
                            sim_param[matchData->caseIdx-1][3];
                if(sim_param[matchData->caseIdx-1][3] <= matchData->measured_parameter_value[3]) {
                    set_param_max[0] = param_val;
                    set_param_min[0] = set_param[matchData->caseIdx-1][0];
                    last = 1;//max
                }
                else {
                    set_param_min[0] = param_val;
                    set_param_max[0] = set_param[matchData->caseIdx-1][0];
                    last = 0;//min
                }
            }
            //<min
            else if(!bound_found && 1==last && sim_param[matchData->caseIdx-1][3] < matchData->measured_parameter_value[3]) {
                param_val = set_param[matchData->caseIdx-1][0] * matchData->measured_parameter_value[3] / 
                            sim_param[matchData->caseIdx-1][3];
                set_param_max[0] = param_val;
                set_param_min[0] = set_param[matchData->caseIdx-1][0];
            }
            //>max
            else if(!bound_found && 0==last && sim_param[matchData->caseIdx-1][3] > matchData->measured_parameter_value[3]) {
                param_val = set_param[matchData->caseIdx-1][0] * matchData->measured_parameter_value[3] / 
                            sim_param[matchData->caseIdx-1][3];
                set_param_min[0] = param_val;
                set_param_max[0] = set_param[matchData->caseIdx-1][0];
            }
            else {
                if(!bound_found)
                    bound_found = 1;

                //now, we have got the min and max
                if(sim_param[matchData->caseIdx-1][3] <= matchData->measured_parameter_value[3]) 
                    set_param_min[0] = set_param[matchData->caseIdx-1][0];
                else
                    set_param_max[0] = set_param[matchData->caseIdx-1][0];

                param_val = (set_param_min[0]+set_param_max[0])/2.0;

            }
           
            //set parameters and save settings
            setParam(matchData->iteration_parameter_name[0], param_val);
            set_param[matchData->caseIdx][0] = param_val;

            setParam(matchData->iteration_parameter_name[2], set_param[matchData->caseIdx-1][2]);
            set_param[matchData->caseIdx][2] = set_param[matchData->caseIdx-1][2];

            setParam(matchData->iteration_parameter_name[1], set_param[matchData->caseIdx][2]-17.0);
            set_param[matchData->caseIdx][1] = set_param[matchData->caseIdx][2]-17.0;
        }


        for (setParamIdx=0; setParamIdx<=matchData->num_set_parameter-1; setParamIdx++) {
            setParam(matchData->set_parameter_name[setParamIdx], matchData->set_parameter_value[setParamIdx]);
        }


        if (matchData->hasThresholdDifference) {
            phsnp->int_threshold_call_drop_cs_db    = phsnp->int_threshold_call_request_cs_db + matchData->thresholdDifference;
            phsnp->sir_threshold_call_drop_cs_db    = phsnp->sir_threshold_call_request_cs_db - matchData->thresholdDifference;
            phsnp->int_threshold_call_drop_ps_db    = phsnp->int_threshold_call_request_ps_db + matchData->thresholdDifference;
            phsnp->sir_threshold_call_drop_ps_db    = phsnp->sir_threshold_call_request_ps_db - matchData->thresholdDifference;

            phsnp->int_threshold_call_drop_cs         = exp(phsnp->int_threshold_call_drop_cs_db * log(10.0)/10.0);
            phsnp->sir_threshold_call_drop_cs         = exp(phsnp->sir_threshold_call_drop_cs_db * log(10.0)/10.0);
            phsnp->sir_threshold_call_drop_ps         = exp(phsnp->sir_threshold_call_drop_ps_db * log(10.0)/10.0);
            phsnp->int_threshold_call_drop_ps         = exp(phsnp->int_threshold_call_drop_ps_db * log(10.0)/10.0);
        }
        if (matchData->has_comm_lreg_arrival_ratio) {
            set_total_arrival_rate(lreg_traffic_idx, matchData->comm_arrival_rate / matchData->comm_lreg_arrival_ratio);
        }

        /*sprintf(msg,"matchData->comm_arrival_rate:%15.10e\n",matchData->comm_arrival_rate);
        PRMSG(stdout, msg);
        sprintf(msg,"matchData->comm_lreg_arrival_ratio:%15.10e\n",matchData->comm_lreg_arrival_ratio);
        PRMSG(stdout, msg);
        sprintf(msg,"lreg_traffic:%15.10e\n",matchData->comm_arrival_rate / matchData->comm_lreg_arrival_ratio);
        PRMSG(stdout, msg);
        */
        
        if (matchData->has_seed) {
            rg->RandomInit(matchData->seed);
        }

        /*sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[0], set_param[matchData->caseIdx][0]);
            PRMSG(stdout, msg);
        sprintf(msg, "matchData->comm_lreg_arrival_ratio: %15.10e\n", matchData->comm_lreg_arrival_ratio);
			PRMSG(stdout, msg);
        sprintf(msg, "set_total_traffic -traffic_type COMM -arrival_rate %15.10e\n", set_param[matchData->caseIdx][0]);
			PRMSG(stdout, msg);
        sprintf(msg, "set_total_traffic -traffic_type LREG -arrival_rate %15.10e\n", set_param[matchData->caseIdx][0] / matchData->comm_lreg_arrival_ratio);
			PRMSG(stdout, msg);
        sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[1], set_param[matchData->caseIdx][1]);
            PRMSG(stdout, msg);
        sprintf(msg, "set -int_threshold_call_drop_cs_db %15.10e\n", set_param[matchData->caseIdx][1]+ matchData->thresholdDifference);
			PRMSG(stdout, msg);
        sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[2], set_param[matchData->caseIdx][2]);
            PRMSG(stdout, msg);
        sprintf(msg, "set -int_threshold_call_drop_ps_db %15.10e\n", set_param[matchData->caseIdx][2]+ matchData->thresholdDifference);
			PRMSG(stdout, msg);
        */
        
        reset_system();

#if 0
        printf("MFK: BEGIN RUN SIMULATION...\n");

        if (matchData->caseIdx == 0) {
            stat->fp_event = fopen("/tmp/MFKmatch_event.txt", "w");
            stat->plot_event = 1;
        }
#endif

        run_simulation(0, -1, it);
//        run_simulation(0, -1, 36);
        if (error_state) { return; }
        reset_call_statistics(0);
        run_simulation(0, -1, rt);
//        run_simulation(0, -1, 36);
        if (error_state) { return; }

#if 0
        if (matchData->caseIdx == 0) {
            stat->plot_event = 0;
            fclose(stat->fp_event);
        }
#endif


        /**********************************************************************************/
        /**** Extract data from PHSStatCountClass                                      ****/
        /**********************************************************************************/
        sc = (PHSStatCountClass *) stat_count;

#if 0
        if (matchData->caseIdx == 0) {
            print_call_statistics("/tmp/MFKstat.txt", (char *) NULL, 1);
        }
#endif

        
        comm_total_num_request  = 0;
        comm_total_num_request_blocked_phys_chan = 0;
        comm_total_num_request_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[comm_traffic_idx])->get_num_attempt(CConst::RequestEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            comm_total_num_request                   += sc->num_request[comm_traffic_idx][attempt];
            comm_total_num_request_blocked_phys_chan += sc->num_request_blocked_phys_chan[comm_traffic_idx][attempt];
            comm_total_num_request_blocked_sir_cs     += sc->num_request_blocked_sir_cs[comm_traffic_idx][attempt];
/*
            sprintf(msg, "MFKcomm_total_num_request[%d] = %d\n", attempt, sc->num_request[comm_traffic_idx][attempt]);
            PRMSG(stdout, msg);
  */      }

        comm_total_num_handover = 0;
        comm_total_num_handover_blocked_phys_chan = 0;
        comm_total_num_handover_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[comm_traffic_idx])->get_num_attempt(CConst::HandoverEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            comm_total_num_handover                   += sc->num_handover[comm_traffic_idx][attempt];
            comm_total_num_handover_blocked_phys_chan += sc->num_handover_blocked_phys_chan[comm_traffic_idx][attempt];
            comm_total_num_handover_blocked_sir_cs    += sc->num_handover_blocked_sir_cs[comm_traffic_idx][attempt];
        }

        lreg_total_num_handover_blocked_phys_chan = 0;
        lreg_total_num_handover_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[lreg_traffic_idx])->get_num_attempt(CConst::HandoverEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            lreg_total_num_handover_blocked_phys_chan += sc->num_handover_blocked_phys_chan[lreg_traffic_idx][attempt];
            lreg_total_num_handover_blocked_sir_cs += sc->num_handover_blocked_sir_cs[lreg_traffic_idx][attempt];
        }

        lreg_total_num_request  = 0;
        lreg_total_num_handover = 0;
        lreg_total_num_request_blocked_phys_chan = 0;
        lreg_total_num_request_blocked_sir_cs = 0;        
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[lreg_traffic_idx])->get_num_attempt(CConst::RequestEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            lreg_total_num_request  += sc->num_request[lreg_traffic_idx][attempt];
            lreg_total_num_handover                   += sc->num_handover[lreg_traffic_idx][attempt];
            lreg_total_num_request_blocked_phys_chan  += sc->num_request_blocked_phys_chan[lreg_traffic_idx][attempt];
            lreg_total_num_request_blocked_sir_cs     += sc->num_request_blocked_sir_cs[lreg_traffic_idx][attempt];
        }

        comm_num_rereq = sc->num_rereq[comm_traffic_idx];
        lreg_num_rereq = sc->num_rereq[lreg_traffic_idx];

        /**********************************************************************************/

        /**********************************************************************************/
        /**** Compute Measured Parameters                                              ****/
        /**********************************************************************************/
        sc = (PHSStatCountClass *) stat_count;
        total_num_requests = comm_total_num_request
                           + comm_total_num_handover
                           + lreg_total_num_request
                           + lreg_total_num_handover;

        total_phys_block   = comm_total_num_request_blocked_phys_chan
                           + comm_total_num_handover_blocked_phys_chan
                           + lreg_total_num_request_blocked_phys_chan
                           + lreg_total_num_handover_blocked_phys_chan;

        total_cs_block     = comm_total_num_request_blocked_sir_cs
                           + comm_total_num_handover_blocked_sir_cs
                           + lreg_total_num_request_blocked_sir_cs
                           + lreg_total_num_handover_blocked_sir_cs;

        total_num_rereq    = comm_num_rereq
                           + lreg_num_rereq;


        if (total_num_requests) {
            phys_block_rate    = (double)total_phys_block / (double)total_num_requests;
            cs_block_rate      = (double)total_cs_block / (double)total_num_requests;
            rereq_rate         = (double)total_num_rereq / (double)total_num_requests;
            request_rate       = (double)total_num_requests / stat->duration;
        } else {
            phys_block_rate    = 0.0;
            cs_block_rate      = 0.0;
            rereq_rate         = 0.0;
            request_rate       = 0.0;
        }
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Compute metric                                                           ****/
        /**********************************************************************************/
        matchData->metric[matchData->caseIdx] = 0.0;
        for (measParamIdx=0; measParamIdx<=matchData->num_measured_parameter-1; measParamIdx++) {
            if (strcmp(matchData->measured_parameter_name[measParamIdx], "phys_block_rate")==0) {
                simVal = phys_block_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "cs_block_rate")==0) {
                simVal = cs_block_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "rereq_rate")==0) {
                simVal = rereq_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "request_rate")==0) {
                simVal = request_rate;
            } else {
                sprintf(msg, "ERROR: Unrecognized parameter name \"%s\"\n", matchData->measured_parameter_name[measParamIdx]);
                PRMSG(stdout, msg);
                error_state = 1;
            }
            measVal = matchData->measured_parameter_value[measParamIdx];
            sim_param[matchData->caseIdx][measParamIdx] = simVal; 
            matchData->metric[matchData->caseIdx] += matchData->measured_parameter_weight[measParamIdx] * (measVal - simVal) * (measVal - simVal);
            
        }
        e_rerequest[matchData->caseIdx] = fabs((matchData->measured_parameter_value[2] - rereq_rate) / matchData->measured_parameter_value[2]);
        e_request[matchData->caseIdx] = fabs((matchData->measured_parameter_value[3] - request_rate) / matchData->measured_parameter_value[3]);
        /*
        if(!done1)
            metric_single[matchData->caseIdx] = (matchData->measured_parameter_value[2] - rereq_rate) *
                                                (matchData->measured_parameter_value[2] - rereq_rate);
        else
            metric_single[matchData->caseIdx] = (matchData->measured_parameter_value[3] - request_rate) *
                                                (matchData->measured_parameter_value[3] - request_rate);*/

        /**********************************************************************************/

        /*sprintf(msg, "phys_block_rate = %15.10e\n", sim_param[matchData->caseIdx][0]);
        PRMSG(stdout, msg);
        sprintf(msg, "cs_block_rate = %15.10e\n", sim_param[matchData->caseIdx][1]);
        PRMSG(stdout, msg);
        sprintf(msg, "rereq_rate = %15.10e\n", sim_param[matchData->caseIdx][2]);
        PRMSG(stdout, msg);
        sprintf(msg, "request_rate = %15.10e\n", sim_param[matchData->caseIdx][3]);
        PRMSG(stdout, msg);
        */
/*
        sprintf(msg, "METRIC_SINGLE = %15.10e\n", metric_single[matchData->caseIdx]);
        PRMSG(stdout, msg);

        sprintf(msg, "METRIC = %15.10e\n", matchData->metric[matchData->caseIdx]);
        PRMSG(stdout, msg);
*/

        /*sprintf(msg, "E_REREQUEST = %f\t",e_rerequest[matchData->caseIdx]);
        PRMSG(stdout, msg);

        sprintf(msg, "E_REQUEST = %f\n", e_request[matchData->caseIdx]);
        PRMSG(stdout, msg);
        */
        
        if(0==turn) {
            turn = 1;
            turn_done = 1;
        }
        else if(1==turn) {
            if(e_rerequest[matchData->caseIdx] < e_request[matchData->caseIdx] || step_idx1 >= max_step-1) {
                if(step_idx1==max_step-1)
                    done = 1;
                step_idx1 = 0;
                turn = 2;
                turn_done = 1;
            }
            else {
                step_idx1++;
                turn_done = 0;
            }
        }
        else {
            if(e_rerequest[matchData->caseIdx] > e_request[matchData->caseIdx] || step_idx2 >= max_step-1) {
                if(step_idx2==max_step-1)
                    done = 1;
                step_idx2 = 0;
                turn = 1;
                turn_done = 1;
            }
            else {
                step_idx2++;
                turn_done = 0;
            }
        }


        //wait until turn finish
        if (matchData->caseIdx >= matchData->numCases && turn_done) 
            done = 1;
        
        //value good enough
        if(e_rerequest[matchData->caseIdx]<0.01 && e_request[matchData->caseIdx]<0.01) {
            done = 1;
            sprintf(msg, "appropriate values found\n");
            PRMSG(stdout, msg);
        }

        if(1==done) {
            //find the best parameters set
            min_idx = 0;
            min_metric = e_rerequest[0] + e_request[0];
            for(i=1; i<=matchData->caseIdx; i++) {
                if(e_rerequest[i]+e_request[i] < min_metric) {
                    min_idx = i;
                    min_metric = e_rerequest[i]+e_request[i];
                }
            }

            //show the best parameters set
            sprintf(msg, "============MATCH RESULTS============\n");
			    PRMSG(stdout, msg);
            sprintf(msg, "set_total_traffic -traffic_type COMM -arrival_rate %15.10e\n", set_param[min_idx][0]);
			    PRMSG(stdout, msg);
            sprintf(msg, "set_total_traffic -traffic_type LREG -arrival_rate %15.10e\n", set_param[min_idx][0] / matchData->comm_lreg_arrival_ratio);
			    PRMSG(stdout, msg);
            sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[1], set_param[min_idx][1]);
                PRMSG(stdout, msg);
            sprintf(msg, "set -int_threshold_call_drop_cs_db %15.10e\n", set_param[min_idx][1]+ matchData->thresholdDifference);
			    PRMSG(stdout, msg);
            sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[2], set_param[min_idx][2]);
                PRMSG(stdout, msg);
            sprintf(msg, "set -int_threshold_call_drop_ps_db %15.10e\n", set_param[min_idx][2]+ matchData->thresholdDifference);
			    PRMSG(stdout, msg);

            sprintf(msg, "phys_block_rate = %15.10e\n", sim_param[min_idx][0]);
            PRMSG(stdout, msg);
            sprintf(msg, "cs_block_rate = %15.10e\n", sim_param[min_idx][1]);
            PRMSG(stdout, msg);
            sprintf(msg, "rereq_rate = %15.10e\n", sim_param[min_idx][2]);
            PRMSG(stdout, msg);
            sprintf(msg, "request_rate = %15.10e\n", sim_param[min_idx][3]);
            PRMSG(stdout, msg);
            
            /*sprintf(msg, "INDEX  = %d\n", min_idx);
            PRMSG(stdout, msg);
            sprintf(msg, "E_REREQUEST = %f\t",e_rerequest[min_idx]);
            PRMSG(stdout, msg);
            sprintf(msg, "E_REQUEST = %f\n", e_request[min_idx]);
            PRMSG(stdout, msg);
            */
            
            /******************************************************************
             * call process command to write results to WiSim data structure
             ******************************************************************/
            sprintf(phsnp->line_buf, "switch_mode -mode edit_geom");
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -ps_dca_alg int");
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -cs_dca_alg int");
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -int_threshold_call_request_ps_db %15.10f", 
                    set_param[min_idx][1]);
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -int_threshold_call_request_cs_db %15.10f", 
                    set_param[min_idx][2]);
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -int_threshold_call_drop_ps_db %15.10f", 
                    set_param[min_idx][1]+ matchData->thresholdDifference);
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set -int_threshold_call_drop_cs_db %15.10f", 
                    set_param[min_idx][2]+ matchData->thresholdDifference);
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set_total_traffic -traffic_type COMM -arrival_rate %15.10f", 
                    set_param[min_idx][0]);
            phsnp->process_command(phsnp->line_buf);

            sprintf(phsnp->line_buf, "set_total_traffic -traffic_type LREG -arrival_rate %15.10f", 
                    set_param[min_idx][0] / matchData->comm_lreg_arrival_ratio);
            phsnp->process_command(phsnp->line_buf);
        }

        matchData->runCount++;

        matchData->caseIdx++;


        /****
        if (matchData->caseIdx == matchData->numCases) {

            min_idx = 0;
            min_metric = matchData->metric[0];
            for (i=1; i<=matchData->numCases-1; i++) {
                if (matchData->metric[i] < min_metric) {
                    min_idx = i;
                    min_metric = matchData->metric[i];
                }
            }

            min_idx_single = 0;
            min_metric_single = metric_single[0];
            for (i=1; i<=matchData->numCases-1; i++) {
                if (metric_single[i] < min_metric_single) {
                    min_idx_single = i;
                    min_metric_single = metric_single[i];
                }
            }
            
            int min_idx_result;
            double min_metric_result;
            
            min_idx_result = min_idx_single;
            min_metric_result = min_metric_single;

            if(!done) {
                min_idx_result = min_idx_single;
                min_metric_result = min_metric_single;
            }
            else {
                min_idx_result = min_idx;
                min_metric_result = min_metric;
            }*/
            /******
            if (matchData->runCount == matchData->numCases) {
                done1 = 1;
                best_int_threshold_call_request_ps_db = set_param[min_idx_result][2];
            }
            else if (matchData->runCount == 2*matchData->numCases) {
                done2 = 1;
                done = 1;
            }


			sprintf(msg, "------------------\nparameter results:\n");
            PRMSG(stdout, msg);

            for (i=0; i<=matchData->num_iteration_parameter-1; i++) {

                sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[i], set_param[min_idx_result][i]);
                PRMSG(stdout, msg);
                char tmp_msg[100];
                if (i==0) {
				    sprintf(tmp_msg, "set_total_traffic -traffic_type COMM -arrival_rate %15.10e\n", set_param[min_idx_result][i]);
				    PRMSG(stdout, tmp_msg);
				    sprintf(tmp_msg, "set_total_traffic -traffic_type LREG -arrival_rate %15.10e\n", set_param[min_idx_result][i] / matchData->comm_lreg_arrival_ratio);
				    PRMSG(stdout, tmp_msg);
                }
                if (i==1){
				    sprintf(tmp_msg, "set -int_threshold_call_drop_cs_db %15.10e\n", set_param[min_idx_result][i]+ matchData->thresholdDifference);
				    PRMSG(stdout, tmp_msg);
                }
                if (i==2){
				    sprintf(tmp_msg, "set -int_threshold_call_drop_ps_db %15.10e\n", set_param[min_idx_result][i]+ matchData->thresholdDifference);
				    PRMSG(stdout, tmp_msg);
                }
            }

            sprintf(msg, "phys_block_rate = %15.10e\n", sim_param[min_idx_result][0]);
            PRMSG(stdout, msg);
            sprintf(msg, "cs_block_rate = %15.10e\n", sim_param[min_idx_result][1]);
            PRMSG(stdout, msg);
            sprintf(msg, "rereq_rate = %15.10e\n", sim_param[min_idx_result][2]);
            PRMSG(stdout, msg);
            sprintf(msg, "request_rate = %15.10e\n", sim_param[min_idx_result][3]);
            PRMSG(stdout, msg);

            sprintf(msg, "INDEX  = %d\n", min_idx_result);
            PRMSG(stdout, msg);
            sprintf(msg, "METRIC_SINGLE = %15.10e\n", min_metric_result);
            PRMSG(stdout, msg);

            for (i=0; i<=matchData->numCases-1; i++) {
                matchData->metric[i] = 0;
                metric_single[i] = 0;
            }

            matchData->caseIdx = 0;

        }*******/
        //matchData->writeCheckPointFile(this);
    }

    sprintf(tmp_msg, "match ends\n");
    PRMSG(stdout, tmp_msg);

    free(matchData->p);
    free(matchData->metric);

    // free two-demention array
    for( int i=0; i<matchData->numCases+max_step; i++ )
    {
        free(set_param[i]);
        free(sim_param[i]);
    }
    free(set_param);
    free(sim_param);

    // free one-demention array
    free(e_rerequest);
    free(e_request);
    free(set_param_min);
    free(set_param_max);
    free(metric_single);

    return;

#endif

    return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::run_match                                                ****/
/******************************************************************************************/
void NetworkClass::run_match()
{
    int i, q, m, done, min_idx;
    int start_from_check_point_file = 0; // add this later ??
    int setParamIdx, measParamIdx;
    int comm_traffic_idx = get_traffic_type_idx("COMM", 1);
    int lreg_traffic_idx = get_traffic_type_idx("LREG", 1);
    double simVal, measVal;
    double param_val, min_metric;
    PHSNetworkClass *phsnp = (PHSNetworkClass *) this;

    int num_attempt, attempt;
    int comm_total_num_request;
    int comm_total_num_request_blocked_phys_chan;
    int comm_total_num_request_blocked_sir_cs;
    int comm_total_num_handover;
    int comm_total_num_handover_blocked_phys_chan;
    int comm_total_num_handover_blocked_sir_cs;
    int lreg_total_num_request;
    int lreg_total_num_request_blocked_phys_chan;
    int lreg_total_num_request_blocked_sir_cs;
    int comm_num_rereq;
    int lreg_num_rereq;

    int total_num_requests;
    int total_phys_block;
    int total_cs_block;
    int total_num_rereq;

    double phys_block_rate;
    double cs_block_rate;
    double rereq_rate;
    double request_rate;

    PHSStatCountClass *sc;

    matchData->numCases = 1;
    for (i=0; i<=matchData->num_iteration_parameter-1; i++) {
        matchData->numCases *= 3;
    }

    matchData->p = DVECTOR(matchData->num_iteration_parameter);
    matchData->metric = DVECTOR(matchData->numCases);

    if (start_from_check_point_file) {
        // read_check_point_file($check_point_file);
    } else {
        for (i=0; i<=matchData->num_iteration_parameter-1; i++) {
            matchData->p[i] = matchData->iteration_parameter_initial_value[i];
        }

        for (i=0; i<=matchData->numCases-1; i++) {
            matchData->metric[i] = 0.0;
        }
        matchData->setIdx = 0;
        matchData->caseIdx = 0;
        matchData->runCount = 0;
    }

    if (matchData->setIdx == matchData->num_sets) {
        done = 1;
    } else {
        done = 0;
    }

    while (!done) {
        q = matchData->caseIdx;
        for (i=0; i<=matchData->num_iteration_parameter-1; i++) {
            m = q % 3;
            param_val = matchData->p[i] + (m-1)*matchData->iteration_parameter_step_value[i]/(1<<matchData->setIdx);
            setParam(matchData->iteration_parameter_name[i], param_val);
            q = (q - m) / 3;
        }

        for (setParamIdx=0; setParamIdx<=matchData->num_set_parameter-1; setParamIdx++) {
            setParam(matchData->set_parameter_name[setParamIdx], matchData->set_parameter_value[setParamIdx]);
        }
        if (matchData->hasThresholdDifference) {
            phsnp->int_threshold_call_drop_cs_db    = phsnp->int_threshold_call_request_cs_db + matchData->thresholdDifference;
            phsnp->sir_threshold_call_drop_cs_db    = phsnp->sir_threshold_call_request_cs_db - matchData->thresholdDifference;
            phsnp->int_threshold_call_drop_ps_db    = phsnp->int_threshold_call_request_ps_db + matchData->thresholdDifference;
            phsnp->sir_threshold_call_drop_ps_db    = phsnp->sir_threshold_call_request_ps_db - matchData->thresholdDifference;

            phsnp->int_threshold_call_drop_cs         = exp(phsnp->int_threshold_call_drop_cs_db * log(10.0)/10.0);
            phsnp->sir_threshold_call_drop_cs         = exp(phsnp->sir_threshold_call_drop_cs_db * log(10.0)/10.0);
            phsnp->sir_threshold_call_drop_ps         = exp(phsnp->sir_threshold_call_drop_ps_db * log(10.0)/10.0);
            phsnp->int_threshold_call_drop_ps         = exp(phsnp->int_threshold_call_drop_ps_db * log(10.0)/10.0);
        }
        if (matchData->has_comm_lreg_arrival_ratio) {
            set_total_arrival_rate(lreg_traffic_idx, matchData->comm_arrival_rate / matchData->comm_lreg_arrival_ratio);
        }

        if (matchData->has_seed) {
            rg->RandomInit(matchData->seed);
        }

        reset_system();
        run_simulation(0, -1, matchData->initTime);
        if (error_state) { return; }
        reset_call_statistics(0);
        run_simulation(0, -1, matchData->simulationTime);
        if (error_state) { return; }

        /**********************************************************************************/
        /**** Extract data from PHSStatCountClass                                      ****/
        /**********************************************************************************/
        sc = (PHSStatCountClass *) stat_count;
        comm_total_num_request  = 0;
        comm_total_num_request_blocked_phys_chan = 0;
        comm_total_num_request_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[comm_traffic_idx])->get_num_attempt(CConst::RequestEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            comm_total_num_request                   += sc->num_request[comm_traffic_idx][attempt];
            comm_total_num_request_blocked_phys_chan += sc->num_request_blocked_phys_chan[comm_traffic_idx][attempt];
            comm_total_num_request_blocked_sir_cs     += sc->num_request_blocked_sir_cs[comm_traffic_idx][attempt];
        }

        comm_total_num_handover = 0;
        comm_total_num_handover_blocked_phys_chan = 0;
        comm_total_num_handover_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[comm_traffic_idx])->get_num_attempt(CConst::HandoverEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            comm_total_num_handover                   += sc->num_handover[comm_traffic_idx][attempt];
            comm_total_num_handover_blocked_phys_chan += sc->num_handover_blocked_phys_chan[comm_traffic_idx][attempt];
            comm_total_num_handover_blocked_sir_cs    += sc->num_handover_blocked_sir_cs[comm_traffic_idx][attempt];
        }

        lreg_total_num_request  = 0;
        lreg_total_num_request_blocked_phys_chan = 0;
        lreg_total_num_request_blocked_sir_cs = 0;
        num_attempt = ((PHSTrafficTypeClass *) traffic_type_list[lreg_traffic_idx])->get_num_attempt(CConst::RequestEvent);
        for (attempt=0; attempt<=num_attempt-1; attempt++) {
            lreg_total_num_request  += sc->num_request[lreg_traffic_idx][attempt];
            lreg_total_num_request_blocked_phys_chan += sc->num_request_blocked_phys_chan[lreg_traffic_idx][attempt];
            lreg_total_num_request_blocked_sir_cs     += sc->num_request_blocked_sir_cs[lreg_traffic_idx][attempt];
        }

        comm_num_rereq = sc->num_rereq[comm_traffic_idx];
        lreg_num_rereq = sc->num_rereq[lreg_traffic_idx];
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Compute Measured Parameters                                              ****/
        /**********************************************************************************/
        sc = (PHSStatCountClass *) stat_count;
        total_num_requests = comm_total_num_request
                           + comm_total_num_handover
                           + lreg_total_num_request;

        total_phys_block   = comm_total_num_request_blocked_phys_chan
                           + comm_total_num_handover_blocked_phys_chan
                           + lreg_total_num_request_blocked_phys_chan;

        total_cs_block     = comm_total_num_request_blocked_sir_cs
                           + comm_total_num_handover_blocked_sir_cs
                           + lreg_total_num_request_blocked_sir_cs;

        total_num_rereq    = comm_num_rereq
                           + lreg_num_rereq;

        if (total_num_requests) {
            phys_block_rate    = total_phys_block / total_num_requests;
            cs_block_rate      = total_cs_block / total_num_requests;
            rereq_rate         = total_num_rereq / total_num_requests;
            request_rate       = total_num_requests / stat->duration;
        } else {
            phys_block_rate    = 0.0;
            cs_block_rate      = 0.0;
            rereq_rate         = 0.0;
            request_rate       = 0.0;
        }
        /**********************************************************************************/

        /**********************************************************************************/
        /**** Compute metric                                                           ****/
        /**********************************************************************************/
        matchData->metric[matchData->caseIdx] = 0.0;
        for (measParamIdx=0; measParamIdx<=matchData->num_measured_parameter-1; measParamIdx++) {
            if (strcmp(matchData->measured_parameter_name[measParamIdx], "phys_block_rate")==0) {
                simVal = phys_block_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "cs_block_rate")==0) {
                simVal = cs_block_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "rereq_rate")==0) {
                simVal = rereq_rate;
            } else if (strcmp(matchData->measured_parameter_name[measParamIdx], "request_rate")==0) {
                simVal = request_rate;
            } else {
                sprintf(msg, "ERROR: Unrecognized parameter name \"%s\"\n", matchData->measured_parameter_name[measParamIdx]);
                PRMSG(stdout, msg);
                error_state = 1;
            }
            measVal = matchData->measured_parameter_value[measParamIdx];
            matchData->metric[matchData->caseIdx] += matchData->measured_parameter_weight[measParamIdx] * (measVal - simVal) * (measVal - simVal);
        }
        /**********************************************************************************/

        matchData->runCount++;

        matchData->caseIdx++;

        if (matchData->caseIdx == matchData->numCases) {

            min_idx = 0;
            min_metric = matchData->metric[0];
            for (i=1; i<=matchData->numCases-1; i++) {
                if (matchData->metric[i] < min_metric) {
                    min_idx = i;
                    min_metric = matchData->metric[i];
                }
            }

            sprintf(msg, "SET %d:\n", matchData->setIdx);
            PRMSG(stdout, msg);
            q = min_idx;
            for (i=0; i<=matchData->num_iteration_parameter-1; i++) {
                m = q % 3;
                param_val = matchData->p[i] + (m-1)*matchData->iteration_parameter_step_value[i]/(1<<matchData->setIdx);
                matchData->p[i] = param_val;
                q = (q - m) / 3;
                sprintf(msg, "set -%s %15.10e\n", matchData->iteration_parameter_name[i], param_val);
                PRMSG(stdout, msg);
                char tmp_msg[100];

                if (i==0) {
                    sprintf(tmp_msg, "set_total_traffic -traffic_type COMM -arrival_rate %15.10e\n", param_val);
                    PRMSG(stdout, tmp_msg);
                    sprintf(tmp_msg, "set_total_traffic -traffic_type LREG -arrival_rate %15.10e\n", param_val / matchData->comm_lreg_arrival_ratio);
                    PRMSG(stdout, tmp_msg);
                }
                if (i==1){
                    sprintf(tmp_msg, "set -int_threshold_call_drop_cs_db %15.10e\n", param_val+ matchData->thresholdDifference);
                    PRMSG(stdout, tmp_msg);
                }
                if (i==2){
                    sprintf(tmp_msg, "set -int_threshold_call_drop_ps_db %15.10e\n", param_val+ matchData->thresholdDifference);
                    PRMSG(stdout, tmp_msg);
                }
            }

            sprintf(msg, "INDEX  = %d\n", min_idx);
            PRMSG(stdout, msg);
            sprintf(msg, "METRIC = %15.10e\n", min_metric);
            PRMSG(stdout, msg);

            matchData->caseIdx = 0;
            for (i=0; i<=matchData->numCases-1; i++) {
                matchData->metric[i] = 0.0;
            }

            matchData->setIdx++;

            if (matchData->setIdx == matchData->num_sets) {
                done = 1;
            }
        }
        matchData->writeCheckPointFile(this);
    }

    free(matchData->p);
    free(matchData->metric);

    return;

}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: NetworkClass::setParam                                                 ****/
/******************************************************************************************/
void NetworkClass::setParam(char *paramName, double paramValue)
{
    int comm_traffic_idx;

    if (strcmp(paramName, "comm_arrival_rate")==0) {
        comm_traffic_idx = get_traffic_type_idx("COMM", 1);
        set_total_arrival_rate(comm_traffic_idx, paramValue);
        matchData->comm_arrival_rate = paramValue;
    } else if (strcmp(paramName, "int_threshold_call_request_cs_db")==0) {
        ((PHSNetworkClass *) this)->int_threshold_call_request_cs_db = paramValue;
        ((PHSNetworkClass *) this)->int_threshold_call_request_cs    = exp(paramValue * log(10.0)/10.0);
    } else if (strcmp(paramName, "sir_threshold_call_request_cs_db")==0) {
        ((PHSNetworkClass *) this)->sir_threshold_call_request_cs_db = paramValue;
        ((PHSNetworkClass *) this)->sir_threshold_call_request_cs    = exp(paramValue * log(10.0)/10.0);
    } else if (strcmp(paramName, "int_threshold_call_request_ps_db")==0) {
        ((PHSNetworkClass *) this)->int_threshold_call_request_ps_db = paramValue;
        ((PHSNetworkClass *) this)->int_threshold_call_request_ps    = exp(paramValue * log(10.0)/10.0);
    } else if (strcmp(paramName, "sir_threshold_call_request_ps_db")==0) {
        ((PHSNetworkClass *) this)->sir_threshold_call_request_ps_db = paramValue;
        ((PHSNetworkClass *) this)->sir_threshold_call_request_ps    = exp(paramValue * log(10.0)/10.0);

    } else if (strcmp(paramName, "init_time")==0) {
        matchData->initTime = paramValue;
    } else if (strcmp(paramName, "run_time")==0) {
        matchData->simulationTime = paramValue;
    } else if (strcmp(paramName, "threshold_difference")==0) {
        matchData->thresholdDifference = paramValue;
        matchData->hasThresholdDifference = 1;
    } else if (strcmp(paramName, "comm_lreg_arrival_ratio")==0) {
        matchData->comm_lreg_arrival_ratio = paramValue;
        matchData->has_comm_lreg_arrival_ratio = 1;
    } else if (strcmp(paramName, "seed")==0) {
        matchData->seed = (int) paramValue;
        matchData->has_seed = 1;

    } else {
        sprintf(msg, "ERROR: Unrecognized parameter name \"%s\"\n", paramName);
        PRMSG(stdout, msg);
        error_state = 1;
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: MatchDataClass::writeCheckPointFile                                    ****/
/******************************************************************************************/
void MatchDataClass::writeCheckPointFile(NetworkClass *np)
{
    int i;
    char *chptr;
    FILE *fp;

    if ( (checkPointFile == NULL) || (strcmp(checkPointFile, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(checkPointFile, "w")) ) {
        sprintf(np->msg, "ERROR: Unable to write to file %s\n", checkPointFile);
        PRMSG(stdout, np->msg); np->error_state = 1;
        return;
    }

    chptr = np->msg;
    chptr += sprintf(chptr, "RUN_COUNT: %d\n", runCount);
    chptr += sprintf(chptr, "NUM_PARAM: %d\n", num_iteration_parameter);
    chptr += sprintf(chptr, "SET_IDX: %d\n", setIdx);
    chptr += sprintf(chptr, "CASE_IDX: %d\n", caseIdx);
    PRMSG(fp, np->msg);

    for (i=0; i<=num_iteration_parameter-1; i++) {
        chptr = np->msg;
        chptr += sprintf(chptr, "P[%d]: %15.10f\n", i, p[i]);
        PRMSG(fp, np->msg);
    }
    for (i=0; i<=numCases-1; i++) {
        chptr = np->msg;
        chptr += sprintf(chptr, "METRIC[%d]: %15.10f\n", i, metric[i]);
        PRMSG(fp, np->msg);
    }

    if (fp != stdout) {
        fclose(fp);
    }

    return;
}
/******************************************************************************************/
