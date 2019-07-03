/******************************************************************************************/
/**** FILE: match_data.h                                                               ****/
/******************************************************************************************/

#ifndef MATCH_DATA_H
#define MATCH_DATA_H

#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************/
/**** CLASS: MatchDataClass                                                            ****/
/******************************************************************************************/
class MatchDataClass
{
public:
    MatchDataClass();
    ~MatchDataClass();
    void readData(char *filename);
    void writeCheckPointFile(NetworkClass *np);
    friend class NetworkClass;

private:
    int num_iteration_parameter;
    char **iteration_parameter_name;
    double *iteration_parameter_initial_value;
    double *iteration_parameter_step_value;

    int num_measured_parameter;
    char **measured_parameter_name;
    double *measured_parameter_value;
    double *measured_parameter_weight;

    int num_set_parameter;
    char **set_parameter_name;
    double *set_parameter_value;

    int num_sets;
    double initTime, simulationTime;
    double thresholdDifference;
    int hasThresholdDifference;
    double comm_lreg_arrival_ratio;
    int has_comm_lreg_arrival_ratio;
    double comm_arrival_rate;
    int seed, has_seed;

    char *checkPointFile;
    int runCount;
    int numCases;
    double *p, *metric;
    int setIdx, caseIdx;
};
/******************************************************************************************/

#endif
