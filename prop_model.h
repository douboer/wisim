/******************************************************************************************/
/**** FILE: prop_model.h                                                               ****/
/******************************************************************************************/

#ifndef PROP_MODEL_H
#define PROP_MODEL_H

#include <stdio.h>

class FigureEditor;
class NetworkClass;
class SectorClass;
class MapClutterClass;

class PropModelClass
{
public:
    PropModelClass(char *strid = (char *) NULL);
    virtual ~PropModelClass();

    char *get_strid();
    void set_strid(char *, int);
    void report(NetworkClass *np, FILE *fp);
    virtual const int type();
    virtual double prop_power_loss(NetworkClass *, SectorClass *, int, int, int useheight=1, double angle_deg=0.0);
    virtual void get_prop_model_param_ptr(int, char *, int&, int *&, double *&);
    virtual int  comp_num_prop_param();
    virtual void print_params(FILE *, char *, int);
    virtual int is_clutter_model();

private:
    char *strid;
    int flag;
};

class ExpoPropModelClass : public PropModelClass
{
public:
    ExpoPropModelClass(char *strid = (char *) NULL);
    const int type();
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
    friend class PropModMgrDia;
    friend class ExponentialInfoWidClass;

private:
    double exponent, coefficient;
};

class RSquaredPropModelClass : public PropModelClass
{
public:
    RSquaredPropModelClass(char *strid = (char *) NULL);
    const int type();
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
    friend class PropModMgrDia;
private:
    double coefficient;
};

class PwLinPropModelClass : public PropModelClass
{
public:
    PwLinPropModelClass(char *strid = (char *) NULL);
    const int type();
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
private:
    double y0, ys, py, s1, k0, k1;
};

class TerrainPropModelClass : public PropModelClass
{
// xxxxxxxxxx DELETE
// Currently only used for backward compatability with old geometry files
public:
    TerrainPropModelClass(MapClutterClass *& p_map_clutter, char *strid = (char *) NULL);
    const int type();
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
private:
    MapClutterClass *& map_clutter;
    int      num_clutter_type;  //the number of the clutter type
    int      useheight;         //consider the bs height in the model.0 NA,1 valid
    double   val_y;             //the value of the inflexion of line
    double   val_py;            //the power value of the inflexion
    double   val_s1;            //the slope of the first part
    double   val_s2;            //the slope of the second part
    double   *vec_k;            //the coefficient of the clutter and base station height
};

class SegmentPropModelClass : public PropModelClass
{
public:
    SegmentPropModelClass(MapClutterClass *& p_map_clutter, char *strid = (char *) NULL);
    ~SegmentPropModelClass();
    const int type();
    int fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double avg_max_logd, double &error);
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    double err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x,double *vec_b);
    double err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b, double min_height);
    double optimize_clutter_coeffs(int num_road_test_pt, double **mx_a,double *vec_b,double *vec_logd);
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power,double *vec_logd,int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);
    int adjust_near_field(double min_logd, double logr0, double frequency, char *msg, int ignore_fs_check);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    // friend int NetworkClass::check_prop_model();
    // friend void NetworkClass::report_prop_model_param(int param, int model, char *filename);
    friend class FigureEditor;
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
    friend class PropModMgrDia;
    friend class SegmentInfoWidClass;
    friend class ClutterGlobalPropModelClass;
    friend class GenericClutterPropModelClass;

private:
    MapClutterClass *& map_clutter;
    int num_clutter_type;       //the number of the clutter type
    int useheight;              //consider the bs height in the model.0 NA,1 valid
    int num_inflexion;          //number of points in the piece-wise linear model
    double *x,*y;               //point of inflexion
    double start_slope;         //start slope
    double final_slope;         //final slope
    double *vec_k;              //clutter and height coefficients
    static int cutoff_slope;
};

class SegmentWithThetaPropModelClass : public PropModelClass
{
public:
    SegmentWithThetaPropModelClass(MapClutterClass *& p_map_clutter, char *strid = (char *) NULL);
    ~SegmentWithThetaPropModelClass();
    const int type();
    int fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double &error, double *vec_cos, double *vec_sin);
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    double err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x,double *vec_b, double *vec_cos, double *vec_sin);
    double err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b, double min_height, double *vec_cos, double *vec_sin);
    double optimize_clutter_coeffs(int num_road_test_pt, double **mx_a,double *vec_b,double *vec_logd);
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power,double *vec_logd,int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);
    void set_num_inflexion(int n);
    void add_cutoff_slope(double d, double level, double s_max_level_db, double slope);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model_with_theta(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    // friend int NetworkClass::check_prop_model();
    friend class FigureEditor;
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
    friend class PropModMgrDia;

private:
    MapClutterClass *& map_clutter;
    int num_clutter_type;       //the number of the clutter type
    int useheight;              //consider the bs height in the model.0 NA,1 valid
    int num_inflexion;          //number of points in the piece-wise linear model
    double *x,*y;               //point of inflexion
    int *n_series_y;
    double **c_series_y, **s_series_y;
    double start_slope;         //start slope
    double final_slope;         //final slope
    int n_series_start_slope, n_series_final_slope;
    double *c_series_start_slope, *s_series_start_slope;
    double *c_series_final_slope, *s_series_final_slope;
    double *vec_k;              //clutter and height coefficients
    double max_level_db;
    static int cutoff_slope;
};

class SegmentAnglePropModelClass : public PropModelClass
{
public:
    SegmentAnglePropModelClass(MapClutterClass *& p_map_clutter, char *strid = (char *) NULL);
    ~SegmentAnglePropModelClass();
    const int type();
    int fit_data(int num_road_test_pt, double *vec_logd, double *vec_b, double min_height, double &error, double *vec_cos, double *vec_sin);
    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=1, double angle_deg=0.0);
    double err_fn_of_x(int num_road_test_pt, double &val_y, double *vec_logd, double *vec_x,double *vec_b, double *vec_cos, double *vec_sin);
    double err_fn_of_s2(int num_road_test_pt, double *vec_logd, double *vec_x,double *vec_b, double min_height, double *vec_cos, double *vec_sin);
    double optimize_clutter_coeffs(int num_road_test_pt, double **mx_a,double *vec_b,double *vec_logd);
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power,double *vec_logd,int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int fmt);
    void set_num_inflexion(int n);
    void add_cutoff_slope(double d, double level, double s_max_level_db, double slope);
    void comp_angular_seg(double cos_a, double sin_a, int &a_seg, int &a_seg_p1, double &alpha);

    friend class NetworkClass;
    // friend int NetworkClass::process_command(char *line);
    // friend void NetworkClass::comp_prop_model_segment_angle(int num_scan_index, int *scan_index_list, int useheight, int adjust_angles, const int err);
    // friend int NetworkClass::check_prop_model();
    friend class FigureEditor;
    friend class SelPropModDia;
    friend class PropModWidget;
    friend class PropModWidgetView;
    friend class PropModMgrDia;

private:
    MapClutterClass *& map_clutter;
    int num_clutter_type;       //the number of the clutter type
    int useheight;              //consider the bs height in the model.0 NA,1 valid
    int num_inflexion;          //number of points in the piece-wise linear model
    double *x,**y;              //point of inflexion
    int n_angle;
    int *use_n_y;
    double *start_slope;        //start slope
    double *final_slope;        //final slope
    int use_n_start_slope, use_n_final_slope;
    double *vec_k;              //clutter and height coefficients
    double max_level_db;
    double *cc, *ss;
    static int cutoff_slope;
};
#endif

