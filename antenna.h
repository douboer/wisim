/******************************************************************************************/
/**** FILE: antenna.h                                                                  ****/
/******************************************************************************************/

#ifndef ANTENNA_H
#define ANTENNA_H

class AntennaClass
{
public:
    AntennaClass(int type, char *strid = (char *) NULL);
    ~AntennaClass();
    int readFile(char *filepath, char *filename);
    double gainDB(double dx, double dy, double dz, double h_angle_rad);
    int checkGain(char *flname, int orient, int numpts);
    char *get_strid();
    int get_type();
    int get_is_omni();
    char *get_filename();
    double h_width;
    static int color;

private:
    char *filename, *strid;
    int type, is_omni;
    double *gain_db_v, *gain_db_h;
    double tilt_rad;
    double gain_fwd_db;   /* gain_v( tilt_rad      ) */
    double gain_back_db;  /* gain_v( PI - tilt_rad ) */
};
/******************************************************************************************/

#endif
