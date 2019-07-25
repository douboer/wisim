/******************************************************************************************/
/**** FILE: antenna.cpp                                                                ****/
/**** Michael Mandell 9/16/03                                                          ****/
/******************************************************************************************/
/**** Simulate cellular network using event-based Monte-Carlo simulation               ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <QDebug>

#include "cconst.h"
#include "wisim.h"
#include "antenna.h"

/******************************************************************************************/
/**** FUNCTION: AntennaClass::AntennaClass                                             ****/
/******************************************************************************************/
AntennaClass::AntennaClass(int p_type, char *p_strid)
{
    if (p_strid) {
        strid = strdup(p_strid);
    } else {
        strid = (char *) NULL;
    }
    type = p_type;

    filename = (char *) NULL;
    gain_db_v = (double *) NULL;
    gain_db_h = (double *) NULL;
    is_omni = (type == CConst::antennaOmni ? 1 : 0);
    h_width = 360.0;
}
AntennaClass::~AntennaClass()
{
    if (strid) {
        free(strid);
    }
    if (filename) {
        free(filename);
    }
    if (gain_db_h) {
        free(gain_db_h);
    }
    if (gain_db_v) {
        free(gain_db_v);
    }
}
/******************************************************************************************/
/**** FUNCTION: AntennaClass:: "get_" functions                                        ****/
/******************************************************************************************/
char *AntennaClass::get_strid()    { return(strid);    }
int   AntennaClass::get_type()     { return(type);     }
int   AntennaClass::get_is_omni()  { return(is_omni);  }
char *AntennaClass::get_filename() { return(filename); }
/******************************************************************************************/
/**** FUNCTION: AntennaClass::readFile                                                 ****/
/******************************************************************************************/
int AntennaClass::readFile(char *filepath, char *p_filename)
{
    char *errmsg = CVECTOR(MAX_LINE_SIZE);
    char *full_path_filename;
    FILE *fp;

    filename = strdup(p_filename);
    full_path_filename = CVECTOR( strlen(filepath) + strlen(filename) );
    sprintf(full_path_filename, "%s%s", filepath, filename);

    if ( !(fp = fopen(full_path_filename, "rb")) ) {
        sprintf(errmsg, "ERROR: cannot open antenna file %s\n", full_path_filename);
        PRMSG(stdout, errmsg);
        return(0);
    }

    sprintf(errmsg, "Reading antenna file: %s\n", full_path_filename);
    PRMSG(stdout, errmsg);

    free(full_path_filename);

    enum state_enum {
        STATE_HEADER,
        STATE_HORIZONTAL,
        STATE_VERTICAL,
        STATE_DONE
    };

    int state = STATE_HEADER;
    int linenum = 0;
    int num_h = -1;
    int num_v = -1;
    int idx   = -1;
    double gain_db = 0.0;
    double tilt_deg;
    double *f_phs_h  = (double *) NULL;
    double *f_gain_h = (double *) NULL;
    double *f_phs_v  = (double *) NULL;
    double *f_gain_v = (double *) NULL;
    char *line = CVECTOR(MAX_LINE_SIZE);
    char *str1, *str2;

    while ( fgetline(fp, line) ) {
#if 0
        printf("%s", line);
#endif
        linenum++;
        str1 = strtok(line, CHDELIM);
        if ( str1 && (str1[0] != '#') ) {
            str2 = strtok(NULL, CHDELIM);
            switch(state) {
                case STATE_HEADER:
                    if (strcmp(str1, "NAME") == 0) {
                        strid = strdup(str2);
                    } else if (strcmp(str1, "FREQUENCY") == 0) {
                    } else if (strcmp(str1, "H_WIDTH") == 0) {
                        h_width = atof(str2);
                        is_omni = ( (h_width == 360.0) ? 1 : 0 );
                    } else if (strcmp(str1, "V_WIDTH") == 0) {
                    } else if (strcmp(str1, "FRONT_TO_BACK") == 0) {
                    } else if (strcmp(str1, "GAIN") == 0) {
                        int n = strlen(str2);
                        if ((n>3) && strcmp(str2+n-3, "dBi") == 0) {
                            str2[n-3] = (char) NULL;
                            gain_db = atof(str2);
                        } else {
                            sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                            "Gain: \"%s\" must be in dBi\n", filename, linenum, str2);
                            PRMSG(stdout, errmsg);
                            return(0);
                        }
                    } else if (strcmp(str1, "TILT") == 0) {
                        tilt_deg = -fabs(atof(str2));  /* Force tilt to be interpreted as pointing down */
                        tilt_rad = tilt_deg * PI / 180.0;
                        while(tilt_rad >= PI/2) { tilt_rad -= 2*PI; }
                        while(tilt_rad < -PI/2) { tilt_rad += 2*PI; }
                        if ( (tilt_rad < -PI/2) || (tilt_rad > PI/2) ) {
                            sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                            "TILT: \"%s\" must be between +/- 90 degrees\n", filename, linenum, str2);
                            PRMSG(stdout, errmsg);
                            return(0);
                        }
                    } else if (strcmp(str1, "POLARIZATION") == 0) {
                    } else if (strcmp(str1, "HORIZONTAL") == 0) {
                        num_h = atoi(str2);
                        f_phs_h  = DVECTOR(num_h);
                        f_gain_h = DVECTOR(num_h);
                        idx = 0;
                        state = STATE_HORIZONTAL;
                    } else {
                        sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                        "Unrecognized keywork in header: \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, errmsg);
                        return(0);
                    }
                    break;
                case STATE_HORIZONTAL:
                    if (idx <= num_h-1) {
                        f_phs_h[idx] = atof(str1);
                        f_gain_h[idx] = gain_db - atof(str2);
                        idx++;
                    } else if (strcmp(str1, "VERTICAL") == 0) {
                        num_v = atoi(str2);
                        f_phs_v  = DVECTOR(num_v);
                        f_gain_v = DVECTOR(num_v);
                        idx = 0;
                        state = STATE_VERTICAL;
                    } else {
                        sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                        "Unrecognized keywork in header: \"%s\"\n", filename, linenum, str1);
                        PRMSG(stdout, errmsg);
                        return(0);
                    }
                    break;
                case STATE_VERTICAL:
                    if (idx <= num_v-1) {
                        f_phs_v[idx] = atof(str1);
                        f_gain_v[idx] = gain_db - atof(str2);
                        idx++;
                    } else {
                        CORE_DUMP;
                    }
                    if (idx == num_v) {
                        state = STATE_DONE;
                    }
                    break;
                case STATE_DONE:
                    sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                     "False additional data encountered\n", filename, linenum);
                    PRMSG(stdout, errmsg);
                    return(0);
                    break;
                default:
                    sprintf(errmsg, "ERROR: invalid antenna file \"%s(%d)\"\n"
                                     "Invalid state (%d) encountered\n", filename, linenum, state);
                    PRMSG(stdout, errmsg);
                    return(0);
                    break;
            }
        }
    }

    printf("Smoothing Vertical Antenna Pattern for %s ... \n", filename);
    int sn = 4, k, k_idx;
    double pwr;
    double *new_gain_v = DVECTOR(num_v);
    for (idx=0; idx<=num_v-1; idx++) {
        pwr = 0.0;
        for (k=-sn; k<=sn; k++) {
            k_idx = (idx + k + num_v) % num_v;
            pwr += exp(f_gain_v[k_idx]*log(10.0)/10.0);
        }
        pwr /= (2*sn+1);
        new_gain_v[idx] = 10.0*log(pwr)/log(10.0);
    }
    free(f_gain_v);
    f_gain_v = new_gain_v;

    if (state != STATE_DONE) {
        sprintf(errmsg, "ERROR: invalid antenna file \"%s\"\n"
                         "Premature EOF encountered\n", filename);
        PRMSG(stdout, errmsg);
        return(0);
    }

    if ( (type == CConst::antennaLUT) || (type == CConst::antennaLUT_H) ) {
        gain_db_h = DVECTOR(CConst::antenna_num_interp_pts);
        if (!spline_init_lut(f_phs_h, f_gain_h, num_h, CConst::antenna_num_interp_pts, gain_db_h)) {
            sprintf(errmsg, "ERROR: invalid antenna file \"%s\"\n"
                         "Unable to create spline for horizontal antenna radiation pattern\n", filename);
            PRMSG(stdout, errmsg);
            return(0);
        }
    } else {
        gain_db_h = (double *) NULL;
    }

    if ( (type == CConst::antennaLUT) || (type == CConst::antennaLUT_V) ) {
        gain_db_v = DVECTOR(CConst::antenna_num_interp_pts);
        if (!spline_init_lut(f_phs_v, f_gain_v, num_v, CConst::antenna_num_interp_pts, gain_db_v)) {
            sprintf(errmsg, "ERROR: invalid antenna file \"%s\"\n"
                         "Unable to create spline for vertical antenna radiation pattern\n", filename);
            PRMSG(stdout, errmsg);
            return(0);
        }
        double pi_minus_tilt = PI - tilt_rad;
        while(pi_minus_tilt >= PI) { pi_minus_tilt -= 2*PI; }

        gain_fwd_db  = spline_eval_lut(tilt_rad,      gain_db_v, CConst::antenna_num_interp_pts);
        gain_back_db = spline_eval_lut(pi_minus_tilt, gain_db_v, CConst::antenna_num_interp_pts);
    } else {
        gain_db_v = (double *) NULL;
    }

    free(f_phs_h);
    free(f_gain_h);
    free(f_phs_v);
    free(f_gain_v);
    free(line);
    free(errmsg);

    fclose(fp);

    return(1);
}
/******************************************************************************************/
/**** FUNCTION:  AntennaClass::gainDB                                                  ****/
/**** This routine computes the antenna power gain for the specified sectorted antenna ****/
/**** in the direction of the vector(dx, dy, dz).                                      ****/
/******************************************************************************************/
double AntennaClass::gainDB(double dx, double dy, double dz, double h_angle_rad)
{
    double theta   = 0.0;
    double phi     = 0.0;
    double gain_db = 0.0;

    if (type == CConst::antennaOmni) {
        gain_db = 0.0;
    } else if (type == CConst::antennaLUT_H) {
        phi = atan2(dy, dx);
        phi -= h_angle_rad;
        while(phi >= PI) { phi -= 2*PI; }
        while(phi < -PI) { phi += 2*PI; }
        gain_db = spline_eval_lut(phi, gain_db_h, CConst::antenna_num_interp_pts);
    } else if (type == CConst::antennaLUT_V) {
        theta = atan2(dz, sqrt(dx*dx + dy*dy));
        gain_db = spline_eval_lut(theta, gain_db_v, CConst::antenna_num_interp_pts);
    } else if (type == CConst::antennaLUT) {
        phi = atan2(dy, dx);
        phi -= h_angle_rad;
        while(phi >= PI) { phi -= 2*PI; }
        while(phi < -PI) { phi += 2*PI; }
        theta = atan2(dz, sqrt(dx*dx + dy*dy));

        double pi_minus_theta = PI - theta;
        while(pi_minus_theta >= PI) { pi_minus_theta -= 2*PI; }

        double gv1 = spline_eval_lut(theta,          gain_db_v, CConst::antenna_num_interp_pts);
        double gv2 = spline_eval_lut(pi_minus_theta, gain_db_v, CConst::antenna_num_interp_pts);
        double gh   = spline_eval_lut(phi,           gain_db_h, CConst::antenna_num_interp_pts);

        gain_db = (1.0 - fabs(phi)/PI)*(gv1 - gain_fwd_db)
                + (fabs(phi)/PI)*(gv2 - gain_back_db)
                + gh;
    } else {
        CORE_DUMP;
    }

    return(gain_db);
}
/******************************************************************************************/
/**** FUNCTION: check_antenna_gain                                                     ****/
/**** This routine writes antenna gain in two column format to the specified file.     ****/
/**** The first column is angle in degrees from -180 to 180 and the second column is   ****/
/**** antenna gain in dB.  The purpose of this routine is to privide a means of        ****/
/**** verifying the integrity of the spline interpolation used on the antenna data.    ****/
/****     orient == 0 : Horizontal pattern                                             ****/
/****     orient == 1 : Vertical   pattern                                             ****/
/******************************************************************************************/
int AntennaClass::checkGain(char *flname, int orient, int numpts)
{
    int i;
    double phase_deg, phase_rad, dx, dy, dz, gain_db;
    char *chptr;
    char *errmsg = CVECTOR(MAX_LINE_SIZE);
    FILE *fp;

    if (numpts <= 0) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine check_antenna_gain()\n");
        chptr += sprintf(chptr, "numpts = %d must be > 0\n", numpts);
        PRMSG(stdout, errmsg);
        return(0);
    }

    if (!flname) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine check_antenna_gain()\n");
        chptr += sprintf(chptr, "No filename specified\n");
        PRMSG(stdout, errmsg);
        return(0);
    }

    if ( !(fp = fopen(flname, "w")) ) {
        chptr = errmsg;
        chptr += sprintf(chptr, "ERROR in routine check_antenna_gain()\n");
        chptr += sprintf(chptr, "Unable to write to file %s\n", flname);
        PRMSG(stdout, errmsg);
        return(0);
    }

    chptr = errmsg;
    chptr += sprintf(chptr, "CHECKING %s ANTENNA GAIN\n", (orient==0 ? "HORIZONTAL" : "VERTICAL") );
    chptr += sprintf(chptr, "WRITING %d POINTS to FILE %s\n", numpts, flname);
    PRMSG(stdout, errmsg);

    for (i=0; i<=numpts-1; i++) {
        phase_deg = -180.0 + 360.0*i/numpts;
        phase_rad = phase_deg * PI / 180.0;
        dx = cos(phase_rad);
        dy = sin(phase_rad);
        if (orient == 0) {
            dz = sin(tilt_rad);
            gain_db = gainDB(dx, dy, dz, 0.0);
        } else {
            gain_db = gainDB(dx, 0.0, dy, 0.0);
        }
        sprintf(errmsg, "%12.10f %12.10f\n", phase_deg, gain_db);
        PRMSG(fp, errmsg);
    }
    sprintf(errmsg, "\n");
    PRMSG(fp, errmsg);

    fclose(fp);
    free(errmsg);

    return(1);
}
/******************************************************************************************/
